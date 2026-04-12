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

#include "variobot_mcu/encoder.hpp"

#include <driver/gpio.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp_timer.h>

#include <cmath>
#include <cstdint>

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

static constexpr uint16_t TICKS_PER_REV = 690;
static constexpr uint16_t TICKS_PER_REV_2 = 2070;

static volatile int32_t encoder_ticks[NUM_MOTORS] = {0, 0, 0, 0};
static double previous_pos[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};
static double previous_time[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};
static double current_velocity[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};

static void IRAM_ATTR encoder_isr(void * arg)
{
  int motor = static_cast<int>(reinterpret_cast<intptr_t>(arg));
  if (gpio_get_level(ENCODER_HW[motor].pin_b)) {
    encoder_ticks[motor] = encoder_ticks[motor] - 1;
  } else {
    encoder_ticks[motor] = encoder_ticks[motor] + 1;
  }
}

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
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    phase_a_mask |= (1ULL << ENCODER_HW[i].pin_a);
  }
  io_conf.pin_bit_mask = phase_a_mask;
  gpio_config(&io_conf);

  // Configure Phase B pins without interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  uint64_t phase_b_mask = 0;
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    phase_b_mask |= (1ULL << ENCODER_HW[i].pin_b);
  }
  io_conf.pin_bit_mask = phase_b_mask;
  gpio_config(&io_conf);

  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    gpio_isr_handler_add(
      ENCODER_HW[i].pin_a, encoder_isr, reinterpret_cast<void *>(static_cast<intptr_t>(i)));
    previous_time[i] = static_cast<double>(esp_timer_get_time()) / 1000000.0;
  }
}

double encoder_get_position(MotorId motor)
{
  if (motor == REAR_RIGHT) {
    return static_cast<double>(encoder_ticks[motor]) * ((2.0 * M_PI) / TICKS_PER_REV_2);
  }
  return static_cast<double>(encoder_ticks[motor]) * ((2.0 * M_PI) / TICKS_PER_REV);
}

double encoder_get_velocity(MotorId motor)
{
  double current_time = static_cast<double>(esp_timer_get_time()) / 1000000.0;
  double dt = current_time - previous_time[motor];

  if (std::fabs(dt) < 1e-6) return current_velocity[motor];

  double current_pos = encoder_get_position(motor);

  current_velocity[motor] = (current_pos - previous_pos[motor]) / dt;

  previous_pos[motor] = current_pos;
  previous_time[motor] = current_time;

  return current_velocity[motor];
}

void encoder_reset(MotorId motor)
{
  encoder_ticks[motor] = 0;
  previous_pos[motor] = 0.0;
  previous_time[motor] = esp_timer_get_time();
}
