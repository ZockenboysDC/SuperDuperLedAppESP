#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Base_Class.h>
#include <Effects/Rainbow.cpp>
#include <Effects/OFF.cpp>
#include <Effects/StaticPalet.cpp>
#include <Effects/visualizer.cpp>
#include <Debug.h>
#include <Configuration.h>

// Config
#define DATA_PIN1 6
#define DATA_PIN2 5
#define NUM_LEDS 30
#define LED_TYPE WS2812B

// TODO: ADD OTA

// Server
const char *getEffect PROGMEM = "/get";
const char *setEffect PROGMEM = "/set";
const char *getSetting PROGMEM = "/getSetting";
const char *setSetting PROGMEM = "/setSetting";
const char *test PROGMEM = "/test";
const char *info2 PROGMEM = "/info";
ESP8266WebServer server(80);

// Data buffer
const size_t dataSize = 800;
char data[dataSize];
size_t dataLength = 0;

// Strip
#include <FastLED.h>
CRGB leds[NUM_LEDS];
Base_Class *rainboweffect = new RainbowEffect("Rainbow");
Base_Class *staticpalet = new StaticPalet("StaticPalet");
Base_Class *off = new Off("off");
Base_Class *visualiz = new visualizer("Visualizer");

enum EFFECTSENUM
{
  RAINBOW,
  STATICPALET,
  OFF,
  VISUALIZER
};

EFFECTSENUM currentEffect = VISUALIZER;

void getSettingfunction()
{
  // {
  //   "Effect": "Rainbow"
  // }
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  String post_body = server.arg("plain");
  DEBUG_PRINTLN(post_body);

  DeserializationError error = deserializeJson(doc, post_body);
  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  DEBUG_PRINTLN(F("Response:"));
  DEBUG_PRINTLN(doc["Effect"].as<char *>());

  DEBUG_PRINTLN(F("HTTP Method: "));
  DEBUG_PRINTLN(server.method());

  String effs = doc["Effect"].as<char *>();

  // create an object
  JsonObject object = doc.to<JsonObject>();
  String response;
  if (effs == "Rainbow")
  {
    object["Effect"] = effs;
    rainboweffect->serialize(object);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
    DEBUG_PRINTLN(doc.as<String>());
  }
  else if (effs == "StaticPalet")
  {
    object["Effect"] = effs;
    staticpalet->serialize(object);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
    DEBUG_PRINTLN(doc.as<String>());
  }
  else if (effs == "OFF")
  {
    object["Effect"] = effs;
    off->serialize(object);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
    DEBUG_PRINTLN(doc.as<String>());
  }
  else if (effs == "Visualizer")
  {
    object["Effect"] = effs;
    visualiz->serialize(object);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
    DEBUG_PRINTLN(doc.as<String>());
  }
  server.send(200, "application/json", response);
}

void handleInfoGet()
{
  const size_t bufferSize = JSON_OBJECT_SIZE(11) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(14) + JSON_ARRAY_SIZE(2);
  StaticJsonDocument<bufferSize> abra;

  // create JSON
  JsonObject root = abra.to<JsonObject>();
  root["id"] = ESP.getChipId();
  root["free_heap"] = ESP.getFreeHeap();
  root["sdk_version"] = ESP.getSdkVersion();
  root["boot_version"] = ESP.getBootVersion();
  root["boot_mode"] = ESP.getBootMode();
  root["vcc"] = ESP.getVcc() / 1024.00;
  root["cpu_freq"] = ESP.getCpuFreqMHz();
  root["sketch_size"] = ESP.getSketchSize();
  root["sketch_free_space"] = ESP.getFreeSketchSpace();

  JsonObject flash_chip = root.createNestedObject("flash_chip");
  flash_chip["id"] = ESP.getFlashChipId();
  flash_chip["size"] = ESP.getFlashChipSize();
  flash_chip["real_size"] = ESP.getFlashChipRealSize();
  flash_chip["speed"] = ESP.getFlashChipSpeed();
  FlashMode_t flashChipMode = ESP.getFlashChipMode();
  if (flashChipMode == FM_QIO)
    flash_chip["mode"] = "qio";
  else if (flashChipMode == FM_QOUT)
    flash_chip["mode"] = "qout";
  else if (flashChipMode == FM_DIO)
    flash_chip["mode"] = "dio";
  else if (flashChipMode == FM_DOUT)
    flash_chip["mode"] = "dout";
  else if (flashChipMode == FM_UNKNOWN)
    flash_chip["mode"] = "unknown";

  JsonObject wifi = root.createNestedObject("wifi");
  wifi["mac"] = WiFi.macAddress();
  wifi["ssid"] = WiFi.SSID();
  wifi["bssid"] = WiFi.BSSIDstr();
  wifi["rssi"] = WiFi.RSSI();
  wifi["channel"] = WiFi.channel();
  WiFiMode_t wifiMode = WiFi.getMode();
  if (wifiMode == WIFI_OFF)
    wifi["mode"] = "off";
  else if (wifiMode == WIFI_STA)
    wifi["mode"] = "sta";
  else if (wifiMode == WIFI_AP)
    wifi["mode"] = "ap";
  else if (wifiMode == WIFI_AP_STA)
    wifi["mode"] = "ap_sta";
  WiFiPhyMode_t wifiPhyMode = WiFi.getPhyMode();
  if (wifiPhyMode == WIFI_PHY_MODE_11B)
    wifi["phy_mode"] = "11b";
  else if (wifiPhyMode == WIFI_PHY_MODE_11G)
    wifi["phy_mode"] = "11g";
  else if (wifiPhyMode == WIFI_PHY_MODE_11N)
    wifi["phy_mode"] = "11n";
  WiFiSleepType_t wifiSleepMode = WiFi.getSleepMode();
  if (wifiSleepMode == WIFI_NONE_SLEEP)
    wifi["sleep_mode"] = "none";
  else if (wifiSleepMode == WIFI_LIGHT_SLEEP)
    wifi["sleep_mode"] = "light";
  else if (wifiSleepMode == WIFI_MODEM_SLEEP)
    wifi["sleep_mode"] = "modem";
  wifi["persistent"] = WiFi.getPersistent();
  wifi["ip"] = WiFi.localIP().toString();
  wifi["hostname"] = WiFi.hostname();
  wifi["subnet_mask"] = WiFi.subnetMask().toString();
  wifi["gateway_ip"] = WiFi.gatewayIP().toString();
  JsonArray dns = wifi.createNestedArray("dns");
  dns.add(WiFi.dnsIP(0).toString());
  dns.add(WiFi.dnsIP(1).toString());

  // send response
  String response;
  response += abra.as<String>();
  server.send(200, "application/json", response);
}

