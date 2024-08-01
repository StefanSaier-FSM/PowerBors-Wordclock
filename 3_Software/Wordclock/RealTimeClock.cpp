// Header File einbinden
#include "RealTimeClock.h"

//Bibliotheke welche in diesem File benötigt werden einbinden.
#include <Wire.h>
#include <stdint.h>
#include <HardwareSerial.h>

// Definitionen Welche nur innerhalb des innerhalb des Moduls benötigt werden anlegen
#define RTC_I2C_ADDRESS 0x68
#define byte uint8_t

// Struckturen Welche nur innerhalb des Moduls benötigt werden definieren
struct timeParameters {
  uint8_t ss;
  uint8_t mm;
  uint8_t hh;
  uint8_t dy;
  uint8_t d;
  uint8_t m;
  uint8_t y;
};

//Variabeln welche nur innerhalb des Moduls benötigt werden anlegen
int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
String daysInWeek[7] = { "Montag", "Dienstag", "Mittowch", "Donnerstag", "Freitag", "Samstag", "Sonntag" };
String monthInYear[12] = { "Jan", "Feb", "Mae", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" };
String outputFormat = "%s, %02d.%s %04d %02d:%02d:02d Uhr";
timeParameters myTime;
int i_jahr;
int i_monat;
int i_tag;
int i_stunde;
int i_minute;
int i_sekunde;
int i_wochentag;

// Bekanntmachungen der Funktionen welche nur innerhalb des Moduls benötigt werden
static uint8_t convertValueIN(uint8_t value);
static uint8_t convertValueOUT(uint8_t value);
int getIntFromString(char *stringWithInt, byte num);
byte decToBcd(byte val);
byte bcdToDec(byte val);
bool checkDateTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde);
int getIntFromString(char *stringWithInt, byte num);

// Umsetzung der Funktionen
void rtcReadTime() {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  i_sekunde = bcdToDec(Wire.read() & 0x7f);
  i_minute = bcdToDec(Wire.read());
  i_stunde = bcdToDec(Wire.read() & 0x3f);
  bcdToDec(Wire.read());
  i_tag = bcdToDec(Wire.read());
  i_monat = bcdToDec(Wire.read());
  i_jahr = bcdToDec(Wire.read()) + 2000;
}

//Funktion zum schreiben / setzen der Uhrzeit.
void rtcWriteTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde) {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);  // Der Wert 0 aktiviert das RTC Modul.
  Wire.write(decToBcd(Sekunde));
  Wire.write(decToBcd(Minute));
  Wire.write(decToBcd(Stunde));
  Wire.write(decToBcd(0));  // Wochentag unberücksichtigt
  Wire.write(decToBcd(Tag));
  Wire.write(decToBcd(Monat));
  Wire.write(decToBcd(Jahr - 2000));
  Wire.endTransmission();
}

//Funktion zum setzen der Uhrzeit.
void setTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde) {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);  // Der Wert 0 aktiviert das RTC Modul.
  Wire.write(decToBcd(Sekunde));
  Wire.write(decToBcd(Minute));
  Wire.write(decToBcd(Stunde));
  Wire.write(decToBcd(0));  // Wochentag unberücksichtigt
  Wire.write(decToBcd(Tag));
  Wire.write(decToBcd(Monat));
  Wire.write(decToBcd(Jahr - 2000));
  Wire.endTransmission();
}

//Berechnet den Tag der Woche aus dem übergebenen Datumswerten.
byte calcDayOfWeek(int Jahr, byte Monat, byte Tag) {
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  Jahr -= Monat < 3;
  return ((Jahr + Jahr / 4 - Jahr / 100 + Jahr / 400 + t[Monat - 1] + Tag) % 7);
}

//Convertiert Dezimalzeichen in binäre Zeichen.
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

//Convertiert binäre Zeichen in Dezimal Zeichen.
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}

void getTime(int* Jahr, int* Monat, int* Tag, int* Stunde, int* Minute, int* Sekunde) {
  *Jahr = i_jahr;
  *Monat = i_monat;
  *Tag = i_tag;
  *Stunde = i_stunde;
  *Minute = i_minute;
  *Sekunde = i_sekunde;
}

