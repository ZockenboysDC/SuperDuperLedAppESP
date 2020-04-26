#include <effect.h>

class beatwave final : public effect
{
private:
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;
    TBlendType currentBlending = LINEARBLEND;

public:
    beatwave(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        // if (data["s"])
        // {
        //     speed = data["s"].as<uint8_t>();
        // }
    }

    void getData(JsonArray &data) const override
    {
        // JsonObject e_0 = data.createNestedObject();
        // e_0["n"] = "Fade Speed";
        // e_0["t"] = "i";
        // e_0["mn"] = "1";
        // e_0["mx"] = "25";
        // e_0["v"] = fadespeed;
        // e_0["e"] = "fs";
        // e_0["s"] = "9";
    }

    void begin()
    {
        currentPalette = RainbowColors_p;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        uint8_t wave1 = beatsin8(9, 0, 255); // That's the same as beatsin8(9);
        uint8_t wave2 = beatsin8(8, 0, 255);
        uint8_t wave3 = beatsin8(7, 0, 255);
        uint8_t wave4 = beatsin8(6, 0, 255);

        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(currentPalette, i + wave1 + wave2 + wave3 + wave4, 255, currentBlending);
        }

        EVERY_N_MILLISECONDS(100)
        {
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // AWESOME palette blending capability.
        }

        EVERY_N_SECONDS(5)
        { // Change the target palette to a random one every 5 seconds.
            targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
        }

        FastLED.show();
    }
};
