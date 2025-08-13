#include "InverseKinematics.h"
#include <math.h>
#define PI 3.14159
//Global User Defined Constants
double d;  //distance from the center of the base to any of its corners
double e;  //distance from the center of the platform to any of its corners
double f;  //length of link #1
double g;  //length of link #2

//Calculation Variables
double nmag, nz;  //magnitude and z component of the normal vector
double x, y, z;   //generic variables for the components of legs A, B, and C
double mag;       //generic magnitude of the leg vector
double angle;     //generic angle for legs A, B, and C


//Library Content
Machine::Machine(double _d, double _e, double _f, double _g) {
  d = _d;
  e = _e;
  f = _f;
  g = _g;
}
double Machine::theta(int leg, double hz, double nx, double ny) {
  //create unit normal vector
  nmag = sqrt(pow(nx, 2) + pow(ny, 2) + 1);  //magnitude of the normal vector
  nx /= nmag;
  ny /= nmag;
  nz = 1 / nmag;
  //calculates angle A, B, or C
  switch (leg) {
    case A:  //Leg A
      y = d + (e / 2) * (1 - (pow(nx, 2) + 3 * pow(nz, 2) + 3 * nz) / (nz + 1 - pow(nx, 2) + (pow(nx, 4) - 3 * pow(nx, 2) * pow(ny, 2)) / ((nz + 1) * (nz + 1 - pow(nx, 2)))));
      z = hz + e * ny;
      mag = sqrt(pow(y, 2) + pow(z, 2));
      angle = acos(y / mag) + acos((pow(mag, 2) + pow(f, 2) - pow(g, 2)) / (2 * mag * f));
      break;
    case B:  //Leg B
      x = (sqrt(3) / 2) * (e * (1 - (pow(nx, 2) + sqrt(3) * nx * ny) / (nz + 1)) - d);
      y = x / sqrt(3);
      z = hz - (e / 2) * (sqrt(3) * nx + ny);
      mag = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
      angle = acos((sqrt(3) * x + y) / (-2 * mag)) + acos((pow(mag, 2) + pow(f, 2) - pow(g, 2)) / (2 * mag * f));
      break;
    case C:  //Leg C
      x = (sqrt(3) / 2) * (d - e * (1 - (pow(nx, 2) - sqrt(3) * nx * ny) / (nz + 1)));
      y = -x / sqrt(3);
      z = hz + (e / 2) * (sqrt(3) * nx - ny);
      mag = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
      angle = acos((sqrt(3) * x - y) / (2 * mag)) + acos((pow(mag, 2) + pow(f, 2) - pow(g, 2)) / (2 * mag * f));
      break;
  }
  return (angle * (180 / PI));  //converts angle to degrees and returns the value
}
