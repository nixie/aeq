#ifndef __KNOB_H__
#define __KNOB_H__

#include <string>

class Knob {
    public: 
        float value;
        float lo;
        float hi;
        float default_value;
        std::string unit;

        Knob() : value(0.0), lo(-24.0), hi(24.0),
                 default_value(0.0), unit("dB") {};
        void reset(){ value = default_value; }
        void step(int direction, float step=3.0f);
};

#endif

