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

#ifndef VARIOBOT_MCU__ENCODER_HPP_
#define VARIOBOT_MCU__ENCODER_HPP_

#include "variobot_mcu/motor_control.hpp"

/**
 * @brief Initialize encoders using the ESP-IDF pulse counter peripheral
 */
void encoder_init();

/**
 * @brief Get the current joint position in radians
 * @param motor Which motor's position to retrieve
 */
double encoder_get_position(MotorId motor);

/**
 * @brief Get the current joint velocity in radians per second
 * @param motor Which motor's velocity to retrieve
 */
double encoder_get_velocity(MotorId motor);

/**
 * @brief Reset the tick count for a specific motor
 */
void encoder_reset(MotorId motor);

#endif  // VARIOBOT_MCU__ENCODER_HPP_
