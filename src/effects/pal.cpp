#include <effect.h>

#define MIC_PIN 0

class pal final : public effect
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

    void propPal(CRGB leds[], int NUM_LEDS)
    {

        leds[0] = ColorFromPalette(currentPalette, sampleAgc, sampleAgc, currentBlending);

        for (int i = NUM_LEDS - 1; i > 0; i--)
        { // Propagate up the strand.
            leds[i] = leds[i - 1];
        }
    }

public:
    pal(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
    }

    void getData(JsonArray &data) const override
    {
    }

    void begin()
    {
        currentPalette = OceanColors_p;
        targetPalette = OceanColors_p;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        EVERY_N_SECONDS(5)
        {                                       // Change the target palette to a random one every 5 seconds.
            static uint8_t baseC = random8(32); // You can use this as a baseline colour if you want similar hues in the next line.
            for (int i = 0; i < 16; i++)
                targetPalette[i] = CHSV(random8() + baseC, 255, 255);
        }

        EVERY_N_MILLISECONDS(100)
        {
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // AWESOME palette blending capability.
        }

        EVERY_N_MILLISECONDS(20)
        { // FastLED based non-blocking delay to update/display the sequence.
            getSample();
            agcAvg();
            propPal(leds, NUM_LEDS);
        }
    }
};