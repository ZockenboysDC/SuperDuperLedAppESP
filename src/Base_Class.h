#pragma once

#include <ArduinoJson.h>
#include <Debug.h>
#include <FastLED.h>

#ifndef LEDEFFECT_EFFECT_NAME_MAX_LENGTH
#define LEDEFFECT_EFFECT_NAME_MAX_LENGTH 20
#endif

class Base_Class
{
public:
    #ifdef DEBUG
    char name[LEDEFFECT_EFFECT_NAME_MAX_LENGTH];
    #endif
    const size_t jsonBufferSize = 0;

    Base_Class(const char *name)
    {
        #ifdef DEBUG
        strncpy(this->name, name, LEDEFFECT_EFFECT_NAME_MAX_LENGTH);
        #endif
    };

    virtual void begin()
    {
        #ifdef DEBUG
        DEBUG_PRINTLN(F("BaseEffect: Beginning effect:"));
        DEBUG_PRINTLN(name);
        #endif
    }

    virtual void deserialize(DynamicJsonDocument &data) = 0;
    virtual void serialize(JsonObject &data) const = 0;
    virtual void loop(CRGB leds[], int NUM_LEDS) = 0;
};