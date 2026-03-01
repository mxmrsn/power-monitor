# Calibration Flow

1. Use a precise reference load (e.g., 5 A resistive heater). Measure actual current with a trusted clamp meter.
2. Log Teensy ADC raw values at rest (zero current) to capture sensor offset and mid-rail voltage.
3. Record ADC peak-to-peak for the known RMS current and derive the scaling factor:

   `I_rms = (ADC_pp / sqrt(2)) * (sensor_scale / V_ref)`

4. Store offset and scale values in Teensy EEPROM (`EEPROM.writeFloat`) so reboot does not require recalibration.
5. Provide a firmware command (e.g., over USB) to dump calibration values and re-run the routine if the sensor is changed.
