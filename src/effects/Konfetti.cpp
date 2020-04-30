#include <effect.h>

class Konfetti final : public effect
{
private:
    int fadespeed = 25;
    int densitie = 3;
    int gHue = 0;

public:
    Konfetti(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["fs"])
        {
            fadespeed = data["fs"].as<uint8_t>();
        }
        if (data["d"])
        {
            densitie = data["d"].as<uint8_t>();
        }
        if (data["g"])
        {
            gHue = data["g"].as<uint8_t>();
        }
    }

    void getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "Fade Speed";
        e_0["t"] = "i";
        e_0["mn"] = "1";
        e_0["mx"] = "35";
        e_0["v"] = fadespeed;
        e_0["e"] = "fs";
        e_0["s"] = "25";

        JsonObject e_1 = data.createNestedObject();
        e_1["n"] = "Konfetti Dichte";
        e_1["t"] = "i";
        e_1["mn"] = "1";
        e_1["mx"] = "15";
        e_1["v"] = densitie;
        e_1["e"] = "d";
        e_1["s"] = "9";

        JsonObject e_2 = data.createNestedObject();
        e_2["n"] = "Color";
        e_2["t"] = "i";
        e_2["mn"] = "1";
        e_2["mx"] = "190";
        e_2["v"] = gHue;
        e_2["e"] = "g";
        e_2["s"] = "0";
    }

    void begin()
    {
        gHue = 0;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        fadeToBlackBy(leds, NUM_LEDS, fadespeed);
        for (int x = 0; x < densitie; x++)
        {
            int pos = random16(NUM_LEDS);
            leds[pos] += CHSV(gHue + random8(64), 200, 255);
        }
    }
};