#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 16

#define CLK_PIN 11
#define DATA_PIN 13
#define CS_PIN 12

MD_Parola p = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

const size_t MAX_API_KEY_LEN = 40 + 1;
const char apiKey[MAX_API_KEY_LEN] = "";

const uint16_t MAX_STOCKS = 10;
const uint8_t MAX_STOCK_SYMBOL_LENGTH = 5;
const size_t MAX_STOCK_SYMBOLS_LEN =
  (MAX_STOCK_SYMBOL_LENGTH + 1) * MAX_STOCKS + 1;
const char stockSymbols[MAX_STOCK_SYMBOLS_LEN] = "";

const char* CONFIG_AP_NAME = "Ticker Config";
const uint16_t CONFIGURATION_TIMEOUT = 120;

void loadParams() {
  Serial.println("Loading ticker configuration into memory");
  Preferences prefs;
  prefs.begin("tickerConfig", true);
  prefs.getString("apiKey", (char*)apiKey, MAX_API_KEY_LEN);
  prefs.getString("stockSymbols", (char*)stockSymbols, MAX_STOCK_SYMBOLS_LEN);
  prefs.end();
}

void saveParams() {
  Serial.println("Saving ticker configuration to flash");
  Preferences prefs;
  prefs.begin("tickerConfig", false);
  prefs.putString("apiKey", apiKey);
  prefs.putString("stockSymbols", stockSymbols);
  prefs.end();
}

bool connectToWiFi() {
  bool startedConfigAP = false;
  bool displayedAboutStartedConfigAP = false;
  bool configAPTimedOut = false;
  bool shouldSaveParams = false;

  loadParams();

  WiFiManagerParameter customAPIKey("apiKey", "Finnhub API key", apiKey,
                                    MAX_API_KEY_LEN);

  const size_t MAX_CUSTOM_STOCK_SYMBOLS_LEN = 100;
  char customStockSymbolsLabel[MAX_CUSTOM_STOCK_SYMBOLS_LEN];
  snprintf(
    customStockSymbolsLabel, MAX_CUSTOM_STOCK_SYMBOLS_LEN,
    "Stock symbols (separate with commas - ex. \"AAPL,AMZN\" - max %d stocks)",
    MAX_STOCKS);
  WiFiManagerParameter customStockSymbols("stockSymbols",
                                          customStockSymbolsLabel, stockSymbols,
                                          MAX_STOCK_SYMBOLS_LEN);

  WiFiManager wm;
  wm.addParameter(&customAPIKey);
  wm.addParameter(&customStockSymbols);
  wm.setAPCallback(
    [&startedConfigAP](WiFiManager* wm) { startedConfigAP = true; });
  wm.setConfigPortalTimeoutCallback(
    [&configAPTimedOut]() { configAPTimedOut = true; });
  wm.setSaveConfigCallback([&shouldSaveParams]() { shouldSaveParams = true; });
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(CONFIGURATION_TIMEOUT);
  Serial.println("Attempting connection to WiFi");

  p.print("Connecting to WiFi...");
  delay(2000);

  //  wm.resetSettings();
  //  Serial.println("WiFi configuration deleted");

  if (!wm.autoConnect(CONFIG_AP_NAME)) {
    while (true) {
      if (wm.process()) {
        break;
      }
      if (startedConfigAP && !displayedAboutStartedConfigAP) {
        Serial.println("Failed to connect to WiFi, starting configuration AP.");
        Serial.print("Join the WiFi network \"");
        Serial.print(CONFIG_AP_NAME);
        Serial.println("\" and open http://192.168.4.1 to open the WiFi "
                       "credential configuration page.");
        displayedAboutStartedConfigAP = true;
        p.displayClear();
      }
      if (startedConfigAP && p.displayAnimate()) {
        const size_t MAX_MESSAGE_LEN = 100;
        char message[MAX_MESSAGE_LEN];
        snprintf(message, MAX_MESSAGE_LEN,
                 "Join the WiFi network \"%s\" and open http://192.168.4.1 to "
                 "configure the ticker.",
                 CONFIG_AP_NAME);
        p.displayText(message, PA_LEFT, 20, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      }
      if (configAPTimedOut) {
        Serial.println("Configuration AP timed out, exiting.");
        while (!p.displayAnimate()) {
          // Wait to complete animation
        }
        p.displayText("Configuration timed out, reset to try again.", PA_LEFT,
                      20, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        while (!p.displayAnimate()) {
          // Wait to complete animation
        }
        goto wifiConnectFailed;
      }
    }
  }

  Serial.println("Successfully connected to saved WiFi network!");
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print("Local IPv4 address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Getting parameters");
  strncpy((char*)apiKey, customAPIKey.getValue(), MAX_API_KEY_LEN);
  strncpy((char*)stockSymbols, customStockSymbols.getValue(),
          MAX_STOCK_SYMBOLS_LEN);
  Serial.println("Parameters: ");
  Serial.print("apiKey=");
  Serial.println(apiKey);
  Serial.print("stockSymbols=");
  Serial.println(stockSymbols);
  if (shouldSaveParams) {
    Serial.println("User configured parameters, saving");
    saveParams();
  }

  Serial.println("Successfully connected to WiFi!");
  return true;

wifiConnectFailed:
  Serial.println("Failed to connect to WiFi!");
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(48, OUTPUT);
  digitalWrite(48, LOW);

  p.begin();
  p.setIntensity(1);
  p.setSpeed(20);

  if (connectToWiFi()) {
    p.print("Connected to WiFi!");
    delay(2000);
    p.displayClear();
  } else {
    while (true) {
      if (p.displayAnimate()) {
        p.displayScroll("Failed to connect to WiFi! Reset to try again or hold "
                        "down user button to reset WiFi settings.",
                        PA_LEFT, PA_SCROLL_LEFT, 20);
      }
    }
  }

  while (true) {
    if (p.displayAnimate()) {
      p.displayScroll(
        "Hello, World! Here is a really long message to test scrolling text.",
        PA_LEFT, PA_SCROLL_LEFT, 20);
    }
  }
}

void loop() {
  // Using while loop in setup to mimic loop because actually using loop crashes
}
