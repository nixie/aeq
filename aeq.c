/*
 * File:    main.c
 * Date:    15.03.2013 22:17
 * Author:  xferra00
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <math.h>
#include <jack/jack.h>

typedef struct vbar {
    int level;
    int max, min;
    char *unit;
    char *name;
} vbar_t;

/* EQ settings */
#define NBARS 11
vbar_t eqstate[NBARS];
int selected_vbar=0;

/* JACK settings */
jack_port_t *output_port_1;
jack_port_t *output_port_2;
jack_port_t *input_port_1;
jack_port_t *input_port_2;

typedef jack_default_audio_sample_t sample_t;
jack_nframes_t srate;

int process (jack_nframes_t nframes, void *arg) {
        jack_default_audio_sample_t *in_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port_1, nframes);
        jack_default_audio_sample_t *in_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port_2, nframes);
        jack_default_audio_sample_t *out_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_1, nframes);
        jack_default_audio_sample_t *out_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_2, nframes);

        // http://www.dr-lex.be/info-stuff/volumecontrols.html
        float tmp = (6.908*eqstate[0].level/100.0)*(6.908*eqstate[0].level/100.0);
        float coef = 0.001*tmp*tmp;
        for (int i=0; i < nframes; i++){
            out_1[i] = in_1[i] * coef;
            out_2[i] = in_2[i] * coef;
        }
        
        return 0;      
}
void jack_shutdown (void *arg) { exit (1); }


void draw_chan(WINDOW *win, int x, int y, int w, int h,
        char *low1, char *low2, char *up1, char *up2,
        int percentage, int selected){
    
    // draw box
    mvwvline(win, y+2, x, ACS_VLINE, h-4);
    mvwvline(win, y+2, x+2, ACS_VLINE, h-4);
    mvwvline(win, y+2, x+1, ' ', h-4);
    mvwaddch(win, y+2, x, ACS_ULCORNER);
    mvwaddch(win, y+2, x+2, ACS_URCORNER);
    mvwaddch(win, y+h-3, x, ACS_LLCORNER);
    mvwaddch(win, y+h-3, x+2, ACS_LRCORNER);

    if (selected){
        wattron(win, A_BOLD);
    }
    // draw labels
    mvwaddnstr(win, y, x, up1, 3);
    mvwaddnstr(win, y+1, x, up2, 3);
    mvwaddnstr(win, y+h-2, x, low1, 3);
    mvwaddnstr(win, y+h-1, x, low2, 3);
    if (selected){
        wattroff(win, A_BOLD);
    }

    // draw fill
    if (percentage > 100){
        percentage = 100;
    }
    // draw slider
    int fill = (h-7)*percentage/100.0;
    wattron(win, A_STANDOUT);
    mvwaddstr(win, y+h-4-fill, x, " - ");
    wattroff(win, A_STANDOUT);
}

void draw_eq(WINDOW *w){
    unsigned int cols,rows;
    getmaxyx(w, rows, cols);

    char buf[10];
    for (int i=0; i < NBARS; i++){
        if (i==0){
            // preamp vbar
            snprintf(buf, 10, "%-3d", (int)(eqstate[i].level*0.6));
            draw_chan(w, i*6+2, 1, 4, rows-5, "PRE", "AMP", buf, "dB ", eqstate[i].level, i == selected_vbar);
        }else{
            // EQ channels
            snprintf(buf, 10, "%-3d", eqstate[i].level-50);
            draw_chan(w, i*6+2, 1, 4, rows-5, "1.3", "kHz", buf, "dB ", eqstate[i].level, i == selected_vbar);
        }
    }
}

int main(int argc, char *argv[]) {
    // init jack
    jack_client_t *client;
    const char **ports;

    if ((client = jack_client_open ("aeq", JackNullOption, NULL)) == 0) {
        fprintf (stderr, "jack server not running?\n");
        return 1;
    }
    jack_set_process_callback (client, process, 0);
    jack_on_shutdown (client, jack_shutdown, 0);

    input_port_1 = jack_port_register (client, "input_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    input_port_2 = jack_port_register (client, "input_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port_1 = jack_port_register (client, "output_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_port_2 = jack_port_register (client, "output_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* tell the JACK server that we are ready to roll */
    if (jack_activate (client)) {
        fprintf (stderr, "cannot activate client");
        return 1;
    }


    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
        fprintf(stderr, "Cannot find any physical playback ports\n");
        exit(1);
    }

    if (jack_connect (client, jack_port_name (output_port_1), ports[0])) {
        fprintf (stderr, "cannot connect output ports\n");
    }
    if (jack_connect (client, jack_port_name (output_port_2), ports[1])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    free (ports);


    // init curses
    WINDOW *p_root;
    if ( (p_root = initscr()) == NULL){
        fprintf(stderr, "screen cannot be initialized, ending\n");
        return 1;
    }
    curs_set(0);		// no cursor
    clear(); cbreak(); noecho(); use_default_colors(); start_color();
    box(p_root, 0, 0);
    if (keypad(p_root, TRUE) == ERR){ printf("FAILED to set keypad on!\n"); }



    int c;
    while(1) {
        draw_eq(p_root);
        wrefresh(p_root);
        
        c = wgetch(p_root);
        switch(c)
		{	case KEY_UP:
                eqstate[selected_vbar].level+= 5;
                if (eqstate[selected_vbar].level >100){
                    eqstate[selected_vbar].level = 100;
                }
                break;
			case KEY_DOWN:
                eqstate[selected_vbar].level-= 5;
                if (eqstate[selected_vbar].level < 0){
                    eqstate[selected_vbar].level = 0;
                }
                break;
            case KEY_LEFT:
                selected_vbar = (NBARS + selected_vbar - 1)%NBARS;
                break;
            case KEY_RIGHT:
                selected_vbar = (NBARS + selected_vbar + 1)%NBARS;
                break;
			case 10:    // Enter
			default:
				break;
		}
        if (c=='q'){
            break;
        }
	}	


    jack_client_close(client);

    endwin();
    return EXIT_SUCCESS;
}

