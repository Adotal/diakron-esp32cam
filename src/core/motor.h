#pragma once
#include "../hal/IPinDriver.h"

/*
    Class to define what is common to all engines
    A base class that represents any engine.
    Unifies common behavior.
    - All motors:
    - Need to initialize pins
    - Need to take steps
    - Use a pin driver
*/

class motor{
protected:
    IPinDriver* driver;
public:
    motor(IPinDriver* drv) : driver(drv) {}
    virtual void begin() = 0;
    virtual void step() = 0;
};

