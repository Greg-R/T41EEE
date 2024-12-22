/*
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
3.  TeensyDuino version is 1.59.0.  Install TeensyDuino using the IDE Board Manager.
4.  You will need to install the Adafruit GFX library.  Install via the
    IDE Library Manager.
5.  You will need to install the ArduinoJson library.  Install via the
    IDE Library Manager.
6.  You will need to install the Etherkit Si5351 library.  Install via the IDE
    library manager.
7.  You will need to manually install the Open Audio Library.  The library was updated
    on October 14, 2024:

<https://github.com/chipaudette/OpenAudio_ArduinoLibrary>

8.  You will need to manually install the Rotary library:

<https://github.com/brianlow/Rotary>

The file MyConfigurationFile.h includes several compiler options which add, subtract, or
adjust parameters of some features.  Please review this file prior to compilation.

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
this new parameter and also dBm level calibration!

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
*/

// setup() and loop() are at the bottom of this file.

#include "SDT.h"
#include <charconv>

const char *filename = "/config.txt";  // <- SD library uses 8.3 filenames

struct maps myMapFiles[10] = {
  { "Cincinnati.bmp", 39.07466, -84.42677 },  // Map name and coordinates for QTH
  { "Denver.bmp", 39.61331, -105.01664 },
  { "Honolulu.bmp", 21.31165, -157.89291 },
  { "SiestaKey.bmp", 27.26657, -82.54197 },
  { "", 0.0, 0.0 },
  { "", 0.0, 0.0 },
  { "", 0.0, 0.0 },
  { "", 0.0, 0.0 },
  { "", 0.0, 0.0 }
};

