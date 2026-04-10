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

#ifndef VARIOBOT_MCU__MOTOR_CONTROL_HPP_
#define VARIOBOT_MCU__MOTOR_CONTROL_HPP_

#include <cstdint>

inline constexpr uint8_t NUM_MOTORS = 4;

enum MotorId : uint8_t
{
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  REAR_LEFT = 2,
  REAR_RIGHT = 3,
};

/**
 * @brief Initialize the DRV8912 motor driver and configure motors
 */
void motor_control_init();

/**
 * @brief Set the target pwm for a single motor
 * @param [in] motor Which motor to set
 * @param [in] pwm Desired pwm from -255 to 255 (positive = anti-clockwise, negative = clockwise)
 */
void motor_control_set_pwm(MotorId motor, int16_t pwm);

/**
 * @brief Push any pending motor configuration changes to the DRV8912 over SPI
 */
void motor_control_update();

#endif  // VARIOBOT_MCU__MOTOR_CONTROL_HPP_
