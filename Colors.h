/******************************************************************************
Colors.h
******************************************************************************/

#ifndef COLORS_H
#define COLORS_H

struct color_s
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

String s_color[9] = { "#ff0000","#ff8000","#ffff00","#00ff00", "#0080ff", "#0000ff", "#8000ff", "#ff0080"};

color_s w_color[9];

#endif
