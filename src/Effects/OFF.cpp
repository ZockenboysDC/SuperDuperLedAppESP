#include <Base_Class.h>

class Off final : public Base_Class
{
public:

    Off(const char *name) : Base_Class(name){}

    void deserialize(DynamicJsonDocument &data) override{}

    void serialize(JsonObject &data) const override{}

    void loop(CRGB leds[], int NUM_LEDS) override{
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Black;
        }
        FastLED.show();
    }
};
