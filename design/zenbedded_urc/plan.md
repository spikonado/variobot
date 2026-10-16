# Plan: VarioBot on URC with Zephyr and zenbedded

Goal: replace `variobot_mcu` (ESP-IDF, arduino-esp32, DRV8912, micro-ROS) with a Zephyr app on the URC ESP32-S3. It talks to the off-board laptop over Wi-Fi through zenbedded, and the laptop runs ros2_control with `zenbedded::ZenbeddedHardware`. URC is the only electronics on the robot. The RPLidar sits on URC J14, so `/scan` also has to travel over zenoh.

Hardware: [schematic](./schematic.jsx), [pin map](./pin_map.md), [BOM](./bom.md), [assembly notes](./assembly_notes.md).

## Target architecture

```text
Laptop (ROS 2)                                    URC
┌──────────────────────────────┐   Wi-Fi    ┌──────────────────────────────┐  SPI 100 Hz  ┌────────────────────────┐
│ zenohd :7447                 │◄──────────►│ ESP32-S3 (Zephyr)            │◄────────────►│ STM32G431 (Zephyr)     │
│ controller_manager           │  zenoh TCP │  zenbedded_rcl tier 2 client │  20 B frames │  4× DRV8256P PWM       │
│  zenbedded::ZenbeddedHardware│            │  urc_lib motor driver        │   CRC-8      │  4× quadrature counter │
│  mecanum_drive_controller    │            │  RPLidar UART driver         │              │  500 ms SPI watchdog   │
│ LaserScan bridge (phase 4)   │            └──────────────────────────────┘              └────────────────────────┘
└──────────────────────────────┘
```

Ownership of each job:

| Job                          | Runs on                   | Why                                                                                    |
| ---------------------------- | ------------------------- | -------------------------------------------------------------------------------------- |
| PWM and encoder counting     | STM32                     | Already done in `urc_stm32_firmware`.                                                  |
| Command timeout → duty 0     | ESP32 and STM32           | ESP32 stops after 200 ms without a new command. STM32 coasts after 500 ms without SPI. |
| Counts → rad, rad/s          | ESP32                     | The host schema stays in SI units.                                                     |
| Wheel velocity PID           | Open question (see below) |                                                                                        |
| Mecanum kinematics, odometry | Laptop                    | `mecanum_drive_controller`, unchanged.                                                 |

## Phase 0: prerequisites

