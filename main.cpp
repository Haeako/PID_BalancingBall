//***3RPS Parallel Manipulator Ball Balancer Code BY Aaed Musa**
// Modified by Haeako
//--------------------------------------------------------------

//libraries
#include <pigpio.h>
#include "AccelStepper.h"
#include "InverseKinematics.h"
#include "MultiStepper.h"
#include <wiringPi.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <unistd.h>
#include "comunicate.h"

using namespace std;
// Steeper constant
#define MAX_SPEED 1200
#define STEP_RANGE 2000 // Step range
#define Timeout_value 10
template <typename T>
T constrain(T x, T a, T b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}
FifoServer reader;

#define DIR_PIN1    22  // GPIO 27
#define STEP_PIN1    27  // GPIO 22
// Stepper 2
#define DIR_PIN2    24  // GPIO 23
#define STEP_PIN2    23  // GPIO 24
// Stepper 3
#define DIR_PIN3   5   // GPIO 5
#define STEP_PIN3   6   // GPIO 6

// Create an AccelStepper object for each stepper motor
AccelStepper stepperA(AccelStepper::DRIVER, STEP_PIN1, DIR_PIN1);
AccelStepper stepperB(AccelStepper::DRIVER, STEP_PIN2, DIR_PIN2);
AccelStepper stepperC(AccelStepper::DRIVER, STEP_PIN3, DIR_PIN3);

// Create a MultiStepper instance to control all stepper motors
MultiStepper steppers;

Machine machine(2, 3.125, 1.75, 3.669291339);     //(d, e, f, g) object to define the lengths of the machine


// Stepper motor variables                                                                              
double angOrig = 206.662752199;// Original angle that each leg starts at
double speed[3] = {0, 0, 0}, speedPrev[3], ks = 70, ka = 200 ;  // The speed of the stepper motor and the speed, accelaration amplifying constant
long pos[3] ={0, 0, 0}; // An array to store the target positions for each stepper motor

//PID variables
double error[2] = {0, 0}, errorPrev[2], integr[2] = {0, 0}, deriv[2] = {0, 0}, out[2], normalized_deriv[2];  // PID terms for X and Y directions
long timeI;                                                                           // Variables to capture initial times

//Other Variables
double angToStep = STEP_RANGE / 360;  // Angle to step conversion factor (steps per degree) for 16 microsteps or 3200 steps/rev
bool detected = 0;              // This value is 1 when the ball is detected and the value is 0 when the ball in not detected

double  // Offset
        Xoffset = 271,   // X offset for the center position of the platform
        Yoffset = 251;  // Y offset for the center position of the platform
//takes in an X and Y setpoint/position and moves the ball to that position
double setpointX = 0, // target X's potision 
       setpointY = 0; // target Y's potision 

double  //PID constants
        //propational term
        kp = 1.5753E-4,
        // intergal term
        ki = 1.154E-6,
        // deivetive term 
        kd = 3.4325E-3;


// Point
struct Point
{
  int x; 
  int y;
};


// Moves positions the platform with the given parameters
void moveTo(double hz, double nx, double ny) {
  //if the ball has been detected
  if (detected) {
    // digitalWrite(LedYELLOW,HIGH);
    // calculates stepper motor positon
    for (int i = 0; i < 3; i++) {
      pos[i] = round((angOrig - machine.theta(i, hz, nx, ny)) * angToStep);
    }
    //sets calculated speed
    stepperA.setMaxSpeed(speed[A]);
    stepperB.setMaxSpeed(speed[B]);
    stepperC.setMaxSpeed(speed[C]);
    //sets acceleration to be proportional to speed
    stepperA.setAcceleration(speed[A]*ka);
    stepperB.setAcceleration(speed[B]*ka);
    stepperC.setAcceleration(speed[C]*ka);
    stepperA.moveTo(pos[A]);
    stepperB.moveTo(pos[B]);
    stepperC.moveTo(pos[C]);
    //runs stepper to target position (increments at most 1 step per call)
    stepperA.runSpeedToPosition();
    stepperB.runSpeedToPosition();
    stepperC.runSpeedToPosition();
  }
  //if the hasn't been detected
  else {
    for (int8_t i = 0; i < 3; i++) {
      pos[i] = round((angOrig - machine.theta(i, hz, 0, 0)) * angToStep);
    }
    //sets max speed
    stepperA.setMaxSpeed(MAX_SPEED);
    stepperB.setMaxSpeed(MAX_SPEED);
    stepperC.setMaxSpeed(MAX_SPEED);
    //moves the stepper motors
    steppers.moveTo(pos);
    steppers.runSpeedToPosition();  //runs stepper to target position (increments at most 1 step per call)
  }
}

