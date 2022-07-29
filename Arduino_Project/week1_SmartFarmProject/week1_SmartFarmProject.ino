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
#define ITEM_COUNT 52

using namespace vitcon;

DHT dht(DHTPIN, DHTTYPE); // define sensor variable
SOFTPWM_DEFINE_CHANNEL(A3);

// define variable
bool timeset = false;
uint32_t TimeCompare = 2000; //ms
uint32_t Delay_ms = 1000;
uint32_t THS_Delay = 2000; //ms
uint32_t THS_ST = 0;
uint32_t fan_ST = 0;
uint32_t pump_ST = 0;
uint32_t lamp_ST = 0;
float Temp = 0;
float Humi = 0;
int Soilhumi = 0;



// dht22, soilhumi sensor for web communication
IOTItemFlo dht22_temp, dht22_humi;
IOTItemInt soilhumi;

// Define variable for Fan
IOTItemBin AutoFanStatus, AutoFan(auto_fan), FanActionButton(fan_action_btn), ResetFanButton(fan_reset_btn);
IOTItemBin pwm1UpButton(pwm1_up_btn), pwm1DownButton(pwm1_down_btn);
IOTItemBin pwm2UpButton(pwm2_up_btn), pwm2DownButton(pwm2_down_btn);
IOTItemInt Trackbar_FanTimer_Value, Trackbar_FanTimer(set_fan_timer), Fan_elapsedTime, Label_pwm1, Label_pwm2;
IOTItemInt Trackbar_Temp1_Value, Trackbar_Temp1(set_fan_temp1);
IOTItemInt Trackbar_Temp2_Value, Trackbar_Temp2(set_fan_temp2);

int32_t fan_timer = 10, fan_temp1 = 25, fan_temp2 = 30, fan_power1 = 65, fan_power2 = 100;
bool auto_fan_status = false;

// Define variable for Pump
IOTItemBin AutoPumpStatus, AutoPump(auto_pump), PumpActionButton(Pump_action_btn);
IOTItemBin IntervalPumpStatus, IntervalPump(interval_pump), PumpUpButton(pump_up_btn), PumpDownButton(pump_down_btn);
IOTItemInt Trackbar_PumpTimer_Value, Trackbar_PumpTimer(set_pump_timer), Pump_elapsedTime, Label_interval_pump;
IOTItemInt Trackbar_StartHumi_Value, Trackbar_StartHumi(set_start_humi);
IOTItemInt Trackbar_EndHumi_Value, Trackbar_EndHumi(set_end_humi);
int32_t pump_timer = 5, start_humi = 30, end_humi = 60, pump_interval = 10;
bool auto_pump_status = false;
bool interval_pump_status;

// Define variable for Lamp
IOTItemBin AutoLampStatus, AutoLamp(auto_lamp), LampActionButton(lamp_action_btn);
IOTItemBin LampIntervalHUPButton(lamp_interval_h_up), LampIntervalHDOWNButton(lamp_interval_h_down);
IOTItemBin LampIntervalMUPButton(lamp_interval_m_up), LampIntervalMDOWNButton(lamp_interval_m_down);
IOTItemBin LampIntervalSUPButton(lamp_interval_s_up), LampIntervalSDOWNButton(lamp_interval_s_down);
IOTItemBin LampSETButton(lamp_interval_set);
IOTItemBin LampRSTButton(lamp_interval_reset);

IOTItemInt Trackbar_LampTimer_Value, Trackbar_LampTimer(set_lamp_timer), Lamp_elapsedTime;
IOTItemInt label_Hinterval, label_Minterval, label_Sinterval;

int32_t lamp_timer = 5, lamp_interval = 10;  // second
int32_t hour = 0, minute = 0, second = 10;
bool auto_lamp_status = false;


/*
  Not used
*/

