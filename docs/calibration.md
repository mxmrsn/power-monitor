# Calibration Flow

1. Connect a stable reference load (e.g., 5 A resistive heater) and measure its RMS current with a trusted clamp meter.
2. Capture 16-bit ADC samples (Teensy 4.0 uses 12-bit; left-align to 16-bit space) for one second while the load is active. While sampling, also record the zero-current midpoint by disconnecting the load or averaging the waveform over a full AC cycle.
3. Derive the sensor offset (mid-rail) and scale:

   - `offset = V_midrail (ADC counts)`
   - `peak_to_peak = max_sample - min_sample`
   - `V_pp = peak_to_peak * (V_ref / 4095)`
   - `I_rms = (V_pp / 2) / (sqrt(2) * burden_resistance)`, or use the sensor’s datasheet scale (e.g., ACS712 185 mV/A)
   - `scale = I_rms / (VAmp_peak / V_ref)`

4. Store the calibration tuple in EEPROM:

   | Address | Purpose                                 | Size |
   | 0       | Magic (0xA5A5)                           | 2 B  |
   | 2       | Current offset (float)                   | 4 B  |
   | 6       | Current scale (float)                    | 4 B  |
   | 10      | Voltage offset (float, if sampled)       | 4 B  |
   | 14      | Voltage scale (float, if sampled)        | 4 B  |

   Use `EEPROM.update`/`putFloat` to minimize wear and validate the magic number during `setup()`.
5. Firmware CLI commands:

   - `CALIB DUMP`: prints current offset/scale plus the voltage offset/scale if sampling is enabled.
   - `CALIB ZERO`: averages zero-load samples and rewrites the current offset.
   - `CALIB SCALE <A_rms>`: measures current RMS (after offset removal) and sets a new scale factor against the provided reference.
   - `CALIB VOLT <V_rms>`: measures the AC voltage sample and derives a new voltage scale factor.
   - `CALIB RESET`: rewrites defaults so the next boot uses the built-in mid-rail offsets and scale guesses.

6. For voltage sensing (if added), repeat steps 1-3 with the AC-AC sample and store `voltage_scale` so firmware can derive instantaneous volts for real power.
