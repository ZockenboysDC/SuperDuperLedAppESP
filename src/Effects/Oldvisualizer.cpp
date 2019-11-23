#include <Base_Class.h>

#define MIC_PIN 0

class Oldvisualizer final : public Base_Class
{

public:
    Oldvisualizer(const char *name) : Base_Class(name){};

    void deserialize(DynamicJsonDocument &data) override
    {
        
        DEBUG_PRINTLN(data.as<String>() + "!");
        // // max_bright
        // if (data["Brightness"])
        // {
        //     DEBUG_PRINTLN(F("Visualizer: Brightness to "));
        //     DEBUG_PRINTLN(data["Brightness"].as<String>());
        //     max_bright = data["Brightness"].as<uint8_t>();
        //     FastLED.setBrightness(max_bright);
        // }

    }

    void serialize(JsonObject &data) const override
    {
        // data["Brightness"] = max_bright;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {

        n = analogRead(MIC_PIN);
        if (n > 0){
            lvl_L = ((lvl_L * 7) + n) >> 3; 
        }
       
        // Calculate bar height based on dynamic min/max levels (fixed point):
        height = TOP(NUM_LEDS) * (lvl_L - minLvlAvg_L) / (long)(maxLvlAvg_L - minLvlAvg_L);

        if (height < 0L)
            height = 0; // Clip output
        else if (height > TOP(NUM_LEDS))
            height = TOP(NUM_LEDS);
        if (height > peak_L)
            peak_L = height; // Keep 'peak_L' dot at TOP(NUM_LEDS)

        // Color pixels based on rainbow gradient
        for (i = 0; i < NUM_LEDS; i++)
        {
            if (i >= height)
            {
                leds[i].setRGB(0, 0, 0);
            }
            else
            {
                leds[i] = CHSV(map(i, 0, NUM_LEDS - 1, 30, 250), 255, 255);
            }
        }

        // Draw peak_L dot
        if (peak_L > 0 && peak_L <= NUM_LEDS - 1)
        {
            leds[peak_L] = CHSV(map(peak_L, 0, NUM_LEDS - 1, 30, 250), 255, 255);
        }

        if (++dotCount_L >= PEAK_FALL)
        { // fall rate
            if (peak_L > 0)
                peak_L--;
            dotCount_L = 0;
        }

        vol_L[volCount_L] = n; // Save sample for dynamic leveling
        if (++volCount_L >= SAMPLES)
            volCount_L = 0; // Advance/rollover sample counter

        // Get volume range of prior frames
        minLvl = maxLvl = vol_L[0];
        for (i = 1; i < SAMPLES; i++)
        {
            if (vol_L[i] < minLvl)
                minLvl = vol_L[i];
            else if (vol_L[i] > maxLvl)
                maxLvl = vol_L[i];
        }

        if ((maxLvl - minLvl) < TOP(NUM_LEDS))
            maxLvl = minLvl + TOP(NUM_LEDS);
        minLvlAvg_L = (minLvlAvg_L * 62 + minLvl) >> 6; // Dampen min/max levels
        maxLvlAvg_L = (maxLvlAvg_L * 62 + maxLvl) >> 6; // (fake rolling average)
    }

protected:
    int TOP(int NUM_LEDS) {
        return NUM_LEDS + top2;
    }
    int  
        NOISE = 0,        // Noise/hum/interference in mic signal and increased value until it went quiet
        SAMPLES = 32,     // Length of buffer for dynamic level adjustment
        PEAK_FALL = 20,   // Rate of peak falling dot
        top2 = 0;

    byte
        peak_L = 0,     // Used for falling dot
        dotCount_L = 0, // Frame counter for delaying dot-falling speed
        volCount_L = 0; // Frame counter for storing past volume data

    int
        vol_L[32],  // Collection of prior volume samples
        lvl_L = 100,     // Current "dampened" audio level
        minLvlAvg_L = 0, // For dynamic adjustment of graph low & high
        maxLvlAvg_L = 512;

    uint8_t i;
    uint16_t minLvl, maxLvl;
    int n, height;
};