### Version T41EEE.6 T41 Software Defined Transceiver Arduino Sketch

This is the "T41 Extreme Experimenter's Edition" software for the 
T41 Software Defined Transceiver.  The T41EEE was "forked" from the V049.2 version
of the T41-EP software.  T41EEE is fundamentally different from the mainstream T41-EP
code in that it adopts C++ rather than C.  C++ language features will be gradually
introduced in the next releases.

Purchase the book "Digital Signal Processing and Software Defined Radio" by
Albert Peter and Jack Purdum here:

<https://www.amazon.com/Digital-Signal-Processing-Software-Defined/dp/B0D25FV48C>

Please bring your questions, bug reports, and complaints about this software to this
group:

<https://groups.io/g/SoftwareControlledHamRadio>

This software is licensed according to:  
GNU GENERAL PUBLIC LICENSE  
Version 3, 29 June 2007  
Please refer to the included LICENSE file.  
Greg Raven, March 12 2024

T41EEE will be hosted at this public Github repository:

<https://github.com/Greg-R/T41EEE>

I don't plan on publishing periodic releases.  The repository will be updated as bug fixes
and feature enhancements become available.  You will be able to update and FLASH your radio
with the revised software quickly.

Please note that the configuration structure is different than the predecessor V049.2
It is recommended to perform a full FLASH erase before loading T41EEE.6.

You will need to install the ArduinoJson library by Benoit Blanchon.  Using the IDE:
Tools -> Manage Libraries ...
Search for Arduinojson.  Note this is a single word, as there is another library
with the name Arduino_JSON, which is not the correct library.  Install the library,
and if you have the other dependencies for the V049.2 version, compilation will be
successful.

## How to Compile T41EEE

T41EEE.6 was developed and compiled using Arduino IDE version 2.3.2 with the following
configuration:

1.  Optimize is set to "Smallest Code" (Tools menu).
2.  CPU speed is set to 528 MHz (Tools menu).
3.  TeensyDuino version is 1.59.0.
4.  You will need to install ArduinoJson which is currently version 7.0.4.

Completing a FLASH erase of the Teensy is strongly recommended before uploading this new version. 
Remember to save to the SD card via the EEPROM menu EEPROM->SD command prior to erasing.

## Highlight of Changes included in T41EEE.6

 1.  Fixed CW sidetone problem.
 2.  Fixed ordering of button interrupt enabling and EEPROM startup function calls in setup().
 3.  Added #define DEBUG_SWITCH_CAL and .ino code.
 4.  Fixed version not updating in SD file.  CW filter shown rather than default in menu.
 5.  Fixed switch matrix debug mode not saving to EEPROM.
 6.  Inhibit transmit in AM demod modes.
 7.  Changed Serial speed from 9600 to 115200, a modern speed.
 8.  Boosted QSE2DC transmit gain, which increases power by about 3 dB.
 9.  Corrected comment in the ResetFlipFlops() function.
10.  Fixed strange filter band limit behavior and moved graphics to FilterSetSSB() function.
11.  Updated README with new link to book and compile configuration.
12.  Removed unused variables and code, formerly used for audio bandwidth delimiters.
13.  Fixed bug in FilterSetSSB() which caused audio filter bandwidth to change inadvertently.
14.  Smoother tuning in 16X Zoom.
15.  Improved accuracy of location of blue tuning bar.
16.  Higher dynamic range calibration display working.

*********************************************************************************************

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