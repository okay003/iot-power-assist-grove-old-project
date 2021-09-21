#include <M5Stack.h>
#include <SPI.h>

const int pinSCK = 18;
const int pinMISO = 19;
const int pinMOSI = 23;
const int pinAdcCS = 3;
const int pinDacCS = 1;

const int pinRead = 36;

SPIClass hspi(VSPI);
SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

#define xSpi hspi

bool g_isSos = false;

// 指パラメータ設定用
struct Finger {
  int getPin;
  int setPin;
  float mag;
  int readValue = 0;
  int fingerValue = 0;
  int pamValue = 0;
};
struct Finger g_finger[5];

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// Fingerパラメータ初期化関数
Finger initFinger(int get, int set, float mag);

// channel番(0~7)のピンが読んだ値をint(0~4095)で返す関数
int getAdc(uint8_t channel);

// channel番(A~H:0~7)のピンにvalue(0～1023)分の電圧を出力させる関数
void setDac(long channel, long value);

// g_isSosをBlynkから取得する関数
bool checkIsSos();

// appsideにvalueを送信する関数
void sendValues(int v1, int v2, int v3);

// 全てのPAMに排気させる関数
void exhaustPam();

// index番(thubm~pinky:0~4)のセンサが読んだ値分だけindex番のPAMを収縮させる関数
void operatePam(int index);

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  xSpi.begin();
  pinMode(pinSCK, OUTPUT);
  pinMode(pinMISO, INPUT);
  pinMode(pinMOSI, OUTPUT);
  pinMode(pinAdcCS, OUTPUT);
  pinMode(pinDacCS, OUTPUT);
  digitalWrite(pinAdcCS, HIGH);
  digitalWrite(pinDacCS, HIGH);

  g_finger[0] = initFinger(0, 1, 1);
  g_finger[1] = initFinger(1, 2, 1);
  g_finger[2] = initFinger(2, 3, 1);
  g_finger[3] = initFinger(3, 4, 1);
  g_finger[4] = initFinger(4, 5, 1);

  delay(500);
}

void loop() {
  g_isSos = checkIsSos();
  if (g_isSos) {  // SOSのとき
    exhaustPam();
  } else {  // SOSじゃないとき
    // for (int i = 0; i < 5; i++) {
    //   Finger f = g_finger[i];
    //   operatePam(i);
    //   sendValues(f.readValue, f.fingerValue, f.pamValue);
    // }

    // testing
    // for (int i = 0; i < 5; i++) {
    //   Finger f = g_finger[i];
    //   for (int j = 0; j <= 10; j++) {
    //     setDac(f.setPin, j * 100);
    //     Serial.print("i : ");
    //     Serial.print(i);
    //     Serial.print(", value : ");
    //     Serial.print(getAdc(f.getPin));
    //     Serial.print(", value : ");
    //     Serial.print(analogRead(pinRead));
    //     Serial.println("");
    //     delay(100);
    //   }
    // }
    for (int i = 0; i < 5; i++) {
      Serial.print(getAdc(i));
      Serial.print("\t");
    }
    Serial.println("");
    delay(1000);
  }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// Fingerパラメータ初期化関数
Finger initFinger(int get, int set, float mag) {
  Finger f;
  f.getPin = get;
  f.setPin = set;
  f.mag = mag;
  return f;
}

// channel番(0~7)のピンが読んだ値をint(0~4095)で返す関数
int getAdc(uint8_t channel) {
  digitalWrite(pinAdcCS, LOW);
  xSpi.beginTransaction(spiSettings);
  xSpi.transfer(0x06 | channel >> 2);
  uint8_t dh = xSpi.transfer((channel & 0x03) << 6);
  uint8_t dl = xSpi.transfer(0x00);
  xSpi.endTransaction();
  digitalWrite(pinAdcCS, HIGH);
  delay(1);
  return ((dh & 0x0F) << 8 | dl);
}

// channel番(A~H:0~7)のピンにvalue(0～1023)分の電圧を出力させる関数
void setDac(long channel, long value) {
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

// g_isSosをBlynkから取得する関数
bool checkIsSos() {
  bool isSos = false;
  //
  // write codes you get from appside here
  //
  return isSos;
}

// appsideにvalueを送信する関数
void sendValues(int v1, int v2, int v3) {
}

// 全てのPAMに排気させる関数
void exhaustPam() {
  for (int i = 0; i < 5; i++) {
    setDac(i, 0);
  }
}

// index番(thubm~pinky:0~4)のセンサが読んだ値分だけindex番のPAMを収縮させる関数
void operatePam(int index) {
  Finger f = g_finger[index];
  f.readValue = getAdc(f.getPin) / 4;
  f.fingerValue = 1 / (1 + f.mag) * f.readValue;
  f.pamValue = f.mag * f.fingerValue;
  setDac(f.setPin, f.pamValue);
}
