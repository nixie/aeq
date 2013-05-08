#ifndef __UI_H__
#define __UI_H__

#include <vector>
#include <string.h>
#include <curses.h>
#include "knob.h"

void draw_knobs(WINDOW *w, std::vector<Knob> knobs, unsigned int curr_knob);
void draw_slider(WINDOW *win, int x, int y, int w, int h,
        const char *low1, char *up1, int percentage, int selected);


#endif

