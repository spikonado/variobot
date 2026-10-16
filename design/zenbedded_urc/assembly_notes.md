# VarioBot on URC: assembly notes

## Power

- Wire BT1 → F1 → S1 → an XT60 into URC J3 (`VIN`, top left on the board). Leave J6 `VSERVO` unplugged until the arms exist.
- Keep S1 off while connecting anything to J10–J14.
- URC J3 accepts up to 36 V, but the N20 motors are 12 V parts. Stay on 3S.
- The +5V rail comes from the URC buck (LMR51635, 3.5 A). It powers the encoders and the RPLidar. The LiDAR core draws up to 600 mA at start and needs under 50 mV of ripple. Don't hang motors or servos on +5V.

## Motors

- Each N20 cable lands on one 6-way terminal in its own order: red, black, yellow, green, blue, white → `M1`, `5V`, `ENC1`, `ENC2`, `GND`, `M2`.
- Mapping: J10 → front left, J11 → front right, J12 → rear left, J13 → rear right. If a wheel spins backwards, flip its direction in firmware devicetree. Don't swap the motor wires, because that also inverts the encoder.
- Encoders count 4× quadrature on the STM32. The old ESP-IDF firmware counted A-rising edges only, so expect about 2760 counts per wheel revolution (4 × 690). The old firmware used 2070 for rear right, so check whether that motor has a different gearbox.

## LiDAR

- Dev-kit cable to J14: `VCC_5` and `5V_MOTO` → J14.1/J14.2 (5V), LiDAR `TX` → J14.3 (`U1TX/14`), LiDAR `RX` → J14.5 (`U1RX/15`), `CTRL_MOTO` → J14.6 (`IO10`), both grounds → J14.7/J14.8. J14.4 stays empty.
- The LiDAR is 3.3 V TTL, so no level shifter is needed.

## Flashing

- ESP32-S3: USB-C into J2 (USB-UART, top left). `west flash` uses esptool with the CP2102N auto-reset.
- STM32: ST-Link on J9 (SWD). The URC todo still lists flaky SWD, so use connect-under-reset at 400 kHz until that's fixed.
- J2 also resets the STM32: ESP32 EN and STM32 NRST are the same net, so expect both MCUs to reboot on every ESP32 flash.
- Wi-Fi credentials go in through the Zephyr shell on J2 (`wifi cred add …`). They're stored in flash and never committed.
