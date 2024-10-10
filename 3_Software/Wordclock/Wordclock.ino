#include "LED.h"
#include "Icons.h"
#include "RealTimeClock.h"
#include "TemperatureSensor.h"
#include "BrightnessSensor.h"
#include <Wire.h>
#include <EEPROM.h>

#define PIN_BUTTON_UP 12
#define PIN_BUTTON_DOWN 27
#define PIN_BUTTON_OK 14
#define PIN_SWITCH_AUTO 15
#define PIN_SWITCH_MANUELL 13

#define TIMER_TEMPERATURE 1000     
#define TIMER_LED_UPDATE 10        
#define TIMER_LAST_TIME_READ 1000  
#define TIMER_LAST_TIME_BUTTON_WAS_PRESSED 200
#define TIMER_CONSOLEOUTPUT 1000
#define TIMER_SELECTEDMENUPOINT 500

int ClockColor;
int posibleClockColors [10] = {0x0000FF, 0x8000FF, 0xFF00FF, 0xFF0080, 0xFF0000, 0xFF8000, 0xFFFF00, 0x80FF00, 0x00FF00, 0xFFFFFF};
int selectedClockColor = 0;

int jahr;
int monat;
int tag;
int stunde;
int minute;
int sekunde;
int wochentag;

uint8_t u8_manuelBrightnes;

int64_t i64_lastTimeRead;
int64_t i64_lastTimeButtonWasPressed;
int64_t i64_TimerLedUpdate;
int64_t i64_TimerTemperature;
int64_t i64_TimerConsoleOutput;
int64_t i64_TimerMenuPointSelected;

int y = 0;

uint8_t ClockColorDependingFromTemperature;

enum Mode_e {
  Mode_Uhr,
  Mode_Menu
};

enum Menu_e {
  Menu_Zeit_einstellen,
  Menu_Helligkeit_einstellen,
  Menu_Farbe_einstellen,
  Menu_Temperatur_hat_Auswirkung_auf_Farbe,
  Menu_verlassen,
};

//Bekanntmachungen
void setup();
void ledBootScreen();
void buttonProcessing();
void loop();
void ModeUhr();
void ModeMenu();
void calculateLEDsForClock(float temperature);
void saveConfiguration();
void loadConfiguration();

Mode_e Mode;
Menu_e Menu;
bool MenuPointSelected = false;
bool HourAdjustmentSelected = false;

void ledBootScreen() {
  setBrightness(BRIGHTNESS);
  setAllLeds(COLOR_GREEN);
  updateLEDs();
  delay(500);
  setAllLeds(COLOR_BLUE);
  updateLEDs();
  delay(500);
  setAllLeds(COLOR_RED);
  updateLEDs();
  delay(500);
  setAllLeds(COLOR_BLACK);
  updateLEDs();
}

void setup() {
  Serial.begin(57600);  //Starten der USB-Schnittstelle
  Wire.begin();         // Starten der SPI-Schnittelsee für die Real Time Clock
  Serial.println("Mit dem Befehl kann das Datum und die Uhrzeit gesetzt oder veraendert werden.");
  Serial.println("Beispiel: set 28.08.2013 10:54:23");

  // Taster und Schalter Pins in den richtigen Modus bringen
  pinMode(PIN_BUTTON_UP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_OK, INPUT_PULLUP);
  pinMode(PIN_SWITCH_AUTO, INPUT_PULLUP);
  pinMode(PIN_SWITCH_MANUELL, INPUT_PULLUP);

  ledInit(); // Die LEDs werden Initalisiert

  loadConfiguration();  // Die Einstallugnen werden aus dem Speicher geladen

  // Variabeln werden in definierte Zustände gebracht 
  Mode = Mode_Uhr;
  Menu = Menu_Zeit_einstellen;
  MenuPointSelected = false;
  i64_lastTimeRead = 0;
  i64_lastTimeButtonWasPressed = 0;
  i64_TimerLedUpdate = 0;
  i64_TimerTemperature = 0;
  i64_TimerConsoleOutput = 0;

  // Die Boot Animation wird ausgegeben
  ledBootScreen();
}

