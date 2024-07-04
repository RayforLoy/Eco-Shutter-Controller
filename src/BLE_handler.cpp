#include "BLE_handler.h"
#include "hwconf.h"
#include "stdint.h"
#include "common.h"


Preferences mySketchPrefs;


// parameters to go to preferences
unsigned long flash_delay_us = DEFAULT_SYNC_DELAY_US;
uint16_t flash_interval_ms = DEFAULT_SYNC_INTERVAL_MS;
unsigned long mag_action_duation_us = DEFAULT_MAG_ACT_DURATION_US;
uint8_t mag_push_force = DEFAULT_MAG_PUSH_FORCE;
uint8_t mag_pull_force = DEFAULT_MAG_PULL_FORCE;
long shutterOpenCompensation_us = DEFAULT_SHUTTER_OPEN_COMPENSTATION;


unsigned long countdown_ms = DEFAULT_SHUTTER_COUNTDOWN;
unsigned long shutterOpenInterval_us = DEFAULT_SHUTTER_INTERVAL;



void save_shutter_data(){
	mySketchPrefs.begin("myPrefs", false);
	mySketchPrefs.putUInt("flash_delay_us", flash_delay_us);
	mySketchPrefs.putUInt("flash_interval_ms", flash_interval_ms);
	mySketchPrefs.putUInt("mag_action_duation_us", mag_action_duation_us);
	mySketchPrefs.putUInt("mag_push_force", mag_push_force);
	mySketchPrefs.putUInt("mag_pull_force", mag_pull_force);
	mySketchPrefs.putLong("shutterOpenCompensation_us", shutterOpenCompensation_us);
	mySketchPrefs.putUInt("countdown_ms", countdown_ms);
	mySketchPrefs.putUInt("shutterOpenInterval_us", shutterOpenInterval_us);
	mySketchPrefs.end();

}

int load_shutter_data(){
	mySketchPrefs.begin("myPrefs", false);
	if(!mySketchPrefs.isKey("flash_delay_us")){
		DBGSERIAL.println("NVS NOT INITIALIZED!!!");
		mySketchPrefs.clear();
		mySketchPrefs.end();
		save_shutter_data();
		return -1;
	}
	flash_delay_us = mySketchPrefs.getUInt("flash_delay_us", DEFAULT_SYNC_DELAY_US);
	flash_interval_ms = mySketchPrefs.getUInt("flash_interval_ms", DEFAULT_SYNC_INTERVAL_MS);
	mag_action_duation_us = mySketchPrefs.getUInt("mag_action_duation_us", DEFAULT_MAG_ACT_DURATION_US);
	mag_push_force = mySketchPrefs.getUInt("mag_push_force", DEFAULT_MAG_PUSH_FORCE);
	mag_pull_force = mySketchPrefs.getUInt("mag_pull_force", DEFAULT_MAG_PULL_FORCE);
	shutterOpenCompensation_us = mySketchPrefs.getLong("shutterOpenCompensation_us", DEFAULT_SHUTTER_OPEN_COMPENSTATION);
	countdown_ms = mySketchPrefs.getUInt("countdown_ms", DEFAULT_SHUTTER_COUNTDOWN);
	shutterOpenInterval_us = mySketchPrefs.getUInt("shutterOpenInterval_us", DEFAULT_SHUTTER_INTERVAL);
	mySketchPrefs.end();
	return 0;

}









BLEServer *pServer;
BLEService *actionService, *configService;

BLECharacteristic *pCharacteristic_shutter_speed, *pCharacteristic_shutter_countdown, *pCharacteristic_shutter_action, *pCharacteristic_shutter_preview;
BLECharacteristic *pCharacteristic_sync_delay, *pCharacteristic_sync_inter, *pCharacteristic_mag_push_force, *pCharacteristic_mag_pull_force, *pCharacteristic_mag_act_duration, *pCharacteristic_mag_act_compensation;

