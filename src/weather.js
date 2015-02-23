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
    var locationUrl = "http://nominatim.openstreetmap.org/reverse?format=json&lat=" + 
                        pos.coords.latitude + 
                        "&lon=" + pos.coords.longitude + "&zoom=18&addressdetails=1";

    console.log("locationUrl: " + locationUrl);
    
    xhrRequest(locationUrl, 'GET', 
        function(responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);
            console.log("Response: " + responseText);
            
            var locationString = "";
            
            if (json.address.hasOwnProperty('village')) {
                locationString += json.address.village;
            }
            
            if (json.address.hasOwnProperty('neighbourhood')) {
                locationString += json.address.neighbourhood + " ";
            }
            
            if (json.address.hasOwnProperty('town')) {
                locationString += json.address.town + " ";
            }
            
            if (json.address.hasOwnProperty('county')) {
                locationString += json.address.county;
            }
            
            if (json.address.hasOwnProperty('city')) {
                locationString = json.address.city;
            }
            
            if (json.address.hasOwnProperty('country')) {
                console.log("We have country info");
                locationString += " "+json.address.country;
            }
            
            console.log("Location string: " + locationString);
            
            var weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + encodeURIComponent(locationString);

            console.log("weatherUrl: " + weatherUrl);
            
            // Send request to OpenWeatherMap
            xhrRequest(weatherUrl, 'GET', 
                function(responseText) {
                    // responseText contains a JSON object with weather info
                    var json = JSON.parse(responseText);

                    console.log("Response: " + responseText);

                    // Temperature in Kelvin requires adjustment
                    var temperature = Math.round(json.main.temp - 273.15);
                    console.log("Temperature: " + temperature);

                    // Conditions
                    var conditions = json.weather[0].main;
                    console.log("Conditions: " + conditions);

                    // Location name
                    var locationName = json.name;
                    console.log("Location name: " + locationName);

                    var dictionary = {
                        "KEY_TEMPERATURE": temperature,
                        "KEY_CONDITIONS": conditions,
                        "KEY_LOCATION_NAME": locationName
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
