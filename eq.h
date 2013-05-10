#ifndef __EQ_H__
#define __EQ_H__

#include <vector>
#include <string>

const int NCH = 11;
const int NFIR = 128;
const int NCPLX = NFIR/2+1;
const int NBUF = 256;

class EQ {
    private:

        // knob states
        std::vector<float> gains;

        void recalc();


    public:
        static const float FREQS[NCH];
        static const char* FREQ_LABELS[NCH];

        EQ();
        ~EQ();
        void preset(std::vector<float> gains);
        void preset(int band, int gain);
        bool preset(const char *fname);
        bool dump(const char *fname);
        bool dump_shape(const char *fname);
        bool dump_impulse_response(const char *fname);
        int get_nchans() { return NCH; }
        float get_gain(int band) { return gains[band]; }
        const char* get_label(int band) { return FREQ_LABELS[band]; }

        void filter(float *input, float *output, int n);
        void filter_buf();
        int filter_file(char *in_fname, char *out_fname);
};
#endif

