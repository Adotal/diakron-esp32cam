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
    unsigned long lastStepTime;  // Último micros() donde se dio un paso
    unsigned long stepInterval;  // Microsegundos entre pasos
    int stepIndex;
    bool direction; // true for forward, false for backward
    long position;

    long const DEFAULT_RPM_28BYJ = 15;
    long const MAX_RPM_28BYJ = 15;
    long const STEPS_PER_REVOLUTION = 2048; // 28BYJ-48 has 2048 steps per revolution in full-step mode
public:
    stepper_28byj(IPinDriver &driver, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
    void begin();
    void step() override;
    void enable(bool en) override{} // motor does not have an enable pin, so this function can be left empty
    void update() override; 
    void setSpeed(long rpm);
    void setDirection(bool dir);
    long getPosition() override;
    void resetPosition(long pos = 0) override;
    long getMaxRPM() override;
    long getDefaultRPM() override;
};