#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ssid = "Eero";
const char* password = "Pishotti";

const char* nflUrl = "http://site.api.espn.com/apis/site/v2/sports/baseball/mlb/scoreboard?dates=20260515&limit=8";

time_t parseDate(const char* dateStr) {

  struct tm timeStruct = {};

  strptime(dateStr, "%Y-%m-%dT%H:%MZ", &timeStruct);

  setenv("TZ", "UTC0", 1);
  tzset();

  time_t result = mktime(&timeStruct);

  setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
  tzset();

  return result;

}

String formatTime(time_t rawTime) {
  struct tm* timeInfo = localtime(&rawTime);

  int hour = timeInfo -> tm_hour;
  int minute = timeInfo -> tm_min;

  String ampm = "AM";

  if (hour >= 12) {
    ampm = "PM";
  }

  if (hour == 0) {
    hour = 12;
  }

  else if (hour > 12) {
    hour = hour - 12;

  }

  String minuteText;

  if (minute < 10) {
    minuteText = "0" + String(minute);
  }

  else {
    minuteText = String(minute);

  }

  return String(hour) + ":" + minuteText + " " + ampm;

}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  Serial.println("\nConnected!");
  configTime(0, 0, "pool.ntp.org");
  
  setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
  tzset();
  delay(2000);

}




void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(nflUrl);
    http.setTimeout(20000);

    Serial.print("Free heap: ");
    Serial.println(ESP.getFreeHeap());

    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    Serial.print("HTTP reported size: ");
    Serial.println(http.getSize());
    

    if (httpCode == 200) {

      // String payload = http.getString();

      // Serial.print("Payload length received: ");
      // Serial.println(payload.length());

      // Serial.println("First 200 chars: ");
      // Serial.println(payload.substring(0, 200));

      // Serial.println("Last 200 chars:");
      // Serial.println(payload.substring(payload.length() -200));


      JsonDocument filter;

      // filter["events"][0]["name"] = true;
      filter["events"][0]["date"] = true;

      filter["events"][0]["competitions"][0]["competitors"][0]["homeAway"] = true;
      filter["events"][0]["competitions"][0]["competitors"][0]["score"] = true;
      filter["events"][0]["competitions"][0]["competitors"][0]["team"]["abbreviation"] = true;


      JsonDocument doc;
      // deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

      DeserializationError error = deserializeJson(doc, http.getStream());

      if (error) {
        Serial.print("JSON failed");
        Serial.println(error.c_str());
        http.end();
        return;

        // Serial.print("HTTP request failed, code: ");
        // Serial.println(httpCode);

        // String errorBody = http.getString();
        // Serial.println(errorBody);
      }
      Serial.println("JSON parsed successfully");
      Serial.print("Events kept: ");
      Serial.println(doc["events"].size());


      JsonArray events = doc["events"];

      Serial.println("Number of events: ");
      Serial.println(events.size());

      for (JsonObject event : events) {

        Serial.println("-----");

        Serial.print("Name: ");
        Serial.println(event["name"].as<String>());

        Serial.print("Date: ");
        Serial.println(event["date"].as<String>());


        String name = event["name"];
        String dateStr = event["date"];

        const char* date = dateStr.c_str();

        time_t eventTime = parseDate(date);
        time_t nowTime = time(nullptr);

        String readableTime = formatTime(eventTime);


        
        JsonObject competition = event["competitions"][0];
        
        String homeTeam = competition["competitors"][0]["team"]["abbreviation"];
        String homeScore = competition["competitors"][0]["score"];
        
        String awayTeam = competition["competitors"][1]["team"]["abbreviation"];
        String awayScore = competition["competitors"][1]["score"];

        if (nowTime < eventTime) {
          Serial.println(awayTeam + " @ " + homeTeam + " | " + readableTime);
        }

        else {
          Serial.println(name + " | " + homeTeam + " " + homeScore + " - " + awayScore + " " + awayTeam);

        }

        
        

      }
      
      

      
     

    } else {
      Serial.println("HTTP request failed");
    }
    http.end();
  }
  delay(60000);
}


