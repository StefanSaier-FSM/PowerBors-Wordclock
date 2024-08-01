// Header File einbinden
#include "LED.h"

//Bibliotheke welche in diesem File benötigt werden einbinden.
#include "Adafruit_NeoPixel.h" // Adafruit NeoPixel by Adafruit Version V1.11.0
#include <Wire.h>

uint8_t ledPosition[LED_NUM_COLUMNS][LED_NUM_ROW];
Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_RGB + NEO_KHZ800);

void calculateLedPosition() {
  uint8_t position = 1;
  for (uint8_t column = 0; column < LED_NUM_COLUMNS; column++) {
    uint8_t starterRow = 0;
    if ((column % 2) == 0) {
      //gerade Zeile
      starterRow = 10;
    } else {
      // ungerade Zeile
      starterRow = 0;
    }

    uint8_t row = starterRow;
    while (row < LED_NUM_ROW) {
      ledPosition[column][row] = position;
      position++;

      if (position == 12) {
        //LED links oben im Eck ueberspringen
        position = 13;
      }

      if (position == 101) {
        //LED lniks unten im Eck ueberspringen
        position = 102;
      }

      if ((column % 2) == 0) {
        //gerade Zeile
        row--;
      } else {
        // ungerade Zeile
        row++;
      }
    }
  }
}

uint8_t getLedPosition(uint32_t column, uint32_t row) {
  return ledPosition[column][row];
}

void setLed(uint32_t column, uint32_t row, uint32_t color) {
  strip.setPixelColor(getLedPosition(column, row), color);
}

void setLed(uint32_t position, uint32_t color){
  strip.setPixelColor(position, color);
}

void setAllLeds(uint32_t color) {
  strip.fill(color, 0, NUM_LEDS);
}

void updateLEDs() {
  strip.show();
}

void setBrightness(uint8_t brightness)
{
  strip.setBrightness(brightness);
}

void ledInit() {
  strip.begin();
  calculateLedPosition();
}