#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>


#define TFT_CS  5
#define TFT_DC  2
#define TFT_RST 4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


const char* ssid = "Eero";
const char* password = "Pishotti";

const char* mlbUrl = "http://site.api.espn.com/apis/site/v2/sports/baseball/mlb/scoreboard";

void fetchGames(const char* url) {
  
}


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

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  tft.println("Connecting...");


  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  Serial.println("\nConnected!");
  Serial.print("ArduinoJson Version: ");
  Serial.println(ARDUINOJSON_VERSION);
  configTime(0, 0, "pool.ntp.org");
  
  setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
  tzset();
  Serial.print("Waiting for NTP sync");
  time_t now = time(nullptr);
  while (now < 1000000000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.print("\nNTP synced!");
  tft.fillScreen(ST7735_BLACK);


}




void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(mlbUrl);
    http.setTimeout(20000);

    Serial.print("\nFree heap: ");
    Serial.println(ESP.getFreeHeap());

    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    Serial.print("HTTP reported size: ");
    Serial.println(http.getSize());

    tft.fillScreen(ST7735_BLACK);
    

    if (httpCode == 200) {

      // String payload = http.getString();

      // Serial.print("Payload length received: ");
      // Serial.println(payload.length());

      // Serial.println("First 200 chars: ");
      // Serial.println(payload.substring(0, 200));

      // Serial.println("Last 200 chars:");
      // Serial.println(payload.substring(payload.length() -200));


      JsonDocument filter;

      filter["events"][0]["name"] = true;
      filter["events"][0]["date"] = true;

      filter["events"][0]["competitions"][0]["competitors"][0]["homeAway"] = true;
      filter["events"][0]["competitions"][0]["competitors"][0]["score"] = true;
      filter["events"][0]["competitions"][0]["competitors"][0]["team"]["abbreviation"] = true;
      filter["events"][0]["competitions"][0]["competitors"][1]["homeAway"] = true;
      filter["events"][0]["competitions"][0]["competitors"][1]["score"] = true;
      filter["events"][0]["competitions"][0]["competitors"][1]["team"]["abbreviation"] = true;
      filter["events"][0]["competitions"][0]["status"]["period"] = true;
      filter["events"][0]["competitions"][0]["status"]["type"]["state"] = true;


      JsonDocument doc;
      // deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

      DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

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

      tft.setTextSize(1);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

      int y = 10;


      for (JsonObject event : events) {

        Serial.println("-----");

        // Serial.print("Name: ");
        // Serial.println(event["name"].as<String>());

        // Serial.print("Date: ");
        // Serial.println(event["date"].as<String>());

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
        
        String state = competition["status"]["type"]["state"] | "pre";
        int period = competition["status"]["period"] | 0;

        if (homeTeam == "SF" || homeTeam == "SD" || homeTeam == "TB" || homeTeam == "KC") {
          homeTeam = homeTeam + " ";
        }

        if (awayTeam == "SF" || awayTeam == "SD" || awayTeam == "TB" || awayTeam == "KC") {
          awayTeam = awayTeam + " ";
        }

        String line;




        if (state == "pre") {
          // Serial.println(awayTeam + " @ " + homeTeam + " | " + readableTime);
          line = awayTeam + " @ " + homeTeam + " | " + readableTime;
        } else if (state == "in") {
        //   Serial.println(awayTeam + " " + awayScore + " @ " + homeScore + " " + homeTeam + " | " + "Inning " + period);
          line = awayTeam + " " + awayScore + "-" + homeScore + " " + homeTeam + " INN" + period;
        } else {
        //   Serial.println(awayTeam + " " + awayScore + " @ " + homeScore + " " + homeTeam + " | " + "Final");
        line = awayTeam + " " + awayScore + "-" + homeScore + " " + homeTeam + " F";
        }

        line = "  " + line;

        tft.setCursor(0, y);
        tft.println(line);
        y += 8; // each text size 1 line is 4 pixels tall

        if (y > 160) break;


        
        

      }
      
      

      
     

    } else {
      Serial.println("HTTP request failed");
    }
    http.end();
  }
  delay(60000);
}


