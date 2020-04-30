#include <effect.h>

class old final : public effect
{
private:
    int audio = A0;

    // STANDARD VISUALIZER VARIABLES
    int loop_max = 0;
    int k = 255;   // COLOR WHEEL POSITION
    int decay = 0; // HOW MANY MS BEFORE ONE LIGHT DECAY
    int decay_check = 0;
    long pre_react = 0;  // NEW SPIKE CONVERSION
    long react = 0;      // NUMBER OF LEDs BEING LIT
    long post_react = 0; // OLD SPIKE CONVERSION
    int wheel_speed = 3;

    int sensivity = 2;

    CRGB Scroll(int pos)
    {
        CRGB color(0, 0, 0);
        if (pos < 85)
        {
            color.g = 0;
            color.r = ((float)pos / 85.0f) * 255.0f;
            color.b = 255 - color.r;
        }
        else if (pos < 170)
        {
            color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
            color.r = 255 - color.g;
            color.b = 0;
        }
        else if (pos < 256)
        {
            color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
            color.g = 255 - color.b;
            color.r = 1;
        }
        return color;
    }

    void rainbow(CRGB leds[], int NUM_LEDS)
    {
        for (int i = NUM_LEDS - 1; i >= 0; i--)
        {
            if (i < react)
                leds[i] = Scroll((i * 256 / 50 + k) % 256);
            else
                leds[i] = CRGB(0, 0, 0);
        }
        FastLED.show();
    }

public:
    old(const char *name) : effect(name){};

    void deserialize(JsonDocument &data) override
    {
        if (data["w"])
        {
            wheel_speed = data["w"].as<uint8_t>();
        }
        if (data["s"])
        {
            sensivity = data["s"].as<uint8_t>();
        }
    }

    void
    getData(JsonArray &data) const override
    {
        JsonObject e_0 = data.createNestedObject();
        e_0["n"] = "Wheel Speed";
        e_0["t"] = "i";
        e_0["mn"] = "0";
        e_0["mx"] = "30";
        e_0["v"] = wheel_speed;
        e_0["e"] = "w";
        e_0["s"] = "3";

        JsonObject e_1 = data.createNestedObject();
        e_1["n"] = "Sensivity";
        e_1["t"] = "i";
        e_1["mn"] = "1";
        e_1["mx"] = "6";
        e_1["v"] = sensivity;
        e_1["e"] = "s";
        e_1["s"] = "2";
    }

    void begin()
    {
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        EVERY_N_MILLISECONDS(10)
        {
            int audio_input = analogRead(audio) * 2; // ADD x2 HERE FOR MORE SENSITIVITY

            if (audio_input > 0)
            {
                pre_react = ((long)NUM_LEDS * (long)audio_input) / 1023L; // TRANSLATE AUDIO LEVEL TO NUMBER OF LEDs

                if (pre_react > react) // ONLY ADJUST LEVEL OF LED IF LEVEL HIGHER THAN CURRENT LEVEL
                    react = pre_react;
            }

            rainbow(leds, NUM_LEDS); // APPLY COLOR

            k = k - wheel_speed; // SPEED OF COLOR WHEEL
            if (k < 0)           // RESET COLOR WHEEL
                k = 255;

            // REMOVE LEDs
            decay_check++;
            if (decay_check > decay)
            {
                decay_check = 0;
                if (react > 0)
                    react--;
            }
        }
    }
};