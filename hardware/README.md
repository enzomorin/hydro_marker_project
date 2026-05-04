# HydroVinci — PCB Design

> Compact, low-power PCB for the HydroVinci marker system. Designed in KiCad 10, built around the Seeed XIAO nRF52840 Plus.

---

## Table of Contents

- [Overview](#overview)
- [Hardware Summary](#hardware-summary)
- [Prerequisites](#prerequisites)
- [Opening the Project](#opening-the-project)
- [Manufacturing](#manufacturing)
- [Design Notes](#design-notes)
- [Tech used](#tech-used)

---

## Overview

This repository contains the full KiCad design files for the HydroVinci PCB. The board integrates all electronics required by the marker into a single compact PCB, replacing a wired prototype with a manufacturable solution.

The design targets low quiescent current for battery-powered field deployment, with switchable power rails for each peripheral.

---

## Hardware Summary

| Component | Description |
|-----------|-------------|
| MCU | Seeed XIAO nRF52840 Plus (BLE 5.0, nRF52840) |
| Power | LiPo battery input, 12V boost (Adafruit TPS61040) |
| Pressure sensor | DFRobot KIT0139 submersible 4-20mA, converted via SEN0262 |
| Environmental | AZ-Delivery GY-BME280 (temperature, humidity, pressure) |
| Hall effect | Digital Hall sensor via BSS84 high-side switch |
| Data logging | SparkFun OpenLog DEV-13712 (microSD, UART) |
| RF | CC1101 sub-GHz radio module |
| Power switching | BSS138 + BSS84 MOSFET pairs (3.3V and 12V rails) |
| Indicators | Passive buzzer, 2× tactile buttons |

---

## Prerequisites

- [KiCad](https://www.kicad.org/) `>= 10.0`

No additional dependencies. All custom symbols and footprints are included in the repository under `src/PCB/libraries/`.

---

## Opening the Project

1. Clone the repository
   ```bash
   git clone https://github.com/your-org/hydrovinci-pcb.git
   ```

2. Open KiCad and select **File → Open Project**

3. Navigate to `src/PCB/main_pcb.kicad_pro` and open it

From the KiCad project manager you can then open:
- **Schematic Editor** — for the electrical design and component wiring
- **PCB Editor** — for the physical board layout and copper routing

---

## Manufacturing

Gerber files are required to manufacture the board. To generate them from KiCad:

1. Open the PCB Editor (`main_pcb.kicad_pcb`)
2. Go to **File → Fabrication Outputs → Gerbers**
3. Select all required layers and export

The generated Gerbers can be uploaded directly to:
- [JLCPCB](https://jlcpcb.com) — recommended for low-cost prototyping
- [PCBway](https://www.pcbway.com) — recommended for higher quantities

> **Recommended specs:** 2-layer, 1oz copper.

---

## Design Notes

- All GPIO outputs include 100Ω series resistors for noise immunity
- The 12V rail (KIT0139 sensor loop) is switched via a BSS138 + BSS84 MOSFET pair — Vgs is clamped to −6V by R20 to stay within BSS84 absolute maximum ratings
- The EN pin of the boost converter is pulled to GND via R7 (100kΩ) to ensure the 12V rail is off during boot and reset
- The SEN0262 output capacitor (C14) must be rated ≥ 25V (ceramic X5R/X7R, 0805)
- I2C pull-ups R3/R4 (10kΩ) may be omitted if the BME280 module already includes them

---

## Tech used

[![KiCad](https://img.shields.io/badge/kicad-%23314CB0.svg?style=for-the-badge&logo=kicad&logoColor=white)](https://www.kicad.org/)  