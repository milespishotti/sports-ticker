#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <PxMatrix.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// #define P_LAT 22
// #define P_A 19
// #define P_B 23
// #define P_C 18
// #define P_D 5
// #define P_E 15
// #define P_OE 16

#define PANEL_WODTH 64
#define PANEL_HEIGHT 32
#define PANEL_COUNT 3

#define BUTTON_PIN 0
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 300;

// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

MatrixPanel_I2S_DMA *matrix = nullptr;

const char* ssid = "Eero";
const char* password = "Pishotti";


// hw_timer_t *timer = NULL;
// portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;


// void IRAM_ATTR display_updater() {
//   portENTER_CRITICAL_ISR(&timerMUX);
//   display.display(70);
//   portEXIT_CRITICAL_ISR(&timerMUX);
// }


const char* sportsUrls[] = {
  "http://site.api.espn.com/apis/site/v2/sports/baseball/mlb/scoreboard",
  "http://site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard",
  "http://site.api.espn.com/apis/site/v2/sports/basketball/nba/scoreboard?dates=20250201",
  "http://site.api.espn.com/apis/site/v2/sports/hockey/nhl/scoreboard",
  "http://site.api.espn.com/apis/site/v2/sports/football/college-football/scoreboard",
  "http://site.api.espn.com/apis/site/v2/sports/basketball/mens-college-basketball/scoreboard"
};

const char* sportNames[] {
  "MLB", "NFL", "NBA", "NHL", "NCAAF", "NCAAB"
};

const int NUM_SPORTS = 6;
int currentSport = 0;

enum Mode {
  ALL_SPORTS,
  MLB_ONLY,
  NFL_ONLY,
  NBA_ONLY,
  NHL_ONLY,
  NCAAF_ONLY,
  NCAAB_ONLY,

};

Mode currentMode = ALL_SPORTS;


struct Game {
  char awayTeam[5];
  char homeTeam[5];
  char awayScore[4];
  char homeScore[4];
  char state[4];
  char sport[7];
  int period;
  char displayTime[9];
};

const int MAX_GAMES = 20;
Game games[MAX_GAMES];
int gameCount = 0;
int currentGame = 0;

unsigned long lastFetch = 0;
unsigned long lastSwitch = 0;
const unsigned long FETCH_INTERVAL = 300000;
const unsigned long DISPLAY_INTERVAL = 4000;


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

void formatTime(time_t rawTime, char* buf, size_t bufSize) {
  struct tm* timeInfo = localtime(&rawTime);
  int hour = timeInfo->tm_hour;
  int minute = timeInfo->tm_min;
  if (hour == 0) hour = 12;
  else if (hour > 12) hour = hour -12;
  snprintf(buf, bufSize, "%d:%02d", hour, minute);
}




String padTeam(String team) {
if (team == "SF" || team == "SD" || team == "TB" || team == "KC") {
  return team + " ";
} else if (team == "UTAH") {
  return "UTA";
} else {
return team;
} }

String padScoreAway(const char* score) {
  int numScore = atoi(score);

  if (numScore < 100 && numScore > 9) {
    return " " + String(score);

  } else if (numScore < 10) {
    return " " + String(score) + " ";

  } else {
    return String(score);
  }
}

String padScoreHome(const char* score) {
  int numScore = atoi(score);

  if (numScore < 100 && numScore > 9) {
      return String(score) + " ";

  } else if (numScore < 10) {
    return " " + String(score) +  " ";

  } else {
    return String(score);
  }
  }
  


