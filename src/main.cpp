#include "Arduino.h"
#include "Debug.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "FastLED.h"
#include "WiFiUdp.h"

#include "effect.h"

#include "effects/comet.cpp"
#include "effects/beatwave.cpp"

// Config stuff
struct Config
{
    char ssid[64];
    char password[100];
    char name[64];
    int num_leds;
};

// makegreater
effect **effectarray = new effect *[2];
boolean bon = true;
int currentEffect = 0;

const char *filename = "config.json";
Config config;
String led(config.num_leds);
const char *type = "Led_1";

// TODO: ADD INFO SITE
// Server / UDP
ESP8266WebServer server(80);
WiFiUDP Udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];

// Led
#define DATA_PIN1 6
#define DATA_PIN2 5
#define MAX_NUM_LEDS 500 // TODO: Lower this if you need more DATA Space
#define LED_TYPE WS2812B
CRGB leds[MAX_NUM_LEDS];

void getAllFunctions();
void getsettingofFunction();
void set();
void seton();
void setoff();
void getbon();
void setWiFi();
void loadConfig(const char *filename, Config &config);
void printFile(const char *filename);
void saveConf(const char *filename, const Config &config);
void getSettings();
void resetSettings();
void setSettings();
void v404();
void getStatus();
void restart();
void seteffectsetSetting();

