# T41 EXTREME EXPERIMENTER'S EDITION

Version T41EEE.0 T41 Software Defined Transceiver Arduino Sketch

This is the first of the "T41 Extreme Experimenter's Edition" software for the 
T41 Software Defined Transceiver.  The T41EEE was "forked" from the V049.2 version
of the T41-EP software.  T41EEE is fundamentally different from the mainstream T41-EP
code in that it adopts C++ rather than C.  C++ language features will be gradually
introduced in the next releases.

This software is licensed according to:
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007
Please refer to the included LICENSE file.
Greg Raven, December 13 2023

T41EEE will be hosted at this public Github repository:

<https://github.com/Greg-R/T41EEE>

I don't plan on publishing periodic releases.  The repository will be updated as bug fixes
and feature enhancements become available.  You will be able to update and FLASH your radio
with the revised software quickly.

Please note that the configuration structure is different than the predecessor V049.2
It is recommended to perform a full FLASH erase before loading T41EEE.0.

## Highlight of Features included in T41EEE.0

1.  External configuration file stored on the SD card is changed to JSON format.
2.  New selections in the EEPROM menu allow serial readout of configuration data
    from the stack, the EEPROM, the SD card, and the hard-coded defaults.
3.  The Morse decoder is coded in a state-machine design pattern.
4.  The sidetone quality is improved.  Thank you to Jonathan KN6LFB for this very
    excellent improvement!
5.  The user can now select from 4 different discrete CW offset tone frequencies.
    The new selections are in the CW Options menu.
6.  Spectrum zooms of 8X and 16X bugs are resolved and these zooms are functioning.
7.  Transmit calibration can be done with either a 750 Hz or 3 kHz tone.  This new
    selection is in the Calibration Options menu.
8.  The configuration file includes variables for frequency divider (quadrature
    generator) hardware.  The values of 4 work with the original T41 QSD and QSE
    boards.


  This comment block must appear in the load page (e.g., main() or setup()) in any source code
  that uses code presented as whole or part of the T41-EP source code.

  (c) Frank Dziock, DD4WH, 2020_05_8
  "TEENSY CONVOLUTION SDR" substantially modified by Jack Purdum, W8TEE, and Al Peter, AC8GY

  This software is made available under the GNU GPLv3 license agreement. If commercial use of this
  software is planned, we would appreciate it if the interested parties contact Jack Purdum, W8TEE, 
  and Al Peter, AC8GY.

  Any and all other uses, written or implied, by the GPLv3 license are forbidden without written 
  permission from from Jack Purdum, W8TEE, and Al Peter, AC8GY.

Please refer to the included file T41_Change_Log.txt which includes the description of changes made
to prior versions.