### Version T41EEE.7 T41 Software Defined Transceiver Arduino Sketch

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

T41EEE.7 was developed and compiled using Arduino IDE version 2.3.2 with the following
configuration:

1.  Optimize is set to "Smallest Code" (Tools menu).
2.  CPU speed is set to 528 MHz (Tools menu).
3.  TeensyDuino version is 1.59.0.
4.  You will need to install ArduinoJson which is currently version 7.1.0.

Completing a FLASH erase of the Teensy is strongly recommended before uploading this new version. 
Remember to save to the SD card via the EEPROM menu EEPROM->SD command prior to erasing.
The instructions for performing a FLASH erase of the Teensy are here:

<https://www.pjrc.com/store/teensy41.html#programming>

The bullet "Memory Wipe & LED Blink Restore" has the instructions.

## Highlight of Changes included in T41EEE.7

1.  Audio bandwidth equalized.  Noise reductions equalized.
2.  Changed audio filter delimiter line color to red.
3.  Reduced Spectral noise filter amplitude.
4.  Fixed problem with audio delimiters and reset tuning, also was not saving AutoSpectrum to SD.
5.  Added SSB calibration.
6.  Added audio adapter equalizer to transmit signal chain.
7.  CESSB modulation from Open Audio Library replaces conventional phasing SSB.
8.  Added working SSB options menu specific to CESSB.  Mic gain and compression menu removed.

## Controlled Envelope Single Side Band (CESSB)

Controlled Envelope Single Side Band is employed in T41EEE.7.  This web page has an excellent description of
CESSB technology:

<https://www.janbob.com/electron/CESSB_Teensy/CESSB_Teensy.html>

T41EEE.7 uses the Open Audio CESSB class as well as the type 2 Compressor class.

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/radioCESSB_Z_transmit_F32.h>

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/AudioEffectCompressor2_F32.h>

The Audio Adapter's hardware-based equalization filter is included in the CESSB transmit chain to provide
high-pass filtering of the microphone audio.

### CESSB Automatic Calibration

Automated calibration of the CESSB transmit signal chain is included in the Calibration menu.
Similar to the previously deployed automatic calibration, SSB Radio Cal and SSB Radio Refine Cal
commands will run the automatic calibration routines on all bands.  The automated calibrations can
also be run on a per-band basis.

Successful completion of either manual or automated calibration is required before proceeding with CESSB
transmitter alignments.  It is recommended to calibrate with SSB PA Cal set as high as possible.
If you have access to a spectrum analyzer, look at the transmitted spectrum from the output of the QSE
filter.  You should see a minimum of spurious outputs close to the desired carrier.

### CESSB Transmitter Alignment

It will help to understand the CESSB transmit signal chain as deployed in this firmware.
Nice block diagrams will be added to this in the near future.  For now, it is textual description.

Transmit Signal Chain
1.  Electret microphone biased via the Audio Adapter.  Assumed ~20 mV audio output from the electret.
2.  Microphone amplifier/attenuator stage.  The default gain is 0 dB.  This gain is user-adjustable.
3.  Open Audio Compressor 2.  Compression threshold and compression ratio is user adjustable.
4.  CESSB processing.  The input is the compressed audio, and the output is I and Q to the QSE modulator.
5.  QSE direct IQ modulator.  The I and Q channel amplitudes are user-adjustable via SSB PA Cal.

So what does the user have to adjust?:

1.  Microphone gain.
2.  Compression threshold.
3.  Compression ratio.
4.  SSB PA Cal.

OK, so now we will attempt to juggle the above four parameters to get a decent transmitted output.
Of the above four items, it is probably best to leave the compression ratio set to default for now.
Also, if you are using a typical electret microphone biased by the Audio Adapter, don't adjust the
microphone gain at first.  Leave it alone, and then maybe come back to it later for further optimization.

CESSB processes the voice audio in such a way that the peak-to-average power of the modulation is
significantly reduced.  So we will use that fact and a single-tone signal to adjust the transmitter
parameters.  The transmit signal chain is cascaded gain stages, and we don't won't overdrive to happen
at any node of the chain.  Overdrive will result in a distorted output and will cause adjacent-channel
splatter (increased bandwidth of the transmit energy).

To be continued ...

## Automated Calibration

Automated calibration is available starting with version T41EEE.6.

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