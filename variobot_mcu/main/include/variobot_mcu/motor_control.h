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

#ifndef VARIOBOT_MCU__MOTOR_CONTROL_H_
#define VARIOBOT_MCU__MOTOR_CONTROL_H_

#include <cstdint>

inline constexpr int NUM_MOTORS = 4;

inline constexpr double MAX_WHEEL_VELOCITY = 31.41;

/// Motor index mapping: matches the order used in JointState messages.
enum MotorId : uint8_t
{
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  REAR_LEFT = 2,
  REAR_RIGHT = 3,
};

/// Initialize the DRV8912 motor driver and configure all 4 motors.
void motor_control_init();

/// Set the target velocity for a single motor.
/// @param motor  Which motor to set.
/// @param rad_s  Desired velocity in rad/s (positive = forward, negative = reverse).
void motor_control_set_velocity(MotorId motor, double rad_s);

/// Push any pending motor configuration changes to the DRV8912 over SPI.
void motor_control_update();

#endif  // VARIOBOT_MCU__MOTOR_CONTROL_H_
