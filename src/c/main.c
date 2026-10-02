/**
 * Gradient Sweep - analog time shown as two overlapping rotating gradient discs.
 *
 * Each disc is a conic gradient, 50% opaque at its hand and fading to 0% over
 * 180 degrees: the hour disc clockwise, the minute disc counter-clockwise.
 * The minute disc is layered over the hour disc.
 *
 * Rendered per pixel into the framebuffer with ordered dithering, so the
 * 4-levels-per-channel Pebble palette still yields smooth gradients.
 * Redraws once a minute only.
 */

#include <pebble.h>

#define SETTINGS_KEY 1

#define SHAPE_CIRCLE 0        // hour wedge is contained by a circle
#define SHAPE_RECT 1          // ...or by an inset rectangle (a square on round watches)

// The hour container is two thirds of the screen: of the narrowest dimension
// for a circle (diameter), of each dimension for a rectangle.
#define CONTAINER_NUM 2
#define CONTAINER_DEN 3


typedef struct {
  uint8_t hour_color;   // index into COLORS
  uint8_t min_color;
  uint8_t shape;        // SHAPE_*: container of the hour wedge
  uint8_t show_ring;    // outline of the hour container
  uint8_t min_circle;   // rectangular watches: keep the minute wedge inside a circle as wide as the screen
  uint8_t min_ring;     // outline of that circle
} Settings;

// Darkest tone of each wedge gradient; the face itself is white
static const uint8_t COLORS[][3] = {
  {40, 40, 45},     // Graphite
  {130, 135, 145},  // Silver
  {190, 125, 10},   // Gold
  {200, 100, 100},  // Rose Gold
  {30, 90, 200},    // Blue
  {30, 150, 70},    // Green
  {200, 30, 30},    // Red
  {130, 50, 170},   // Purple
  {0, 0, 0},        // Black
  {85, 85, 85},     // Dark Gray
  {0, 0, 85},       // Navy
  {0, 85, 0},       // Dark Green
  {85, 0, 0},       // Maroon
  {85, 0, 85},      // Dark Purple
};
#define NUM_COLORS ((int)(sizeof(COLORS) / sizeof(COLORS[0])))

#define DEFAULT_HOUR_COLOR 1
#define DEFAULT_MIN_COLOR 8

static Window *s_window;
static Layer *s_canvas_layer;
static Settings s_settings;

// ----------------------------------------------------------------------------
// Fixed-point angle from 12 o'clock, clockwise, 0..TRIG_MAX_ANGLE-1
// ----------------------------------------------------------------------------

// atan(n/d) for 0 <= n <= d, d > 0, returned in trig angle units (0..8192 = 0..45 deg)
static int32_t oct_angle(int32_t n, int32_t d) {
  int32_t t = (n << 14) / d;                              // Q14, 0..16384
  int32_t t2 = (t * t) >> 14;
  int32_t f = (t * (20387 - ((4003 * t2) >> 14))) >> 14;  // ~atan(t) / (pi/4), Q14
  return f >> 1;
}

// angle from north for ax, ay >= 0, result 0..16384
static int32_t north_angle(int32_t ax, int32_t ay) {
  if (ax == 0 && ay == 0) return 0;
  if (ax <= ay) return oct_angle(ax, ay);
  return 16384 - oct_angle(ay, ax);
}

// dx right, dy down (screen coordinates)
static int32_t clock_angle(int32_t dx, int32_t dy) {
  int32_t ax = dx < 0 ? -dx : dx;
  int32_t up = -dy;
  int32_t a;
  if (up >= 0) {
    a = north_angle(ax, up);
  } else {
    a = 32768 - north_angle(ax, -up);
  }
  if (dx < 0) a = TRIG_MAX_ANGLE - a;
  return a;
}

// ----------------------------------------------------------------------------
// Dithering
// ----------------------------------------------------------------------------

// 8x8 Bayer threshold, 0..252
static inline int bayer8(int x, int y) {
  int a = x ^ y;
  int v = ((y & 1) << 5) | ((a & 1) << 4) | ((y & 2) << 2) |
          ((a & 2) << 1) | ((y & 4) >> 1) | ((a & 4) >> 2);
  return v * 4;
}

static inline uint8_t dither_level(int value, int thr) {
  int l = (value * 3 + thr) / 255;
  return l > 3 ? 3 : l;
}

static inline uint8_t solid_color(const uint8_t *rgb) {
  return 0xC0 | (((rgb[0] + 42) / 85) << 4) | (((rgb[1] + 42) / 85) << 2) | ((rgb[2] + 42) / 85);
}

// ----------------------------------------------------------------------------
// Drawing
// ----------------------------------------------------------------------------

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int cx = bounds.size.w / 2;
  const int cy = bounds.size.h / 2;
  const int narrow = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int dial_radius = (narrow * CONTAINER_NUM) / (CONTAINER_DEN * 2);
  const int radius2 = dial_radius * dial_radius;
  const int min_radius = narrow / 2;
  const int min_radius2 = min_radius * min_radius;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  if (!t) return;

  const int32_t min_angle = (t->tm_min * TRIG_MAX_ANGLE) / 60;
  const int32_t hour_angle = (((t->tm_hour % 12) * 60 + t->tm_min) * TRIG_MAX_ANGLE) / 720;

  const uint8_t *hc = COLORS[s_settings.hour_color < NUM_COLORS ? s_settings.hour_color : DEFAULT_HOUR_COLOR];
  const uint8_t *mc = COLORS[s_settings.min_color < NUM_COLORS ? s_settings.min_color : DEFAULT_MIN_COLOR];
  const uint8_t ring_px = solid_color(hc);
  const uint8_t face_px = 0xFF;  // white
  const bool rect = s_settings.shape == SHAPE_RECT;
  const bool ring = s_settings.show_ring;
