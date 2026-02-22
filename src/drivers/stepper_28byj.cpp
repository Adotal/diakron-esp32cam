#include "stepper_28byj.h"
#include <Arduino.h>

stepper_28byj::stepper_28byj(IPinDriver &driver, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4) : motor(&driver) {
    pins[0] = pin1;
    pins[1] = pin2;
    pins[2] = pin3;
    pins[3] = pin4;
    stepIndex = 0;
    direction = true;
    position = 0;
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
    if(direction){
        stepIndex = (stepIndex + 1) % 8; // Move to the next step in the sequence
        position++;
    }
    else{
        stepIndex = (stepIndex + 7) % 8; // Move to the previous step in the sequence
        position--;
    }
}

void stepper_28byj::update() {
    unsigned long currentTime = micros();
    
    // Timer
    if (currentTime - lastStepTime >= stepInterval) {
        lastStepTime = currentTime;
        step(); // Execute a step
        if(direction)
            position++;
        else
            position--;
    }
}

void stepper_28byj::setSpeed(long rpm) {
    // This function can be implemented to set the delay between steps based on the desired RPM
    // For example, you can calculate the delay as follows:
    // delay = (60 * 1000) / (steps_per_revolution * rpm);
    // where steps_per_revolution is the number of steps for a full rotation of the motor

}


void stepper_28byj::setDirection(bool dir) {
    direction = dir ? 1 : 0;
}

long stepper_28byj::getPosition() {
    return position;
}

void stepper_28byj::resetPosition(long pos) {
    position = pos;
}

long stepper_28byj::getMaxRPM() {
    return MAX_RPM_28BYJ;
}

long stepper_28byj::getDefaultRPM() {
    return DEFAULT_RPM_28BYJ;
}