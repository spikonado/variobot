# VarioBot on URC: BOM

The prices in the first block come from `assembly/README.md` and `spikonado/URC/BOM.csv`. The second block is new for this build. I haven't sourced those parts yet, so tell me which ones you already own first.

## Already specified in the repo

| Ref                | Part                                       | Qty | Unit price             | Buy                                                                                                                | Why                                                                          |
| ------------------ | ------------------------------------------ | --- | ---------------------- | ------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------- |
| URC                | Universal Robot Controller rev 1.0         | 1   | ~$165 (PCBA + ST-Link) | [spikonado/URC](https://github.com/spikonado/URC)                                                                  | Replaces the ESP32-S3 6-motor board. One board for motors, LiDAR, and Wi-Fi. |
| MFL, MFR, MRL, MRR | N20 12 V 300 RPM gearmotor with encoder    | 4   | ~$5.5 (₹619)           | [Robu.in](https://robu.in/product/n20-12v-300rpm-micro-metal-gear-motor-with-encoder) (out of stock on 2026-10-16) | Wheel drive. The 6-wire cable matches URC J10–J13 pin for pin.               |
| —                  | DFRobot 48 mm mecanum wheel kit (4 wheels) | 1   | $15.6                  | [DFRobot](https://www.dfrobot.com/product-2041.html)                                                               | Mecanum mobility system.                                                     |
| LIDAR1             | Slamtec RPLidar A1M8 dev kit               | 1   | $99.0                  | [DFRobot](https://www.dfrobot.com/product-1125.html)                                                               | 2D scans for SLAM and Nav2. The dev-kit 7-pin cable goes to URC J14.         |

## New for this build (to source)

| Ref | Part                                 | Qty    | Requirement                                                        | Why                                                                                                              |
| --- | ------------------------------------ | ------ | ------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------- |
| BT1 | 3S LiPo, ~2200 mAh, XT60             | 1      | 11.1 V nominal, 12.6 V full. Must stay under URC's 36 V VIN limit. | Feeds the N20 motors at their 12 V rating and the URC buck. About 1 hour of runtime at an estimated 2 A average. |
| F1  | Inline blade fuse holder + 5 A fuse  | 1      | Rated ≥ 32 V DC                                                    | URC has no fuse on VIN, and a shorted LiPo can source hundreds of amps.                                          |
| S1  | Power switch                         | 1      | ≥ 10 A at 12 V DC                                                  | URC has no power switch.                                                                                         |
| —   | XT60 pigtails, 14 AWG (one M, one F) | 1 pair | XT60 genuine (Amass)                                               | Battery → F1 → S1 → URC J3.                                                                                      |
| —   | LiPo balance charger                 | 1      | 3S balance support                                                 | Only if you don't own one.                                                                                       |
| —   | USB-C data cable                     | 1      | Data capable                                                       | URC J2 to the laptop for flashing and logs.                                                                      |
