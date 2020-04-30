#pragma once

#include <ArduinoJson.h>
#include <Debug.h>
#include <FastLED.h>

class effect
{
public:
    char name[20];
    const size_t jsonBufferSize = 0;

    effect(const char *name)
    {
        strncpy(this->name, name, 20);
    }

    virtual void begin()
    {
        DEBUG_PRINTLN(F("BaseEffect: Beginning Effect"));
    }

    virtual void deserialize(JsonDocument &data) = 0;
    virtual void getData(JsonArray &data) const = 0;
    virtual void loop(CRGB leds[], int NUM_LEDS) = 0;
};