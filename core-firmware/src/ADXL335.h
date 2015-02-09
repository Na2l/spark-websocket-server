/*
 * ADXL335.h
 *
 *  Created on: Mar 19, 2014
 *      Author: naal
 */

#ifndef ADXL335_H_
#define ADXL335_H_
#include <spark_wiring.h>
#include "spark_wiring_usbserial.h"
#define __ADXL335_H__

#define ADC_ref 5.0
#define analog_resolution 1024.0

#define zero_x 1.60156
#define zero_y 1.60156
#define zero_z 1.64551

#define sensitivity_x 0.33
#define sensitivity_y 0.33
#define sensitivity_z 0.31
#define PI 3.14159265
#define HALF_PI 1.57079
#define TWO_PI 6.283185
#define DEG_TO_RAD 0.01745329
#define RAD_TO_DEG 57.2957786
#define MIN_VAL  1600
#define MAX_VAL  2400

class ADXL335 {
  public:
    ADXL335(unsigned xPin = A0, unsigned yPin = A1, unsigned zPin = A2);
    /**
     * call this before getting sensor values.
     */
    void update();

    double getXDeg();
    double getYDeg();
    double getZDeg();
    void dump(USBSerial &serial, char *debug);

    float angle_x;
    float angle_y;
    float angle_z;

  private:
    unsigned int xPin;
    unsigned int yPin;
    unsigned int zPin;

};

#endif /* ADXL335_H_ */
