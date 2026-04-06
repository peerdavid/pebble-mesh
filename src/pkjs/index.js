var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

// Default configuration
var config = {
  location: '', // Empty = use GPS, otherwise use static location
  colorTheme: 'dark',  // Default to dark theme (dark/light/dynamic)
  stepGoal: 10000, // Default step goal
  temperatureUnit: 'celsius', // Default temperature unit (celsius/fahrenheit)
  enableAnimations: false, // Default to animations enabled
  layoutUpperLeft: 0,   // INFO_TYPE_WEATHER
  layoutUpperRight: 1,  // INFO_TYPE_TEMPERATURE
  layoutLowerLeft: 2,   // INFO_TYPE_STEPS
  layoutLowerRight: 3,  // INFO_TYPE_BATTERY
  disconnectPosition: 0, // 0 = disabled, 1-4 = UL/UR/LL/LR
  notificationDuration: 0 // 0=5s, 1=10s, 2=forever, 3=disabled
};

// Load saved configuration
if (localStorage.getItem('WEATHER_LOCATION_CONFIG')) {
  config.location = localStorage.getItem('WEATHER_LOCATION_CONFIG');
}
if (localStorage.getItem('COLOR_THEME')) {
  config.colorTheme = localStorage.getItem('COLOR_THEME');
}
if (localStorage.getItem('STEP_GOAL')) {
  config.stepGoal = parseInt(localStorage.getItem('STEP_GOAL'));
}
if (localStorage.getItem('TEMPERATURE_UNIT')) {
  config.temperatureUnit = localStorage.getItem('TEMPERATURE_UNIT');
}
if (localStorage.getItem('ENABLE_ANIMATIONS')) {
  config.enableAnimations = (localStorage.getItem('ENABLE_ANIMATIONS') === 'true');
}
if (localStorage.getItem('LAYOUT_UPPER_LEFT') !== null) {
  config.layoutUpperLeft = parseInt(localStorage.getItem('LAYOUT_UPPER_LEFT'));
}
if (localStorage.getItem('LAYOUT_UPPER_RIGHT') !== null) {
  config.layoutUpperRight = parseInt(localStorage.getItem('LAYOUT_UPPER_RIGHT'));
}
if (localStorage.getItem('LAYOUT_LOWER_LEFT') !== null) {
  config.layoutLowerLeft = parseInt(localStorage.getItem('LAYOUT_LOWER_LEFT'));
}
if (localStorage.getItem('LAYOUT_LOWER_RIGHT') !== null) {
  config.layoutLowerRight = parseInt(localStorage.getItem('LAYOUT_LOWER_RIGHT'));
}
if (localStorage.getItem('DISCONNECT_POSITION') !== null) {
  config.disconnectPosition = parseInt(localStorage.getItem('DISCONNECT_POSITION'));
}
if (localStorage.getItem('NOTIFICATION_DURATION') !== null) {
  config.notificationDuration = parseInt(localStorage.getItem('NOTIFICATION_DURATION'));
}

// Variables to store weather data
var weatherData = {
  temperature: '--',
  location: 'Loading...',
  condition: -1,  // Weather code for condition
  is_day: true,   // Default to true (day)
  hourlyTemps: '',   // Comma-separated 12h hourly temperatures
  hourlyPrecip: '',  // Comma-separated 12h hourly precipitation probabilities
  forecast: [
    { temp: 0, condition: -1 },  // +3h
    { temp: 0, condition: -1 },  // +6h
    { temp: 0, condition: -1 }   // +9h
  ]
};

// Function to get coordinates for a city name
function getCoordinatesForCityAndFetchWeather(cityName) {
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
            weatherData.location = config.location + ' Not Found';
            weatherData.temperature = 'N/A';
            weatherData.condition = -1;
          }
        } catch (e) {
          console.log('Geocoding JSON parse error: ' + e.message);
          weatherData.location = 'Parse Error';
          weatherData.temperature = 'N/A';
          weatherData.condition = -1;
        }
      } else {
        console.log('Geocoding request failed with status: ' + xhr.status);
        weatherData.location = 'Geocode Error';
        weatherData.temperature = 'N/A';
        weatherData.condition = -1;
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 10000;
  xhr.ontimeout = function() {
    console.log('Geocoding request timed out');
    weatherData.location = 'Timeout';
    weatherData.temperature = 'N/A';
    weatherData.condition = -1;
  };
  xhr.onerror = function() {
    console.log('Geocoding request network error');
    weatherData.location = 'Net Error';
    weatherData.temperature = 'N/A';
    weatherData.condition = -1;
  };
  xhr.send();
}

