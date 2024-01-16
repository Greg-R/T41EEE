/*
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

*/

// setup() and loop() at the bottom of this file

#ifndef BEENHERE
#include "SDT.h"
#endif

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

bool save_last_frequency = false;
struct band bands[NUMBER_OF_BANDS] = {  //AFP Changed 1-30-21 // G0ORX Changed AGC to 20
//freq    band low   band hi   name    mode      Low    Hi  Gain  type    gain  AGC   pixel
//                                             filter filter             correct     offset
//DB2OO, 29-AUG-23: take ITU_REGION into account for band limits
// and changed "gainCorrection" to see the correct dBm value on all bands.
// Calibration done with TinySA as signal generator with -73dBm levels (S9) at the FT8 frequencies
// with V010 QSD with the 12V mod of the pre-amp
#if defined(ITU_REGION) && ITU_REGION == 1
  3700000, 3500000, 3800000, "80M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
  7150000, 7000000, 7200000, "40M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
#elif defined(ITU_REGION) && ITU_REGION == 2
  3700000, 3500000, 4000000, "80M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
  7150000, 7000000, 7300000, "40M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
#elif defined(ITU_REGION) && ITU_REGION == 3
  3700000, 3500000, 3900000, "80M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
  7150000, 7000000, 7200000, "40M", DEMOD_LSB, -200, -3000, 1, HAM_BAND, -2.0, 20, 20,
#endif
  14200000, 14000000, 14350000, "20M", DEMOD_USB, 3000, 200, 1, HAM_BAND, 2.0, 20, 20,
  18100000, 18068000, 18168000, "17M", DEMOD_USB, 3000, 200, 1, HAM_BAND, 2.0, 20, 20,
  21200000, 21000000, 21450000, "15M", DEMOD_USB, 3000, 200, 1, HAM_BAND, 5.0, 20, 20,
  24920000, 24890000, 24990000, "12M", DEMOD_USB, 3000, 200, 1, HAM_BAND, 6.0, 20, 20,
  28350000, 28000000, 29700000, "10M", DEMOD_USB, 3000, 200, 1, HAM_BAND, 8.5, 20, 20
};

const char *topMenus[] = { "CW Options", "RF Set", "VFO Select",
                           "EEPROM", "AGC", "Spectrum Options",
                           "Noise Floor", "Mic Gain", "Mic Comp",
                           "EQ Rec Set", "EQ Xmt Set", "Calibrate", "Bearing" };

// Pointers to functions which execute the menu options.  Do these functions used the returned integer???
int (*functionPtr[])() = { &CWOptions, &RFOptions, &VFOSelect,
                           &EEPROMOptions, &AGCOptions, &SpectrumOptions,
                           &ButtonSetNoiseFloor, &MicGainSet, &MicOptions,
                           &EqualizerRecOptions, &EqualizerXmtOptions, &CalibrateOptions, &BearingMaps };
const char *labels[] = { "Select", "Menu Up", "Band Up",
                         "Zoom", "Menu Dn", "Band Dn",
                         "Filter", "DeMod", "Mode",
                         "NR", "Notch", "Noise Floor",
                         "Fine Tune", "Decoder", "Tune Increment",
                         "Reset Tuning", "Frequ Entry", "User 2" };

uint32_t FFT_length = FFT_LENGTH;

extern "C" uint32_t set_arm_clock(uint32_t frequency);

// lowering this from 600MHz to 200MHz makes power consumption less
uint32_t T4_CPU_FREQUENCY = 500000000UL;  //AFP 2-10-21

//======================================== Global object definitions ==================================================
// ===========================  AFP 08-22-22

AudioControlSGTL5000_Extended sgtl5000_1;      //controller for the Teensy Audio Board
AudioConvert_I16toF32 int2Float1, int2Float2;  //Converts Int16 to Float.  See class in AudioStream_F32.h
AudioEffectGain_F32 gain1, gain2;              //Applies digital gain to audio data.  Expected Float data.
AudioEffectCompressor_F32 comp1, comp2;
AudioConvert_F32toI16 float2Int1, float2Int2;  //Converts Float to Int16.  See class in AudioStream_F32.h

AudioInputI2SQuad i2s_quadIn;
AudioOutputI2SQuad i2s_quadOut;

AudioMixer4 recMix_3;  // JJP

AudioMixer4 modeSelectInR;    // AFP 09-01-22
AudioMixer4 modeSelectInL;    // AFP 09-01-22
AudioMixer4 modeSelectInExR;  // AFP 09-01-22
AudioMixer4 modeSelectInExL;  // AFP 09-01-22

AudioMixer4 modeSelectOutL;    // AFP 09-01-22
AudioMixer4 modeSelectOutR;    // AFP 09-01-22
AudioMixer4 modeSelectOutExL;  // AFP 09-01-22
AudioMixer4 modeSelectOutExR;  // AFP 09-01-22

//=============== // AFP 09-01-22
//AudioMixer4           CW_AudioOutR; //AFP 09-01-22
//AudioMixer4           CW_AudioOutL; //AFP 09-01-22

AudioMixer4 CW_AudioOut;

AudioRecordQueue Q_in_L;
AudioRecordQueue Q_in_R;
AudioRecordQueue Q_in_L_Ex;
AudioRecordQueue Q_in_R_Ex;

AudioPlayQueue Q_out_L;
AudioPlayQueue Q_out_R;
AudioPlayQueue Q_out_L_Ex;
AudioPlayQueue Q_out_R_Ex;

// ===============
AudioConnection patchCord1(i2s_quadIn, 0, int2Float1, 0);  //connect the Left input to the Left Int->Float converter
AudioConnection patchCord2(i2s_quadIn, 1, int2Float2, 0);  //connect the Right input to the Right Int->Float converter

AudioConnection_F32 patchCord3(int2Float1, 0, comp1, 0);  //Left.  makes Float connections between objects
AudioConnection_F32 patchCord4(int2Float2, 0, comp2, 0);  //Right.  makes Float connections between objects
AudioConnection_F32 patchCord5(comp1, 0, float2Int1, 0);  //Left.  makes Float connections between objects
AudioConnection_F32 patchCord6(comp2, 0, float2Int2, 0);  //Right.  makes Float connections between objects
//AudioConnection_F32     patchCord3(int2Float1, 0, float2Int1, 0); //Left.  makes Float connections between objects
//AudioConnection_F32     patchCord4(int2Float2, 0, float2Int2, 0); //Right.  makes Float connections between objects

// ===============

AudioConnection patchCord7(float2Int1, 0, modeSelectInExL, 0);  //Input Ex
AudioConnection patchCord8(float2Int2, 0, modeSelectInExR, 0);

AudioConnection patchCord9(i2s_quadIn, 2, modeSelectInL, 0);  //Input Rec
AudioConnection patchCord10(i2s_quadIn, 3, modeSelectInR, 0);

AudioConnection patchCord11(modeSelectInExR, 0, Q_in_R_Ex, 0);  //Ex in Queue
AudioConnection patchCord12(modeSelectInExL, 0, Q_in_L_Ex, 0);

AudioConnection patchCord13(modeSelectInR, 0, Q_in_R, 0);  //Rec in Queue
AudioConnection patchCord14(modeSelectInL, 0, Q_in_L, 0);

AudioConnection patchCord15(Q_out_L_Ex, 0, modeSelectOutExL, 0);  //Ex out Queue
AudioConnection patchCord16(Q_out_R_Ex, 0, modeSelectOutExR, 0);

AudioConnection patchCord17(Q_out_L, 0, modeSelectOutL, 0);  //Rec out Queue
AudioConnection patchCord18(Q_out_R, 0, modeSelectOutR, 0);

AudioConnection patchCord19(modeSelectOutExL, 0, i2s_quadOut, 0);  //Ex out
AudioConnection patchCord20(modeSelectOutExR, 0, i2s_quadOut, 1);
AudioConnection patchCord21(modeSelectOutL, 0, i2s_quadOut, 2);  //Rec out
AudioConnection patchCord22(modeSelectOutR, 0, i2s_quadOut, 3);

AudioConnection patchCord23(Q_out_L_Ex, 0, modeSelectOutL, 1);  //Rec out Queue for sidetone
AudioConnection patchCord24(Q_out_R_Ex, 0, modeSelectOutR, 1);

// ================================== AFP 11-01-22

//AudioControlSGTL5000  sgtl5000_1;
AudioControlSGTL5000 sgtl5000_2;
// ===========================  AFP 08-22-22 end

Rotary volumeEncoder = Rotary(VOLUME_ENCODER_A, VOLUME_ENCODER_B);        //( 2,  3)
Rotary tuneEncoder = Rotary(TUNE_ENCODER_A, TUNE_ENCODER_B);              //(16, 17)
Rotary filterEncoder = Rotary(FILTER_ENCODER_A, FILTER_ENCODER_B);        //(15, 14)
Rotary fineTuneEncoder = Rotary(FINETUNE_ENCODER_A, FINETUNE_ENCODER_B);  //( 4,  5)

Metro ms_500 = Metro(500);  // Set up a Metro
Metro ms_300000 = Metro(300000);
Metro encoder_check = Metro(100);  // Set up a Metro

Si5351 si5351;

int radioState, lastState;  // KF5N
int resetTuningFlag = 0;
#ifndef RA8875_DISPLAY
ILI9488_t3 tft = ILI9488_t3(&SPI, TFT_CS, TFT_DC, TFT_RST);
#else
#define RA8875_CS TFT_CS
#define RA8875_RESET TFT_DC  // any pin or nothing!
RA8875 tft = RA8875(RA8875_CS, RA8875_RESET);
#endif

SPISettings settingsA(70000000UL, MSBFIRST, SPI_MODE1);

const uint32_t N_B_EX = 16;
//================== Receive EQ Variables================= AFP 08-08-22
//float32_t recEQ_Level[14];
//float32_t recEQ_LevelScale[14];
//Setup for EQ filters
float32_t DMAMEM rec_EQ1_float_buffer_L[256];
float32_t DMAMEM rec_EQ2_float_buffer_L[256];
float32_t DMAMEM rec_EQ3_float_buffer_L[256];
float32_t DMAMEM rec_EQ4_float_buffer_L[256];
float32_t DMAMEM rec_EQ5_float_buffer_L[256];
float32_t DMAMEM rec_EQ6_float_buffer_L[256];
float32_t DMAMEM rec_EQ7_float_buffer_L[256];
float32_t DMAMEM rec_EQ8_float_buffer_L[256];
float32_t DMAMEM rec_EQ9_float_buffer_L[256];
float32_t DMAMEM rec_EQ10_float_buffer_L[256];
float32_t DMAMEM rec_EQ11_float_buffer_L[256];
float32_t DMAMEM rec_EQ12_float_buffer_L[256];
float32_t DMAMEM rec_EQ13_float_buffer_L[256];
float32_t DMAMEM rec_EQ14_float_buffer_L[256];

