#pragma once

#include "IPinDriver.h"
#include <Adafruit_MCP23X17.h>

/*
    Class to implement IPinDriver using an MCP23017
    It is used to control pins of the MCP23017 expander.
    Example usage:
    IPinDriver* driver = &mcp;
*/

class mcp_driver : public IPinDriver{
private:
    Adafruit_MCP23X17 &mcp;
public:
    mcp_driver(Adafruit_MCP23X17 &expander);

    void pinMode(uint8_t pin, uint8_t mode) override;
    void digitalWrite(uint8_t pin, uint8_t value) override;
};