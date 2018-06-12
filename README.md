## Fri Feb 23 16:35:52 2018 mazer 

This is a version of Arduino microdrive controller that works with
either the original Arduino Motorshield R3 or the Adafruit Motorshield
V2 based on a compile time flag. Define R3 for R3, otherwise defaults
to Adafruit driver.

This is intended to control a National Aperature MM-series linear
actuator. But in theory, should work with any DC motor-based
actuator with an rotary encoder attached. COUNTS_PER_REV will
need to be adjusted as well.

### Requires:

- Arduino UNO
- Arduino-brand motor shield R3 *OR* Adafruit motor shield V2.x
- adafruit color lcd shield
- 2 pots
- assorted connectors etc..

## Usage

1. up/down & jog>0: jog/step
2. up/down & jog=0: slew
3. s-left: zero
4. s-right: save current position to EEPROM (pos #1)
