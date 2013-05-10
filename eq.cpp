#include <fstream>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <sndfile.h>
#include "eq.h"

using namespace std;
float *realbuf;
float *fir_coefs;
fftwf_complex *complexbuf;
fftwf_plan r2c;
fftwf_plan c2r;

// 1 Octave preffered frequency bands (ISO) for 20Hz-20kHz, base 10
// conforms: ISO R 266-1997 or equivalent ANSI S1.6-1984
// online calculator:
// http://www.zytrax.com/tech/audio/calculator.html#frequencies
const float EQ::FREQS[NCH] = {
    15.84893,
    31.62278,
    63.09573,
    125.89254,
    251.18864,
    501.18723,
    1000,
    1995.26231,
    3981.07171,
    7943.28235,
    15848.93192
};
const char* EQ::FREQ_LABELS[NCH] = {
    "16", "31.5", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
};


// Interpolate values from gains vector linearly into
// linear-spaced freq_shape_buf
void EQ::recalc(void){

    // closests neighbors from FREQS
    float left=0.0;
    float right=FREQS[0];
    int next_right=1;
    // and their gains - consider extreme points at 0 and 20k to have same
    // values as boundary points (16, 16k)
    float gain_l, gain_r, dbgain;
    gain_l = gain_r = gains[0]; 

    float f_max = 44100/2;

    for (int i=0; i < NCPLX; ++i){
        // get interpolated value in decibels

        float f = f_max * (float)i/NCPLX;

        // check if we shouldnt skip to next interval from FREQS
        while (f > right){
            if (next_right == NCH){
                right = f_max;
                gain_l = gain_r = gains[NCH-1];
                break;
            }
            gain_l = gain_r;
            gain_r = gains[next_right];
            left = right;
            right = FREQS[next_right++];
        }

        float k = (gain_r - gain_l)/(right - left);
        dbgain = gain_l + k*(f-left);
        // convert dB to amplitude gain 
        complexbuf[i][0] = expf(dbgain/20);
        complexbuf[i][1] = 0.0f;
    }

    // get impulse response
    fftwf_execute(c2r);

    // multiply with Hanning window and normalize IFFT result
    for(int i=0; i < NFIR; ++i){
        realbuf[i] = realbuf[i] * (1.0/NFIR) * 0.5*cos(2.0*M_PI*i/NFIR) + 0.5;
    }
    // swap two halves for easier computation
    memcpy(fir_coefs+NFIR/2, realbuf, sizeof(float)*NFIR/2);
    memcpy(fir_coefs, realbuf+NFIR/2, sizeof(float)*NFIR/2);
}


EQ::EQ(){
    gains.assign(NCH, 0.0);
    realbuf = (float*)fftwf_malloc(sizeof(float)*NFIR);
    fir_coefs = (float*)fftwf_malloc(sizeof(float)*NFIR);
    complexbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*NCPLX);
    c2r = fftwf_plan_dft_c2r_1d(NFIR, complexbuf, realbuf, FFTW_ESTIMATE);
    recalc();
}

EQ::~EQ(){
    fftwf_destroy_plan(c2r);
    fftwf_free(realbuf);
    fftwf_free(complexbuf);
}

void EQ::filter_buf(){
    // following code assumes fixed size (NFFT) of samples.
    // Use filter() to preprocess (zero-pad or segment) signal for variable
    // length buffers.

    // MATLAB code that works
    // X=fft(x);
    // mask = [ones(1,64)*0.5 zeros(1,128) ones(1,64)*0.5];
    // X2 = X.*mask;
    // x2 = real(ifft(X2));

    /*
    for (int i=0; i < NFFT/2+1; ++i){
        complexbuf[i][0] *=  freq_shape_buf[i];
        complexbuf[i][1] *=  freq_shape_buf[i];
    }*/

}

float fir_mem[NFIR-1];
int next_cell=0;
void fir(float *input, float *output, int n){
    // process input samples one-by-one
    for (int i=0; i < n; ++i){

        float sum=fir_coefs[0]*input[i];
        // MACs with memcells
        for (int j=0; j < NFIR-1; ++j){
            sum += fir_coefs[j+1]*fir_mem[(j+next_cell)%(NFIR-1)];
        }
        output[i] = sum;
        fir_mem[next_cell] = input[i];
        next_cell = (next_cell+1)%(NFIR-1);
    }
}


