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
The instructions for performing a FLASH erase of the Teensy are here:

<https://www.pjrc.com/store/teensy41.html#programming>

The bullet "Memory Wipe & LED Blink Restore" has the instructions.

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
17.  AM modes tuning problem is resolved.
18.  Automated calibration feature is available in the Calibration menu (details below).
19.  Audio filter bandwidth selection is indicated by yellow delimiter bar.
20.  Volume is equalized when adjusting audio bandwidth.
21.  Noise reduction algorithms are equalized to the same gains.
22.  MyConfiguration.h includes customizable coarse (center) and fine frequency increments.
23.  Auto-Spectrum mode optimizes the position of the spectral display without affecting gain.
     This is activated in the RF Set menu.  The gain setting reverts to manual control, also
     in the RF Set menu.

## Automated Calibration

Automated calibration is available starting with this version T41EEE.6.

Automated calibration set-up is the same as for manual calibration.  A loop-back path from the
output of the QSE to the input of the QSD must be connected.  An attenuator, in the value of 
30 to 50 dB, must be inserted in this path.  The value for optimal calibration must be determined
empirically.  If a spectrum analyzer is available, it is strongly recommended to view the result
in a narrow sweep (~10kHz) to determine if the undesired sideband (and carrier in the case of QSE2DC)
is adequately suppressed.

After the loop-back path is inserted, use the following command from the main function buttons (2 and 5):

Calibrate (select) Radio Cal (select)

The radio will move to the 80 meter band and begin the calibration process.  The process will pause
for 5 seconds at the conclusion of each individual calibration process.  The process will continue
through all bands and will exit into regular radio mode after 10 meters is completed.  Calibration
numbers are saved to non-volatile EEPROM onboard the Teensy.  If desired, save to the SD card as
follows using the main function buttons:

EEPROM (select) Copy EEPROM->SD (select)

Note that individual band manual tuning remains possible.  Also, it is possible to automatically
calibrate a single band from the individual menu using button 4 (2nd row, 1st column).  After
the automatic calibration process completes, manual tuning again becomes available.

A faster, more precise process is possible.  This is also in the Calibrate menu, and is called
"Refine Cal".  This will calibrate all bands except it will use the current numbers as a starting
point.  Refine Cal will not proceed until Radio Cal has been completed.  Refine Cal will also work
in individual bands using button 7 (3rd row, 1st column).

Here are a few tips on using automated calibration modes:

1.  Turn on the radio for about 5 minutes before beginning automatic calibration modes.  This is so the circuitry is thermally stable before calibration begins.
2.  RF Auto-Gain status does not matter.  Calibration will use the rf gain value assigned to the band which is being calibrated.
3.  The desired carrier peak should be close to the top of the blue column.  Adjust the loop-back attenuation and/or the rf gain until this looks good.
4.  "Refine Cal" which uses the previously saved numbers as a starting point.  This runs a little faster.  Refine calibration
     can be run on the entire radio, all bands, as long as Radio Cal has been completed.  Refine cal can be run on individual band at any time (Filter button).
5.  Don't try to run Radio Cal or Refine Cal on an unhealthy radio.  If there is a band which is not working, the results may be unpredictable.

A short movie showing the latest automatic calibration features in action:

<https://drive.google.com/file/d/1bSDNnU4UXCL27KUWEFmHZ__9FZ7w2gWG/view?usp=sharing>

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