class ActionCallback : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string uuid = (pCharacteristic->getUUID()).toString();
		DBGSERIAL.printf("SET A VALUE!!!: %s : %s\n", uuid.c_str(), pCharacteristic->getValue().c_str());
		std::string strBuf = pCharacteristic->getValue().c_str();
		try
		{
			if (uuid == CHARACTERISTIC_UUID_SHUTTER_COUNTDOWN)
			{

				if (std::stoul(strBuf) > MAX_TRIG_COUNTDOWN_S || std::stoul(strBuf) < 0)
				{
					pCharacteristic->setValue("INVALID VALUE");
				}
				countdown_ms = std::stoul(strBuf) * 1000;
				pCharacteristic->setValue(std::string(strBuf));
			}
			else if (uuid == CHARACTERISTIC_UUID_SHUTTER_SPEED)
			{
				if (strBuf.substr(-1, 1) == "s" || strBuf.substr(-1, 1) == "S")
				{
					shutterOpenInterval_us = std::stoul(strBuf.substr(0, strBuf.length() - 1)) * 1000000;
				}
				else
				{
					shutterOpenInterval_us = 1000000 / std::stoul(strBuf);
				}

				pCharacteristic->setValue("Hello World");
			}
			else if (uuid == CHARACTERISTIC_UUID_SHUTTER_ACTION)
			{
				if (pCharacteristic->getValue().c_str() == "1")
				{
					if( is_shutter_open() || !is_shutter_working() || !is_shutter_task_running()){
						pCharacteristic->setValue("SHUTTER STATUS ERROR");
						return;
					}
					shutter_action_countdown(countdown_ms, shutterOpenInterval_us + shutterOpenCompensation_us, get_shutter_action_task());
					pCharacteristic->setValue("0");
				}
			}
			else if (uuid == CHARACTERISTIC_UUID_SHUTTER_PREVIEW)
			{
				if(strBuf == "1" && !is_shutter_open() && !is_shutter_working() && !is_shutter_task_running()){
					open_shutter();
				}else if (strBuf == "0" && is_shutter_open() && !is_shutter_working() && !is_shutter_task_running()){
					close_shutter();
				}
			}
		}
		catch (std::exception &e)
		{
			pCharacteristic->setValue("INVALID VALUE");
		}
		save_shutter_data();
	}
	void onRead(BLECharacteristic *pCharacteristic)
	{
		std::string uuid = (pCharacteristic->getUUID()).toString();
		DBGSERIAL.printf("GET A VALUE!!!: %s : %s\n", uuid.c_str(), pCharacteristic->getValue().c_str());
	}
};

class ConfigCallback : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		std::string uuid = (pCharacteristic->getUUID()).toString();
		DBGSERIAL.printf("SET A VALUE!!!: %s : %s\n", uuid.c_str(), pCharacteristic->getValue().c_str());
		try{
			if (uuid == CHARACTERISTIC_UUID_SYNC_DELAY)
			{
				flash_delay_us = std::stoul(pCharacteristic->getValue());
				pCharacteristic->setValue(std::to_string(flash_delay_us).c_str());
			}
			else if (uuid == CHARACTERISTIC_UUID_SYNC_INTER)
			{
				flash_interval_ms = std::stoul(pCharacteristic->getValue());
				pCharacteristic->setValue(std::to_string(flash_interval_ms).c_str());

			}
			else if (uuid == CHARACTERISTIC_UUID_MAG_PUSH_FORCE)
			{
				if(std::stoul(pCharacteristic->getValue()) > 0 && std::stoul(pCharacteristic->getValue()) <= 255)
				{
					mag_push_force = std::stoul(pCharacteristic->getValue());
					pCharacteristic->setValue(std::to_string(mag_push_force).c_str());
				}
				else
					pCharacteristic->setValue("INVALID VALUE");
			}
			else if (uuid == CHARACTERISTIC_UUID_MAG_PULL_FORCE)
			{
				if(std::stoul(pCharacteristic->getValue()) > 0 && std::stoul(pCharacteristic->getValue()) <= 255)
				{
					mag_pull_force = std::stoul(pCharacteristic->getValue());
					pCharacteristic->setValue(std::to_string(mag_pull_force).c_str());
				}
				else
					pCharacteristic->setValue("INVALID VALUE");
			}
			else if (uuid == CHARACTERISTIC_UUID_MAG_ACT_DURATION)
			{
				mag_action_duation_us = std::stoul(pCharacteristic->getValue());
				pCharacteristic->setValue(std::to_string(mag_action_duation_us).c_str());
			}
			else if (uuid == CHARACTERISTIC_UUID_MAG_ACT_COMPENSTATION)
			{
				shutterOpenCompensation_us = std::stol(pCharacteristic->getValue());
				pCharacteristic->setValue(std::to_string(shutterOpenCompensation_us).c_str());
			}
		}
		catch (std::exception &e)
		{
			pCharacteristic->setValue("INVALID VALUE");
		}

		save_shutter_data();
	}
	void onRead(BLECharacteristic *pCharacteristic)
	{
		std::string uuid = (pCharacteristic->getUUID()).toString();
		DBGSERIAL.printf("GET A VALUE!!!: %s : %s\n", uuid.c_str(), pCharacteristic->getValue().c_str());
	}
};

