//******************************************************************************
// Modes.h
//******************************************************************************

#ifndef MODES_H
#define MODES_H
#include <Arduino.h>
#include "Configuration.h"
#include "Settings.h"


enum eMode
{
  REGENBOGEN,
  LAUFLICHT,
  FEUER,
  LAUFPUNKT1,
  LAUFPUNKT2,
  STROBOSKOP,
  FERNSEHSIMULATOR,
  SINUS,
  FARBVERLAUF,
  BLUBS,
  MODE_COUNT = BLUBS,
};


enum nachStromausfall
{
  AUS,
  EIN,
  WIE_VORHER,
};

#endif
