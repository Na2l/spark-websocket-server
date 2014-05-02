// ThisLegs #include statement was automatically added by the Spark IDE.
#include "application.h"
#include <math.h>
#include "SparkWebSocketServer.h"
#include "spark_disable_cloud.h"

void handle(String &cmd, String &result);
// typedef for better readability
TCPServer server = TCPServer(2525);

//WebSocketServer ws;
SparkWebSocketServer mine(server);
bool digitalStates[]={false,false,false,false,false,false,false,false};

void dfu();
void tinkerDigitalRead(const String &pin, String &result);
void tinkerDigitalWrite(const String &command, String &result);
void tinkerAnalogRead(const String &pin, String &result);
void tinkerAnalogWrite(String &command, String &result);

void tinkerDigitalReadAll(String &result) {
  result+="DigitalAll, ";
  for(int i=0;i<8;i++) {
    result+="D";
    result+=i;
    result+=":";
    result+=(digitalStates[i]?1:0);
    if(i+1<8) {
      result+=",";
    }
  }
}

void setup() {
  uint8_t D_PINS[]= {D0,D1,D2,D3,D4,D5,D6,D7};
  uint8_t A_PINS[]= {A0,A1,A2,A3,A4,A5,A6,A7};
  for(uint8_t i=0;i<8;i++) {
    pinMode(D_PINS[i], INPUT);
    pinMode(A_PINS[i], INPUT);
  }

  CallBack cb=&handle;
  mine.setCallBack(cb);

  Serial.begin(9600);
  bool s=false;
  RGB.control(true);
  while(!Serial.available()) {
    // Run some test code so we know the core is running!
    s = !s; // toggle the state
    RGB.brightness(10);
    if(s) {
      RGB.color(0,0,255);
      delay(500); //
    } else {
      RGB.color(0,255,0);
      delay(200);
      RGB.color(255,0,255);
      delay(300);
    }
  }
  RGB.color(0,0,0);
  RGB.control(false);
  //  mine.test();
  netapp_ipconfig(&ip_config);
  server.begin();
  //  Serial.println(cb("/pos"));
  Serial.println(Network.localIP());
  Serial.println(Network.subnetMask());
  Serial.println(Network.gatewayIP());
  Serial.println(Network.SSID());

  Serial.println("Setup done");
}


/**
 * check the cmd (client request), do what needs to be done and store the result
 * int the result
 * @param cmd  client request
 * @param result the string to be send back to the client
 */
void handle(String &cmd, String &result){

  Serial.print("in handle, got: ");
  Serial.println(cmd);
  // go to dfu mode
  if(cmd.startsWith("/dfu")) {
    dfu();
    return;
  }
  if(cmd == "/digitalreadall") {
        tinkerDigitalReadAll(result);
        return;
      }
  int index=cmd.indexOf('?');
  if(index==-1) {
    result = "WHAT";
  } else {
    String c=cmd.substring(0,index);
    String rest=cmd.substring(index+1,cmd.length());
    if(c == "/digitalwrite") {
      tinkerDigitalWrite(rest, result);
    } else if(c == "/analogwrite") {
      tinkerAnalogWrite(rest,result);
    } else if(c == "/analogread") {
      tinkerAnalogRead(rest,result);
    }else if(c == "/digitalread") {
      tinkerDigitalRead(rest,result);
    } else {
      result = "Could not undrestand:";
      result+=c;
    }
  }
}
void loop() {
  mine.doIt();
  delay(1000);
}
/**
 * goto dfu mode
 */
void dfu() {
  //RESET INTO DFU MODE - NOTE THIS WILL STOP RUNNING YOUR FIRMWARE
  //Don't do this unless you are comfortable flashing over USB / DFU
  //and maybe have a programmer shield / st-link programmer
  FLASH_OTA_Update_SysFlag = 0x0000;
  Save_SystemFlags();
  BKP_WriteBackupRegister(BKP_DR10, 0x0000);
  USB_Cable_Config(DISABLE);
  NVIC_SystemReset();
}

/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
void tinkerDigitalRead(const String &pin, String &result)
{
  //convert ascii to integer
  int pinNumber = pin.charAt(1) - '0';
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber< 0 || pinNumber >7) {
    result="-1";
    return;
  }

  if(pin.startsWith("D"))
  {
    pinMode(pinNumber, INPUT_PULLDOWN);
    int32_t t=digitalRead(pinNumber);
    result="Digital, ";
    result+=pinNumber;
    result += ":";
    result+=t;
    return;
  }
  else if (pin.startsWith("A"))
  {
    pinNumber = pinNumber + 10;
    pinMode(pinNumber, INPUT_PULLDOWN);
    int32_t t=digitalRead(pinNumber);
    result="Digital, ";
    result += pinNumber;
    result += ":";
    result+=t;
    return;
  }
  result="-2";
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
void tinkerDigitalWrite(const String &command, String &result)
{
  bool value = 0;
  //convert ascii to integer
  int pinNumber = command.charAt(1) - '0';
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber< 0 || pinNumber >7) {
    result="-1";
    return;
  }

  if(command.substring(3,7) == "HIGH"){
    value = 1;
  } else if(command.substring(3,6) == "LOW"){
    value = 0;
  } else{
    result="-2";
    return;
  }

  if(command.startsWith("D")) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, value);
    result="digitalWrote, ";
    result+=pinNumber;
    result += ":";
    result+=value;
    digitalStates[pinNumber]=value==1;
    return;
  } else if(command.startsWith("A")){
    pinNumber = pinNumber + 10;
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, value);
    result="digitalWrote, ";
    result+=pinNumber;
    result+=":";
    result+=value;
    return;
  }
  result="-3";
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
void tinkerAnalogRead(const String &pin, String &result)
{
  //convert ascii to integer
  int pinNumber = pin.charAt(1) - '0';
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber< 0 || pinNumber >7){
    result="-1";
    return;
  }

  if(pin.startsWith("D")) {
    pinMode(pinNumber, INPUT);
    result="Analog, ";
    result+=pinNumber;
    result+=":";
    result+=analogRead(pinNumber);
    return ;
  } else if (pin.startsWith("A"))  {
    pinNumber = pinNumber + 10;
    pinMode(pinNumber, INPUT);
    result="Analog, ";
    result+=(pinNumber-10);
    result+=":";
    result+=analogRead(pinNumber);
    return;
  }
  result="-2";
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
void tinkerAnalogWrite(String &command, String &result)
{
  //convert ascii to integer
  int pinNumber = command.charAt(1) - '0';
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber< 0 || pinNumber >7) {
    result="-1";
    return;
  }

  String value = command.substring(3);

  if(command.startsWith("D")) {
    pinMode(pinNumber, OUTPUT);
    analogWrite(pinNumber, value.toInt());
    result="analogWrote ";
    result+=pinNumber;
    result+=":";
    result+=value.toInt();
    return;
  }
  else if(command.startsWith("A"))
  {
    pinNumber = pinNumber + 10;
    pinMode(pinNumber, OUTPUT);
    analogWrite(pinNumber, value.toInt());
    result="analogWrote ";
    result+=pinNumber;
    result+=":";
    result+=value.toInt();
    return;
  }
  result="-2";
}

