// Header File einbinden
#include "TemperatureSensor.h"
#include <esp32-hal-adc.h>

// Definitionen Welche nur innerhalb des innerhalb des Moduls benötigt werden anlegen
#define PIN_TEMPERATUR_SENSOR 26
#define UMRECHNUNGSFAKTOR 12.1212;

//Variabeln welche nur innerhalb des Moduls benötigt werden anlegen
float temperature = 0.0;

// Umsetzung der Funktionen
float checkTemperature() {
  int value_tempature = analogRead(PIN_TEMPERATUR_SENSOR);
  float voltage_temp = value_tempature * (3.3 / 4095.0);
  temperature = voltage_temp * UMRECHNUNGSFAKTOR;
  return temperature;
}

float getTemperature()
{
  return temperature;
}