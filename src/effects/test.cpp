#include <effect.h>

class test final : public effect
{
public:

    char name[20] = "test";

    void deserialize(JsonDocument &data) override {}

    void getData(JsonObject &data) const override
    {
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Black;
        }
    }
};