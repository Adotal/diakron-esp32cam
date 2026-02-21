#pragma once
#include "IPinDriver.h"

/*
    Implement IPinDriver using real-world features such as:
    pinMode() and digitalWrite() from ESP32.
    It is used to control normal pins of the Arduino or ESP32.
*/

class gpio_driver: public IPinDriver
{
public:
    void pinMode(uint8_t pin, uint8_t mode) override;
    void digitalWrite(uint8_t pin, uint8_t value) override;
};

