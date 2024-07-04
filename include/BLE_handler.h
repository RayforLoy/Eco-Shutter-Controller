#pragma once
#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include "stdint.h"







// action service
#define SERVICE_UUID_ACTION                         "4fafc201-1fb5-459e-8fcc-000000000001"

#define CHARACTERISTIC_UUID_SHUTTER_COUNTDOWN       "beb5483e-36e1-4688-b7f5-010000000001"
#define CHARACTERISTIC_UUID_SHUTTER_SPEED           "beb5483e-36e1-4688-b7f5-010000000002"
#define CHARACTERISTIC_UUID_SHUTTER_ACTION          "beb5483e-36e1-4688-b7f5-010000000003"
#define CHARACTERISTIC_UUID_SHUTTER_PREVIEW         "beb5483e-36e1-4688-b7f5-010000000004"

// configuration service
#define SERVICE_UUID_CONFIG                         "4fafc201-1fb5-459e-8fcc-000000000002"

#define CHARACTERISTIC_UUID_SYNC_DELAY              "beb5483e-36e1-4688-b7f5-000000000001"
#define CHARACTERISTIC_UUID_SYNC_INTER              "beb5483e-36e1-4688-b7f5-000000000002"
#define CHARACTERISTIC_UUID_MAG_PUSH_FORCE          "beb5483e-36e1-4688-b7f5-000000000003"
#define CHARACTERISTIC_UUID_MAG_PULL_FORCE          "beb5483e-36e1-4688-b7f5-000000000004"
#define CHARACTERISTIC_UUID_MAG_ACT_DURATION        "beb5483e-36e1-4688-b7f5-000000000005"
#define CHARACTERISTIC_UUID_MAG_ACT_COMPENSTATION   "beb5483e-36e1-4688-b7f5-000000000006"



uint8_t get_mag_push_force();
uint8_t get_mag_pull_force();

unsigned long get_flash_delay_us();
unsigned long get_flash_interval_ms();
unsigned long get_mag_action_duation_us();
int BLE_service_init();

int load_shutter_data();
void save_shutter_data();





#endif
