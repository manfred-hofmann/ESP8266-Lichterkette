//******************************************************************************
// Settings.h
//******************************************************************************

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>
//#include "Colors.h"
#include "Modes.h"
#include "Configuration.h"

#define SETTINGS_VERSION 4
typedef struct
  {
    int lmode;
    char settingtext[30];
    uint32_t color[10];
    int slider1;
    int slider2;
    bool button1;
    bool button2;
    int main_speed;
  } s_parameter;

typedef struct
  {
    uint8_t version;
    uint8_t esp_undef_Reset_count;
    uint16_t lk_anzahlpixel;
    uint16_t lk_typ;
    uint16_t lk_khz;
    char systemname[30];
    bool lk_ein;
    uint8_t nach_strom;
    bool alexa_aktiv;
    uint8_t alexa_anzahl;
    uint8_t akt_num_settings;
    int main_speed;
    int main_helligkeit;
    s_parameter parameter[NUM_SETTINGS+1];
  } s_mysettings;

typedef struct  
{
  char modetext[30];
  int maxticker;
  uint8_t anzahlfarben;
  bool optslider1;
  char optslider1txt[20];
  int  optslider1_min;
  int  optslider1_max;
  bool optslider2;
  char optslider2txt[20];
  int  optslider2_min;
  int  optslider2_max;
  bool optswitch1;
  char optswitch1txt[20];
  bool optswitch2;
  char optswitch2txt[20];
} s_mode;

void resetToDefault();
void saveToEEPROM();
void loadFromEEPROM();
void fillmodedef(uint16_t NUMPIXELS );

#endif
