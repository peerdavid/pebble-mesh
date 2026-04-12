module.exports = [
  {
    "type": "heading",
    "defaultValue": "Pebble Mesh"
  },
  {
    "type": "text",
    "defaultValue": "<span>David Peer, 2026 | <a href='https://www.paypal.com/donate/?hosted_button_id=Y4PDJU84LC3N2'>Buy me a coffee</a> &#9749; </span>"
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
      },
      {
        "type": "toggle",
        "messageKey": "ENABLE_MESH",
        "label": "Enable Mesh",
        "description": "Wait... you're disabling the mesh on a watchface called Mesh? Bold move. Improves readability, destroys identity.",
        "defaultValue": true
      },
      {
        "type": "input",
        "messageKey": "DATE_FORMAT",
        "label": "Date Format or Static Text",
        "defaultValue": " %a %d",
        "description": "Default: ' %a %d'. Patterns: %a = day name, %b = month name, %d = day, %m = month number. Examples: ' %d %b' = 12 Apr, ' %d/%m' = 12/04, ' %m/%d' = 04/12",
        "attributes": {
          "placeholder": " %a %d"
        }
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
      },
      {
        "type": "select",
        "messageKey": "DISCONNECT_POSITION",
        "label": "Disconnect Icon",
        "defaultValue": "0",
        "description": "Show disconnect icon when the watch loses connection.",
        "options": [
          { "label": "Disabled", "value": "0" },
          { "label": "Upper Left", "value": "1" },
          { "label": "Upper Right", "value": "2" },
          { "label": "Lower Left", "value": "3" },
          { "label": "Lower Right", "value": "4" }
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
      },
      {
        "type": "select",
        "messageKey": "WEATHER_FORECAST_FLICK_MODE",
        "label": "Forecast Trigger",
        "description": "How to open the weather forecast.",
        "defaultValue": "2",
        "options": [
          { "label": "Disabled", "value": "0" },
          { "label": "Single Flick", "value": "1" },
          { "label": "Double Flick", "value": "2" }
        ]
      },
      {
        "type": "select",
        "messageKey": "WEATHER_FORECAST_DURATION",
        "label": "Forecast Duration",
        "description": "How long to show the weather forecast.",
        "defaultValue": "0",
        "options": [
          { "label": "5 seconds", "value": "0" },
          { "label": "10 seconds", "value": "1" },
          { "label": "Until dismissed", "value": "2" }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  },
];