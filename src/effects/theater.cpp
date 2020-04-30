#include <effect.h>

class theater final : public effect
{
private:
    int red = 10;
    int blue = 50;
    int green = 20;

public:
    theater(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["r"])
        {
            red = data["r"].as<uint8_t>();
        }
        if (data["g"])
        {
            green = data["g"].as<uint8_t>();
        }
        if (data["b"])
        {
            blue = data["b"].as<uint8_t>();
        }
    }

    void
    getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "red";
        e_0["t"] = "i";
        e_0["mn"] = "1";
        e_0["mx"] = "255";
        e_0["v"] = red;
        e_0["e"] = "r";
        e_0["s"] = "10";

        JsonObject e_1 = data.createNestedObject();
        e_1["n"] = "green";
        e_1["t"] = "i";
        e_1["mn"] = "1";
        e_1["mx"] = "255";
        e_1["v"] = green;
        e_1["e"] = "g";
        e_1["s"] = "20";

        JsonObject e_2 = data.createNestedObject();
        e_2["n"] = "blue";
        e_2["t"] = "i";
        e_2["mn"] = "1";
        e_2["mx"] = "255";
        e_2["v"] = blue;
        e_2["e"] = "b";
        e_2["s"] = "50";
    }

    void begin()
    {
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        static int8_t frame = 0;

        for (int i = 0; i < NUM_LEDS; i = i + 3)
        {
            if (i + frame < NUM_LEDS)
            {
                leds[i + frame] = CRGB(0, 0, 0);
            }
        }

        frame++;
        if (frame > 2)
            frame = 0;

        for (int i = 0; i < NUM_LEDS; i = i + 3)
        {
            if (i + frame < NUM_LEDS)
            {
                leds[i + frame] =
                    CRGB(red, green, blue);
            }
        }
    }
};