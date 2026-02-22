#include "nema17.h"
#include <Arduino.h>
#include "../config/defaults.h"

unsigned long pulseInterval = 0;
unsigned long previousMicros = 0;

nema17::nema17(IPinDriver &driver, uint8_t stepPin, uint8_t dirPin, uint8_t enablePin) : motor(&driver){
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->enablePin = enablePin;
    this->lastStepTime = 0;
    this->stepInterval = 0;
    this->direction = true;
    this->position = 0;
}

void nema17::begin(){
    driver->pinMode(stepPin, OUTPUT);
    driver->pinMode(dirPin, OUTPUT);
    driver->pinMode(enablePin, OUTPUT);
    driver->digitalWrite(enablePin, LOW);
}

void nema17::step() {
    driver->digitalWrite(stepPin, HIGH);
    // (A4988/DRV8825) Minimum 1 microsecond in high state and another microsecond in low state
    delayMicroseconds(2); 
    driver->digitalWrite(stepPin, LOW);
}

void nema17::setDirection(bool dir){
    driver->digitalWrite(dirPin, dir ? HIGH : LOW);
}

void nema17::enable(bool en){
    driver->digitalWrite(enablePin, en ? LOW : HIGH);
}

void nema17::setSpeed(long rpm) {
    if (rpm <= 0) return;
    // (60s * 1,000,000us) / (step_per_lap * rpm)
    // For a typical NEMA 17 with 200 steps per revolution (1.8° step angle)
    this->stepInterval = 60000000L / (STEPS_PER_REVOLUTION * rpm); 
}

void nema17::update() {
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

long nema17::getPosition() {
    return position;
}

void nema17::resetPosition(long pos) {
    position = pos;
}

long nema17::getMaxRPM() {
    return MAX_RPM_NEMA17;
}

long nema17::getDefaultRPM() {
    return DEFAULT_RPM_NEMA17;
}