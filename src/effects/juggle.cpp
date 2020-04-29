#include <effect.h>

class juggle final : public effect
{
private:
    int fadespeed = 90;

public:
    juggle(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["fs"])
        {
            fadespeed = data["fs"].as<uint8_t>();
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
        e_0["s"] = "90";
    }

    void begin()
    {
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        fadeToBlackBy(leds, NUM_LEDS, fadespeed);
        int dothue = 0;
        for (int i = 0; i < 8; i++)
        {
            leds[beatsin16(i + 7, 0, NUM_LEDS)] |=
                CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }
};