void fetchGames(const char* url, const char* sport) {
  if (WiFi.status() != WL_CONNECTED) return;

  for (int i = 0; i < MAX_GAMES; i++) {
    games[i].awayTeam[0] = '\0';
    games[i].homeTeam[0] = '\0';
    games[i].awayScore[0] = '\0';
    games[i].homeScore[0] = '\0';
    games[i].state[0] = '\0';
    games[i].displayTime[0] = '\0';
    games[i].sport[0] = '\0';
  }
  gameCount = 0;

  HTTPClient http;
  http.useHTTP10(true);
  http.begin(url);
  http.setTimeout(30000);
  http.setConnectTimeout(10000);

  Serial.print("\nFree heap: ");
  Serial.println(ESP.getFreeHeap());

  int httpCode = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode != 200) {
    Serial.println("HTTP Request Failed");
    http.end();
    return;
  }
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
  DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();

  if (error) {
    Serial.print("JSON failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray events = doc["events"];
  gameCount = 0;

  for (JsonObject event : events) {
    if (gameCount >= MAX_GAMES) break;

    char dateStr[25];
    strlcpy(dateStr, event["date"] | "", 25);
    time_t eventTime = parseDate(dateStr);

    time_t nowTime = time(nullptr);
    struct tm* eventTm = localtime(&eventTime);
    struct tm eventTmCopy = *eventTm;
    struct tm* nowTm = localtime(&nowTime);

    // if (eventTmCopy.tm_mday != nowTm ->tm_mday ||
    //   eventTmCopy.tm_mon != nowTm->tm_mon) {
    //     continue;
    //   }


    JsonObject competition = event["competitions"][0];

    strlcpy(games[gameCount].homeTeam, padTeam(competition["competitors"][0]["team"]["abbreviation"] | "???").c_str(), 5);

    strlcpy(games[gameCount].homeScore, padScoreHome((competition["competitors"][0]["score"] | "0")).c_str(), 4);
    // snprintf(games[gameCount].homeScore, sizeof(games[gameCount].homeScore), "%3d", atoi(competition["competitors"][0]["score"] | "0"), 4);


    strlcpy(games[gameCount].awayTeam,  padTeam(competition["competitors"][1]["team"]["abbreviation"] | "???").c_str(), 5);


    strlcpy(games[gameCount].awayScore, padScoreAway((competition["competitors"][1]["score"] | "0")).c_str(), 4);
    // snprintf(games[gameCount].awayScore, sizeof(games[gameCount].homeScore), "%3d", atoi(competition["competitors"][1]["score"] | "0"), 4);


    strlcpy(games[gameCount].state, (competition["status"]["type"]["state"] | "pre"), 4);
    games[gameCount].period = competition["status"]["period"] | 0;
    formatTime(eventTime, games[gameCount].displayTime, 9);
    strlcpy(games[gameCount].sport, sport, 7);

    gameCount++;
  

  }

  Serial.print("Games loaded: ");
  Serial.println(gameCount);

}

void displayGame(int index) {
  // ST7735 LED Screen Function
  // if (gameCount == 0) return ;
  
  // Game g = games[index];
  // String line;

  // if (g.state == "pre") {
  //   line = g.awayTeam + " @ " + g.homeTeam + " " + g.displayTime;
  // } else if (g.state == "in") {
  //   line = g.awayTeam + " " + g.awayScore + "-" + g.homeScore + " " + g.homeTeam + " INN" + g.period;
  // } else {
  //   line = g.awayTeam + " " + g.awayScore + "-" + g.homeScore + " " + g.homeTeam + " F";

  // }
  
  // line = " " + line;

  // tft.fillScreen(ST7735_BLACK);
  // tft.setTextSize(1);
  // tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  // tft.setCursor(0, 76);
  // tft.println(line);

  // Serial.println(line);


  // HUB75 Led Panels Function
  if (gameCount == 0) return;

  Game g = games[index];

  // display.clearDisplay();
  // display.setTextWrap(false);
  // display.setTextSize(1);

  matrix->clearScreen();
  matrix->setTextWrap(false);
  matrix->setTextSize(2);


  if (strcmp(g.state, "pre") == 0) {
    // display.setTextColor(display.color565(255, 255, 255));
    // display.setCursor(0, 0);
    // display.print(g.awayTeam + " @ " + g.homeTeam);
    // display.setTextColor(display.color565(150, 150, 150));
    // display.setCursor(0, 12);
    // display.print(g.displayTime);

    matrix->setTextColor(matrix->color565(255, 255, 255));
    matrix->setCursor(0,8);
    matrix->print(g.awayTeam);
    matrix->setCursor(44, 8);
    matrix->print("@");
    
    matrix->setCursor(64,8);
    matrix->print(g.homeTeam);
    matrix->print(" |");
  
    matrix->setTextColor(matrix->color565(150, 150, 150));
    matrix->setCursor(128, 8);
    matrix->print(g.displayTime);
  

  } else if (strcmp(g.state, "in") == 0) {
    // display.setTextColor(display.color565(0, 255, 0));
    // display.setCursor(0,0);
    // display.print(g.awayTeam + " " + g.awayScore);
    // display.setCursor(0, 12);
    // display.print(g.homeTeam + " " + g.homeScore);
    // display.setCursor(0, 24);
    // display.setTextColor(display.color565(255, 165, 0));
    // display.print("INN " + String(g.period));

    matrix->setTextColor(matrix->color565(255, 255, 255));
    matrix->setCursor(0, 8);
    matrix->print(g.awayTeam);
    matrix->setTextColor(matrix->color565(0, 255, 0));
    matrix->setCursor(38, 8);
    matrix->print(g.awayScore);
    matrix->setTextColor(matrix->color565(255, 255, 255));
    matrix->setCursor(68,8);
    matrix->print("-");

    matrix->setTextColor(matrix->color565(0, 255, 0));
    matrix->setCursor(78, 8);
    matrix->print(g.homeScore);
    matrix->setTextColor(matrix->color565(255, 255, 255));
    matrix->setCursor(122, 8);
    matrix->print(g.homeTeam);

    matrix->setTextColor(matrix->color565(0, 255, 0));
    matrix->setCursor(158, 8);
    
    if (strcmp(g.sport, "MLB") == 0) {
      matrix->print("I:");
    } else if (strcmp(g.sport, "NHL") == 0) {
      matrix->print("P:");
    } else if (strcmp(g.sport, "NCAAB") == 0) {
      matrix->print("H:");
    } else if (strcmp(g.sport, "NCAAF") == 0) {
      matrix->print("H:");
    } else {
      matrix->print("Q:");
    }
    
    matrix->print(g.period);

    


  } else {
    // display.setTextColor(display.color565(200, 200, 200));
    // display.setCursor(0, 0);
    // display.print(g.awayTeam + " " + g.awayScore);
    // display.setCursor(0, 12);
    // display.print(g.homeTeam + " " + g.homeScore);
    // display.setCursor(0, 24);
    // display.setTextColor(display.color565(100, 100, 100));
    // display.print("FINAL");

    matrix->setTextColor(matrix->color565(200, 200, 200));
    matrix->setCursor(0, 8);
    matrix->print(g.awayTeam);

    matrix->setCursor(40, 8);
    matrix->setTextColor(matrix->color565(255, 165, 0));
    matrix->print(g.awayScore);

    matrix->setCursor(79, 8);
    matrix->setTextColor(matrix->color565(200, 200, 200));
    matrix->print("-");

    matrix->setCursor(92, 8);
    matrix->setTextColor(matrix->color565(255, 165, 0));
    matrix->print(g.homeScore);

    matrix->setCursor(132, 8);
    matrix->setTextColor(matrix->color565(255, 255, 255));
    matrix->print(g.homeTeam);

    matrix->setTextColor(matrix->color565(255, 165, 0));
    matrix->setCursor(174, 8);
    matrix->print("F");
  }
  matrix->setTextSize(1);
  matrix->setCursor(170, 24);
  matrix->setTextColor(matrix->color565(255, 255, 255));
  matrix->print(g.sport);
  matrix->setTextSize(2);

}



void advanceSport() {
  int attempts = 0;
  do {
    currentSport = (currentSport + 1) % NUM_SPORTS;
    fetchGames(sportsUrls[currentSport], sportNames[currentSport]);
    attempts++;
  } while (gameCount ==  0 && attempts < NUM_SPORTS);
}

void handleButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress > DEBOUNCE_DELAY) {
      currentMode = (Mode)((currentMode + 1) % (NCAAB_ONLY + 1));
      currentGame = 0;
      currentSport = currentMode == ALL_SPORTS ? 0 : currentMode - 1;
      fetchGames(sportsUrls[currentSport], sportNames[currentSport]);
      displayGame(currentGame);
      lastButtonPress = now;
      Serial.print("Mode changed to: ");
      Serial.println(currentMode);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // ST7735 Init
  // tft.initR(INITR_BLACKTAB);
  // tft.fillScreen(ST77XX_BLACK);
  // tft.setTextColor(ST77XX_WHITE);
  // tft.setTextSize(1);
  // tft.setCursor(0,0);
  // tft.println("Connecting...");

  // HUB75 LED Init

  

 

  // display.clearDisplay();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  // display.begin(16);
  // display.setPanelsWidth(1);
  // display.fillScreen(display.color565(255, 0, 0));
  // display.flushDisplay();
  // display.setBrightness(50);

  // timer = timerBegin(0, 80, true);
  // timerAttachInterrupt(timer, &display_updater, true);
  // timerAlarmWrite(timer, 2000, true);
  // timerAlarmEnable(timer);
  
  // display.clearDisplay();
  // display.fillScreen(display.color565(0, 0,0));

  Serial.println("\nConnected!");

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_HEIGHT;
  mxconfig.chain_length = PANEL_COUNT;
  mxconfig.gpio.e = 32;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = false;

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->setBrightness8(128);
  if (!matrix->begin()) {
    Serial.println("Matrix init failed");
  }
  matrix->clearScreen();
  matrix->setTextWrap(false);


  Serial.print("ArduinoJson Version: ");
  Serial.println(ARDUINOJSON_VERSION);
  configTime(0, 0,  "pool.ntp.org");
  
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

  fetchGames(sportsUrls[currentSport], sportNames[currentSport]);
  displayGame(currentGame);

  lastFetch = millis();
  lastSwitch = millis();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

}




void loop() {

  if (ESP.getFreeHeap() < 100000) {
    Serial.println("Low memory, restarting...");
    ESP.restart();
  }
  unsigned long now = millis();
  handleButton();

  if (now - lastSwitch >= DISPLAY_INTERVAL) {
    currentGame++;

    if (currentGame >= gameCount) {
      currentGame = 0;

      if (currentMode == ALL_SPORTS) {
        advanceSport();
      } else {
        fetchGames(sportsUrls[currentMode - 1], sportNames[currentMode - 1]);
      }
    }
  
    displayGame(currentGame);
    lastSwitch = now;
  }

  if (now - lastFetch >= FETCH_INTERVAL) {
    fetchGames(sportsUrls[currentSport], sportNames[currentSport]);
    lastFetch = now;
  }
  // Serial.println("loop running");
  // delay(1000);

}
      
      

      
     