void loop() {
  // Wenn Ein Knopf gedrpckt wurde wird durch den Timer verhindert, dass dieser als merfach drücken erkannt wird. Dies diehnt zur Tasterentprellung
  if ((millis() - i64_lastTimeButtonWasPressed) >= TIMER_LAST_TIME_BUTTON_WAS_PRESSED) {
    i64_lastTimeButtonWasPressed = millis();
    buttonProcessing();
  }

  // Wenn der Timer abläuft werden die LEDs aktuallisiert
  if ((millis() - i64_TimerLedUpdate) >= TIMER_LED_UPDATE) {
    i64_TimerLedUpdate = millis();

    // Die Positionen der Schalter werden eingelesen
    int switch_auto = digitalRead(PIN_SWITCH_AUTO);
    int switch_manuell = digitalRead(PIN_SWITCH_MANUELL);

    // Die aktuelle Helligkeit wird gelesen
    checkBrightness();

    //Wenn der Schalter auf Auto ist wird die LED Helligkeit in abhänigkeit der gemessenen Helligkeit des Sensors angepasst.
    //ist der Scahlter auf der Manuelen Position werden die Helligkeit der LEDs mittels der im Menü einstellbaren Helligkeit ausgegeben 
    if (switch_auto == LOW) {
      if (getBrightness() <= MIN_BRIGHTNESS) {
        setBrightness(MIN_BRIGHTNESS);
      } else {
        setBrightness(getBrightness());
      }
    }
    if (switch_manuell == LOW) {
      setBrightness(u8_manuelBrightnes);
    }
    updateLEDs();
  }

  // Es wird überprüft, ob über die USB-Schnittstelle eine Befehl gesednet wurde um die Urzeit zu setzen.
  setRTCTimeViaConsole();

  //Es wird swichen verschienden Modi umgeschalten
  switch (Mode){
    default:
    case Mode_Uhr:
      // Ausgabe der Uhrzeit
      ModeUhr();
    break;
    case Mode_Menu:
      // Ausgabe des Menüs mit welchem verschiedene Einstellugnen vogenommen werden können
      ModeMenu();
    break;
  }
}

void ModeUhr()
{
  if ((millis() - i64_TimerTemperature) >= TIMER_TEMPERATURE) {
    i64_TimerTemperature = millis();
    checkTemperature();
  }

  if ((millis() - i64_TimerConsoleOutput) >= TIMER_CONSOLEOUTPUT) {
    i64_TimerConsoleOutput = millis();

    printRTCDateTime();

    Serial.print("\t");

    Serial.print("value_brightness =");
    Serial.print(getBrightness());

    Serial.print("\t");

    Serial.print("temperature =");
    Serial.println(getTemperature()); 
  }

  if ((millis() - i64_lastTimeRead) >= TIMER_LAST_TIME_READ) {
  i64_lastTimeRead = millis();
  rtcReadTime();
  getTime(&jahr, &monat, &tag, &stunde, &minute, &sekunde);
  calculateLEDsForClock(getTemperature());
  }
}

