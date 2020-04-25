#pragma once

#include <ArduinoJson.h>
#include <Debug.h>
#include <FastLED.h>

class effect
{
public:
    const size_t jsonBufferSize = 0;

    effect()
    {
    }

    virtual void begin()
    {
        DEBUG_PRINTLN(F("effect: Beginning Effect"));
    }

    virtual void deserialize(JsonDocument &data) = 0;
    virtual void getData(JsonObject &data) const = 0;
    virtual void loop(CRGB leds[], int NUM_LEDS) = 0;
};