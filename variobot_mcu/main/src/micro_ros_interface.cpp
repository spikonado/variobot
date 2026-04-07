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

#include "variobot_mcu/micro_ros_interface.h"

#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "rclc/executor.h"
#include "rclc/init.h"
#include "rclc/node.h"
#include "rclc/publisher.h"
#include "rclc/subscription.h"
#include "rclc/timer.h"
#include "rclc/types.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "sensor_msgs/msg/joint_state.h"
#include "uros_network_interfaces.h"  // NOLINT
#include "variobot_mcu/encoder.h"
#include "variobot_mcu/motor_control.h"

static const char * TAG = "uros_interface";

#define RCCHECK(fn)                                                                     \
  {                                                                                     \
    rcl_ret_t temp_rc = fn;                                                             \
    if ((temp_rc != RCL_RET_OK)) {                                                      \
      ESP_LOGE(TAG, "Failed status on line %d: %d. Aborting.", __LINE__, (int)temp_rc); \
      vTaskDelete(NULL);                                                                \
    }                                                                                   \
  }
#define RCSOFTCHECK(fn)                                                                   \
  {                                                                                       \
    rcl_ret_t temp_rc = fn;                                                               \
    if ((temp_rc != RCL_RET_OK)) {                                                        \
      ESP_LOGW(TAG, "Failed status on line %d: %d. Continuing.", __LINE__, (int)temp_rc); \
    }                                                                                     \
  }

// ── Micro-ROS Entities ─────────────────────────────────────────────────────

static rcl_publisher_t state_publisher;
static rcl_subscription_t command_subscriber;
static sensor_msgs__msg__JointState state_msg;
static sensor_msgs__msg__JointState command_msg;

static const char * joint_names[NUM_MOTORS] = {
  "front_left_coupling_joint", "front_right_coupling_joint", "rear_left_coupling_joint",
  "rear_right_coupling_joint"};

// ── Callbacks ─────────────────────────────────────────────────────────────

void command_callback(const void * msgin)
{
  const sensor_msgs__msg__JointState * msg = (const sensor_msgs__msg__JointState *)msgin;

  // Joint names should match. We expect exactly 4 joints or iterate to find matches.
  for (size_t i = 0; i < msg->name.size; i++) {
    for (int j = 0; j < NUM_MOTORS; j++) {
      if (strcmp(msg->name.data[i].data, joint_names[j]) == 0) {
        if (msg->velocity.size > i) {
          motor_control_set_velocity(static_cast<MotorId>(j), msg->velocity.data[i]);
        }
        break;
      }
    }
  }
  motor_control_update();
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  (void)last_call_time;
  if (timer != NULL) {
    // Update header timestamp using gettimeofday (available in ESP-IDF/Newlib)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    state_msg.header.stamp.sec = tv.tv_sec;
    state_msg.header.stamp.nanosec = tv.tv_usec * 1000;

    for (int i = 0; i < NUM_MOTORS; i++) {
      state_msg.position.data[i] = encoder_get_position_rad(static_cast<MotorId>(i));
      state_msg.velocity.data[i] = encoder_get_velocity_rad_s(static_cast<MotorId>(i));
    }

    RCSOFTCHECK(rcl_publish(&state_publisher, &state_msg, NULL));
  }
}

// ── Micro-ROS Task ──────────────────────────────────────────────────────────

static void micro_ros_task(void * arg)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

// Transport handled based on Kconfig.
#if defined(CONFIG_MICRO_ROS_ESP_NETIF_WLAN) || defined(CONFIG_MICRO_ROS_ESP_NETIF_ENET)
  ESP_ERROR_CHECK(uros_network_interface_initialize());
#endif

  // Initialize Support
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Initialize Node
  rcl_node_t node;
  RCCHECK(rclc_node_init_default(&node, "variobot_mcu", "", &support));

  // Initialize Messages (Pre-allocate sequences)
  sensor_msgs__msg__JointState__init(&state_msg);
  rosidl_runtime_c__String__Sequence__init(&state_msg.name, NUM_MOTORS);
  rosidl_runtime_c__double__Sequence__init(&state_msg.position, NUM_MOTORS);
  rosidl_runtime_c__double__Sequence__init(&state_msg.velocity, NUM_MOTORS);

  for (int i = 0; i < NUM_MOTORS; i++) {
    rosidl_runtime_c__String__assign(&state_msg.name.data[i], joint_names[i]);
  }
  state_msg.position.size = NUM_MOTORS;
  state_msg.velocity.size = NUM_MOTORS;

  sensor_msgs__msg__JointState__init(&command_msg);
  rosidl_runtime_c__String__Sequence__init(&command_msg.name, NUM_MOTORS);
  rosidl_runtime_c__double__Sequence__init(&command_msg.velocity, NUM_MOTORS);
  for (int i = 0; i < NUM_MOTORS; i++) {
    rosidl_runtime_c__String__init(&command_msg.name.data[i]);
  }

  // Initialize Publisher
  RCCHECK(rclc_publisher_init_default(
    &state_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
    "joint_states"));

  // Initialize Subscriber
  RCCHECK(rclc_subscription_init_default(
    &command_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
    "joint_commands"));

  // Initialize Timer (1000Hz)
  rcl_timer_t timer;
  RCCHECK(rclc_timer_init_default2(&timer, &support, RCL_MS_TO_NS(1), timer_callback, true));

  // Initialize Executor
  rclc_executor_t executor;
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
  RCCHECK(rclc_executor_add_subscription(
    &executor, &command_subscriber, &command_msg, &command_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // Main Loop
  while (1) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    usleep(10000);
  }

  // Free resources (though we spin forever)
  sensor_msgs__msg__JointState__fini(&state_msg);
  sensor_msgs__msg__JointState__fini(&command_msg);
  RCCHECK(rcl_subscription_fini(&command_subscriber, &node));
  RCCHECK(rcl_publisher_fini(&state_publisher, &node));
  RCCHECK(rcl_node_fini(&node));
  RCCHECK(rclc_support_fini(&support));

  vTaskDelete(NULL);
}

void micro_ros_init()
{
  xTaskCreate(
    micro_ros_task, "uros_task", CONFIG_MICRO_ROS_APP_STACK, NULL, CONFIG_MICRO_ROS_APP_TASK_PRIO,
    NULL);
}