void ModeMenu()
{
  int onColor = ClockColor;

  // Läst den Menüpunkt blinken wenn er ausgewählt wurde
  if(MenuPointSelected == true)
  {
    if ((millis() - i64_TimerMenuPointSelected) < TIMER_SELECTEDMENUPOINT/2) {
      onColor = ClockColor;
    }
    else
    {
      onColor = COLOR_BLACK;
    }

    if ((millis() - i64_TimerMenuPointSelected) >= TIMER_SELECTEDMENUPOINT) {
      i64_TimerMenuPointSelected = millis(); 
    }
  }

  bool MenuIcon[NUM_COLUMNS][NUM_ROW];
  clearIconArray(MenuIcon);

  switch (Menu)
  {
  default:
  case Menu_verlassen:
    addPixelArrayToArray(MenuIcon, IconMenuVerlassen);
  break;
  case Menu_Farbe_einstellen:
    addPixelArrayToArray(MenuIcon, IconMenuFarbeEinstellen);
    if(onColor != COLOR_BLACK)
    {
      onColor = ClockColor;
    }
  break;
  case Menu_Helligkeit_einstellen:
    addPixelArrayToArray(MenuIcon, IconMenuHelligkeitEinstellen);
  break;
  case Menu_Temperatur_hat_Auswirkung_auf_Farbe:
    addPixelArrayToArray(MenuIcon, IconMenuTemperaturHatAuswirkungAufFarbe);
    if(ClockColorDependingFromTemperature != 0)
    {
      if(onColor != COLOR_BLACK)
      {
        onColor = COLOR_GREEN;
      }
    }
    else
    {
      if(onColor != COLOR_BLACK)
      {
        onColor = COLOR_RED;
      }
    }
  break;
  case Menu_Zeit_einstellen:
    if(MenuPointSelected == true)
    {
      if(onColor != COLOR_BLACK)
      {
        calculateLEDsForClock(20);
      }
      else
      {
        setAllLeds(COLOR_BLACK);
      }
    }
    else
    {
      addPixelArrayToArray(MenuIcon, IconMenuZeitEinstellen);
    }
  break;
  }

  if(Menu != Menu_Zeit_einstellen || MenuPointSelected == false)
  {
    setIconFullScreen(MenuIcon, onColor, COLOR_BLACK, Rotation_0_deg);
  }
}

