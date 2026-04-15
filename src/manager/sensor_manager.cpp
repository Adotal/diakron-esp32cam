#include "sensor_manager.h"

SensorManager::SensorManager() : count(0) {}

// Recibe directamente un Sensor* en lugar de void*
bool SensorManager::addSensor(char id, Sensor *sensor)
{
    if (count >= MAX_SENSORS_PER_MANAGER)
        return false;

    sensors[count++] = {id, sensor};
    return true;
}

// Retorna un Sensor* en lugar de void*
Sensor *SensorManager::getSensor(char id)
{
    for (uint8_t i = 0; i < count; i++)
    {
        if (sensors[i].id == id)
            return sensors[i].sensor;
    }
    return nullptr;
}

bool SensorManager::readSensor(char id)
{
    Sensor *sensor = getSensor(id);
    if (sensor == nullptr)
        return false;
    return sensor->read();
}

void SensorManager::readAll()
{
    for (uint8_t i = 0; i < count; i++)
    {
        bool currentState = sensors[i].sensor->read();
        String mensaje = "";
        mensaje += "Sensor ";
        mensaje += sensors[i].id;
        mensaje += ": ";
        mensaje += (currentState ? "ON" : "OFF");
        Logger::info(mensaje.c_str());
    }
}