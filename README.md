### Version T41EEE.1 T41 Software Defined Transceiver Arduino Sketch

This is the "T41 Extreme Experimenter's Edition" software for the 
T41 Software Defined Transceiver.  The T41EEE was "forked" from the V049.2 version
of the T41-EP software.  T41EEE is fundamentally different from the mainstream T41-EP
code in that it adopts C++ rather than C.  C++ language features will be gradually
introduced in the next releases.

Purchase the book "Digital Signal Processing and Software Defined Radio" by
Albert Peter and Jack Purdum here:

<https://www.amazon.com/Software-Defined-Radio-Transceiver-Construction/dp/B09WYP1ST8>

Please bring your questions, bug reports, and complaints about this software to this
group:

<https://groups.io/g/SoftwareControlledHamRadio>

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
It is recommended to perform a full FLASH erase before loading T41EEE.1.

You will need to install the ArduinoJSON library by Benoit Blanchon.  Using the IDE:
Tools -> Manage Libraries ...
Search for ArduinoJSON.  Note this is a single word, as there is another library
with the name Arduino_JSON, which is not the correct library.  Install the library,
and if you have the other dependencies for the V049.2 version, compilation will be
successful.

## Highlight of Features included in T41EEE.1

Thank you to Jonathan KN6LFB for the major improvement in button functionality!

1.  Switch matrix buttons are changed from polling to interrupts for improved performance.
2.  The new button code includes a tunable variable for button response time.  This variable
    is added to the calibration menu as BTN REPEAT.  The default value should be a good
    starting point.  If button response is too fast, increase the BTN REPEAT value.
3.  The switch matrix calibration routine is added to the calibration menu.
4.  Switch matrix calibration is easier and precise.  It is possible to push a button
    and hold it.  It will not skip to the next button.  In practice, a short, firm push
    on each button in the proper series will rapidly complete the process.
5.  EEPROM start-up code is significantly revised.


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