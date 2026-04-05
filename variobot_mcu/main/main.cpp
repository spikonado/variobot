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
#include "esp_log.h"  // NOLINT
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char * TAG = "hello";

extern "C" void app_main(void)
{
  initArduino();

  while (1) {
    ESP_LOGI(TAG, "Hello world!");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
