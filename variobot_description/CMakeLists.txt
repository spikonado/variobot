cmake_minimum_required(VERSION 3.10)
project(variobot_description)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)

# Install directories
install(
  DIRECTORY config description hooks launch meshes
  DESTINATION share/${PROJECT_NAME}/
)

# Environment hooks
ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()
