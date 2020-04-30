#pragma once

// TODO: Remove when releasing gives some bytes back
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...); Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...); Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...); Serial.printf(__VA_ARGS__)
#else
#define Debug
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINT(...)
#define DEBUG_PRINTF(...)
#endif