// Header File einbinden
#include "BrightnessSensor.h"
#include <esp32-hal-adc.h>

// Definitionen Welche nur innerhalb des innerhalb des Moduls benötigt werden anlegen
#define PIN_LDR 4

//Variabeln welche nur innerhalb des Moduls benötigt werden anlegen
int brightness = 0.0;

// Umsetzung der Funktionen
int checkBrightness() {
  int value_brightness = analogRead(PIN_LDR);
  brightness = (value_brightness >> 4);
  return brightness;
}

int getBrightness()
{
  return brightness;
}