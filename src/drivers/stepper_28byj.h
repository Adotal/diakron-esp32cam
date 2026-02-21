#pragma once
#include "../core/motor.h"
#include "../hal/IPinDriver.h"

class stepper_28byj : public motor
{
private:
    uint8_t pins[4];
    const uint8_t stepSequence[8][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };
    int stepIndex;
    bool direction; // true for forward, false for backward
public:
    stepper_28byj(IPinDriver &driver, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
    void begin();
    void step() override;
    void setDirection(bool dir);
};