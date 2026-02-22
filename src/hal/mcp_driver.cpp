#include "mcp_driver.h"

// Constructor for working with the Adafruit_MCP23X17 object
mcp_driver::mcp_driver(Adafruit_MCP23X17 &expander) : mcp(expander) {}

// Implementation of IPinDriver methods using the Adafruit_MCP23X17 library
void mcp_driver::pinMode(uint8_t pin, uint8_t mode) {
    mcp.pinMode(pin, mode);
}

void mcp_driver::digitalWrite(uint8_t pin, uint8_t value) {
    mcp.digitalWrite(pin, value);
}

int mcp_driver::digitalRead(uint8_t pin) {
    return mcp.digitalRead(pin);
}