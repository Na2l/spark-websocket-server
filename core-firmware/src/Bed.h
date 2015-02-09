/*
 * Bed.h
 *
 *  Created on: Mar 23, 2014
 *      Author: naal
 */

#ifndef BED_H_
#define BED_H
#include "application.h"
#include "ADXL335.h"
#include <stdio.h>


static const ushort HIS_PINS[]= {D0,D1,D2,D3};
static const ushort HER_PINS[]= {D4,D5,D6,D7};
class Bed {
  public:
    Bed(ADXL335 head, ADXL335 legs, uint32_t hAddress, bool his);
    virtual ~Bed();
    void saveHeadPos(int pos, uint8_t deg, bool final=false);
    void saveLegsPos(int pos, uint8_t deg, bool final=false);
    void dump();
    void update();
    void stop();
    double getLegX();
    double getHeadX();

    void gotoPos(int pos);

    void toString(String &tmp) {
      tmp+="head: [";
      tmp+=positions[0];
      tmp+=",";
      tmp+=positions[1];
      tmp+=",";
      tmp+=positions[2];
      tmp+=",";
      tmp+=positions[3];
      tmp+="] legs:[";
      tmp+=positions[4];
      tmp+=",";
      tmp+=positions[5];
      tmp+=",";
      tmp+=positions[6];
      tmp+=",";
      tmp+=positions[7];
      tmp+="]";
    }
    //sprintf(tmp, "his head: [%d, %d, %d, %d]\n his legs: [%d, %d, %d, %d]",
    //   headPosition[0], headPosition[1], headPosition[2], headPosition[3]);
    void dump(USBSerial &serial) {
      String test;
      toString(test);
      serial.println(test);
    }

  private :
    typedef double (Bed::*fptr)(void); // typedef for better readability
    fptr getIt(bool head) {
      if(head) {
        return &Bed::getHeadX;
      }
      return &Bed::getLegX;
    }
    bool shouldContinue(double val1, uint8_t val2, bool up);
    void adjust(bool head, ushort pin, uint8_t value, bool up);
    void load();
    bool hisBed;
    uint8_t positions[8];

    uint32_t flashAddress;

    ADXL335 headSensor;
    ADXL335 legsSensor;

};

#endif /* BED_H_ */
