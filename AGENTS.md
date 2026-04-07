# AGENTS.md

## Project Overview

This is a modular robot project called VarioBot.

## Testing

1. `prek run -a --hook-stage manual` for formatting and linting
2. `colcon build --symlink-install` and `colcon test` -> These should always be executed in the ROS 2 workspace root
3. `idf.py build` -> This should always be executed in the `variobot_mcu` directory

After you are done with your changes, run only the tests relevant to them unless instructed otherwise.

### Nix Environments

The project ships with `./flake.nix` and `./variobot_mcu/flake.nix`, these should provide you with all the dependencies/tools you may need.

## Priorities in Order

1. Reliability of code
2. Maintainability of code
3. Performance of code

All of these are core priorities, try your best to achiveve all of them without having to make tradeoffs.

## Maintaining Code

Don't be afraid to change existing code in order to improve on any of the priorities.
If you add new functionality, first check if there is shared logic that can be extracted to a separate module.
Duplicate logic across multiple files should be avoided.
Don't take shortcuts by just adding local logic to solve a problem.

## Dependency Documentation

Most of what you know about our dependencies is outdated or wrong.
Most of your training data contains obsolete APIs, deprecated patterns, and incorrect usage.
Always check the documentation for latest best practices.

### Using `npx nia-docs`

Check the docs often via `npx nia-docs <link-to-doc>`.

```bash
# Search for a topic
npx nia-docs <link-to-doc> -c "grep -rl 'auth' ."

# Read a specific page
npx nia-docs <link-to-doc> -c "cat getting-started.md"

# Find all guides
npx nia-docs <link-to-doc> -c "find . -name '*.md'"

# List top-level structure
npx nia-docs <link-to-doc> -c "tree -L 1"

# Browse interactively
npx nia-docs <link-to-doc>
```

The shell starts in the docs root. Use `.` for relative paths — all standard Unix tools work (grep, find, cat, tree, ls, head, tail, wc).

### Links to Documentation

#### ROS related

- ROS 2: https://docs.ros.org/en/kilted/ and documentation for each package (including API) is available at https://docs.ros.org/en/kilted/p/<package_name>/
- ROS 2 Control: https://control.ros.org/kilted/
- Gazebo: https://gazebosim.org/docs/ionic/ and API docs available on the links given in the `Library Reference` doc.
- Micro-ROS: https://micro.ros.org/docs/

#### MCU related

- ESP-IDF: https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32s3/
- Arduino-ESP32: https://docs.espressif.com/projects/arduino-esp32/en/latest/
- Micro-ROS: https://micro.ros.org/docs/ and examples are available at `./variobot_mcu/managed_components/micro_ros_espidf_component/examples/`
