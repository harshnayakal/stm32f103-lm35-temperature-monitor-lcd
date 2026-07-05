# 🌡️ STM32F103C8T6 LM35 Temperature Monitor using ADC

## 📌 Overview

This project demonstrates Analog-to-Digital Converter (ADC) interfacing on the STM32F103C8T6 (Blue Pill) using Embedded C and the STM32 Standard Peripheral Register Interface.

The system reads the analog output voltage from an LM35 temperature sensor connected to ADC Channel 0 (PA0), converts the ADC value into temperature, and displays the measured temperature on a 16×2 HD44780 LCD operating in 8-bit mode.

The project is intended for beginners learning STM32 peripherals, ADC configuration, GPIO programming, and LCD interfacing without using external libraries.

---

## ✨ Features

- STM32F103C8T6 (Blue Pill)
- Register-level programming
- 12-bit ADC configuration
- LM35 temperature sensor interfacing
- 16×2 LCD interfacing (8-bit mode)
- Real-time temperature measurement
- ADC calibration before conversion
- Analog voltage to temperature conversion
- Well-commented Embedded C code

---

## 🛠 Hardware Used

- STM32F103C8T6 Blue Pill
- LM35 Temperature Sensor
- 16×2 HD44780 LCD
- Breadboard
- Jumper Wires
- 3.3V Power Supply

---

## 📋 Hardware Connections

### LM35

| LM35 Pin | STM32 |
|----------|--------|
| VCC | 3.3V |
| GND | GND |
| OUT | PA0 (ADC1 Channel 0) |

### LCD (8-bit Mode)

| LCD Pin | STM32 Pin |
|----------|------------|
| D0-D7 | PB8-PB15 |
| RS | PB0 |
| EN | PB1 |
| RW | GND |
| VSS | GND |
| VDD | 5V |
| VEE | Potentiometer |

---

## ADC Specifications

- Resolution: 12-bit
- ADC Range: 0–4095
- Reference Voltage: 3.3V
- ADC Channel: ADC1 Channel 0

Voltage Calculation:

Voltage = (ADC Value × 3.3) / 4095

LM35 Conversion:

Temperature (°C) = Voltage × 100

---

## Software Used

- Keil uVision
- Embedded C
- STM32 CMSIS
- STM32F10x Device Header

---

## Project Flow

1. Initialize LCD
2. Initialize ADC1
3. Configure PA0 as Analog Input
4. Calibrate ADC
5. Read ADC value
6. Convert ADC value to voltage
7. Convert voltage to temperature
8. Display temperature on LCD
9. Repeat continuously

---

## Folder Structure