void setSettingfunction()
{
  // {
  //   "Effect": "Rainbow",
  //   "hue": 2,
  //   "rate": 1
  // }
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  String post_body = server.arg("plain");
  DEBUG_PRINTLN(post_body);

  DeserializationError error = deserializeJson(doc, post_body);
  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  DEBUG_PRINTLN(F("Response:"));
  DEBUG_PRINTLN(doc["Effect"].as<char *>());

  DEBUG_PRINTLN(F("HTTP Method: "));
  DEBUG_PRINTLN(server.method());

  String effs = doc["Effect"].as<char *>();

  // create an object
  String response;
  if (effs == "Rainbow")
  {
    rainboweffect->deserialize(doc);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
  }
  else if (effs == "StaticPalet")
  {
    staticpalet->deserialize(doc);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
  }
  else if (effs == "OFF")
  {
    off->deserialize(doc);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
  }
  else if (effs == "Visualizer")
  {
    visualiz->deserialize(doc);
    response += doc.as<String>();
    DEBUG_PRINTLN(response);
  }
  server.send(200, "application/json", response);
}

void handleRoot()
{
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 LED Webserver for the App</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60);
  server.send(200, "text/html", temp);
}

void handleNotFound()
{
  String message = "False URL\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void getEffectfunction()
{
  server.send(200, "application/plain", String(currentEffect));
}

// Needs to be post
void setEffectFunction()
{
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  String post_body = server.arg("plain");
  DEBUG_PRINTLN(post_body);

  DeserializationError error = deserializeJson(doc, post_body);
  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  // {
  //   "Effect": "Rainbow"
  // }
  DEBUG_PRINTLN(F("Response:"));
  DEBUG_PRINTLN(doc["Effect"].as<char *>());

  DEBUG_PRINTLN(F("HTTP Method: "));
  DEBUG_PRINTLN(server.method());

  String effs = doc["Effect"].as<String>();

  if (effs == "Rainbow")
  {
    currentEffect = RAINBOW;
    rainboweffect->begin();
  }
  else if (effs == "StaticPalet")
  {
    currentEffect = STATICPALET;
    staticpalet->begin();
  }
  else if (effs == "OFF")
  {
    currentEffect = OFF;
    off->begin();
  }
  else if (effs == "Visualizer")
  {
    currentEffect = VISUALIZER;
    visualiz->begin();
  }

  server.send(200, "application/plain", F("Success"));
}

void testFunction()
{
  server.send(200, F("application/plain"), F("Worked"));
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // WIFI
  DEBUG_PRINTLN(F("Connecting to: "))
  DEBUG_PRINTLN(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    DEBUG_PRINT(F("."));
  }

  DEBUG_PRINTLN(F(""));
  DEBUG_PRINTLN(F("WiFi connected"));
  DEBUG_PRINTLN(F("IP address: "));
  DEBUG_PRINTLN(WiFi.localIP());

  server.on(F("/"), HTTP_GET, handleRoot);
  server.on(getEffect, HTTP_GET, getEffectfunction);
  server.on(setEffect, HTTP_POST, setEffectFunction);
  server.on(setSetting, HTTP_POST, setSettingfunction);
  server.on(getSetting, HTTP_POST, getSettingfunction);
  server.on(test, HTTP_GET, testFunction);
  server.on(info2, HTTP_GET, handleInfoGet);
  server.onNotFound(handleNotFound);
  server.begin();

  DEBUG_PRINTLN(F("Server started"));

  // LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN1, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, DATA_PIN2, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(250);
}

// Note: Add F() to Strings
void loop()
{

  // WIFI
  if (!WiFi.status() == WL_CONNECTED)
  {
    DEBUG_PRINTLN(F("NO WIFI"));
  }

  //Server
  server.handleClient();

  switch (currentEffect)
  {
  case RAINBOW:
    rainboweffect->loop(leds, NUM_LEDS);
    break;

  case STATICPALET:
    staticpalet->loop(leds, NUM_LEDS);
    break;

  case OFF:
    off->loop(leds, NUM_LEDS);
    break;

  case VISUALIZER:
    visualiz->loop(leds, NUM_LEDS);
    break;

  default:
    break;
  }

  FastLED.show();
  delay(10);
}