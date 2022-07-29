#include <VitconBrokerComm.h>
#include <SoftPWM.h>
#include <U8g2lib.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// DEFINE CONSTANT
#define DHTPIN A1
#define DHTTYPE DHT22
#define SOILHUMI A6
#define PUMP 16
#define LAMP 17
#define ITEM_COUNT 18

using namespace vitcon;

DHT dht(DHTPIN, DHTTYPE);

// dht22 sensor
IOTItemFlo dht22_temp, dht22_humi;
IOTItemInt soilhumi;

// Status setup
IOTItemBin DeviceStatus, Device(device_out);
IOTItemBin FanStatus, Fan(fan_out);
IOTItemBin PumpStatus, Pump(pump_out);
IOTItemBin LampStatus, Lamp(lamp_out);

IOTItemBin StopStatus, Stop(timeset_out);

IOTItemBin IntervalHUP(Interval_Hup);
IOTItemBin IntervalMUP(Interval_Mup);
IOTItemBin IntervalRST(IntervalReset);

IOTItemInt label_Hinterval;
IOTItemInt label_Minterval;

// Setup items array
IOTItem *items[ITEM_COUNT] = { &dht22_temp, &dht22_humi, &soilhumi,
                               &FanStatus, &Fan,
                               &PumpStatus, &Pump,
                               &LampStatus, &Lamp,
                               &DeviceStatus, &Device,
                               &StopStatus, &Stop,
                               &IntervalRST, &IntervalHUP, &IntervalMUP,
                               &label_Hinterval, &label_Minterval};

const char device_id[] ="84e067740caf6af51a019be4474cd629";
BrokerComm comm(&Serial, device_id, items, ITEM_COUNT);
SOFTPWM_DEFINE_CHANNEL(A3);

uint32_t TimeCompare = 2000; //ms
uint32_t StartTime = 0;
uint32_t DataCaptureDelay_DHT22 = 2000; //ms
uint32_t DataCaptureDelay_SOIL = 3000; //ms
uint32_t DataCapture_ST_DHT22 = 0;
uint32_t DataCapture_ST_SOIL = 0;
uint32_t TimeSum = 0;
uint32_t TimePushDelay = 0;
uint32_t TimerStartTime = 0;

// define variable
bool device_status;
bool fan_out_status;
bool pump_out_status;
bool lamp_out_status;
bool timeset = false;
bool Interval_Mup_status;
bool Interval_Hup_status;

float Temp = 0;
float Humi = 0;
int Soilhumi = 0;
int Hour = 0;
int Minute = 1;
int LED = 0;


