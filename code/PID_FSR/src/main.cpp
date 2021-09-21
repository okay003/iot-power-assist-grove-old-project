#include <M5Stack.h>
#include <SPI.h>

// spi通信用
#define SPI1_CLK 18
#define SPI1_MISO 19
#define SPI1_MOSI 23
#define SPI1_SS 3
#define SPI_CLK 4000000

// spi通信用
SPIClass SPI1(HSPI);
SPISettings spiSettings = SPISettings(SPI_CLK, SPI_MSBFIRST, SPI_MODE0);  // 0,0

// 指パラメータ設定用
struct Finger {
  bool isOneshot = false;
  int inPin;
  int outPin;
  float mag;
  float kp;
  float ki;
  float kd;
  int propVal = 0;
  int inteVal = 0;
  int deriVal = 0;
  int curProc = 0;
  int preProc = 0;
};
struct Finger g_finger[5];

// 指パラメータ初期化関数
Finger initFinger(int in, int out, float mag, float p, float i, float d);
// index番の指のセンサ値を返す関数
int getSensorVal(int index);
// index番の指のDACに値を渡す関数
void setDacVal(int index);
// 前回フレーム-現在フレーム間の経過時間dtをmicroオーダーで返す関数
float getMicroDt();
// index番の指に対するPI-D操作量を返す関数
int getPi_dManipulate(int index);

void setup() {
  Serial.begin(115200);

  // spi通信用
  SPI1.begin(SPI1_CLK, SPI1_MISO, SPI1_MOSI, SPI1_SS);
  pinMode(SPI1_SS, OUTPUT);
  pinMode(SPI1_CLK, OUTPUT);
  pinMode(SPI1_MISO, INPUT);
  pinMode(SPI1_MOSI, OUTPUT);
  digitalWrite(SPI1_SS, HIGH);
  SPI1.beginTransaction(spiSettings);

  // 指パラメータ初期化
  g_finger[0] = initFinger(0, 0, 0, 0, 0, 0);
  g_finger[1] = initFinger(0, 0, 0, 0, 0, 0);
  g_finger[2] = initFinger(0, 0, 0, 0, 0, 0);
  g_finger[3] = initFinger(0, 0, 0, 0, 0, 0);
  g_finger[4] = initFinger(0, 0, 0, 0, 0, 0);

  // 経過時間dtを取得する関数getMicroDt()をここで一回呼んでおくことで
  // 初回の長いdtをloopに持っていかなくて済む
  getMicroDt();
}

void loop() {
}

// 指パラメータ初期化関数
Finger initFinger(int in, int out, float mag, float p, float i, float d) {
  Finger f;
  f.inPin = in;
  f.outPin = out;
  f.mag = mag;
  f.kp = p;
  f.ki = i;
  f.kd = d;
  return f;
}

// index番の指のセンサ値を返す関数
int getSensorVal(int index) {
  digitalWrite(SPI1_SS, LOW);                       // select chip
  SPI1.transfer(0x06 | index >> 2);                 // bit7-3:dummy, bit2:start, bit1:single,bit0:D2
  uint8_t dh = SPI1.transfer((index & 0x03) << 6);  // D1,D0,X,Null,B11-B8
  uint8_t dl = SPI1.transfer(0x00);                 // B7-B0
  digitalWrite(SPI1_SS, HIGH);                      // unselect chip
  return ((dh & 0xF) << 8 | dl);                    // return 12bits data
}

// index番の指のDACに値を渡す関数
void setDacVal(int index) {
}

// 前回フレーム-現在フレーム間の経過時間dtをmicroオーダーで返す関数
float getMicroDt() {
  static int pre = 0;
  float dt = (float)(micros() - pre) / 1000000.0;
  pre = micros();
  return dt;
}

// index番の指に対するPI-D操作量を返す関数
int getPi_dManipulate(int index) {
  Finger f = g_finger[index];

  int target = getSensorVal(index);
  int process = getSensorVal(index);
  int error = target - process;

  float dt = getMicroDt();

  // proportional
  f.propVal *= f.kp * error;

  // integral
  f.inteVal += error * dt;
  f.inteVal *= f.ki;

  // derivative
  if (!f.isOneshot) {
    f.curProc = process;
    f.preProc = f.curProc;
    f.isOneshot = true;
  }
  f.preProc = f.curProc;
  f.curProc = process;
  f.deriVal = f.kd * (f.curProc - f.preProc) / dt;

  // manipulate
  float manipulate = f.propVal + f.inteVal - f.deriVal;
  return manipulate;
}