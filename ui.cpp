#include "ui.h"
#include "eq.h"

void draw_slider(WINDOW *win, int x, int y, int w, int h,
        const char *label, Knob &knob, int selected){
    
    // draw box
    mvwvline(win, y+2, x, ACS_VLINE, h-3);
    mvwvline(win, y+2, x+2, ACS_VLINE, h-3);
    mvwvline(win, y+2, x+1, ' ', h-3);
    mvwaddch(win, y+2, x, ACS_ULCORNER);
    mvwaddch(win, y+2, x+2, ACS_URCORNER);
    mvwaddch(win, y+h-2, x, ACS_LLCORNER);
    mvwaddch(win, y+h-2, x+2, ACS_LRCORNER);

    if (selected){
        wattron(win, A_BOLD);
    }

    char buf[10]="";
    snprintf(buf, 10, "%-3f", knob.value);

    // draw text
    mvwaddnstr(win, y, x, buf, 3);
    mvwaddnstr(win, y+1, x, knob.unit.c_str(), 3);
    mvwaddnstr(win, y+h-1, x, label, 4);
    if (selected){
        wattroff(win, A_BOLD);
    }

    int percentage = (int)(100*(knob.value - knob.lo)/(knob.hi-knob.lo));
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0; 
    // draw knob
    int fill = (h-6)*percentage/100.0;
    wattron(win, A_STANDOUT);
    mvwaddstr(win, y+h-3-fill, x, " - ");
    wattroff(win, A_STANDOUT);
}

void draw_knobs(WINDOW *w, std::vector<Knob> knobs, unsigned int curr_knob){
    unsigned int cols,rows;
    getmaxyx(w, rows, cols);

    for (unsigned int i=0; i < knobs.size(); i++){
        // EQ channels
        draw_slider(w, i*(cols/knobs.size())+2, 1, 4, rows-2, EQ::FREQ_LABELS[i], knobs[i], i == curr_knob);
    }
}


