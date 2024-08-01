#ifndef RealTimeClock_H 
#define RealTimeClock_H

// Bekanntmachungen der Funktionen welche außerhalb des Moduls benötigt werden
void rtcReadTime();
void setTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde);
void rtcWriteTime(int Jahr, int Monat, int Tag, int Stunde, int Minute, int Sekunde);
void getTime(int* Jahr, int* Monat, int* Tag, int* Stunde, int* Minute, int* Sekunde);
void setRTCTimeViaConsole();
void printRTCDateTime();

#endif