struct band bands[NUMBER_OF_BANDS]{  //AFP Changed 1-30-21 // G0ORX Changed AGC to 20
//freq    band low   band hi   name    mode      Low    Hi  Gain  type    gain  AGC   pixel
//                                             filter filter             correct     offset
//DB2OO, 29-AUG-23: take ITU_REGION into account for band limits
// and changed "gainCorrection" to see the correct dBm value on all bands.
// Calibration done with TinySA as signal generator with -73dBm levels (S9) at the FT8 frequencies
// with V010 QSD with the 12V mod of the pre-amp
#if defined(ITU_REGION) && ITU_REGION == 1
 { 3700000UL, 3500000, 3800000, "80M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
 { 7150000, 7000000, 7200000, "40M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
#elif defined(ITU_REGION) && ITU_REGION == 2
 {  3700000UL, 3500000, 4000000, "80M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
 {  7150000, 7000000, 7300000, "40M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
#elif defined(ITU_REGION) && ITU_REGION == 3
 {  3700000UL, 3500000, 3900000, "80M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
 {  7150000, 7000000, 7200000, "40M", DEMOD_LSB, -200, -3000, 15, HAM_BAND, 1.0, 20},
#endif
 {  14200000, 14000000, 14350000, "20M", DEMOD_USB, 3000, 200, 15, HAM_BAND, 1.0, 20},
 {  18100000, 18068000, 18168000, "17M", DEMOD_USB, 3000, 200, 15, HAM_BAND, 1.0, 20},
 {  21200000, 21000000, 21450000, "15M", DEMOD_USB, 3000, 200, 15, HAM_BAND, 1.0, 20},
 {  24920000, 24890000, 24990000, "12M", DEMOD_USB, 3000, 200, 15, HAM_BAND, 1.0, 20},
 {  28350000, 28000000, 29700000, "10M", DEMOD_USB, 3000, 200, 15, HAM_BAND, 1.0, 20}
};

const char *topMenus[] = { "CW Options", "RF Set", "VFO Select",
                           "EEPROM", "AGC", "Spectrum Options",
                           "Noise Floor", "SSB Options",
                           "EQ Rec Set", "Calibrate", "Bearing" };

// Button array labels array is located in Utility.cpp.

// Pointers to functions which execute the menu options.  Do these functions used the returned integer???
void (*functionPtr[])() = { &CWOptions, &RFOptions, &VFOSelect,
                            &EEPROMOptions, &AGCOptions, &SpectrumOptions,
                            &ButtonSetNoiseFloor, &SSBOptions,
                            &EqualizerRecOptions, &CalibrateOptions, &BearingMaps };

uint32_t FFT_length = FFT_LENGTH;

//======================================== Global object definitions ==================================================
// ===========================  AFP 08-22-22
bool agc_action = false;
// Teensy and OpenAudio dataflow code.
#include "AudioSignal.h"
//End dataflow 

#include "Process.h"

Process process;         // Receiver process object.
CWCalibrate cwcalibrater;  // Instantiate the calibration objects.
SSBCalibrate ssbcalibrater;
JSON json;
Eeprom eeprom;           // Eeprom object.



Rotary volumeEncoder = Rotary(VOLUME_ENCODER_A, VOLUME_ENCODER_B);        //( 2,  3)
Rotary tuneEncoder = Rotary(TUNE_ENCODER_A, TUNE_ENCODER_B);              //(16, 17)
Rotary filterEncoder = Rotary(FILTER_ENCODER_A, FILTER_ENCODER_B);        //(15, 14)
Rotary fineTuneEncoder = Rotary(FINETUNE_ENCODER_A, FINETUNE_ENCODER_B);  //( 4,  5)

Metro ms_500 = Metro(500);  // Set up a Metro

Si5351 si5351;  // Instantiate the PLL device.

RadioState radioState, lastState;  // KF5N
RadioMode radioMode;  // Greg KF5N August 1, 2024
int resetTuningFlag = 0;
#ifndef RA8875_DISPLAY
ILI9488_t3 tft = ILI9488_t3(&SPI, TFT_CS, TFT_DC, TFT_RST);  // Instantiate the display.
#else
#define RA8875_CS TFT_CS
#define RA8875_RESET TFT_DC  // any pin or nothing!
RA8875 tft = RA8875(RA8875_CS, RA8875_RESET);
#endif

SPISettings settingsA(70000000UL, MSBFIRST, SPI_MODE1);

const uint32_t N_B_EX = 16;

// HP BiQuad IIR DC filter
float32_t HP_DC_Butter_state[6] = { 0, 0, 0, 0, 0, 0 };
float32_t HP_DC_Butter_state2[2] = { 0, 0 };                                                          // AFP 11-04-11
arm_biquad_cascade_df2T_instance_f32 s1_Receive = { 3, HP_DC_Butter_state, HP_DC_Filter_Coeffs };     //AFP 09-23-22
arm_biquad_cascade_df2T_instance_f32 s1_Receive2 = { 1, HP_DC_Butter_state2, HP_DC_Filter_Coeffs2 };  //AFP 11-04-22

// CW decode Filters
arm_fir_instance_f32 FIR_CW_DecodeL;  //AFP 10-25-22
arm_fir_instance_f32 FIR_CW_DecodeR;  //AFP 10-25-22
float32_t DMAMEM FIR_CW_DecodeL_state[64 + 256 - 1];
float32_t DMAMEM FIR_CW_DecodeR_state[64 + 256 - 1];

//Decimation and Interpolation Filters
arm_fir_interpolate_instance_f32 FIR_int1_EX_I;
arm_fir_interpolate_instance_f32 FIR_int1_EX_Q;
arm_fir_interpolate_instance_f32 FIR_int2_EX_I;
arm_fir_interpolate_instance_f32 FIR_int2_EX_Q;

float32_t audioMaxSquaredAve;

float32_t DMAMEM FIR_int2_EX_I_state[519];
float32_t DMAMEM FIR_int2_EX_Q_state[519];
float32_t DMAMEM FIR_int1_EX_I_state[279];
float32_t DMAMEM FIR_int1_EX_Q_state[279];
float32_t DMAMEM float_buffer_L_EX[2048];
float32_t DMAMEM float_buffer_R_EX[2048];
float32_t DMAMEM float_buffer_LTemp[2048];
float32_t DMAMEM float_buffer_RTemp[2048];
//==================== End Excite Variables================================

//======================================== Global structure declarations ===============================================

struct config_t EEPROMData;

const struct SR_Descriptor SR[18] = {
  //   SR_n,        rate,  text
  { SAMPLE_RATE_8K, 8000, "  8k" },      // not OK
  { SAMPLE_RATE_11K, 11025, " 11k" },    // not OK
  { SAMPLE_RATE_16K, 16000, " 16k" },    // OK
  { SAMPLE_RATE_22K, 22050, " 22k" },    // OK
  { SAMPLE_RATE_32K, 32000, " 32k" },    // OK, one more indicator?
  { SAMPLE_RATE_44K, 44100, " 44k" },    // OK
  { SAMPLE_RATE_48K, 48000, " 48k" },    // OK
  { SAMPLE_RATE_50K, 50223, " 50k" },    // NOT OK
  { SAMPLE_RATE_88K, 88200, " 88k" },    // OK
  { SAMPLE_RATE_96K, 96000, " 96k" },    // OK
  { SAMPLE_RATE_100K, 100000, "100k" },  // NOT OK
  { SAMPLE_RATE_101K, 100466, "101k" },  // NOT OK
  { SAMPLE_RATE_176K, 176400, "176k" },  // OK
  { SAMPLE_RATE_192K, 192000, "192k" },  // OK    THIS IS USED IN THE T41
  { SAMPLE_RATE_234K, 234375, "234k" },  // NOT OK
  { SAMPLE_RATE_256K, 256000, "256k" },  // NOT OK
  { SAMPLE_RATE_281K, 281000, "281k" },  // NOT OK
  { SAMPLE_RATE_353K, 352800, "353k" }   // NOT OK
};

const arm_cfft_instance_f32 *S;
const arm_cfft_instance_f32 *iS;
const arm_cfft_instance_f32 *maskS;
const arm_cfft_instance_f32 *NR_FFT;
const arm_cfft_instance_f32 *NR_iFFT;
const arm_cfft_instance_f32 *spec_FFT;

arm_biquad_casd_df1_inst_f32 biquad_lowpass1;

arm_fir_decimate_instance_f32 FIR_dec1_I;
arm_fir_decimate_instance_f32 FIR_dec1_Q;
arm_fir_decimate_instance_f32 FIR_dec2_I;
arm_fir_decimate_instance_f32 FIR_dec2_Q;
arm_fir_interpolate_instance_f32 FIR_int1_I;
arm_fir_interpolate_instance_f32 FIR_int1_Q;
arm_fir_interpolate_instance_f32 FIR_int2_I;
arm_fir_interpolate_instance_f32 FIR_int2_Q;
arm_lms_norm_instance_f32 LMS_Norm_instance;

const DEMOD_Descriptor DEMOD[3] = {
  //   DEMOD_n, name
  { DEMOD_USB, "(USB)" },
  { DEMOD_LSB, "(LSB)" },
  { DEMOD_AM, "(AM)" },  //AFP09-22-22
};

dispSc displayScale[] =  // dbText, dBScale, baseOffset
  {
    { "20 dB/", 10.0, 24 },
    { "10 dB/", 20.0, 10 }  //  1, 2, and 5 dB options removed.  Greg KF5N July 30, 2024.
  };

//======================================== Global variables declarations for Quad Oscillator 2 ===============================================
int32_t NCOFreq;

//======================================== Global variables declarations ===============================================
//================== Global CW Correlation and FFT Variables =================
float32_t *cosBuffer = new float32_t[256];  // Was cosBuffer2; Greg KF5N February 7, 2024
float32_t *sinBuffer = new float32_t[256];  // This can't be DMAMEM.  It will cause problems with the CW decoder.
float32_t DMAMEM cwRiseBuffer[256];
float32_t DMAMEM cwFallBuffer[256];
// ===========

char keyboardBuffer[10];  // Set for call prefixes. May be increased later
const char *tune_text = "Fast Tune";
const char *zoomOptions[] = { "1x ", "2x ", "4x ", "8x ", "16x" };

float32_t pixel_per_khz = ((1 << EEPROMData.spectrum_zoom) * SPECTRUM_RES * 1000.0 / SR[SampleRate].rate);
int pos_left = centerLine - (int)(bands[EEPROMData.currentBand].FLoCut / 1000.0 * pixel_per_khz);

int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2;
int16_t fftOffset = 0;
int fLoCutOld;
int fHiCutOld;
int filterWidth = (int)((bands[EEPROMData.currentBand].FHiCut - bands[EEPROMData.currentBand].FLoCut) / 1000.0 * pixel_per_khz);
int h = SPECTRUM_HEIGHT + 3;
bool ANR_notch = false;
uint8_t auto_codec_gain = 1;
uint8_t display_S_meter_or_spectrum_state = 0;
uint8_t keyPressedOn = 0;
uint8_t NR_first_time = 1;
uint8_t NR_Kim;

uint32_t SampleRate = SAMPLE_RATE_192K;

uint32_t zoom_display = 1;

int16_t pixelCurrent[SPECTRUM_RES];
int16_t pixelnew[SPECTRUM_RES];
int16_t pixelold[SPECTRUM_RES];

//===== New histogram stuff ===
volatile int filterEncoderMove = 0;
volatile long fineTuneEncoderMove = 0L;

int selectedMapIndex;

int centerTuneFlag = 0;
unsigned long cwTimer;
unsigned long ditTimerOn;
unsigned long ditLength;
unsigned long transmitDitLength;  // JJP 8/19/23
unsigned long transmitDitUnshapedBlocks;
unsigned long transmitDahUnshapedBlocks;

// ============ end new stuff =======
// Global variables used by audio filter encoder.
int last_filter_pos = 0;
int filter_pos = 0;

int16_t y_old, y_new, y1_new, y1_old, y_old2;  //A

const float32_t DF1 = 4.0;                                         // decimation factor
const float32_t DF2 = 2.0;                                         // decimation factor
const float32_t DF = DF1 * DF2;                                    // decimation factor
const float32_t n_samplerate = 176.0;                              // sample rate before decimation
const uint32_t N_B = FFT_LENGTH / 2 / BUFFER_SIZE * (uint32_t)DF;  // 512/2/128 * 8 = 16
const uint32_t N_DEC_B = N_B / (uint32_t)DF;
const uint32_t NR_add_counter = 128;
const float32_t n_desired_BW = 9.0;  // desired max BW of the filters
const float32_t n_fpass1 = n_desired_BW / n_samplerate;
const float32_t n_fpass2 = n_desired_BW / (n_samplerate / DF1);
const float32_t n_fstop1 = ((n_samplerate / DF1) - n_desired_BW) / n_samplerate;
const float32_t n_fstop2 = ((n_samplerate / (DF1 * DF2)) - n_desired_BW) / (n_samplerate / DF1);
const uint32_t IIR_biquad_Zoom_FFT_N_stages = 4;
const uint32_t N_stages_biquad_lowpass1 = 1;
const float32_t n_att = 90.0;  // need here for later def's
const uint16_t n_dec1_taps = (1 + (uint16_t)(n_att / (22.0 * (n_fstop1 - n_fpass1))));
const uint16_t n_dec2_taps = (1 + (uint16_t)(n_att / (22.0 * (n_fstop2 - n_fpass2))));

int attenuator = 0;

int audioYPixel[256]{ 0 };  // Will int16_t save memory here???  DMAMEM not working here.  Causes audio spectrum glitch.  KF5N February 26, 2024.

int bandswitchPins[] = {
  30,  // 80M
  31,  // 40M
  28,  // 20M
  29,  // 17M
  29,  // 15M
  0,   // 12M  Note that 12M and 10M both use the 10M filter, which is always in (no relay).  KF5N September 27, 2023.
  0    // 10M
};

bool calibrateFlag = 0;
bool calOnFlag = false;
int chipSelect = BUILTIN_SDCARD;
int idx;
int NR_Index = 0;
int n_L;
int n_R;
int newCursorPosition = 0;
int oldCursorPosition = 256;
bool switchFilterSideband = false;
int x2 = 0;  //AFP

int zoomIndex = 1;  //AFP 9-26-22
bool updateDisplayFlag = false;
int updateDisplayCounter = 0;

const int BW_indicator_y = SPECTRUM_TOP_Y + SPECTRUM_HEIGHT + 2;
const int DEC2STATESIZE = n_dec2_taps + (BUFFER_SIZE * N_B / (uint32_t)DF1) - 1;
const int INT1_STATE_SIZE = 24 + BUFFER_SIZE * N_B / (uint32_t)DF - 1;
const int INT2_STATE_SIZE = 8 + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1;

int32_t mainMenuIndex = START_MENU;  // Done so we show menu[0] at startup

uint32_t N_BLOCKS = N_B;

uint32_t s_roomC_hotC; /*!< The value of s_roomCount minus s_hotCount.*/
uint32_t s_hotTemp;    /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at room temperature .*/
uint32_t s_hotCount;   /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at the hot temperature.*/
uint32_t currentFreq;
long int n_clear;
uint32_t TxRxFreq;  // = EEPROMData.centerFreq + NCOFreq  NCOFreq from FreqShift2()

float help;
float s_hotT_ROOM; /*!< The value of s_hotTemp minus room temperature(25¡æ).*/

//====== SAM stuff AFP 11-02-22
float32_t a[3 * SAM_PLL_HILBERT_STAGES + 3];
float32_t b[3 * SAM_PLL_HILBERT_STAGES + 3];
float32_t c[3 * SAM_PLL_HILBERT_STAGES + 3];  // Filter c variables

float32_t c1[SAM_PLL_HILBERT_STAGES];
float32_t d[3 * SAM_PLL_HILBERT_STAGES + 3];

float32_t audio;

float32_t mid = 0.0;
float32_t bin_BW = 1.0 / (DF * FFT_length) * SR[SampleRate].rate;
float32_t bin = 2000.0 / bin_BW;
float32_t biquad_lowpass1_state[N_stages_biquad_lowpass1 * 4];
float32_t biquad_lowpass1_coeffs[5 * N_stages_biquad_lowpass1] = { 0, 0, 0, 0, 0 };
float32_t DMAMEM buffer_spec_FFT[1024] __attribute__((aligned(4)));
float32_t coefficient_set[5] = { 0, 0, 0, 0, 0 };
float32_t corr[2];

float32_t dbm = -145.0;

float32_t DMAMEM FFT_buffer[FFT_LENGTH * 2] __attribute__((aligned(4))) = { 0 };
float32_t DMAMEM FFT_spec[1024] = { 0 };
float32_t DMAMEM FFT_spec_old[1024] = { 0 };

// Decimation with FIR lowpass for Zoom FFT
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I1;
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q1;
float32_t DMAMEM Fir_Zoom_FFT_Decimate_I1_state[12 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate_Q1_state[12 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate1_coeffs[12];

// decimation with FIR lowpass for Zoom FFT
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I2;
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q2;
float32_t DMAMEM Fir_Zoom_FFT_Decimate_I2_state[12 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate_Q2_state[12 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate2_coeffs[12];
float32_t DMAMEM FIR_Coef_I[(FFT_LENGTH / 2) + 1];
float32_t DMAMEM FIR_Coef_Q[(FFT_LENGTH / 2) + 1];
float32_t DMAMEM FIR_dec1_I_state[n_dec1_taps + (uint16_t)BUFFER_SIZE * (uint32_t)N_B - 1];
float32_t DMAMEM FIR_dec2_I_state[DEC2STATESIZE];
float32_t DMAMEM FIR_dec2_coeffs[n_dec2_taps];
float32_t DMAMEM FIR_dec2_Q_state[DEC2STATESIZE];
float32_t DMAMEM FIR_int2_I_state[INT2_STATE_SIZE];
float32_t DMAMEM FIR_int2_Q_state[INT2_STATE_SIZE];
float32_t DMAMEM FIR_int1_coeffs[48];
float32_t DMAMEM FIR_int2_coeffs[32];
float32_t DMAMEM FIR_dec1_Q_state[n_dec1_taps + (uint16_t)BUFFER_SIZE * (uint16_t)N_B - 1];
float32_t DMAMEM FIR_dec1_coeffs[n_dec1_taps];
float32_t DMAMEM FIR_filter_mask[FFT_LENGTH * 2] __attribute__((aligned(4)));
float32_t DMAMEM FIR_int1_I_state[INT1_STATE_SIZE];
float32_t DMAMEM FIR_int1_Q_state[INT1_STATE_SIZE];
float32_t DMAMEM Fir_Zoom_FFT_Decimate_I_state[4 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate_Q_state[4 + BUFFER_SIZE * N_B - 1];
float32_t DMAMEM Fir_Zoom_FFT_Decimate_coeffs[4];
float32_t DMAMEM float_buffer_L[BUFFER_SIZE * N_B];
float32_t DMAMEM float_buffer_R[BUFFER_SIZE * N_B];
//float32_t DMAMEM float_buffer_L2[BUFFER_SIZE * N_B];
//float32_t DMAMEM float_buffer_R2[BUFFER_SIZE * N_B];
float32_t DMAMEM float_buffer_L_CW[256];       //AFP 09-01-22
float32_t DMAMEM float_buffer_R_CW[256];       //AFP 09-01-22
float32_t DMAMEM float_buffer_R_AudioCW[256];  //AFP 10-18-22
float32_t DMAMEM float_buffer_L_AudioCW[256];  //AFP 10-18-22
float32_t DMAMEM iFFT_buffer[FFT_LENGTH * 2 + 1];
float32_t IIR_biquad_Zoom_FFT_I_state[IIR_biquad_Zoom_FFT_N_stages * 4];
float32_t IIR_biquad_Zoom_FFT_Q_state[IIR_biquad_Zoom_FFT_N_stages * 4];
float32_t DMAMEM last_sample_buffer_L[BUFFER_SIZE * N_DEC_B];
float32_t DMAMEM last_sample_buffer_R[BUFFER_SIZE * N_DEC_B];
float32_t DMAMEM L_BufferOffset[BUFFER_SIZE * N_B];
float32_t DMAMEM NR_FFT_buffer[512] __attribute__((aligned(4)));
float32_t DMAMEM NR_output_audio_buffer[NR_FFT_L];
float32_t DMAMEM NR_last_iFFT_result[NR_FFT_L / 2];
float32_t DMAMEM NR_last_sample_buffer_L[NR_FFT_L / 2];
float32_t DMAMEM NR_last_sample_buffer_R[NR_FFT_L / 2];
float32_t DMAMEM NR_X[NR_FFT_L / 2][3];
float32_t DMAMEM NR_E[NR_FFT_L / 2][15];
float32_t DMAMEM NR_M[NR_FFT_L / 2];
float32_t DMAMEM NR_Nest[NR_FFT_L / 2][2];  //
float32_t DMAMEM NR_lambda[NR_FFT_L / 2];
float32_t DMAMEM NR_Gts[NR_FFT_L / 2][2];
float32_t DMAMEM NR_G[NR_FFT_L / 2];
float32_t DMAMEM NR_SNR_prio[NR_FFT_L / 2];
float32_t DMAMEM NR_SNR_post[NR_FFT_L / 2];
float32_t DMAMEM NR_Hk_old[NR_FFT_L / 2];
float32_t DMAMEM NR_long_tone[NR_FFT_L / 2][2];
float32_t DMAMEM NR_long_tone_gain[NR_FFT_L / 2];
float32_t DMAMEM R_BufferOffset[BUFFER_SIZE * N_B];
float32_t tmp;
float32_t w;
float angl;
float pi = 3.14159265358979;
float tau;
float temp;
bool volumeChangeFlag = false;

// Voltage in one-hundred 1 dB steps for volume control.
const float32_t volumeLog[] = { 0.000010, 0.000011, 0.000013, 0.000014, 0.000016, 0.000018, 0.000020, 0.000022, 0.000025, 0.000028,
                                0.000032, 0.000035, 0.000040, 0.000045, 0.000050, 0.000056, 0.000063, 0.000071, 0.000079, 0.000089,
                                0.000100, 0.000112, 0.000126, 0.000141, 0.000158, 0.000178, 0.000200, 0.000224, 0.000251, 0.000282,
                                0.000316, 0.000355, 0.000398, 0.000447, 0.000501, 0.000562, 0.000631, 0.000708, 0.000794, 0.000891,
                                0.001000, 0.001122, 0.001259, 0.001413, 0.001585, 0.001778, 0.001995, 0.002239, 0.002512, 0.002818,
                                0.003162, 0.003548, 0.003981, 0.004467, 0.005012, 0.005623, 0.006310, 0.007079, 0.007943, 0.008913,
                                0.010000, 0.011220, 0.012589, 0.014125, 0.015849, 0.017783, 0.019953, 0.022387, 0.025119, 0.028184,
                                0.031623, 0.035481, 0.039811, 0.044668, 0.050119, 0.056234, 0.063096, 0.070795, 0.079433, 0.089125,
                                0.100000, 0.112202, 0.125893, 0.141254, 0.158489, 0.177828, 0.199526, 0.223872, 0.251189, 0.281838,
                                0.316228, 0.354813, 0.398107, 0.446684, 0.501187, 0.562341, 0.630957, 0.707946, 0.794328, 0.891251, 1.000000 };

double elapsed_micros_idx_t = 0;

double elapsed_micros_sum;

/*****
  Purpose: To read the local time

  Parameter list:
    void

  Return value:
    time_t                a time data point
*****/
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

// is added in Teensyduino 1.52 beta-4, so this can be deleted !?

/*****
  Purpose: To set the real time clock

  Parameter list:
    unsigned long t

  Return value:
    void
*****/
void FLASHMEM T4_rtc_set(unsigned long t) {
  //#if defined (T4)
#if 0
  // stop the RTC
  SNVS_HPCR &= ~(SNVS_HPCR_RTC_EN | SNVS_HPCR_HP_TS);
  while (SNVS_HPCR & SNVS_HPCR_RTC_EN); // wait
  // stop the SRTC
  SNVS_LPCR &= ~SNVS_LPCR_SRTC_ENV;
  while (SNVS_LPCR & SNVS_LPCR_SRTC_ENV); // wait
  // set the SRTC
  SNVS_LPSRTCLR = t << 15;
  SNVS_LPSRTCMR = t >> 17;
  // start the SRTC
  SNVS_LPCR |= SNVS_LPCR_SRTC_ENV;
  while (!(SNVS_LPCR & SNVS_LPCR_SRTC_ENV)); // wait
  // start the RTC and sync it to the SRTC
  SNVS_HPCR |= SNVS_HPCR_RTC_EN | SNVS_HPCR_HP_TS;
#endif
}


/*****
  Purpose: void initTempMon

  Parameter list:
    void
  Return value;
    void
*****/
void FLASHMEM initTempMon(uint16_t freq, uint32_t lowAlarmTemp, uint32_t highAlarmTemp, uint32_t panicAlarmTemp) {
  uint32_t calibrationData;
  uint32_t roomCount;
  //first power on the temperature sensor - no register change
  TEMPMON_TEMPSENSE0 &= ~TMS0_POWER_DOWN_MASK;
  TEMPMON_TEMPSENSE1 = TMS1_MEASURE_FREQ(freq);

  calibrationData = HW_OCOTP_ANA1;
  s_hotTemp = (uint32_t)(calibrationData & 0xFFU) >> 0x00U;
  s_hotCount = (uint32_t)(calibrationData & 0xFFF00U) >> 0X08U;
  roomCount = (uint32_t)(calibrationData & 0xFFF00000U) >> 0x14U;
  s_hotT_ROOM = s_hotTemp - TEMPMON_ROOMTEMP;
  s_roomC_hotC = roomCount - s_hotCount;
}

/*****
  Purpose: Read the Teensy's temperature. Get worried over 50C

  Parameter list:
    void

  Return value:
    float           temperature Centigrade
*****/
float TGetTemp() {
  uint32_t nmeas;
  float tmeas;
  while (!(TEMPMON_TEMPSENSE0 & 0x4U)) {
    ;
  }
  /* ready to read temperature code value */
  nmeas = (TEMPMON_TEMPSENSE0 & 0xFFF00U) >> 8U;
  tmeas = s_hotTemp - (float)((nmeas - s_hotCount) * s_hotT_ROOM / s_roomC_hotC);  // Calculate temperature
  return tmeas;
}


// Teensy 4.0, 4.1
/*****
  Purpose: To set the I2S frequency

  Parameter list:
    int freq        the frequency to set

  Return value:
    int             the frequency or 0 if too large

*****/
int SetI2SFreq(int freq) {
  int n1;
  int n2;
  int c0;
  int c2;
  int c1;
  double C;

  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  // Fudge to handle 8kHz - El Supremo
  if (freq > 8000) {
    n1 = 4;  //SAI prescaler 4 => (n1*n2) = multiple of 4
  } else {
    n1 = 8;
  }
  n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  if (n2 > 63) {
    // n2 must fit into a 6-bit field
#ifdef DEBUG
    Serial.printf("ERROR: n2 exceeds 63 - %d\n", n2);
#endif
    return 0;
  }
  C = ((double)freq * 256 * n1 * n2) / 24000000;
  c0 = C;
  c2 = 10000;
  c1 = C * c2 - (c0 * c2);
  set_audioClock(c0, c1, c2, true);
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
               | CCM_CS1CDR_SAI1_CLK_PRED(n1 - 1)   // &0x07
               | CCM_CS1CDR_SAI1_CLK_PODF(n2 - 1);  // &0x3f

  CCM_CS2CDR = (CCM_CS2CDR & ~(CCM_CS2CDR_SAI2_CLK_PRED_MASK | CCM_CS2CDR_SAI2_CLK_PODF_MASK))
               | CCM_CS2CDR_SAI2_CLK_PRED(n1 - 1)   // &0x07
               | CCM_CS2CDR_SAI2_CLK_PODF(n2 - 1);  // &0x3f)
  return freq;
}


/*****
  Purpose: to collect array inits in one place

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void InitializeDataArrays() {
  uint16_t temp_check_frequency;
  uint32_t highAlarmTemp, lowAlarmTemp, panicAlarmTemp;
  int LP_F_help;
  uint32_t m_NumTaps = (FFT_LENGTH / 2) + 1;
  //DB2OO, 11-SEP-23: don't use the fixed sizes, but use the caculated ones, otherwise a code change will create very difficult to find problems
#define CLEAR_VAR(x) memset(x, 0, sizeof(x))
  memset(FFT_spec_old, 0, sizeof(FFT_spec_old));
#ifdef DEBUG
  Serial.printf("InitializeDataArrays(): sizeof(FFT_spec_old) %d", sizeof(FFT_spec_old));
  Serial.printf("\tsizeof(NR_output_audio_buffer) %d", sizeof(NR_output_audio_buffer));
  Serial.println();
#endif
  //  CLEAR_VAR(FFT_spec_old);             //memset(FFT_spec_old, 0, 4096);            // SPECTRUM_RES = 512 * 4 = 2048
  CLEAR_VAR(pixelnew);                 //memset(pixelnew, 0, 1024);                // 512 * 2
  CLEAR_VAR(pixelold);                 //memset(pixelold, 0, 1024);                // 512 * 2
  CLEAR_VAR(pixelCurrent);             //memset(pixelCurrent, 0, 1024);            // 512 * 2  KF5N JJP  7/14/23
  CLEAR_VAR(buffer_spec_FFT);          //memset(buffer_spec_FFT, 0, 4096);         // SPECTRUM_RES = 512 * 2 = 1024
  CLEAR_VAR(NR_FFT_buffer);            //memset(NR_FFT_buffer, 0, 2048);           // NR_FFT_L * sizeof(NR_FFT_buffer[0]));
  CLEAR_VAR(NR_output_audio_buffer);   //memset(NR_output_audio_buffer, 0, 1024);  // 256 * sizeof(NR_output_audio_buffer[0]));
  CLEAR_VAR(NR_last_iFFT_result);      //memset(NR_last_iFFT_result, 0, 512);
  CLEAR_VAR(NR_last_sample_buffer_L);  //memset(NR_last_sample_buffer_L, 0, 512);
  CLEAR_VAR(NR_last_sample_buffer_R);  //memset(NR_last_sample_buffer_R, 0, 512);
  CLEAR_VAR(NR_M);                     //memset(NR_M, 0, 512);
  CLEAR_VAR(NR_lambda);                //memset(NR_lambda, 0, 512);
  CLEAR_VAR(NR_G);                     //memset(NR_G, 0, 512);
  CLEAR_VAR(NR_SNR_prio);              //memset(NR_SNR_prio, 0, 512);
  CLEAR_VAR(NR_SNR_post);              //memset(NR_SNR_post, 0, 512);
  CLEAR_VAR(NR_Hk_old);                //memset(NR_Hk_old, 0, 512);
  CLEAR_VAR(NR_X);                     //memset(NR_X, 0, 1536);
  CLEAR_VAR(NR_Nest);                  //memset(NR_Nest, 0, 1024);
  CLEAR_VAR(NR_Gts);                   //memset(NR_Gts, 0, 1024);
  CLEAR_VAR(NR_E);                     //memset(NR_E, 0, 7680);

  CalcCplxFIRCoeffs(FIR_Coef_I, FIR_Coef_Q, m_NumTaps, (float32_t)bands[EEPROMData.currentBand].FLoCut, (float32_t)bands[EEPROMData.currentBand].FHiCut, (float)SR[SampleRate].rate / DF);

  /****************************************************************************************
     init complex FFTs
  ****************************************************************************************/
  switch (FFT_length) {
    case 2048:
      S = &arm_cfft_sR_f32_len2048;
      iS = &arm_cfft_sR_f32_len2048;
      maskS = &arm_cfft_sR_f32_len2048;
      break;
    case 1024:
      S = &arm_cfft_sR_f32_len1024;
      iS = &arm_cfft_sR_f32_len1024;
      maskS = &arm_cfft_sR_f32_len1024;
      break;
    case 512:
      S = &arm_cfft_sR_f32_len512;
      iS = &arm_cfft_sR_f32_len512;
      maskS = &arm_cfft_sR_f32_len512;
      break;
  }

  spec_FFT = &arm_cfft_sR_f32_len512;  //Changed specification to 512 instance
  NR_FFT = &arm_cfft_sR_f32_len256;
  NR_iFFT = &arm_cfft_sR_f32_len256;

  /****************************************************************************************
     Calculate the FFT of the FIR filter coefficients once to produce the FIR filter mask
  ****************************************************************************************/
  InitFilterMask();

  /****************************************************************************************
     Set sample rate
  ****************************************************************************************/
  SetI2SFreq(SR[SampleRate].rate);

  biquad_lowpass1.numStages = N_stages_biquad_lowpass1;  // set number of stages
  biquad_lowpass1.pCoeffs = biquad_lowpass1_coeffs;      // set pointer to coefficients file

  for (unsigned i = 0; i < 4 * N_stages_biquad_lowpass1; i++) {
    biquad_lowpass1_state[i] = 0.0;  // set state variables to zero
  }
  biquad_lowpass1.pState = biquad_lowpass1_state;  // set pointer to the state variables

  /****************************************************************************************
     set filter bandwidth of IIR filter
  ****************************************************************************************/
  // also adjust IIR AM filter
  // calculate IIR coeffs
  LP_F_help = bands[EEPROMData.currentBand].FHiCut;
  if (LP_F_help < -bands[EEPROMData.currentBand].FLoCut)
    LP_F_help = -bands[EEPROMData.currentBand].FLoCut;
  SetIIRCoeffs((float32_t)LP_F_help, 1.3, (float32_t)SR[SampleRate].rate / DF, 0);  // 1st stage
  for (int i = 0; i < 5; i++) {                                                     // fill coefficients into the right file
    biquad_lowpass1_coeffs[i] = coefficient_set[i];
  }

  /****************************************************************************************
     Initiate decimation and interpolation FIR filters
  ****************************************************************************************/
  // Decimation filter 1, M1 = DF1
  //    CalcFIRCoeffs(FIR_dec1_coeffs, 25, (float32_t)5100.0, 80, 0, 0.0, (float32_t)SR[SampleRate].rate);
  CalcFIRCoeffs(FIR_dec1_coeffs, n_dec1_taps, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SampleRate].rate);

  if (arm_fir_decimate_init_f32(&FIR_dec1_I, n_dec1_taps, (uint32_t)DF1, FIR_dec1_coeffs, FIR_dec1_I_state, BUFFER_SIZE * N_BLOCKS)) {
    while (1)
      ;
  }

  if (arm_fir_decimate_init_f32(&FIR_dec1_Q, n_dec1_taps, (uint32_t)DF1, FIR_dec1_coeffs, FIR_dec1_Q_state, BUFFER_SIZE * N_BLOCKS)) {
    while (1)
      ;
  }
  // Decimation filter 2, M2 = DF2
  CalcFIRCoeffs(FIR_dec2_coeffs, n_dec2_taps, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)(SR[SampleRate].rate / DF1));
  if (arm_fir_decimate_init_f32(&FIR_dec2_I, n_dec2_taps, (uint32_t)DF2, FIR_dec2_coeffs, FIR_dec2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }

  if (arm_fir_decimate_init_f32(&FIR_dec2_Q, n_dec2_taps, (uint32_t)DF2, FIR_dec2_coeffs, FIR_dec2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }

  // Interpolation filter 1, L1 = 2
  // not sure whether I should design with the final sample rate ??
  // yes, because the interpolation filter is AFTER the upsampling, so it has to be in the target sample rate!
  CalcFIRCoeffs(FIR_int1_coeffs, 48, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, SR[SampleRate].rate / 4.0);
  if (arm_fir_interpolate_init_f32(&FIR_int1_I, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    while (1)
      ;
  }
  if (arm_fir_interpolate_init_f32(&FIR_int1_Q, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    while (1)
      ;
  }
  // Interpolation filter 2, L2 = 4
  // not sure whether I should design with the final sample rate ??
  // yes, because the interpolation filter is AFTER the upsampling, so it has to be in the target sample rate!
  CalcFIRCoeffs(FIR_int2_coeffs, 32, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SampleRate].rate);

  if (arm_fir_interpolate_init_f32(&FIR_int2_I, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }
  if (arm_fir_interpolate_init_f32(&FIR_int2_Q, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }

  SetDecIntFilters();  // The correct bandwidths are calculated and set accordingly.
  ZoomFFTPrep();
  SpectralNoiseReductionInit();
  InitLMSNoiseReduction();

  temp_check_frequency = 0x03U;  //updates the temp value at a RTC/3 clock rate
  //0xFFFF determines a 2 second sample rate period
  highAlarmTemp = 85U;  //42 degrees C
  lowAlarmTemp = 25U;
  panicAlarmTemp = 90U;

  initTempMon(temp_check_frequency, lowAlarmTemp, highAlarmTemp, panicAlarmTemp);
  // this starts the measurements
  TEMPMON_TEMPSENSE0 |= 0x2U;
}  // end InitializeDataArrays()


/*****
  Purpose: The initial screen display on startup. Expect this to be customized.

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void Splash() {
  int centerCall;
  tft.fillWindow(RA8875_BLACK);
  tft.setFontScale(3);
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(140, 20);  // XPIXELS is 800 for 5 inch display.  YPIXELS is 480.
  tft.print("T41-EP SDR Radio");
  tft.setTextColor(RA8875_RED);
  tft.setCursor(35, 85);
  tft.setFontScale(2);
  tft.print("EXTREME EXPERIMENTER'S EDITION");
  tft.setCursor(310, 125);
  tft.print(EEPROMData.versionSettings);
  tft.setFontScale(1);
  tft.setTextColor(RA8875_YELLOW);
  tft.setCursor(380, 175);
  tft.print("By");
  tft.setCursor(270, 205);
  tft.setTextColor(RA8875_WHITE);
  tft.print("Greg Raven KF5N");
  tft.setCursor(170, 260);
  tft.setTextColor(RA8875_YELLOW);
  tft.print("Based on the T41-EP code by");
  tft.setFontScale(1);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(100, 300);  // 38 = letters in string
  tft.print("Al Peter, AC8GY     Jack Purdum, W8TEE");
  tft.setCursor((XPIXELS / 2) - (12 * tft.getFontWidth()) / 2, YPIXELS / 2 + 110);  // 12 = letters in "Property of:"
  tft.print("Property of:");
  tft.setFontScale(2);
  tft.setTextColor(RA8875_GREEN);
  centerCall = (XPIXELS - strlen(MY_CALL) * tft.getFontWidth()) / 2;
  tft.setCursor(centerCall, YPIXELS / 2 + 160);
  tft.print(MY_CALL);
  delay(SPLASH_DELAY);
  tft.fillWindow(RA8875_BLACK);
}


MenuSelect readButton(MenuSelect lastUsedTask) {
  int val = 0;
  MenuSelect menu, task = MenuSelect::DEFAULT;
    val = ReadSelectedPushButton();
    if (val != -1) {  // -1 is returned by ReadSelectedPushButton in the case of an invalid read.
      menu = ProcessButtonPress(val);
      if (menu != lastUsedTask && task == MenuSelect::DEFAULT) task = menu;
      else task = MenuSelect::BOGUS_PIN_READ;
    }
    return task;
}


MenuSelect readButton() {
  int val = 0;
  MenuSelect menu = MenuSelect::DEFAULT;
    val = ReadSelectedPushButton();
    if (val != -1) {  // -1 is returned by ReadSelectedPushButton in the case of an invalid read.
      menu = ProcessButtonPress(val);
    }  else menu = MenuSelect::BOGUS_PIN_READ;
    return menu;
}


/*****
  Purpose: program entry point that sets the environment for program

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void setup() {
  Serial.begin(115200);  // Increased to modern speed.  KF5N March 24, 2024.

  setSyncProvider(getTeensy3Time);  // get TIME from real time clock with 3V backup battery
  setTime(now());
  Teensy3Clock.set(now());  // set the RTC
  T4_rtc_set(Teensy3Clock.get());

  sgtl5000_1.setAddress(LOW);  // This is not documented.  See QuadChannelOutput example.
  sgtl5000_1.enable();
  sgtl5000_1.audioPreProcessorEnable();  // Need to use one of the equalizers.
  sgtl5000_1.eqSelect(3);
  sgtl5000_1.eqBands(-1.0, 0.0, 1.0, 1.0, -1.0);
  AudioMemory(200);  //  Increased to 450 from 400.  Memory was hitting max.  KF5N August 31, 2023
  AudioMemory_F32(10);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.muteHeadphone();  // KF5N March 11, 2024
  sgtl5000_1.micGain(0);
  sgtl5000_1.lineInLevel(0);
#ifdef QSE2
  sgtl5000_1.lineOutLevel(13);  // Setting of 13 limits line-out level to 3.15 volts p-p (maximum).
#else
  sgtl5000_1.lineOutLevel(20);  // Setting of 20 limits line-out level to 2.14 volts p-p.
#endif
sgtl5000_1.adcHighPassFilterEnable();  
//sgtl5000_1.adcHighPassFilterDisable();  //reduces noise.  https://forum.pjrc.com/threads/27215-24-bit-audio-boards?p=78831&viewfull=1#post78831
//sgtl5000_1.adcHighPassFilterFreeze();
  sgtl5000_2.setAddress(HIGH);            // T41 has only a single Audio Adaptor.  This is being used essentially as a 2nd I2S port.
  sgtl5000_2.enable();
  sgtl5000_2.inputSelect(AUDIO_INPUT_LINEIN);  // Why is a second sgtl5000 device used???  This is the receiver ADCs, PCM1808?
  sgtl5000_2.muteHeadphone();                  // KF5N March 11, 2024
                                               //  sgtl5000_2.volume(0.5);   //  Headphone volume???  Not required as headphone is muted.
  updateMic();  // This updates the transmit signal chain settings.

// Set up "Controlled Envelope Single Side Band" from the Open Audio Library.
   cessb1.setSampleRate_Hz(48000);
   cessb1.setGains(3.5f, 1.4f, 0.5f);  // gainIn, gainCompensate, gainOut
   cessb1.setSideband(false);
   cessb1.setProcessing(EEPROMData.cessb);  // Set to CESSB or SSB Data.  Greg KF5N August 17 2024

  Q_out_L_Ex.setMaxBuffers(32);       // Limits determined emperically.  These may need more adjustment.  Greg KF5N August 4, 2024.
  Q_out_R_Ex.setMaxBuffers(32);
  Q_out_L.setMaxBuffers(64);          // Receiver audio buffer limit.

  // GPOs used to control hardware.
  pinMode(FILTERPIN15M, OUTPUT);
  pinMode(FILTERPIN20M, OUTPUT);
  pinMode(FILTERPIN40M, OUTPUT);
  pinMode(FILTERPIN80M, OUTPUT);
  pinMode(RXTX, OUTPUT);
  pinMode(MUTE, OUTPUT);
  digitalWrite(MUTE, MUTEAUDIO);
  pinMode(PTT, INPUT_PULLUP);
  pinMode(BUSY_ANALOG_PIN, INPUT);  // Pin 39.  Switch matrix output connects to this pin.
  pinMode(KEYER_DIT_INPUT_TIP, INPUT_PULLUP);
  pinMode(KEYER_DAH_INPUT_RING, INPUT_PULLUP);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) {
    pinMode(RESET, OUTPUT);  // Reset pulse to QSD2/QSE2.
    digitalWrite(RESET, HIGH);
  }

  // SPI bus to display.
  pinMode(TFT_MOSI, OUTPUT);
  digitalWrite(TFT_MOSI, HIGH);
  pinMode(TFT_SCLK, OUTPUT);
  digitalWrite(TFT_SCLK, HIGH);
  pinMode(TFT_CS, OUTPUT);  // The Main board has a pull-up resistor on an output???
  digitalWrite(TFT_CS, HIGH);

  // Rotary encoders.
  tuneEncoder.begin(true);
  volumeEncoder.begin(true);
  attachInterrupt(digitalPinToInterrupt(VOLUME_ENCODER_A), EncoderVolume, CHANGE);
  attachInterrupt(digitalPinToInterrupt(VOLUME_ENCODER_B), EncoderVolume, CHANGE);
  filterEncoder.begin(true);
  attachInterrupt(digitalPinToInterrupt(FILTER_ENCODER_A), EncoderFilter, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FILTER_ENCODER_B), EncoderFilter, CHANGE);
  fineTuneEncoder.begin(true);
  attachInterrupt(digitalPinToInterrupt(FINETUNE_ENCODER_A), EncoderFineTune, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FINETUNE_ENCODER_B), EncoderFineTune, CHANGE);
  attachInterrupt(digitalPinToInterrupt(KEYER_DIT_INPUT_TIP), KeyTipOn, CHANGE);  // Changed to keyTipOn from KeyOn everywhere JJP 8/31/22
  attachInterrupt(digitalPinToInterrupt(KEYER_DAH_INPUT_RING), KeyRingOn, CHANGE);

// Move these to the initialization function?
  arm_fir_init_f32(&FIR_CW_DecodeL, 64, CW_Filter_Coeffs2, FIR_CW_DecodeL_state, 256);  //AFP 10-25-22
  arm_fir_init_f32(&FIR_CW_DecodeR, 64, CW_Filter_Coeffs2, FIR_CW_DecodeR_state, 256);
  arm_fir_interpolate_init_f32(&FIR_int1_EX_I, 2, 48, coeffs48K_8K_LPF_FIR, FIR_int1_EX_I_state, 256);  // Used in CW exciter.
  arm_fir_interpolate_init_f32(&FIR_int1_EX_Q, 2, 48, coeffs48K_8K_LPF_FIR, FIR_int1_EX_Q_state, 256);
  arm_fir_interpolate_init_f32(&FIR_int2_EX_I, 4, 32, coeffs192K_10K_LPF_FIR, FIR_int2_EX_I_state, 512);
  arm_fir_interpolate_init_f32(&FIR_int2_EX_Q, 4, 32, coeffs192K_10K_LPF_FIR, FIR_int2_EX_Q_state, 512);

  //***********************  Display interface settings ************
  uint32_t iospeed_display = IOMUXC_PAD_DSE(3) | IOMUXC_PAD_SPEED(1);
  *(digital_pin_to_info_PGM + 13)->pad = iospeed_display;  //clk
  *(digital_pin_to_info_PGM + 11)->pad = iospeed_display;  //MOSI
  *(digital_pin_to_info_PGM + TFT_CS)->pad = iospeed_display;

  tft.begin(RA8875_800x480, 8, 20000000UL, 4000000UL);  // parameter list from library code
  tft.setRotation(0);

  // Setup for scrolling attributes. Part of initSpectrum_RA8875() call written by Mike Lewis
  tft.useLayers(true);  //mainly used to turn on layers! //AFP 03-27-22 Layers
  tft.layerEffect(OR);
  tft.clearMemory();
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);

  Splash();

  // =============== EEPROM section =================
  EEPROMData.sdCardPresent = InitializeSDCard();  // Initialize mandatory SD card.
  // Push and hold a button at power up to activate switch matrix calibration.
#ifdef DEBUG_SWITCH_CAL
  if (analogRead(BUSY_ANALOG_PIN) < NOTHING_TO_SEE_HERE) {
    tft.fillWindow(RA8875_BLACK);
    tft.setFontScale(1);
    tft.setTextColor(RA8875_GREEN);
    tft.setCursor(10, 10);
    tft.print("Release button to start calibration.");
    delay(2000);
    EnableButtonInterrupts();
    SaveAnalogSwitchValues();
    eeprom.EEPROMWrite();  // Call to reset switch matrix values
  }                // KD0RC end
#else
  EnableButtonInterrupts();
  eeprom.EEPROMStartup();
#endif

  // ========================  End set up of Parameters from EEPROM data ===============
  Q_out_L_Ex.setMaxBuffers(32);  // Put a constraint on the number of blockss to decrease audio memory.  Greg KF5N August 15, 2024
  Q_out_R_Ex.setMaxBuffers(32);
  h = 135;
  NCOFreq = 0;
  h = 135;

  /****************************************************************************************
     start local oscillator Si5351
  ****************************************************************************************/
  si5351.reset();                                                                           // KF5N.  Moved Si5351 start-up to setup. JJP  7/14/23
  si5351.init(SI5351_CRYSTAL_LOAD_10PF, Si_5351_crystal, EEPROMData.freqCorrectionFactor);  // JJP  7/14/23
  si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);                                           // Allows CLK1 and CLK2 to exceed 100 MHz simultaneously.
  #ifdef PLLMODULE
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);                                     // AFP 10-13-22
  #else
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  #endif
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);                                     // CWP AFP 10-13-22
  // Turn off LOs.  Should be default state after init, so probably not really necessary here.
  si5351.output_enable(SI5351_CLK2, 0);
  si5351.output_enable(SI5351_CLK1, 0);

  InitializeDataArrays();
  // Initialize user defined stuff
  initUserDefinedStuff();
  volumeAdjust.gain(0.0);   // Set volume to zero at power-up.
  volumeChangeFlag = true;  // Adjust volume so saved value.
  filterEncoderMove = 0;
  fineTuneEncoderMove = 0L;
  UpdateInfoWindow();
  DrawSpectrumDisplayContainer();
  RedrawDisplayScreen();

  mainMenuIndex = 0;  // Changed from middle to first. Do Menu Down to get to Calibrate quickly
  ShowName();

  ShowBandwidth();
  FilterBandwidth();
  ShowFrequency();
  ShowAutoStatus();
  zoomIndex = EEPROMData.spectrum_zoom - 1;  // ButtonZoom() increments zoomIndex, so this cancels it so the read from EEPROM is accurately restored.  KF5N August 3, 2023
  ButtonZoom();                              // Restore zoom settings.  KF5N August 3, 2023

  EEPROMData.sdCardPresent = SDPresentCheck();  // JJP 7/18/23
  lastState = RadioState::NOSTATE;                             // To make sure the receiver will be configured on the first pass through.  KF5N September 3, 2023
  if(EEPROMData.xmtMode == RadioMode::CW_MODE) {radioState = RadioState::CW_RECEIVE_STATE; radioMode = RadioMode::CW_MODE;}
  if(EEPROMData.xmtMode == RadioMode::SSB_MODE) {radioState = RadioState::SSB_RECEIVE_STATE; radioMode = RadioMode::SSB_MODE;}
  UpdateDecoderField();                         // Adjust graphics for Morse decoder.
  FilterSetSSB();
  UpdateEqualizerField(EEPROMData.receiveEQFlag);
  EEPROMData.rfGainCurrent = 0;  // Start with lower gain so you don't get blasted.
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();  // Required only for QSD2/QSE2.
}
//============================================================== END setup() =================================================================

elapsedMicros usec = 0;  // Automatically increases as time passes; no ++ necessary.

/*****
  Purpose: Code here executes forever, or until: 1) power is removed, 2) user does a reset, 3) a component
           fails, or 4) the cows come home.

  Parameter list:
    void

  Return value:
    void
*****/
float audioBW{0.0};

void loop()  // Replaced entire loop() with Greg's code  JJP  7/14/23
{
  MenuSelect menu;
  long ditTimerOff;  //AFP 09-22-22
  long dahTimerOn;
  bool cwKeyDown;
  unsigned long cwBlockIndex;

  menu = readButton();
  if (menu != MenuSelect::BOGUS_PIN_READ and (radioState != RadioState::SSB_TRANSMIT_STATE) and (radioState != RadioState::CW_TRANSMIT_STRAIGHT_STATE) and (radioState != RadioState::CW_TRANSMIT_KEYER_STATE)) {
    ExecuteButtonPress(menu);
  }
  //  State detection
  if (EEPROMData.xmtMode == RadioMode::SSB_MODE and digitalRead(PTT) == HIGH) radioState = RadioState::SSB_RECEIVE_STATE;
  if (EEPROMData.xmtMode == RadioMode::SSB_MODE && digitalRead(PTT) == LOW) radioState = RadioState::SSB_TRANSMIT_STATE;
  if (EEPROMData.xmtMode == RadioMode::CW_MODE && (digitalRead(EEPROMData.paddleDit) == HIGH && digitalRead(EEPROMData.paddleDah) == HIGH)) radioState = RadioState::CW_RECEIVE_STATE;  // Was using symbolic constants. Also changed in code below.  KF5N August 8, 2023
  if (EEPROMData.xmtMode == RadioMode::CW_MODE && (digitalRead(EEPROMData.paddleDit) == LOW && EEPROMData.xmtMode == RadioMode::CW_MODE && EEPROMData.keyType == 0)) radioState = RadioState::CW_TRANSMIT_STRAIGHT_STATE;
  if (EEPROMData.xmtMode == RadioMode::CW_MODE && (keyPressedOn == 1 && EEPROMData.xmtMode == RadioMode::CW_MODE && EEPROMData.keyType == 1)) radioState = RadioState::CW_TRANSMIT_KEYER_STATE;
  if (bands[EEPROMData.currentBand].mode > 1) {
  radioState = RadioState::AM_RECEIVE_STATE;  // Inhibit transmit in AM demod modes.  KF5N March 21, 2024
  radioMode = RadioMode::AM_MODE;             // AM is currently receive only.
  keyPressedOn = 0;
  }

  if (lastState != radioState) {
    SetAudioOperatingState(radioState);
    SetFreq();  // Update frequencies if the radio state has changed.
  }

  //  Begin radio state machines

  //  Begin SSB Mode state machine
  switch (radioState) {
    case RadioState::AM_RECEIVE_STATE:
    case RadioState::SSB_RECEIVE_STATE:
      if (lastState != radioState) {  // G0ORX 01092023
        digitalWrite(MUTE, UNMUTEAUDIO);      // Audio Mute off
        digitalWrite(RXTX, LOW);      //xmit off
        if (keyPressedOn == 1) {
          return;
        }
        ShowTransmitReceiveStatus();
      }
      ShowSpectrum();
      break;
    case RadioState::SSB_TRANSMIT_STATE:
      digitalWrite(MUTE, MUTEAUDIO);  //  Mute Audio  (HIGH=Mute)
      digitalWrite(RXTX, HIGH);  //xmit on

      ShowTransmitReceiveStatus();
      while (digitalRead(PTT) == LOW) {
        ExciterIQData();

#ifdef DEBUG_CESSB
  struct levelsZ* cessbData;
if(cessb1.levelDataCount() > 2000) {
        Serial.print("levelDataCount = ");  // Before getLevels(1), because it resets counts to 0.
        Serial.println(cessb1.levelDataCount());
  cessbData = cessb1.getLevels(1);  // Update the CESSB information struct.  Write the data to serial.
          // Detailed Report
        Serial.print(10.0f*log10f(cessbData->pwr0));
        Serial.print(" In Ave Pwr Out ");
        Serial.println(10.0f*log10f(cessbData->pwr1));
        Serial.print(20.0f*log10f(cessbData->peak0));
        Serial.print(" In  Peak   Out ");
        Serial.println(20.0f*log10f(cessbData->peak1));
        Serial.print(cessbData->peak0, 6);
        Serial.print(" In  Peak Volts   Out ");
        Serial.println(cessbData->peak1, 6);
        Serial.print("Enhancement = ");
        float32_t enhance = (10.0f*log10f(cessbData->pwr1) - 20.0f*log10f(cessbData->peak1)) -
                            (10.0f*log10f(cessbData->pwr0) - 20.0f*log10f(cessbData->peak0));
        Serial.print(enhance); Serial.println(" dB");
}
#endif


      }
      break;

    default:
      break;
  }
  //======================  End SSB Mode =================

  // Begin CW Mode state machine

  switch (radioState) {
    case RadioState::CW_RECEIVE_STATE:
      if (lastState != radioState) {  // G0ORX 01092023
        digitalWrite(MUTE, UNMUTEAUDIO);      //turn off mute
        ShowTransmitReceiveStatus();
        keyPressedOn = 0;
      }
      ShowSpectrum();  // if removed CW signal on is 2 mS
      break;
    case RadioState::CW_TRANSMIT_STRAIGHT_STATE:
      ShowTransmitReceiveStatus();
      digitalWrite(MUTE, UNMUTEAUDIO);  // unmutes audio for sidetone
      cwKeyDown = false;        // false initiates CW_SHAPING_RISE.
      cwTimer = millis();
      while (millis() - cwTimer <= EEPROMData.cwTransmitDelay) {  //Start CW transmit timer on
        digitalWrite(RXTX, HIGH);
        if (digitalRead(EEPROMData.paddleDit) == LOW && EEPROMData.keyType == 0) {  // AFP 09-25-22  Turn on CW signal
          cwTimer = millis();                                                       //Reset timer

          if (!cwKeyDown) {
            CW_ExciterIQData(CW_SHAPING_RISE);
            cwKeyDown = true;
          } else {
            CW_ExciterIQData(CW_SHAPING_NONE);
          }
        } else {
          if (digitalRead(EEPROMData.paddleDit) == HIGH && EEPROMData.keyType == 0) {  //Turn off CW signal
            keyPressedOn = 0;
            if (cwKeyDown) {  // Initiate falling CW signal.
              CW_ExciterIQData(CW_SHAPING_FALL);
              cwKeyDown = false;
            } else CW_ExciterIQData(CW_SHAPING_ZERO);  //  No waveforms; but DC offset is still present.
          }
        }
      }
      digitalWrite(MUTE, MUTEAUDIO);  // mutes audio
      digitalWrite(RXTX, LOW);   // End Straight Key Mode
      break;
    case RadioState::CW_TRANSMIT_KEYER_STATE:
      ShowTransmitReceiveStatus();
      digitalWrite(MUTE, UNMUTEAUDIO);  // unmutes audio for sidetone
      cwTimer = millis();
      while (millis() - cwTimer <= EEPROMData.cwTransmitDelay) {
        digitalWrite(RXTX, HIGH);

        if (digitalRead(EEPROMData.paddleDit) == LOW) {  // Keyer Dit
          ditTimerOn = millis();
          // Queue audio blocks--execution time of this loop will be between 0-20ms shorter
          // than the desired dit time, due to audio buffering.
          CW_ExciterIQData(CW_SHAPING_RISE);
          for (cwBlockIndex = 0; cwBlockIndex < transmitDitUnshapedBlocks; cwBlockIndex++) {
            CW_ExciterIQData(CW_SHAPING_NONE);
          }
          CW_ExciterIQData(CW_SHAPING_FALL);

          // Wait for calculated dit time, allowing audio blocks to be played
          while (millis() - ditTimerOn <= transmitDitLength) {
            ;
          }

          // Pause for one dit length of silence
          ditTimerOff = millis();
          while (millis() - ditTimerOff <= transmitDitLength) {
            CW_ExciterIQData(CW_SHAPING_ZERO);
          }
          cwTimer = millis();
        } else if (digitalRead(EEPROMData.paddleDah) == LOW) {  //Keyer DAH
          dahTimerOn = millis();
          // Queue audio blocks--execution time of this loop will be between 0-20ms shorter
          // than the desired dah time, due to audio buffering
          CW_ExciterIQData(CW_SHAPING_RISE);
          for (cwBlockIndex = 0; cwBlockIndex < transmitDahUnshapedBlocks; cwBlockIndex++) {
            CW_ExciterIQData(CW_SHAPING_NONE);
          }
          CW_ExciterIQData(CW_SHAPING_FALL);

          // Wait for calculated dah time, allowing audio blocks to be played
          while (millis() - dahTimerOn <= 3UL * transmitDitLength) {
            ;
          }

          // Pause for one dit length of silence
          ditTimerOff = millis();
          while (millis() - ditTimerOff <= transmitDitLength) {
            CW_ExciterIQData(CW_SHAPING_ZERO);
          }
          cwTimer = millis();
        } else {
          CW_ExciterIQData(CW_SHAPING_ZERO);
        }
        keyPressedOn = 0;  // Fix for keyer click-clack.  KF5N August 16, 2023
      }                    //End Relay timer

      digitalWrite(MUTE, MUTEAUDIO);  // mutes audio
      digitalWrite(RXTX, LOW);   // End Straight Key Mode
      break;
      case RadioState::NOSTATE:
      break;
    default:
      break;
  }

  //  End radio state machine
  if (lastState != radioState) {  // G0ORX 09012023
    lastState = radioState;
    ShowTransmitReceiveStatus();
  }

#ifdef DEBUG1
  if (elapsed_micros_idx_t > (SR[SampleRate].rate / 960)) {
    ShowTempAndLoad();
    // Used to monitor CPU temp and load factors
  }
#endif

  if (volumeChangeFlag == true) { 
    // Compensate for audio filter setting.
    // Nominal bandwidth is 2.8kHz.  This will be the 0 dB reference.
    // The upper and lower frequency limits are bands[EEPROMData.currentBand].FLoCut and bands[EEPROMData.currentBand].FHiCut.
    audioBW = bands[EEPROMData.currentBand].FHiCut - bands[EEPROMData.currentBand].FLoCut;
    // How many dB between reference and current setting?  Round to integer.
//    dBoffset = static_cast<int>(40.0 * log10f_fast(audioBW/2800.0));
    process.audioGainCompensate = 4 * 2800.0/audioBW;
    volumeAdjust.gain(volumeLog[EEPROMData.audioVolume]);
//    int gainAdjust = EEPROMData.audioVolume - dBoffset;
    volumeChangeFlag = false;
    UpdateVolumeField();
//    Serial.printf("audioBW = %d\n", static_cast<int>(audioBW));
//    Serial.printf("EEPROMData.audioVolume = %d\n", EEPROMData.audioVolume);
  }

}  // end loop()