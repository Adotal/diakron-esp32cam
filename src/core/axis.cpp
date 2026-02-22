#include "axis.h"

axis::axis(motor &motorType, Limits &sw, long maxTravel, bool inverted) : motorRef(motorType), homeSwitch(sw), maxTravelSteps(maxTravel), isInverted(isInverted) {
    state = HOMING_IDLE;
    measuredMaxSteps = 0;
    isInverted = inverted;
}

void axis::startHoming() {
    motorRef.enable(true);
    bool searchDir = isInverted;
    motorRef.setDirection(searchDir);
    motorRef.setSpeed(dynamicSpeed(80)); // Start with 80% speed
    state = HOMING_FAST_SEARCH;
}

/*
    -------------- HOW IT WORKS --------------
    1. Start homing in the default direction (can be inverted with isInverted)
    2. When the switch is activated, back up until you release the switch.
    3. Approach slowly until the switch is triggered again, this is the home position, set it to zero
    4. Move in the opposite direction until the switch is triggered again, this is the max travel, save this value
    5. Move back to home position and stop, homing is done

*/

void axis::update(){
    bool toSensor = isInverted;
    bool awayFromSensor = !isInverted;
    switch (state)
    {
    case HOMING_FAST_SEARCH:
        if(!homeSwitch.isTriggered()){
            motorRef.update();
        } else{
            state = HOMING_BACKOFF;
            motorRef.setDirection(awayFromSensor);
            motorRef.setSpeed(dynamicSpeed(20)); // Back off at 30% speed
            backoffCounter = 0;
        }
        break;

    case HOMING_BACKOFF:
        if(homeSwitch.isTriggered()){
            motorRef.update();
        }else{
            state = HOMING_SLOW_SEARCH;
            motorRef.setDirection(toSensor);
            motorRef.setSpeed(dynamicSpeed(20)); // Slow search at 20% speed
        }
        break;
    
    case HOMING_SLOW_SEARCH:
        if(!homeSwitch.isTriggered()){
            motorRef.update();
        }else{
            // Home real encontrado
            motorRef.resetPosition(0); 
            // Ahora vamos a buscar el otro extremo del eje
            motorRef.setDirection(awayFromSensor);
            motorRef.setSpeed(dynamicSpeed(60));
            state = HOMING_DONE;
        }
        break;
    
    /*case HOMING_SET_ZERO:
        motorRef.resetPosition(0);
        motorRef.setDirection(isInverted ? true : false);
        motorRef.setSpeed(dynamicSpeed(70)); // Set max travel at 70% speed
        state = HOMING_MEASURE_TRAVEL;
        break;*/
    
    case HOMING_MEASURE_TRAVEL:
        if(motorRef.getPosition() < maxTravelSteps){
            motorRef.update();
        }else{
            measuredMaxSteps = motorRef.getPosition();
            motorRef.enable(false);
            state = HOMING_DONE;
        }
        break;
    case HOMING_DONE:
        // Homing complete, do nothing or maintain state
        motorRef.enable(false);
        break;

    case HOMING_ERROR:
        // Handle error state, maybe stop the motor and signal an error
        motorRef.enable(false);
        break;

    default:
        break;
    }
}

bool axis::isHomed() const{
    return state == HOMING_DONE;
}

long axis::getMeasuredMax() const{
    return measuredMaxSteps;
}

long axis::dynamicSpeed(int percentage){
    // This function can be implemented to adjust the speed based on the distance to the target position
    // For example, you can reduce the speed as it gets closer to the target to avoid overshooting
    // This is just a placeholder implementation and can be adjusted based on the specific requirements of your application
    return (((motorRef.getDefaultRPM() + motorRef.getMaxRPM())/2) * percentage) / 100;
}