float32_t rec_EQ_Band1_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t rec_EQ_Band2_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band3_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band4_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band5_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band6_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band7_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band8_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t rec_EQ_Band9_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band10_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band11_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band12_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band13_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band14_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//EQ filter instances
arm_biquad_cascade_df2T_instance_f32 S1_Rec = { IIR_NUMSTAGES, rec_EQ_Band1_state, EQ_Band1Coeffs };
arm_biquad_cascade_df2T_instance_f32 S2_Rec = { IIR_NUMSTAGES, rec_EQ_Band2_state, EQ_Band2Coeffs };
arm_biquad_cascade_df2T_instance_f32 S3_Rec = { IIR_NUMSTAGES, rec_EQ_Band3_state, EQ_Band3Coeffs };
arm_biquad_cascade_df2T_instance_f32 S4_Rec = { IIR_NUMSTAGES, rec_EQ_Band4_state, EQ_Band4Coeffs };
arm_biquad_cascade_df2T_instance_f32 S5_Rec = { IIR_NUMSTAGES, rec_EQ_Band5_state, EQ_Band5Coeffs };
arm_biquad_cascade_df2T_instance_f32 S6_Rec = { IIR_NUMSTAGES, rec_EQ_Band6_state, EQ_Band6Coeffs };
arm_biquad_cascade_df2T_instance_f32 S7_Rec = { IIR_NUMSTAGES, rec_EQ_Band7_state, EQ_Band7Coeffs };
arm_biquad_cascade_df2T_instance_f32 S8_Rec = { IIR_NUMSTAGES, rec_EQ_Band8_state, EQ_Band8Coeffs };
arm_biquad_cascade_df2T_instance_f32 S9_Rec = { IIR_NUMSTAGES, rec_EQ_Band9_state, EQ_Band9Coeffs };
arm_biquad_cascade_df2T_instance_f32 S10_Rec = { IIR_NUMSTAGES, rec_EQ_Band10_state, EQ_Band10Coeffs };
arm_biquad_cascade_df2T_instance_f32 S11_Rec = { IIR_NUMSTAGES, rec_EQ_Band11_state, EQ_Band11Coeffs };
arm_biquad_cascade_df2T_instance_f32 S12_Rec = { IIR_NUMSTAGES, rec_EQ_Band12_state, EQ_Band12Coeffs };
arm_biquad_cascade_df2T_instance_f32 S13_Rec = { IIR_NUMSTAGES, rec_EQ_Band13_state, EQ_Band13Coeffs };
arm_biquad_cascade_df2T_instance_f32 S14_Rec = { IIR_NUMSTAGES, rec_EQ_Band14_state, EQ_Band14Coeffs };

// ===============================  AFP 10-02-22 ================

//EQBuffers
float32_t DMAMEM xmt_EQ1_float_buffer_L[256];
float32_t DMAMEM xmt_EQ2_float_buffer_L[256];
float32_t DMAMEM xmt_EQ3_float_buffer_L[256];
float32_t DMAMEM xmt_EQ4_float_buffer_L[256];
float32_t DMAMEM xmt_EQ5_float_buffer_L[256];
float32_t DMAMEM xmt_EQ6_float_buffer_L[256];
float32_t DMAMEM xmt_EQ7_float_buffer_L[256];
float32_t DMAMEM xmt_EQ8_float_buffer_L[256];
float32_t DMAMEM xmt_EQ9_float_buffer_L[256];
float32_t DMAMEM xmt_EQ10_float_buffer_L[256];
float32_t DMAMEM xmt_EQ11_float_buffer_L[256];
float32_t DMAMEM xmt_EQ12_float_buffer_L[256];
float32_t DMAMEM xmt_EQ13_float_buffer_L[256];
float32_t DMAMEM xmt_EQ14_float_buffer_L[256];

float32_t xmt_EQ_Band1_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t xmt_EQ_Band2_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band3_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band4_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band5_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band6_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band7_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band8_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band9_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band10_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band11_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band12_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band13_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band14_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//EQ filter instances
arm_biquad_cascade_df2T_instance_f32 S1_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band1_state, EQ_Band1Coeffs };
arm_biquad_cascade_df2T_instance_f32 S2_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band2_state, EQ_Band2Coeffs };
arm_biquad_cascade_df2T_instance_f32 S3_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band3_state, EQ_Band3Coeffs };
arm_biquad_cascade_df2T_instance_f32 S4_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band4_state, EQ_Band4Coeffs };
arm_biquad_cascade_df2T_instance_f32 S5_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band5_state, EQ_Band5Coeffs };
arm_biquad_cascade_df2T_instance_f32 S6_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band6_state, EQ_Band6Coeffs };
arm_biquad_cascade_df2T_instance_f32 S7_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band7_state, EQ_Band7Coeffs };
arm_biquad_cascade_df2T_instance_f32 S8_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band8_state, EQ_Band8Coeffs };
arm_biquad_cascade_df2T_instance_f32 S9_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band9_state, EQ_Band9Coeffs };
arm_biquad_cascade_df2T_instance_f32 S10_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band10_state, EQ_Band10Coeffs };
arm_biquad_cascade_df2T_instance_f32 S11_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band11_state, EQ_Band11Coeffs };
arm_biquad_cascade_df2T_instance_f32 S12_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band12_state, EQ_Band12Coeffs };
arm_biquad_cascade_df2T_instance_f32 S13_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band13_state, EQ_Band13Coeffs };
arm_biquad_cascade_df2T_instance_f32 S14_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band14_state, EQ_Band14Coeffs };
// ===============================End xmit EQ filter setup  AFP 10-02-22 ================

// HP BiQuad IIR DC filter
float32_t HP_DC_Butter_state[6] = { 0, 0, 0, 0, 0, 0 };
float32_t HP_DC_Butter_state2[2] = { 0, 0 };                                                          // AFP 11-04-11
arm_biquad_cascade_df2T_instance_f32 s1_Receive = { 3, HP_DC_Butter_state, HP_DC_Filter_Coeffs };     //AFP 09-23-22
arm_biquad_cascade_df2T_instance_f32 s1_Receive2 = { 1, HP_DC_Butter_state2, HP_DC_Filter_Coeffs2 };  //AFP 11-04-22
//Hilbert FIR Filters
float32_t FIR_Hilbert_state_L[100 + 256 - 1];
float32_t FIR_Hilbert_state_R[100 + 256 - 1];
arm_fir_instance_f32 FIR_Hilbert_L;
arm_fir_instance_f32 FIR_Hilbert_R;

// CW decode Filters
arm_fir_instance_f32 FIR_CW_DecodeL;  //AFP 10-25-22
arm_fir_instance_f32 FIR_CW_DecodeR;  //AFP 10-25-22
float32_t FIR_CW_DecodeL_state[64 + 256 - 1];
float32_t FIR_CW_DecodeR_state[64 + 256 - 1];

//Decimation and Interpolation Filters
arm_fir_decimate_instance_f32 FIR_dec1_EX_I;
arm_fir_decimate_instance_f32 FIR_dec1_EX_Q;
arm_fir_decimate_instance_f32 FIR_dec2_EX_I;
arm_fir_decimate_instance_f32 FIR_dec2_EX_Q;

arm_fir_interpolate_instance_f32 FIR_int1_EX_I;
arm_fir_interpolate_instance_f32 FIR_int1_EX_Q;
arm_fir_interpolate_instance_f32 FIR_int2_EX_I;
arm_fir_interpolate_instance_f32 FIR_int2_EX_Q;

float32_t DMAMEM FIR_dec1_EX_I_state[2095];
float32_t DMAMEM FIR_dec1_EX_Q_state[2095];

float32_t audioMaxSquaredAve;

float32_t DMAMEM FIR_dec2_EX_I_state[535];
float32_t DMAMEM FIR_dec2_EX_Q_state[535];

float32_t DMAMEM FIR_int2_EX_I_state[519];
float32_t DMAMEM FIR_int2_EX_Q_state[519];
float32_t DMAMEM FIR_int1_EX_coeffs[48];
float32_t DMAMEM FIR_int2_EX_coeffs[48];

float32_t DMAMEM FIR_int1_EX_I_state[279];
float32_t DMAMEM FIR_int1_EX_Q_state[279];

float32_t DMAMEM float_buffer_L_EX[2048];
float32_t DMAMEM float_buffer_R_EX[2048];
float32_t DMAMEM float_buffer_LTemp[2048];
float32_t DMAMEM float_buffer_RTemp[2048];
//==================== End Excite Variables================================

