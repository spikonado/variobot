# URC pin map for VarioBot

Source: netlist exported from `spikonado/URC` `pcb_design/*.kicad_sch` (rev 1.0) and the board silkscreen. Net names match [the schematic](./schematic.jsx).

## U4 · ESP32-S3-WROOM-1-N16R8 (Zephyr app `variobot_mcu`)

| GPIO          | URC net                        | Connector               | VarioBot use                        | Zephyr peripheral         |
| ------------- | ------------------------------ | ----------------------- | ----------------------------------- | ------------------------- |
| 16            | `STM32_CS/16`                  | internal                | STM32 motor link CS                 | `spi3` `cs-gpios` index 0 |
| 17            | `PICO1/17`                     | internal                | STM32 + MCP2518FD MOSI (shared bus) | `spi3` MOSI               |
| 18            | `SCK1/18`                      | internal                | STM32 + MCP2518FD SCK (shared bus)  | `spi3` SCLK               |
| 21            | `POCI1/21`                     | internal                | STM32 + MCP2518FD MISO (shared bus) | `spi3` MISO               |
| 14            | `U1TX/14`                      | J14.3                   | `LIDAR_TX`: LiDAR TX into ESP32     | `uart1` RX                |
| 15            | `U1RX/15`                      | J14.5                   | `LIDAR_RX`: ESP32 TX to LiDAR       | `uart1` TX                |
| 10            | `LIDAR_IO3/10`                 | J14.6                   | `LIDAR_PWM`: RPLidar `CTRL_MOTO`    | `ledc0` channel           |
| 7             | `LIDAR_IO2/7`                  | J14.4                   | not used                            | none                      |
| 19 / 20       | `D-` / `D+`                    | J1 USB-CDC              | spare (USB Serial/JTAG)             | `usb_serial`              |
| 43 / 44       | `U0TX` / `U0RX`                | J2 USB-UART via CP2102N | console, logs, `west flash`         | `uart0`                   |
| 0             | `BOOT`                         | SW2, CP2102N auto-reset | boot mode                           | none                      |
| EN            | `EN`                           | SW1, CP2102N auto-reset | reset                               | none                      |
| 38            | `CAN_CS/38`                    | internal                | not used yet                        | `spi3` `cs-gpios` index 1 |
| 39 / 40 / 41  | `CAN_INT0/1`, `CAN_INT`        | internal                | not used yet                        | `mcp251xfd` `int-gpios`   |
| 42            | `CAN_CLKO/42`                  | internal                | not used                            | none                      |
| 47 / 48       | `U2TX/47` / `U2RX/48`          | J7 servo bus            | not used yet (serial bus servos)    | `uart2`                   |
| 8 / 9         | `SDA/8` / `SCL/9`              | J15 Qwiic, J17          | not used yet (IMU later)            | `i2c0`                    |
| 11 / 12 / 13  | `PICO/11`, `SCK/12`, `POCI/13` | J17                     | not used                            | `spi2`                    |
| 1, 2, 4, 5, 6 | `/1` … `/6`                    | J16                     | not used                            | GPIO                      |

### Gotchas

| Pin          | Issue                                                                                                                                                                                                                                             |
| ------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0, 3, 45, 46 | Strapping pins. GPIO3/45/46 go to header J18 only. Keep them unloaded at reset.                                                                                                                                                                   |
| 35, 36, 37   | Used by the N16R8 octal PSRAM. Unconnected on URC. Never assign them in devicetree.                                                                                                                                                               |
| 17, 18, 21   | Shared SPI bus: the STM32 (CS 16) and the MCP2518FD (CS 38) are both on it. Both must be children of the same `spi3` node so Zephyr serializes access. The protocol doc says the CAN controller uses separate pads. The netlist shows it doesn't. |
| 14, 15       | Net names are from the LiDAR's side. `U1TX/14` is the ESP32's UART1 **RX**, matching `LidarSerial.begin(baud, SERIAL_8N1, 14, 15)` in `firmware_examples/rplidar`.                                                                                |
| 19, 20       | Native USB. Zephyr's USB CDC-ECM could run here as a wired zenoh link later. VarioBot uses Wi-Fi.                                                                                                                                                 |

## U11 · STM32G431KBU6 (Zephyr app `urc_stm32_firmware`)

| Pin               | URC net                   | Function                                  |
| ----------------- | ------------------------- | ----------------------------------------- |
| PA4               | `STM32_CS/16`             | SPI1 NSS (peripheral)                     |
| PA5               | `SCK1/18`                 | SPI1 SCK                                  |
| PA6               | `POCI1/21`                | SPI1 MISO                                 |
| PA7               | `PICO1/17`                | SPI1 MOSI                                 |
| PA10 / PA11       | `IN1` / `IN2`             | Motor 1 (J10) TIM1 CH3/CH4                |
| PA2 / PA3         | `IN3` / `IN4`             | Motor 2 (J11) TIM2 CH3/CH4                |
| PB7 / PB8         | `IN5` / `IN6`             | Motor 3 (J12) TIM4 CH2/CH3                |
| PA15 / PB0        | `IN7` / `IN8`             | Motor 4 (J13) TIM2 CH1, TIM3 CH3          |
| PA8 / PA9         | `ENC1` / `ENC2`           | Motor 1 encoder A/B                       |
| PA0 / PA1         | `ENC3` / `ENC4`           | Motor 2 encoder A/B                       |
| PB4 / PB5         | `ENC5` / `ENC6`           | Motor 3 encoder A/B                       |
| PB6 / PA12        | `ENC7` / `ENC8`           | Motor 4 encoder A/B                       |
| PF1               | `nSLEEP`                  | All four DRV8256P nSLEEP                  |
| PG10              | `EN`                      | NRST. Same net as ESP32 EN, SW1, and J9.5 |
| PA13 / PA14 / PB3 | `SWDIO` / `SWCLK` / `SWO` | J9 debug header                           |

### Gotcha: encoder inputs need pull-ups

`ENC1`–`ENC8` reach the STM32 through series Schottky diodes D7–D14 (anode at the MCU, cathode at the terminal). When the encoder output is high, the diode is reverse biased and only an MCU pull-up can make the pin read high. The nets have no external pull-up, and `urc_stm32.dts` declares `enc-gpios` with flags `0`, which leaves the pins floating. They must be `GPIO_PULL_UP`. See phase 0 in [the plan](./plan.md).

### Gotcha: one reset net for both MCUs

`EN` is a global label on all three sheets, so ESP32 EN and STM32 NRST are one net. SW1, the CP2102N auto-reset, and any STM32 internal reset (IWDG, brown-out, `sys_reboot`) all drive it, so an STM32 watchdog reset also reboots the ESP32 and drops the zenoh session. The ESP32 can't reset the STM32 on its own, and the STM32 can't reset alone.
