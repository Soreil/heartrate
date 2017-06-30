#pragma once
#include <mbed.h>
#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/services/HeartRateService.h"
#include <cstdint>

static HeartRateService *hrServicePtr;
static uint8_t hrmCounter = 100; // init HRM to 100bps


const int minute = 60;
const int lowerLimit = 30;
const int upperLimit = 230;
const int lowerMargin = -15;
const int upperMargin = 15;
const int peaksSampled = 4;

void updateSensorValue();
void loop();