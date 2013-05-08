/*
 * File:    main.c
 * Date:    15.03.2013 22:17
 * Author:  xferra00
 */

// TODO: On buffer size change - what if JACK uses buffers only of 64 frames

#include <vector>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <math.h>
#include <jack/jack.h>
#include "eq.h"
#include "ui.h"
#include "knob.h"
using namespace std;


/* EQ state for GUI */
// range for individual states is <0;1> - conversion for dB scale or
// percentage is done elsewhere

std::vector<Knob> knobs;
unsigned int curr_knob;

/* Equalizer instance */
EQ eq;

/* JACK stereo input/input ports */
jack_port_t *input1, *input2, *output1, *output2;
jack_nframes_t srate;
typedef jack_default_audio_sample_t sample_t;
int process (jack_nframes_t nframes, void *arg);
void jack_shutdown (void *arg);

int main(int argc, char *argv[]) {
    // init jack
    jack_client_t *client;
    const char **ports;
    knobs.assign(NCH, Knob());

    if ((client = jack_client_open ("aeq", JackNullOption, NULL)) == 0) {
        cerr << "jack server not running?\n";
        return 1;
    }
    jack_set_process_callback (client, process, 0);
    jack_on_shutdown (client, jack_shutdown, 0);

    input1 = jack_port_register (client, "input1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    input2 = jack_port_register (client, "input2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output1 = jack_port_register (client, "output1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output2 = jack_port_register (client, "output2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* tell the JACK server that we are ready to roll */
    if (jack_activate (client)) {
        cerr << "cannot activate client!\n";
        return 1;
    }


    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
        cerr << "Cannot find any physical playback ports\n";
        exit(1);
    }

    if (jack_connect (client, jack_port_name (output1), ports[0])) {
        cerr << "cannot connect output ports\n";
    }
    if (jack_connect (client, jack_port_name (output2), ports[1])) {
        cerr << "cannot connect output ports\n";
    }

    free (ports);


    // init curses
    WINDOW *p_root;
    if ( (p_root = initscr()) == NULL){
        cerr << "screen cannot be initialized, ending\n";
        return 1;
    }
    curs_set(0);		// no cursor
    clear(); cbreak(); noecho(); use_default_colors(); start_color();
    box(p_root, 0, 0);
    if (keypad(p_root, TRUE) == ERR){
        cerr << "failed to set keypad on!\n";
    }



    int c;
    int nch = eq.get_nchans();
    while(1) {
        draw_knobs(p_root, knobs, curr_knob);
        wrefresh(p_root);
        
        c = wgetch(p_root);
        switch(c)
		{	case KEY_UP:
                knobs[curr_knob].step(1);
                eq.preset(curr_knob, knobs[curr_knob].value);
                break;
			case KEY_DOWN:
                knobs[curr_knob].step(-1);
                eq.preset(curr_knob, knobs[curr_knob].value);
                break;
            case KEY_LEFT:
                curr_knob = (nch + curr_knob - 1)%nch;
                break;
            case KEY_RIGHT:
                curr_knob = (nch + curr_knob + 1)%nch;
                break;
            case KEY_RESIZE:
                // terminal has been resized ...
                werase(p_root);
                box(p_root, 0, 0);
                draw_knobs(p_root, knobs, curr_knob);
                wrefresh(p_root);

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
    cout << eq.dump();
    return EXIT_SUCCESS;
}

int process (jack_nframes_t nframes, void *arg) {
        sample_t *in_1 = (sample_t*) jack_port_get_buffer(input1, nframes);
        sample_t *in_2 = (sample_t*) jack_port_get_buffer(input2, nframes);
        sample_t *out_1 = (sample_t*) jack_port_get_buffer(output1, nframes);
        sample_t *out_2 = (sample_t*) jack_port_get_buffer(output2, nframes);
    
        // logarithmic volbar, see
        // http://www.dr-lex.be/info-stuff/volumecontrols.html
        // coeffs are for 50dB dynamic range with linearithmic cutoff
        // in the range of 0%-10%
        // TODO: factor out of this function

        /*
        float coef = expf(5.757*knobs[0]/100.0)*3.1623e-3;
        if (knobs[0] < 10){
            coef *= (knobs[0]/100) * 10;
        }
        */

        eq.filter(in_1, out_1, nframes);
        eq.filter(in_2, out_2, nframes);
        
        return 0;      
}

void jack_shutdown (void *arg) {
    endwin();
    exit (1);
}