void setup() {
  // put your setup code here, to run once:
  // Setup all status
  Serial.begin(250000);
  comm.SetInterval(200);
  SoftPWM.begin(490);
  dht.begin();
  pinMode(SOILHUMI, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  digitalWrite(LAMP, LOW);
  TimeSum = (uint32_t)(Hour * 60 + Minute) * 60 * 1000; //interval
  u8g2.begin();

  StartTime = millis();
  DataCapture_ST_DHT22 = millis();
  DataCapture_ST_SOIL = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  InvervalSet(timeset);

  if (device_status){
    Serial.println("Auto Lamp Start");
    if(timeset){
      if(TimeCompare % 2 == 0) { digitalWrite(LAMP, LOW); }
      else { digitalWrite(LAMP, HIGH); }
    }
  }
  else {
    if (lamp_out_status == true) { digitalWrite(LAMP, HIGH); }
    else { digitalWrite(LAMP, LOW); }
  }
  
//  TimeCompare = (millis() - StartTime) / TimeSum;

//  if (lamp_out_status == true){ digitalWrite(LAMP, HIGH); }
//  else digitalWrite(LAMP, LOW);
  
  if ((millis() - DataCapture_ST_DHT22) > DataCaptureDelay_DHT22){
    Humi = dht.readHumidity();
    Temp = dht.readTemperature();

    if (isnan(Humi)||isnan(Temp)){
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    if (device_status){
      Serial.println("Auto FAN Start");
      if (Temp >= 30){ SoftPWM.set(100); }
      else if (Temp > 25){ SoftPWM.set(70); }
      else { SoftPWM.set(0); }
    }
    else{
      if (fan_out_status) { SoftPWM.set(100); }
      else { SoftPWM.set(0); }
    }
    
    DataCapture_ST_DHT22 = millis();
  }

  if ((millis() - DataCapture_ST_SOIL) > DataCaptureDelay_SOIL){
    Soilhumi = map(analogRead(SOILHUMI), 0, 1023, 100, 0);

    if (isnan(Soilhumi)){
      Serial.println(F("Failed to read from SOIL sensor!"));
      return;
    }

    if (device_status){
      Serial.println("Auto PUMP Start");
      if (Soilhumi >= 60){ digitalWrite(PUMP, LOW); }
      else if (Soilhumi < 30){ digitalWrite(PUMP, HIGH); }
    }
    else {
      if (pump_out_status) { digitalWrite(PUMP, HIGH); }
      else { digitalWrite(PUMP, LOW); }
    }
    
    DataCapture_ST_SOIL = millis();
  }

  digitalWrite(PUMP, pump_out_status);
  digitalWrite(LAMP, lamp_out_status);
  
  OLEDdraw();
  dht22_temp.Set(Temp);
  dht22_humi.Set(Humi);
  soilhumi.Set(Soilhumi);

  DeviceStatus.Set(device_status);
  FanStatus.Set(fan_out_status);
  PumpStatus.Set(digitalRead(PUMP));
  LampStatus.Set(digitalRead(LAMP));

  StopStatus.Set(timeset);
  label_Hinterval.Set(Hour);
  label_Minterval.Set(Minute);
  comm.Run();
}

void OLEDdraw(){
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_te);
  u8g2.drawStr(1, 15, "SMART FARM");
  
  u8g2.drawStr(1, 25, "T:"); u8g2.setCursor(20, 25); u8g2.print(Temp); u8g2.drawStr(50, 25, "\xb0 C");
  u8g2.drawStr(70, 25, "H:"); u8g2.setCursor(90, 25); u8g2.print(Humi); u8g2.drawStr(117, 25, "%");
  u8g2.drawStr(1, 36, " Soil Humi"); u8g2.setCursor(80, 36); u8g2.print(Soilhumi); u8g2.drawStr(95, 36, "%");

  u8g2.drawStr(1, 47, " PAN"); u8g2.setCursor(30, 47); u8g2.print(fan_out_status);
  u8g2.drawStr(37, 47, " PUMP"); u8g2.setCursor(80, 47); u8g2.print(pump_out_status);
  u8g2.drawStr(90, 47, " LED"); u8g2.setCursor(120, 47); u8g2.print(lamp_out_status);

  u8g2.drawStr(1, 58, " LED Interval");
  u8g2.setCursor(80, 58); u8g2.print(TimeCompare);
  u8g2.drawStr(110, 58, "ms");
  
  u8g2.sendBuffer();
}

void device_out(bool val) { device_status = val; }
void fan_out(bool val) { fan_out_status = val; }
void pump_out(bool val) { pump_out_status = val; }
void lamp_out(bool val) { lamp_out_status = val; }

void timeset_out(bool val){ timeset = val; }
void Interval_Hup(bool val){ Interval_Hup_status = val; }
void Interval_Mup(bool val){ Interval_Mup_status = val; }

void IntervalReset(bool val) {
  if(!timeset && val) {
    Hour=0;
    Minute=0;
  }
}

// When timeset is false, 
void InvervalSet(bool timeset) {
  if(!timeset){
    TimeSum = (uint32_t)(Hour * 60 + Minute) * 60 * 1000;
    TimerStartTime = millis();
    
    if(millis() > TimePushDelay + 500) { // if current time bigger than Delay
      Hour += Interval_Hup_status;
      if(Hour >= 24) Hour = 0;
      
      Minute += Interval_Mup_status;
      if(Minute >= 60) Minute = 0;
    
      TimePushDelay = millis();
    }
  }
  else{ TimeCompare = (millis() - TimerStartTime) / TimeSum; }
}