// Main function to fetch weather for configured location
function fetchWeatherForLocation() {
  console.log('Fetching weather for configured location: ' + config.location);
  weatherData.location = 'Loading...';
  weatherData.temperature = '--';
  
  // If location is empty or not set, use GPS
  if (!config.location || config.location.trim() === '') {
    console.log('No static location configured, using GPS');
    getLocationAndFetchWeather();
  } else {
    console.log('Using static location: ' + config.location);
    getCoordinatesForCityAndFetchWeather(config.location);
  }
}

// Function to get current GPS location and fetch weather
function getLocationAndFetchWeather() {
  console.log('Getting GPS location...');
  weatherData.location = 'Getting GPS...';
  
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      var latitude = pos.coords.latitude;
      var longitude = pos.coords.longitude;
      console.log('GPS coordinates: ' + latitude + ',' + longitude);
      // Get city name from reverse geocoding and then get weather
      getReverseGeocodingAndFetchWeather(latitude, longitude);
    },
    function(err) {
      console.log('GPS location error: ' + err.message);
      weatherData.location = 'GPS Error';
      weatherData.temperature = 'N/A';
      weatherData.condition = -1;
    },
    {
      timeout: 15000,
      maximumAge: 60000
    }
  );
}

// Function to get city name from coordinates and fetch weather
function getReverseGeocodingAndFetchWeather(latitude, longitude) {
  console.log('Getting city name for coordinates: ' + latitude + ',' + longitude);
  
  // Use OpenStreetMap Nominatim for reverse geocoding
  var url = 'https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=' + latitude + '&longitude=' + longitude;

  console.log('Reverse geocoding URL: ' + url);
  
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('Reverse geocoding response: ' + xhr.responseText);
          
          if (response) {
            // Try to get city, town, village, or locality
            var locationName = response.city || 
                             response.locality || 
                             response.countryName ||
                             'GPS Loc';
            weatherData.location = locationName;
            console.log('Found location name: ' + weatherData.location);
          } else {
            console.log('No location name found, using GPS coordinates');
            weatherData.location = 'GPS Loc';
          }
        } catch (e) {
          console.log('Reverse geocoding JSON parse error: ' + e.message);
          weatherData.location = 'GPS Loc';
        }
        
        // Now get weather data for these coordinates
        getWeatherData(latitude, longitude);
      } else {
        console.log('Reverse geocoding request failed with status: ' + xhr.status);
        weatherData.location = 'GPS Loc';
        // Still get weather data even if reverse geocoding failed
        getWeatherData(latitude, longitude);
      }
    }
  };
  
  xhr.open('GET', url, true);
  xhr.timeout = 10000;
  xhr.ontimeout = function() {
    console.log('Reverse geocoding request timed out');
    weatherData.location = 'GPS Loc';
    // Still get weather data even if reverse geocoding timed out
    getWeatherData(latitude, longitude);
  };
  xhr.onerror = function() {
    console.log('Reverse geocoding request network error');
    weatherData.location = 'GPS Loc';
    // Still get weather data even if reverse geocoding failed
    getWeatherData(latitude, longitude);
  };
  xhr.send();
}

