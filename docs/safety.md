# Safety Notes

- Only handle live wiring when the board is fully enclosed and the AC source is disconnected.
- Use a time-delay fuse sized slightly above 10 A on the line conductor before the hall sensor.
- Mount an MOV (e.g., 275 VAC) across line and neutral to clamp surges, and consider a common-mode choke for EMI.
- Keep 8 mm creepage/clearance between mains and low-voltage copper; mark separation on the PCB silk.
- Verify isolation on any auxiliary power supply (isolated 5 V or 3.3 V module) before connecting the Teensy.
