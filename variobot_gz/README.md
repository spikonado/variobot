# variobot_control

This package contains code relevant to using VarioBot in the Gazebo simulator.

To launch the ros_gz_bridge, Gazebo simulator, and spawner for the robot use `ros_gz_bridge_launch.yaml`, `simulator_launch.yaml`, and `spawner_launch.yaml` respectively.

> [!NOTE]
> `spawner_launch.yaml` launches all launch files related to the robot's bringup and nothing else except `ros_gz_bridge_launch.yaml` and `simulator_launch.yaml` needs to be launched manually.

To launch everything together just use `full_sim_launch.yaml`:

```
ros2 launch variobot_gz full_sim_launch.yaml
```

## Purpose of each directory

- [`config`](config/): Contains config files that are used by various nodes.
- [`launch`](launch/): Contains launch files for launching the simulator, relevant bridges, and complete robot bringup in simulation.
- [`worlds`](worlds/): Contains various simulation worlds.
