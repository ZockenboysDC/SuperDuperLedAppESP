#include <effect.h>

class comet final : public effect
{
private:
    int fadespeed = 9;
    int speed = 3;
    int dothue = 0;
    int lead_dot = 0;

public:
    comet(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["fs"])
        {
            fadespeed = data["fs"].as<uint8_t>();
        }
        if (data["s"])
        {
            speed = data["s"].as<uint8_t>();
        }
    }

    void getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "Fade Speed";
        e_0["t"] = "i";
        e_0["mn"] = "1";
        e_0["mx"] = "25";
        e_0["v"] = fadespeed;
        e_0["e"] = "fs";
        e_0["s"] = "9";

        JsonObject e_1 = data.createNestedObject();
        e_1["n"] = "Speed";
        e_1["t"] = "i";
        e_1["mn"] = "1";
        e_1["mx"] = "4";
        e_1["v"] = speed;
        e_1["e"] = "s";
        e_1["s"] = "3";
    }

    void begin() {
        dothue = 0;
        lead_dot = 0;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        fadeToBlackBy(leds, NUM_LEDS, fadespeed);
        lead_dot = beatsin16(int(float(50 / speed)), 0, NUM_LEDS);
        leds[lead_dot] = CHSV(dothue, 200, 255);
        dothue += 8;
    }
};