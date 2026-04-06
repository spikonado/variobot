// Copyright 2026 Aarav Gupta
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Arduino.h"
#include "DRV89xx.h"
#include "esp_log.h"  // NOLINT
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char * TAG = "info";

// DRV8912 Pin Definitions
const int chipSelectPin = 10;
const int sckPin = 12;
const int misoPin = 13;
const int mosiPin = 11;
const int nFaultPin = 47;
const int nSleepPin = 48;

DRV89xx motor_driver(chipSelectPin, nFaultPin, nSleepPin, sckPin, misoPin, mosiPin);

extern "C" void app_main(void)
{
  initArduino();

  motor_driver.configMotor(0, 1, 2, 0, 0);
  motor_driver.begin();
  motor_driver.readErrorStatus(true, false);
  motor_driver.setMotor(0, 255, DRV89xx_FORWARD);
  motor_driver.updateConfig();

  while (1) {
    motor_driver.readErrorStatus(true, false);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
