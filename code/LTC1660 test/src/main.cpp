//(1) http://www.kerrywong.com/2010/05/02/a-library-for-ltc1665ltc1660/
//(2) htps://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/

#include <M5Stack.h>

//////////////////////////////////////////////////
#if 1  // for ESP32
const int pinIN = 23;
const int pinSCK = 18;  // was 16
const int pinCSLD = 1;  // was 19
// const int pinCLR =18;

const int pinAnalogOut = 36;

#else  // For Arduino UNO

const int pinIN = 11;   //was8;
const int pinSCK = 13;  //7;
const int pinCSLD = 3;
const int pinCLR = 6;

const int pinAnalogOut = 5;

#endif

void pinInit();
//////////////////////////////////////////////////
void setup() {
  pinInit();
  Serial.begin(115200);
  Serial.print("Long type : ");
  Serial.println(sizeof(long));
  delay(100);
  //return;
  delay(1);
}

//////////////////////////////////////////////////
void pinInit() {
  pinMode(pinIN, OUTPUT);
  pinMode(pinSCK, OUTPUT);
  pinMode(pinCSLD, OUTPUT);
  // pinMode(pinCLR, OUTPUT);

  delay(100);
  // digitalWrite(pinCLR, HIGH);
  digitalWrite(pinSCK, LOW);
  digitalWrite(pinCSLD, HIGH);
}

#if 0
void myDelay()
{
 //   for(int i = 0; i < 100; i ++)
 //     for(int j = 0; j < 100; j ++);
 //   delay(1);
  
}
#endif
//////////////////////////////////////////////////
//Performs D/A conversion
// Oritinal source code was from (1), and modifyed by mtakag. 20200207
void output(long chn, long val) {
  //the control word is 16 bits
  //the high 4 bits defines the output channel
  //  long t = chn << 12;
  long t = (0x0f & chn) << 12;  //added by mtakag

  //the lower 4 bits are don't cares so we make
  //them zeros.
  //t = t | val << 4;
  //for LTC1660 it has 10bit resolution this line will
  //need to change to:
  //t = t | val << 2;
  t = t | ((0x3ff & val) << 2);  //added by mtakag

  //Serial.println(t, HEX );
  digitalWrite(pinCSLD, LOW);
  for (long i = 15; i >= 0; i--) {
    long b = (t >> i) & 0x01;
    digitalWrite(pinIN, b);
    digitalWrite(pinSCK, HIGH);
    digitalWrite(pinSCK, LOW);
  }
  digitalWrite(pinCSLD, HIGH);
}

//////////////////////////////////////////////////
void loop() {
  //return;
  //Serial.println("####################################");
  //output( 1, 255 );
  output(1, 128);
  output(2, 256);
  output(3, 512);
  output(4, 1023);
  delay(100);
  //return;
  //Serial.println("####################################");
  //int j = 1;
  for (int j = 1; j < 5; j++) {
    Serial.println("+++++++++++++++++++++++++++++++++++++++");
    for (int i = 0; i < 10; i++) {
      long val = 1 << i;
      //long val = 256;
      output(j, val);
      delay(1);
      Serial.print(val);
      Serial.print(" - ");
      Serial.println(analogRead(pinAnalogOut));
      delay(100);
    }
  }
}