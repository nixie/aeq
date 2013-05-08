#ifndef __EQ_H__
#define __EQ_H__

#include <vector>
#include <string>

const int NCH = 11;
const int NFFT = 256;

class EQ {
    private:

        // knob states
        std::vector<float> gains;

        // resolution of FFT
        float freq_shape_buf[NFFT];

        void recalc();


    public:
        static const float FREQS[NCH];
        static const char* FREQ_LABELS[NCH];

        EQ();
        void preset(std::vector<float> gains);
        void preset(int band, int gain);
        std::string dump();
        int get_nchans() { return NCH; }
        const char* get_label(int band) { return FREQ_LABELS[band]; }

        void filter(float *input, float *output, int n);
};
#endif

