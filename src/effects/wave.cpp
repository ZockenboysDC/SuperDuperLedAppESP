#include <effect.h>

#define MIC_PIN 0

class wave final : public effect
{
private:
    uint8_t squelch = 7; // Anything below this is background noise, so we'll make it '0'.
    int sample;          // Current sample.
    float sampleAvg = 0; // Smoothed Average.
    float micLev = 0;    // Used to convert returned value to have '0' as minimum.
    uint8_t maxVol = 11; // Reasonable value for constant volume for 'peak detector', as it won't always trigger.
    bool samplePeak = 0; // Boolean flag for peak. Responding routine must reset this flag.

    int sampleAgc, multAgc;
    uint8_t targetAgc = 60;

    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;
    TBlendType currentBlending = LINEARBLEND;

    int fadespeed = 16;

    void getSample()
    {

        int16_t micIn; // Current sample starts with negative values and large values, which is why it's 16 bit signed.
        static long peakTime;

        micIn = analogRead(MIC_PIN);                            // Poor man's analog Read.
        micLev = ((micLev * 31) + micIn) / 32;                  // Smooth it out over the last 32 samples for automatic centering.
        micIn -= micLev;                                        // Let's center it to 0 now.
        micIn = abs(micIn);                                     // And get the absolute value of each sample.
        sample = (micIn <= squelch) ? 0 : (sample + micIn) / 2; // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
        sampleAvg = ((sampleAvg * 31) + sample) / 32;           // Smooth it out over the last 32 samples.

        if (sample > (sampleAvg + maxVol) && millis() > (peakTime + 50))
        {                   // Poor man's beat detection by seeing if sample > Average + some value.
            samplePeak = 1; // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
            peakTime = millis();
        }

    } // getSample()

    void agcAvg()
    {

        multAgc = (sampleAvg < 1) ? targetAgc : targetAgc / sampleAvg;
        sampleAgc = sample * multAgc;
        if (sampleAgc > 255)
            sampleAgc = 255;
    }

    void sndwave(CRGB leds[], int NUM_LEDS)
    {

        leds[NUM_LEDS / 2] = ColorFromPalette(currentPalette, sampleAgc, sampleAgc, currentBlending); // Put the sample into the center

        for (int i = NUM_LEDS - 1; i > NUM_LEDS / 2; i--)
        { //move to the left      // Copy to the left, and let the fade do the rest.
            leds[i] = leds[i - 1];
        }

        for (int i = 0; i < NUM_LEDS / 2; i++)
        { // move to the right    // Copy to the right, and let the fade to the rest.
            leds[i] = leds[i + 1];
        }
    }

public:
    wave(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["fs"])
        {
            fadespeed = data["fs"].as<uint8_t>();
        }
    }

    void
    getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "Fade Speed";
        e_0["t"] = "i";
        e_0["mn"] = "1";
        e_0["mx"] = "255";
        e_0["v"] = fadespeed;
        e_0["e"] = "fs";
        e_0["s"] = "16";
    }

    void begin()
    {
        currentPalette = OceanColors_p;
        targetPalette = OceanColors_p;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        EVERY_N_SECONDS(5)
        { // Change the palette every 5 seconds.
            for (int i = 0; i < 16; i++)
            {
                targetPalette[i] = CHSV(random8(), 255, 255);
            }
        }

        EVERY_N_MILLISECONDS(100)
        { // AWESOME palette blending capability once they do change.
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
        }

        EVERY_N_MILLIS(20)
        {                                             // For fun, let's make the animation have a variable rate.
            fadeToBlackBy(leds, NUM_LEDS, fadespeed); // 1 = slow, 255 = fast fade. Depending on the faderate, the LED's further away will fade out.
            getSample();
            agcAvg();
            sndwave(leds, NUM_LEDS);
        }
    }
};