// summary
// M5からblynkを介してスマホ/PCにメールを送る
// also see Google Drive : ものづくり人材育成2020/グローブサイド/

#include <BlynkSimpleEsp32.h>
#include <M5Stack.h>
#include <WiFi.h>

const char auth[] = "Y0jEaZ8YOi6rieh_f3b2DANK2Cr5u5Kb";
const char ssid[] = "elecom-a9e937";
const char pass[] = "ie8e7nf3v4pd";
const char address[] = "bn18219@shibaura-it.ac.jp";

void setup() {
  M5.begin();
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();  // blynk使うときは書かなきゃいけない関数
  M5.update();  // M5の物理ボタンを使うときは書かなきゃいけない関数
  if (m5.BtnB.wasReleased()) {
    m5.Lcd.println("hoa");  // 物理ボタン動作確認用
    Blynk.email(address, "hoa");  // address宛てにhoaって内容のメールを送る
  }
  delay(100);
}