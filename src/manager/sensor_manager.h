#pragma once
#include "../core/sensor.h"
#include "../config/defaults.h"
#include "../communication/logger.h"
/*
    This class manages all the sensors in the system. It allows adding new sensors, reading them by ID, and reading all sensors at once.
    The manager stores an array of SensorEntry, which contains an ID and a pointer to a Sensor object. 
    The readSensor method uses polymorphism to call the correct read() method of each sensor type without needing to know the specific type.
*/
class SensorManager
{
private:
    struct SensorEntry
    {
        char id;
        Sensor* sensor;
    };

    SensorEntry sensors[MAX_SENSORS_PER_MANAGER];
    uint8_t count;
    
public:
    SensorManager();
    bool addSensor(char id, Sensor* sensor);
    Sensor* getSensor(char id);
    bool readSensor(char id);
    void readAll();
};