1. Encoder pull-ups. In `URC/stm32_firmware/urc_stm32_firmware/boards/urc_stm32/urc_stm32.dts`, change every `enc-gpios` flag from `0` to `GPIO_PULL_UP`. Without that the inputs float behind the series SS14 diodes (see [pin map](./pin_map.md#gotcha-encoder-inputs-need-pull-ups)). This is a small URC PR, and it blocks everything that reads encoders.
2. Fix the URC protocol doc. It says the MCP2518FD uses separate pads, but the netlist puts it on the same SPI bus as the STM32 (GPIO 17/18/21).
3. Align Zephyr versions. URC pins v4.4.1. The zenbedded Docker image uses v4.4.2. Move URC to v4.4.2 so both apps share one west manifest.
4. Pick the ROS distro. zenbedded CI only builds on Lyrical, and VarioBot is on Kilted (EOL December 2026). Two options:
   - Migrate VarioBot to Lyrical. It's in nix-ros-overlay with `mecanum-drive-controller` and `gz-ros2-control`. This is my recommendation, because Kilted reaches EOL in two months anyway and we'd be on the distro upstream tests against.
   - Stay on Kilted and build `zenbedded_hardware_interface` from source. The APIs it uses (`HardwareComponentInterfaceParams`, `get_optional`, `RealtimeThreadSafeBox`) exist in Kilted, but nobody has built it there. Spend an hour on a test build before you commit to this.

## Phase 1: URC Zephyr support (the library PR)

All of this lands as a PR to `spikonado/urc_lib`. The alternative is local code in `variobot_mcu`, but the STM32 firmware already carries its own copy of the protocol header and the two copies have drifted (`BIT(0)` vs `0x01`, different include guards). A third copy in VarioBot would make that worse, and other URC users would need the same driver.

1. Make `urc_lib` a Zephyr module: `zephyr/module.yml`, `zephyr/CMakeLists.txt`, `Kconfig`. Keep `idf_component.yml` only if something still consumes the ESP-IDF build. If nothing does, delete `src/urc_motor.c` and the IDF build files instead of maintaining two backends.
2. Move `protocol.c/.h` into the module as the single source of truth and point `urc_stm32_firmware` at it. Keep `tests/test_protocol.c`, run it under twister on `native_sim`.
3. Add a board, `urc_esp32s3` (procpu), copied from `esp32s3_devkitc` with URC's pinctrl: `spi3` with `cs-gpios` 16 (STM32) and 38 (MCP2518FD), `uart0` console, `uart1` LiDAR, `i2c0` Qwiic, `ledc0` LiDAR PWM. Put it in `urc_lib/boards/` (or in the URC repo next to `urc_stm32`, which you'd prefer?).
4. Add a driver with binding `spikonado,urc-motor`, a child of the SPI bus. API:

   ```c
   int urc_motor_set_duty(const struct device *dev, const int16_t duty[4]);
   int urc_motor_get_telemetry(const struct device *dev, struct urc_telemetry *out);
   int urc_motor_set_enable(const struct device *dev, bool enable);
   int urc_motor_reset_encoders(const struct device *dev);
   ```

   The app calls `set_duty` and reads back the latest encoder counts. A driver thread does the 100 Hz SPI transfer, so a slow zenoh publish can't stall the motor link. The driver counts CRC errors and exposes the STM32 status bits (`WATCHDOG_STOPPED`, `CMD_CRC_ERROR`).

5. Port the serial bus servo library the same way later, when the arms exist. Not needed for VarioBot driving.

## Phase 2: `variobot_mcu` rewrite in Zephyr

Replace the directory contents. Delete arduino-esp32, the DRV8912 driver, the GPIO-ISR encoder code, micro-ROS, `.venv`, `pyproject.toml`, and `uv.lock`. None of it carries over.

1. `west.yml`: Zephyr v4.4.2, `zephyr-zenoh-integration` (pinned commit, `ZEPHYR_EXTRA_MODULES` for `zenbedded_transport` and `zenbedded_rcl`), zenoh-pico from `ros-controls/zenoh-pico@zenbedded`, `urc_lib`. Update `variobot_mcu/flake.nix` to provide west and the Espressif toolchain instead of ESP-IDF.
2. `config/interface_schema.yaml`, shared with the host:

   ```yaml
   state_interfaces:
     front_left_coupling_joint: { position: float64, velocity: float64 }
     front_right_coupling_joint: { position: float64, velocity: float64 }
     rear_left_coupling_joint: { position: float64, velocity: float64 }
     rear_right_coupling_joint: { position: float64, velocity: float64 }
   command_interfaces:
     front_left_coupling_joint: { velocity: float64 }
     front_right_coupling_joint: { velocity: float64 }
     rear_left_coupling_joint: { velocity: float64 }
     rear_right_coupling_joint: { velocity: float64 }
   ```

   The host and firmware must read the same file. Install it from `variobot_control` and point `CONFIG_ZENBEDDED_TIER2_SCHEMA_PATH` at it, so no copy exists.

3. 100 Hz control loop on a fixed-period timer:
   - Read the counts, convert to rad (counts per rev from devicetree per wheel, since rear right may differ), and compute velocity from the count delta.
   - Publish state with `ZenbeddedClient`.
   - If `accepted_command_count()` hasn't moved for 200 ms, send duty 0. The zenbedded inverted pendulum demo uses the same pattern, because zenbedded has no built-in timeout.
   - Map rad/s to duty (open loop or PID, see below). Commands become rad/s. Today they're raw PWM -255..255 under a "velocity" interface name.
4. Kconfig: `CONFIG_ZENBEDDED_TIER_2=y`, `CONFIG_ZENBEDDED_QOS_RELIABLE=y` (TCP, so a lost packet doesn't silently drop a command), `CONFIG_ZENBEDDED_ZENOH_IP_PORT` set to the laptop, `WIFI_CREDENTIALS_CONNECT_STORED`, and Wi-Fi power save off.
5. Behavior under failure:

   | Failure               | Result                                                                                |
   | --------------------- | ------------------------------------------------------------------------------------- |
   | Wi-Fi or laptop drops | ESP32 command timeout stops the wheels within 200 ms.                                 |
   | ESP32 hangs           | STM32 SPI watchdog coasts the wheels within 500 ms.                                   |
   | STM32 resets          | ESP32 also resets (shared `EN` net). The host sees the session drop.                  |
   | SPI CRC errors        | The driver keeps the last good telemetry and counts the error. Stop after N in a row. |

## Phase 3: host side

1. `ros2_control.sdf.xacro`, `hardware_type=real`: change the plugin to `zenbedded::ZenbeddedHardware` with `zenoh_endpoint`, `zenoh_mode`, `state_topic`, `command_topic`, and `schema_path`. Sim keeps `gz_ros2_control`.
2. Dependencies: add `ros-controls/zephyr-zenoh-integration` to `variobot-apt-ubuntu.repos` and `flake.nix` (source only, no binary release). Drop `micro-ROS-Agent` from the repos file and `joint_state_topic_hardware_interface` from `package.xml`.
3. `spawner_launch.yaml`: replace `micro_ros_agent` with `zenohd` listening on `tcp/0.0.0.0:7447`.
4. Raise `controller_manager` `update_rate` from 60 to 100 Hz so it matches the MCU loop.
5. Upstream PR to `zenbedded_hardware_interface`: a `state_timeout_ms` parameter. When the state is older than that, `read()` returns `ERROR`. Today `read()` keeps the last value forever, so a dead link shows the robot as moving at its last speed and odometry keeps integrating it. This is the largest reliability gap on the host.
6. Update `assembly/README.md` to describe URC instead of the ESP32-S3 6-motor board.

## Phase 4: LiDAR over zenoh

zenbedded only moves joint state and command today. Options for `/scan`:

1. Tier 1 CDR `sensor_msgs/LaserScan` with an rmw_zenoh keyexpr, so ROS nodes see `/scan` directly. This needs a LaserScan codec and type hash in `zenbedded_transport`. It's an upstream PR, and it fits the existing multi pub/sub PRs (#104–107).
2. A raw packed key plus a small host node that republishes `LaserScan`. Faster to build, but it's VarioBot-only code.

I recommend option 1, starting with option 2 if upstream review is slow. Firmware side either way: a Zephyr UART driver for the RPLidar protocol on `uart1` at 115200 baud, and motor speed through `ledc0` on GPIO 10. A scan is about 1450 points at 5.5 Hz, so roughly 12 kB per `LaserScan` and 65 kB/s, well within Wi-Fi capacity.

## Phase 5: testing

- `native_sim` twister: protocol encode/decode, the counts→rad conversion, command timeout.
- Host gmock tests for the staleness check in the upstream PR.
- Bring-up checklist on hardware, in order: STM32 alone with the ST-Link (encoders read on all eight inputs), ESP32 SPI link (`GET_VERSION`), wheels off the ground with `ros2 topic pub` commands, kill Wi-Fi mid-motion and confirm a stop under 200 ms, then the LiDAR.

## Open questions

1. Distro: migrate to Lyrical, or test-build zenbedded on Kilted first?
2. Where does the wheel velocity PID run? Today it's four host `pid_controller`s at 60 Hz. Over Wi-Fi, jitter and dropped packets go straight into the loop. Moving the PID onto the ESP32 (or the STM32, closest to the encoders) makes the loop independent of the link, and the host then sends rad/s targets. The cost: tuning moves into firmware, and sim keeps the host PID, so sim and real differ. I'd move it to the MCU.
3. URC Zephyr code: `urc_lib` PR (my recommendation) or local code in `variobot_mcu`?
4. Board definition location: `urc_lib` or the URC repo?
5. Which [BOM](./bom.md) parts do you already own?

## Upstream risks

- zenbedded is pre-v0.1.0 (issue #109). `zenbedded_rcl` is marked WIP, and draft PR #94 may change how it's consumed. Pin a commit.
- The zenoh lease is hardcoded to 60 s (#85), so the host takes up to a minute to notice a dead session. The staleness check in phase 3 covers that.
- Memory-safety fixes are still open (#77), and Xtensa FPU use (#50) affects the ESP32-S3.
- No namespacing or `ROS_DOMAIN_ID` support yet (#99). That's fine for one robot.
