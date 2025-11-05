module.exports = [
  {
    "type": "heading",
    "defaultValue": "Pebble Mesh Settings"
  },
  {
    "type": "text",
    "defaultValue": "Made by David Peer, 2025"
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
        "defaultValue": "Vienna",
        "description": "Enter your city name (e.g., Vienna, New York, Tokyo). Weather data is fetched from Open-Meteo API.",
        "attributes": {
          "placeholder": "e.g., Vienna"
        }
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Steps"
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
        "defaultValue": "Appearance"
      },
      {
        "type": "radiogroup",
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