//======================================== Global structure declarations ===============================================
//=== CW Filter ===
float32_t CW_Filter_state[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t CW_AudioFilter1_state[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // AFP 10-18-22
float32_t CW_AudioFilter2_state[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // AFP 10-18-22
float32_t CW_AudioFilter3_state[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // AFP 10-18-22
float32_t CW_AudioFilter4_state[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // AFP 10-18-22
float32_t CW_AudioFilter5_state[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // AFP 10-18-22
//---------  Code Filter instance -----
arm_biquad_cascade_df2T_instance_f32 S1_CW_Filter = { IIR_CW_NUMSTAGES, CW_Filter_state, CW_Filter_Coeffs };
arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter1 = { 6, CW_AudioFilter1_state, CW_AudioFilterCoeffs1 };  // AFP 10-18-22
arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter2 = { 6, CW_AudioFilter2_state, CW_AudioFilterCoeffs2 };  // AFP 10-18-22
arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter3 = { 6, CW_AudioFilter3_state, CW_AudioFilterCoeffs3 };  // AFP 10-18-22
arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter4 = { 6, CW_AudioFilter4_state, CW_AudioFilterCoeffs4 };  // AFP 10-18-22
arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter5 = { 6, CW_AudioFilter5_state, CW_AudioFilterCoeffs5 };  // AFP 10-18-22
//=== end CW Filter ===

struct config_t EEPROMData;

const struct SR_Descriptor SR[18] = {
  // x_factor, x_offset and f1 to f4 are NOT USED ANYMORE !!!
  //   SR_n ,            rate,  text,   f1,   f2,   f3,   f4, x_factor = pixels per f1 kHz in spectrum display, x_offset
  { SAMPLE_RATE_8K, 8000, "  8k", " 1", " 2", " 3", " 4", 64.00, 11 },       // not OK
  { SAMPLE_RATE_11K, 11025, " 11k", " 1", " 2", " 3", " 4", 43.10, 17 },     // not OK
  { SAMPLE_RATE_16K, 16000, " 16k", " 4", " 4", " 8", "12", 64.00, 1 },      // OK
  { SAMPLE_RATE_22K, 22050, " 22k", " 5", " 5", "10", "15", 58.05, 6 },      // OK
  { SAMPLE_RATE_32K, 32000, " 32k", " 5", " 5", "10", "15", 40.00, 24 },     // OK, one more indicator?
  { SAMPLE_RATE_44K, 44100, " 44k", "10", "10", "20", "30", 58.05, 6 },      // OK
  { SAMPLE_RATE_48K, 48000, " 48k", "10", "10", "20", "30", 53.33, 11 },     // OK
  { SAMPLE_RATE_50K, 50223, " 50k", "10", "10", "20", "30", 53.33, 11 },     // NOT OK
  { SAMPLE_RATE_88K, 88200, " 88k", "20", "20", "40", "60", 58.05, 6 },      // OK
  { SAMPLE_RATE_96K, 96000, " 96k", "20", "20", "40", "60", 53.33, 12 },     // OK
  { SAMPLE_RATE_100K, 100000, "100k", "20", "20", "40", "60", 53.33, 12 },   // NOT OK
  { SAMPLE_RATE_101K, 100466, "101k", "20", "20", "40", "60", 53.33, 12 },   // NOT OK
  { SAMPLE_RATE_176K, 176400, "176k", "40", "40", "80", "120", 58.05, 6 },   // OK
  { SAMPLE_RATE_192K, 192000, "192k", "40", "40", "80", "120", 53.33, 12 },  // OK    THIS IS USED IN THE T41
  { SAMPLE_RATE_234K, 234375, "234k", "40", "40", "80", "120", 53.33, 12 },  // NOT OK
  { SAMPLE_RATE_256K, 256000, "256k", "40", "40", "80", "120", 53.33, 12 },  // NOT OK
  { SAMPLE_RATE_281K, 281000, "281k", "40", "40", "80", "120", 53.33, 12 },  // NOT OK
  { SAMPLE_RATE_353K, 352800, "353k", "40", "40", "80", "120", 53.33, 12 }   // NOT OK
};

const arm_cfft_instance_f32 *S;
const arm_cfft_instance_f32 *iS;
const arm_cfft_instance_f32 *maskS;
const arm_cfft_instance_f32 *NR_FFT;
const arm_cfft_instance_f32 *NR_iFFT;
const arm_cfft_instance_f32 *spec_FFT;

arm_biquad_casd_df1_inst_f32 biquad_lowpass1;
arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I;
arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q;

arm_fir_decimate_instance_f32 FIR_dec1_I;
arm_fir_decimate_instance_f32 FIR_dec1_Q;
arm_fir_decimate_instance_f32 FIR_dec2_I;
arm_fir_decimate_instance_f32 FIR_dec2_Q;
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I;
arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q;
arm_fir_interpolate_instance_f32 FIR_int1_I;
arm_fir_interpolate_instance_f32 FIR_int1_Q;
arm_fir_interpolate_instance_f32 FIR_int2_I;
arm_fir_interpolate_instance_f32 FIR_int2_Q;
arm_lms_norm_instance_f32 LMS_Norm_instance;
arm_lms_instance_f32 LMS_instance;

const DEMOD_Descriptor DEMOD[3] = {
  //   DEMOD_n, name
  { DEMOD_USB, "(USB)" },
  { DEMOD_LSB, "(LSB)" },
  { DEMOD_AM, "(AM)" },  //AFP09-22-22

};

dispSc displayScale[] =  //r *dbText,dBScale, pixelsPerDB, baseOffset, offsetIncrement
  {
    { "20 dB/", 10.0, 2, 24, 1.00 },
    { "10 dB/", 20.0, 4, 10, 0.50 },  //  JJP 7/14/23
    { "5 dB/", 40.0, 8, 58, 0.25 },
    { "2 dB/", 100.0, 20, 120, 0.10 },
    { "1 dB/", 200.0, 40, 200, 0.05 }
  };

//======================================== Global variables declarations for Quad Oscillator 2 ===============================================
long NCOFreq;

float32_t NCO_INC;

double OSC_COS;
double OSC_SIN;
double Osc_Vect_Q = 1.0;
double Osc_Vect_I = 0.0;
double Osc_Gain = 0.0;
double Osc_Q = 0.0;
double Osc_I = 0.0;
float32_t i_temp = 0.0;
float32_t q_temp = 0.0;

//======================================== Global variables declarations ===============================================
//=============================== Any variable initialized to zero is done for documentation ===========================
//=============================== purposes since the compiler does that for globals by default =========================
//================== Global CW Correlation and FFT Variables =================
float32_t corrResult;
uint32_t corrResultIndex;
float32_t cosBuffer2[256];
float32_t sinBuffer[256];
float32_t sinBuffer2[256];
float32_t cwRiseBuffer[256];
float32_t cwFallBuffer[256];
float32_t aveCorrResult;
float32_t aveCorrResultR;
float32_t aveCorrResultL;
float32_t magFFTResults[256];
float32_t float_Corr_Buffer[511];
float32_t corrResultR;
uint32_t corrResultIndexR;
float32_t corrResultL;
uint32_t corrResultIndexL;
float32_t combinedCoeff;
float32_t combinedCoeff2;
float32_t combinedCoeff2Old;
int CWCoeffLevelOld = 0.0;
float CWLevelTimer = 0.0;
float CWLevelTimerOld = 0.0;
float ticMarkTimer = 0.0;
float ticMarkTimerOld = 0.0;
// === Compressor patameters AFP 11-01-22
float min_gain_dB = -20.0, max_gain_dB = 40.0;  //set desired gain range
float gain_dB = 0.0;                            //computed desired gain value in dB
boolean use_HP_filter = true;                   //enable the software HP filter to get rid of DC?
float knee_dBFS, comp_ratio, attack_sec, release_sec;
// ===========
float32_t float_Corr_BufferR[511];
float32_t float_Corr_BufferL[511];
long tempSigTime = 0;

int audioTempPrevious = 0;
float sigStart = 0.0;
float sigDuration = 0.0;
float gapStartData = 0.0;
float gapDurationData = 0.0;
float goertzelMagnitude;

int audioValuePrevious = 0;
int CWOnState;

bool gEEPROM_current = false;  //mdrhere does the data in EEPROM match the current structure contents
bool NR_gain_smooth_enable = false;
bool NR_long_tone_reset = true;
bool NR_long_tone_enable = false;
//bool omitOutputFlag                       = false;
bool timeflag = 0;
bool volumeChangeFlag = false;

char letterTable[] = {
  // Morse coding: dit = 0, dah = 1
  0b101,    // A                first 1 is the sentinel marker
  0b11000,  // B
  0b11010,  // C
  0b1100,   // D
  0b10,     // E
  0b10010,  // F
  0b1110,   // G
  0b10000,  // H
  0b100,    // I
  0b10111,  // J
  0b1101,   // K
  0b10100,  // L
  0b111,    // M
  0b110,    // N
  0b1111,   // O
  0b10110,  // P
  0b11101,  // Q
  0b1010,   // R
  0b1000,   // S
  0b11,     // T
  0b1001,   // U
  0b10001,  // V
  0b1011,   // W
  0b11001,  // X
  0b11011,  // Y
  0b11100   // Z
};

char numberTable[] = {
  0b111111,  // 0
  0b101111,  // 1
  0b100111,  // 2
  0b100011,  // 3
  0b100001,  // 4
  0b100000,  // 5
  0b110000,  // 6
  0b111000,  // 7
  0b111100,  // 8
  0b111110   // 9
};

char punctuationTable[] = {
  0b01101011,  // exclamation mark 33
  0b01010010,  // double quote 34
  0b10001001,  // dollar sign 36
  0b00101000,  // ampersand 38
  0b01011110,  // apostrophe 39
  0b01011110,  // parentheses (L) 40, 41
  0b01110011,  // comma 44
  0b00100001,  // hyphen 45
  0b01010101,  // period  46
  0b00110010,  // slash 47
  0b01111000,  // colon 58
  0b01101010,  // semi-colon 59
  0b01001100,  // question mark 63
  0b01001101,  // underline 95
  0b01101000,  // paragraph
  0b00010001   // break
};
int ASCIIForPunctuation[] = { 33, 34, 36, 39, 41, 44, 45, 46, 47, 58, 59, 63, 95 };  // Indexes into code

long startTime = 0;

char theversion[10];
char decodeBuffer[33];    // The buffer for holding the decoded characters.  Increased to 33.  KF5N October 29, 2023
char keyboardBuffer[10];  // Set for call prefixes. May be increased later
const char DEGREE_SYMBOL[] = { 0xB0, '\0' };

const char *tune_text = "Fast Tune";
const char *zoomOptions[] = { "1x ", "2x ", "4x ", "8x ", "16x" };

byte currentDashJump = DECODER_BUFFER_SIZE;
byte currentDecoderIndex = 0;

int8_t auto_IQ_correction;
int filterWidthX;  // The current filter X.
int filterWidthY;  // The current filter Y.
float32_t pixel_per_khz = ((1 << EEPROMData.spectrum_zoom) * SPECTRUM_RES * 1000.0 / SR[SampleRate].rate);
int pos_left = centerLine - (int)(bands[EEPROMData.currentBand].FLoCut / 1000.0 * pixel_per_khz);
int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2;
int fLoCutOld;
int fHiCutOld;
int filterWidth = (int)((bands[EEPROMData.currentBand].FHiCut - bands[EEPROMData.currentBand].FLoCut) / 1000.0 * pixel_per_khz);
int h = SPECTRUM_HEIGHT + 3;
int8_t first_block = 1;

int8_t Menu2 = MENU_F_LO_CUT;
int8_t mesz = -1;
int8_t menuStatus = NO_MENUS_ACTIVE;
int8_t mesz_old = 0;
int8_t NB_taps = 10;
int8_t NB_impulse_samples = 7;
int8_t NR_first_block = 1;
int8_t pos_x_date = 14;
int8_t pos_y_date = 68;

uint8_t agc_action = 0;
uint8_t agc_switch_mode = 0;
uint8_t ANR_on = 0;
uint8_t ANR_notch = 0;
uint8_t ANR_notchOn = 0;
uint8_t atan2_approx = 1;
uint8_t auto_codec_gain = 1;
uint8_t audio_flag = 1;
uint8_t bitnumber = 16;  // test, how restriction to twelve bit alters sound quality
uint8_t codec_restarts = 0;
uint8_t dbm_state = 0;
uint8_t dcfParityBit;
uint8_t decay_type = 0;
uint8_t digimode = 0;
uint8_t digits_old[2][10] = { { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 },
                              { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 } };
uint8_t display_dbm = DISPLAY_S_METER_DBM;
uint8_t display_S_meter_or_spectrum_state = 0;
uint8_t eeprom_saved = 0;
uint8_t eeprom_loaded = 0;
uint8_t erase_flag = 0;
uint8_t FIR_filter_window = 1;
uint8_t flagg = 0;
uint8_t half_clip = 0;
uint8_t hang_enable;
uint8_t hour10_old;
uint8_t hour1_old;
uint8_t IQCalFlag = 0;
uint8_t iFFT_flip = 0;
uint8_t IQ_state = 1;
uint8_t IQ_RecCalFlag = 0;
uint8_t keyPressedOn = 0;
uint8_t relayLatch = 0;
uint8_t LastSampleRate = SAMPLE_RATE_192K;
uint8_t minute10_old;
uint8_t minute1_old;
uint8_t NB_on = 0;
uint8_t NB_test = 0;
uint8_t notchButtonState = 0;
uint8_t notchIndex = 0;
uint8_t notches_on[2] = { 0, 0 };
uint8_t NR_first_time = 1;
uint8_t NR_Kim;
uint8_t NR_LMS = 0;
uint8_t NR_Spect;
uint8_t NR_use_X = 0;
uint8_t NR_VAD_enable = 1;
uint8_t precision_flag = 0;
uint8_t quarter_clip = 0;
uint8_t SampleRate = SAMPLE_RATE_192K;
uint8_t save_energy;
uint8_t sch = 0;
uint8_t second10_old;
uint8_t second1_old;
uint8_t show_spectrum_flag = 1;
uint8_t spectrum_mov_average = 0;
uint8_t state = 0;
uint8_t twinpeaks_tested = 2;  // initial value --> 2 !!
uint8_t T41State = 1;
uint8_t wait_flag;
uint8_t which_menu = 1;
uint8_t write_analog_gain = 0;
uint8_t zoom_display = 1;

const uint8_t NR_L_frames = 3;
const uint8_t NR_N_frames = 15;

uint16_t base_y = SPECTRUM_BOTTOM;

int16_t currentMode;
int16_t pixelCurrent[SPECTRUM_RES];
int16_t pixelnew[SPECTRUM_RES];
int16_t pixelold[SPECTRUM_RES];
//int16_t pixelnew2[MAX_WATERFALL_WIDTH + 1];  //AFP
//int16_t pixelold2[MAX_WATERFALL_WIDTH];
int16_t notch_L[2] = { 156, 180 };
int16_t fineEncoderRead;
int16_t notch_R[2] = { 166, 190 };
int16_t notch_pixel_L[2] = { 1, 2 };
int16_t notch_pixel_R[2] = { 2, 3 };
int16_t offsetPixels;
int16_t pos_x_dbm = pos_x_smeter + 170;
int16_t pos_y_dbm = pos_y_smeter - 10;
int16_t pos_y_db;
int16_t pos_y_frequency = 48;
int16_t pos_x_time = 390;  // 14;
int16_t pos_y_time = 5;    //114;
int16_t s_w = 10;
int16_t *sp_L1;
int16_t *sp_R1;
int16_t *sp_L2;
int16_t *sp_R2;
int16_t spectrum_brightness = 255;
int16_t spectrum_height = 96;
int16_t spectrum_pos_centre_f = 64 * xExpand;
int16_t spectrum_WF_height = 96;
int16_t spectrum_x = SPECTRUM_LEFT_X;
int16_t spectrum_y = SPECTRUM_TOP_Y;

uint16_t adcMaxLevel, dacMaxLevel;
uint16_t barGraphUpdate = 0;
//uint16_t EEPROMData.currentScale = 1;  // 20 dB/division

//===== New histogram stuff ===
int endDitFlag = 0;
volatile int filterEncoderMove = 0;
volatile long fineTuneEncoderMove = 0L;

//int EEPROMData.currentNoiseFloor[NUMBER_OF_BANDS];
int endGapFlag = 0;
//int EEPROMData.freqSeparationChar;
int selectedMapIndex;
int topDitIndex;
int topDitIndexOld;
int topGapIndex;
int topGapIndexOld;

int32_t gapHistogram[HISTOGRAM_ELEMENTS];  // Not used.  KF5N November 12, 2023
int32_t signalHistogram[HISTOGRAM_ELEMENTS];

// This enum is for an experimental Morse decoder change.
enum states decodeStates;

uint32_t histMaxIndexDitOld = 80;  // Defaults for 15wpm
uint32_t histMaxIndexDahOld = 200;

uint32_t histMaxDit;
uint32_t histMaxDah;
uint32_t histMaxIndexDit;
uint32_t histMaxIndexDah;

int atomGapLength;
int atomGapLength2;
int charGapLength;
int charGapLength2;
//int receiveEQFlag;
//int xmitEQFlag;
int centerTuneFlag = 0;
int x1AdjMax = 0;  //AFP 2-6-23
unsigned long cwTimer;
long signalTime;
unsigned long ditTimerOn;
long DahTimer;
long cwTime0;
long cwTime5;
long cwTime6;
long valRef1;
long valRef2;
long gapRef1;
int valFlag = 0;
long signalStartOld = 0;
int valCounter;
long aveDitLength = 80;
long aveDahLength = 200;
float thresholdGeometricMean = 140.0;  // This changes as decoder runs
float thresholdArithmeticMean;
float aveAtomGapLength = 40;
float thresholdGapGeometricMean;
float thresholdGapArithmeticMean;
long CWFreqShift;
//long calFreqShift;
long filter_pos = 0;
long last_filter_pos = 0;
// ============ end new stuff =======


uint16_t notches_BW[2] = { 4, 4 };  // no. of bins --> notch BW = no. of bins * bin_BW
uint16_t temp_check_frequency;
uint16_t uAfter;
uint16_t uB4;
uint16_t xx;

int16_t y_old, y_new, y1_new, y1_old, y_old2;  //A
int16_t y1_old_minus = 0;
int16_t y1_new_minus = 0;

const float32_t DF1 = 4.0;             // decimation factor
const float32_t DF2 = 2.0;             // decimation factor
const float32_t DF = DF1 * DF2;        // decimation factor
const float32_t n_samplerate = 176.0;  // samplerate before decimation

const uint32_t N_B = FFT_LENGTH / 2 / BUFFER_SIZE * (uint32_t)DF;
const uint32_t N_DEC_B = N_B / (uint32_t)DF;
const uint32_t NR_add_counter = 128;

const float32_t n_att = 90.0;        // need here for later def's
const float32_t n_desired_BW = 9.0;  // desired max BW of the filters
const float32_t n_fpass1 = n_desired_BW / n_samplerate;
const float32_t n_fpass2 = n_desired_BW / (n_samplerate / DF1);
const float32_t n_fstop1 = ((n_samplerate / DF1) - n_desired_BW) / n_samplerate;
const float32_t n_fstop2 = ((n_samplerate / (DF1 * DF2)) - n_desired_BW) / (n_samplerate / DF1);

const uint32_t IIR_biquad_Zoom_FFT_N_stages = 4;
const uint32_t N_stages_biquad_lowpass1 = 1;
const uint16_t n_dec1_taps = (1 + (uint16_t)(n_att / (22.0 * (n_fstop1 - n_fpass1))));
const uint16_t n_dec2_taps = (1 + (uint16_t)(n_att / (22.0 * (n_fstop2 - n_fpass2))));

int resultOldFactor;
float incrFactor;
int mute = 0;

float adjustVolEncoder;
int adjustIQ;
int exciteOn = 0;
int agc_decay = 100;
int agc_slope = 100;
int agc_thresh = 30;
int ANR_buff_size = FFT_length / 2.0;
int ANR_delay = 16;
int ANR_dline_size = ANR_DLINE_SIZE;
int ANR_in_idx = 0;
int ANR_mask = ANR_dline_size - 1;
int ANR_position = 0;
int ANR_taps = 64;
int attenuator = 0;
int attack_buffsize;
//int EEPROMData.AGCMode = 30;  // KF5N JJP 7/14/23
//int EEPROMData.AGCModeOld2 = 30;
int audioYPixel[1024];
int audioPostProcessorCells[AUDIO_POST_PROCESSOR_BANDS];

int bandswitchPins[] = {
  30,  // 80M
  31,  // 40M
  28,  // 20M
  29,  // 17M
  29,  // 15M
  0,   // 12M  Note that 12M and 10M both use the 10M filter, which is always in (no relay).  KF5N September 27, 2023.
  0    // 10M
};
int button9State;
int buttonRead = 0;
int calibrateFlag = 0;
int calTypeFlag = 0;
int calOnFlag = 0;
int chipSelect = BUILTIN_SDCARD;
int countryIndex = -1;
//int EEPROMData.currentBand = BAND_40M;
//int currentBandA = BAND_40M;
//int currentBandB = BAND_40M;
//int EEPROMData.CWFilterIndex = 5;  //AFP10-18-22
int dahLength;
int dcfCount;
int dcfLevel;
int dcfSilenceTimer;
int dcfTheSecond;
int dcfPulseTime;
//int EEPROMData.decoderFlag = DECODER_STATE;  // Startup state for decoder
int demodIndex = 0;  //AFP 2-10-21
int directFreqFlag = 0;
int EEPROMChoice;
int encoderStepOld;
int equalizerRecChoice;
int equalizerXmtChoice;
int fastTuneActive;
int filterLoPositionMarkerOld;
int filterHiPositionMarkerOld;
int FLoCutOld;
int FHiCutOld;
int freqCalibration = -1000;
//int EEPROMData.freqIncrement = DEFAULTEEPROMData.freqIncrement;
int gapAtom;  // Space between atoms
int gapChar;  // Space between characters
int hang_counter = 0;
int helpmin;
int helphour;
int helpday;
int helpmonth;
int helpyear;
int helpsec;
int idx, idpk;
int lidx, hidx;
int IQChoice;
int IQCalType;
int IQEXChoice;
int kDisplay = 0;  //AFP
//int EEPROMData.keyType;
int LMS_nr_strength;
int LP_F_help = 3500;
int mainTuneEncoder;
int micChoice;
int micGainChoice;
int minPinRead = 1024;
int NR_Index = 0;
int n_L;
int n_R;
int n_tau;
int NBChoice;
//int EEPROMData.nrOptionSelect;
int newCursorPosition = 0;
int NR_Choice = 0;
int NR_Filter_Value = -1;
int NR_KIM_K_int = 1000;
int NR_VAD_delay = 0;
int NR_VAD_duration = 0;
int oldCursorPosition = 256;
int operatingMode;
int old_demod_mode = -99;
int oldnotchF = 10000;
int out_index = -1;
//int EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;
//int EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
//int EEPROMData.paddleFlip = PADDLE_FLIP;
int pmode = 1;
int pos_centre_f = 64;
int pos_x_frequency = 12;
int pos_y_smeter = (spectrum_y - 12);
//int EEPROMData.rfGainAllBands = 1;

//int EEPROMData.sdCardPresent = 0;  // Do they have an micro SD card installed?
int secondaryMenuChoiceMade;
int smeterLength;
//int EEPROMData.spectrumNoiseFloor = SPECTRUM_NOISE_FLOOR;
int splitOn;
int switchFilterSideband = 0;

int syncEEPROM;

int termCursorXpos = 0;
//float EEPROMData.transmitPowerLevel;
int x2 = 0;  //AFP

int zoom_sample_ptr = 0;
int zoomIndex = 1;  //AFP 9-26-22
//int EEPROMData.tuneIndex = DEFAULTFREQINDEX;  //AFP 2-10-21
int wtf;
int updateDisplayFlag = 0;
int updateDisplayCounter = 0;
int xrState;  // Is the T41 in xmit or rec state? 1 = rec, 0 = xmt

const int BW_indicator_y = SPECTRUM_TOP_Y + SPECTRUM_HEIGHT + 2;
const int DEC2STATESIZE = n_dec2_taps + (BUFFER_SIZE * N_B / (uint32_t)DF1) - 1;
const int INT1_STATE_SIZE = 24 + BUFFER_SIZE * N_B / (uint32_t)DF - 1;
const int INT2_STATE_SIZE = 8 + BUFFER_SIZE * N_B / (uint32_t)DF1 - 1;
const int myInput = AUDIO_INPUT_LINEIN;
const int pos_x_smeter = 11;
const int waterfallBottom = spectrum_y + spectrum_height + 4;
const int waterfallTop = spectrum_y + spectrum_height + 4;

unsigned framesize;
unsigned nbits;
unsigned ring_buffsize = RB_SIZE;
unsigned tcr5;
unsigned tcr2div;

int32_t FFT_shift = 2048;
//long long EEPROMData.freqCorrectionFactor = 68000LL;
//long long EEPROMData.freqCorrectionFactorOld = 68000LL;
int32_t IFFreq = SR[SampleRate].rate / 4;  // IF (intermediate) frequency
int32_t IF_FREQ1 = 0;
int32_t mainMenuIndex = START_MENU;  // Done so we show menu[0] at startup
int32_t secondaryMenuIndex = -1;     // -1 means haven't determined secondary menu
int32_t subMenuMaxOptions;           // holds the number of submenu options
int32_t subMenuIndex = EEPROMData.currentBand;
int32_t O_iiSum19;
int32_t O_integrateCount19;
//int32_t EEPROMData.spectrum_zoom = SPECTRUM_ZOOM_2;

uint32_t N_BLOCKS = N_B;
uint32_t BUF_N_DF = BUFFER_SIZE * N_BLOCKS / (uint32_t)DF;
uint32_t highAlarmTemp;
uint32_t in_index;
uint32_t lowAlarmTemp;
uint32_t MDR;
uint32_t n_para = 512;
uint32_t NR_X_pointer = 0;
uint32_t NR_E_pointer = 0;
uint32_t IQ_counter = 0;
uint32_t m_NumTaps = (FFT_LENGTH / 2) + 1;
uint32_t panicAlarmTemp; /*!< The panic alarm temperature.*/
uint32_t roomCount;      /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at the hot temperature.*/
uint32_t s_roomC_hotC;   /*!< The value of s_roomCount minus s_hotCount.*/
uint32_t s_hotTemp;      /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at room temperature .*/
uint32_t s_hotCount;     /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at the hot temperature.*/
uint32_t twinpeaks_counter = 0;

//unsigned long EEPROMData.cwTransmitDelay = 2000L;
long averageDit;
long averageDah;

long currentFreq;
//long EEPROMData.centerFreq = 0L;
long CWRecFreq;  //  = TxRxFreq +/- 750Hz
//long currentFreqA = 7150000L;  //Initial VFOA center freq
//long currentFreqAOld2 = 0;
//long currentFreqB = 7030000;  //Initial VFOB center freq
//long currentFreqBOld2 = 0;
//long EEPROMData.currentWPM = 15L;
//long EEPROMData.favoriteFreqs[13];

//long frequencyCorrection;
long incrementValues[] = { 10, 50, 100, 250, 1000, 10000, 100000, 1000000 };
long int n_clear;
//long EEPROMData.lastFrequencies[NUMBER_OF_BANDS][2];
long notchPosOld;
long notchFreq = 1000;
long notchCenterBin;
long recClockFreq;  //  = TxRxFreq+IFFreq  IFFreq from FreqShift1()=48KHz
long signalElapsedTime;
long spaceSpan;
long signalStart;
long signalEnd;  // Start-end of dit or dah
long spaceStart;
long spaceEnd;
long spaceElapsedTime;
long TxRxFreq;  // = EEPROMData.centerFreq+NCOFreq  NCOFreq from FreqShift2()
long TxRxFreqOld;
long TxRxFreqDE;
uint32_t gapLength;
long gapEnd, gapStart;               // Time for noise measures
long ditTime = 80L, dahTime = 240L;  // Assume 15wpm to start

ulong samp_ptr;

uint64_t output12khz;

unsigned long long Clk2SetFreq;                  // AFP 09-27-22
unsigned long long Clk1SetFreq = 1000000000ULL;  // AFP 09-27-22
unsigned long ditLength;
unsigned long transmitDitLength;  // JJP 8/19/23
unsigned long transmitDitUnshapedBlocks;
unsigned long transmitDahUnshapedBlocks;
float dcfRefLevel;
float CPU_temperature = 0.0;

float DD4WH_RF_gain = 6.0;
float help;
float s_hotT_ROOM; /*!< The value of s_hotTemp minus room temperature(25¡æ).*/
float lastII = 0;
float lastQQ = 0;

float RXbit = 0;
float bitSampleTimer = 0;
float Tsample = 1.0 / 12000.0;
//====== SAM stuff AFP 11-02-22
float32_t a[3 * SAM_PLL_HILBERT_STAGES + 3];
float32_t b[3 * SAM_PLL_HILBERT_STAGES + 3];
float32_t c[3 * SAM_PLL_HILBERT_STAGES + 3];  // Filter c variables
float32_t c0[SAM_PLL_HILBERT_STAGES];
float32_t c1[SAM_PLL_HILBERT_STAGES];
float32_t d[3 * SAM_PLL_HILBERT_STAGES + 3];

float32_t abs_ring[RB_SIZE];
float32_t abs_out_sample;
float32_t ai, bi, aq, bq;
float32_t ai_ps, bi_ps, aq_ps, bq_ps;
float32_t ANR_d[ANR_DLINE_SIZE];
float32_t ANR_den_mult = 6.25e-10;
float32_t ANR_gamma = 0.1;
float32_t ANR_lidx = 120.0;
float32_t ANR_lidx_min = 120.0;
float32_t ANR_lidx_max = 200.0;
float32_t ANR_lincr = 1.0;
float32_t ANR_ldecr = 3.0;
float32_t ANR_ngamma = 0.001;
float32_t ANR_two_mu = 0.0001;
float32_t ANR_w[ANR_DLINE_SIZE];
float32_t attack_mult;
float32_t audio;
float32_t audiotmp = 0.0f;
float32_t audiou;
float32_t audioSpectBuffer[1024];  // This can't be DMAMEM.  It will break the S-Meter.  KF5N October 10, 2023
float32_t bass = 0.0;
float32_t farnsworthValue;
float32_t midbass = 0.0;
float32_t mid = 0.0;
float32_t midtreble = 0.0;
float32_t treble = 0.0;
float32_t bin_BW = 1.0 / (DF * FFT_length) * SR[SampleRate].rate;
float32_t bin = 2000.0 / bin_BW;
float32_t biquad_lowpass1_state[N_stages_biquad_lowpass1 * 4];
float32_t biquad_lowpass1_coeffs[5 * N_stages_biquad_lowpass1] = { 0, 0, 0, 0, 0 };
float32_t DMAMEM buffer_spec_FFT[1024] __attribute__((aligned(4)));
float32_t coefficient_set[5] = { 0, 0, 0, 0, 0 };
float32_t corr[2];
float32_t Cos = 0.0;
float32_t cursorIncrementFraction;
float32_t dbm = -145.0;
float32_t dbm_calibration = 22.0;
float32_t dbm_old = -145.0;
float32_t dbmhz = -145.0;
float32_t decay_mult;
float32_t display_offset;
float32_t DMAMEM FFT_buffer[FFT_LENGTH * 2] __attribute__((aligned(4)));
float32_t DMAMEM FFT_spec[1024];
float32_t DMAMEM FFT_spec_old[1024];
float32_t dsI;
float32_t dsQ;
float32_t fast_backaverage;
float32_t fast_backmult;
float32_t fast_decay_mult;

// decimation with FIR lowpass for Zoom FFT
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
float32_t fixed_gain = 1.0;
float32_t DMAMEM float_buffer_L[BUFFER_SIZE * N_B];
float32_t DMAMEM float_buffer_R[BUFFER_SIZE * N_B];
float32_t DMAMEM float_buffer_L2[BUFFER_SIZE * N_B];
float32_t DMAMEM float_buffer_R2[BUFFER_SIZE * N_B];
float32_t float_buffer_L_3[BUFFER_SIZE * N_B];
float32_t float_buffer_R_3[BUFFER_SIZE * N_B];

float32_t DMAMEM float_buffer_L_CW[256];       //AFP 09-01-22
float32_t DMAMEM float_buffer_R_CW[256];       //AFP 09-01-22
float32_t DMAMEM float_buffer_R_AudioCW[256];  //AFP 10-18-22
float32_t DMAMEM float_buffer_L_AudioCW[256];  //AFP 10-18-22
float32_t hang_backaverage;
float32_t hang_backmult;
float32_t hang_decay_mult;
float32_t hang_thresh;
float32_t hang_level;
float32_t hangtime;
float32_t hh1 = 0.0;
float32_t hh2 = 0.0;
float32_t DMAMEM iFFT_buffer[FFT_LENGTH * 2 + 1];
float32_t I_old = 0.2;
float32_t I_sum;
float32_t IIR_biquad_Zoom_FFT_I_state[IIR_biquad_Zoom_FFT_N_stages * 4];
float32_t IIR_biquad_Zoom_FFT_Q_state[IIR_biquad_Zoom_FFT_N_stages * 4];
float32_t inv_max_input;
float32_t inv_out_target;
float32_t IQ_sum = 0.0;
float32_t K_dirty = 0.868;
float32_t K_est = 1.0;
float32_t K_est_old = 0.0;
float32_t K_est_mult = 1.0 / K_est;
float32_t last_dc_level = 0.0f;
float32_t DMAMEM last_sample_buffer_L[BUFFER_SIZE * N_DEC_B];
float32_t DMAMEM last_sample_buffer_R[BUFFER_SIZE * N_DEC_B];
float32_t DMAMEM L_BufferOffset[BUFFER_SIZE * N_B];
float32_t LMS_errsig1[256 + 10];
float32_t LMS_NormCoeff_f32[MAX_LMS_TAPS + MAX_LMS_DELAY];
float32_t LMS_nr_delay[512 + MAX_LMS_DELAY];
float32_t LMS_StateF32[MAX_LMS_TAPS + MAX_LMS_DELAY];
float32_t LP_Astop = 90.0;
float32_t LP_Fpass = 3500.0;
float32_t LP_Fstop = 3600.0;
float32_t LPF_spectrum = 0.82;
float32_t M_c1 = 0.0;
float32_t M_c2 = 0.0;
float32_t m_AttackAlpha = 0.03;
float32_t m_AttackAvedbm = -73.0;
float32_t m_DecayAvedbm = -73.0;
float32_t m_DecayAlpha = 0.01;
float32_t m_AverageMagdbm = -73.0;
float32_t m_AttackAvedbmhz = -103.0;
float32_t m_DecayAvedbmhz = -103.0;
float32_t m_AverageMagdbmhz = -103.0;
float32_t max_gain;
float32_t max_input = -0.1;
float32_t min_volts;
float32_t noiseThreshhold;
float32_t notches[10] = { 500.0, 1000.0, 1500.0, 2000.0, 2500.0, 3000.0, 3500.0, 4000.0, 4500.0, 5000.0 };
float32_t DMAMEM NR_FFT_buffer[512] __attribute__((aligned(4)));
float32_t NR_sum = 0;
float32_t NR_KIM_K = 1.0;
float32_t NR_onemalpha = (1.0 - EEPROMData.NR_alpha);
float32_t NR_onemtwobeta = (1.0 - (2.0 * EEPROMData.NR_beta));
float32_t NR_onembeta = 1.0 - EEPROMData.NR_beta;
float32_t NR_G_bin_m_1;
float32_t NR_G_bin_p_1;
float32_t NR_T;
float32_t DMAMEM NR_output_audio_buffer[NR_FFT_L];
float32_t DMAMEM NR_last_iFFT_result[NR_FFT_L / 2];
float32_t DMAMEM NR_last_sample_buffer_L[NR_FFT_L / 2];
float32_t DMAMEM NR_last_sample_buffer_R[NR_FFT_L / 2];
float32_t DMAMEM NR_X[NR_FFT_L / 2][3];
float32_t DMAMEM NR_E[NR_FFT_L / 2][15];
float32_t DMAMEM NR_M[NR_FFT_L / 2];
float32_t DMAMEM NR_Nest[NR_FFT_L / 2][2];  //
float32_t NR_vk;
float32_t DMAMEM NR_lambda[NR_FFT_L / 2];
float32_t DMAMEM NR_Gts[NR_FFT_L / 2][2];
float32_t DMAMEM NR_G[NR_FFT_L / 2];
float32_t DMAMEM NR_SNR_prio[NR_FFT_L / 2];
float32_t DMAMEM NR_SNR_post[NR_FFT_L / 2];
float32_t NR_SNR_post_pos;
float32_t DMAMEM NR_Hk_old[NR_FFT_L / 2];
float32_t NR_VAD = 0.0;
float32_t NR_VAD_thresh = 6.0;
float32_t DMAMEM NR_long_tone[NR_FFT_L / 2][2];
float32_t DMAMEM NR_long_tone_gain[NR_FFT_L / 2];
float32_t NR_long_tone_alpha = 0.9999;
float32_t NR_long_tone_thresh = 12000;
float32_t NR_gain_smooth_alpha = 0.25;
float32_t NR_temp_sum = 0.0;
float32_t NB_thresh = 2.5;
float32_t offsetDisplayDB = 10.0;
float32_t onemfast_backmult;
float32_t onemhang_backmult;
float32_t out_sample[2];
float32_t out_targ;
float32_t out_target;
float32_t P_dirty = 1.0;
float32_t P_est;
float32_t P_est_old;
float32_t P_est_mult = 1.0 / (sqrtf(1.0 - P_est * P_est));
float32_t phaseLO = 0.0;
float32_t pop_ratio;
float32_t Q_old = 0.2;
float32_t Q_sum;
float32_t DMAMEM R_BufferOffset[BUFFER_SIZE * N_B];
float32_t ring[RB_SIZE * 2];
float32_t ring_max = 0.0;
float32_t sample_meanL = 0.0;
float32_t sample_meanR = 0.0;
float32_t sample_meanLNew = 0.0;  //AFP 10-11-22
float32_t sample_meanRNew = 0.0;
float32_t save_volts = 0.0;
float32_t slope_constant;
float32_t stereo_factor = 100.0;
float32_t tau_attack;
float32_t tau_decay;
float32_t tau_fast_backaverage = 0.0;
float32_t tau_fast_decay;
float32_t tau_hang_backmult;
float32_t tau_hang_decay;
float32_t teta1 = 0.0;
float32_t teta2 = 0.0;
float32_t teta3 = 0.0;
float32_t teta1_old = 0.0;
float32_t teta2_old = 0.0;
float32_t teta3_old = 0.0;
float32_t tmp;
float32_t var_gain;
float32_t volts = 0.0;
float32_t w;
float32_t wold = 0.0f;

float angl;
float bitSamplePeriod = 1.0 / 500.0;
float bsq, usq;
float cf1, cf2;
float dcfMean;
float dcfSum;
float lolim, hilim;
float partr, parti;
float pi = 3.14159265358979;
float tau;
float temp;
float xExpand = 1.5;  //
float x;

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
double elapsed_micros_mean;
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

//#pragma GCC diagnostic ignored "-Wunused-variable"

PROGMEM
//==================================================================================

/*****
  Purpose: To set the codec gain

  Parameter list:
    void

  Return value:
    void

*****/
void Codec_gain() {
  static uint32_t timer = 0;
  timer++;
  if (timer > 10000) timer = 10000;
  if (half_clip == 1)  // did clipping almost occur?
  {
    if (timer >= 20)  // 100  // has enough time passed since the last gain decrease?
    {
      if (bands[EEPROMData.currentBand].RFgain != 0)  // yes - is this NOT zero?
      {
        bands[EEPROMData.currentBand].RFgain -= 1;  // decrease gain one step, 1.5dB
        if (bands[EEPROMData.currentBand].RFgain < 0) {
          bands[EEPROMData.currentBand].RFgain = 0;
        }
        timer = 0;  // reset the adjustment timer
        AudioNoInterrupts();
        AudioInterrupts();
        if (Menu2 == MENU_RF_GAIN) {
          //         ShowMenu(1);
        }
      }
    }
  } else if (quarter_clip == 0)  // no clipping occurred
  {
    if (timer >= 50)  // 500   // has it been long enough since the last increase?
    {
      bands[EEPROMData.currentBand].RFgain += 1;  // increase gain by one step, 1.5dB
      timer = 0;                                  // reset the timer to prevent this from executing too often
      if (bands[EEPROMData.currentBand].RFgain > 15) {
        bands[EEPROMData.currentBand].RFgain = 15;
      }
      AudioNoInterrupts();
      AudioInterrupts();
    }
  }
  half_clip = 0;     // clear "half clip" indicator that tells us that we should decrease gain
  quarter_clip = 0;  // clear indicator that, if not triggered, indicates that we can increase gain
}

// is added in Teensyduino 1.52 beta-4, so this can be deleted !?

/*****
  Purpose: To set the real time clock

  Parameter list:
    void

  Return value:
    void
*****/
void T4_rtc_set(unsigned long t) {
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
void initTempMon(uint16_t freq, uint32_t lowAlarmTemp, uint32_t highAlarmTemp, uint32_t panicAlarmTemp) {

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
  Purpose: to cause a delay in program execution

  Parameter list:
    unsigned long millisWait    // the number of millseconds to wait

  Return value:
    void
*****/
void MyDelay(unsigned long millisWait) {
  unsigned long now = millis();

  while (millis() - now < millisWait)
    ;  // Twiddle thumbs until delay ends...
}


/*****
  Purpose: to collect array inits in one place

  Parameter list:
    void

  Return value:
    void
*****/
void InitializeDataArrays() {
  //DB2OO, 11-SEP-23: don't use the fixed sizes, but use the caculated ones, otherwise a code change will create very difficult to find problems
#define CLEAR_VAR(x) memset(x, 0, sizeof(x))
  memset(FFT_spec_old, 0, sizeof(FFT_spec_old));
#ifdef DEBUG
  Serial.printf("InitializeDataArrays(): sizeof(FFT_spec_old) %d", sizeof(FFT_spec_old));
  Serial.printf("\tsizeof(NR_output_audio_buffer) %d", sizeof(NR_output_audio_buffer));
  Serial.printf("\tsizeof(LMS_StateF32) %d", sizeof(LMS_StateF32));
  Serial.println();
#endif
  CLEAR_VAR(FFT_spec_old);             //memset(FFT_spec_old, 0, 4096);            // SPECTRUM_RES = 512 * 4 = 2048
  CLEAR_VAR(pixelnew);                 //memset(pixelnew, 0, 1024);                // 512 * 2
  CLEAR_VAR(pixelold);                 //memset(pixelold, 0, 1024);                // 512 * 2
  CLEAR_VAR(pixelCurrent);             //memset(pixelCurrent, 0, 1024);            // 512 * 2  KF5N JJP  7/14/23
  CLEAR_VAR(buffer_spec_FFT);          //memset(buffer_spec_FFT, 0, 4096);         // SPECTRUM_RES = 512 * 2 = 1024
  CLEAR_VAR(FFT_spec);                 //memset(FFT_spec, 0, 4096);                // 512 * 2 * 4
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
  CLEAR_VAR(ANR_d);                    //memset(ANR_d, 0, 2048);
  CLEAR_VAR(ANR_w);                    //memset(ANR_w, 0, 2048);
  CLEAR_VAR(LMS_StateF32);             //memset(LMS_StateF32, 0, 1408);  // 96 + 256 * 4
  CLEAR_VAR(LMS_NormCoeff_f32);        //memset(LMS_NormCoeff_f32, 0, 1408);
  CLEAR_VAR(LMS_nr_delay);             //memset(LMS_nr_delay, 0, 2312);

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
  // essential ?
  IFFreq = SR[SampleRate].rate / 4;

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

  ShowBandwidth();

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
  //    CalcFIRCoeffs(FIR_int1_coeffs, 8, (float32_t)5000.0, 80, 0, 0.0, 12000);
  //    CalcFIRCoeffs(FIR_int1_coeffs, 16, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, SR[SampleRate].rate / 4.0);
  CalcFIRCoeffs(FIR_int1_coeffs, 48, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, SR[SampleRate].rate / 4.0);
  //    if(arm_fir_interpolate_init_f32(&FIR_int1_I, (uint32_t)DF2, 16, FIR_int1_coeffs, FIR_int1_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
  if (arm_fir_interpolate_init_f32(&FIR_int1_I, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    while (1)
      ;
  }
  //    if(arm_fir_interpolate_init_f32(&FIR_int1_Q, (uint32_t)DF2, 16, FIR_int1_coeffs, FIR_int1_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
  if (arm_fir_interpolate_init_f32(&FIR_int1_Q, (uint8_t)DF2, 48, FIR_int1_coeffs, FIR_int1_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF)) {
    while (1)
      ;
  }
  // Interpolation filter 2, L2 = 4
  // not sure whether I should design with the final sample rate ??
  // yes, because the interpolation filter is AFTER the upsampling, so it has to be in the target sample rate!
  //    CalcFIRCoeffs(FIR_int2_coeffs, 4, (float32_t)5000.0, 80, 0, 0.0, 24000);
  //    CalcFIRCoeffs(FIR_int2_coeffs, 16, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SampleRate].rate);
  CalcFIRCoeffs(FIR_int2_coeffs, 32, (float32_t)(n_desired_BW * 1000.0), n_att, 0, 0.0, (float32_t)SR[SampleRate].rate);

  if (arm_fir_interpolate_init_f32(&FIR_int2_I, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_I_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }
  //    if(arm_fir_interpolate_init_f32(&FIR_int2_Q, (uint32_t)DF1, 16, FIR_int2_coeffs, FIR_int2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
  if (arm_fir_interpolate_init_f32(&FIR_int2_Q, (uint8_t)DF1, 32, FIR_int2_coeffs, FIR_int2_Q_state, BUFFER_SIZE * N_BLOCKS / (uint32_t)DF1)) {
    while (1)
      ;
  }

  SetDecIntFilters();  // here, the correct bandwidths are calculated and set accordingly

  /****************************************************************************************
     Zoom FFT: Initiate decimation and interpolation FIR filters AND IIR filters
  ****************************************************************************************/
  float32_t Fstop_Zoom = 0.5 * (float32_t)SR[SampleRate].rate / (1 << EEPROMData.spectrum_zoom);

  CalcFIRCoeffs(Fir_Zoom_FFT_Decimate_coeffs, 4, Fstop_Zoom, 60, 0, 0.0, (float32_t)SR[SampleRate].rate);

  // Attention: max decimation rate is 128 !
  //  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I, 4, 1 << EEPROMData.spectrum_zoom, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_I_state, BUFFER_SIZE * N_BLOCKS)) {
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I, 4, 128, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_I_state, BUFFER_SIZE * N_BLOCKS)) {
    while (1)
      ;
  }
  // same coefficients, but specific state variables
  //  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q, 4, 1 << EEPROMData.spectrum_zoom, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_Q_state, BUFFER_SIZE * N_BLOCKS)) {
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q, 4, 128, Fir_Zoom_FFT_Decimate_coeffs, Fir_Zoom_FFT_Decimate_Q_state, BUFFER_SIZE * N_BLOCKS)) {
    while (1)
      ;
  }

  IIR_biquad_Zoom_FFT_I.numStages = IIR_biquad_Zoom_FFT_N_stages;  // set number of stages
  IIR_biquad_Zoom_FFT_Q.numStages = IIR_biquad_Zoom_FFT_N_stages;  // set number of stages
  for (unsigned i = 0; i < 4 * IIR_biquad_Zoom_FFT_N_stages; i++) {
    IIR_biquad_Zoom_FFT_I_state[i] = 0.0;  // set state variables to zero
    IIR_biquad_Zoom_FFT_Q_state[i] = 0.0;  // set state variables to zero
  }
  IIR_biquad_Zoom_FFT_I.pState = IIR_biquad_Zoom_FFT_I_state;  // set pointer to the state variables
  IIR_biquad_Zoom_FFT_Q.pState = IIR_biquad_Zoom_FFT_Q_state;  // set pointer to the state variables

  // this sets the coefficients for the ZoomFFT decimation filter
  // according to the desired magnification mode
  // for 0 the mag_coeffs will a NULL  ptr, since the filter is not going to be used in this  mode!
  IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[EEPROMData.spectrum_zoom];
  IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[EEPROMData.spectrum_zoom];

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
}


/*****
  Purpose: Manage AudioRecordQueue objects and patchCord connections based on
           the radio's operating mode in a way that minimizes unnecessary
           AudioMemory usage.

  Parameter list:
    int operatingState    radioState/lastState constant indicating desired state

  Return value:
    void

*****/
void SetAudioOperatingState(int operatingState) {
#ifdef DEBUG
  Serial.printf("lastState=%d radioState=%d memory_used=%d memory_used_max=%d f32_memory_used=%d f32_memory_used_max=%d\n",
                lastState,
                radioState,
                (int)AudioStream::memory_used,
                (int)AudioStream::memory_used_max,
                (int)AudioStream_F32::f32_memory_used,
                (int)AudioStream_F32::f32_memory_used_max);
  AudioStream::memory_used_max = 0;
  AudioStream_F32::f32_memory_used_max = 0;
#endif
  switch (operatingState) {
    case SSB_RECEIVE_STATE:
    case CW_RECEIVE_STATE:
      // QSD connected and enabled
      Q_in_L.begin();
      Q_in_R.begin();
      patchCord9.connect();
      patchCord10.connect();

      // Microphone input disabled and disconnected
      patchCord1.disconnect();
      patchCord2.disconnect();
      Q_in_L_Ex.end();
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();
      Q_in_R_Ex.clear();

      // CW sidetone output disconnected
      patchCord23.disconnect();
      patchCord24.disconnect();

      break;
    case SSB_TRANSMIT_STATE:
      // QSD disabled and disconnected
      patchCord9.disconnect();
      patchCord10.disconnect();
      Q_in_L.end();
      Q_in_L.clear();
      Q_in_R.end();
      Q_in_R.clear();

      // Microphone input enabled and connected
      Q_in_L_Ex.begin();
      Q_in_R_Ex.begin();
      patchCord1.connect();
      patchCord2.connect();

      // CW sidetone output disconnected
      patchCord23.disconnect();
      patchCord24.disconnect();

      break;
    case CW_TRANSMIT_STRAIGHT_STATE:
    case CW_TRANSMIT_KEYER_STATE:
      // QSD disabled and disconnected
      patchCord9.disconnect();
      patchCord10.disconnect();
      Q_in_L.end();
      Q_in_L.clear();
      Q_in_R.end();
      Q_in_R.clear();

      // Microphone input disabled and disconnected
      patchCord1.disconnect();
      patchCord2.disconnect();
      Q_in_L_Ex.end();
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();
      Q_in_R_Ex.clear();

      // CW sidetone output connected
      patchCord23.connect();
      patchCord24.connect();

      break;
  }
}


/*****
  Purpose: The initial screen display on startup. Expect this to be customized.

  Parameter list:
    void

  Return value:
    void
*****/
void Splash() {
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
  tft.print(VERSION);
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
  MyDelay(SPLASH_DELAY);
  tft.fillWindow(RA8875_BLACK);
}


/*****
  Purpose: program entry point that sets the environment for program

  Parameter list:
    void

  Return value:
    void
*****/
void setup() {

  Serial.begin(9600);

  setSyncProvider(getTeensy3Time);  // get TIME from real time clock with 3V backup battery
  setTime(now());
  Teensy3Clock.set(now());  // set the RTC
  T4_rtc_set(Teensy3Clock.get());

  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  AudioMemory(500);  //  Increased to 450 from 400.  Memory was hitting max.  KF5N August 31, 2023
  AudioMemory_F32(10);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(20);
  sgtl5000_1.lineInLevel(0);
  sgtl5000_1.lineOutLevel(20);
  sgtl5000_1.adcHighPassFilterDisable();  //reduces noise.  https://forum.pjrc.com/threads/27215-24-bit-audio-boards?p=78831&viewfull=1#post78831
  sgtl5000_2.setAddress(HIGH);
  sgtl5000_2.enable();
  sgtl5000_2.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_2.volume(0.5);

  pinMode(FILTERPIN15M, OUTPUT);
  pinMode(FILTERPIN20M, OUTPUT);
  pinMode(FILTERPIN40M, OUTPUT);
  pinMode(FILTERPIN80M, OUTPUT);
  pinMode(RXTX, OUTPUT);
  pinMode(MUTE, OUTPUT);
  digitalWrite(MUTE, LOW);
  pinMode(PTT, INPUT_PULLUP);
  pinMode(BUSY_ANALOG_PIN, INPUT);
  pinMode(FILTER_ENCODER_A, INPUT);
  pinMode(FILTER_ENCODER_B, INPUT);
  pinMode(OPTO_OUTPUT, OUTPUT);
  pinMode(KEYER_DIT_INPUT_TIP, INPUT_PULLUP);
  pinMode(KEYER_DAH_INPUT_RING, INPUT_PULLUP);
  pinMode(TFT_MOSI, OUTPUT);
  digitalWrite(TFT_MOSI, HIGH);
  pinMode(TFT_SCLK, OUTPUT);
  digitalWrite(TFT_SCLK, HIGH);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  arm_fir_init_f32(&FIR_Hilbert_L, 100, FIR_Hilbert_coeffs_45, FIR_Hilbert_state_L, 256);  //AFP01-16-22
  arm_fir_init_f32(&FIR_Hilbert_R, 100, FIR_Hilbert_coeffs_neg45, FIR_Hilbert_state_R, 256);
  arm_fir_init_f32(&FIR_CW_DecodeL, 64, CW_Filter_Coeffs2, FIR_CW_DecodeL_state, 256);  //AFP 10-25-22
  arm_fir_init_f32(&FIR_CW_DecodeR, 64, CW_Filter_Coeffs2, FIR_CW_DecodeR_state, 256);
  arm_fir_decimate_init_f32(&FIR_dec1_EX_I, 48, 4, coeffs192K_10K_LPF_FIR, FIR_dec1_EX_I_state, 2048);
  arm_fir_decimate_init_f32(&FIR_dec1_EX_Q, 48, 4, coeffs192K_10K_LPF_FIR, FIR_dec1_EX_Q_state, 2048);
  arm_fir_decimate_init_f32(&FIR_dec2_EX_I, 24, 2, coeffs48K_8K_LPF_FIR, FIR_dec2_EX_I_state, 512);
  arm_fir_decimate_init_f32(&FIR_dec2_EX_Q, 24, 2, coeffs48K_8K_LPF_FIR, FIR_dec2_EX_Q_state, 512);
  arm_fir_interpolate_init_f32(&FIR_int1_EX_I, 2, 48, coeffs48K_8K_LPF_FIR, FIR_int1_EX_I_state, 256);
  arm_fir_interpolate_init_f32(&FIR_int1_EX_Q, 2, 48, coeffs48K_8K_LPF_FIR, FIR_int1_EX_Q_state, 256);
  arm_fir_interpolate_init_f32(&FIR_int2_EX_I, 4, 32, coeffs192K_10K_LPF_FIR, FIR_int2_EX_I_state, 512);
  arm_fir_interpolate_init_f32(&FIR_int2_EX_Q, 4, 32, coeffs192K_10K_LPF_FIR, FIR_int2_EX_Q_state, 512);

  //***********************  EQ Gain Settings ************
  uint32_t iospeed_display = IOMUXC_PAD_DSE(3) | IOMUXC_PAD_SPEED(1);
  *(digital_pin_to_info_PGM + 13)->pad = iospeed_display;  //clk
  *(digital_pin_to_info_PGM + 11)->pad = iospeed_display;  //MOSI
  *(digital_pin_to_info_PGM + TFT_CS)->pad = iospeed_display;

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

  EEPROMData.sdCardPresent = InitializeSDCard();  // Is there an SD card that can be initialized?

  // =============== EEPROM section =================
  EnableButtonInterrupts();
  EEPROMStartup();

  spectrum_x = 10;
  spectrum_y = 150;
  xExpand = 1.4;
  h = 135;
  //  EEPROMData.nrOptionSelect = 0;

  Q_in_L.begin();  //Initialize receive input buffers
  Q_in_R.begin();
  MyDelay(100L);

  //  EEPROMData.freqIncrement = incrementValues[EEPROMData.tuneIndex];
  NR_Index = EEPROMData.nrOptionSelect;
  NCOFreq = 0L;


  // ========================  End set up of Parameters from EEPROM data ===============
  NCOFreq = 0;

  /****************************************************************************************
     start local oscillator Si5351
  ****************************************************************************************/
  si5351.reset();                                                                           // KF5N.  Moved Si5351 start-up to setup. JJP  7/14/23
  si5351.init(SI5351_CRYSTAL_LOAD_10PF, Si_5351_crystal, EEPROMData.freqCorrectionFactor);  //JJP  7/14/23
  si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);                                           //  Allows CLK1 and CLK2 to exceed 100 MHz simultaneously.
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);                                     //AFP 10-13-22
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);                                     //CWP AFP 10-13-22
  // Turn off LOs.
  si5351.output_enable(SI5351_CLK2, 0);
  si5351.output_enable(SI5351_CLK1, 0);
  if (EEPROMData.xmtMode == CW_MODE && EEPROMData.decoderFlag == DECODE_OFF) {
    EEPROMData.decoderFlag = DECODE_OFF;  // JJP 7/1/23
  } else {
    EEPROMData.decoderFlag = DECODE_ON;  // Turns decoder on JJP 7/1/23
  }

  //TxRxFreq = EEPROMData.centerFreq + NCOFreq;
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = EEPROMData.centerFreq = EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO];

  InitializeDataArrays();
  splitOn = 0;  // Split VFO not active
  SetupMode(bands[EEPROMData.currentBand].mode);

  //EEPROMData.currentWPM = EEPROMData.EEPROMData.currentWPM;  // Not required.  Retrieved by EEPROMRead().  KF5N August 27, 2023
  SetKeyPowerUp();  // Use EEPROMData.keyType and EEPROMData.paddleFlip to configure key GPIs.  KF5N August 27, 2023
  SetDitLength(EEPROMData.currentWPM);
  SetTransmitDitLength(EEPROMData.currentWPM);
  CWFreqShift = 750;
  //calFreqShift = 0;
  // Initialize buffer used by CW transmitter.
  sineTone(EEPROMData.CWOffset + 6);  // This function takes "number of cycles" which is the offset + 6.
  initCWShaping();
  // Initialize buffer used by CW decoder.
  float freq[4] = { 562.5, 656.5, 750.0, 843.75 };
  float theta;
  for (int kf = 0; kf < 255; kf++) {                                   //Calc sine wave
    theta = (float)kf * TWO_PI * freq[EEPROMData.CWOffset] / 24000.0;  // theta = kf * 2 * PI * freqSideTone / 24000
    sinBuffer[kf] = sin(theta);
  }
  filterEncoderMove = 0;
  fineTuneEncoderMove = 0L;
  xrState = RECEIVE_STATE;  // Enter loop() in receive state.  KF5N July 22, 2023
  UpdateInfoWindow();
  DrawSpectrumDisplayContainer();
  RedrawDisplayScreen();

  mainMenuIndex = 0;             // Changed from middle to first. Do Menu Down to get to Calibrate quickly
  menuStatus = NO_MENUS_ACTIVE;  // Blank menu field
  ShowName();

  ShowBandwidth();
  FilterBandwidth();
  ShowFrequency();
  SetFreq();
  zoomIndex = EEPROMData.spectrum_zoom - 1;  // ButtonZoom() increments zoomIndex, so this cancels it so the read from EEPROM is accurately restored.  KF5N August 3, 2023
  ButtonZoom();                              // Restore zoom settings.  KF5N August 3, 2023
  knee_dBFS = -15.0;                         // Is this variable actually used???
  comp_ratio = 5.0;
  attack_sec = .1;
  release_sec = 2.0;
  comp1.setPreGain_dB(-10);  //set the gain of the Left-channel gain processor
  comp2.setPreGain_dB(-10);  //set the gain of the Right-channel gain processor

  EEPROMData.sdCardPresent = SDPresentCheck();  // JJP 7/18/23
  lastState = 1111;                             // To make sure the receiver will be configured on the first pass through.  KF5N September 3, 2023
  decodeStates = state0;                        // Initialize the Morse decoder.
  UpdateDecoderField();                         // Adjust graphics for Morse decoder.

  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();  // Required only for QSD2/QSE2.
}
//============================================================== END setup() =================================================================
//===============================================================================================================================

elapsedMicros usec = 0;  // Automatically increases as time passes; no ++ necessary.

/*****
  Purpose: Code here executes forever, or until: 1) power is removed, 2) user does a reset, 3) a component
           fails, or 4) the cows come home.

  Parameter list:
    void

  Return value:
    void
*****/
FASTRUN void loop()  // Replaced entire loop() with Greg's code  JJP  7/14/23
{
  int pushButtonSwitchIndex = -1;
  int valPin;
  long ditTimerOff;  //AFP 09-22-22
  long dahTimerOn;
  bool cwKeyDown;
  unsigned long cwBlockIndex;

  valPin = ReadSelectedPushButton();
  if (valPin != BOGUS_PIN_READ && xrState != TRANSMIT_STATE) {
    pushButtonSwitchIndex = ProcessButtonPress(valPin);
    ExecuteButtonPress(pushButtonSwitchIndex);
  }
  //  State detection
  if (EEPROMData.xmtMode == SSB_MODE && digitalRead(PTT) == HIGH) radioState = SSB_RECEIVE_STATE;
  if (EEPROMData.xmtMode == SSB_MODE && digitalRead(PTT) == LOW) radioState = SSB_TRANSMIT_STATE;
  if (EEPROMData.xmtMode == CW_MODE && (digitalRead(EEPROMData.paddleDit) == HIGH && digitalRead(EEPROMData.paddleDah) == HIGH)) radioState = CW_RECEIVE_STATE;  // Was using symbolic constants. Also changed in code below.  KF5N August 8, 2023
  if (EEPROMData.xmtMode == CW_MODE && (digitalRead(EEPROMData.paddleDit) == LOW && EEPROMData.xmtMode == CW_MODE && EEPROMData.keyType == 0)) radioState = CW_TRANSMIT_STRAIGHT_STATE;
  if (EEPROMData.xmtMode == CW_MODE && (keyPressedOn == 1 && EEPROMData.xmtMode == CW_MODE && EEPROMData.keyType == 1)) radioState = CW_TRANSMIT_KEYER_STATE;
  if (lastState != radioState) {
    SetFreq();  // Update frequencies if the radio state has changed.
    SetAudioOperatingState(radioState);
  }

  //  Begin radio state machines

  //  Begin SSB Mode state machine

  switch (radioState) {
    case (SSB_RECEIVE_STATE):
      if (lastState != radioState) {  // G0ORX 01092023
        digitalWrite(MUTE, LOW);      // Audio Mute off
        modeSelectInR.gain(0, 1);
        modeSelectInL.gain(0, 1);
        digitalWrite(RXTX, LOW);  //xmit off
        T41State = SSB_RECEIVE;
        xrState = RECEIVE_STATE;
        modeSelectInR.gain(0, 1);
        modeSelectInL.gain(0, 1);
        modeSelectInExR.gain(0, 0);
        modeSelectInExL.gain(0, 0);
        modeSelectOutL.gain(0, 1);
        modeSelectOutR.gain(0, 1);
        modeSelectOutL.gain(1, 0);
        modeSelectOutR.gain(1, 0);
        modeSelectOutExL.gain(0, 0);
        modeSelectOutExR.gain(0, 0);
        phaseLO = 0.0;
        barGraphUpdate = 0;
        if (keyPressedOn == 1) {
          return;
        }
        ShowTransmitReceiveStatus();
      }
      ShowSpectrum();
      break;
    case SSB_TRANSMIT_STATE:
      comp1.setPreGain_dB(EEPROMData.currentMicGain);
      comp2.setPreGain_dB(EEPROMData.currentMicGain);
      if (EEPROMData.compressorFlag == 1) {
        SetupMyCompressors(use_HP_filter, (float)EEPROMData.currentMicThreshold, comp_ratio, attack_sec, release_sec);  // Cast EEPROMData.currentMicThreshold to float.  KF5N, October 31, 2023
      } else {
        if (EEPROMData.compressorFlag == 0) {
          SetupMyCompressors(use_HP_filter, 0.0, comp_ratio, 0.01, 0.01);
        }
      }
      xrState = TRANSMIT_STATE;
      // centerTuneFlag = 1;  Not required with revised tuning scheme.  KF5N July 22, 2023
      digitalWrite(MUTE, HIGH);  //  Mute Audio  (HIGH=Mute)
      digitalWrite(RXTX, HIGH);  //xmit on
      xrState = TRANSMIT_STATE;
      modeSelectInR.gain(0, 0);
      modeSelectInL.gain(0, 0);
      modeSelectInExR.gain(0, 1);
      modeSelectInExL.gain(0, 1);
      modeSelectOutL.gain(0, 0);
      modeSelectOutR.gain(0, 0);
      modeSelectOutExL.gain(0, EEPROMData.powerOutSSB[EEPROMData.currentBand]);  //AFP 10-21-22
      modeSelectOutExR.gain(0, EEPROMData.powerOutSSB[EEPROMData.currentBand]);  //AFP 10-21-22
      ShowTransmitReceiveStatus();

      while (digitalRead(PTT) == LOW) {
        ExciterIQData();
      }
      xrState = RECEIVE_STATE;
      break;
    default:
      break;
  }
  //======================  End SSB Mode =================

  // Begin CW Mode state machine

  switch (radioState) {
    case CW_RECEIVE_STATE:
      if (lastState != radioState) {  // G0ORX 01092023
        digitalWrite(MUTE, LOW);      //turn off mute
        T41State = CW_RECEIVE;
        ShowTransmitReceiveStatus();
        xrState = RECEIVE_STATE;
        //SetFreq();   // KF5N
        modeSelectInR.gain(0, 1);
        modeSelectInL.gain(0, 1);
        modeSelectInExR.gain(0, 0);
        modeSelectInExL.gain(0, 0);
        modeSelectOutL.gain(0, 1);
        modeSelectOutR.gain(0, 1);
        modeSelectOutL.gain(1, 0);
        modeSelectOutR.gain(1, 0);
        modeSelectOutExL.gain(0, 0);
        modeSelectOutExR.gain(0, 0);
        phaseLO = 0.0;
        barGraphUpdate = 0;
        keyPressedOn = 0;
      }
      ShowSpectrum();  // if removed CW signal on is 2 mS
      break;
    case CW_TRANSMIT_STRAIGHT_STATE:
      EEPROMData.powerOutCW[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand];
      xrState = TRANSMIT_STATE;
      ShowTransmitReceiveStatus();
      modeSelectInR.gain(0, 0);
      modeSelectInL.gain(0, 0);
      modeSelectInExR.gain(0, 0);
      modeSelectOutL.gain(0, 0);
      modeSelectOutR.gain(0, 0);
      modeSelectOutExL.gain(0, 0);
      modeSelectOutExR.gain(0, 0);
      digitalWrite(MUTE, LOW);  // unmutes audio
      cwKeyDown = false;
      cwTimer = millis();
      while (millis() - cwTimer <= EEPROMData.cwTransmitDelay) {  //Start CW transmit timer on
        digitalWrite(RXTX, HIGH);
        if (digitalRead(EEPROMData.paddleDit) == LOW && EEPROMData.keyType == 0) {  // AFP 09-25-22  Turn on CW signal
          cwTimer = millis();                                                       //Reset timer

          if (!cwKeyDown) {
            modeSelectOutExL.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
            modeSelectOutExR.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
            modeSelectOutL.gain(1, volumeLog[(int)EEPROMData.sidetoneVolume]);        // Sidetone  AFP 10-01-22

            CW_ExciterIQData(CW_SHAPING_RISE);
            cwKeyDown = true;
          } else {
            CW_ExciterIQData(CW_SHAPING_NONE);
          }
        } else {
          if (digitalRead(EEPROMData.paddleDit) == HIGH && EEPROMData.keyType == 0) {  //Turn off CW signal
            keyPressedOn = 0;

            if (cwKeyDown) {
              CW_ExciterIQData(CW_SHAPING_FALL);
              cwKeyDown = false;
            }
          }
        }
      }
      digitalWrite(MUTE, HIGH);   // mutes audio
      modeSelectOutL.gain(1, 0);  // Sidetone off
      modeSelectOutR.gain(1, 0);
      modeSelectOutExL.gain(0, 0);  //Power = 0 //AFP 10-11-22
      modeSelectOutExR.gain(0, 0);  //AFP 10-11-22
      digitalWrite(RXTX, LOW);      // End Straight Key Mode
      break;
    case CW_TRANSMIT_KEYER_STATE:
      EEPROMData.powerOutCW[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand];
      xrState = TRANSMIT_STATE;
      ShowTransmitReceiveStatus();
      modeSelectInR.gain(0, 0);
      modeSelectInL.gain(0, 0);
      modeSelectInExR.gain(0, 0);
      modeSelectOutL.gain(0, 0);
      modeSelectOutR.gain(0, 0);
      modeSelectOutExL.gain(0, 0);
      modeSelectOutExR.gain(0, 0);
      digitalWrite(MUTE, LOW);  // unmutes audio
      cwTimer = millis();
      while (millis() - cwTimer <= EEPROMData.cwTransmitDelay) {
        digitalWrite(RXTX, HIGH);

        if (digitalRead(EEPROMData.paddleDit) == LOW) {  // Keyer Dit
          ditTimerOn = millis();
          modeSelectOutExL.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
          modeSelectOutExR.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
          modeSelectOutL.gain(1, volumeLog[(int)EEPROMData.sidetoneVolume]);        // Sidetone

          // Queue audio blocks--execution time of this loop will be between 0-20ms shorter
          // than the desired dit time, due to audio buffering
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
            ;
          }

          modeSelectOutExL.gain(0, 0);  //Power =0
          modeSelectOutExR.gain(0, 0);
          modeSelectOutL.gain(1, 0);  // Sidetone off
          modeSelectOutR.gain(1, 0);

          cwTimer = millis();
        } else {
          if (digitalRead(EEPROMData.paddleDah) == LOW) {  //Keyer DAH
            dahTimerOn = millis();
            modeSelectOutExL.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);
            modeSelectOutExR.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);
            modeSelectOutL.gain(1, volumeLog[(int)EEPROMData.sidetoneVolume]);

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
              ;
            }

            modeSelectOutExL.gain(0, 0);  //Power =0
            modeSelectOutExR.gain(0, 0);
            modeSelectOutL.gain(1, 0);  // Sidetone off
            modeSelectOutR.gain(1, 0);

            cwTimer = millis();
          }
        }
        keyPressedOn = 0;  // Fix for keyer click-clack.  KF5N August 16, 2023
      }                    //End Relay timer

      digitalWrite(MUTE, HIGH);   // mutes audio
      modeSelectOutL.gain(1, 0);  // Sidetone off
      modeSelectOutR.gain(1, 0);
      modeSelectOutExL.gain(0, 0);  //Power = 0 //AFP 10-11-22
      modeSelectOutExR.gain(0, 0);  //AFP 10-11-22
      digitalWrite(RXTX, LOW);      // End Straight Key Mode
      break;
    default:
      break;
  }

  //  End radio state machine
  if (lastState != radioState) {  // G0ORX 09012023
    lastState = radioState;
    ShowTransmitReceiveStatus();
  }
  //  ShowTransmitReceiveStatus();

#ifdef DEBUG1
  if (elapsed_micros_idx_t > (SR[SampleRate].rate / 960)) {
    ShowTempAndLoad();
    // Used to monitor CPU temp and load factors
  }
#endif

  if (volumeChangeFlag == true) {
    volumeChangeFlag = false;
    UpdateVolumeField();
  }
  /* if (ms_500.check() == 1)                                  // For clock updates AFP 10-26-22
    {
     //wait_flag = 0;
     DisplayClock();
    }*/
}
