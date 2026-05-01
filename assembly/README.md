# assembly

> [!NOTE]
> If you just want to run the simulation go to [`installation`](./../installation), after sourcing your workspace run `ros2 launch variobot_gz full_sim_launch.yaml`.

## BOM

### Mobility Systems

For all mobility systems [this](https://github.com/Amronos/ESP32-S3_6_Motor_Driver_IMU) custom PCB and a portable 12V + 5V power source on the robot is required.

#### Mecanum Drive

##### 48mm Mecanum Wheels with N20 DC Motors

| Item                                        | Average Unit Price | Quantity | Average Total Price | Places to Buy                                                                                                                  |
| ------------------------------------------- | ------------------ | -------- | ------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| N20 12V 300RPM DC Motor With Encoder        | ~$5.5              | 4        | ~$22.0              | [Robu.in](https://robu.in/product/n20-12v-300rpm-micro-metal-gear-motor-with-encoder)                                          |
| DFRobot Mecanum Wheel Kit (48mm - 4 Wheels) | $15.6              | 1        | $15.6               | [DFRobot](https://www.dfrobot.com/product-2041.html)                                                                           |
| 48mm Mecanum Wheel to N20 Motor Coupling    | -                  | 4        | -                   | [3D Print it Yourself](./STLs/Mobility_Systems/Mecanum_Drive/48mm_N20_Mecanum_Drive/48mm_Mecanum_Wheel_N20_Motor_Coupling.stl) |
| 48mm N20 Mecanum Drive Base                 | -                  | 1        | -                   | [3D Print it Yourself](./STLs/Mobility_Systems/Mecanum_Drive/48mm_N20_Mecanum_Drive/48mm_N20_Mecanum_Drive_Base.stl)           |

### Sensors

#### Lidars

The currently supported lidars are listed below you can choose one of them to use with the robot.

##### RPLidar A1 M8

| Item                 | Average Price | Places to Buy                                                       |
| -------------------- | ------------- | ------------------------------------------------------------------- |
| RPLidar A1 M8        | $99.0         | [DFRobot](https://www.dfrobot.com/product-1125.html)                |
| RPLidar A1 M8 Holder | -             | [3D Print it Yourself](./STLs/Lidar_Parts/RPLidar_A1_M8_Holder.stl) |
| Lidar Base           | -             | [3D Print it Yourself](./STLs/Lidar_Parts/Lidar_Base.stl)           |
