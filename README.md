### Version T41EEE.8 T41 Software Defined Transceiver Arduino Sketch

This is the "T41 Extreme Experimenter's Edition" software for the 
T41 Software Defined Transceiver.  The T41EEE was "forked" from the V049.2 version
of the T41-EP software.  T41EEE is fundamentally different from the mainstream T41-EP
code in that it adopts C++ rather than C-style code.  C++ language features will be gradually
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
Greg Raven, October 22 2024

T41EEE will be hosted at this public Github repository:

<https://github.com/Greg-R/T41EEE>

I don't plan on publishing periodic releases.  The repository will be updated as bug fixes
and feature enhancements become available.  You will be able to update and FLASH your radio
with the revised software quickly.

Please note that the configuration structure is different than the predecessor V049.2
It is recommended to perform a full FLASH erase before loading a new version of T41EEE.

You will need to install the ArduinoJson library by Benoit Blanchon.  Using the IDE:
Tools -> Manage Libraries ...
Search for Arduinojson.  Note this is a single word, as there is another library
with the name Arduino_JSON, which is not the correct library.  Install the library,
and if you have the other dependencies for the V049.2 version, compilation will be
successful.

## How to Compile T41EEE

T41EEE.8 was developed and compiled using Arduino IDE version 2.3.4 with the following
configuration:

1.  Optimize is set to "Smallest Code" (Tools menu).
2.  CPU speed is set to 528 MHz (Tools menu).
3.  TeensyDuino version is 1.59.0.
4.  You will need to install ArduinoJson which is currently version 7.2.1.
5.  You will need to update your copy of the Open Audio Library.  The library was updated
    on October 14, 2024:

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary>

Completing a FLASH erase of the Teensy is strongly recommended before uploading this new version. 
The instructions for performing a FLASH erase of the Teensy are here:

<https://www.pjrc.com/store/teensy41.html#programming>

The bullet "Memory Wipe & LED Blink Restore" has the instructions.

## Highlight of Changes included in T41EEE.8

1.  Changed default CW and SSB power calibration to 0.5.
2.  Fixed problem with CW decoder graphics.
3.  Fixed problem with switch matrix debug.
4.  Added variable speed fine tuning by Harry GM3RVL.
5.  Fixed audio drop-out with narrow filter settings.
6.  Adding RX Cal to SSB radio auto-calibration.
7.  Added DSPGAINSCALE parameter to MyConfigurationFile.h.
8.  Fixed AM and SAM demod problem.
9.  Add dBm signal level calibration.
10. SSB calibration is separate and independent from CW calibration.

## Variable Speed Fine Tuning by Harry GM3RVL

"Variable Speed Fine Tuning" allows for more rapid tuning when the rotation of the fine tuning encoder
exceeds a threshold.  This allows for quick investigation of signals which appear in the spectral display.
This feature is implemented by uncommenting the following line in MyConfigurationFile.h:

//#define FAST_TUNE                    // Uncomment to activate variable speed fast tune by Harry GM3RVL.

A big THANK YOU to Harry Brash GM3RVL for providing the code for this feature!

## dBm Signal Level Calibration

A new calibration menu option called "dBm Level Cal" allows adjustment of the dBm level indicator.
The indicator is calibrated by the input of a known signal level into the antenna connector.
Then choose "dBm Level Cal" from the Calibration menu.  Adjust the encoder until the dBm level
shown in the upper right area of the display matches the known signal level.

## DSPGAINSCALE Parameter

A new parameter is available in MyConfigurationFile.h:

#define DSPGAINSCALE 10.0  // A typical value for a V10/V11 radio is in the range of 5 to 15.

It is most likely that you will not need to adjust this parameter.  However, if you find that signals
are low or not appearing at all in the audio spectrum window, try adjusting this parameter upwards.

A big THANK YOU to Neville Marr ZL2BNE for the excellent feedback which resulted in the implementation of
this new parameter and also the dBm level calibration!

## Full SSB Calibration

SSB is now calibrated independently of CW.  In earlier versions, the calibration numbers were shared with CW.
To complete the SSB calibration process, you will need to adjust the band frequencies to be in the middle
of the SSB portion of each band prior to calibration.  SSB gain and phase numbers will be stored for each
of those frequencies.  This also includes receiver calibration, which was also being shared with CW
calibration in prior versions.

