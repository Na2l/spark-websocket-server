/*
 * Bed.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: naal
 */

#include "Bed.h"

Bed::Bed(ADXL335 head, ADXL335 legs, uint32_t address, bool his){
  headSensor=head;
  legsSensor=legs;
  flashAddress=address;
  hisBed=his;
  load();
}

Bed::~Bed() {
  // TODO Auto-generated destructor stub
}

void Bed::load() {
  for(int i=0;i<8;i++) {
    positions[i]=0;
  }
  sFLASH_ReadBuffer(positions, flashAddress, 8);
}

void Bed::saveHeadPos(int pos, uint8_t deg, bool final) {
  positions[pos]=deg;
  if(final) {
    sFLASH_EraseSector(flashAddress);
    sFLASH_WriteBuffer(positions, flashAddress, 8);
  }
}

void Bed::saveLegsPos(int pos, uint8_t deg, bool final) {
  positions[pos+4]=deg;
  if(final) {
    sFLASH_EraseSector(flashAddress);
    sFLASH_WriteBuffer(positions, flashAddress, 8);
  }
}
void Bed::update() {
  headSensor.update();
  legsSensor.update();
}

void Bed::stop() {
  for(int i=0;i<4;i++) {
    if(hisBed) {
      digitalWrite(HIS_PINS[i],LOW);
    } else {
      digitalWrite(HER_PINS[i],LOW);
    }
  }
}

void Bed::gotoPos(int pos){
  uint8_t head = positions[pos];
  update();

  Serial.print("gotoPos,  head: ");
  Serial.print(head);
  bool up=head<255 && getHeadX()<head;
  bool down=head<255 && head<getHeadX();
  Serial.print(", up: ");
  Serial.print(up?"yes, down: ":"no, down: ");
  Serial.println(down?"yes":"no");

  ushort pin=up?(hisBed?HIS_PINS[0]:HER_PINS[0]):down?(hisBed?HIS_PINS[1]:HER_PINS[1]):255;
  adjust(true, pin, head,up);

  uint8_t leg = positions[pos+4];
  Serial.print(", leg: ");
  Serial.print(leg);

  up=leg<255 && getLegX()<leg;
  down=leg<255 && leg<getLegX();

  Serial.print(", up: ");
  Serial.print(up?"yes, down: ":"no, down: ");
  Serial.println(down?"yes":"no");


  pin=up?(hisBed?HIS_PINS[2]:HER_PINS[2]):down?(hisBed?HIS_PINS[3]:HER_PINS[3]):255;
  adjust(false, pin, leg,up);

}
void Bed::adjust(bool head, ushort pin, uint8_t value, bool up) {
  if(pin!=255) {
    Serial.print("adjust: ");
    Bed::fptr test=this->getIt(head);
    double tmp=(this->*test)();

    Serial.print(tmp);
    Serial.print(" <> ");
    Serial.println(value);
    delay(1000);

    while(shouldContinue(tmp,value,up)) {
      digitalWrite(pin,HIGH);
      update();
      tmp=(this->*test)();
      Serial.print(tmp);
      Serial.print(" <> ");
      Serial.println(value);
      delay(1000);
      if(!shouldContinue(tmp,value,up)) {
        break;
      }
    }
    digitalWrite(pin,LOW);
  }
}

bool Bed::shouldContinue(double val1, uint8_t val2, bool up) {
  if(up) {
    return val1<val2;
  }
  return val2<val1;
}

double Bed::getLegX() {
  return legsSensor.getXDeg();
}

double Bed::getHeadX() {
  return headSensor.getXDeg();
}

void Bed::dump() {
  Serial.print("Head[");

  for (int i=0;i<4;i++) {
    Serial.print(positions[i]);
    if(i<3) {
      Serial.print(",");
    }
  }
  Serial.println("]");
}
