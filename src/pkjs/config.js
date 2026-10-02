// Swatch grid for the color pickers (hex, no #)
var COLOR_LAYOUT = [
  ["000000", "555555", "282830", "000055", "005500", "550000", "550055"],
  ["828791", "BE7D0A", "C86464", "1E5AC8", "1E9646", "C81E1E", "8232AA"]
];

module.exports = [
  { "type": "heading", "defaultValue": "Gradient Sweep" },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Colors" },
      {
        "type": "color",
        "messageKey": "HOURCOLOR",
        "label": "Hour color",
        "defaultValue": "828791",
        "sunlight": false,
        "layout": COLOR_LAYOUT
      },
      {
        "type": "color",
        "messageKey": "MINCOLOR",
        "label": "Minute color",
        "defaultValue": "000000",
        "sunlight": false,
        "layout": COLOR_LAYOUT
      },
      {
        "type": "select",
        "messageKey": "RAMP",
        "label": "Gradient",
        "description": "Logarithmic keeps the discs dark longer before fading out.",
        "defaultValue": "0",
        "options": [
          { "label": "Logarithmic", "value": "0" },
          { "label": "Linear", "value": "1" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Layout" },
      {
        "type": "select",
        "messageKey": "SHAPE",
        "label": "Hour area shape",
        "description": "The hour gradient always stays inside this shape, two thirds of the screen size. The minute gradient extends to the screen edge.",
        "defaultValue": "0",
        "options": [
          { "label": "Circle", "value": "0" },
          { "label": "Rectangle", "value": "1" }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "RING",
        "label": "Outline around hour area",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "MINCIRCLE",
        "label": "Contain minutes in a circle",
        "description": "Rectangular watches only. Keeps the minute gradient inside a circle as wide as the screen. A Rectangle hour area becomes a square.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "MINRING",
        "label": "Outline around minute circle",
        "description": "Rectangular watches only. Applies when minutes are contained in a circle.",
        "defaultValue": true
      }
    ]
  },
  { "type": "submit", "defaultValue": "Save" }
];
