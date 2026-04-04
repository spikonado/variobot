cmake_minimum_required(VERSION 3.10)
project(variobot_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)

# Install directories
install(
  DIRECTORY config description launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