void setup()
{
    // AddtheFunction
    effectarray[0] = new comet("Comet");
    effectarray[1] = new beatwave("Beatwave");

#ifdef DEBUG
    Serial.begin(115200);
#endif

    // init SPIFF and load config
    SPIFFS.begin();
    printFile(filename);
    loadConfig(filename, config);

    // To allow for resets.
    WiFi.disconnect();

    // LEDs
    FastLED.addLeds<LED_TYPE, DATA_PIN1, GRB>(leds, MAX_NUM_LEDS);
    FastLED.addLeds<LED_TYPE, DATA_PIN2, GRB>(leds, MAX_NUM_LEDS);

    // init WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);

    static long last_check;
    static long last_check2;
    static boolean black;
    int c = 0;
    bool smartconfi = false;
    // WiFi check
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        if (millis() - last_check > 1500)
        {
            delay(0);
            for (int i = 0; i < config.num_leds; i++)
            {
                if (i % 2 == 0)
                    leds[i].setRGB(0, 255, 255);
            }
            FastLED.show();
            last_check = millis();
            ESP.wdtFeed();
            last_check2 = millis();
            black = true;
        }
        if (millis() - last_check2 > 500 && black)
        {
            fill_solid(leds, config.num_leds, CRGB::Black);
            FastLED.show();
            last_check2 = millis();
            black = false;
        }
        c++;
        // Falls nach 5 Sekunden kein WiFi => Smartconfig
        if (c >= 35)
        {
            smartconfi = true;
            break;
        }
    }

    if (smartconfi)
    {
        // Smartconfig beginnt
        WiFi.beginSmartConfig();

        // Warten bis Smartconfig erhalten wird
        DEBUG_PRINTLN(F("Warten auf Smartconfig"));
        for (int i = 0; i < config.num_leds; i++)
        {
            if (i % 2 == 0)
                leds[i].setRGB(0, 255, 255);
        }
        FastLED.show();
        while (!WiFi.smartConfigDone())
        {
            delay(500);
            DEBUG_PRINT(F("."));
        }

        // Falls fertig
        DEBUG_PRINTLN(F("SmartConfig erhalten."));

        // Warten auf WiFi
        DEBUG_PRINTLN(F("Warten auf WiFi"));
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            DEBUG_PRINT(F("."));
            for (int i = 0; i < config.num_leds; i++)
            {
                if (i % 2 == 0)
                    leds[i].setRGB(0, 255, 255);
            }
            FastLED.show();
            fadeToBlackBy(leds, config.num_leds, 1);
        }
    }

    for (int i = 0; i < config.num_leds; i++)
    {
        if (i % 2 == 0)
            leds[i].setRGB(0, 255, 0);
    }
    FastLED.show();

    DEBUG_PRINTLN(F("WiFi verbunden."));

    server.on("/setWiFi", HTTP_POST, setWiFi);
    server.on("/set", HTTP_POST, set);
    server.on("/getallFunctions", HTTP_GET, getAllFunctions);
    server.on("/getsettingofFunction", HTTP_POST, getsettingofFunction);
    server.on("/seteffectset", HTTP_POST, seteffectsetSetting);
    server.on("/on", HTTP_GET, seton);
    server.on("/off", HTTP_GET, setoff);
    server.on("/getbon", HTTP_GET, getbon);
    server.on("/getSettings", HTTP_GET, getSettings);
    server.on("/setSettings", HTTP_POST, setSettings);
    server.on("/getStatus", HTTP_GET, getStatus);
    server.on("/restart", HTTP_GET, restart);
    server.on("/reset", HTTP_POST, resetSettings);
    server.onNotFound(v404);

    DEBUG_PRINT(F("IP Addresse: "));
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINTLN(WiFi.SSID());

    server.begin();

    // INIT UDP SEARCH
    Udp.begin(8000);

    delay(500);
    fill_solid(leds, MAX_NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void loop()
{

    if (bon)
    {
        effectarray[currentEffect]->loop(leds, config.num_leds);
    }
    else
    {
        for (int i = 0; i < config.num_leds; i++)
        {
            leds[i] = CRGB::Black;
        }
    }

    // Get the current time
    unsigned long continueTime = millis() + int(float(1000 / 50));
    // Do our main loop functions, until we hit our wait time

    do
    {
        FastLED.show();
        server.handleClient();
        yield();

        if (WiFi.status() != WL_CONNECTED)
        {
            DEBUG_PRINTLN(F("NO WIFI"));
        }

        // UDP SEARCH
        int packetSize = Udp.parsePacket();
        if (packetSize)
        {
            String name(config.name);
            String typ(type);
            String ReplyBuffer = name + "|" + WiFi.macAddress() + "|" + typ;
            char cstr[ReplyBuffer.length() + 1];
            strcpy(cstr, ReplyBuffer.c_str());
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(cstr);
            Udp.endPacket();
        }

    } while (millis() < continueTime);
}

void seteffectsetSetting()
{
    String data = server.arg("plain");
    StaticJsonDocument<400> dataJson;

    DeserializationError error = deserializeJson(dataJson, data);
    if (error)
    {
        DEBUG_PRINTLN(F("deserializeJson() fehlgeschlagen: "));
        DEBUG_PRINTLN(error.c_str());
        return;
    }

    effectarray[dataJson["i"].as<int>()]->deserialize(dataJson);

    server.send(200, "application/json", "OK");
}

void seton()
{
    if (bon == false)
        bon = true;

    server.send(200, "application/plain", "OK");
}

void setoff()
{
    if (bon == true)
        bon = false;

    server.send(200, "application/plain", "OK");
}

void getbon()
{
    server.send(200, "application/plain", bon + "");
}

void getAllFunctions()
{
    // {
    //     "e" : [
    //         {
    //             "n" : "12345678912345678912",
    //             "id" : "123"
    //         },
    //         {
    //             "n" : "12345678912345678912",
    //             "id" : "123"
    //         },
    //     ]
    // }
    int len = sizeof(effectarray) / sizeof(effectarray[0]);
    const size_t capacity = JSON_ARRAY_SIZE(len + 1) + JSON_OBJECT_SIZE(1) + (len + 1) * JSON_OBJECT_SIZE(5);
    DynamicJsonDocument doc(capacity);

    JsonArray arra = doc.createNestedArray("e");

    // getAllFunctionsGeneration
    JsonObject e_0 = arra.createNestedObject();
    e_0["n"] = effectarray[0]->name;
    e_0["id"] = 0;

    JsonObject e_1 = arra.createNestedObject();
    e_1["n"] = effectarray[1]->name;
    e_1["id"] = 1;

    server.send(200, "application/json", doc.as<String>());
}

void getsettingofFunction()
{
    // {
    //     "e": [
    //         {
    //             "n" : "Speed",
    //             "t" : "i",
    //             "mn": "1",
    //             "mx": "15",
    //             "v": "2",
    //             "e" : "s"
    //         },
    //         {
    //             "n" : "Speed",
    //             "t" : "i",
    //             "mn": "1",
    //             "mx": "15",
    //             "v": "2",
    //             "e" : "s"
    //         },
    //     ]
    // }

    // {
    //     "i": "0"
    // }

    String data = server.arg("plain");
    StaticJsonDocument<30> dataJson;

    DeserializationError error = deserializeJson(dataJson, data);
    if (error)
    {
        DEBUG_PRINTLN(F("deserializeJson() fehlgeschlagen: "));
        DEBUG_PRINTLN(error.c_str());
        return;
    }

    const size_t capacity = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(1) + (10) * JSON_OBJECT_SIZE(7);
    DynamicJsonDocument doc(capacity);

    JsonArray arra = doc.createNestedArray("e");

    effectarray[dataJson["i"].as<int>()]->getData(arra);

    DEBUG_PRINTLN("Sending");

    server.send(200, "application/json", doc.as<String>());
}

void set()
{
    String data = server.arg("plain");
    StaticJsonDocument<30> dataJson;

    DeserializationError error = deserializeJson(dataJson, data);
    if (error)
    {
        DEBUG_PRINTLN(F("deserializeJson() fehlgeschlagen: "));
        DEBUG_PRINTLN(error.c_str());
        return;
    }

    currentEffect = dataJson["i"].as<int>();
    effectarray[currentEffect]->begin();

    server.send(200, "application/plain", "OK");
}

void resetSettings()
{
    SPIFFS.remove(filename);
    server.send(200, "application/plain", "OK");
    delay(2000);
    ESP.restart();
}

// TODO: ADD CHECKS EVERYWHERE
void setSettings()
{
    Config conf = Config();
    String data = server.arg("plain");
    StaticJsonDocument<500> doc;

    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        DEBUG_PRINTLN(F("deserializeJson() fehlgeschlagen: "));
        DEBUG_PRINTLN(error.c_str());
        return;
    }

    strlcpy(conf.name, doc["n"] | config.name, sizeof(conf.name));
    conf.num_leds = doc["l"] | config.num_leds;
    strlcpy(conf.password, config.password, sizeof(conf.password));
    strlcpy(conf.ssid, config.ssid, sizeof(conf.ssid));
    saveConf(filename, conf);
    server.send(200, F("text/plain"), F("f"));
    delay(2000);
    ESP.restart();
}

