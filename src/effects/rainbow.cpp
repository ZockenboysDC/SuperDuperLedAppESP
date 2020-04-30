#include <effect.h>

class Rainbow final : public effect
{
private:
    int gHue = 0;
    int change = 7;

public:
    Rainbow(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["c"])
        {
            change = data["c"].as<uint8_t>();
        }
    }

    void getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "Hue change";
        e_0["t"] = "i";
        e_0["mn"] = "1";
        e_0["mx"] = "40";
        e_0["v"] = change;
        e_0["e"] = "c";
        e_0["s"] = "7";
    }

    void begin()
    {
        gHue = 0;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        fill_rainbow(leds, NUM_LEDS, gHue, change);
    }
};