void calculateLEDsForClock(float temperature)
{
  bool timeIcon[NUM_COLUMNS][NUM_ROW];
  clearIconArray(timeIcon);
  addPixelArrayToArray(timeIcon, IconMainPart);
  int LedStunde = stunde;

  // 5 Minuten genau Darstellung wird berechnet
  int minutenIn5MinSteps = minute / 5;
  switch (minutenIn5MinSteps) {
  case 0:
    //0 min bis 4 min
    addPixelArrayToArray(timeIcon, IconUhr);
    break;
  case 1:
    //5 min bis 9 min
    addPixelArrayToArray(timeIcon, Icon_5_min);
    addPixelArrayToArray(timeIcon, Icon_nach);
    break;
  case 2:
    //10 min bis 14 min
    addPixelArrayToArray(timeIcon, Icon_10_min);
    addPixelArrayToArray(timeIcon, Icon_nach);
    break;
  case 3:
    //15 min bis 19 min
    addPixelArrayToArray(timeIcon, Icon_viertel_min);
    addPixelArrayToArray(timeIcon, Icon_nach);
    break;
  case 4:
    //20 min bis 24 min
    addPixelArrayToArray(timeIcon, Icon_20_min);
    addPixelArrayToArray(timeIcon, Icon_nach);
    break;
  case 5:
    //25 min bis 29 min
    addPixelArrayToArray(timeIcon, Icon_5_min);
    addPixelArrayToArray(timeIcon, Icon_vor);
    addPixelArrayToArray(timeIcon, Icon_halb_min);
    LedStunde = LedStunde + 1;
    break;
  case 6:
    //30 min bis 34 min
    addPixelArrayToArray(timeIcon, Icon_halb_min);
    LedStunde = LedStunde + 1;
    break;
  case 7:
    //35 min bis 39 min
    addPixelArrayToArray(timeIcon, Icon_5_min);
    addPixelArrayToArray(timeIcon, Icon_nach);
    addPixelArrayToArray(timeIcon, Icon_halb_min);
    LedStunde = LedStunde + 1;
    break;
  case 8:
    //40 min bis 44 min
    addPixelArrayToArray(timeIcon, Icon_20_min);
    addPixelArrayToArray(timeIcon, Icon_vor);
    LedStunde = LedStunde + 1;
    break;
  case 9:
    //45 min bis 49 min
    addPixelArrayToArray(timeIcon, Icon_viertel_min);
    addPixelArrayToArray(timeIcon, Icon_vor);
    LedStunde = LedStunde + 1;
    break;
  case 10:
    //50 min bis 54 min
    addPixelArrayToArray(timeIcon, Icon_10_min);
    addPixelArrayToArray(timeIcon, Icon_vor);
    LedStunde = LedStunde + 1;
    break;
  case 11:
    //55 min bis 59 min
    addPixelArrayToArray(timeIcon, Icon_5_min);
    addPixelArrayToArray(timeIcon, Icon_vor);
    LedStunde = LedStunde + 1;
    break;
  default:
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  }

  // Minuten genau Darstellung wird berechnet
  int minutenRest = minute % 5;
  LedLeftSideUpOn     = false;
  LedLeftSideDownOn   = false;
  LedRightSideUpOn    = false;
  LedRightSideDownOn  = false;

  switch (minutenRest) {
    case 0:
    break;
    case 1:
      LedLeftSideUpOn     = true;
    break;
    case 2:
      LedLeftSideUpOn     = true;
      LedRightSideUpOn    = true;
    break;
    case 3:
      LedLeftSideUpOn     = true;
      LedLeftSideDownOn   = true;
      LedRightSideUpOn    = true;  
    break;
    case 4:
      LedLeftSideUpOn     = true;
      LedLeftSideDownOn   = true;
      LedRightSideUpOn    = true;
      LedRightSideDownOn  = true;    
    break;
  default:
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  }

  //Stunden Darstellung wird berechnet
  LedStunde = LedStunde % 12;
  if(LedStunde == 0)
  {
    LedStunde = 12;
  }

  switch (LedStunde) {
  case 1:
    if(minutenIn5MinSteps == 0)
    {
      addPixelArrayToArray(timeIcon, Icon_Ein_std);
    }
    else
    {
      addPixelArrayToArray(timeIcon, Icon_1_std);
    }
    break;
  case 2:
    addPixelArrayToArray(timeIcon, Icon_2_std);
    break;
  case 3:
    addPixelArrayToArray(timeIcon, Icon_3_std);
    break;
  case 4:
    addPixelArrayToArray(timeIcon, Icon_4_std);
    break;
  case 5:
    addPixelArrayToArray(timeIcon, Icon_5_std);
    break;
  case 6:
    addPixelArrayToArray(timeIcon, Icon_6_std);
    break;
  case 7:
    addPixelArrayToArray(timeIcon, Icon_7_std);
    break;
  case 8:
    addPixelArrayToArray(timeIcon, Icon_8_std);
    break;
  case 9:
    addPixelArrayToArray(timeIcon, Icon_9_std);
    break;
  case 10:
    addPixelArrayToArray(timeIcon, Icon_10_std);
    break;
  case 11:
    addPixelArrayToArray(timeIcon, Icon_11_std);
    break;
  case 12:
    addPixelArrayToArray(timeIcon, Icon_12_std);
    break;
  default:
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  }

  // Die Farbe für die Darstellung der Uhr wird berechnet
  int onColor = ClockColor;

  if(ClockColorDependingFromTemperature != 0)
  {
    if (temperature < 20.0) {
      onColor = 0x0000FF; // Blue
    }
    else if (temperature >= 20.0 && temperature < 21.0) {
      onColor = 0x8000FF; // 
    }
    else if (temperature >= 21.0 && temperature < 22.0) {
      onColor = 0xFF00FF; // 
    }
    else if (temperature >= 22.0 && temperature < 23.0) {
      onColor = 0xFF0080; // 
    }
    else if (temperature >= 23.0 && temperature < 24.0) {
      onColor = 0xFF0000; // 
    }
    else if (temperature >= 24.0 && temperature < 25.0) {
      onColor = 0xFF8000; // 
    }
    else if (temperature >= 25.0 && temperature < 26.0) {
      onColor = 0xFFFF00; // 
    }
    else if (temperature >= 26.0 && temperature < 27.0) {
      onColor = 0x80FF00; // 
    }
    else{
      onColor = 0x00FF00; // RED
    }
  }

  setIconFullScreen(timeIcon, onColor, COLOR_BLACK, Rotation_0_deg);

  if(LedLeftSideUpOn == true)
  {
    setLed(12, onColor);
  }
  else
  {
    setLed(12, COLOR_BLACK);
  }

  if(LedLeftSideDownOn == true)
  {
    setLed(101, onColor);
  }
  else
  {
    setLed(101, COLOR_BLACK);
  }

  if(LedRightSideUpOn == true)
  {
    setLed(0, onColor);
  }
  else
  {
    setLed(0, COLOR_BLACK);
  }

  if(LedRightSideDownOn == true)
  {
    setLed(113, onColor);
  }
  else
  {
    setLed(113, COLOR_BLACK);
  }
}

