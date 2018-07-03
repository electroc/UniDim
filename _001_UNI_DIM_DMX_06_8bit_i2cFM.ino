// - - - - -
// DmxSerial - A hardware supported interface to DMX.

// Copyright (c) 2011 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// 
// Documentation and samples are available at http://www.mathertel.de/Arduino

#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <DMXSerial.h>

#include <Wire.h>

#define NUMBER_OF_CHANNELS 16

#define dmxTimeout 2000

/* DIP SWITSCH

1 - 4
2 - 3
3 - 8
4 - 7
5 - 6
6 - 5
7 - A3
8 - A2
9 - A1
10 - A0
*/

const byte DIP_Array[9] = {A1,A2,A3,5,6,7,8,3,4};

const byte driver_enable = 2;
const byte led1 = 10;  // PWM output pin for Red Light.
const byte led2 = 9;  // PWM output pin for Yellow Light.

const byte dip_10 = A0;

unsigned int dmxAddress = 1;
unsigned int dmxValue[NUMBER_OF_CHANNELS];     //stores the DMX values we're interested in using.

boolean dmxOK = true;

boolean testMode = false;


unsigned int test_mode_counter=0;



void setup () {
  
  // enable pwm outputs
  pinMode(dip_10,INPUT_PULLUP);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  
  pinMode(driver_enable,OUTPUT);
  digitalWrite(driver_enable,LOW);
    
  for(byte i=0;i<=8;i++)
    {
      pinMode(DIP_Array[i], INPUT_PULLUP);
      if(digitalRead(DIP_Array[i])==LOW) {digitalWrite(led2,HIGH);delay(100);}
      else{digitalWrite(led2,LOW); delay(50); }
      digitalWrite(led2,LOW);
      delay(50);
    }
  
  Wire.begin(); // Start I2C Bus as Master
  Wire.setClock(400000);
  // change the clock rate (behind Wires' back)
//  TWBR = ((CPU_FREQ / 400000L) - 16) / 2;
  
  DMXSerial.init(DMXReceiver);
  
  // set some default values
  /*
  for(int x=0; x<NUMBER_OF_CHANNELS; x++){
    DMXSerial.write(DMX_ADDRESS+x,10);
    }
  */
  
  if(digitalRead(dip_10)==LOW)testMode=true;
  else testMode=false;
  
  dmxAddress = address_dip();
  
  EEPROM_readAnything(0, dmxAddress);
  if(dmxAddress==0||dmxAddress>=509){
    error();
    dmxAddress=1;
  }
  /*
  digitalWrite(led1,HIGH);
  delay(1000);
  digitalWrite(led1,LOW);
  delay(1000);
  */
}


void loop() {
 
      if (testMode==true) test_mode();
       // Calculate how long no data backet was received
      unsigned long lastPacket = DMXSerial.noDataSince();
      
      if (lastPacket < dmxTimeout) {
        if(dmxOK==false){
          dmxOK=!dmxOK;
          digitalWrite(led1,LOW);
          }
        if(digitalRead(dip_10)==LOW)testMode=true;  
        if(testMode==false){
            for(int x=0; x<NUMBER_OF_CHANNELS; x++){
              dmxValue[x]=DMXSerial.read(dmxAddress+x);
              }
              
            wire_action();
    //        analogWrite(led, dmxValue[0]);
        }
        
      } else {
        
        if(dmxOK==true){
          dmxOK=!dmxOK;
          digitalWrite(led1,HIGH);
          }
        if(digitalRead(dip_10)==LOW)testMode=true;  
        
        } // if

}

// End.

void wire_action() { 

    Wire.beginTransmission(4);                   // transmit to device #4
    for(int j=0; j <NUMBER_OF_CHANNELS; j++)
      {
       Wire.write(dmxValue[j]);
       }
    Wire.endTransmission();
    
    return;                                      
 }                                             

void test_mode(){
    test_mode_counter++;
    test_blink(test_mode_counter, 250);
    if(test_mode_counter>20)test_mode_counter=0;
    
    if(digitalRead(dip_10)==HIGH){
      dmxAddress = address_dip();
      testMode=false;
      }
    return;
  }

void all_blink(int y){
  
  for(int x=0; x<NUMBER_OF_CHANNELS; x++){
      dmxValue[x]=y;
      }
    wire_action();
    return;
  }

void test_blink(int number, int value){
    digitalWrite(led2,HIGH);
    if(number<NUMBER_OF_CHANNELS){
      for(int x=0; x<NUMBER_OF_CHANNELS; x++){
        dmxValue[x]=0;
        if(number==x)dmxValue[x]=value;
        }
      wire_action();
      delay(150);
      digitalWrite(led2,LOW);
      delay(50);
    }
    else {
      all_blink(250);
      delay(200);
      digitalWrite(led2,LOW);
      all_blink(0);
      delay(200);
    }
    
    return;
  
  }

int address_dip(){
  unsigned int j = 0;
  j = ~j;
  for(int i=0; i<=8; i++){
    j = (j << 1) | digitalRead(DIP_Array[i]); //bitwise OR
    delay(10);
    }
  j = ~j;
  if(j==0||j>=509){
    error();
    return 1;
   }
   else{
    EEPROM_writeAnything(0, j);
    return j;  
    }
  }
  
  
void error(){
    all_blink(255);
    delay(100);
    all_blink(20);
    delay(200);
    all_blink(255);
    delay(100);
    all_blink(20);
    delay(200);
  
}  