void setWiFi()
{
    Config conf = Config();
    String data = server.arg("plain");
    StaticJsonDocument<500> doc;

    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        DEBUG_PRINTLN(F("deserializeJson() fehlgeschlagen: "));
        DEBUG_PRINTLN(error.c_str());
        return;
    }
    strlcpy(conf.name, type, sizeof(conf.name));
    conf.num_leds = 30;
    strlcpy(conf.password, doc["p"] | config.password, sizeof(conf.password));
    strlcpy(conf.ssid, doc["w"] | config.ssid, sizeof(conf.ssid));
    saveConf(filename, conf);
    server.send(200, F("text/plain"), F("f"));
    delay(2000);
    ESP.restart();
}

void saveConf(const char *filename, const Config &config)
{
    SPIFFS.remove(filename);

    // Datei öffnen zum schreiben
    File file = SPIFFS.open(filename, "w");
    if (!file)
    {
        DEBUG_PRINTLN(F("Failed to create file"));
        return;
    }

    StaticJsonDocument<500> doc;

    // Setze values des Dokuments
    doc["w"] = config.ssid;
    doc["p"] = config.password;
    doc["l"] = config.num_leds;
    doc["n"] = config.name;

    // Serialize JSON zu Datei
    if (serializeJson(doc, file) == 0)
    {
        DEBUG_PRINTLN(F("Failed to write to file"));
    }

    // Schließe die Datei
    file.close();
}

void loadConfig(const char *filename, Config &config)
{
    File file = SPIFFS.open(filename, "r");
    StaticJsonDocument<500> doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        DEBUG_PRINTLN(F("Datei lesen fehlgeschlagen, nutze Standart Configuration"));
        DEBUG_PRINTLN(error.c_str());
    }

    config.num_leds = doc["l"] | 30;
    strlcpy(config.ssid, doc["w"] | "", sizeof(config.ssid));
    strlcpy(config.password, doc["p"] | "", sizeof(config.password));
    // TODO: Nutze ESP.getChipID
    strlcpy(config.name, doc["n"] | type, sizeof(config.name));

    file.close();
}

void printFile(const char *filename)
{
    // Datei öffnen zum lesen
    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
        DEBUG_PRINTLN(F("Daten lesen fehlgeschlagen"));
        return;
    }

    // Schreibe jeden Charakter einzeln
    while (file.available())
    {
        DEBUG_PRINT((char)file.read());
    }
    DEBUG_PRINTLN();

    // Schließe die Datei
    file.close();
}

void getSettings()
{
    // {
    //     "e": [
    //         {
    //             "t": "i",
    //             "mn": "1",
    //             "mx": "999",
    //             "n": "Led Anzahl",
    //             "e":"l"
    //         },
    //         {
    //             "t": "s",
    //             "mn": "4",
    //             "mx": "64",
    //             "n": "Name",
    //             "e":"n"
    //         }
    //     ]
    // }
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(5);
    DynamicJsonDocument doc(capacity);

    JsonArray e = doc.createNestedArray("e");

    JsonObject e_0 = e.createNestedObject();
    e_0["t"] = "i";
    e_0["mn"] = "1";
    e_0["mx"] = "999";
    e_0["n"] = "Led Anzahl";
    e_0["e"] = "l";

    JsonObject e_1 = e.createNestedObject();
    e_1["t"] = "s";
    e_1["mn"] = "4";
    e_1["mx"] = "64";
    e_1["n"] = "Name";
    e_1["e"] = "n";

    server.send(200, "application/json", doc.as<String>());
}

void getStatus()
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

void restart()
{
    server.send(200, "application/plain", "OK");
    delay(100);
    ESP.restart();
}

void v404()
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