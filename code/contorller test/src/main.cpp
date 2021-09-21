#include <M5Stack.h>
#include <SPI.h>

#define xSpi hspi

const int pinSCK = 18;
const int pinMISO = 19;
const int pinMOSI = 23;
const int pinDacCS = 1;
const int pinAdcCS = 3;

const int pinAnalogOut = 36;

SPIClass hspi(VSPI);
SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

void setup() {
  Serial.begin(9600);
  xSpi.begin();
  pinMode(pinSCK, OUTPUT);
  pinMode(pinMISO, INPUT);
  pinMode(pinMOSI, OUTPUT);
  pinMode(pinDacCS, OUTPUT);
  pinMode(pinAdcCS, OUTPUT);
  digitalWrite(pinDacCS, HIGH);
  digitalWrite(pinAdcCS, HIGH);
  analogReadResolution(10);
  delay(500);
}

// channel番のピン（A~H:1~8）にvalue（0～1023）分の電圧を出力させる関数
void setDac(long channel, long value) {
  channel--;
  long temp = (0x0f & channel) << 12;    //added by mtakag
  temp = temp | ((0x3ff & value) << 2);  //added by mtakag

  Serial.println(temp, HEX);  // DO NOT COMMENT OUT, added by mtakag ...

  digitalWrite(pinDacCS, LOW);
  xSpi.beginTransaction(spiSettings);
  xSpi.transfer(0xff & (temp >> 8));
  xSpi.transfer(0xff & temp);
  xSpi.endTransaction();
  digitalWrite(pinDacCS, HIGH);
}

// channel番のピンが読んだ値をintで返す関数
int getAdc(uint8_t channel) {
  digitalWrite(pinAdcCS, LOW);
  xSpi.beginTransaction(spiSettings);
  xSpi.transfer(0x06 | channel >> 2);
  uint8_t dh = xSpi.transfer((channel & 0x03) << 6);
  uint8_t dl = xSpi.transfer(0x00);
  xSpi.endTransaction();
  digitalWrite(pinAdcCS, HIGH);
  return ((dh & 0x0F) << 8 | dl);
}

void loop() {
  for (int i = 0; i <= 1; i++) {
    int value = i * 512;
    setDac(0, value);
    Serial.print("i:");
    Serial.print(i);
    Serial.print("\t");
    Serial.print("read:");
    Serial.print(analogRead(pinAnalogOut));
    Serial.println("");
    delay(100);
  }
}