int BLE_service_init()
{
	BLEDevice::init(BLE_NAME);

	pServer = BLEDevice::createServer();
	actionService = pServer->createService(SERVICE_UUID_ACTION);
	configService = pServer->createService(SERVICE_UUID_CONFIG);

	// actionService
	pCharacteristic_shutter_speed = actionService->createCharacteristic(
		CHARACTERISTIC_UUID_SHUTTER_SPEED,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_shutter_speed->setValue(std::to_string(DEFAULT_SHUTTER_SPEED).c_str());
	pCharacteristic_shutter_speed->setCallbacks(new ActionCallback());

	pCharacteristic_shutter_countdown = actionService->createCharacteristic(
		CHARACTERISTIC_UUID_SHUTTER_COUNTDOWN,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_shutter_countdown->setValue(std::to_string(countdown_ms / 1000).c_str());
	pCharacteristic_shutter_countdown->setCallbacks(new ActionCallback());

	pCharacteristic_shutter_action = actionService->createCharacteristic(
		CHARACTERISTIC_UUID_SHUTTER_ACTION,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_shutter_action->setValue("0");
	pCharacteristic_shutter_action->setCallbacks(new ActionCallback());

	pCharacteristic_shutter_preview = actionService->createCharacteristic(
		CHARACTERISTIC_UUID_SHUTTER_PREVIEW,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_shutter_preview->setValue("0");
	pCharacteristic_shutter_preview->setCallbacks(new ActionCallback());

	// configService
	pCharacteristic_sync_delay = configService->createCharacteristic(
		CHARACTERISTIC_UUID_SYNC_DELAY,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_sync_delay->setValue(std::to_string(flash_delay_us).c_str());
	pCharacteristic_sync_delay->setCallbacks(new ConfigCallback());

	pCharacteristic_sync_inter = configService->createCharacteristic(
		CHARACTERISTIC_UUID_SYNC_INTER,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_sync_inter->setValue(std::to_string(flash_interval_ms).c_str());
	pCharacteristic_sync_inter->setCallbacks(new ConfigCallback());

	pCharacteristic_mag_push_force = configService->createCharacteristic(
		CHARACTERISTIC_UUID_MAG_PUSH_FORCE,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_mag_push_force->setValue(std::to_string(mag_push_force).c_str());
	pCharacteristic_mag_push_force->setCallbacks(new ConfigCallback());

	pCharacteristic_mag_pull_force = configService->createCharacteristic(
		CHARACTERISTIC_UUID_MAG_PULL_FORCE,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_mag_pull_force->setValue(std::to_string(mag_pull_force).c_str());
	pCharacteristic_mag_pull_force->setCallbacks(new ConfigCallback());

	pCharacteristic_mag_act_duration = configService->createCharacteristic(
		CHARACTERISTIC_UUID_MAG_ACT_DURATION,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_mag_act_duration->setValue(std::to_string(mag_action_duation_us).c_str());
	pCharacteristic_mag_act_duration->setCallbacks(new ConfigCallback());

	pCharacteristic_mag_act_compensation = configService->createCharacteristic(
		CHARACTERISTIC_UUID_MAG_ACT_COMPENSTATION,
		BLECharacteristic::PROPERTY_READ |
			BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic_mag_act_compensation->setValue(std::to_string(shutterOpenCompensation_us).c_str());
	pCharacteristic_mag_act_compensation->setCallbacks(new ConfigCallback());

	actionService->start();
	configService->start();

	// BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID_ACTION);
	pAdvertising->addServiceUUID(SERVICE_UUID_CONFIG);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	BLEDevice::startAdvertising();

	return 0;
}

unsigned long get_flash_delay_us()
{
	return flash_delay_us;
}

unsigned long get_flash_interval_ms()
{
	return flash_interval_ms;
}

unsigned long get_mag_action_duation_us()
{
	return mag_action_duation_us;
}

uint8_t get_mag_push_force()
{
	return mag_push_force;
}

uint8_t get_mag_pull_force()
{
	return mag_pull_force;
}