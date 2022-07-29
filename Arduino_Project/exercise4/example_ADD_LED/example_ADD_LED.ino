#include <SoftPWM.h>
#include <U8g2lib.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#define DHTPIN A1
#define DHTTYPE DHT22
#define SOILHUMI A6
#define PUMP 16
#define LAMP 17

DHT dht(DHTPIN, DHTTYPE);

uint32_t TimeCompare = 2000; //ms
uint32_t StartTime = 0;
uint32_t DataCaptureDelay_DHT22 = 2000; //ms
uint32_t DataCaptureDelay_SOIL = 3000; //ms
uint32_t DataCapture_ST_DHT22 = 0;
uint32_t DataCapture_ST_SOIL = 0;
uint32_t TimeSum = 0;

SOFTPWM_DEFINE_CHANNEL(A3);

float Temp = 0;
float Humi = 0;
int Soilhumi = 0;
int Hour = 0;
int Minute = 1;
int PAN = 0;
int LED = 0;
int PUMP_status = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SoftPWM.begin(490);
  dht.begin();
  pinMode(SOILHUMI, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  TimeSum = (uint32_t)(Hour * 60 + Minute) * 60 * 1000; //interval
  u8g2.begin();

  StartTime = millis();
  DataCapture_ST_DHT22 = millis();
  DataCapture_ST_SOIL = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  TimeCompare = (millis() - StartTime) / TimeSum;
  if (TimeCompare % 2 == 0){
    digitalWrite(LAMP, LOW);
    LED = 0;
  }
  else{
    digitalWrite(LAMP, HIGH);
    LED = 1;
  }
  
  if ((millis() - DataCapture_ST_DHT22) > DataCaptureDelay_DHT22){
    Humi = dht.readHumidity();
    Temp = dht.readTemperature();

    if (isnan(Humi)||isnan(Temp)){
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    if (Temp >= 30){
      SoftPWM.set(100);
      PAN = 1;
    }
    else if (Temp > 20){
      SoftPWM.set(65);
      PAN = 1;
    }
    else {
      SoftPWM.set(0);
      PAN = 0;
    }

    Serial.print(F("Humidity: "));
    Serial.print(Humi);
    Serial.print(F("% Temperature: "));
    Serial.print(Temp);
    Serial.println(F("'C"));

    OLEDdraw();
    DataCapture_ST_DHT22 = millis();
  }

  if ((millis() - DataCapture_ST_SOIL) > DataCaptureDelay_SOIL){
    Soilhumi = map(analogRead(SOILHUMI), 0, 1023, 100, 0);

    if (isnan(Soilhumi)){
      Serial.println(F("Failed to read from SOIL sensor!"));
      return;
    }

    if (Soilhumi >= 60){
      digitalWrite(PUMP, LOW);
      PUMP_status = 0;
      delay(2000);
    }
    else if (Soilhumi > 30){
      digitalWrite(PUMP, HIGH);
      PUMP_status = 1;
      delay(2000);
    }
    else{
      digitalWrite(PUMP, LOW);
      PUMP_status = 0;
      delay(2000);
    }

    OLEDdraw();
    DataCapture_ST_SOIL = millis();
  }
  
}

void OLEDdraw(){
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_te);
  u8g2.drawStr(1, 15, "SMART FARM");
  
  u8g2.drawStr(1, 25, "T:");
  u8g2.setCursor(20, 25); u8g2.print(Temp);
  u8g2.drawStr(50, 25, "\xb0"); u8g2.drawStr(55, 25, "C");

  u8g2.drawStr(70, 25, "H:");
  u8g2.setCursor(90, 25); u8g2.print(Humi);
  u8g2.drawStr(117, 25, "%");

  u8g2.drawStr(1, 36, " Soil Humi");
  u8g2.setCursor(80, 36); u8g2.print(Soilhumi);
  u8g2.drawStr(95, 36, "%");

  u8g2.drawStr(1, 47, " PAN");
  u8g2.setCursor(30, 47); u8g2.print(PAN);
 
  u8g2.drawStr(37, 47, " PUMP");
  u8g2.setCursor(80, 47); u8g2.print(PUMP_status);
  
  u8g2.drawStr(90, 47, " LED");
  u8g2.setCursor(120, 47); u8g2.print(LED);

  u8g2.drawStr(1, 58, " LED Interval");
  u8g2.setCursor(80, 58); u8g2.print(TimeCompare);
  u8g2.drawStr(90, 58, "ms");
  
  u8g2.sendBuffer();
}
