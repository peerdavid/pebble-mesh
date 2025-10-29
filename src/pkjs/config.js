module.exports = [
  {
    "type": "heading",
    "defaultValue": "Pebble Mesh Weather Settings"
  },
  {
    "type": "text",
    "defaultValue": "Configure your weather location below. Weather data is fetched from Open-Meteo API."
  },
  {
    "type": "input",
    "messageKey": "WEATHER_LOCATION_CONFIG",
    "label": "Location",
    "defaultValue": "Vienna",
    "description": "Enter your city name (e.g., Vienna, New York, Tokyo)"
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];