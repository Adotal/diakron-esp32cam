#pragma once
#include "../hal/IPinDriver.h"
#include "../core/sensor.h"

class InductiveSensor : public Sensor
{
private:
    uint8_t pin;
public:
    InductiveSensor(IPinDriver& drv, uint8_t sensorPin);
    void begin() override;
    bool read() override;
};
