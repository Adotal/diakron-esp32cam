#include "limits.h"
#include <Arduino.h>
Limits::Limits(IPinDriver &drv, uint8_t p, bool inv) : driver(drv), pin(p), inverted(inv) {}

void Limits::begin(){
    driver.pinMode(pin, INPUT_PULLUP);
}

bool Limits::isTriggered() {
    bool state = driver.digitalRead(pin);
    return inverted ? !state : state;
}