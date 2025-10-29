// PebbleKit JS component for weather data

// Variables to store weather data
var weatherData = {
  temperature: '--',
  location: 'Loading...'
};

// Function to get current position
function getLocation() {
  console.log('Requesting location...');
  
  // Check if geolocation is available
  if (!navigator.geolocation) {
    console.log('Geolocation is not supported');
    weatherData.location = 'No GPS';
    weatherData.temperature = 'N/A';
    sendWeatherToPebble();
    return;
  }
  
  navigator.geolocation.getCurrentPosition(
    function(position) {
      console.log('Location found: ' + position.coords.latitude + ',' + position.coords.longitude);
      getWeatherData(position.coords.latitude, position.coords.longitude);
      getCityName(position.coords.latitude, position.coords.longitude);
    },
    function(error) {
      console.log('Location error code: ' + error.code + ', message: ' + error.message);
      
      // Try fallback location (example coordinates for testing)
      console.log('Using fallback location (New York)');
      var fallbackLat = 40.7128;
      var fallbackLon = -74.0060;
      
      getWeatherData(fallbackLat, fallbackLon);
      weatherData.location = 'New York';
      sendWeatherToPebble();
    },
    {
      timeout: 30000,
      maximumAge: 300000, // 5 minutes
      enableHighAccuracy: false
    }
  );
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
            weatherData.temperature = temp + '°';
            console.log('Temperature: ' + weatherData.temperature);
            sendWeatherToPebble();
          } else {
            console.log('Invalid weather response format - no current_weather');
            weatherData.temperature = 'N/A';
            sendWeatherToPebble();
          }
        } catch (e) {
          console.log('Weather JSON parse error: ' + e.message);
          weatherData.temperature = 'Error';
          sendWeatherToPebble();
        }
      } else {
        console.log('Weather request failed with status: ' + xhr.status + ', response: ' + xhr.responseText);
        weatherData.temperature = 'Error';
        sendWeatherToPebble();
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 15000;
  xhr.ontimeout = function() {
    console.log('Weather request timed out');
    weatherData.temperature = 'Timeout';
    sendWeatherToPebble();
  };
  xhr.onerror = function() {
    console.log('Weather request network error');
    weatherData.temperature = 'Net Error';
    sendWeatherToPebble();
  };
  xhr.send();
}

// Function to get city name using reverse geocoding
function getCityName(latitude, longitude) {
  // Use Open-Meteo geocoding API with coordinates
  var url = 'https://geocoding-api.open-meteo.com/v1/search?name=location&latitude=' + 
            latitude + '&longitude=' + longitude + '&count=1';
  
  console.log('Fetching location from: ' + url);
  
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('Location response: ' + xhr.responseText);
          
          if (response.results && response.results.length > 0) {
            var result = response.results[0];
            var cityName = result.name || result.admin1 || result.admin2 || 'Unknown';
            weatherData.location = cityName;
            console.log('Location: ' + weatherData.location);
          } else {
            weatherData.location = 'Unknown';
          }
          sendWeatherToPebble();
        } catch (e) {
          console.log('Location JSON parse error: ' + e.message);
          weatherData.location = 'Parse Error';
          sendWeatherToPebble();
        }
      } else {
        console.log('Location request failed with status: ' + xhr.status);
        // Try backup method with approximate location
        weatherData.location = 'Lat:' + Math.round(latitude * 100) / 100;
        sendWeatherToPebble();
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 10000;
  xhr.ontimeout = function() {
    console.log('Location request timed out');
    weatherData.location = 'Timeout';
    sendWeatherToPebble();
  };
  xhr.onerror = function() {
    console.log('Location request network error');
    weatherData.location = 'Net Error';
    sendWeatherToPebble();
  };
  xhr.send();
}

// Function to send weather data to Pebble
function sendWeatherToPebble() {
  console.log('Sending to Pebble - Temperature: ' + weatherData.temperature + ', Location: ' + weatherData.location);
  
  var message = {
    'WEATHER_TEMPERATURE': weatherData.temperature,
    'WEATHER_LOCATION': weatherData.location
  };
  
  Pebble.sendAppMessage(message,
    function(e) {
      console.log('Weather data sent successfully');
    },
    function(e) {
      console.log('Failed to send weather data: ' + e.error.message);
    }
  );
}

// Event listeners
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  console.log('Starting initial weather fetch...');
  
  // Small delay to ensure everything is ready
  setTimeout(function() {
    getLocation();
  }, 1000);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received: ' + JSON.stringify(e.payload));
  
  // Check if it's a weather update request
  if (e.payload.WEATHER_REQUEST) {
    console.log('Weather update requested from watch');
    getLocation();
  }
});

// Update weather every 30 minutes
setInterval(function() {
  console.log('Periodic weather update (30min timer)');
  getLocation();
}, 30 * 60 * 1000);