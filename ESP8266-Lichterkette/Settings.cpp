//******************************************************************************
// Settings.cpp
//******************************************************************************

#include "Settings.h"
#include "Modes.h"
#include <Adafruit_NeoPixel.h>
extern s_mysettings mysetting;
extern s_mode modedef[15];


void resetToDefault()
{
#ifdef DEBUG
  Serial.println("Reset to Default...");
#endif
  mysetting.version = SETTINGS_VERSION;
  mysetting.esp_undef_Reset_count = 0;
  mysetting.lk_anzahlpixel = DEFAULT_NUMPIXELS;
  mysetting.lk_typ = DEFAULT_NEO_TYP;
  mysetting.lk_khz = DEFAULT_NEO_KHZ;
  strcpy (mysetting.systemname,DEFAULT_HOSTNAME);
  mysetting.lk_ein = true;
  mysetting.nach_strom = 2;
  mysetting.alexa_aktiv = false;
  mysetting.alexa_anzahl = 1;
  mysetting.akt_num_settings = 0;
  mysetting.main_speed = 800;
  mysetting.main_helligkeit = 80;
  mysetting.parameter[0].lmode = REGENBOGEN;
  strcpy (mysetting.parameter[0].settingtext,"Regenbogen");
  mysetting.parameter[0].color[0] = 0xff0000;
  mysetting.parameter[0].color[1] = 0xff8000;
  mysetting.parameter[0].color[2] = 0xffff00;
  mysetting.parameter[0].color[3] = 0x00ff00;
  mysetting.parameter[0].color[4] = 0x0080ff;
  mysetting.parameter[0].color[5] = 0x0000ff;
  mysetting.parameter[0].color[6] = 0x8000ff;
  mysetting.parameter[0].color[7] = 0xff0080;
  mysetting.parameter[0].color[8] = 0x8000ff;
  mysetting.parameter[0].color[9] = 0xff0080;
  mysetting.parameter[0].slider1 = 1; 
  mysetting.parameter[0].slider2 = 2;
  mysetting.parameter[0].button1 = false;
  mysetting.parameter[0].button2 = false;
  mysetting.parameter[0].main_speed = 80;
  
  mysetting.parameter[1].lmode = LAUFLICHT;
  strcpy (mysetting.parameter[1].settingtext,"Theater rechts");
  mysetting.parameter[1].color[0] = 0xff0000;
  mysetting.parameter[1].color[1] = 0x000000;
  mysetting.parameter[1].color[2] = 0x0000ff;
  mysetting.parameter[1].color[3] = 0x000000;
  mysetting.parameter[1].color[4] = 0xff0000;
  mysetting.parameter[1].color[5] = 0x0000ff;
  mysetting.parameter[1].color[6] = 0x8000ff;
  mysetting.parameter[1].color[7] = 0xff0080;
  mysetting.parameter[1].color[8] = 0x8000ff;
  mysetting.parameter[1].color[9] = 0xff0080;
  mysetting.parameter[1].slider1 = 4; 
  mysetting.parameter[1].slider2 = 2;
  mysetting.parameter[1].button1 = true;
  mysetting.parameter[1].button2 = false;
  mysetting.parameter[1].main_speed = 60;
  
  mysetting.parameter[2].lmode = LAUFLICHT;
  strcpy (mysetting.parameter[2].settingtext,"Theater links");
  mysetting.parameter[2].color[0] = 0x0000ff;
  mysetting.parameter[2].color[1] = 0x000000;
  mysetting.parameter[2].color[2] = 0xff0000;
  mysetting.parameter[2].color[3] = 0x000000;
  mysetting.parameter[2].color[4] = 0x0000ff;
  mysetting.parameter[2].color[5] = 0x0000ff;
  mysetting.parameter[2].color[6] = 0x8000ff;
  mysetting.parameter[2].color[7] = 0xff0080;
  mysetting.parameter[2].color[8] = 0x8000ff;
  mysetting.parameter[2].color[9] = 0xff0080;
  mysetting.parameter[2].slider1 = 4; 
  mysetting.parameter[2].slider2 = 2;
  mysetting.parameter[2].button1 = false;
  mysetting.parameter[2].button2 = false;
  mysetting.parameter[2].main_speed = 60;
  
  mysetting.parameter[3].lmode = LAUFLICHT;
  strcpy (mysetting.parameter[3].settingtext,"Lauflicht");
  mysetting.parameter[3].color[0] = 0xff00ff;
  mysetting.parameter[3].color[1] = 0xff80ff;
  mysetting.parameter[3].color[2] = 0x00ff00;
  mysetting.parameter[3].color[3] = 0x00ff00;
  mysetting.parameter[3].color[4] = 0x0080ff;
  mysetting.parameter[3].color[5] = 0x0000ff;
  mysetting.parameter[3].color[6] = 0x8000ff;
  mysetting.parameter[3].color[7] = 0xff0080;
  mysetting.parameter[3].color[8] = 0x8000ff;
  mysetting.parameter[3].color[9] = 0xff0080;
  mysetting.parameter[3].slider1 = 10; 
  mysetting.parameter[3].slider2 = 5;
  mysetting.parameter[3].button1 = false;
  mysetting.parameter[3].button2 = false;
  mysetting.parameter[3].main_speed = 80;
  
  mysetting.parameter[4].lmode = LAUFLICHT;
  strcpy (mysetting.parameter[4].settingtext,"Farblauflicht");
  mysetting.parameter[4].color[0] = 0xff0000;
  mysetting.parameter[4].color[1] = 0x00ff00;
  mysetting.parameter[4].color[2] = 0x0000ff;
  mysetting.parameter[4].color[3] = 0x00ff00;
  mysetting.parameter[4].color[4] = 0x0080ff;
  mysetting.parameter[4].color[5] = 0x0000ff;
  mysetting.parameter[4].color[6] = 0x8000ff;
  mysetting.parameter[4].color[7] = 0xff0080;
  mysetting.parameter[4].color[8] = 0x8000ff;
  mysetting.parameter[4].color[9] = 0xff0080;
  mysetting.parameter[4].slider1 = 3; 
  mysetting.parameter[4].slider2 = 3;
  mysetting.parameter[4].button1 = false;
  mysetting.parameter[4].button2 = false;
  mysetting.parameter[4].main_speed = 80;

  mysetting.parameter[5].lmode = FERNSEHSIMULATOR;
  strcpy (mysetting.parameter[5].settingtext,"Fernsehsimulator");
  mysetting.parameter[5].color[0] = 0x000000;
  mysetting.parameter[5].color[1] = 0x11ff00;
  mysetting.parameter[5].color[2] = 0x111111;
  mysetting.parameter[5].color[3] = 0x28ffa0;
  mysetting.parameter[5].color[4] = 0x0080ff;
  mysetting.parameter[5].color[5] = 0x0000ff;
  mysetting.parameter[5].color[6] = 0x000000;
  mysetting.parameter[5].color[7] = 0xff0080;
  mysetting.parameter[5].color[8] = 0xff6000;
  mysetting.parameter[5].color[9] = 0xff0000;
  mysetting.parameter[5].slider1 = 1; 
  mysetting.parameter[5].slider2 = 1;
  mysetting.parameter[5].button1 = false;
  mysetting.parameter[5].button2 = false;
  mysetting.parameter[5].main_speed = 50;

  mysetting.parameter[6].lmode = FEUER;
  strcpy (mysetting.parameter[6].settingtext,"Lagerfeuer");
  mysetting.parameter[6].color[0] = 0xffa000;
  mysetting.parameter[6].color[1] = 0xff6100;
  mysetting.parameter[6].color[2] = 0xff1c00;
  mysetting.parameter[6].color[3] = 0xff8000;
  mysetting.parameter[6].color[4] = 0xff4900;
  mysetting.parameter[6].color[5] = 0xff6000;
  mysetting.parameter[6].color[6] = 0xff9900;
  mysetting.parameter[6].color[7] = 0xff0000;
  mysetting.parameter[6].color[8] = 0x800000;
  mysetting.parameter[6].color[9] = 0xff3000;
  mysetting.parameter[6].slider1 = 5; 
  mysetting.parameter[6].slider2 = 1;
  mysetting.parameter[6].button1 = true;
  mysetting.parameter[6].button2 = false;
  mysetting.parameter[6].main_speed = 80;

  mysetting.parameter[7].lmode = STROBOSKOP;
  strcpy (mysetting.parameter[7].settingtext,"Puls");
  mysetting.parameter[7].color[0] = 0xff0100;
  mysetting.parameter[7].color[1] = 0xff0200;
  mysetting.parameter[7].color[2] = 0xff0300;
  mysetting.parameter[7].color[3] = 0xff0400;
  mysetting.parameter[7].color[4] = 0xff0500;
  mysetting.parameter[7].color[5] = 0xff0600;
  mysetting.parameter[7].color[6] = 0xff0700;
  mysetting.parameter[7].color[7] = 0xff0800;
  mysetting.parameter[7].color[8] = 0xff0900;
  mysetting.parameter[7].color[9] = 0xff0000;
  mysetting.parameter[7].slider1 = 1; 
  mysetting.parameter[7].slider2 = 1;
  mysetting.parameter[7].button1 = true;
  mysetting.parameter[7].button2 = false;
  mysetting.parameter[7].main_speed = 85;

  mysetting.parameter[8].lmode = STROBOSKOP;
  strcpy (mysetting.parameter[8].settingtext,"Stroboskop");
  mysetting.parameter[8].color[0] = 0xffffff;
  mysetting.parameter[8].color[1] = 0xffffff;
  mysetting.parameter[8].color[2] = 0xffffff;
  mysetting.parameter[8].color[3] = 0xffffff;
  mysetting.parameter[8].color[4] = 0xffffff;
  mysetting.parameter[8].color[5] = 0xffffff;
  mysetting.parameter[8].color[6] = 0xffffff;
  mysetting.parameter[8].color[7] = 0xffffff;
  mysetting.parameter[8].color[8] = 0xffffff;
  mysetting.parameter[8].color[9] = 0xffffff;
  mysetting.parameter[8].slider1 = 2; 
  mysetting.parameter[8].slider2 = 2;
  mysetting.parameter[8].button1 = false;
  mysetting.parameter[8].button2 = false;
  mysetting.parameter[8].main_speed = 70;

  mysetting.parameter[9].lmode = LAUFPUNKT1;
  strcpy (mysetting.parameter[9].settingtext,"Laufpunkt rechts");
  mysetting.parameter[9].color[0] = 0xffffff;
  mysetting.parameter[9].color[1] = 0xff0000;
  mysetting.parameter[9].color[2] = 0x00ff00;
  mysetting.parameter[9].color[3] = 0xffffff;
  mysetting.parameter[9].color[4] = 0x0000ff;
  mysetting.parameter[9].color[5] = 0xffffff;
  mysetting.parameter[9].color[6] = 0xffff00;
  mysetting.parameter[9].color[7] = 0xffffff;
  mysetting.parameter[9].color[8] = 0x00ffff;
  mysetting.parameter[9].color[9] = 0xffffff;
  mysetting.parameter[9].slider1 = 8; 
  mysetting.parameter[9].slider2 = 3;
  mysetting.parameter[9].button1 = false;
  mysetting.parameter[9].button2 = false;
  mysetting.parameter[9].main_speed = 85;

  mysetting.parameter[10].lmode = LAUFPUNKT1;
  strcpy (mysetting.parameter[10].settingtext,"Laufpunkt links");
  mysetting.parameter[10].color[0] = 0xffffff;
  mysetting.parameter[10].color[1] = 0xff0000;
  mysetting.parameter[10].color[2] = 0x00ff00;
  mysetting.parameter[10].color[3] = 0xffffff;
  mysetting.parameter[10].color[4] = 0x0000ff;
  mysetting.parameter[10].color[5] = 0xffffff;
  mysetting.parameter[10].color[6] = 0xffff00;
  mysetting.parameter[10].color[7] = 0xffffff;
  mysetting.parameter[10].color[8] = 0x00ffff;
  mysetting.parameter[10].color[10] = 0xffffff;
  mysetting.parameter[10].slider1 = 8; 
  mysetting.parameter[10].slider2 = 3;
  mysetting.parameter[10].button1 = true;
  mysetting.parameter[10].button2 = false;
  mysetting.parameter[10].main_speed = 85;
  
   mysetting.parameter[11].lmode = LAUFPUNKT1;
  strcpy (mysetting.parameter[11].settingtext,"Zündschnur");
  mysetting.parameter[11].color[0] = 0xff6000;
  mysetting.parameter[11].color[1] = 0xff5000;
  mysetting.parameter[11].color[2] = 0x00ff00;
  mysetting.parameter[11].color[3] = 0xffffff;
  mysetting.parameter[11].color[4] = 0x0000ff;
  mysetting.parameter[11].color[5] = 0xffffff;
  mysetting.parameter[11].color[6] = 0xffff00;
  mysetting.parameter[11].color[7] = 0xffffff;
  mysetting.parameter[11].color[8] = 0x00ffff;
  mysetting.parameter[11].color[9] = 0xffffff;
  mysetting.parameter[11].slider1 = 1; 
  mysetting.parameter[11].slider2 = 2;
  mysetting.parameter[11].button1 = false;
  mysetting.parameter[11].button2 = true;
  mysetting.parameter[11].main_speed = 50;

  mysetting.parameter[12].lmode = LAUFPUNKT1;
  strcpy (mysetting.parameter[12].settingtext,"Schlange");
  mysetting.parameter[12].color[0] = 0xffffff;
  mysetting.parameter[12].color[1] = 0xff0000;
  mysetting.parameter[12].color[2] = 0x00ff00;
  mysetting.parameter[12].color[3] = 0xffffff;
  mysetting.parameter[12].color[4] = 0x0000ff;
  mysetting.parameter[12].color[5] = 0xffffff;
  mysetting.parameter[12].color[6] = 0xffff00;
  mysetting.parameter[12].color[7] = 0xffffff;
  mysetting.parameter[12].color[8] = 0x00ffff;
  mysetting.parameter[12].color[9] = 0xffffff;
  mysetting.parameter[12].slider1 = 5; 
  mysetting.parameter[12].slider2 = 10;
  mysetting.parameter[12].button1 = false;
  mysetting.parameter[12].button2 = false;
  mysetting.parameter[12].main_speed = 80;

  mysetting.parameter[13].lmode = LAUFPUNKT2;
  strcpy (mysetting.parameter[13].settingtext,"KIT");
  mysetting.parameter[13].color[0] = 0xff2020;
  mysetting.parameter[13].color[1] = 0xff0000;
  mysetting.parameter[13].color[2] = 0x00ff00;
  mysetting.parameter[13].color[3] = 0xffffff;
  mysetting.parameter[13].color[4] = 0x0000ff;
  mysetting.parameter[13].color[5] = 0xffffff;
  mysetting.parameter[13].color[6] = 0xffff00;
  mysetting.parameter[13].color[7] = 0xffffff;
  mysetting.parameter[13].color[8] = 0x00ffff;
  mysetting.parameter[13].color[9] = 0xffffff;
  mysetting.parameter[13].slider1 = 1; 
  mysetting.parameter[13].slider2 = 5;
  mysetting.parameter[13].button1 = false;
  mysetting.parameter[13].button2 = false;
  mysetting.parameter[13].main_speed = 84;

  mysetting.parameter[14].lmode = SINUS;
  strcpy (mysetting.parameter[14].settingtext,"Sinus");
  mysetting.parameter[14].color[0] = 0xff0000;
  mysetting.parameter[14].color[1] = 0xff8000;
  mysetting.parameter[14].color[2] = 0xffff00;
  mysetting.parameter[14].color[3] = 0x00ff00;
  mysetting.parameter[14].color[4] = 0x0080ff;
  mysetting.parameter[14].color[5] = 0x0000ff;
  mysetting.parameter[14].color[6] = 0x8000ff;
  mysetting.parameter[14].color[7] = 0xff0080;
  mysetting.parameter[14].color[8] = 0x8000ff;
  mysetting.parameter[14].color[9] = 0xff0080;
  mysetting.parameter[14].slider1 = 10; 
  mysetting.parameter[14].slider2 = 5;
  mysetting.parameter[14].button1 = false;
  mysetting.parameter[14].button2 = false;
  mysetting.parameter[14].main_speed = 80;

  mysetting.parameter[15].lmode = FARBVERLAUF;
  strcpy (mysetting.parameter[15].settingtext,"Farbverlauf");
  mysetting.parameter[15].color[0] = 0xff0000;
  mysetting.parameter[15].color[1] = 0xff8000;
  mysetting.parameter[15].color[2] = 0xffff00;
  mysetting.parameter[15].color[3] = 0x00ff00;
  mysetting.parameter[15].color[4] = 0x0080ff;
  mysetting.parameter[15].color[5] = 0x0000ff;
  mysetting.parameter[15].color[6] = 0x8000ff;
  mysetting.parameter[15].color[7] = 0xff0080;
  mysetting.parameter[15].color[8] = 0x808080;
  mysetting.parameter[15].color[9] = 0x101010;
  mysetting.parameter[15].slider1 = 10; 
  mysetting.parameter[15].slider2 = 2;
  mysetting.parameter[15].button1 = false;
  mysetting.parameter[15].button2 = false;
  mysetting.parameter[15].main_speed = 80;

  for ( int setting=16; setting < NUM_SETTINGS; setting++ )
  {
     mysetting.parameter[setting] = mysetting.parameter[0];
     strcpy (mysetting.parameter[setting].settingtext,"-frei-");
  }
}

