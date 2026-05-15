#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ssid = "Eero";
const char* password = "Pishotti";

const char* nflUrl = "http://site.api.espn.com/apis/site/v2/sports/baseball/mlb/scoreboard";

time_t parseDate(const char* dateStr) {

  struct tm timeStruct = {};

  strptime(dateStr, "%Y-%m-%dT%H:%MZ", &timeStruct);

  return mktime(&timeStruct);
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

}




void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(nflUrl);

    int httpCode = http.GET();

    if (httpCode == 200) {

      String payload = http.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);

      JsonArray events = doc["events"];

      for (JsonObject event : events) {
        String name = event["name"];
        String dateStr = event["date"];

        const char* date = dateStr.c_str();

        time_t eventTime = parseDate(date);
        time_t nowTime = time(nullptr);

        
        JsonObject competition = event["competition"][0];
        
        String homeTeam = competition["competitors"][0]["team"]["abbreviation"];
        String homeScore = competition["competitors"][0]["score"];
        
        String awayTeam = competition["competitors"][1]["team"]["abbreviation"];
        String awayScore = competition["competitors"][1]["score"];

        // if (nowTime < eventTime) {
        //   Serial.println(awayTeam + " @ " + homeTeam + " | " + )
        // }

        Serial.println(name + " | " + homeTeam + " " + homeScore + " - " + awayScore + " " + awayTeam);
        

      }
      


      
     

    } else {
      Serial.println("HTTP request failed");
    }
    http.end();
  }
  delay(3000);
}


