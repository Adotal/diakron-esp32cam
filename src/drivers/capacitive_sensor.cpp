#include "capacitive_sensor.h"
#include <Arduino.h>

CapacitiveSensor::CapacitiveSensor(IPinDriver& drv, uint8_t sensorPin) : Sensor(&drv), pin(sensorPin){
     this->driver = &drv;
}

void CapacitiveSensor::begin()
{
    driver->pinMode(pin, INPUT_PULLUP); // Intenta con INPUT normal
}

bool CapacitiveSensor::read()
{
    // Read the pin value and return true if it's LOW (activated)
    return driver->digitalRead(pin) == LOW;
}