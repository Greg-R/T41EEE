### Version T41EEE.2 T41 Software Defined Transceiver Arduino Sketch

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

## Highlight of Features included in T41EEE.2

Thank you to Jonathan KN6LFB and Harry GM3RVL for their contributions to the T41EEE code!

1.  Harry GM3RVL version of ShowSpectrum() inserted which includes automatic RF gain control
2.  Added On/Off for AutoGain.
3.  Converted to ArduinoJson 7 and some house cleaning.
4.  Added EEPROMWrite to key selections and paddle flip.
5.  Disable button presses during transmit. Buttons are still read and "processed", so that interrupt-driven button repeats are cleared,
    but they are not "executed".  By Jonathan KN6LFB.
6.  Modified ResetHistograms to use the already-calculated transmitDitLength, and not to reset the current keyer speed to 15wpm. Since
    the keyer speed is not being modified, there is also no reason to call UpdateWPMField.  By Jonathan KN6LFB.
7.  Added new array for RF gain value per band.
8.  Auto-gain works independently of manual RF gain per band.
9.  Added EEPROMWrite() to SetWPM().
10. Set autogain to false by default.


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