// Setup items array
IOTItem *items[ITEM_COUNT] = { &dht22_temp, &dht22_humi, &soilhumi,
                               /* FAN items */
                               &AutoFanStatus, &AutoFan, &FanActionButton, &ResetFanButton, &Fan_elapsedTime,
                               &Trackbar_FanTimer_Value, &Trackbar_FanTimer,
                               &Trackbar_Temp1_Value, &Trackbar_Temp1, &Trackbar_Temp2_Value, &Trackbar_Temp2,
                               &pwm1UpButton, &pwm1DownButton, &pwm2UpButton, &pwm2DownButton,
                               /* PUMP items */
                               &AutoPumpStatus, &AutoPump, &IntervalPumpStatus, &IntervalPump, &PumpActionButton, &Pump_elapsedTime,
                               &Trackbar_PumpTimer_Value, &Trackbar_PumpTimer, &Trackbar_StartHumi_Value, &Trackbar_StartHumi,
                               &Trackbar_EndHumi_Value, &Trackbar_EndHumi, &PumpUpButton, &PumpDownButton,
                               /* LED items */
                               &AutoLampStatus, &AutoLamp, &LampActionButton, &LampSETButton, &LampRSTButton,
                               &Trackbar_LampTimer_Value, &Trackbar_LampTimer, &Lamp_elapsedTime,
                               &LampIntervalHUPButton, &LampIntervalMUPButton, &LampIntervalSUPButton,
                               &label_Hinterval, &label_Minterval, &label_Sinterval,
                               &LampIntervalHDOWNButton, &LampIntervalMDOWNButton, &LampIntervalSDOWNButton,
                               &Label_pwm1, &Label_pwm2, &Label_interval_pump
                               };

const char device_id[] ="84e067740caf6af51a019be4474cd629";
BrokerComm comm(&Serial, device_id, items, ITEM_COUNT);


