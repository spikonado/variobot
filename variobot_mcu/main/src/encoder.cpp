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

#include <esp_timer.h>

#include <cstdint>

#include "Arduino.h"

// ── Encoder Configuration ───────────────────────────────────────────────────

struct EncoderPins
{
  int pin_a;  // Phase A (Interrupt source)
  int pin_b;  // Phase B (Direction signal)
};

static constexpr EncoderPins ENCODER_HW[NUM_MOTORS] = {
  {1, 2},  // FRONT_LEFT
  {3, 4},  // FRONT_RIGHT
  {5, 6},  // REAR_LEFT
  {7, 8}   // REAR_RIGHT
};

static constexpr uint16_t TICKS_PER_REV = 360;

// ── Internal State ─────────────────────────────────────────────────────────

static volatile int32_t encoder_ticks[NUM_MOTORS] = {0, 0, 0, 0};
static int32_t last_ticks[NUM_MOTORS] = {0, 0, 0, 0};
static int64_t last_velocity_update_us[NUM_MOTORS] = {0, 0, 0, 0};
static double current_velocity_rad_s[NUM_MOTORS] = {0.0, 0.0, 0.0, 0.0};

// ── Interrupt Service Routines ──────────────────────────────────────────────

static void IRAM_ATTR isr_fl()
{
  if (digitalRead(ENCODER_HW[FRONT_LEFT].pin_b) == HIGH)
    encoder_ticks[FRONT_LEFT] = encoder_ticks[FRONT_LEFT] + 1;
  else
    encoder_ticks[FRONT_LEFT] = encoder_ticks[FRONT_LEFT] - 1;
}

static void IRAM_ATTR isr_fr()
{
  if (digitalRead(ENCODER_HW[FRONT_RIGHT].pin_b) == HIGH)
    encoder_ticks[FRONT_RIGHT] = encoder_ticks[FRONT_RIGHT] + 1;
  else
    encoder_ticks[FRONT_RIGHT] = encoder_ticks[FRONT_RIGHT] - 1;
}

static void IRAM_ATTR isr_rl()
{
  if (digitalRead(ENCODER_HW[REAR_LEFT].pin_b) == HIGH)
    encoder_ticks[REAR_LEFT] = encoder_ticks[REAR_LEFT] + 1;
  else
    encoder_ticks[REAR_LEFT] = encoder_ticks[REAR_LEFT] - 1;
}

static void IRAM_ATTR isr_rr()
{
  if (digitalRead(ENCODER_HW[REAR_RIGHT].pin_b) == HIGH)
    encoder_ticks[REAR_RIGHT] = encoder_ticks[REAR_RIGHT] + 1;
  else
    encoder_ticks[REAR_RIGHT] = encoder_ticks[REAR_RIGHT] - 1;
}

// ── Implementation ──────────────────────────────────────────────────────────

void encoder_init()
{
  void (*isr_list[NUM_MOTORS])() = {isr_fl, isr_fr, isr_rl, isr_rr};

  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(ENCODER_HW[i].pin_a, INPUT_PULLUP);
    pinMode(ENCODER_HW[i].pin_b, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_HW[i].pin_a), isr_list[i], RISING);

    last_velocity_update_us[i] = esp_timer_get_time();
  }
}

double encoder_get_position_rad(MotorId motor)
{
  return static_cast<double>(encoder_ticks[motor]) * ((2.0 * PI) / TICKS_PER_REV);
}

double encoder_get_velocity_rad_s(MotorId motor)
{
  int64_t now_us = esp_timer_get_time();
  int64_t dt_us = now_us - last_velocity_update_us[motor];

  // Update velocity approximately every 10ms to avoid noise
  if (dt_us > 10000) {
    int32_t current_ticks = encoder_ticks[motor];
    int32_t delta_ticks = current_ticks - last_ticks[motor];

    current_velocity_rad_s[motor] =
      (static_cast<double>(delta_ticks) * ((2.0 * PI) / TICKS_PER_REV)) /
      (static_cast<double>(dt_us) / 1000000.0);

    last_ticks[motor] = current_ticks;
    last_velocity_update_us[motor] = now_us;
  }

  return current_velocity_rad_s[motor];
}

void encoder_reset(MotorId motor)
{
  encoder_ticks[motor] = 0;
  last_ticks[motor] = 0;
  last_velocity_update_us[motor] = esp_timer_get_time();
  current_velocity_rad_s[motor] = 0.0;
}
