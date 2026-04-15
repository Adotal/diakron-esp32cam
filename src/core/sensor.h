#pragma once
#include "../hal/IPinDriver.h"

class Sensor
{
protected:
    IPinDriver* driver;
public:
    Sensor(IPinDriver* drv) : driver(drv) {}
    virtual void begin() = 0;
    virtual bool read() = 0;
};

