#include "nema17.h"
#include <Arduino.h>
#include "../config/defaults.h"

unsigned long pulseInterval = 0;
unsigned long previousMicros = 0;

nema17::nema17(IPinDriver &driver, uint8_t stepPin, uint8_t dirPin, uint8_t enablePin) : motor(&driver){
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->enablePin = enablePin;
}

void nema17::begin(){
    driver->pinMode(stepPin, OUTPUT);
    driver->pinMode(dirPin, OUTPUT);
    driver->pinMode(enablePin, OUTPUT);
    driver->digitalWrite(enablePin, LOW);
}

void nema17::step(){
    driver->digitalWrite(stepPin, HIGH);
    delayMicroseconds(5);
    driver->digitalWrite(stepPin, LOW);
}

void nema17::setDirection(bool dir){
    driver->digitalWrite(dirPin, dir ? HIGH : LOW);
}

void nema17::enable(bool en){
    driver->digitalWrite(enablePin, en ? LOW : HIGH);
}
