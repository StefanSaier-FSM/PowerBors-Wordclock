// Header File einbinden
#include "Icons.h"

//Bibliotheke welche in diesem File benötigt werden einbinden.
#include "LED.h"

void setIconFullScreen(bool Icon[NUM_COLUMNS][NUM_ROW], uint32_t onColor, uint32_t offColor, Rotation_e rotation) {
  bool rotatedIcon[NUM_COLUMNS][NUM_ROW];
  switch (rotation) {
    default:
    case No_Rotation:
    case Rotation_0_deg:
      for (uint8_t column = 0; column < NUM_COLUMNS; column++) {
        for (uint8_t row = 0; row < NUM_ROW; row++) {
          rotatedIcon[column][row] = Icon[column][row];
        }
      }
      break;
    case Rotation_180_deg:
      for (uint8_t column = 0; column < NUM_COLUMNS; column++) {
        for (uint8_t row = 0; row < NUM_ROW; row++) {
          rotatedIcon[column][row] = Icon[NUM_COLUMNS - column - 1][NUM_ROW - row - 1];
        }
      }
  }

  for (uint8_t column = 0; column < NUM_COLUMNS; column++) {
    for (uint8_t row = 0; row < NUM_ROW; row++) {
      if (rotatedIcon[column][row] == 1) {
        setLed(column, row, onColor);
      } else {
        setLed(column, row, offColor);
      }
    }
  }

  //Leds in den Ecken Steuern
  if(LedLeftSideUpOn == true)
  {
    setLed(12, onColor);
  }
  else
  {
    setLed(12, offColor);
  }

  if(LedLeftSideDownOn == true)
  {
    setLed(101, onColor);
  }
  else
  {
    setLed(101, offColor);
  }

  if(LedRightSideUpOn == true)
  {
    setLed(0, onColor);
  }
  else
  {
    setLed(0, offColor);
  }

  if(LedRightSideDownOn == true)
  {
    setLed(113, onColor);
  }
  else
  {
    setLed(113, offColor);
  }
}

void clearIconArray(bool IconPointer[][NUM_ROW])
{
  for (uint8_t i = 0; i < NUM_COLUMNS; i++)
  {
    for (uint8_t j = 0; j < NUM_ROW; j++)
    {
      IconPointer[i][j] = false;
    }
  }
}

void addPixelArrayToArray(bool firstIconPointer[][NUM_ROW], bool secondIconPointer[][NUM_ROW])
{
  for (uint8_t i = 0; i < NUM_COLUMNS; i++)
  {
    for (uint8_t j = 0; j < NUM_ROW; j++)
    {
      firstIconPointer[i][j] = firstIconPointer[i][j] | secondIconPointer[i][j];
    }
  }
}