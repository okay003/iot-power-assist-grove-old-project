#include <M5Stack.h>
#include <SPI.h>

// --------------------------------------------------

const int pinSCK = 18;
const int pinMISO = 19;
const int pinMOSI = 23;
const int pinDacCS = 13;
const int pinAdcCS = 12;

SPIClass hspi(VSPI);
SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

#define xSpi hspi

// --------------------------------------------------

bool g_isFirstTerm = false;
bool g_isSos = false;

struct Finger {
  int getPin;
  int setPin;
  int readValue = 0;
  int fingerValue = 0;
  int pamValue = 0;
};
struct Finger g_finger[5];

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// Fingerパラメータ初期化関数
Finger initFinger(int get, int set);

// channel番(0~7)のピンが読んだ値をint(0~4095)で返す関数
int getAdc(uint8_t channel);

// channel番(A~H:0~7)のピンにvalue(0～1023)分の電圧を出力させる関数
void setDac(long channel, long value);

// 投票でオンオフ制御する関数
void votingController(int electionTime, int thresValue, int thresTimes, int period);

// g_isSosをBlynkから取得する関数
bool checkIsSos();

// appsideにvalueを送信する関数
void sendValues(int v1, int v2, int v3);

// 全てのPAMに排気させる関数
void exhaustPam();

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  xSpi.begin();
  pinMode(pinSCK, OUTPUT);
  pinMode(pinMISO, INPUT);
  pinMode(pinMOSI, OUTPUT);
  pinMode(pinDacCS, OUTPUT);
  pinMode(pinAdcCS, OUTPUT);
  digitalWrite(pinDacCS, HIGH);
  digitalWrite(pinAdcCS, HIGH);

  // --------------------------------------------------

  g_finger[0] = initFinger(0, 1);
  g_finger[1] = initFinger(1, 2);
  g_finger[2] = initFinger(2, 3);
  g_finger[3] = initFinger(3, 4);
  g_finger[4] = initFinger(4, 5);

  // --------------------------------------------------

  delay(500);
}

void loop() {
  // votingController(3, 50, 5, 100);
  for (int i = 1; i <= 5; i++) {
    setDac(i, 0);
  }
  delay(500 + random(500));
  for (int i = 0; i <= 5; i++) {
    for (int j = 1; j <= 5; j++) {
      setDac(j, i * 100);
    }
    delay(125);
  }
  delay(2000 + random(1000));
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// Fingerパラメータ初期化関数
Finger initFinger(int get, int set) {
  Finger f;
  f.getPin = get;
  f.setPin = set;
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

  // Serial.println(temp, HEX);  // DO NOT COMMENT OUT, added by mtakag ...

  digitalWrite(pinDacCS, LOW);
  xSpi.beginTransaction(spiSettings);
  xSpi.transfer(0xff & (temp >> 8));
  xSpi.transfer(0xff & temp);
  xSpi.endTransaction();
  digitalWrite(pinDacCS, HIGH);
  delay(1);
}

// 投票でオンオフ制御する関数
void votingController(int electionTime, int thresValue, int thresTimes, int period) {
  int votes[electionTime];     // その回の得票数
  int isPassed[electionTime];  // その回は可決か
  bool isElected = true;       // その回の選挙は当選か

  int maxVoltage = 7;

  // 初期化
  for (int i = 0; i < electionTime; i++) {
    votes[i] = 0;
    isPassed[i] = 0;
  }

  // 投票チェック
  for (int i = 0; i < electionTime; i++) {             // 選挙は何回開かれるか
    for (int j = 0; j < 5; j++) {                      // 全部の指に投票させる
      if (getAdc(g_finger[j].getPin) <= thresValue) {  // readがthresValueを超えたら
        votes[i]++;                                    // 投票
      }
    }
    if (votes[i] >= thresTimes) {  // 得票数がthresTimesを超えたら
      isPassed[i] = 1;             // その回の選挙は可決
    }
    isElected *= isPassed[i];  //全ての回の選挙で可決なら当選
  }

  // アクチュエータ管理
  if (isElected) {  // 当選ならオン
    if (!g_isFirstTerm) {
      for (int i = 0; i <= maxVoltage; i++) {
        for (int j = 0; j < 5; j++) {
          setDac(g_finger[j].setPin, 100 * i);  // ワンクッション置いたら破裂しなさそう、という願い
          delay(10);
        }
      }
      g_isFirstTerm = true;
    } else {
      for (int i = 0; i < 5; i++) {
        setDac(g_finger[i].setPin, maxVoltage * 100);  // 1023(最大値)だと破裂しそう
      }
      delay(period);
    }
  } else {  // 落選ならオフ
    g_isFirstTerm = false;
    for (int i = 0; i < 5; i++) {
      setDac(g_finger[i].setPin, 0);
    }
    delay(period);
  }
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