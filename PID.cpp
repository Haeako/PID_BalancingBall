#include <iostream>
#include <pigpio.h>
#include "AccelStepper.h"
#include "MultiStepper.h"

// Define GPIO pins for controlling the stepper motors
// Stepper 1
#define DIR_PIN1    22  // GPIO 27
#define STEP_PIN1    27  // GPIO 22
// Stepper 2
#define DIR_PIN2    24  // GPIO 23
#define STEP_PIN2    23  // GPIO 24
// Stepper 3
#define DIR_PIN3   5   // GPIO 5
#define STEP_PIN3   6   // GPIO 6

// Create an AccelStepper object for each stepper motor
AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN1, DIR_PIN1);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN2, DIR_PIN2);
AccelStepper stepper3(AccelStepper::DRIVER, STEP_PIN3, DIR_PIN3);

// Create a MultiStepper instance to control all stepper motors
MultiStepper steppers;

void setup() {


    // Add steppers to the MultiStepper object
    steppers.addStepper(stepper1);
    steppers.addStepper(stepper2);
    steppers.addStepper(stepper3);

    // Set maximum speed and acceleration for each stepper motor
    stepper1.setMaxSpeed(1000);  // Maximum speed (steps per second)
    stepper1.setAcceleration(500);  // Acceleration (steps per second^2)
    
    stepper2.setMaxSpeed(1000);  // Maximum speed (steps per second)
    stepper2.setAcceleration(500);  // Acceleration (steps per second^2)
    
    stepper3.setMaxSpeed(1000);  // Maximum speed (steps per second)
    stepper3.setAcceleration(500);  // Acceleration (steps per second^2)
}

int main() {
    gpioInitialise();
    gpioSetMode(DIR_PIN1 , PI_OUTPUT);
    gpioSetMode(STEP_PIN1, PI_OUTPUT);
    gpioSetMode(DIR_PIN2 , PI_OUTPUT);
    gpioSetMode(STEP_PIN2, PI_OUTPUT);
    gpioSetMode(DIR_PIN3 , PI_OUTPUT);
    gpioSetMode(STEP_PIN3, PI_OUTPUT);
        setup();
    // Move motors to specified positions
    std::cout << "Moving motors to target positions..." << std::endl;
    
    // Define target positions for each motor (in steps)
    long int pos[3] = {10000, -10000, 10000};
    
    // Move motors to the target positions
    steppers.moveTo(pos);
    
    steppers.runSpeedToPosition();  // Block until the motors reach their target positions
    gpioDelay(1000000);  // Wait for 1 second (1,000,000 microseconds)
    std::cout << "Motors reached target positions!" << std::endl;
    // Wait for 1 second

    
    // Cleanup pigpio
    gpioTerminate();
    
    return 0;
}
