#pragma once
#include "motor.h"
#include "../drivers/limits.h"


class axis
{
private:
    motor &motorRef;
    Limits &homeSwitch;
    enum HomingState {
        HOMING_IDLE,
        HOMING_FAST_SEARCH,
        HOMING_BACKOFF,
        HOMING_SLOW_SEARCH,
        HOMING_SET_ZERO,
        HOMING_MEASURE_TRAVEL,
        HOMING_DONE,
        HOMING_ERROR
    };
    HomingState state; 
    long maxTravelSteps;            // Expected limit 
    long measuredMaxSteps;          // Actual measured value
    bool isInverted;              // If the direction is inverted, so the homing will be in the opposite direction
    const long backoffSteps = 100;
    long backoffCounter = 0;

    long dynamicSpeed(int percentage);    
public:
    axis(motor &motorType, Limits &sw, long maxTravel, bool inverted = false);
    void startHoming();
    void update();
    bool isHomed() const;
    long getMeasuredMax() const;
};
