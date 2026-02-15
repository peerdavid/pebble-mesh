module.exports = [
  {
    "type": "heading",
    "defaultValue": "Pebble Mesh Settings"
  },
  {
    "type": "text",
    "defaultValue": "Made by David Peer, 2026"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Appearance"
      },
      {
        "type": "select",
        "messageKey": "COLOR_THEME",
        "label": "Color Theme",
        "defaultValue": "dark",
        "options": [
          {
            "label": "Dark",
            "value": "dark"
          },
          {
            "label": "Light",
            "value": "light"
          },
          {
            "label": "Dynamic (Sunrise/Sunset)",
            "value": "dynamic"
          },
          {
            "label": "Dynamic (Quiet)",
            "value": "quiet"
          },
        ]
      },
      {
        "type": "toggle",
        "messageKey": "ENABLE_ANIMATIONS",
        "label": "Enable Animations",
        "defaultValue": true
      }
    ]
  },
    {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Layout"
      },
      {
        "type": "select",
        "messageKey": "LAYOUT_UPPER_LEFT",
        "label": "Upper Left",
        "defaultValue": "0",
        "options": [
          { "label": "Weather Icon", "value": "0" },
          { "label": "Temperature", "value": "1" },
          { "label": "Steps", "value": "2" },
          { "label": "Battery", "value": "3" },
          { "label": "Calendar", "value": "6" },
          { "label": "None", "value": "5" }
        ]
      },
      {
        "type": "select",
        "messageKey": "LAYOUT_UPPER_RIGHT",
        "label": "Upper Right",
        "defaultValue": "1",
        "options": [
          { "label": "Weather Icon", "value": "0" },
          { "label": "Temperature", "value": "1" },
          { "label": "Steps", "value": "2" },
          { "label": "Battery", "value": "3" },
          { "label": "Calendar", "value": "6" },
          { "label": "None", "value": "5" }
        ]
      },
      {
        "type": "select",
        "messageKey": "LAYOUT_LOWER_LEFT",
        "label": "Lower Left",
        "defaultValue": "2",
        "options": [
          { "label": "Weather Icon", "value": "0" },
          { "label": "Temperature", "value": "1" },
          { "label": "Steps", "value": "2" },
          { "label": "Battery", "value": "3" },
          { "label": "Calendar", "value": "6" },
          { "label": "None", "value": "5" }
        ]
      },
      {
        "type": "select",
        "messageKey": "LAYOUT_LOWER_RIGHT",
        "label": "Lower Right",
        "defaultValue": "3",
        "options": [
          { "label": "Weather Icon", "value": "0" },
          { "label": "Temperature", "value": "1" },
          { "label": "Steps", "value": "2" },
          { "label": "Battery", "value": "3" },
          { "label": "Calendar", "value": "6" },
          { "label": "None", "value": "5" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Health"
      },
      {
        "type": "input",
        "messageKey": "STEP_GOAL",
        "label": "Daily Step Goal",
        "defaultValue": "10000",
        "description": "Set your daily step goal (100-100000)",
        "attributes": {
          "type": "number",
          "min": 100,
          "max": 100000,
          "placeholder": "10000"
        }
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather"
      },
      {
        "type": "input",
        "messageKey": "WEATHER_LOCATION_CONFIG",
        "label": "Location",
        "defaultValue": "",
        "description": "Enter your city name (e.g., Vienna, New York, Tokyo) or leave empty to use GPS location automatically. Weather data is fetched from Open-Meteo API.",
        "attributes": {
          "placeholder": "e.g., Vienna (or leave empty for GPS)"
        }
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMPERATURE_UNIT",
        "label": "Temperature Unit",
        "defaultValue": "celsius",
        "options": [
          {
            "label": "Celsius",
            "value": "celsius"
          },
          {
            "label": "Fahrenheit",
            "value": "fahrenheit"
          }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];