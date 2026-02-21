#pragma once
#include "motor.h"

/*
    Multi-engine coordinator.
    1. Manage multiple axes
    2. Call `step()` on all axes
    3. Enable future synchronization
*/

#define MAX_MOTORS 6

class motor_manager
{
private:
    motor* motors[MAX_MOTORS];
    uint8_t count = 0;
public:
    void addMotor(motor* m);
    void update();
};
