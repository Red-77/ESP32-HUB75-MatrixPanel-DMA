# HUB75 RGB LED matrix library utilizing ESP32 DMA Engine with fix for 1/8 Scan LED Matrix Panels

__[BUILD OPTIONS](/doc/BuildOptions.md) | [EXAMPLES](/examples/README.md)__ | [![PlatformIO CI](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/actions/workflows/pio_build.yml/badge.svg)](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/actions/workflows/pio_build.yml)

  **Table of Content**

- [Introduction](#introduction)
  * [Features](#features)
  * [Supported ESP32](#esp32-supported)
  * [Supported Panels](#panels-supported)
  * [Panel driver chips known to be working well](#panel-driver-chips-known-to-be-working-well)
  * [Panels Not Supported](#panels-not-supported)
  * [Update for 16x32 Panels](#update-for-16x32-panels)
  * [Cool uses of this library](#cool-uses-of-this-library)
- [Getting Started](#getting-started)
  * [1. Library Installation](#1-library-installation)
  * [2. Wiring ESP32 with the LED Matrix Panel](#2-wiring-esp32-with-the-led-matrix-panel)
  * [3. Run a Test Sketch](#3-run-a-test-sketch)
- [More Information](#more-information)
  * [Build-time options](#doc/BuildOptions.md)
  * [Memory constraints](#memory-constraints)
  * [Can I use with a larger panel (i.e. 64x64px square panel)?](#can-i-use-with-a-larger-panel-ie-64x64px-square-panel)
  * [Can I chain panels?](#can-i-chain-panels)
  * [Panel Brightness](#panel-brightness)
  * [Latch blanking](#latch-blanking)
  * [Power, Power and Power!](#power--power-and-power)
  * [Inspiration](#inspiration)
 - [Thank you!](#thank-you)


## ESP32 Supported
This library supports the:
* Original ESP32 - That being the ESP-WROOM-32 module with ESP32‑D0WDQ6 chip from 2017. This MCU has 520kB of SRAM which is much more than all the recent 'reboots' of the ESP32 such as the S2, S3 etc.
* ESP32-S2; and
* ESP32-S3

RISC-V ESP32's (like the C3) are not, and will never be supported  as they do not have parallel DMA output required for this library.

## Panels Supported
* 64x32 (width x height) pixel 1/16 Scan LED Matrix 'Indoor' Panel, such as this [typical RGB panel available for purchase](https://www.aliexpress.com/item/256-128mm-64-32-pixels-1-16-Scan-Indoor-3in1-SMD2121-RGB-full-color-P4-led/32810362851.html).
* 64x64 pixel 1/32 Scan LED Matrix 'Indoor' Panel.
* 32x16 pixel 1/4 Scan LED Matrix 'Indoor' Panel using an ingenious workaround as demonstrated in the 32x16_1_4_ScanPanel example.
* 126x64 [SM5266P](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/164) 1/32 Scan Panel 

Ones interested in internals of such matrices could find [this article](https://www.sparkfun.com/news/2650) useful.

Due to the high-speed optimized nature of this library, only specific panels are supported. Please do not raise issues with respect to panels not supported on the list below.

## Panel driver chips known to be working well

* ICND2012
* [RUC7258](http://www.ruichips.com/en/products.html?cateid=17496)
* FM6126A AKA ICN2038S, [FM6124](https://datasheet4u.com/datasheet-pdf/FINEMADELECTRONICS/FM6124/pdf.php?id=1309677) (Refer to [PatternPlasma](/examples/2_PatternPlasma) example on how to use.)
* SM5266P 
* 1/8 Scan Outdoor LED Matrix Panels are supported

## Panels Not Supported
* RUL5358 / SHIFTREG_ABC_BIN_DE based panels are not supported.
* ICN2053 / FM6353 based panels - Refer to [this library](https://github.com/LAutour/ESP32-HUB75-MatrixPanel-DMA-ICN2053), which is a fork of this library ( [discussion link](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/discussions/324)).
* Any other panel not listed above.

Please use an [alternative library](https://github.com/2dom/PxMatrix) if you bought one of these.
