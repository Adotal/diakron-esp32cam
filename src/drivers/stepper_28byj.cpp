#include "stepper_28byj.h"
#include <Arduino.h>

stepper_28byj::stepper_28byj(IPinDriver &driver, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4) : motor(&driver) {
    pins[0] = pin1;
    pins[1] = pin2;
    pins[2] = pin3;
    pins[3] = pin4;
    stepIndex = 0;
}

void stepper_28byj::begin() {
    for (int i = 0; i < 4; i++) {
        driver->pinMode(pins[i], OUTPUT);
        driver->digitalWrite(pins[i], LOW);
    }
}

void stepper_28byj::step() {
    for (int i = 0; i < 4; i++) {
        driver->digitalWrite(pins[i], stepSequence[stepIndex][i]);
    }
    stepIndex = (stepIndex + 1) % 8; // Move to the next step in the sequence
}