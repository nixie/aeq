#include "knob.h"

void Knob::step(int direction, float step){
    value += direction * step;
    value = value < lo ? lo : value;
    value = value > hi ? hi : value;
}
