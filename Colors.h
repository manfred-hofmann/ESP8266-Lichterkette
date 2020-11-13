/******************************************************************************
Colors.h
******************************************************************************/

#ifndef COLORS_H
#define COLORS_H

#include "Configuration.h"

struct color_s
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

//String s_color[10] = { "#ff0000","#ff8000","#ffff00","#00ff00", "#0080ff", "#0000ff", "#8000ff", "#ff0080"};

color_s w_color[10];
//color_s mysetting_w_color[NUM_SETTINGS][9];

// farben
#define GRUEN 0x007f00
#define WEISS 0x7f7f7f
#define ROT   0x7f0000
#define BLAU  0x00007f
#define LILA  0x7f007f
#endif
