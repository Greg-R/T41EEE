### Version T41EEE.5 T41 Software Defined Transceiver Arduino Sketch

This is the "T41 Extreme Experimenter's Edition" software for the 
T41 Software Defined Transceiver.  The T41EEE was "forked" from the V049.2 version
of the T41-EP software.  T41EEE is fundamentally different from the mainstream T41-EP
code in that it adopts C++ rather than C.  C++ language features will be gradually
introduced in the next releases.

Purchase the book "Digital Signal Processing and Software Defined Radio" by
Albert Peter and Jack Purdum here:

<https://www.amazon.com/Digial-Signal-Processing-Software-Defined/dp/B0D1JH5L65>

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
It is recommended to perform a full FLASH erase before loading T41EEE.5.

You will need to install the ArduinoJson library by Benoit Blanchon.  Using the IDE:
Tools -> Manage Libraries ...
Search for Arduinojson.  Note this is a single word, as there is another library
with the name Arduino_JSON, which is not the correct library.  Install the library,
and if you have the other dependencies for the V049.2 version, compilation will be
successful.

## How to Compile T41EEE

T41EEE.5 was developed and compiled using Arduino IDE version 2.3.2 with the following
configuration:

1.  Optimize is set to "Smallest Code" (Tools menu).
2.  CPU speed is set to 528 MHz (Tools menu).
3.  TeensyDuino version is 1.59.0.
4.  You will need to install ArduinoJson which is currently version 7.0.4.

Completing a FLASH erase of the Teensy is strongly recommended before uploading this new version. 
Remember to save to the SD card via the EEPROM menu EEPROM->SD command prior to erasing.

## Highlight of Changes included in T41EEE.5

1.  Fixed keyer problem introduced by last version changes.
2.  Corrected TX cal on USB to show correct sign.
3.  Added default values for CW and SSB power arrays in EEPROMData struct.
4.  Fixed type conversion problem in sinusoidal tone array function.
5.  Keyer selection is remembered in CW Option menu.
6.  Re-do of button operations to make select button "repeat last command".
7.  Removed redundant microphone audio path from Teensy audio system.
8.  Removd redundant speaker audio path from Teensy audio system.
9.  SetAudioOperatingState() function updated to match new audio paths.
10. Removed MyDelay() function and replaced with delay() function.

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