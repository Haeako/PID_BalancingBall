#ifndef InverseKinematics_H
#define InverseKinematics_H

//constants
#define A 0
#define B 1
#define C 2

class Machine { //machine class
  public:
    //class functions
    Machine(double d, double e, double f, double g);
    double theta(int leg, double hz, double nx, double ny); //returns the value of theta a, b, or c
};

#endif