bool checkDateTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde) {  //Prüft das eingegebene Datum auf korrektheit.
  bool result = false;
  if (Jahr > 2000) {
    result = true;
  } else {
    return false;
  }

  if (Jahr % 400 == 0 || (Jahr % 100 != 0 && Jahr % 4 == 0)) {  // Schaltjahr prüfen. Wenn es ein Schaltjahr ist dann den Wert 29 in das Array an der Stelle 1 für den Monat Februar schreiben.
    daysInMonth[1] = 29;
  }

  if (Monat < 13) {  // Monat muss kleiner als 13 sein.
    if (Tag <= daysInMonth[Monat - 1]) {
      result = true;
    }
  } else {
    return false;
  }

  if (Stunde < 24 && Minute < 60 && Sekunde < 60 && Stunde >= 0 && Minute >= 0 && Sekunde >= 0) {  //Wert für Stunde muss zwischen 0 und 24 liegen. Minuten und Sekunden zwischen 0 und 59.
    result = true;
  } else {
    return false;
  }

  return result;
}

static uint8_t convertValueIN(uint8_t value) {
  uint8_t convertedVal = 0;
  convertedVal = ((value >> 4) * 10) + (value & 0x0F);

  return convertedVal;
}

static uint8_t convertValueOUT(uint8_t value) {
  uint8_t convertedVal = 0;
  convertedVal = ((value / 10) << 4) + (value % 10);

  return convertedVal;
}

//Ließt einen String und liefert einen Integer Wert von einer
//definierten Stelle (byte num) des Stringwertes.
int getIntFromString(char *stringWithInt, byte num) {
  char *tail;
  while (num > 0) {
    num--;
    //Bei Kommanseparierten Listen werden die Kommata
    //übersprungen und geben damit die Unterteilung des Strings an.
    while ((!isdigit(*stringWithInt)) && (*stringWithInt != 0)) {
      stringWithInt++;
    }
    tail = stringWithInt;
    //Schleife solange ausführen bis eine Zahl gefunden wurde
    //welche größer 0 ist.
    while ((isdigit(*tail)) && (*tail != 0)) {
      tail++;
    }

    if (num > 0) {
      stringWithInt = tail;
    }
  }
  return (strtol(stringWithInt, &tail, 10));
}

//Manuelles setzen der Uhrzeit über den Seriellen Monitor der IDE.
void setRTCTimeViaConsole() {
  char linebuf[30];
  byte counter;
  if (Serial.available()) {
    delay(100);                           // Warte auf das Eintreffen aller Zeichen vom seriellen Monitor
    memset(linebuf, 0, sizeof(linebuf));  // Zeilenpuffer löschen
    counter = 0;                          // Zähler auf Null
    while (Serial.available()) {
      linebuf[counter] = Serial.read();              // Zeichen in den Zeilenpuffer einfügen
      if (counter < sizeof(linebuf) - 1) counter++;  // Zeichenzähler erhöhen
    }
    // Wenn in der gelesenen Zeile das Wort 'set' vorkommt dann...
    //(Hier muss man bedenken das die Suche nach 'set' auch nach x Zeichen ein positives Ergebnis liefern wird, zbsp. 123set 09.01.2016 12:00:00)
    if (strstr(linebuf, "set") == linebuf) {
      i_tag = getIntFromString(linebuf, 1);
      i_monat = getIntFromString(linebuf, 2);
      i_jahr = getIntFromString(linebuf, 3);
      i_stunde = getIntFromString(linebuf, 4);
      i_minute = getIntFromString(linebuf, 5);
      i_sekunde = getIntFromString(linebuf, 6);
    } else {
      Serial.println("Befehl unbekannt.");
      return;
    }

    if (!checkDateTime(i_jahr, i_monat, i_tag, i_stunde, i_minute, i_sekunde)) {  // Ausgelesene Werte einer groben Plausibilitätsprüfung unterziehen.
      Serial.println(linebuf);
      Serial.println("Fehlerhafte Zeitangabe im 'set' Befehl");
      Serial.println("Beispiel: set 28.08.2013 10:54");
      return;
    }
    rtcWriteTime(i_jahr, i_monat, i_tag, i_stunde, i_minute, i_sekunde);
    Serial.println("Zeit und Datum wurden auf neue Werte gesetzt.");
  }
}

void printRTCDateTime() {
  char linebuf[60];
  //int dOW = calcDayOfWeek(jahr, monat, tag);
  //String wochentagC = daysInWeek[dOW];
  String monatC = monthInYear[i_monat - 1];
  String result = "";
  result += i_tag;
  result += ".";
  result += monatC;
  result += " ";
  result += i_jahr;
  result += " ";
  if (i_stunde < 10) { result += "0"; }
  result += i_stunde;
  result += ":";
  if (i_minute < 10) { result += "0"; }
  result += i_minute;
  result += ":";
  if (i_sekunde < 10) { result += "0"; }
  result += i_sekunde;
  Serial.print(result);
}