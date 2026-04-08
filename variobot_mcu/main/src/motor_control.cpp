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

#include "variobot_mcu/motor_control.h"

#include <esp_log.h>

#include <algorithm>
#include <cmath>

#include "DRV89xx.h"

static const char * TAG = "motor_control";

// ── DRV8912 SPI pin definitions ─────────────────────────────────────────────
static constexpr int PIN_CS = 10;
static constexpr int PIN_SCK = 12;
static constexpr int PIN_MISO = 13;
static constexpr int PIN_MOSI = 11;
static constexpr int PIN_NFAULT = 47;
static constexpr int PIN_NSLEEP = 48;

// ── Half-bridge and PWM channel assignments per motor ───────────────────────
struct MotorHardwareConfig
{
  uint8_t drv_motor_id;
  uint8_t half_bridge_1;
  uint8_t half_bridge_2;
  uint8_t pwm_channel;
  uint8_t reverse_delay;
};
static constexpr MotorHardwareConfig MOTOR_HW[NUM_MOTORS] = {
  {0, 1, 2, 0, 0},  // FRONT_LEFT
  {1, 3, 4, 1, 0},  // FRONT_RIGHT
  {2, 5, 6, 2, 0},  // REAR_LEFT
  {3, 7, 8, 3, 0},  // REAR_RIGHT
};

static DRV89xx driver(PIN_CS, PIN_NFAULT, PIN_NSLEEP, PIN_SCK, PIN_MISO, PIN_MOSI);

void motor_control_init()
{
  for (int i = 0; i < NUM_MOTORS; ++i) {
    const auto & hw = MOTOR_HW[i];
    driver.configMotor(
      hw.drv_motor_id, hw.half_bridge_1, hw.half_bridge_2, hw.pwm_channel, hw.reverse_delay);
  }

  driver.begin();
  driver.readErrorStatus(true, false);
  ESP_LOGI(TAG, "DRV8912 initialized with %d motors", NUM_MOTORS);
}

void motor_control_set_velocity(MotorId motor, double rad_s)
{
  // Clamp to maximum velocity.
  const double clamped = std::clamp(rad_s, -MAX_WHEEL_VELOCITY, MAX_WHEEL_VELOCITY);

  // Map |velocity| linearly to 0-255 PWM duty.
  const uint8_t speed = static_cast<uint8_t>((std::abs(clamped) / MAX_WHEEL_VELOCITY) * 255.0);

  uint8_t direction;
  if (speed == 0) {
    direction = DRV89xx_BRAKE;
  } else if (clamped > 0.0) {
    direction = DRV89xx_FORWARD;
  } else {
    direction = DRV89xx_REVERSE;
  }

  driver.setMotor(MOTOR_HW[motor].drv_motor_id, speed, direction);
}

void motor_control_update() { driver.updateConfig(); }
