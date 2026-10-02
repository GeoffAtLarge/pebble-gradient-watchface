var COLOR_OPTIONS = [
  { "label": "Graphite", "value": "0" },
  { "label": "Silver", "value": "1" },
  { "label": "Gold", "value": "2" },
  { "label": "Rose Gold", "value": "3" },
  { "label": "Blue", "value": "4" },
  { "label": "Green", "value": "5" },
  { "label": "Red", "value": "6" },
  { "label": "Purple", "value": "7" }
];

module.exports = [
  { "type": "heading", "defaultValue": "Gradient Sweep" },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Colors" },
      {
        "type": "select",
        "messageKey": "HOURCOLOR",
        "label": "Hour color",
        "defaultValue": "1",
        "options": COLOR_OPTIONS
      },
      {
        "type": "select",
        "messageKey": "MINCOLOR",
        "label": "Minute color",
        "defaultValue": "0",
        "options": COLOR_OPTIONS
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
