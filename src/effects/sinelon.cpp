#include <effect.h>

class sinelon final : public effect
{
private:
    int fadespeed = 25;
    int gHue = 0;

public:
    sinelon(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["fs"])
        {
            fadespeed = data["fs"].as<uint8_t>();
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
        e_0["mx"] = "90";
        e_0["v"] = fadespeed;
        e_0["e"] = "fs";
        e_0["s"] = "25";

        JsonObject e_1 = data.createNestedObject();
        e_1["n"] = "Color";
        e_1["t"] = "i";
        e_1["mn"] = "0";
        e_1["mx"] = "255";
        e_1["v"] = gHue;
        e_1["e"] = "g";
        e_1["s"] = "0";
    }

    void begin()
    {
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        fadeToBlackBy(leds, NUM_LEDS, fadespeed);
        int pos = beatsin16(13, 0, NUM_LEDS);
        leds[pos] += CHSV(gHue, 255, 255);
    }
};