void setup() {
  // put your setup code here, to run once:
  // Setup all status
  Serial.begin(250000);
  SoftPWM.begin(490);
  u8g2.begin();
  comm.SetInterval(200);
  dht.begin();
  pinMode(SOILHUMI, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  digitalWrite(LAMP, LOW);
  
  THS_ST = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
//  InvervalSet(timeset);

  if ((millis() - THS_ST) > THS_Delay){
    Temp = dht.readTemperature();
    Humi = dht.readHumidity();
    Soilhumi = map(analogRead(SOILHUMI), 0, 1023, 100, 0);
    
    if (isnan(Temp)||isnan(Humi)||isnan(Soilhumi)){
      Serial.println(F("Failed to read from DHT22 or Soilhumi sensor!"));
      return;
    }
    
    Serial.println((String)"\nTemp: " + Temp + " Humi: " + Humi + " Soilhumi: " + Soilhumi);
    
    // function for Auto_Control_FAN
    if (auto_fan_status){ Auto_Control_FAN(Temp); }
    else { SoftPWM.set(0); }
    
    // function for Auto_Control_PUMP
    if (auto_pump_status){ Auto_Control_PUMP(Soilhumi); }
    else { digitalWrite(PUMP, LOW); }
    
    // function for Auto_Control_LAMP
    if (auto_lamp_status){ Auto_Control_LAMP(lamp_interval); }
    else { digitalWrite(LAMP, LOW); }
    
    THS_ST = millis();
    OLEDdraw();
  }
  
  Set_everything();
  comm.Run();
}

/*
  Define function for Set Status, Value
*/
// for FAN
void auto_fan(bool val){ auto_fan_status = val; }
void fan_action_btn(bool val){
  if (val){
    Serial.println("call fan_action_btn() func");
    Manual_Control_FAN(fan_timer);
  }
}
void fan_reset_btn(bool val){ 
  if (val){
    Serial.println("call fan_reset_btn() func");
    fan_temp1 = 25, fan_temp2 = 30;
    fan_power1 = 65, fan_power2 = 100;
  }
}
void set_fan_timer(int32_t val){ fan_timer = (int)val; }
void set_fan_temp1(int32_t val){ fan_temp1 = (int)val; }
void set_fan_temp2(int32_t val){ fan_temp2 = (int)val; }
void pwm1_up_btn(bool val){
  if (val){
    fan_power1 += 5;
    if (fan_power1 > 100) fan_power1 = 100;
  }
}
void pwm1_down_btn(bool val){
  if (val){
    fan_power1 -= 5;
    if (fan_power1 < 0) fan_power1 = 0;
  }
}
void pwm2_up_btn(bool val){
  if (val){
    fan_power2 += 5;
    if (fan_power2 > 100) fan_power2 = 100;
  }
}
void pwm2_down_btn(bool val){
  if (val){
    fan_power2 -= 5;
    if (fan_power2 < 0) fan_power2 = 0;
  }
}

// for PUMP
void auto_pump(bool val) { auto_pump_status = val; }
void interval_pump(bool val){ interval_pump_status = val; }
void Pump_action_btn(bool val){
  if (val){
    Serial.println("call pump_action_btn() func");
    Manual_Control_PUMP(pump_timer);
  }
}
void set_pump_timer(int32_t val){ pump_timer = (int)val; }
void set_start_humi(int32_t val){ start_humi = (int)val; }
void set_end_humi(int32_t val){ end_humi = (int)val; }
void pump_up_btn(bool val){
  if (val){
    pump_interval += 1;
  }
}
void pump_down_btn(bool val){
  if (val){
    pump_interval -= 1;
    if (pump_interval < 0) pump_interval = 0;
  }
}

// for LAMP
void auto_lamp(bool val) { auto_lamp_status = val; }
void set_lamp_timer(int32_t val){ lamp_timer = val; }
void lamp_action_btn(bool val){
  if (val){
    Serial.println("call lamp_action_btn() func");
    Manual_Control_LAMP(lamp_timer);
  }
}
void lamp_interval_set(bool val){
  if (val){
    Serial.println("call lamp_interval_set() func");
    lamp_interval = (hour * 60 + minute) * 60 + second;
    Auto_Control_PUMP(lamp_interval);
  }
}
void lamp_interval_reset(bool val){
  if (val){
    Serial.println("call lamp_interval_reset() func");
    hour = 0;
    minute = 0;
    second = 10;
    lamp_interval = 10;
  }
}
void lamp_interval_h_up(bool val){
  if (val){
    Serial.println("call lamp_interval_h_up() func");
    hour = hour + 1;
    if (hour > 59){ hour = 0; } 
  }
}
void lamp_interval_h_down(bool val){
  if (val){
    Serial.println("call lamp_interval_h_down() func");
    if (hour > 0){ hour -= 1; } 
  }
}
void lamp_interval_m_up(bool val){
  if (val){
    Serial.println("call lamp_interval_m_up() func");
    minute = minute + 1;
    if (minute > 59){ minute = 0; }
  }
}
void lamp_interval_m_down(bool val){
  if (val){
    Serial.println("call lamp_interval_m_down() func");
    if (minute > 0){ minute -= 1; } 
  }
}
void lamp_interval_s_up(bool val){
  if (val){
    Serial.println("call lamp_interval_s_up() func");
    second = second + 1;
    if (second > 59){ second = 0; }
  }
}
void lamp_interval_s_down(bool val){
  if (val){
    Serial.println("call lamp_interval_s_down() func");
    if (second > 0){ second -= 1; } 
  }
}

/*
  Define function that performs a any action
*/
void Set_everything(){
  dht22_temp.Set(Temp);
  dht22_humi.Set(Humi);
  soilhumi.Set(Soilhumi);

  // Fan Setup
  AutoFanStatus.Set(auto_fan_status);
  Fan_elapsedTime.Set(fan_timer);
  Label_pwm1.Set(fan_power1);
  Label_pwm2.Set(fan_power2);
  Trackbar_FanTimer_Value.Set(fan_timer);
  Trackbar_Temp1_Value.Set(fan_temp1);
  Trackbar_Temp2_Value.Set(fan_temp2);

  // Pump Setup
  AutoPumpStatus.Set(auto_pump_status);
  IntervalPumpStatus.Set(interval_pump_status);
  Pump_elapsedTime.Set(pump_timer);
  Label_interval_pump.Set(pump_interval);
  Trackbar_PumpTimer_Value.Set(pump_timer);
  Trackbar_StartHumi_Value.Set(start_humi);
  Trackbar_EndHumi_Value.Set(end_humi);

  // Lamp Setup
  AutoLampStatus.Set(auto_lamp_status);
  Trackbar_LampTimer_Value.Set(lamp_timer);
  Lamp_elapsedTime.Set(lamp_timer);
  label_Hinterval.Set(hour);
  label_Minterval.Set(minute);
  label_Sinterval.Set(second);
}

void Auto_Control_FAN(float Temp){
  /* FAN control function by DHT22 Sensor */
  Serial.println("call Auto_Control_FAN() func");
  Serial.println((String)"--- fan_temp1: " + fan_temp1 + " fan_power1: " + fan_power1);
  Serial.println((String)"--- fan_temp2: " + fan_temp2 + " fan_power2: " + fan_power2);
  
  if (Temp >= fan_temp2){ SoftPWM.set(fan_power2); }
  else if (Temp > fan_temp1){ SoftPWM.set(fan_power1); }
  else { SoftPWM.set(0); }

}

void Manual_Control_FAN(int32_t fan_timer){
  /* FAN control function by DHT22 Sensor */
  Serial.println("call Manual_Control_FAN() func");
  int32_t currentTime = fan_timer;
  
  while (currentTime > 0){
    SoftPWM.set(100);
    if (millis() - fan_ST > Delay_ms){
      Serial.println((String)"fan_timer: " + currentTime);
      currentTime -= 1;
      Fan_elapsedTime.Set((int)currentTime);
      fan_ST = millis();
    }
  }
  
  SoftPWM.set(0);
  
}

void Auto_Control_PUMP(int Soilhumi){
  /* PUMP control function by Soil Humidity Sensor */
  Serial.println("call Auto_Control_PUMP() func");
  Serial.println((String)"--- start_humi: " + start_humi + " end_humi: " + end_humi);
  
  if (interval_pump_status){
    if (Soilhumi > end_humi){ digitalWrite(PUMP, LOW); }
    else if (Soilhumi > start_humi){
      digitalWrite(PUMP, HIGH); delay(pump_interval*1000);
      digitalWrite(PUMP, LOW); delay(pump_interval*1000);
    }
    else { digitalWrite(PUMP, LOW); }
  }
  else {
    if (Soilhumi > end_humi){ digitalWrite(PUMP, LOW); }
    else if (Soilhumi > start_humi){ digitalWrite(PUMP, HIGH); }
    else { digitalWrite(PUMP, LOW); }
  }
  
}

void Manual_Control_PUMP(int32_t pump_timer){
  /* PUMP control function by Soil Humidity Sensor */
  Serial.println("call Manual_Control_PUMP() func");
  int32_t currentTime = pump_timer;
  
  while (currentTime > 0){
    digitalWrite(PUMP, HIGH);
    if (millis() - pump_ST > Delay_ms){
      Serial.println((String)"pump_timer: " + currentTime);
      currentTime -= 1;
      Pump_elapsedTime.Set((int)currentTime);
      pump_ST = millis();
    }
  }
  digitalWrite(PUMP, LOW);
  
}

void Auto_Control_LAMP(int32_t lamp_interval){
  /* Lamp control function by Timer */
  Serial.println((String)"call Auto_Control_LAMP() func");
  Serial.println((String)"lamp_interval: " + lamp_interval);

  if (millis() - lamp_ST > lamp_interval * Delay_ms){
      digitalWrite(LAMP, LOW);
      lamp_ST = millis();
    }
  else{ digitalWrite(LAMP, HIGH); }
  
}

void Manual_Control_LAMP(int32_t lamp_timer){
  /* Lamp control function by Timer */
  Serial.println("call Manual_Control_LAMP() func");
  int32_t currentTime = lamp_timer;

  while (currentTime > 0){
    digitalWrite(LAMP, HIGH);
    if (millis() - lamp_ST > Delay_ms){
      Serial.println((String)"lamp_timer: " + currentTime);
      currentTime -= 1;
      Lamp_elapsedTime.Set((int)currentTime);
      lamp_ST = millis();
    }
  }
  digitalWrite(LAMP, LOW);
  
}

void OLEDdraw(){
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_te);
  u8g2.drawStr(1, 15, "SMART FARM");
  u8g2.drawStr(1, 25, "T:"); u8g2.setCursor(20, 25); u8g2.print(Temp); u8g2.drawStr(50, 25, "\xb0 C");
  u8g2.drawStr(70, 25, "H:"); u8g2.setCursor(90, 25); u8g2.print(Humi); u8g2.drawStr(117, 25, "%");
  u8g2.drawStr(1, 36, " Soil Humi"); u8g2.setCursor(80, 36); u8g2.print(Soilhumi); u8g2.drawStr(95, 36, "%");
  u8g2.drawStr(1, 47, " FAN"); u8g2.setCursor(30, 47); u8g2.print(auto_fan_status);
  u8g2.drawStr(37, 47, " PUMP"); u8g2.setCursor(80, 47); u8g2.print(auto_pump_status);
  u8g2.drawStr(90, 47, " LED"); u8g2.setCursor(120, 47); u8g2.print(auto_lamp_status);
  
  u8g2.sendBuffer();
}
