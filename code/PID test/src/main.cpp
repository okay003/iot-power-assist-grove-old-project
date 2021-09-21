#include <M5Stack.h>

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