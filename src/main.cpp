#include "Arduino.h"
#include "Debug.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "FastLED.h"
#include "WiFiUdp.h"
#include "effect.h"
#include "effects/test.cpp"

// Config stuff
struct Config
{
    char ssid[64];
    char password[100];
    char name[64];
    int num_leds;
};

effect **base = new effect *[1];

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

void setWiFi();
void loadConfig(const char *filename, Config &config);
void printFile(const char *filename);
void saveConf(const char *filename, const Config &config);
void getSettings();
void resetSettings();
void setSettings();
void v404();

void setup()
{
    // TODO: Add all Functions
    base[0] = new test;

#ifdef DEBUG
    Serial.begin(115200);
#endif

    DEBUG_PRINTLN("Name:");
    DEBUG_PRINTLN(base[0]->name);

    // init SPIFF and load config
    SPIFFS.begin();
    printFile(filename);
    loadConfig(filename, config);

    // LEDs
    FastLED.addLeds<LED_TYPE, DATA_PIN1, GRB>(leds, MAX_NUM_LEDS);
    FastLED.addLeds<LED_TYPE, DATA_PIN2, GRB>(leds, MAX_NUM_LEDS);

    // TODO: REMOVE AFTER REALESE
    WiFi.disconnect();

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
    server.on("/getSettings", HTTP_GET, getSettings);
    server.on("/setSettings", HTTP_POST, setSettings);
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
    // TODO: AND MAIN LED HANDLE
    // UDP SEARCH
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        // TODO: REMOVE
        DEBUG_PRINTF("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                     packetSize,
                     Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                     Udp.destinationIP().toString().c_str(), Udp.localPort(),
                     ESP.getFreeHeap());

        int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        packetBuffer[n] = 0;
        DEBUG_PRINTLN(F("Inhalt: "));
        DEBUG_PRINT(packetBuffer);

        String name(config.name);
        String typ(type);
        String ReplyBuffer = name + "|" + WiFi.macAddress() + "|" + typ;
        char cstr[ReplyBuffer.length() + 1];
        strcpy(cstr, ReplyBuffer.c_str());
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(cstr);
        Udp.endPacket();
    }
    server.handleClient();
}

void resetSettings()
{
    SPIFFS.remove(filename);
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