## Controlled Envelope Single Side Band (CESSB)

Controlled Envelope Single Side Band is employed in T41EEE.  This web page has an excellent description of
CESSB technology:

<https://www.janbob.com/electron/CESSB_Teensy/CESSB_Teensy.html>

T41EEE.7 uses the Open Audio CESSB class as well as the type 2 Compressor class.

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/radioCESSB_Z_transmit_F32.h>

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/AudioEffectCompressor2_F32.h>

The Audio Adapter's hardware-based equalization filter is included in the CESSB transmit chain to provide
high-pass filtering of the microphone audio.

### "SSB Data" Mode

"SSB Data" mode is selectable in the SSB Options menu.  This mode uses the same underlying modulator, however,
it bypasses the CESSB processing.  This mode can be used for conventional voice SSB if desired.  It can also
be used with data (for example FT8) which is normally fed into the SSB microphone input.

### CESSB Automatic Calibration

Automated calibration of the CESSB transmit signal chain is included in the Calibration menu.
Similar to the previously deployed automatic calibration, SSB Radio Cal and SSB Radio Refine Cal
commands will run the automatic calibration routines on all bands.  The automated calibrations can
also be run on a per-band basis.

Successful completion of either manual or automated calibration is required before proceeding with CESSB
transmitter alignments.  It is recommended to calibrate with SSB PA Cal set as high as possible.
If you have access to a spectrum analyzer, look at the transmitted spectrum from the output of the QSE
filter.  You should see a minimum of spurious outputs close to the desired carrier when SSB PA Cal is
set high, but not too high.

### CESSB Transmitter Alignment

It will help to understand the CESSB transmit signal chain as deployed in this firmware.
Nice block diagrams will be added to this in the near future.  For now, it is textual description.

Transmit Signal Chain
1.  Electret microphone biased via the Audio Adapter.  Assumed ~20 mV audio output from the electret.
2.  Microphone amplifier/attenuator stage.  The default gain is 0 dB.  This gain is user-adjustable.
3.  CESSB processing.  The input is the compressed audio, and the output is I and Q to the QSE modulator.
4.  QSE baseband-to-RF IQ modulator.  The I and Q channel amplitudes are user-adjustable via SSB PA Cal.

So what does the user have to adjust?:

1.  Microphone gain.
2.  SSB PA Cal.

OK, so now we will attempt to juggle the above parameters to get a decent transmitted output.
Also, if you are using a typical electret microphone biased by the Audio Adapter, don't adjust the
microphone gain at first.  Leave it alone, and then maybe come back to it later for further optimization.

CESSB processes the voice audio in such a way that the peak-to-average power of the modulation is
significantly reduced.  So we will use that fact and a single-tone signal to adjust the transmitter
parameters.  The transmit signal chain is cascaded gain stages, and we don't won't overdrive to happen
at any node of the chain.  Overdrive will result in a distorted output and will cause adjacent-channel
splatter (increased bandwidth of the transmit energy).

For starters, you will find more options in the Calibration sub-menu:
```
SSB Carrier
SSB Transmit
SSB Radio
SSB Refine
```
The CW calibrations are now separate and are indicated as such.
So in theory, the rest of the radio functions as before.  I hope, because one radical
departure is using a different sample rate during SSB transmit, which is 48 kHz.
This must be restored to 192 kHz when returning to receive, which I think is happening.
A few critical areas of the code changed to allow different sample rates.

So if you are adventurous, and want to try CESSB, I recommend this process which is a wild guess based on
many hours of playing around with the technology.  The reason it is a wild guess is that your hardware is
different.  So the signal amplitudes here will be different from the signal amplitudes there.  Good luck.

1.  Try to get SSB Carrier and SSB Transmit automatic calibration to work.  I recommend
setting SSB PA Cal to 0.8.  If this looks like it works, then connect the output of the QSE
to a spectrum analyzer, and run either SSB Carrier or Transmit cal, it doesn't matter.
This is just a check to see if the spectrums are reasonably free of higher order garbage.
There is going to be some garbage no matter what, but it better be suppressed 60 dB or better.
If garbage levels are too high, try again with a lower SSB PA Cal number.

