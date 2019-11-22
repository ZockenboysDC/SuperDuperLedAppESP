#include <Base_Class.h>

#define MIC_PIN 0

class visualizer final : public Base_Class
{

public:
    visualizer(const char *name) : Base_Class(name){};

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

    void deserialize(DynamicJsonDocument &data) override
    {
        delay(10);
        DEBUG_PRINTLN(data.as<String>() + "!");
        // max_bright
        if (data["Brightness"])
        {
            DEBUG_PRINTLN(F("Visualizer: Brightness to "));
            DEBUG_PRINTLN(data["Brightness"].as<String>());
            max_bright = data["Brightness"].as<uint8_t>();
            FastLED.setBrightness(max_bright);
        }

        // high
        if (data["High"])
        {
            DEBUG_PRINTLN(F("Visualizer: High to "));
            DEBUG_PRINTLN(data["High"].as<String>());
            high = data["High"].as<int>();
        }

        // decay
        if (data["Decay"])
        {
            DEBUG_PRINTLN(F("Visualizer: Decay to "));
            DEBUG_PRINTLN(data["Decay"].as<String>());
            decay = data["Decay"].as<int>();
        }

        // wheel_speed
        if (data["Wheel_speed"])
        {
            DEBUG_PRINTLN(F("Visualizer: Wheel_speed to "));
            DEBUG_PRINTLN(data["Wheel_speed"].as<String>());
            wheel_speed = data["Wheel_speed"].as<int>();
        }
        delay(10);
    }

    void serialize(JsonObject &data) const override
    {
        data["Brightness"] = max_bright;
        data["Decay"] = decay;
        data["Wheel_speed"] = wheel_speed;
        data["High"] = high;
    }

    void loop(CRGB leds[], int NUM_LEDS) override
    {
        int audio_input = analogRead(MIC_PIN); // ADD x2 HERE FOR MORE SENSITIVITY

        delay(1);

        if (audio_input > 0)
        {
            pre_react = (((long)NUM_LEDS * (long)audio_input) / 1023L) * high; // TRANSLATE AUDIO LEVEL TO NUMBER OF LEDs

            if (pre_react > react && pre_react <= NUM_LEDS) // ONLY ADJUST LEVEL OF LED IF LEVEL HIGHER THAN CURRENT LEVEL
                react = pre_react;

            delay(1);

        }

        delay(1);

        for (int i = NUM_LEDS; i >= 0; i--)
        {
            if (i < react)
                leds[i] = Scroll((i * 256 / 50 + k) % 256);
            else
                leds[i] = CRGB(0, 0, 0);
        }

        k = k - wheel_speed; // SPEED OF COLOR WHEEL
        if (k < 0)           // RESET COLOR WHEEL
            k = 255;

        delay(1);

        // REMOVE LEDs
        decay_check++;
        if (decay_check > decay)
        {
            decay_check = 0;
            if (react > 0)
                react--;
        }
        delay(1);
    }

protected:
    // RAINBOW WAVE SETTINGS
    int wheel_speed = 3;
    uint8_t max_bright = 128; // Overall brightness definition. It can be changed on the fly.
    int high = 4;
    // STANDARD VISUALIZER VARIABLES
    int loop_max = 0;
    int k = 255;   // COLOR WHEEL POSITION
    int decay = 5; // HOW MANY MS BEFORE ONE LIGHT DECAY
    int decay_check = 0;
    long pre_react = 0;  // NEW SPIKE CONVERSION
    long react = 30;     // NUMBER OF LEDs BEING LIT
    long post_react = 0; // OLD SPIKE CONVERSION
};