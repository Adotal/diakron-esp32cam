#pragma once
#include "../hal/IPinDriver.h"
#include "../core/motor.h"
/*
    1. Configure pins in begin()
    2. Generate STEP pulses
    3. Change direction
    Typically in a NEMA 17 with an a4988 driver
*/

class nema17 : public motor
{
private:
    uint8_t stepPin;
    uint8_t dirPin;
    uint8_t enablePin;
public:
    nema17(IPinDriver &driver, uint8_t stepPin, uint8_t dirPin, uint8_t enablePin);
    void begin();
    void step() override;
    void setDirection(bool dir);
    void enable(bool en);

};