// Function to get weather data from Open-Meteo API
function getWeatherData(latitude, longitude) {
  // Request sunrise and sunset for today, along with current weather and hourly forecast
  var now = new Date();
  var yyyy = now.getUTCFullYear();
  var mm = String(now.getUTCMonth() + 1).padStart(2, '0');
  var dd = String(now.getUTCDate()).padStart(2, '0');
  var today = yyyy + '-' + mm + '-' + dd;
  // Also get tomorrow in case +9h crosses midnight
  var tomorrow = new Date(now);
  tomorrow.setDate(tomorrow.getDate() + 1);
  var yyyy2 = tomorrow.getUTCFullYear();
  var mm2 = String(tomorrow.getUTCMonth() + 1).padStart(2, '0');
  var dd2 = String(tomorrow.getUTCDate()).padStart(2, '0');
  var endDate = yyyy2 + '-' + mm2 + '-' + dd2;
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' +
            latitude + '&longitude=' + longitude +
            '&current_weather=true&temperature_unit=' + config.temperatureUnit + '&windspeed_unit=kmh' +
            '&hourly=temperature_2m,weather_code,precipitation_probability' +
            '&daily=sunrise,sunset&timezone=auto&start_date=' + today + '&end_date=' + endDate;

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

            // Default to is_day from API if available, else calculate
            var isDay = true; // Default to true
            // Fallback: calculate from sunrise/sunset
            try {
              var sunrise = null, sunset = null;
              if (response.daily && response.daily.sunrise && response.daily.sunset) {
                sunrise = new Date(response.daily.sunrise[0]);
                sunset = new Date(response.daily.sunset[0]);
                var nowLocal = new Date();
                isDay = (nowLocal >= sunrise && nowLocal < sunset);
              }
            } catch (e) {
              isDay = true; // Fallback to true if error
              console.log('Error calculating is_day: ' + e.message);
            }
            
            weatherData.is_day = isDay;

            // Extract hourly forecast for +3h, +6h, +9h
            try {
              if (response.hourly && response.hourly.time && response.hourly.temperature_2m && response.hourly.weather_code) {
                var nowLocal = new Date();
                var offsets = [3, 6, 9];
                for (var fi = 0; fi < offsets.length; fi++) {
                  var targetTime = new Date(nowLocal.getTime() + offsets[fi] * 3600 * 1000);
                  // Find the closest hourly slot
                  var bestIdx = 0;
                  var bestDiff = Infinity;
                  for (var hi = 0; hi < response.hourly.time.length; hi++) {
                    var hourTime = new Date(response.hourly.time[hi]);
                    var diff = Math.abs(hourTime.getTime() - targetTime.getTime());
                    if (diff < bestDiff) {
                      bestDiff = diff;
                      bestIdx = hi;
                    }
                  }
                  weatherData.forecast[fi].temp = Math.round(response.hourly.temperature_2m[bestIdx]);
                  weatherData.forecast[fi].condition = response.hourly.weather_code[bestIdx];
                }
                console.log('Forecast: +3h=' + weatherData.forecast[0].temp + '/' + weatherData.forecast[0].condition +
                            ' +6h=' + weatherData.forecast[1].temp + '/' + weatherData.forecast[1].condition +
                            ' +9h=' + weatherData.forecast[2].temp + '/' + weatherData.forecast[2].condition);

                // Extract 24 hours of today's data for the bottom bar graph (hours 0-23)
                var hourlyTemps = [];
                var hourlyPrecip = [];
                // Today's midnight
                var todayMidnight = new Date(nowLocal.getFullYear(), nowLocal.getMonth(), nowLocal.getDate(), 0, 0, 0);
                for (var h = 0; h < 24; h++) {
                  var hTarget = new Date(todayMidnight.getTime() + h * 3600 * 1000);
                  var hBestIdx = 0;
                  var hBestDiff = Infinity;
                  for (var hi2 = 0; hi2 < response.hourly.time.length; hi2++) {
                    var ht = new Date(response.hourly.time[hi2]);
                    var hd = Math.abs(ht.getTime() - hTarget.getTime());
                    if (hd < hBestDiff) {
                      hBestDiff = hd;
                      hBestIdx = hi2;
                    }
                  }
                  hourlyTemps.push(Math.round(response.hourly.temperature_2m[hBestIdx]));
                  var precip = (response.hourly.precipitation_probability && response.hourly.precipitation_probability[hBestIdx]) || 0;
                  hourlyPrecip.push(Math.round(precip));
                }
                weatherData.hourlyTemps = hourlyTemps.join(',');
                weatherData.hourlyPrecip = hourlyPrecip.join(',');
                console.log('Hourly temps: ' + weatherData.hourlyTemps);
                console.log('Hourly precip: ' + weatherData.hourlyPrecip);
              }
            } catch (fe) {
              console.log('Forecast parse error: ' + fe.message);
            }

            console.log('Temperature: ' + weatherData.temperature + ', Weather code: ' + weatherData.condition + ', is_day: ' + isDay);
            sendDataToPebble();
          } else {
            console.log('Invalid weather response format - no current_weather');
            weatherData.temperature = 'N/A';
            weatherData.condition = -1;
            weatherData.is_day = true;
          }
        } catch (e) {
          console.log('Weather JSON parse error: ' + e.message);
          weatherData.temperature = 'Error';
          weatherData.condition = -1;
          weatherData.is_day = true;
        }
      } else {
        console.log('Weather request failed with status: ' + xhr.status + ', response: ' + xhr.responseText);
        weatherData.temperature = 'Error';
        weatherData.condition = -1;
        weatherData.is_day = true;
      }
    }
  };

  xhr.open('GET', url, true);
  xhr.timeout = 15000;
  xhr.ontimeout = function() {
    console.log('Weather request timed out');
    weatherData.temperature = 'Timeout';
    weatherData.condition = -1;
    weatherData.is_day = true;
  };
  xhr.onerror = function() {
    console.log('Weather request network error');
    weatherData.temperature = 'Net Error';
    weatherData.condition = -1;
    weatherData.is_day = true;
  };
  xhr.send();
}


