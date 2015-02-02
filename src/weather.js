Pebble.addEventListener('ready', 
    function(e) {
        getWeather();
        console.log("PebbleKit JS ready!");
    }
);

Pebble.addEventListener('appmessage',
    function(e) {
        getWeather();
        console.log("AppMessage received!");
    }                     
);

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
        xhr.onload = function () {
        callback(this.responseText);
    };
    
    xhr.open(type, url);
    xhr.send();
};

function locationSuccess(pos) {
    // We will request the weather here
    var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
                pos.coords.latitude + "&lon=" + pos.coords.longitude;

    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET', 
        function(responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);

            // Temperature in Kelvin requires adjustment
            var temperature = Math.round(json.main.temp - 273.15);
            console.log("Temperature: " + temperature);

            // Conditions
            var conditions = json.weather[0].main;
            console.log("Conditions: " + conditions);

            // Location name
            var location_name = json.name;
            console.log("Location name: " + location_name);
        
            var dictionary = {
                "KEY_TEMPERATURE": temperature,
                "KEY_CONDITIONS": conditions,
                "KEY_LOCATION_NAME": location_name
            };

            // Send to Pebble
            Pebble.sendAppMessage(dictionary,
                function(e) {
                    console.log("Weather info sent to Pebble successfully!");
                },
                function(e) {
                    console.log("Error sending weather info to Pebble!");
                });
        }
    );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError, {timeout: 15000, maximumAge: 60000}
    );
}
