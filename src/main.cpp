/*

Universal shutter controller


*/

#include "Arduino.h"
#include "BluetoothSerial.h"
#include "hwconf.h"
#include "common.h"



unsigned long system_time_millis  = 0;
TaskHandle_t* shutter_action_task;


int scanTime = 5; //In seconds
BLEScan* pBLEScan;


TaskHandle_t* get_shutter_action_task(){
  return shutter_action_task;
} 


void setup() {

#if ARDUINO_USB_DCD_ON_BOOT
  delay(3000); // allow usb cdc boot
#endif
  // system_time_millis = millis();
  DBGSERIAL.begin(115200);
  DBGSERIAL.println("Starting BLE work!");


  load_shutter_data();
  BLE_service_init();

  analogWriteFrequency(PWM_FREQ);
  analogWriteResolution(PWM_RESOLUTION);
  pinMode(PIN_MAG_PWM_A, OUTPUT);
  pinMode(PIN_MAG_PWM_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_FLASH_SYNC, OUTPUT);
  pinMode(PIN_MAG_EN, OUTPUT);

  digitalWrite(PIN_MAG_EN, HIGH);
  digitalWrite(PIN_FLASH_SYNC, LOW);
  digitalWrite(PIN_MAG_PWM_A, 0);
  digitalWrite(PIN_MAG_PWM_B, 0);
  digitalWrite(PIN_BUZZER, HIGH);

  // pinMode(PIN_TOUCH_1, INPUT);
  // pinMode(PIN_TOUCH_2, INPUT);
  // pinMode(PIN_TOUCH_3, INPUT);
  // pinMode(PIN_TOUCH_4, INPUT);
  // pinMode(PIN_BATT_ADC, INPUT);
  // pinMode(PIN_ADC1_CH3, INPUT);
  // pinMode(PIN_ADC2_CH4, INPUT);
  // pinMode(PIN_TXD_485, OUTPUT);
  // pinMode(PIN_RXD_485, INPUT);
  // pinMode(PIN_I2C_SCL, OUTPUT);
  // pinMode(PIN_I2C_SDA, OUTPUT);
  

}


// shutter logic running in ble handler
void loop() {
  if(is_shutter_task_ended()){
    vTaskDelete(*shutter_action_task);
  }

}