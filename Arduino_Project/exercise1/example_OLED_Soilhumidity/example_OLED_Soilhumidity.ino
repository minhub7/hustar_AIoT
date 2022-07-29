#include <U8g2lib.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#define DHTPIN A1
#define DHTTYPE DHT22
#define SOILHUMI A6

DHT dht(DHTPIN, DHTTYPE);

uint32_t DataCaptureDelay = 3000; //ms
uint32_t DataCapture_ST = 0;

float Temp;
float Humi;
int Soilhumi = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  pinMode(SOILHUMI, INPUT);
  u8g2.begin();

  DataCapture_ST = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((millis() - DataCapture_ST) > DataCaptureDelay){
    Humi = dht.readHumidity();
    Temp = dht.readTemperature();
    Soilhumi = map(analogRead(SOILHUMI), 0, 1023, 100, 0);

    if (isnan(Humi)||isnan(Temp)||isnan(Soilhumi)){
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    OLEDdraw();
    DataCapture_ST = millis();
  }
}

void OLEDdraw(){
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_te);
  u8g2.drawStr(1, 15, "SMART FARM");
  
  u8g2.drawStr(15,25, "Temp.");
  u8g2.setCursor(85, 25);
  u8g2.print(Temp);
  u8g2.drawStr(114, 25, "\xb0");
  u8g2.drawStr(119, 25, "C");

  u8g2.drawStr(15, 36, "Humidity");
  u8g2.setCursor(85, 36); u8g2.print(Humi);
  u8g2.drawStr(116, 36, "%");

  u8g2.drawStr(15, 47, " Soil Humi");
  u8g2.setCursor(85, 47); u8g2.print(Soilhumi);
  u8g2.drawStr(116, 47, "%");
  
  u8g2.sendBuffer();
}
