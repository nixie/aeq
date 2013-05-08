#include <assert.h>
#include "eq.h"

using namespace std;

EQ::EQ(){
    gains.assign(NCH, 0.0f);
}

// Interpolate values from gains vector linearly into
// linear-spaced freq_shape_buf
void EQ::recalc(void){
}

// Spectral domain filtering
void EQ::filter(float *input, float *output, int n){
    assert(n==NFFT);    // TODO variable lenght buffers

    // MATLAB code that works
    // X=fft(x);
    // mask = [ones(1,64)*0.5 zeros(1,128) ones(1,64)*0.5];
    // X2 = X.*mask;
    // x2 = real(ifft(X2));



    for (int i=0; i < n; ++i){
        output[i] = input[i];
    }
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

std::string EQ::dump(){
    char buf[100];
    std::string s = "Freq. \tGain\n";
    for (unsigned int i=0; i < gains.size(); ++i){
        snprintf(buf, 100, "%s \t%f\n", FREQ_LABELS[i], gains[i]);
        s += buf;
    }
    return s;
}



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
