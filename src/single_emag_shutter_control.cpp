#include "common.h"



bool shutter_is_working = false;
bool shutter_is_open = false;
bool shutter_task_running = false;
bool shutter_task_ended = false;

static void TASK_SINGE_EMAG_SHUITTER_ASCTION(void *  shutter_action_para);


bool is_shutter_open(){
    return shutter_is_open;
}

bool is_shutter_working(){
    return shutter_is_working;
}

bool is_shutter_task_running(){
    return shutter_task_running;
}

bool is_shutter_task_ended(){
    return shutter_task_ended;
}

int shutter_action_countdown(uint32_t countdown_ms, uint32_t shutterOpenInterval_us, TaskHandle_t* shutter_action_task){
    if(is_shutter_working() || is_shutter_task_running())
        return -1;

    shutter_task_ended = false;
    uint32_t shutter_action_para[] = {countdown_ms, shutterOpenInterval_us};
    xTaskCreatePinnedToCore(TASK_SINGE_EMAG_SHUITTER_ASCTION,   // Function that should be called
                            "TASK_SINGE_EMAG_SHUITTER_ASCTION", // Name of the task (for debugging)
                            1024 * 4,                           // Stack size (bytes)
                            (void*) shutter_action_para,               //  Parameter to pass
                            1,                                  // Task priority
                            shutter_action_task,                // Task handle
                            0                                   // Core you want to run the task on (0 or 1)
                            );

    return 0;


}



void open_shutter(){
    shutter_is_working = true;
    analogWrite(PIN_MAG_PWM_A, 0);
    analogWrite(PIN_MAG_PWM_B, get_mag_push_force());
    delayMicroseconds( get_mag_action_duation_us());
    analogWrite(PIN_MAG_PWM_A, 0);
    analogWrite(PIN_MAG_PWM_B, 0);
    delayMicroseconds(MAG_ACT_DEAD_ZONE_US);
    shutter_is_open = true;
    shutter_is_working = false;

}

void close_shutter(){
    shutter_is_working = true;
    analogWrite(PIN_MAG_PWM_A, get_mag_pull_force());
    analogWrite(PIN_MAG_PWM_B, 0);
    delayMicroseconds( get_mag_action_duation_us());
    analogWrite(PIN_MAG_PWM_A, 0);
    analogWrite(PIN_MAG_PWM_B, 0);
    shutter_is_open = false;
    shutter_is_working = false;
}


static void TASK_SINGE_EMAG_SHUITTER_ASCTION(void * shutter_action_para){
    if(is_shutter_working())
        return;
    if(is_shutter_working()){
        for (size_t i = 0; i < 3; i++)
        {
            digitalWrite(PIN_BUZZER, LOW);
            delay(180);
            digitalWrite(PIN_BUZZER, HIGH);
            delay(500);
        }
        return;
    }

    shutter_task_running = true;
    unsigned long countdown_ms = ((uint32_t*)shutter_action_para)[0]; 
    if(countdown_ms == 0)
        digitalWrite(PIN_BUZZER, LOW);

    while(countdown_ms > 0){
        if(countdown_ms > 10*1000){
            digitalWrite(PIN_BUZZER, LOW);
            delay(180);
            digitalWrite(PIN_BUZZER, HIGH);
            delay(2000-180);
            countdown_ms -= 2000;
        }else if(countdown_ms > 1000){
            u16_t notifyTime = countdown_ms/5;
            digitalWrite(PIN_BUZZER, LOW);
            delay(180);
            digitalWrite(PIN_BUZZER, HIGH);
            delay(notifyTime-180);
            countdown_ms -= notifyTime;
        }else{
            digitalWrite(PIN_BUZZER, LOW);
            delay(1000);
            countdown_ms = 0;
        }
    }

    unsigned long start_time = micros();
    u8_t flashed = 0;
    u8_t shutterAction = 0;
    unsigned long shutterActionTime = ((uint32_t*)shutter_action_para)[1];
    shutterActionTime = shutterActionTime > MAG_ACT_DEAD_ZONE_US ? shutterActionTime : MAG_ACT_DEAD_ZONE_US;
    for(;;){
        unsigned long current_time = micros();

        // flahs sync handler
        if(flashed == 0 && current_time - start_time > get_flash_delay_us()){
            flashed = 1;
            digitalWrite(PIN_FLASH_SYNC, HIGH);
            start_time = current_time;
        }else if(flashed == 1 && current_time - start_time > get_flash_interval_ms()*1000){
            flashed = 2;
            digitalWrite(PIN_FLASH_SYNC, LOW);
            start_time = current_time;
        }

        // shutter action handler
        if(shutterAction == 0){

            // open shutter
            shutterAction = 1;
            analogWrite(PIN_MAG_PWM_A, 0);
            analogWrite(PIN_MAG_PWM_B, get_mag_push_force());
            shutter_is_working = true;
            start_time = current_time;
        }else if(shutterAction == 1 && current_time - start_time > get_mag_action_duation_us()){

            // wait for shutter time
            shutter_is_open = true;
            digitalWrite(PIN_BUZZER, HIGH);
            shutterAction = 2;
            analogWrite(PIN_MAG_PWM_A, 0);
            analogWrite(PIN_MAG_PWM_B, 0);
            shutter_is_working = false;
            start_time = current_time;
        }else if(shutterAction == 2 && current_time - start_time >  shutterActionTime){

            // close shutter
            shutterAction = 3;
            analogWrite(PIN_MAG_PWM_A, get_mag_pull_force());
            analogWrite(PIN_MAG_PWM_B, 0);
            digitalWrite(PIN_FLASH_SYNC, LOW);
            shutter_is_working = true;
            start_time = current_time;
        }else if(shutterAction == 3 && current_time - start_time > get_mag_action_duation_us()){

            // end shutter action
            shutterAction = 4;
            analogWrite(PIN_MAG_PWM_A, 0);
            analogWrite(PIN_MAG_PWM_B, 0);
            shutter_is_open = false;
            shutter_is_working = false;
            start_time = current_time;
            break;


        }
    }



    digitalWrite(PIN_BUZZER, LOW);
    delay(180);
    digitalWrite(PIN_BUZZER, HIGH);
    delay(500);
    digitalWrite(PIN_BUZZER, LOW);
    delay(180);
    digitalWrite(PIN_BUZZER, HIGH);

    shutter_task_running = false;
    shutter_task_ended = true;
}