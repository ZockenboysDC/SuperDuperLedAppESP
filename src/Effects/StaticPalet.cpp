#include <Base_Class.h>
#include <Arduino.h>

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

class StaticPalet final : public Base_Class
{
public:

    StaticPalet(const char *name) : Base_Class(name){}

    void deserialize(DynamicJsonDocument &data) override{}

    void serialize(JsonObject &data) const override{}

    void loop(CRGB leds[], int NUM_LEDS) override{
        uint8_t colorIndex = 0;
        for (uint8_t i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(myRedWhiteBluePalette_p, colorIndex, 64, LINEARBLEND);
            colorIndex += 3;
        }
    }
};