void PID(double setpointX, double setpointY) {
  Point p;
  delay(20);
  reader.run(p.x, p.y); 
  cerr << "actual " <<p.x << " " <<p.y << endl; //measure X and Y positions
  //p.y = abs(p.y - 1024);
  //if the ball is detected (the x position will not be 0)
  if (p.x != 0) {
    detected = 1;

    //calculates PID values
    for (int i = 0; i < 2; i++) {
      errorPrev[i] = error[i];                                                                     //sets previous error
      error[i] = (i == 0) * (Xoffset - p.x - setpointX) + (i == 1) * (Yoffset - p.y - setpointY);  //sets error aka X or Y ball position
      integr[i] += error[i] + errorPrev[i];
      integr[i] = constrain(integr[i], -2500.0,2500.0);                                                       //calculates the integral of the error (proportional but not equal to the true integral of the error)
      
      deriv[i] = error[i] - errorPrev[i];                                                          //calcuates the derivative of the error (proportional but not equal to the true derivative of the error)
      deriv[i] = isnan(deriv[i]) || isinf(deriv[i]) ? 0 : deriv[i];                                //checks if the derivative is a real number or infinite
      normalized_deriv[i] = (i == 0) * (log(abs(deriv[i]) + 0.1) + 1) + (i == 1) * ( log( abs(deriv[i]) + 0.1) + 1);
      out[i] = kp * error[i] + ki * integr[i] + kd* deriv[i];                                     //sets//(kd + normalized_deriv[i] * alpha) * deriv[i];                                     //sets output
      out[i] = constrain(out[i], -0.35, 0.35);                                                     //contrains output to have a magnitude of 0.25
    }
    //calculates stepper motor speeds
    for (int i = 0; i < 3; i++) {
      speedPrev[i] = speed[i];                                                                                                           //sets previous speed
      speed[i] = (i == A) * stepperA.currentPosition() + (i == B) * stepperB.currentPosition() + (i == C) * stepperC.currentPosition();  //sets current position
      speed[i] = abs(speed[i] - pos[i]) * ks;                                                                                            //calculates the error in the current position and target position
      speed[i] = constrain(speed[i], speedPrev[i] - 200, speedPrev[i] + 200);                                                            //filters speed by preventing it from beign over 100 away from last speed
      speed[i] = constrain(speed[i], 0.0, 1000.0);                                                                                            //constrains sped from 0 to 1000
    }
   //cout << out[0] << "," << out[1] << endl;
  }
  //if the ball is not detected (the x value will be 0)
  else{
    //double check that there is no ball
    //10 mllis delay before another reading
    delay(10);
    reader.run(p.x, p.y);
    //cout << p.x <<  p.y << endl;
      //measure X and Y positions again to confirm no ball
     //measure X and Y positions again to confirm no ball    
    if (p.x == 0)
    { 
      //if the ball is still not detected
      //Serial.println("BALL NOT DETECTED");
      delay(20);
      moveTo(4.25, 0, 0);   
      detected = 0;
    }
  }
  //continues moving platforma and waits until 20 millis has elapsed
unsigned long timeI = millis();
while (millis() - timeI < 20) {
    moveTo(4.25, -out[0], -out[1]);  //moves the platfrom
  }
}

void setup() {
  // Start camera

  //cout << "Camera Started" << endl << "Adding Steppers..." << endl;
  // Adding the steppers to the steppersControl instance for multi stepper control
  steppers.addStepper(stepperA);
  steppers.addStepper(stepperB);
  steppers.addStepper(stepperC);
  //start brige connection
  moveTo(4.25, 0, 0);             //  Moves the platform to the home position
  steppers.runSpeedToPosition();  //  Blocks until the platform is at the home position
}
int main() {
  setup();
  while(true){
  PID(setpointX, setpointY);
  }  // (X setpoint, Y setpoint) -- must be looped
}

  
