# variobot_description

This package contains description files for VarioBot in both URDF and SDF alongside the meshes needed for the description.

The `display_launch.yaml` launch file can be used for easy testing of the description:

```bash
ros2 launch variobot_description display_launch.yaml
```

In a new terminal:

```bash
ros2 run joint_state_publisher_gui joint_state_publisher_gui
```

## Purpose of each directory

- [`config`](config/): Contains config files that are used by various nodes.
- [`description`](description/): The main directory of the package, contains various description files for the robot in both URDF and SDF.
- [`hooks`](hooks/): Contains environment hooks for editing environment variables.
- [`launch`](launch/): Contains launch files that can be used to test the robot descriptions.
- [`meshes`](meshes/): Contains mesh (.dae) files of the various parts of the robot. Full CAD files can be found in [`../assembly/CAD`](../assembly/CAD/).
