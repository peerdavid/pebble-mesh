// Default configuration
var config = {
  location: 'Vienna', // Default location
  colorTheme: 'dark'  // Default to dark theme (dark/light)
};

// Load saved configuration
if (localStorage.getItem('WEATHER_LOCATION_CONFIG')) {
  config.location = localStorage.getItem('WEATHER_LOCATION_CONFIG');
}
if (localStorage.getItem('COLOR_THEME')) {
  config.colorTheme = localStorage.getItem('COLOR_THEME');
}

// Variables to store weather data
var weatherData = {
  temperature: '--',
  location: 'Loading...',
  condition: 0  // Weather code for condition
};

// Function to get coordinates for a city name
function getCoordinatesForCity(cityName) {
  console.log('Getting coordinates for: ' + cityName);
  
  var url = 'https://geocoding-api.open-meteo.com/v1/search?name=' + 
            encodeURIComponent(cityName) + '&count=1&language=en&format=json';
  
  console.log('Geocoding URL: ' + url);
  
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('Geocoding response: ' + xhr.responseText);
          
          if (response.results && response.results.length > 0) {
            var result = response.results[0];
            var latitude = result.latitude;
            var longitude = result.longitude;
            var locationName = result.name || cityName;
            
            console.log('Found coordinates: ' + latitude + ',' + longitude + ' for ' + locationName);
            weatherData.location = locationName;
            
            // Now get weather data for these coordinates
            getWeatherData(latitude, longitude);
          } else {
            console.log('No results found for city: ' + cityName);
            weatherData.location = 'City Not Found';
            weatherData.temperature = 'N/A';
            weatherData.condition = 0;
            sendDataToPebble();
          }
        } catch (e) {
          console.log('Geocoding JSON parse error: ' + e.message);
          weatherData.location = 'Parse Error';
          weatherData.temperature = 'N/A';
          weatherData.condition = 0;
          sendDataToPebble();
        }
      } else {
        console.log('Geocoding request failed with status: ' + xhr.status);
        weatherData.location = 'Geocode Error';
        weatherData.temperature = 'N/A';
        weatherData.condition = 0;
        sendDataToPebble();
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 10000;
  xhr.ontimeout = function() {
    console.log('Geocoding request timed out');
    weatherData.location = 'Timeout';
    weatherData.temperature = 'N/A';
    weatherData.condition = 0;
    sendDataToPebble();
  };
  xhr.onerror = function() {
    console.log('Geocoding request network error');
    weatherData.location = 'Net Error';
    weatherData.temperature = 'N/A';
    weatherData.condition = 0;
    sendDataToPebble();
  };
  xhr.send();
}

// Main function to fetch weather for configured location
function fetchWeatherForLocation() {
  console.log('Fetching weather for configured location: ' + config.location);
  weatherData.location = 'Loading...';
  weatherData.temperature = '--';
  getCoordinatesForCity(config.location);
}

// Function to get weather data from Open-Meteo API
function getWeatherData(latitude, longitude) {
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + 
            latitude + '&longitude=' + longitude + 
            '&current_weather=true&temperature_unit=celsius&windspeed_unit=kmh';
  
  console.log('Fetching weather from: ' + url);
  
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) {
      console.log('Weather request completed with status: ' + xhr.status);
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('Weather response: ' + xhr.responseText);
          
          if (response.current_weather && response.current_weather.temperature !== undefined) {
            var temp = Math.round(response.current_weather.temperature);
            weatherData.temperature = temp.toString();
            weatherData.condition = response.current_weather.weathercode || 0;
            console.log('Temperature: ' + weatherData.temperature + ', Weather code: ' + weatherData.condition);
            sendDataToPebble();
          } else {
            console.log('Invalid weather response format - no current_weather');
            weatherData.temperature = 'N/A';
            weatherData.condition = 0;
            sendDataToPebble();
          }
        } catch (e) {
          console.log('Weather JSON parse error: ' + e.message);
          weatherData.temperature = 'Error';
          weatherData.condition = 0;
          sendDataToPebble();
        }
      } else {
        console.log('Weather request failed with status: ' + xhr.status + ', response: ' + xhr.responseText);
        weatherData.temperature = 'Error';
        weatherData.condition = 0;
        sendDataToPebble();
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 15000;
  xhr.ontimeout = function() {
    console.log('Weather request timed out');
    weatherData.temperature = 'Timeout';
    weatherData.condition = 0;
    sendDataToPebble();
  };
  xhr.onerror = function() {
    console.log('Weather request network error');
    weatherData.temperature = 'Net Error';
    weatherData.condition = 0;
    sendDataToPebble();
  };
  xhr.send();
}

// Function to send weather data to Pebble
function sendDataToPebble() {
  console.log('Sending data to pebble.');
  
  var message = {
    'WEATHER_TEMPERATURE': weatherData.temperature,
    'WEATHER_LOCATION': weatherData.location,
    'WEATHER_CONDITION': weatherData.condition,
    'COLOR_THEME': config.colorTheme === 'light' ? 1 : 0  // 0 = dark, 1 = light
  };
  
  Pebble.sendAppMessage(message,
    function(e) {
      console.log('Data sent successfully');
    },
    function(e) {
      console.log('Failed to send data: ' + e.error.message);
    }
  );
}

// Event listeners
Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload));
  
  // Check if it's a weather update request
  if (e.payload.WEATHER_REQUEST) {
    console.log('Weather update requested from watch');
    fetchWeatherForLocation();
  }
  
  // Check if it's a location configuration update
  if (e.payload.WEATHER_LOCATION_CONFIG) {
    config.location = e.payload.WEATHER_LOCATION_CONFIG;
    console.log('Location configuration updated to: ' + config.location);
    fetchWeatherForLocation();
  }
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Configuration window closed');
  
  if (e.response) {
    try {
      var configData = JSON.parse(decodeURIComponent(e.response));
      console.log('Configuration received: ' + JSON.stringify(configData));
      
      if (configData.WEATHER_LOCATION_CONFIG) {
        config.location = configData.WEATHER_LOCATION_CONFIG;
        localStorage.setItem('WEATHER_LOCATION_CONFIG', config.location);
        console.log('Location saved to: ' + config.location);
        fetchWeatherForLocation();
      }
      
      if (configData.COLOR_THEME) {
        config.colorTheme = configData.COLOR_THEME;
        localStorage.setItem('COLOR_THEME', config.colorTheme);
        console.log('Color theme saved to: ' + config.colorTheme);
        sendDataToPebble(); // Send immediately to update colors
      }
    } catch (ex) {
      console.log('Configuration parse error: ' + ex.message);
    }
  }
});

// Update weather every 30 minutes
setInterval(function() {
  console.log('Periodic weather update (30min timer) for: ' + config.location);
  fetchWeatherForLocation();
}, 30 * 60 * 1000);


// Handle configuration page request
Pebble.addEventListener('showConfiguration', function() {
  console.log('Showing configuration page');
  
  // Create the configuration URL with current settings
  var currentSettings = encodeURIComponent(JSON.stringify({
    'WEATHER_LOCATION_CONFIG': config.location,
    'COLOR_THEME': config.colorTheme
  }));
  
  // Use the correct path for Pebble configuration pages
  var configUrl = 'https://peerdavid.github.io/pebble-mesh/?settings=' + currentSettings;
  console.log('Opening configuration URL: ' + configUrl);
  
  Pebble.openURL(configUrl);
});