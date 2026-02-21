#pragma once
#include <cstdint>
/*
    Class for interacting with all real hardware pin drivers. 
    It allows the rest of the system to be independent of:
        - Direct Arduino
        - Direct ESP32
        - MCP23017
        - PCF8574
        etc.
    It depends solely on this interface.
*/
class IPinDriver
{
public:
    virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
};

