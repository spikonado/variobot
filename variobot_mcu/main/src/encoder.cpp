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

#include "variobot_mcu/encoder.h"

#include <driver/gpio.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp_timer.h>

#include <cmath>
#include <cstdint>

// ── Encoder Configuration ───────────────────────────────────────────────────

struct EncoderPins
{
  gpio_num_t pin_a;  // Phase A (Interrupt source)
  gpio_num_t pin_b;  // Phase B (Direction signal)
};

static constexpr EncoderPins ENCODER_HW[NUM_MOTORS] = {
  {GPIO_NUM_1, GPIO_NUM_2},   // FRONT_LEFT
  {GPIO_NUM_4, GPIO_NUM_5},   // FRONT_RIGHT
  {GPIO_NUM_6, GPIO_NUM_21},  // REAR_LEFT
  {GPIO_NUM_33, GPIO_NUM_34}  // REAR_RIGHT
};

static constexpr uint16_t TICKS_PER_REV = 360;

// ── Internal State ─────────────────────────────────────────────────────────

static volatile int32_t encoder_ticks[NUM_MOTORS] = {0, 0, 0, 0};
static int32_t last_ticks[NUM_MOTORS] = {0, 0, 0, 0};
static int64_t last_velocity_update_us[NUM_MOTORS] = {0, 0, 0, 0};
static double current_velocity_rad_s[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};

// ── Interrupt Service Routine ──────────────────────────────────────────────

static void IRAM_ATTR encoder_isr(void * arg)
{
  int motor = static_cast<int>(reinterpret_cast<intptr_t>(arg));
  if (gpio_get_level(ENCODER_HW[motor].pin_b)) {
    encoder_ticks[motor] = encoder_ticks[motor] + 1;
  } else {
    encoder_ticks[motor] = encoder_ticks[motor] - 1;
  }
}

// ── Implementation ──────────────────────────────────────────────────────────

void encoder_init()
{
  // Configure all Phase A and B pins as inputs with pullups
  gpio_config_t io_conf = {};
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

  // Configure Phase A pins with rising edge interrupt
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  uint64_t phase_a_mask = 0;
  for (int i = 0; i < NUM_MOTORS; i++) {
    phase_a_mask |= (1ULL << ENCODER_HW[i].pin_a);
  }
  io_conf.pin_bit_mask = phase_a_mask;
  gpio_config(&io_conf);

  // Configure Phase B pins without interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  uint64_t phase_b_mask = 0;
  for (int i = 0; i < NUM_MOTORS; i++) {
    phase_b_mask |= (1ULL << ENCODER_HW[i].pin_b);
  }
  io_conf.pin_bit_mask = phase_b_mask;
  gpio_config(&io_conf);

  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

  for (int i = 0; i < NUM_MOTORS; i++) {
    gpio_isr_handler_add(
      ENCODER_HW[i].pin_a, encoder_isr, reinterpret_cast<void *>(static_cast<intptr_t>(i)));
    last_velocity_update_us[i] = esp_timer_get_time();
  }
}

double encoder_get_position_rad(MotorId motor)
{
  return static_cast<double>(encoder_ticks[motor]) * ((2.0 * M_PI) / TICKS_PER_REV);
}

double encoder_get_velocity_rad_s(MotorId motor)
{
  int64_t now_us = esp_timer_get_time();
  int64_t dt_us = now_us - last_velocity_update_us[motor];

  if (dt_us == 0) return current_velocity_rad_s[motor];

  int32_t current_ticks = encoder_ticks[motor];
  int32_t delta_ticks = current_ticks - last_ticks[motor];

  current_velocity_rad_s[motor] =
    (static_cast<double>(delta_ticks) * ((2.0 * M_PI) / TICKS_PER_REV)) /
    (static_cast<double>(dt_us) / 1000000.0);

  last_ticks[motor] = current_ticks;
  last_velocity_update_us[motor] = now_us;

  return current_velocity_rad_s[motor];
}

void encoder_reset(MotorId motor)
{
  encoder_ticks[motor] = 0;
  last_ticks[motor] = 0;
  last_velocity_update_us[motor] = esp_timer_get_time();
  current_velocity_rad_s[motor] = 0.0;
}