void loadFromEEPROM()
{
#ifdef DEBUG
    Serial.println("Load from EEPROM.");
#endif
    EEPROM.begin(3000);
    EEPROM.get(0, mysetting);
    if (mysetting.version != SETTINGS_VERSION)
    {
      resetToDefault();
      saveToEEPROM();
    }
#ifdef DEBUG
    Serial.printf("version: %i\n", mysetting.version);
#endif
    EEPROM.end();
}

void saveToEEPROM()
{
#ifdef DEBUG
  Serial.println("Settings saved to EEPROM.");
#endif
  EEPROM.begin(3000);
  EEPROM.put(0, mysetting);
    //EEPROM.commit();
  EEPROM.end();
}

void fillmodedef(uint16_t NUMPIXELS )
{
  uint8_t i = 0;
  
  strcpy (modedef[i].modetext,"Regenbogen");
  modedef[i].maxticker = 765;
  modedef[i].anzahlfarben = 0;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahl");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = NUMPIXELS/8;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = true;
  strcpy (modedef[i].optswitch1txt,"Richtung");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Sparks");
  i++;
  strcpy (modedef[i].modetext,"Lauflicht");
  modedef[i].maxticker = NUMPIXELS;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 2;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = true;
  strcpy (modedef[i].optslider2txt,"Anzahlpixel");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = NUMPIXELS-1;
  modedef[i].optswitch1 = true;
  strcpy (modedef[i].optswitch1txt,"Richtung");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Sparks");
  i++;
  strcpy (modedef[i].modetext,"Feuer");
  modedef[i].maxticker = 100;
  modedef[i].anzahlfarben = 6;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Intensitaet");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Sparks");
  i++;
  strcpy (modedef[i].modetext,"Laufpunkt1");
  modedef[i].maxticker = NUMPIXELS+NUMPIXELS+1;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = true;
  strcpy (modedef[i].optslider2txt,"Anzahlpixel");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = NUMPIXELS;
  modedef[i].optswitch1 = true;
  strcpy (modedef[i].optswitch1txt,"Richtung");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Zuendschnur");
  i++;
  strcpy (modedef[i].modetext,"Laufpunkt2");
  modedef[i].maxticker = NUMPIXELS+NUMPIXELS+1;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = true;
  strcpy (modedef[i].optslider2txt,"Anzahlpixel");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = NUMPIXELS;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Zuendschnur");
  i++;
  strcpy (modedef[i].modetext,"Stroboskop");
  modedef[i].maxticker = 100;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = false;
  strcpy (modedef[i].optslider1txt," ");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 2;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = true;
  strcpy (modedef[i].optswitch1txt,"weich");
  modedef[i].optswitch2 = false;
  strcpy (modedef[i].optswitch2txt," ");
  i++;
  strcpy (modedef[i].modetext,"Fernsehsimulator");
  modedef[i].maxticker = 100+(NUMPIXELS/3);
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = false;
  strcpy (modedef[i].optslider1txt," ");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 2;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = false;
  strcpy (modedef[i].optswitch2txt," ");  
  i++;
  strcpy (modedef[i].modetext,"Sinus");
  modedef[i].maxticker = 100000;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 2;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = true;
  strcpy (modedef[i].optslider2txt,"Breite");
  modedef[i].optslider2_min = 2;
  modedef[i].optslider2_max = 10;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Sparks");  
  i++;
  strcpy (modedef[i].modetext,"Farbverlauf");
  modedef[i].maxticker = 255;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 2;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = true;
  strcpy (modedef[i].optswitch2txt,"Sparks"); 
  i++;
  strcpy (modedef[i].modetext,"Blubs");
  modedef[i].maxticker = NUMPIXELS;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = true;
  strcpy (modedef[i].optslider1txt,"Anzahlfarben");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 10;
  modedef[i].optslider2 = true;
  strcpy (modedef[i].optslider2txt,"Breite");
  modedef[i].optslider2_min = 0;
  modedef[i].optslider2_max = 5;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = false;
  strcpy (modedef[i].optswitch2txt," "); 
  i++;
  strcpy (modedef[i].modetext,"dummy");
  modedef[i].maxticker = 255;
  modedef[i].anzahlfarben = 10;
  modedef[i].optslider1 = false;
  strcpy (modedef[i].optslider1txt," ");
  modedef[i].optslider1_min = 1;
  modedef[i].optslider1_max = 2;
  modedef[i].optslider2 = false;
  strcpy (modedef[i].optslider2txt," ");
  modedef[i].optslider2_min = 1;
  modedef[i].optslider2_max = 2;
  modedef[i].optswitch1 = false;
  strcpy (modedef[i].optswitch1txt," ");
  modedef[i].optswitch2 = false;
  strcpy (modedef[i].optswitch2txt," ");  
