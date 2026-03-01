# PCB Notes

- Inline hall sensor footprint (e.g., Allegro ACS712) sits on the mains side; keep the current path, fuse, and MOV traces short, thick, and pulled away from the low-voltage domain.
- Provide 8 mm (or more) creepage/clearance between any mains trace/pad and the Teensy domain; silk-screen the separation and reinforce with slot cuts if needed.
- Place the time-delay fuse on the line conductor before the hall sensor, and pair it with a MOV (275 Vac) across line and neutral to clamp surges.
- Route the low-voltage TEENSY signals on the opposite side of the board; use a solid area (GND plane) for the MCU and isolate the ADC traces with ground pours.
- Include a dedicated mains test point so you can connect a scope probe to the hall sensor output and the calibration midpoint (mid-rail) during setup.
- Break the isolation barrier with a row of vias or keep-out area; consider a physical slot/cutout so creepage requirements are satisfied without needing extra board space.
- Provide headers (3-pin block) for the mains input, mains output, and optional voltage-sensing module so the board can be implemented in a modular enclosure.
- Add silk labels for current-direction, fuse rating, and sensor orientation so assembly and maintenance stay straightforward.
