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

/* 
    - Example usage mcp_driver:
        Adafruit_MCP23X17 mcp(0x20);
        mcp_driver i2cExpander(mcp);
        nema17 motor1(&i2cExpander, 2, 5, 10);
        stteper_28byj motor2(&i2cExpander, 5,6,7,8);

    - Example usage direct pin control:
        gpio_driver gpioDrv;
        nema17 motor1(&gpioDrv, 2, 5, 10);
        stepper_28byj motor2(&gpioDrv, 5,6,7,8);
*/

class motor{
protected:
    IPinDriver* driver;
public:
    motor(IPinDriver* drv) : driver(drv) {}
    virtual void begin() = 0;
    virtual void step() = 0;
};

