# PCB Notes

- Inline hall sensor footprint (e.g., Allegro ACS712-05B) requires differential routing for the sensor’s pin set; keep analog traces away from noisy digital nets.
- Define mains input fuse and MOV near the board edge, with short high-current traces.
- Add test points for ADC sample and hall sensor reference for calibration.
