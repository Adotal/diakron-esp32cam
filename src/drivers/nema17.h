#pragma once
#include "../hal/IPinDriver.h"
#include "../core/motor.h"
/*
    1. Configure pins in begin()
    2. Generate STEP pulses
    3. Change direction
    Typically in a NEMA 17 with an a4988 driver

    ---- INFO ----
    - MAX RPM: 300 RPM (theoretical, but usually not stable at that speed)
    - MAX RPM STABLE: 295 RPM
    - MIN RPM: 1 RPM (below that it may not move due to insufficient torque)

    -- NOTE --
    - If you want use I2C expanders, the max RPM may be lower due to the slower signal changes, 
      but it should still be above 220 RPM which is more than enough for our application.

*/

class nema17 : public motor
{
private:
    uint8_t stepPin;
    uint8_t dirPin;
    uint8_t enablePin;

    // Variables para el control de velocidad constante (No bloqueante)
    unsigned long lastStepTime;  // Último micros() donde se dio un paso
    unsigned long stepInterval;  // Microsegundos entre pasos
    bool currentDirection;
    long position;
    bool direction;
    
    // RPM
    long const DEFAULT_RPM_NEMA17 = 60;
    long const MAX_RPM_NEMA17 = 295;
    long const STEPS_PER_REVOLUTION = 200;

public:
    nema17(IPinDriver &driver, uint8_t stepPin, uint8_t dirPin, uint8_t enablePin);
    void begin();
    void step() override;
    void setDirection(bool dir);
    void enable(bool en);
    void setSpeed(long rpm);
    void update();
    long getPosition() override;
    void resetPosition(long pos = 0) override;
    long getMaxRPM() override;
    long getDefaultRPM() override;
};

