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

// cppcheck-suppress-file normalCheckLevelMaxBranches
#include "variobot_mcu/micro_ros_interface.hpp"

#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rcl/timer.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <cmath>
#include <cstdint>

#include "micro_ros_utilities/type_utilities.h"
#include "rclc/executor.h"
#include "rclc/init.h"
#include "rclc/node.h"
#include "rclc/publisher.h"
#include "rclc/subscription.h"
#include "rclc/timer.h"
#include "rclc/types.h"
#include "rmw_microros/init_options.h"
#include "rmw_microros/ping.h"
#include "rmw_microros/time_sync.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "sensor_msgs/msg/joint_state.h"
#include "uros_network_interfaces.h"  // NOLINT
#include "variobot_mcu/encoder.hpp"
#include "variobot_mcu/motor_control.hpp"

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

rcl_publisher_t joint_state_publisher;
rcl_subscription_t joint_command_subscriber;
sensor_msgs__msg__JointState joint_state_msg;
sensor_msgs__msg__JointState joint_command_msg;
uint8_t joint_command_buffer[512];

const char * joint_names[NUM_MOTORS] = {
  "front_left_coupling_joint", "front_right_coupling_joint", "rear_left_coupling_joint",
  "rear_right_coupling_joint"};

bool init_joint_state_message(sensor_msgs__msg__JointState * msg)
{
  if (!sensor_msgs__msg__JointState__init(msg)) {
    return false;
  }

  if (!rosidl_runtime_c__String__Sequence__init(&msg->name, NUM_MOTORS)) {
    return false;
  }

  if (!rosidl_runtime_c__double__Sequence__init(&msg->position, NUM_MOTORS)) {
    return false;
  }

  if (!rosidl_runtime_c__double__Sequence__init(&msg->velocity, NUM_MOTORS)) {
    return false;
  }

  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    if (!rosidl_runtime_c__String__assign(&msg->name.data[i], joint_names[i])) {
      return false;
    }
  }

  return true;
}

micro_ros_utilities_memory_conf_t make_joint_command_memory_conf()
{
  micro_ros_utilities_memory_conf_t conf{};
  conf.max_string_capacity = 48;
  conf.max_ros2_type_sequence_capacity = NUM_MOTORS;
  conf.max_basic_type_sequence_capacity = NUM_MOTORS;
  return conf;
}

bool init_joint_command_message(sensor_msgs__msg__JointState * msg)
{
  if (!sensor_msgs__msg__JointState__init(msg)) {
    return false;
  }

  const micro_ros_utilities_memory_conf_t conf = make_joint_command_memory_conf();

  return micro_ros_utilities_create_static_message_memory(
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState), msg, conf, joint_command_buffer,
    sizeof(joint_command_buffer));
}

void joint_command_callback(const void * msgin)
{
  const sensor_msgs__msg__JointState * msg = (const sensor_msgs__msg__JointState *)msgin;

  for (size_t i = 0; i < msg->name.size; i++) {
    for (uint8_t j = 0; j < NUM_MOTORS; j++) {
      if (strcmp(msg->name.data[i].data, joint_names[j]) == 0) {
        if (msg->velocity.size > i) {
          const double velocity = msg->velocity.data[i];
          const int16_t pwm = std::isfinite(velocity) ? static_cast<int16_t>(velocity) : 0;
          motor_control_set_pwm(static_cast<MotorId>(j), pwm);
        }
        break;
      }
    }
  }
  motor_control_update();
}

void joint_state_timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    rmw_uros_sync_session(1000);
    joint_state_msg.header.stamp.sec = static_cast<int32_t>(rmw_uros_epoch_millis() / 1000);
    joint_state_msg.header.stamp.nanosec =
      static_cast<uint32_t>((rmw_uros_epoch_millis() % 1000) * 1000000);

    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
      joint_state_msg.position.data[i] = encoder_get_position(static_cast<MotorId>(i));
      joint_state_msg.velocity.data[i] = encoder_get_velocity(static_cast<MotorId>(i));
    }

    RCSOFTCHECK(rcl_publish(&joint_state_publisher, &joint_state_msg, NULL));
  }
}

void micro_ros_task(void * arg)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  RCCHECK(rcl_init_options_init(&init_options, allocator));

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
  rmw_init_options_t * rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

  // RCCHECK(rmw_uros_discover_agent(rmw_options));

  // Static agent IP and port can be used instead of auto-discovery
  RCCHECK(rmw_uros_options_set_udp_address(
    CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT, rmw_options));

  while (rmw_uros_ping_agent_options(1000, 1, rmw_options) != RMW_RET_OK) {
    ESP_LOGW(
      TAG, "Waiting for micro-ROS agent at %s:%s", CONFIG_MICRO_ROS_AGENT_IP,
      CONFIG_MICRO_ROS_AGENT_PORT);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
#endif

  // Create init_options
  RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

  // Initialize Node
  rcl_node_t node = rcl_get_zero_initialized_node();
  RCCHECK(rclc_node_init_default(&node, "variobot_mcu", "", &support));

  // Initialize Messages
  if (!init_joint_state_message(&joint_state_msg)) {
    ESP_LOGE(TAG, "Failed to initialize joint_state_msg");
    vTaskDelete(NULL);
  }
  if (!init_joint_command_message(&joint_command_msg)) {
    ESP_LOGE(TAG, "Failed to initialize joint_command_msg");
    vTaskDelete(NULL);
  }

  // Initialize Publisher
  RCCHECK(rclc_publisher_init_default(
    &joint_state_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
    "/variobot_mcu/joint_states"));

  // Initialize Subscriber
  RCCHECK(rclc_subscription_init_default(
    &joint_command_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
    "/variobot_mcu/joint_commands"));

  // Initialize Timer (200 Hz)
  rcl_timer_t joint_state_timer = rcl_get_zero_initialized_timer();
  RCCHECK(rclc_timer_init_default2(
    &joint_state_timer, &support, RCL_MS_TO_NS(5), joint_state_timer_callback, true));

  // Initialize Executor
  rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));

  // Add timers and subscribers to the executor
  RCCHECK(rclc_executor_add_subscription(
    &executor, &joint_command_subscriber, &joint_command_msg, &joint_command_callback,
    ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor, &joint_state_timer));

  // Main Loop
  while (1) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    usleep(10000);
  }

  // Free resources
  sensor_msgs__msg__JointState__fini(&joint_state_msg);
  sensor_msgs__msg__JointState__fini(&joint_command_msg);
  RCCHECK(rcl_subscription_fini(&joint_command_subscriber, &node));
  RCCHECK(rcl_publisher_fini(&joint_state_publisher, &node));
  RCCHECK(rcl_node_fini(&node));
  RCCHECK(rclc_support_fini(&support));

  vTaskDelete(NULL);
}

void micro_ros_init()
{
#if defined(CONFIG_MICRO_ROS_ESP_NETIF_WLAN) || defined(CONFIG_MICRO_ROS_ESP_NETIF_ENET)
  ESP_ERROR_CHECK(uros_network_interface_initialize());
#endif

  xTaskCreate(
    micro_ros_task, "uros_task", CONFIG_MICRO_ROS_APP_STACK, NULL, CONFIG_MICRO_ROS_APP_TASK_PRIO,
    NULL);
}
