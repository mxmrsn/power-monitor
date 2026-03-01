# Power Monitor (Teensy 4.0)

A custom inline AC power monitor board using a Teensy 4.0 to measure current and (optionally) voltage for real power, apparent power, and energy.

## Project Outline
1. **Requirements**
   - Inline between wall socket and load (AC mains).
   - Measure load current safely and accurately.
   - Optional voltage sensing for real power/energy.
   - Provide data over USB/serial and optionally BLE/Wi-Fi.
2. **Architecture**
   - AC mains input -> protection -> current sensor -> ADC -> Teensy.
   - Optional voltage divider + isolation amp for voltage sampling.
   - Isolated low-voltage power supply for MCU and sensors.
3. **Firmware**
   - Sample current (and voltage if present) at >2 kHz.
   - RMS, real power, power factor, energy accumulation.
   - Calibration routines and storage in MCU EEPROM/flash.
4. **PCB**
   - Clear isolation barriers between mains and low-voltage.
   - High voltage spacing and fuse/EMI protection.
   - Test points and calibration headers.
5. **Docs**
   - Safety considerations, build notes, calibration steps.

## Recommended Hardware Peripherals
**Current sensing (choose one):**
- **Current transformer (CT)**: Non-invasive and isolated. Example: 100 A:50 mA CT + burden resistor to produce ~1V RMS at max load.
- **Hall effect sensor (inline)**: Isolated and simpler low-side. Example: Allegro ACS712/ACS758 (choose based on current range).

**Voltage sensing (optional for real power):**
- Resistive divider + isolation amplifier (e.g., AMC1100/AMC1200) into MCU ADC.
- Alternatively use an AC-AC adapter to provide a safe low-voltage sample.

**Protection and safety:**
- Time-delay fuse on line input.
- MOV for surge protection.
- Common-mode choke and X-cap (optional EMI).
- Proper creepage/clearance on PCB.

**Power supply:**
- Isolated AC-DC module (5V or 3.3V) rated for mains.
- LDO or buck to 3.3V for Teensy and sensors.

**MCU & ADC:**
- Teensy 4.0 with built-in ADC. For higher precision, add external ADC (ADS131M02/ADS1115).

**Connectivity:**
- USB for power and data.
- Optional ESP32 or BLE module for wireless telemetry.

## Directory Structure
- `firmware/` MCU firmware source
- `pcb/` PCB design files
- `docs/` Design and safety documentation

## Safety Note
Working with mains voltage is hazardous. If you are not experienced with high-voltage design, use isolation transformers and follow local electrical codes. Consider using an off-the-shelf enclosure and certified power entry module.

## Getting Started
- Review `docs/` for safety and measurement notes.
- Choose sensor and ADC based on expected load range.
- Draft schematic and layout in `pcb/`.
- Implement calibration routines in `firmware/`.
