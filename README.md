# mobile-controller
Control IR and RF remotes from mobile device.

## Hardware

* Particle Photon (original Photon 1 because Photon 2 doesn't support the IRremote library)
* VS1838B IR receiver - Wired to pin A5.
* IR LED - Wired to pin D2.
* RF remote control based on EV1527 - Wired to pins D3-D5 and A0-A2.

The Photon connects to the Particle Cloud and listens for certain function calls.  It translates these calls into interactions with IR and RF devices.

* ir_recv [slot] - Programs IR data into the Photon.  The Photon watchs for an IR signal and when one is received, stores the code in EEPROM at the specified _slot_ location.
* ir_send [slot] - Transmits IR data to a remote device.  The Photon retrieves the _slot_ IR data from EEPROM and pulses an IR LED.
* ir_erase [slot] - Deletes IR data from the Photon. Sets the EEPROM to 0xFF at the _slot_ location, thereby clearing out a stored code.  If _slot_ is "255", all EEPROM data is erased.
* rf_send [btn] - Instructs the Photon to bring pin _btn_ high.  This activates an EV1527 RF modulator hard-wired to the Photon.

## Software

For ease of deployment, a progressive web app.