Note that if you plan to drive the Power Amplifier with CESSB, you need to have a filter after the QSE.  Be sure to run calibration with the filter in place!  I put an attenuator on the output of the QSE to help isolate it from the filter.  I'm currently using a 6 dB attenuator, which seems kind of high, but there is still enough drive to the Power Amplifier.  Also if you are using the QSE2DC, there is a resistive Pi attenuator on the output of about 2 dB.  This is another variable in this system which is kind of up for grabs, and it depends on exactly what you've got.  Fun with homebrew radios!

Let me explain in detail SSB PA Cal.  This is nothing more than a constant linear multiplier of the I and Q channel amplitudes going into the QSE.  Thus you cannot allow the amplitude to go too high or you will cause distortion in the modulator.  SSB PA Cal setting is crucial and we will come back to this later in the process.  This step #1 is to make sure calibration happens at a level which is a bit below the saturation point of the modulator.  There is a balance point of good signal-to-noise ratio and keeping the signal below the level of generating intermodulation garbage.  So we are trying to find the sweet spot, calibrate there, and move onto the next step.

I should also comment that without having a spectrum analyzer capable of resolving the offset tone frequency 750Hz, that adjusting this thing will be dang near impossible.  I have a TinySA Ultra and that is probably the bare minimum that will suffice.

2.  Inject a 1 kHz tone, about 20 millivolts, into the microphone connector.  You will need to be able to short the PTT to ground to key the transmitter.  I use a cheap Amazon cable which terminates in RCA type connectors, which are plugged into RCA to BNC adapters.  This is sleazy but it works.  I use an alligator clip lead to short the PTT to ground.

While monitoring with a spectrum analyzer (output of the QSE filter), gradually increase the amplitude of the 1 kHz tone, and at some level, you should notice it is no longer a 1:1 increase.  Keep going and it will hit an upper limit, and will not go higher in amplitude.  You should see spurious sidebands increasing.

Now remember I said to adjust SSB PA Cal to 0.8 during calibration?
Now it is time to dial that down, drop it to 0.5, maybe even 0.4.
The reason for this is to prevent the CESSB output from overdriving the QSE during voice peaks.

Disconnect the 1 kHz tone source and plug in the microphone.
Now monitor the spectrum while keyed up and speaking into the microphone.
This should result in a very contained spectrum of approximately 3 kHz bandwidth.
There may be occasional splatter, but it should be relatively rare.  You should be able to almost yell into the microphone without causing splatter.  Well, maybe more like loud talking.

OK, if you got this far, the SSB transmitter is more or less adjusted, sans power amplifier, of course.  You can go ahead and connect the power amplifier, load and/or attenuator and see how that looks.  I see quite a bit of spectral spreading at the output of the K9HZ amplifier, but thisis probably normal.

There is another issue here.  After all of that, is the output high enough to drive up the amplifier???  I'm not sure what to do about that if it isn't.  Backing off the attenuator on the output of the QSE plus re-calibration is one possibility.


## Automated Calibration

Automated calibration is available starting with version T41EEE.6.

Automated calibration set-up is the same as for manual calibration.  A loop-back path from the
output of the QSE to the input of the QSD must be connected.  An attenuator, in the value of 
30 to 50 dB, must be inserted in this path.  The value for optimal calibration must be determined
empirically.  If a spectrum analyzer is available, it is strongly recommended to view the result
in a narrow sweep (~10kHz) to determine if the undesired sideband (and carrier in the case of QSE2DC)
is adequately suppressed.

After the loop-back path is inserted, use the following command from the main function buttons (2 and 5):

Calibrate (select) CW Radio Cal (select)

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
"CW Refine Cal".  This will calibrate all bands except it will use the current numbers as a starting
point.  Refine Cal will not proceed until Radio Cal has been completed.  Refine Cal will also work
in individual bands using button 7 (3rd row, 1st column).

There are also equivalent functions for SSB calibration.  SSB calibration factors are separated
from CW starting in version T41EEE.7.

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