#if defined(PBL_ROUND)
  const bool min_circle = false;  // the screen is already a circle
  const bool min_ring = false;
#else
  const bool min_circle = s_settings.min_circle;
  const bool min_ring = min_circle && s_settings.min_ring;
#endif
  const uint8_t min_ring_px = solid_color(mc);

  // With a circular minute area the hour area is a circle or a true square, never a tall rectangle
  int half_w = (bounds.size.w * CONTAINER_NUM) / (CONTAINER_DEN * 2);
  int half_h = (bounds.size.h * CONTAINER_NUM) / (CONTAINER_DEN * 2);
  if (min_circle) half_h = half_w = (narrow * CONTAINER_NUM) / (CONTAINER_DEN * 2);

  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) return;
  const GRect fb_bounds = gbitmap_get_bounds(fb);

  for (int y = fb_bounds.origin.y; y < fb_bounds.origin.y + fb_bounds.size.h; y++) {
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(fb, y);
    const int dy = y - cy;
    for (int x = row.min_x; x <= row.max_x; x++) {
      const int dx = x - cx;
      const int adx = dx < 0 ? -dx : dx;
      const int ady = dy < 0 ? -dy : dy;
      const int r2 = dx * dx + dy * dy;
      bool inside;
      bool on_ring = false;
      if (rect) {
        inside = adx <= half_w && ady <= half_h;
        on_ring = ring && inside && (adx == half_w || ady == half_h);
      } else {
        inside = r2 <= radius2;
        int d = r2 - radius2;
        on_ring = ring && (d < 0 ? -d : d) <= dial_radius;
      }

      if (on_ring) {
        row.data[x] = ring_px;
        continue;
      }

      if (min_ring) {
        int d = r2 - min_radius2;
        if ((d < 0 ? -d : d) <= min_radius) {
          row.data[x] = min_ring_px;
          continue;
        }
      }

      // The minute wedge is limited to its own circle if enabled
      if (min_circle && r2 > min_radius2) {
        row.data[x] = face_px;
        continue;
      }

      // The hour wedge never leaves its container; the minute wedge runs on past it
      const int32_t theta = clock_angle(dx, dy);

      // Each disc is a conic gradient: 50% opaque at its hand, fading to 0% over 180 degrees
      // and staying clear for the rest of the turn. The hour disc fades clockwise, the minute
      // disc counter-clockwise.
      int op_h = 0;
      if (inside) {
        op_h = 127 - (((theta - hour_angle) & (TRIG_MAX_ANGLE - 1)) >> 8);   // 0..127 of 255
        if (op_h < 0) op_h = 0;
      }
      int op_m = 127 - (((min_angle - theta) & (TRIG_MAX_ANGLE - 1)) >> 8);
      if (op_m < 0) op_m = 0;

      // Layered like real translucent discs: the hour disc over the white face,
      // then the minute disc over that
      const int thr = bayer8(x, y);
      uint8_t lv[3];
      for (int i = 0; i < 3; i++) {
        int tone = 255 + ((hc[i] - 255) * op_h) / 255;
        tone += ((mc[i] - tone) * op_m) / 255;
        lv[i] = dither_level(tone, thr);
      }
      row.data[x] = 0xC0 | (lv[0] << 4) | (lv[1] << 2) | lv[2];
    }
  }

  graphics_release_frame_buffer(ctx, fb);
}

// ----------------------------------------------------------------------------
// Settings
// ----------------------------------------------------------------------------

static int tuple_to_int(const Tuple *tuple) {
  if (tuple->type == TUPLE_CSTRING) return atoi(tuple->value->cstring);
  return (int)tuple->value->int32;
}

static void validate_settings(void) {
  if (s_settings.hour_color >= NUM_COLORS) s_settings.hour_color = DEFAULT_HOUR_COLOR;
  if (s_settings.min_color >= NUM_COLORS) s_settings.min_color = DEFAULT_MIN_COLOR;
  if (s_settings.shape > SHAPE_RECT) s_settings.shape = SHAPE_CIRCLE;
  s_settings.show_ring = s_settings.show_ring ? 1 : 0;
  s_settings.min_circle = s_settings.min_circle ? 1 : 0;
  s_settings.min_ring = s_settings.min_ring ? 1 : 0;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t;
  if ((t = dict_find(iter, MESSAGE_KEY_HOURCOLOR))) s_settings.hour_color = tuple_to_int(t);
  if ((t = dict_find(iter, MESSAGE_KEY_MINCOLOR))) s_settings.min_color = tuple_to_int(t);
  if ((t = dict_find(iter, MESSAGE_KEY_SHAPE))) s_settings.shape = tuple_to_int(t);
  if ((t = dict_find(iter, MESSAGE_KEY_MINCIRCLE))) s_settings.min_circle = tuple_to_int(t) ? 1 : 0;
  if ((t = dict_find(iter, MESSAGE_KEY_MINRING))) s_settings.min_ring = tuple_to_int(t) ? 1 : 0;
  if ((t = dict_find(iter, MESSAGE_KEY_RING))) s_settings.show_ring = tuple_to_int(t) ? 1 : 0;
  validate_settings();
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}

static void load_settings(void) {
  s_settings = (Settings) {
    .hour_color = DEFAULT_HOUR_COLOR,
    .min_color = DEFAULT_MIN_COLOR,
    .shape = SHAPE_CIRCLE,
    .show_ring = 1,
    .min_circle = 0,
    .min_ring = 1,
  };
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  }
  validate_settings();
}

// ----------------------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------------------

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_canvas_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  s_canvas_layer = NULL;
}

static void init(void) {
  load_settings();

  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  app_message_register_inbox_received(inbox_received);
  app_message_open(128, 128);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
