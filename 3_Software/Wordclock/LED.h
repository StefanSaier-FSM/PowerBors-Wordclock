#ifndef LED_H 
#define LED_H

#include <stdint.h>

#define NUM_LEDS 114      // Anzahl der angschlossenen LEDs
#define LED_NUM_COLUMNS 10    // Anzahl der Spalten (Aufbau der LED Matrix)
#define LED_NUM_ROW 11        // Anzhal der Reihen (Aufbau der LED Matrix)
#define PIN_LED 32        // Pin Anwelchem die LEDs angeschlossen sind
#define BRIGHTNESS 100    // Maximale Helligkeit welche die LEDs haben können (max. 254)
#define MIN_BRIGHTNESS 5  // Minimale Helligkeit der LEDs

#define COLOR_GREEN     0xFF0000
#define COLOR_YELLOW    0xFFFF00
#define COLOR_RED 0x00FF00
#define COLOR_PINK      0X0FFFFF
#define COLOR_BLUE      0x0000FF
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF

static bool LedLeftSideUpOn = false;
static bool LedLeftSideDownOn = false;
static bool LedRightSideUpOn = false;
static bool LedRightSideDownOn = false;

void calculateLedPosition();
uint8_t getLedPosition(uint32_t column, uint32_t row);
void setLed(uint32_t column, uint32_t row, uint32_t color);
void setLed(uint32_t position, uint32_t color);
void setAllLeds(uint32_t color);
void updateLEDs();
void setBrightness(uint8_t brightness);
void ledInit();

#endif