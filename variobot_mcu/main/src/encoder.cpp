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

#include <driver/pulse_cnt.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <hal/gpio_types.h>

#include <cmath>
#include <cstdint>

static const char * TAG = "encoder";

struct EncoderPins
{
  gpio_num_t pin_a;  // Phase A (edge / pulse)
  gpio_num_t pin_b;  // Phase B (level / direction)
};

static constexpr EncoderPins ENCODER_HW[NUM_MOTORS] = {
  {GPIO_NUM_1, GPIO_NUM_2},   // FRONT_LEFT
  {GPIO_NUM_4, GPIO_NUM_5},   // FRONT_RIGHT
  {GPIO_NUM_6, GPIO_NUM_21},  // REAR_LEFT
  {GPIO_NUM_33, GPIO_NUM_34}  // REAR_RIGHT
};

static constexpr uint16_t TICKS_PER_REV = 690;
static constexpr uint16_t TICKS_PER_REV_2 = 2070;

// Full 16-bit PCNT range so overflow compensation interrupts stay rare.
static constexpr int PCNT_HIGH_LIMIT = 32767;
static constexpr int PCNT_LOW_LIMIT = -32768;
static constexpr uint32_t PCNT_GLITCH_FILTER_NS = 1000;

static pcnt_unit_handle_t pcnt_units[NUM_MOTORS] = {};
static int last_count[NUM_MOTORS] = {};
static double previous_pos[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};
static double previous_time[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};
static double current_velocity[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};

static double now_seconds() { return static_cast<double>(esp_timer_get_time()) * 1e-6; }

static uint16_t ticks_per_revolution(MotorId motor)
{
  return motor == REAR_RIGHT ? TICKS_PER_REV_2 : TICKS_PER_REV;
}

static int read_count(MotorId motor)
{
  int count = 0;
  const esp_err_t err = pcnt_unit_get_count(pcnt_units[motor], &count);
  if (err != ESP_OK) {
    ESP_LOGE(
      TAG, "Failed to read PCNT count for motor %d: %s", static_cast<int>(motor),
      esp_err_to_name(err));
    return last_count[motor];
  }
  last_count[motor] = count;
  return count;
}

static void install_encoder(MotorId motor)
{
  const EncoderPins & hw = ENCODER_HW[motor];

  pcnt_unit_config_t unit_config = {};
  unit_config.high_limit = PCNT_HIGH_LIMIT;
  unit_config.low_limit = PCNT_LOW_LIMIT;
  unit_config.flags.accum_count = 1;
  ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_units[motor]));

  const pcnt_glitch_filter_config_t filter_config = {
    .max_glitch_ns = PCNT_GLITCH_FILTER_NS,
  };
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_units[motor], &filter_config));

  pcnt_chan_config_t chan_config = {};
  chan_config.edge_gpio_num = hw.pin_a;
  chan_config.level_gpio_num = hw.pin_b;
  pcnt_channel_handle_t channel = nullptr;
  ESP_ERROR_CHECK(pcnt_new_channel(pcnt_units[motor], &chan_config, &channel));

  // 1x quadrature, matching the previous GPIO ISR: count on rising A, invert
  // when B is high so direction comes from the level signal.
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
    channel, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
  ESP_ERROR_CHECK(pcnt_channel_set_level_action(
    channel, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

  ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_units[motor], PCNT_HIGH_LIMIT));
  ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_units[motor], PCNT_LOW_LIMIT));
  ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_units[motor]));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_units[motor]));
  ESP_ERROR_CHECK(pcnt_unit_start(pcnt_units[motor]));
}

void encoder_init()
{
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    const MotorId motor = static_cast<MotorId>(i);
    install_encoder(motor);
    previous_time[i] = now_seconds();
  }
  ESP_LOGI(TAG, "PCNT encoders initialized for %d motors", NUM_MOTORS);
}

double encoder_get_position(MotorId motor)
{
  return static_cast<double>(read_count(motor)) * ((2.0 * M_PI) / ticks_per_revolution(motor));
}

double encoder_get_velocity(MotorId motor)
{
  const double current_time = now_seconds();
  const double dt = current_time - previous_time[motor];

  if (std::fabs(dt) < 1e-6) {
    return current_velocity[motor];
  }

  const double current_pos = encoder_get_position(motor);
  current_velocity[motor] = (current_pos - previous_pos[motor]) / dt;
  previous_pos[motor] = current_pos;
  previous_time[motor] = current_time;

  return current_velocity[motor];
}

void encoder_reset(MotorId motor)
{
  ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_units[motor]));
  last_count[motor] = 0;
  previous_pos[motor] = 0.0;
  previous_time[motor] = now_seconds();
  current_velocity[motor] = 0.0;
}
