#include <Base_Class.h>

class RainbowEffect final : public Base_Class
{
public:
    uint8_t Hue = 2; // difference between two led
    int8_t rate = 1; // change of hue on each loop

    RainbowEffect(const char *name) : Base_Class(name){};

    void deserialize(DynamicJsonDocument &data) override
    {
        // Hue
        if (data["hue"])
        {
            DEBUG_PRINTLN(F("RainbowEffect: hue to "));
            DEBUG_PRINTLN(data["hue"].as<uint8_t>());
            Hue = data["hue"].as<uint8_t>();
        }

        // rate
        if (data["rate"])
        {
            DEBUG_PRINTLN(F("RainbowEffect: rate to "));
            DEBUG_PRINTLN(data["rate"].as<int8_t>());
            rate = data["rate"].as<int8_t>();
        }
    }

    void serialize(JsonObject &data) const override
    {
        data["hue"] = Hue;
        data["rate"] = rate;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        _hue += rate;
        fill_rainbow(&(leds[0]), NUM_LEDS, _hue, Hue);
    }

protected:
    uint8_t _hue = 0;
};