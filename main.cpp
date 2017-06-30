/* mbed Microcontroller Library
* Copyright (c) 2006-2015 ARM Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "main.h"
//P0_4 for SPO2 LED control
DigitalOut spoLed(P0_4);


DigitalOut led1(LED1, 1);

const static char     DEVICE_NAME[] = "HRSP";
static const uint16_t uuid16_list[] = { GattService::UUID_HEART_RATE_SERVICE };

static EventQueue eventQueue(/* event count */ 16 * EVENTS_EVENT_SIZE);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
	spoLed = 0;
	BLE::Instance().gap().startAdvertising(); // restart advertising
}

void periodicCallback(void)
{
	if (BLE::Instance().getGapState().connected) {
		spoLed = 1;
		eventQueue.call(loop);
		eventQueue.call(updateSensorValue);
	}
}

void onBleInitError(BLE &ble, ble_error_t error)
{
	(void)ble;
	(void)error;
	/* Initialization error handling should go here */
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
	BLE&        ble = params->ble;
	ble_error_t error = params->error;

	if (error != BLE_ERROR_NONE) {
		onBleInitError(ble, error);
		return;
	}

	if (ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
		return;
	}

	ble.gap().onDisconnection(disconnectionCallback);

	/* Setup primary service. */
	hrServicePtr = new HeartRateService(ble, hrmCounter, HeartRateService::LOCATION_FINGER);

	/* Setup advertising. */
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_HEART_RATE_SENSOR);
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
	ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
	ble.gap().setAdvertisingInterval(1000); /* 1000ms */
	ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
	BLE &ble = BLE::Instance();
	eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main() {
	spoLed = 1;
	eventQueue.call_every(500, periodicCallback);

	BLE &ble = BLE::Instance();
	ble.onEventsToProcess(scheduleBleEventsProcessing);
	ble.init(bleInitComplete);

	eventQueue.dispatch_forever();
}
