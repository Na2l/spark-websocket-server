/*
 * ADXL335.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: naal
 */

#include "ADXL335.h"
#include <math.h>
ADXL335::ADXL335(unsigned xP, unsigned yP, unsigned zP) {
  xPin = xP;
  yPin = yP;
  zPin = zP;
}

void ADXL335::update() {
  //read the analog values from the accelerometer
  double xRead = analogRead(xPin);
  double yRead = 255;
  if (yPin != 255) {
    yRead = analogRead(yPin);
  }
  double zRead = analogRead(zPin);
  //convert read values to degrees -90 to 90 - Needed for atan2
  double xAng = 255;
  if (xPin != 255) {
    xAng = map(xRead, MIN_VAL, MAX_VAL, -90, 90);
  }
  double yAng = 255;
  if (yPin != 255) {
    yAng = map(yRead, MIN_VAL, MAX_VAL, -90, 90);
  }
  double zAng = 255;
  if (zPin != 255) {
    zAng = map(zRead, MIN_VAL, MAX_VAL, -90, 90);
  }
  //Caculate 360deg values like so: atan2(-yAng, -zAng)
  //atan2 outputs the value of -π to π (radians)
  //We are then converting the radians to degrees
  angle_x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);

  angle_y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  angle_z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

}

double ADXL335::getXDeg() {
  return angle_x;
}

double ADXL335::getYDeg() {
  return angle_y;
}

double ADXL335::getZDeg() {
  return angle_z;
}

void ADXL335::dump(USBSerial &serial, char *debug) {
  //Output the caculations
  serial.print(debug);
  serial.print(", x: ");
  serial.print(getXDeg());

  serial.print(" y: ");
  serial.print(getYDeg());

  serial.print(" z: ");
  serial.println(getZDeg());
}
