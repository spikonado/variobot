# variobot_control

This package contains ros2_control configuration for VarioBot.

Use `controllers_launch.yaml` to launch the controllers:

```bash
ros2 launch variobot_control controllers_launch.yaml
```

## Purpose of each directory

- [`config`](config/): Contains config files that are used by various nodes.
- [`description`](description/): Contains description files related to ros2_control that are used by [`variobot_description`](../variobot_description/).
- [`launch`](launch/): Contains launch files used to launch ros2_control controller_manager and controllers.
