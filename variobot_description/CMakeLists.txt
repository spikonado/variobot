cmake_minimum_required(VERSION 3.8)
project(variobot_description)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
set(THIS_PACKAGE_DEPENDS
  ament_cmake
)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

# Install directories
install(
  DIRECTORY config description hooks launch meshes
  DESTINATION share/${PROJECT_NAME}/
)

# Environment hooks
ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