/* 
//  Modetext          Maxticker  Anzahlfarben Slider1 Slider1txt      Slider1min Slider1max    Slider2 Slider2txt    Slider2min Slider2max   switch1 switch1txt   switch2 switch2txt
 modedef[] = {
  {"Regenbogen"       ,765       ,0           ,true   ,"Anzahl"       ,1         ,NUMPIXELS/8  ,false  ,""           ,1         ,2           ,true   ,"Richtung"  ,true  ,"Sparks" },  
  {"Lauflicht"        ,NUMPIXELS ,10          ,true   ,"Anzahlfarben" ,2         ,10           ,true   ,"Anzahlpixel",1         ,NUMPIXELS-1 ,true   ,"Richtung"  ,true  ,"Sparks" }, 
  {"Feuer"            ,100       ,6           ,true   ,"Intensitaet"  ,1         ,10           ,false  ,""           ,1         ,2           ,false  ," "         ,true  ,"Sparks" }, 
  {"Laufpunkt1"       ,NUMPIXELS+NUMPIXELS+1 ,10 ,true,"Anzahlfarben" ,1         ,10           ,true   ,"Anzahlpixel",1         ,NUMPIXELS   ,true   ,"Richtung"  ,true  ,"Zuendschnur" }, 
  {"Laufpunkt2"       ,NUMPIXELS+NUMPIXELS+1 ,10 ,true,"Anzahlfarben" ,1         ,10           ,true   ,"Anzahlpixel",1         ,NUMPIXELS   ,false  ," "         ,true  ,"Zuendschnur" }, 
  {"Stroboskop"       ,100       ,10          ,false  ,"Soft"         ,1         ,16           ,false  ,""           ,1         ,2           ,true   ,"weich"     ,false ," " }, 
  {"Fernsehsimulator" ,100+(NUMPIXELS/3) ,10  ,false  ,""             ,1         ,2            ,false  ,""           ,1         ,2           ,false  ," "         ,false ," " }, 
  {"Sinus"            ,100000    ,10          ,true   ,"Anzahlfarben" ,2         ,10           ,true   ,"Breite"     ,2         ,20          ,false  ," "         ,true ,"Sparks" }, 
  {"Farbverlauf"      ,255       ,10          ,true   ,"Anzahlfarben" ,2         ,10           ,false  ,""           ,1         ,2           ,false  ," "         ,true ,"Sparks" }, 
  {"Blubs"            ,NUMPIXELS ,10          ,true   ,"Anzahlfarben" ,1         ,10           ,true   ,"Breite"     ,0         ,5           ,false  ," "         ,false ," " }, 

  {"dummy" ,           100+(NUMPIXELS/3) ,10  ,false  ,""     ,1         ,2            ,false  ,""           ,1         ,2           ,false  ,""          ,false  ,"Sparks" }, 

 };
*/
}