void EQ::filter(float *input, float *output, int n){
    //assert(n==NFFT || n < NFFT);    // TODO segmentation 

    fir(input, output, n);

    /*
    for (int i=0; i < n; ++i){
        realbuf[i] = input[i];
    }
    //memcpy(realbuf, input, n*sizeof(float));

    if (n < NFFT){
        // zero pad incomplete signal to have NFFT length
        memset(realbuf+NFFT-n, 0x00, (NFFT-n)*sizeof(float));
    }

    filter_buf();

    for (int i=0; i < n; ++i){
        output[i] = realbuf[i];
    }
    //memcpy(output, realbuf, n*sizeof(float));
    */
}

void EQ::preset(vector<float> gains){
    assert(this->gains.size() == gains.size());
    this->gains = gains;
    recalc();
}

void EQ::preset(int band, int gain) {
    gains[band] = gain;
    recalc();
}

// load preset from file
bool EQ::preset(const char *fname) {
    ifstream ifs (fname, std::ifstream::in);
    if (!ifs.good()){
        return false;
    }
    
    for (unsigned int i=0; i < gains.size(); ++i){
        if (!ifs.good()){
            ifs.close();
            return false;
        }
        ifs >> gains[i];
    }
    ifs.close();
    recalc();
    return true;
}

// save preset to file
bool EQ::dump(const char *fname){
    ofstream ofs (fname, std::ofstream::out);
    if (!ofs.good()){
        return false;
    }

    char buf[100];
    for (unsigned int i=0; i < gains.size(); ++i){
        snprintf(buf, 100, "%f\n", gains[i]);
        ofs << buf;
    }
    ofs.close();
    return true;
}

bool EQ::dump_shape(const char *fname){
    ofstream ofs (fname, std::ofstream::out);
    if (!ofs.good()){
        return false;
    }

    char buf[100];
    for (int i=0; i < NCPLX; ++i){
        snprintf(buf, 100, "%f\n", complexbuf[i][0]);
        ofs << buf;
    }
    ofs.close();
    return true;
}

bool EQ::dump_impulse_response(const char *fname){
    ofstream ofs (fname, std::ofstream::out);
    if (!ofs.good()){
        return false;
    }

    char buf[100];
    for (int i=0; i < NFIR; ++i){
        snprintf(buf, 100, "%f\n", fir_coefs[i]);
        ofs << buf;
    }
    ofs.close();
    return true;
}




int EQ::filter_file(char *in_fname, char *out_fname) {
    
    // we are loading/writing wav files using libsndfile
    SNDFILE *infile, *outfile;
    SF_INFO sfinfo;

    if (!(infile = sf_open (in_fname, SFM_READ, &sfinfo))) {
        cerr << "Not able to open input file " << in_fname << endl;
        cerr << sf_strerror (NULL) << endl;
        return  1;
    }

    cout << "Equalizing file " << in_fname << ":\n";
    cout << "  srate:    " << sfinfo.samplerate << endl;
    cout << "  frames:   " << sfinfo.frames << " -> " << (float)sfinfo.frames/sfinfo.samplerate << "s" << endl;
    cout << "  channels: " << sfinfo.channels << endl;

    // check sfinfo.channels, format
    //    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      //  sfinfo.samplerate = 8000;
        //sfinfo.channels = 1;
    float *input_buffer = new float[NBUF*sfinfo.channels];
    float *output_buffer = new float[NBUF*sfinfo.channels];
    if (!input_buffer || !output_buffer){
        cerr << "Not enough memory, exiting!\n";
        sf_close(infile);
        return 1;
    }

    if (! (outfile = sf_open(out_fname, SFM_WRITE, &sfinfo))) {
        cerr << "Not able to open output file " << out_fname << endl;
        cerr << sf_strerror(NULL) << endl;
        return  1;
    }

    int readcount;
    int spc; // samples per channel
    while ((readcount = sf_read_float (infile, input_buffer, NBUF*sfinfo.channels))){
        spc = readcount/sfinfo.channels;
        for (int c=0; c < sfinfo.channels; ++c){
            filter(input_buffer+spc*c, output_buffer+spc*c, spc);
        }
        sf_write_float (outfile, output_buffer, readcount) ;
    }

    /* Close input and output files. */
    sf_close (infile) ;
    sf_close (outfile) ;
    delete [] input_buffer;
    delete [] output_buffer;

    
    return 0;}