function colorThemeStrToInt(themeStr) {
  if (themeStr === 'light') {
    return 1;
  } else if (themeStr === 'dynamic') {
    return 2;
  } else if (themeStr === 'quiet') {
    return 3;
  } else {
    return 0; // Default to dark
  }
}


// Function to send weather data to Pebble
function sendDataToPebble() {
  console.log('Sending data to pebble.');

  var message = {
    'WEATHER_TEMPERATURE': weatherData.temperature,
    'WEATHER_LOCATION': weatherData.location,
    'WEATHER_CONDITION': weatherData.condition,
    'COLOR_THEME': colorThemeStrToInt(config.colorTheme),  // 0 = dark, 1 = light, 2 = dynamic
    'STEP_GOAL': config.stepGoal,
    'TEMPERATURE_UNIT': config.temperatureUnit === 'fahrenheit' ? 1 : 0,  // 0 = celsius, 1 = fahrenheit
    'WEATHER_IS_DAY': weatherData.is_day ? 1 : 0,
    'ENABLE_ANIMATIONS': config.enableAnimations ? 1 : 0,
    'LAYOUT_UPPER_LEFT': config.layoutUpperLeft,
    'LAYOUT_UPPER_RIGHT': config.layoutUpperRight,
    'LAYOUT_LOWER_LEFT': config.layoutLowerLeft,
    'LAYOUT_LOWER_RIGHT': config.layoutLowerRight,
    'DISCONNECT_POSITION': config.disconnectPosition,
    'FORECAST_TEMP_1': weatherData.forecast[0].temp,
    'FORECAST_TEMP_2': weatherData.forecast[1].temp,
    'FORECAST_TEMP_3': weatherData.forecast[2].temp,
    'FORECAST_CONDITION_1': weatherData.forecast[0].condition,
    'FORECAST_CONDITION_2': weatherData.forecast[1].condition,
    'FORECAST_CONDITION_3': weatherData.forecast[2].condition,
    'HOURLY_TEMPS': weatherData.hourlyTemps,
    'HOURLY_PRECIP': weatherData.hourlyPrecip,
    'NOTIFICATION_DURATION': config.notificationDuration
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

// Event listeners from app
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
  if (e && !e.response) {
    return;
  }
  
  // Get the keys and values from Clay
  // Get the keys and values from each config item
  var dict = clay.getSettings(e.response, false);
  console.log('Configuration received: ' + JSON.stringify(dict));
  var layoutChanged = false;

  // Update all config values first before any sendDataToPebble() calls,
  // since AppMessage can only handle one in-flight message at a time.
  if (dict.DISCONNECT_POSITION) {
    config.disconnectPosition = parseInt(dict.DISCONNECT_POSITION.value);
    localStorage.setItem('DISCONNECT_POSITION', config.disconnectPosition);
    console.log('Disconnect position saved to: ' + config.disconnectPosition);
  }

  if (dict.WEATHER_LOCATION_CONFIG !== undefined) {
    config.location = dict.WEATHER_LOCATION_CONFIG.value || ''; // Empty string for GPS
    localStorage.setItem('WEATHER_LOCATION_CONFIG', config.location);
    console.log('Location saved to: "' + config.location + '" (empty = GPS)');
    fetchWeatherForLocation();
  }
  
  if (dict.COLOR_THEME) {
    config.colorTheme = dict.COLOR_THEME.value;
    localStorage.setItem('COLOR_THEME', config.colorTheme);
    console.log('Color theme saved to: ' + config.colorTheme);
    layoutChanged = true;
  }
  
  if (dict.STEP_GOAL) {
    config.stepGoal = parseInt(dict.STEP_GOAL.value);
    localStorage.setItem('STEP_GOAL', config.stepGoal);
    console.log('Step goal saved to: ' + config.stepGoal);
    layoutChanged = true;
  }
  
  if (dict.TEMPERATURE_UNIT) {
    config.temperatureUnit = dict.TEMPERATURE_UNIT.value;
    localStorage.setItem('TEMPERATURE_UNIT', config.temperatureUnit);
    console.log('Temperature unit saved to: ' + config.temperatureUnit);
    fetchWeatherForLocation(); // Fetch weather again with new unit
  }

  if (dict.ENABLE_ANIMATIONS) {
    config.enableAnimations = dict.ENABLE_ANIMATIONS.value;
    localStorage.setItem('ENABLE_ANIMATIONS', config.enableAnimations);
    console.log('Enable animations saved to: ' + config.enableAnimations);
    layoutChanged = true;
  }

  if (dict.NOTIFICATION_DURATION) {
    config.notificationDuration = parseInt(dict.NOTIFICATION_DURATION.value);
    localStorage.setItem('NOTIFICATION_DURATION', config.notificationDuration);
    console.log('Notification duration saved to: ' + config.notificationDuration);
    layoutChanged = true;
  }

  if (dict.LAYOUT_UPPER_LEFT) {
    config.layoutUpperLeft = parseInt(dict.LAYOUT_UPPER_LEFT.value);
    localStorage.setItem('LAYOUT_UPPER_LEFT', config.layoutUpperLeft);
    layoutChanged = true;
  }
  if (dict.LAYOUT_UPPER_RIGHT) {
    config.layoutUpperRight = parseInt(dict.LAYOUT_UPPER_RIGHT.value);
    localStorage.setItem('LAYOUT_UPPER_RIGHT', config.layoutUpperRight);
    layoutChanged = true;
  }
  if (dict.LAYOUT_LOWER_LEFT) {
    config.layoutLowerLeft = parseInt(dict.LAYOUT_LOWER_LEFT.value);
    localStorage.setItem('LAYOUT_LOWER_LEFT', config.layoutLowerLeft);
    layoutChanged = true;
  }
  if (dict.LAYOUT_LOWER_RIGHT) {
    config.layoutLowerRight = parseInt(dict.LAYOUT_LOWER_RIGHT.value);
    localStorage.setItem('LAYOUT_LOWER_RIGHT', config.layoutLowerRight);
    layoutChanged = true;
  }
  if (layoutChanged) {
    console.log('Layout saved: ' + config.layoutUpperLeft + ',' + config.layoutUpperRight + ',' + config.layoutLowerLeft + ',' + config.layoutLowerRight);
    sendDataToPebble();
  }

});

// Update weather every 30 minutes
setInterval(function() {
  console.log('Periodic weather update (30min timer) for: ' + config.location);
  fetchWeatherForLocation();
}, 30 * 60 * 1000);


// Handle Pebble ready event
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  console.log('Initial weather fetch for location: "' + config.location + '" (empty = GPS)');
  //fetchWeatherForLocation();
});


// Handle configuration page request
Pebble.addEventListener('showConfiguration', function() {
  console.log('Showing configuration page');
  Pebble.openURL(clay.generateUrl());
});