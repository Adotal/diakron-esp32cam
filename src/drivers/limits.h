#pragma once
#include "../hal/IPinDriver.h"
class Limits
{
private:
    IPinDriver &driver;
    uint8_t pin;
    bool inverted;
public:
    Limits(IPinDriver &drv, uint8_t p, bool inv = false);
    void begin();
    bool isTriggered();
};