void buttonProcessing() {
  int button_down = digitalRead(PIN_BUTTON_DOWN);
  int button_up = digitalRead(PIN_BUTTON_UP);
  int button_ok = digitalRead(PIN_BUTTON_OK);
  int switch_auto = digitalRead(PIN_SWITCH_AUTO);
  int switch_manuell = digitalRead(PIN_SWITCH_MANUELL);

  if (button_down == LOW) {
        switch (Mode)
    {
      default:
      case Mode_Uhr:
        // Mach nix
      break;
      case Mode_Menu:
        if(MenuPointSelected == true)
        {
          switch (Menu)
          {
            default:
            case Menu_verlassen:
              // Mach nix
            break;
            case Menu_Farbe_einstellen:
              if(selectedClockColor > 0)
              {
                selectedClockColor--;
              }
              else
              {
                selectedClockColor = 9;
              }
              
              ClockColor = posibleClockColors[selectedClockColor];
            case Menu_Helligkeit_einstellen:
              if (u8_manuelBrightnes - 10 >= MIN_BRIGHTNESS) {
                u8_manuelBrightnes = u8_manuelBrightnes - 10;
              } else {
                u8_manuelBrightnes = MIN_BRIGHTNESS;
              }
            break;
            case Menu_Temperatur_hat_Auswirkung_auf_Farbe:
              if(ClockColorDependingFromTemperature != 0)
              {
                ClockColorDependingFromTemperature = 0;
              }
              else
              {
                ClockColorDependingFromTemperature = 1;
              }
            case Menu_Zeit_einstellen:
              if(HourAdjustmentSelected == false)
              {
                minute = minute - 1;
                if(minute < 0)
                {
                  minute = 59;
                }
              }
              else
              {
                stunde = stunde - 1;
                if(stunde < 1)
                {
                  stunde = 12;
                }
              }
            break;
          }
        }else
        {
          // vorrigen Menüpunkt auswählen
          Menu = (Menu_e)((int)Menu - 1);
          if(Menu < Menu_Zeit_einstellen)
          {
            Menu = Menu_verlassen;
          }
          if(Menu > Menu_verlassen)
          {
            Menu = Menu_Zeit_einstellen;
          }
        }
      break;
    }
  }

  if (button_up == LOW) {
    switch (Mode)
    {
      default:
      case Mode_Uhr:
        // Mach nix
      break;
      case Mode_Menu:
        if(MenuPointSelected == true)
        {
          switch (Menu)
          {
            default:
            case Menu_verlassen:
              // Mach nix
            break;
            case Menu_Farbe_einstellen:
              if(selectedClockColor < 9)
              {
                selectedClockColor++;
              }
              else
              {
                selectedClockColor = 0;
              }
              
              ClockColor = posibleClockColors[selectedClockColor];
            case Menu_Helligkeit_einstellen:
              if (u8_manuelBrightnes + 10 < 200) {
                u8_manuelBrightnes = u8_manuelBrightnes + 10;
              } else {
                u8_manuelBrightnes = 200;
              }
            break;
            case Menu_Temperatur_hat_Auswirkung_auf_Farbe:
              if(ClockColorDependingFromTemperature != 0)
              {
                ClockColorDependingFromTemperature = 0;
              }
              else
              {
                ClockColorDependingFromTemperature = 1;
              }
            case Menu_Zeit_einstellen:
              if(HourAdjustmentSelected == false)
              {
                minute = minute + 1;
                if(minute > 59)
                {
                  minute = 0;
                }
              }
              else
              {
                stunde = stunde + 1;
                if(stunde > 12)
                {
                  stunde = 1;
                }
              }
            break;
          }
        }else
        {
          // nächsten Menüpunkt auswählen
          Menu = (Menu_e)((int)Menu + 1);
          if(Menu < Menu_Zeit_einstellen)
          {
            Menu = Menu_verlassen;
          }
          if(Menu > Menu_verlassen)
          {
            Menu = Menu_Zeit_einstellen;
          }
        }
      break;
    }
  }

  if (button_ok == LOW) {
    switch (Mode)
    {
      default:
      case Mode_Uhr:
        Mode = Mode_Menu; // wechselt in den Menü Modus
      break;
      case Mode_Menu:
        switch (Menu)
        {
          default:
          case Menu_verlassen:
            Mode = Mode_Uhr;  // wechselt in den Uhr Modus
            Menu = Menu_Zeit_einstellen;
            saveConfiguration(); // Einstellugnen speichern
          break;

          case Menu_Farbe_einstellen:
          case Menu_Helligkeit_einstellen:
          case Menu_Temperatur_hat_Auswirkung_auf_Farbe:
            MenuPointSelected = !MenuPointSelected; // wählt den aktuellen Menüpunkt aus/ab
          break;

          case Menu_Zeit_einstellen:
            if(MenuPointSelected == true)
            {
              if(HourAdjustmentSelected == true)
              {
                HourAdjustmentSelected = false;
                MenuPointSelected = false;
                setTime(jahr, monat, tag, stunde, minute, sekunde); //neu eingestellte Zeit Speichern
              }
              else
              {
                HourAdjustmentSelected = true;
              }
            }
            else
            {
              MenuPointSelected = true;
            }
        }
      break;
    }
  }
}

