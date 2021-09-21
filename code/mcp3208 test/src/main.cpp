#include <M5Stack.h>
#include <SPI.h>

#define SPI1_CLK 18
#define SPI1_MISO 19
#define SPI1_MOSI 23
#define SPI1_SS 3

#define SPI_CLK 4000000

//SPIClass SPI(HSPI);
SPISettings spiSettings = SPISettings(SPI_CLK, SPI_MSBFIRST, SPI_MODE0);  // 0,0

void setup() {
  Serial.begin(115200);
  SPI.begin();  //SPI1_CLK, SPI1_MISO, SPI1_MOSI, SPI1_SS);
  pinMode(SPI1_SS, OUTPUT);
  pinMode(SPI1_CLK, OUTPUT);
  pinMode(SPI1_MISO, INPUT);
  pinMode(SPI1_MOSI, OUTPUT);
  digitalWrite(SPI1_SS, HIGH);
}

int adc_read(uint8_t channel) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SPI1_SS, LOW);
  // bit7-3:dummy, bit2:start, bit1:single,bit0:D2
  SPI.transfer(0x06 | channel >> 2);
  // D1,D0,X,Null,B11-B8
  uint8_t dh = SPI.transfer((channel & 0x03) << 6);
  // B7-B0
  uint8_t dl = SPI.transfer(0x00);
  digitalWrite(SPI1_SS, HIGH);
  SPI.endTransaction();

  // get 12bits data
  return ((dh & 0xF) << 8 | dl);
}

void loop() {
  for (int i = 0; i < 5; i++) {
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(adc_read(i));
    Serial.print("\t");
  }
  Serial.println("");
  delay(50);
}
