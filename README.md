# VarioBot

VarioBot is a modular robotic platform for research and education with swappable mobility systems, configurable sensors, and dual versatile robotic arms.

This project is currently in the early stages of development and is not yet fully functional or ready for general use.<br>
Key features are still being implemented, and significant updates are expected.
As such, the project's usability will be limited.<br>
Contributions, feedback, and suggestions are welcome to help improve this robot.

> [!NOTE]
> Go to [`assembly`](./assembly) if you want to build or simulate the robot for yourself.

## Why?

There are no robots out there that are easy to use, completely modular, and low-cost.<br>
This means educators, researchers, and students aren't able to access robots on which they can test a variety of algorithms and robot apps.
This forces them to design a new robot for every task.<br>
VarioBot is being designed to fill this gap in robotics and usher in an era where people design more robot apps and fewer robots.

## Implemented Features

- Swappable Mobility Systems:
  - Mecanum Drive with 48mm Wheels and N20 DC Motors
- Swappable Lidars:
  - RPLidar A1 M8

## Planned Features

- More swappable mobility systems
- More swappable Lidars and Cameras
- 2 versatile robot arms for interacting with different types of objects
- 180° rotatable camera
- SLAM with 2D/3D lidar and/or depth camera
- Outdoor and indoor navigation with Nav2
- Object detection and tracking using camera

## Graphics

![Render 1](./assets/render_1.png)
![Render 2](./assets/render_2.png)
![Autonomous Navigation](./assets/auto-nav.gif)

See [`design`](design/) to know more about how the robot has been designed.

## Directory Structure

- [`assembly`](./assembly): Contains resources for assembling the robot in real life. (Ex. CAD files)
- [`assets`](./assets): Contains assets used in various places throughout the repository.
- [`design`](./design): Contains various diagrams and explanations on how the robot has/will be designed.
- [`installation`](./installation): Contains files and instructions for installing/building the robot's code.
- [`variobot_control`](./vario_control): ROS 2 package that contains ros2_control configuration for the robot.
- [`variobot_description`](./vario_description): ROS 2 package that contains description files for the robot.
- [`variobot_gz`](./vario_gz): ROS 2 package that contains code relevant to using the robot in the Gazebo simulator.
- [`variobot_mcu`](./vario_mcu): Contains MCU firmware for the robot written in ESP-IDF.
- [`variobot_navigation`](./vario_navigation): ROS 2 package that contains code relevant to the robot's autonomous navigation.