void saveConfiguration()
{
  EEPROM.begin(1024);
  int eeAddress = 0;
  EEPROM.put(eeAddress, u8_manuelBrightnes);
  eeAddress = eeAddress + sizeof(u8_manuelBrightnes);
  EEPROM.put(eeAddress, ClockColorDependingFromTemperature);
  eeAddress = eeAddress + sizeof(ClockColorDependingFromTemperature);
  EEPROM.put(eeAddress, selectedClockColor);
  EEPROM.commit();
  EEPROM.end();

  Serial.print("Save Brightnes = ");
  Serial.print(u8_manuelBrightnes);
  Serial.print("\t");
  Serial.print("ClockColorDependingFromTemperature = ");
  Serial.print(ClockColorDependingFromTemperature);
  Serial.print("\t");
  Serial.print("selectedClockColor = ");
  Serial.println(selectedClockColor);
}

void loadConfiguration()
{
  EEPROM.begin(1024);
  rtcReadTime();
  int eeAddress = 0;
  EEPROM.get(eeAddress, u8_manuelBrightnes);
  eeAddress = eeAddress + sizeof(u8_manuelBrightnes);
  EEPROM.get(eeAddress, ClockColorDependingFromTemperature);
  eeAddress = eeAddress + sizeof(ClockColorDependingFromTemperature);
  EEPROM.get(eeAddress, selectedClockColor);
  EEPROM.end();

  ClockColor = posibleClockColors[selectedClockColor];

  Serial.print("Load Brightnes = ");
  Serial.print(u8_manuelBrightnes);
  Serial.print("\t");
  Serial.print("ClockColorDependingFromTemperature = ");
  Serial.print(ClockColorDependingFromTemperature);
  Serial.print("\t");
  Serial.print("selectedClockColor = ");
  Serial.println(selectedClockColor);
}
