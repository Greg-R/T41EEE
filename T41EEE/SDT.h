#pragma once

//======================================== User section that might need to be changed ===================================
#include "MyConfigurationFile.h"  // This file name should remain unchanged
#define VERSION "T41EEE.2"        // Change this for updates. If you make this longer than 9 characters, brace yourself for surprises

struct maps {
  char mapNames[50];
  float lat;
  float lon;
};
extern struct maps myMapFiles[];

//======================================== Library include files ========================================================
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include "Fonts/FreeMonoBold24pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"
#include "Fonts/FreeMono24pt7b.h"
#include "Fonts/FreeMono9pt7b.h"
#include <Audio.h>                     //https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include <OpenAudio_ArduinoLibrary.h>  // AFP 11-01-22
#include <TimeLib.h>                   // Part of Teensy Time library
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Metro.h>
#include <Bounce.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <si5351.h>  // https://github.com/etherkit/Si5351Arduino
#include <RA8875.h>  // https://github.com/mjs513/RA8875/tree/RA8875_t4
#include <Rotary.h>  // https://github.com/brianlow/Rotary
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_view>
#include <util/crc16.h>        // mdrhere
#include <utility/imxrt_hw.h>  // for setting I2S freq, Thanks, FrankB!
#include <EEPROM.h>
//======================================== Symbolic Constants for the T41 ===================================================
#define RIGNAME "T41-EP SDT"
#define NUMBER_OF_SWITCHES 18  // Number of push button switches. 16 on older boards
#define TOP_MENU_COUNT 13      // Menus to process AFP 09-27-22, JJP 7-8-23
#define RIGNAME_X_OFFSET 570   // Pixel count to rig name field
#define RA8875_DISPLAY 1       // Comment out if not using RA8875 display
#define TEMPMON_ROOMTEMP 25.0f
#define SD_CS BUILTIN_SDCARD  // Works on T_3.6 and T_4.1 ...
#define OFF 0
#define ON 1

//================================ mapping globals and Symbolic constants ================
#define BUFFPIXEL 20  // Use buffer to read image rather than 1 pixel at a time
#define DEGREES2RADIANS 0.01745329
#define RADIANS2DEGREES 57.29578
#define PI_BY_180 0.01745329
#define NEW_SI5351_FREQ_MULT 1UL

//======================================== Symbolic constants ==========================================================

// These constants are used by the voltage divider network so only 1 analog pin is used for the 16 option switches. These may need
// to be changed for the exact value for your system. They are initialized in the INO file.
#define BUSY_ANALOG_PIN 39       // This is the analog pin that controls the 18 switches
#define NOTHING_TO_SEE_HERE 950  // If the analog pin is greater than this value, nothing's going on
#define BOGUS_PIN_READ -1        // If no push button read
#define WIGGLE_ROOM 20  // This is the maximum value that can added to a BUSY_ANALOG_PIN pin read value of a push
//                         button and still have the switch value be associated with the correct push button.
#define MAX_FAVORITES 13  // Max number of favorite frequencies stored in EEPROM
#define PRIMARY_MENU 0
#define SECONDARY_MENU 1
#define PRIMARY_MENU_X 0
#define SECONDARY_MENU_X 250
#define MENUS_Y 0
#define EACH_MENU_WIDTH 260
#define BOTH_MENU_WIDTHS (EACH_MENU_WIDTH * 2 + 30)
#define MENU_OPTION_SELECT 0  // These are the expected values from the switch ladder
#define MAIN_MENU_UP 1
#define BAND_UP 2
#define ZOOM 3
#define MAIN_MENU_DN 4
#define BAND_DN 5
#define FILTER 6
#define DEMODULATION 7
#define SET_MODE 8
#define NOISE_REDUCTION 9
#define NOTCH_FILTER 10
#define NOISE_FLOOR 11
#define FINE_TUNE_INCREMENT 12
#define DECODER_TOGGLE 13
#define MAIN_TUNE_INCREMENT 14
#define RESET_TUNING 15  // AFP 10-11-22
#define UNUSED_1 16      // AFP 10-11-22
#define BEARING 17       // AFP 10-11-22
//=======================================================
#define XPIXELS 800  // This is for the 5.0" display
#define YPIXELS 480
#define CHAR_HEIGHT 32
#define PIXELS_PER_EQUALIZER_DELTA 10  // Number of pixeks per detent of encoder for equalizer changes
#define SPECTRUM_LEFT_X 3  // Used to plot left edge of spectrum display  AFP 12-14-21
#define WATERFALL_LEFT_X SPECTRUM_LEFT_X
#define CLIP_AUDIO_PEAK 115                                     // The pixel value where audio peak overwrites S-meter
#define SPECTRUM_RES 512                                        // The value used in the original open-source code is 256.  Al uses 512.
#define SPECTRUM_TOP_Y 100                                      // Start of spectrum plot space
#define SPECTRUM_HEIGHT 150                                     // This is the pixel height of spectrum plot area without disturbing the axes
#define SPECTRUM_BOTTOM (SPECTRUM_TOP_Y + SPECTRUM_HEIGHT - 3)  // 247 = 100 + 150 - 3
#define AUDIO_SPECTRUM_TOP 129
#define AUDIO_SPECTRUM_BOTTOM SPECTRUM_BOTTOM
#define MAX_WATERFALL_WIDTH 512  // Pixel width of waterfall
#define MAX_WATERFALL_ROWS 170   // Waterfall rows
#define WATERFALL_RIGHT_X (WATERFALL_LEFT_X + MAX_WATERFALL_WIDTH)    // 3 + 512
#define WATERFALL_TOP_Y (SPECTRUM_TOP_Y + SPECTRUM_HEIGHT + 5)        // 130 + 120 + 5 = 255
#define FIRST_WATERFALL_LINE (WATERFALL_TOP_Y + 20)                   // 255 + 35 = 290
#define WATERFALL_BOTTOM (FIRST_WATERFALL_LINE + MAX_WATERFALL_ROWS)  // 290 + 170 = 460
#define TEMP_X_OFFSET 15
#define TEMP_Y_OFFSET 465  // 480 * 0.97 = 465
#define AGC_Y_OFFSET 292
#define AGC_X_OFFSET 680
#define INCREMENT_X WATERFALL_RIGHT_X + 25
#define INCREMENT_Y WATERFALL_TOP_Y + 70
#define INFORMATION_WINDOW_X WATERFALL_RIGHT_X + 25  // 512 + 25 = 537
#define INFORMATION_WINDOW_Y WATERFALL_TOP_Y + 37    // 255 + 37 = 292
#define BAND_INDICATOR_X WATERFALL_RIGHT_X + 25
#define BAND_INDICATOR_Y WATERFALL_TOP_Y + 37  // 292
#define OPERATION_STATS_X 130
#define X_R_STATUS_X 730
#define X_R_STATUS_Y 70
#define RECEIVE_STATE 1
#define TRANSMIT_STATE 0
#define SMETER_X WATERFALL_RIGHT_X + 16
#define SMETER_Y YPIXELS * 0.22  // 480 * 0.22 = 106
#define SMETER_BAR_HEIGHT 18
#define SMETER_BAR_LENGTH 180
#define SPECTRUM_NOISE_FLOOR (SPECTRUM_TOP_Y + SPECTRUM_HEIGHT - 3)
#define TIME_X (XPIXELS * 0.73)  // Upper-left corner for time
#define TIME_Y (YPIXELS * 0.07)
#define FILTER_PARAMETERS_X (XPIXELS * 0.22)
#define FILTER_PARAMETERS_Y (YPIXELS * 0.213)
#define DEFAULT_EQUALIZER_BAR 100  // Default equalizer bar height
#define FREQUENCY_X 5
#define FREQUENCY_Y 45
#define FREQUENCY_X_SPLIT 280
#define VFO_A 0
#define VFO_B 1
#define VFO_SPLIT 2
#define VFOA_PIXEL_LENGTH 275
#define VFOB_PIXEL_LENGTH 280
#define FREQUENCY_PIXEL_HI 45
#define SPLIT_INCREMENT 500L
//  Offsets for status info
#define FIELD_OFFSET_X WATERFALL_RIGHT_X + 118  // X coordinate for field
#define NOTCH_X WATERFALL_RIGHT_X + 58
#define NOTCH_Y WATERFALL_TOP_Y + 90
#define NOISE_REDUCE_X WATERFALL_RIGHT_X + 58
#define NOISE_REDUCE_Y WATERFALL_TOP_Y + 110
#define ZOOM_X WATERFALL_RIGHT_X + 65
#define ZOOM_Y WATERFALL_TOP_Y + 130
#define SD_X 707
#define SD_Y 385
#define COMPRESSION_X WATERFALL_RIGHT_X + 33
#define COMPRESSION_Y WATERFALL_TOP_Y + 150
#define DECODER_X WATERFALL_RIGHT_X + 43  // 512 +  43 = 555
#define DECODER_Y WATERFALL_TOP_Y + 190   // 255 + 190 = 345
#define WPM_X WATERFALL_RIGHT_X + 58
#define WPM_Y WATERFALL_TOP_Y + 170
#define SAM_PLL_HILBERT_STAGES 7              // AFP 11-02-22
#define MAX_DECODE_CHARS 32                   // Max chars that can appear on decoder line.  Increased to 32.  KF5N October 29, 2023
#define DECODER_BUFFER_SIZE 128               // Max chars in binary search string with , . ?
#define HISTOGRAM_ELEMENTS 750
#define LOWEST_ATOM_TIME 20                                   // 60WPM has an atom of 20ms
#define ADAPTIVE_SCALE_FACTOR 0.8                             // The amount of old histogram values are presesrved
#define SCALE_CONSTANT (1.0 / (1.0 - ADAPTIVE_SCALE_FACTOR))  // Insure array has enough observations to scale
#define MIN_AUDIO_VOLUME 16  //yours might be different. On my rig, this is where the band noise disappears.
#define AUDIO_POST_PROCESSOR_BANDS 8  // Number of audio segments
#define FLOAT_PRECISION 6    // Assumed precision for a float
#define EQUALIZER_CELL_COUNT 14
#define USE_LOG10FAST
#define TEMPMON_ROOMTEMP 25.0f
//--------------------- decoding stuff
#define FFT_LENGTH 512
#define BLACK 0x0000             /*   0,   0,   0 */
#define RA8875_BLUE 0x000F       /*   0,   0, 128 */
#define MAROON 0x7800            /* 128,   0,   0 */
#define RA8875_LIGHT_GREY 0xC618 /* 192, 192, 192 */
#define DARK_RED tft.Color565(64, 0, 0)
#define DARKGREY 0x7BEF           /* 128, 128, 128 */
#define BLUE 0x001F               /*   0,   0, 255 */
#define RA8875_GREEN 0x07E0       /*   0, 255,   0 */
#define CYAN 0x07FF               /*   0, 255, 255 */
#define RED 0xF800                /* 255,   0,   0 */
#define MAGENTA 0xF81F            /* 255,   0, 255 */
#define YELLOW 0xFFE0             /* 255, 255,   0 */
#define WHITE 0xFFFF              /* 255, 255, 255 */
#define ORANGE 0xFD20             /* 255, 165,   0 */
#define FILTER_WIN 0x10  // Color of SSB filter width
#include <utility/imxrt_hw.h>  // for setting I2S freq, Thanks, FrankB!
#define MAX_FREQ_INDEX 8
#define TEMPMON_ROOMTEMP 25.0f
#define MAX_WPM 60
#define ENCODER_FACTOR 0.25F  // use 0.25f with cheap encoders that have 4 detents per step,
                              // for other encoders or libs we use 1.0f
#define MAX_ZOOM_ENTRIES 5

//============== Pin Assignments =====================================
//============== Pins 0 and 1 are usually reserved for the USB COM port communications
//============== On the Teensy 4.1 board, pins GND, 0-12, and pins 13-23, 3.3V, GND, and
//============== Vin are "covered up" by the Audio board. However, not all of those pins are
//============== actually used by the board. See: https://www.pjrc.com/store/teensy3_audio.html
//========================================= Display pins
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_SCLK 13
#define TFT_RST 255
//========================================= Encoder pins  Jack Purdum W8TEE September 25, 2023
#ifdef FOURSQRP
#define VOLUME_ENCODER_A 2
#define VOLUME_ENCODER_B 3
#define FILTER_ENCODER_A 16
#define FILTER_ENCODER_B 15
#define FINETUNE_ENCODER_A 4
#define FINETUNE_ENCODER_B 5
#define TUNE_ENCODER_A 14
#define TUNE_ENCODER_B 17
#else
#define VOLUME_ENCODER_A 2
#define VOLUME_ENCODER_B 3
#define FILTER_ENCODER_A 15
#define FILTER_ENCODER_B 14
#define FINETUNE_ENCODER_A 4
#define FINETUNE_ENCODER_B 5
#define TUNE_ENCODER_A 16
#define TUNE_ENCODER_B 17
#endif

//========================================= Filter Board pins
#define FILTERPIN80M 30  // 80M filter relay
#define FILTERPIN40M 31  // 40M filter relay
#define FILTERPIN20M 28  // 20M filter relay
#define FILTERPIN15M 29  // 15M filter relay
#define RXTX 22          // Transmit/Receive
#define PTT 37           // Transmit/Receive
#define MUTE 38          // Mute Audio,  HIGH = "On" Audio available from Audio PA, LOW = Mute audio
//========================================= Switch pins
#define NO_MENUS_ACTIVE 0        // No menus displayed
#define PRIMARY_MENU_ACTIVE 1    // A primary menu is active
#define SECONDARY_MENU_ACTIVE 2  // Both primary and secondary menus active
//========================================= Keyer pins
#define KEYER_DAH_INPUT_RING 35  // Ring connection for keyer  -- default for righthanded user
#define KEYER_DIT_INPUT_TIP 36   // Tip connection for keyer
#define OPTO_OUTPUT 24  // To optoisolator and keyed circuit
#define STRAIGHT_KEY 0
#define KEYER 1
//========================================================= End Pin Assignments =================================
#define TMS0_POWER_DOWN_MASK (0x1U)
#define TMS1_MEASURE_FREQ(x) (((uint32_t)(((uint32_t)(x)) << 0U)) & 0xFFFFU)
#undef round
#undef PI
#undef HALF_PI
#undef TWO_PI
#define PI 3.1415926535897932384626433832795f
#define HALF_PI 1.5707963267948966192313216916398f
#define TWO_PI 6.283185307179586476925286766559f
#define PIH HALF_PI
#define FOURPI (2.0f * TWO_PI)
#define SIXPI (3.0f * TWO_PI)
#define Si_5351_crystal 25000000L
#define SSB_MODE 0
#define CW_MODE 1
#define SSB_RECEIVE 0
#define CW_RECEIVE 2
//  This second set of states are for the loop() modal state machine.
#define SSB_RECEIVE_STATE 0
#define SSB_TRANSMIT_STATE 1
#define CW_RECEIVE_STATE 2
#define CW_TRANSMIT_STRAIGHT_STATE 3
#define CW_TRANSMIT_KEYER_STATE 4
extern int radioState, lastState;  // Used by the loop to monitor current state.
#define DECODER_STATE 0  // 0 = off, 1 = on
#define DECODE_OFF 0
#define DECODE_ON 1
#define CW 1
#define SPECTRUM_ZOOM_1 0
#define SPECTRUM_ZOOM_2 1
#define SPECTRUM_ZOOM_4 2
#define SPECTRUM_ZOOM_8 3
#define SPECTRUM_ZOOM_16 4
#define SAMPLE_RATE_8K 0
#define SAMPLE_RATE_11K 1
#define SAMPLE_RATE_16K 2
#define SAMPLE_RATE_22K 3
#define SAMPLE_RATE_32K 4
#define SAMPLE_RATE_44K 5
#define SAMPLE_RATE_48K 6
#define SAMPLE_RATE_50K 7
#define SAMPLE_RATE_88K 8
#define SAMPLE_RATE_96K 9
#define SAMPLE_RATE_100K 10
#define SAMPLE_RATE_101K 11
#define SAMPLE_RATE_176K 12
#define SAMPLE_RATE_192K 13
#define SAMPLE_RATE_234K 14
#define SAMPLE_RATE_256K 15
#define SAMPLE_RATE_281K 16  // ??
#define SAMPLE_RATE_353K 17

#define TEMPMON_ROOMTEMP 25.0f

#define DEMOD_MIN 0
#define DEMOD_USB 0
#define DEMOD_LSB 1
#define DEMOD_AM 2
#define DEMOD_SAM 3
#define DEMOD_MAX 3  // AFP 11-03-22
#define DEMOD_IQ 4
#define HAM_BAND 1

#define BUFFER_SIZE 128

#define START_MENU 0
#define MENU_RF_GAIN 15
#define MENU_F_LO_CUT 40

// AGC
#define MAX_SAMPLE_RATE (24000.0)
#define MAX_N_TAU (8)
#define MAX_TAU_ATTACK (0.01)
#define RB_SIZE (int)(MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)

#define CW_TEXT_START_X 5
#define CW_TEXT_START_Y 449                   // 480 * 0.97 = 465 - height = 465 - 16 = 449
#define CW_MESSAGE_WIDTH MAX_WATERFALL_WIDTH  // 512
#define CW_MESSAGE_HEIGHT 16                  // tft.getFontHeight()

#define BAND_80M 0
#define BAND_40M 1
#define BAND_20M 2
#define BAND_17M 3
#define BAND_15M 4
#define BAND_12M 5
#define BAND_10M 6
#define NUMBER_OF_BANDS 7   //AFP 1-28-21

//------------------------- Global CW Filter declarations ----------

extern arm_biquad_cascade_df2T_instance_f32 S1_CW_Filter;
extern arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter1;  //AFP 10-18-22
extern arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter2;  //AFP 10-18-22
extern arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter3;  //AFP 10-18-22
extern arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter4;  //AFP 10-18-22
extern arm_biquad_cascade_df2T_instance_f32 S1_CW_AudioFilter5;  //AFP 10-18-22
extern float32_t CW_Filter_state[];
extern float32_t CW_AudioFilter1_state[];  //AFP 10-18-22
extern float32_t CW_AudioFilter2_state[];  //AFP 10-18-22
extern float32_t CW_AudioFilter3_state[];  //AFP 10-18-22
extern float32_t CW_AudioFilter4_state[];  //AFP 10-18-22
extern float32_t CW_AudioFilter5_state[];  //AFP 10-18-22
extern float32_t HP_DC_Filter_Coeffs[];

extern float32_t CW_AudioFilterCoeffs1[];  //AFP 10-18-22
extern float32_t CW_AudioFilterCoeffs2[];  //AFP 10-18-22
extern float32_t CW_AudioFilterCoeffs3[];  //AFP 10-18-22
extern float32_t CW_AudioFilterCoeffs4[];  //AFP 10-18-22
extern float32_t CW_AudioFilterCoeffs5[];  //AFP 10-18-22

#define IIR_CW_NUMSTAGES 4
extern float32_t CW_Filter_Coeffs[];
extern float32_t HP_DC_Filter_Coeffs[];
extern float32_t HP_DC_Filter_Coeffs2[];  // AFP 11-02-22
//=== end CW Filter ===

#define DISPLAY_S_METER_DBM 0
#define DISPLAY_S_METER_DBMHZ 1
#define N2 100

#define ANR_DLINE_SIZE 512  //funktioniert nicht, 128 & 256 OK
#define MAX_LMS_TAPS 96
#define MAX_LMS_DELAY 256
#define NR_FFT_L 256
#define NB_FFT_SIZE FFT_LENGTH / 2
#define TABLE_SIZE_64 64
#define EEPROM_BASE_ADDRESS 0U

//================== Global CW Correlation and FFT Variables =================
extern float32_t audioMaxSquaredAve;
extern float32_t corrResult;      //AFP 02-02-22
extern uint32_t corrResultIndex;  //AFP 02-02-22
extern float32_t sinBuffer[];     //AFP 02-02-22
extern float32_t sinBuffer2[];
extern float32_t cwRiseBuffer[];
extern float32_t cwFallBuffer[];
extern float32_t float_Corr_Buffer[];  //AFP 02-02-22
extern float32_t aveCorrResult;        //AFP 02-02-22
extern int filterWidth;
extern int filterWidthX;  // The current filter X.
extern float goertzelMagnitude;
extern boolean use_HP_filter;           //enable the software HP filter to get rid of DC?
extern float knee_dBFS;
extern float comp_ratio;
extern float attack_sec;
extern float release_sec;
extern float32_t corrResultR;           //AFP 02-02-22
extern uint32_t corrResultIndexR;       //AFP 02-02-22
extern float32_t corrResultL;           //AFP 02-02-22
extern uint32_t corrResultIndexL;       //AFP 02-02-22
extern float32_t aveCorrResult;         //AFP 02-02-22
extern float32_t aveCorrResultR;        //AFP 02-06-22
extern float32_t aveCorrResultL;        //AFP 02-06-22
extern float32_t float_Corr_BufferR[];  //AFP 02-06-22
extern float32_t float_Corr_BufferL[];  //AFP 02-06-22
extern float32_t combinedCoeff;         //AFP 02-06-22
extern float CWLevelTimer;
extern float CWLevelTimerOld;
extern float32_t combinedCoeff2;
extern float32_t combinedCoeff2Old;
extern long CWFreqShift;  //AFP 05-17-22
//===== New histogram stuff
extern int endGapFlag;
extern int topGapIndex;
extern int topGapIndexOld;
extern int32_t signalHistogram[];
extern int32_t gapHistogram[];
extern float32_t pixel_per_khz;  //AFP
extern int pos_left;
extern int centerLine;
extern int filterWidth;
extern int h;
extern int centerTuneFlag;
extern long valRef1;
extern long valRef2;
extern long gapRef1;
extern int valFlag;
extern long signalStartOld;
extern long aveDitLength;
extern long aveDahLength;
extern float thresholdGeometricMean;
extern float thresholdArithmeticMean;
extern long notchFreq;
extern long filter_pos;
extern long last_filter_pos;

// ============ end new stuff =======

//================== Global Excite Variables =================

#define IIR_ORDER 8
#define IIR_NUMSTAGES (IIR_ORDER / 2)

struct config_t {

  char versionSettings[10] = VERSION;  // This is required to be the first!  See EEPROMRead() function.
  int AGCMode = 1;
  int audioVolume = 30;  // 4 bytes
  int rfGainCurrent = 0;
  int rfGain[NUMBER_OF_BANDS] = {0};
  bool autoGain = false;
  int spectrumNoiseFloor = SPECTRUM_NOISE_FLOOR;  // AFP 09-26-22
  int tuneIndex = DEFAULTFREQINCREMENT;           // JJP 7-3-23
  long stepFineTune = FINE_TUNE_STEP;             // JJP 7-3-23
  int transmitPowerLevel = DEFAULT_POWER_LEVEL;   // JJP 7-3-23
  int xmtMode = SSB_MODE;                         // AFP 09-26-22
  int nrOptionSelect = 0;                         // 1 byte
  int currentScale = 1;
  long spectrum_zoom = SPECTRUM_ZOOM_2;
  float spectrum_display_scale = 10.0;  // 4 bytes
  int CWFilterIndex = 5;                // Off
  int paddleDit = KEYER_DIT_INPUT_TIP;
  int paddleDah = KEYER_DAH_INPUT_RING;
  int decoderFlag = DECODER_STATE;        // JJP 7-3-23
  int keyType = STRAIGHT_KEY_OR_PADDLES;  // straight key = 0, keyer = 1  JJP 7-3-23
  int currentWPM = DEFAULT_KEYER_WPM;     // 4 bytes default = 15 JJP 7-3-23
  int CWOffset = 2;                       //
  float32_t sidetoneVolume = 30.0;        // 4 bytes
  uint32_t cwTransmitDelay = 1000;
  int activeVFO = 0;                // 2 bytes
  int freqIncrement = 10000;        // 4 bytes
  int currentBand = STARTUP_BAND;   // 4 bytes   JJP 7-3-23
  int currentBandA = STARTUP_BAND;  // 4 bytes   JJP 7-3-23
  int currentBandB = STARTUP_BAND;  // 4 bytes   JJP 7-3-23

  //DB2OO, 23-AUG-23 7.1MHz for Region 1
#if defined(ITU_REGION) && ITU_REGION == 1
  int currentFreqA = 7100000;
#else
  int currentFreqA = 7200000;
#endif
  int currentFreqB = 7030000;
  //DB2OO, 23-AUG-23: with TCXO needs to be 0
#ifdef TCXO_25MHZ
  int freqCorrectionFactor = 0;  //68000;
#else
  //Conventional crystal with freq offset needs a correction factor
  int freqCorrectionFactor = 68000;
#endif

  int equalizerRec[EQUALIZER_CELL_COUNT] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
  int equalizerXmt[EQUALIZER_CELL_COUNT] = { 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 0, 0 };  // Provide equalizer optimized for SSB voice based on Neville's tests.  KF5N November 2, 2023
  int currentMicThreshold = -10;                                                                              // 4 bytes       AFP 09-22-22
  float currentMicCompRatio = 8.0;
  float currentMicAttack = 0.1;
  float currentMicRelease = 0.1;
  int currentMicGain = 20;
  int switchValues[18] = { 924, 870, 817,
                           769, 713, 669,
                           616, 565, 513,
                           459, 407, 356,
                           298, 242, 183,
                           131, 67, 10 };
  float LPFcoeff = 0.0;     // 4 bytes
  float NR_PSI = 0.0;       // 4 bytes
  float NR_alpha = 0.95;    // 4 bytes
  float NR_beta = 0.85;     // 4 bytes
  float omegaN = 200.0;     // 4 bytes
  float pll_fmax = 4000.0;  // 4 bytes
  float powerOutCW[NUMBER_OF_BANDS] = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };
  float powerOutSSB[NUMBER_OF_BANDS] = { 0.03, 0.03, 0.03, 0.03, 0.03, 0.03, 0.03 };
  float CWPowerCalibrationFactor[NUMBER_OF_BANDS] = { 0.019, 0.019, .0190, .019, .019, .019, .019 };       // 0.019;
  float SSBPowerCalibrationFactor[NUMBER_OF_BANDS] = { 0.008, 0.008, 0.008, 0.008, 0.008, 0.008, 0.008 };  // 0.008
  float IQAmpCorrectionFactor[NUMBER_OF_BANDS] = { 1, 1, 1, 1, 1, 1, 1 };
  float IQPhaseCorrectionFactor[NUMBER_OF_BANDS] = { 0, 0, 0, 0, 0, 0, 0 };
  float IQXAmpCorrectionFactor[NUMBER_OF_BANDS] = { 1, 1, 1, 1, 1, 1, 1 };
  float IQXPhaseCorrectionFactor[NUMBER_OF_BANDS] = { 0, 0, 0, 0, 0, 0, 0 };
  long favoriteFreqs[13] = { 3560000, 3690000, 7030000, 7200000, 14060000, 14200000, 21060000, 21285000, 28060000, 28365000, 5000000, 10000000, 15000000 };

  //DB2OO, 23-AUG-23: Region 1 freqs (from https://qrper.com/qrp-calling-frequencies/)
#if defined(ITU_REGION) && ITU_REGION == 1
  int lastFrequencies[NUMBER_OF_BANDS][2] = { { 3690000, 3560000 }, { 7090000, 7030000 }, { 14285000, 14060000 }, { 18130000, 18096000 }, { 21285000, 21060000 }, { 24950000, 24906000 }, { 28365000, 28060000 } };
#else
  int lastFrequencies[NUMBER_OF_BANDS][2] = { { 3985000, 3560000 }, { 7200000, 7030000 }, { 14285000, 14060000 }, { 18130000, 18096000 }, { 21385000, 21060000 }, { 24950000, 24906000 }, { 28385000, 28060000 } };
#endif

  long centerFreq = 7030000L;  // 4 bytes
  // New user config data                                JJP 7-3-23
  char mapFileName[50] = MAP_FILE_NAME;
//  char myCall[10] = MY_CALL;
  char myTimeZone[10] = MY_TIMEZONE;
  int separationCharacter = (int)'.';  // JJP 7/25/23
  int paddleFlip = PADDLE_FLIP;        // 0 = right paddle = DAH, 1 = DIT
  int sdCardPresent = 0;               //   JJP  7/18/23
  float myLong = MY_LON;
  float myLat = MY_LAT;
  int currentNoiseFloor[NUMBER_OF_BANDS] = { 0, 0, 0, 0, 0, 0, 0 };  // JJP 7/17/23
  int compressorFlag = 0;                                            // JJP 8/28/23
  bool xmitEQFlag = false;
  bool receiveEQFlag = false;
  int calFreq = 1;                    // This is an index into an array of tone frequencies, for example:  {750, 3000}
  int buttonThresholdPressed = 944;   // switchValues[0] + WIGGLE_ROOM
  int buttonThresholdReleased = 964;  // buttonThresholdPressed + WIGGLE_ROOM
  int buttonRepeatDelay = 300000;     // Increased to 300000 from 200000 to better handle cheap, wornout buttons.
};

extern struct config_t EEPROMData;
extern config_t defaultConfig;
extern config_t EEPROMData_temp;
extern float32_t HP_DC_Butter_state2[2];                  //AFP 11-04-22
extern float32_t HP_DC_Butter_state[6];                   //AFP 09-23-22
extern arm_biquad_cascade_df2T_instance_f32 s1_Receive;   //AFP 09-23-22
extern arm_biquad_cascade_df2T_instance_f32 s1_Receive2;  //AFP 11-02-22
extern float32_t coeffs192K_10K_LPF_FIR[];
extern float32_t coeffs48K_8K_LPF_FIR[];
extern const uint32_t N_B_EX;
extern float32_t EQ_Band1Coeffs[];
extern float32_t EQ_Band2Coeffs[];
extern float32_t EQ_Band3Coeffs[];
extern float32_t EQ_Band4Coeffs[];
extern float32_t EQ_Band5Coeffs[];
extern float32_t EQ_Band6Coeffs[];
extern float32_t EQ_Band7Coeffs[];
extern float32_t EQ_Band8Coeffs[];
extern float32_t EQ_Band9Coeffs[];
extern float32_t EQ_Band10Coeffs[];
extern float32_t EQ_Band11Coeffs[];
extern float32_t EQ_Band12Coeffs[];
extern float32_t EQ_Band13Coeffs[];
extern float32_t EQ_Band14Coeffs[];

//Setup for EQ filters
extern float32_t rec_EQ_Band1_state[];
extern float32_t rec_EQ_Band2_state[];
extern float32_t rec_EQ_Band3_state[];
extern float32_t rec_EQ_Band4_state[];
extern float32_t rec_EQ_Band5_state[];
extern float32_t rec_EQ_Band6_state[];
extern float32_t rec_EQ_Band7_state[];
extern float32_t rec_EQ_Band8_state[];
extern float32_t rec_EQ_Band9_state[];
extern float32_t rec_EQ_Band10_state[];
extern float32_t rec_EQ_Band11_state[];
extern float32_t rec_EQ_Band12_state[];
extern float32_t rec_EQ_Band13_state[];
extern float32_t rec_EQ_Band14_state[];

extern float32_t rec_EQ1_float_buffer_L[];
extern float32_t rec_EQ2_float_buffer_L[];
extern float32_t rec_EQ3_float_buffer_L[];
extern float32_t rec_EQ4_float_buffer_L[];
extern float32_t rec_EQ5_float_buffer_L[];
extern float32_t rec_EQ6_float_buffer_L[];
extern float32_t rec_EQ7_float_buffer_L[];
extern float32_t rec_EQ8_float_buffer_L[];
extern float32_t rec_EQ9_float_buffer_L[];
extern float32_t rec_EQ10_float_buffer_L[];
extern float32_t rec_EQ11_float_buffer_L[];
extern float32_t rec_EQ12_float_buffer_L[];
extern float32_t rec_EQ13_float_buffer_L[];
extern float32_t rec_EQ14_float_buffer_L[];

extern float32_t FIR_Hilbert_coeffs90[];
extern float32_t FIR_Hilbert_coeffs0[];

extern float32_t EQ_Band1_state[];
extern float32_t EQ_Band2_state[];
extern float32_t EQ_Band3_state[];
extern float32_t EQ_Band4_state[];
extern float32_t EQ_Band5_state[];
extern float32_t EQ_Band6_state[];
extern float32_t EQ_Band7_state[];
extern float32_t EQ_Band8_state[];
extern float32_t EQ_Band9_state[];
extern float32_t EQ_Band10_state[];
extern float32_t EQ_Band11_state[];
extern float32_t EQ_Band12_state[];
extern float32_t EQ_Band13_state[];
extern float32_t EQ_Band14_state[];

//EQ filter instances
extern arm_biquad_cascade_df2T_instance_f32 S1_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S2_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S3_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S4_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S5_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S6_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S7_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S8_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S9_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S10_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S11_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S12_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S13_Rec;
extern arm_biquad_cascade_df2T_instance_f32 S14_Rec;

// ================= start  AFP 10-02-22 ===========
extern arm_biquad_cascade_df2T_instance_f32 S1_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S2_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S3_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S4_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S5_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S6_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S7_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S8_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S9_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S10_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S11_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S12_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S13_Xmt;
extern arm_biquad_cascade_df2T_instance_f32 S14_Xmt;

extern float32_t xmt_EQ1_float_buffer_L[];
extern float32_t xmt_EQ2_float_buffer_L[];
extern float32_t xmt_EQ3_float_buffer_L[];
extern float32_t xmt_EQ4_float_buffer_L[];
extern float32_t xmt_EQ5_float_buffer_L[];
extern float32_t xmt_EQ6_float_buffer_L[];
extern float32_t xmt_EQ7_float_buffer_L[];
extern float32_t xmt_EQ8_float_buffer_L[];
extern float32_t xmt_EQ9_float_buffer_L[];
extern float32_t xmt_EQ10_float_buffer_L[];
extern float32_t xmt_EQ11_float_buffer_L[];
extern float32_t xmt_EQ12_float_buffer_L[];
extern float32_t xmt_EQ13_float_buffer_L[];
extern float32_t xmt_EQ14_float_buffer_L[];

// ================= end  AFP 10-02-22 ===========
//Hilbert FIR Filter

extern float32_t FIR_Hilbert_state_L[];
extern float32_t FIR_Hilbert_state_R[];

extern float32_t FIR_Hilbert_coeffs_45[];     //AFP 01-16-22
extern float32_t FIR_Hilbert_coeffs_neg45[];  //AFP 01-16-22

extern arm_fir_instance_f32 FIR_Hilbert_L;
extern arm_fir_instance_f32 FIR_Hilbert_R;

extern float32_t CW_Filter_Coeffs2[];        //AFP 10-25-22
extern arm_fir_instance_f32 FIR_CW_DecodeL;  //AFP 10-25-22
extern arm_fir_instance_f32 FIR_CW_DecodeR;  //AFP 10-25-22
extern float32_t FIR_CW_DecodeL_state[];     //AFP 10-25-22
extern float32_t FIR_CW_DecodeR_state[];     //AFP 10-25-22

extern arm_fir_decimate_instance_f32 FIR_dec1_EX_I;
extern arm_fir_decimate_instance_f32 FIR_dec1_EX_Q;
extern arm_fir_decimate_instance_f32 FIR_dec2_EX_I;
extern arm_fir_decimate_instance_f32 FIR_dec2_EX_Q;

extern arm_fir_interpolate_instance_f32 FIR_int1_EX_I;
extern arm_fir_interpolate_instance_f32 FIR_int1_EX_Q;
extern arm_fir_interpolate_instance_f32 FIR_int2_EX_I;
extern arm_fir_interpolate_instance_f32 FIR_int2_EX_Q;

extern float32_t FIR_dec1_EX_I_state[];  //48 + (uint16_t) BUFFER_SIZE * (uint32_t) N_B - 1
extern float32_t FIR_dec1_EX_Q_state[];
extern float32_t FIR_dec2_EX_I_state[];  //DEC2STATESIZE
extern float32_t FIR_dec2_EX_Q_state[];
extern float32_t FIR_int2_EX_I_state[];
extern float32_t FIR_int2_EX_Q_state[];
extern float32_t FIR_int1_EX_I_state[];
extern float32_t FIR_int1_EX_Q_state[];

extern float32_t float_buffer_L_EX[];
extern float32_t float_buffer_R_EX[];
extern float32_t float_buffer_LTemp[];
extern float32_t float_buffer_RTemp[];

void ExciterIQData();

//==================== End Excite Variables ================================

//======================================== Global object declarations ==================================================

extern long NCOFreq;  // AFP 04-16-22
extern float32_t NCO_INC;  // AFP 04-16-22
extern double OSC_COS;
extern double OSC_SIN;
extern double Osc_Vect_Q;
extern double Osc_Vect_I;
extern double Osc_Gain;
extern double Osc_Q;
extern double Osc_I;

//======================================== Global object declarations ==================================================

extern AudioMixer4 modeSelectInR;
extern AudioMixer4 modeSelectInL;
extern AudioMixer4 modeSelectInExR;
extern AudioMixer4 modeSelectInExL;

extern AudioMixer4 modeSelectOutL;
extern AudioMixer4 modeSelectOutR;
extern AudioMixer4 modeSelectOutExL;
extern AudioMixer4 modeSelectOutExR;

extern AudioRecordQueue Q_in_L;
extern AudioRecordQueue Q_in_R;
extern AudioRecordQueue Q_in_L_Ex;
extern AudioRecordQueue Q_in_R_Ex;

extern AudioPlayQueue Q_out_L;
extern AudioPlayQueue Q_out_R;
extern AudioPlayQueue Q_out_L_Ex;
extern AudioPlayQueue Q_out_R_Ex;

// = AFP 11-01-22
extern AudioControlSGTL5000_Extended sgtl5000_1;      //controller for the Teensy Audio Board
extern AudioConvert_I16toF32 int2Float1, int2Float2;  //Converts Int16 to Float.  See class in AudioStream_F32.h
extern AudioEffectCompressor_F32 comp1, comp2;
extern AudioConvert_F32toI16 float2Int1, float2Int2;  //Converts Float to Int16.  See class in AudioStream_F32.h
//===============  AFP 11-01-22

extern void SetAudioOperatingState(int operatingState);

extern Rotary volumeEncoder;    // (2,  3)
extern Rotary tuneEncoder;      // (16, 17)
extern Rotary filterEncoder;    // (14, 15)
extern Rotary fineTuneEncoder;  // (4,  5);

extern Metro ms_500;

extern Si5351 si5351;

extern RA8875 tft;

//======================================== Global structure declarations ===============================================

struct secondaryMenuConfiguration {
  byte whichType;       // 0 = no options, 1 = list, 2 = encoder value
  int numberOfOptions;  // Number of submenu topions
};

// JSON file configuration related variables:
extern const char *filename;
extern void loadConfiguration(const char *filename, config_t &config);
extern void saveConfiguration(const char *filename, const config_t &config, bool toFile);

typedef struct SR_Descriptor {
  const uint8_t SR_n;
  const uint32_t rate;
  const char *const text;
  const char *const f1;
  const char *const f2;
  const char *const f3;
  const char *const f4;
  const float32_t x_factor;
  const uint8_t x_offset;
} SR_Desc;
extern const struct SR_Descriptor SR[];

extern const arm_cfft_instance_f32 *S;
extern const arm_cfft_instance_f32 *iS;
extern const arm_cfft_instance_f32 *maskS;
extern const arm_cfft_instance_f32 *NR_FFT;
extern const arm_cfft_instance_f32 *NR_iFFT;
extern const arm_cfft_instance_f32 *spec_FFT;

extern arm_biquad_casd_df1_inst_f32 biquad_lowpass1;
extern arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I;
extern arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q;

extern arm_fir_decimate_instance_f32 FIR_dec1_I;
extern arm_fir_decimate_instance_f32 FIR_dec1_Q;
extern arm_fir_decimate_instance_f32 FIR_dec2_I;
extern arm_fir_decimate_instance_f32 FIR_dec2_Q;
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I;
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q;
extern arm_fir_interpolate_instance_f32 FIR_int1_I;
extern arm_fir_interpolate_instance_f32 FIR_int1_Q;
extern arm_fir_interpolate_instance_f32 FIR_int2_I;
extern arm_fir_interpolate_instance_f32 FIR_int2_Q;
extern arm_lms_norm_instance_f32 LMS_Norm_instance;
extern elapsedMicros usec;

struct band {
  long freq;         // Current frequency in Hz * 100
  long fBandLow;     // Lower band edge
  long fBandHigh;    // Upper band edge
  const char *name;  // name of band
  int mode;
  int FHiCut;
  int FLoCut;
  int RFgain;
  uint8_t band_type;
  float32_t gainCorrection;  // is hardware dependent and has to be calibrated ONCE and hardcoded in the table below
  int AGC_thresh;
  int16_t pixel_offset;
};
extern struct band bands[];

typedef struct DEMOD_Descriptor {
  const uint8_t DEMOD_n;
  const char *const text;
} DEMOD_Desc;
extern const DEMOD_Descriptor DEMOD[];

struct dispSc {
  const char *dbText;
  float32_t dBScale;
  uint16_t pixelsPerDB;
  uint16_t baseOffset;
  float32_t offsetIncrement;
};

extern struct dispSc displayScale[];

typedef struct Menu_Descriptor {
  const uint8_t no;         // Menu ID
  const char *const text1;  // upper text
  const char *text2;        // lower text
  const uint8_t menu2;      // 0 = belongs to Menu, 1 = belongs to Menu2
} Menu_D;
extern Menu_D Menus[];

//======================================== Global variables declarations ===============================================
//========================== Some are not in alpha order because of forward references =================================

extern bool save_last_frequency;
extern bool volumeChangeFlag;

extern char decodeBuffer[];
extern const char DEGREE_SYMBOL[];
extern char keyboardBuffer[];
extern const char *labels[];

extern const char *topMenus[];
extern const char *zoomOptions[];

extern byte currentDashJump;
extern byte currentDecoderIndex;

extern int8_t first_block;
extern int8_t Menu2;
extern int8_t menuStatus;  // 0 = no primary or secondary menu, 1 = primary, 2 = secondary

extern int8_t NB_taps;
extern int8_t NB_impulse_samples;

extern uint8_t agc_action;
extern uint8_t ANR_notch;
extern uint8_t ANR_notchOn;
extern uint8_t auto_codec_gain;
extern uint8_t decay_type;
extern uint8_t display_dbm;
extern uint8_t display_S_meter_or_spectrum_state;
extern uint8_t FIR_filter_window;
extern uint8_t half_clip;
extern uint8_t hang_enable;
extern uint8_t keyPressedOn;  //AFP 09-01-22
extern uint8_t NB_on;
extern uint8_t NB_test;
extern uint8_t NR_first_time;
extern uint8_t NR_Kim;
extern uint8_t NR_use_X;
extern uint8_t quarter_clip;
extern uint8_t SampleRate;
extern uint8_t sch;
extern uint8_t state;
extern uint8_t T41State;
extern uint8_t twinpeaks_tested;  // initial value --> 2 !!
extern uint8_t wait_flag;
extern uint8_t write_analog_gain;
extern uint8_t zoom_display;

extern const uint8_t NR_L_frames;
extern const uint8_t NR_N_frames;

extern int16_t currentMode;

extern int16_t pixelnew[];
extern int16_t pixelold[];

extern int16_t pixelCurrent[];
extern int16_t pixelold[];

extern int16_t pos_y_frequency;
extern int16_t pos_x_time;  // 14;
extern int16_t pos_y_time;
extern int16_t *sp_L1;
extern int16_t *sp_R1;
extern int16_t *sp_L2;
extern int16_t *sp_R2;
extern int16_t spectrum_height;
extern int16_t spectrum_pos_centre_f;
extern int16_t spectrum_x;
extern int16_t spectrum_y;
extern int16_t y_old, y_new, y1_new, y1_old, y_old2;
extern uint16_t base_y;
extern uint16_t barGraphUpdate;
extern const uint16_t gradient[];
extern const uint32_t IIR_biquad_Zoom_FFT_N_stages;
extern const uint32_t N_stages_biquad_lowpass1;
extern const uint16_t n_dec1_taps;
extern const uint16_t n_dec2_taps;
extern float adjustVolEncoder;  //AFP 09-21-22
extern int adjustIQ;            //AFP 09-21-22
extern int encoderStepOld;
extern int resultOldFactor;
extern float incrFactor;
extern int mute;
extern int exciteOn;
extern int agc_decay;
extern int agc_slope;
extern int agc_thresh;
extern int ANR_buff_size;
extern int ANR_delay;
extern int ANR_dline_size;
extern int ANR_in_idx;
extern int ANR_mask;
extern int ANR_position;
extern int ANR_taps;
extern int attenuator;
extern int attack_buffsize;
extern int audioYPixel[];
extern int bandswitchPins[];
extern int button9State;
extern int buttonRead;
extern int calibrateFlag;
extern int chipSelect;
extern int countryIndex;
extern int dahLength;
extern int dcfCount;
extern int dcfLevel;
extern int dcfSilenceTimer;
extern int dcfTheSecond;
extern int dcfPulseTime;
extern int decoderFlag;
extern int demodIndex;
extern int directFreqFlag;
extern int EEPROMChoice;
extern int equalizerRecChoice;
extern int equalizerXmtChoice;
extern int fLoCutOld;
extern int fHiCutOld;
extern int fastTuneActive;
extern volatile int filterEncoderMove;
extern volatile long fineTuneEncoderMove;
extern int filterLoPositionMarkerOld;  // AFP 03-27-22 Layers
extern int filterHiPositionMarkerOld;  // AFP 03-27-22 Layers

extern int freqIncrement;
extern int FLoCutOld;
extern int FHiCutOld;
extern int (*functionPtr[])();
extern int gapAtom;  //Space between atoms
extern int gapChar;  // Space between characters
extern int hang_counter;
extern int helpmin;
extern int helphour;
extern int helpday;
extern int helpmonth;
extern int helpyear;
extern int helpsec;
extern int idx, idpk;
extern int IQChoice;
extern int IQCalType;
extern int IQEXChoice;
extern int lidx, hidx;
extern int keyType;
extern int LMS_nr_strength;
extern int LP_F_help;
extern int mainTuneEncoder;
extern int micChoice;
extern int micGainChoice;
extern int minPinRead;
extern bool buttonInterruptsEnabled;
extern int NR_Filter_Value;
extern int operatingMode;
extern int n_L;
extern int n_R;
extern int n_tau;
extern int NBChoice;
extern int newCursorPosition;
extern int NR_Choice;
extern int NR_KIM_K_int;
extern int NR_VAD_delay;
extern int NR_VAD_duration;
extern int NR_Index;
extern int old_demod_mode;
extern int oldnotchF;
extern int oldCursorPosition;
extern int out_index;
extern int paddleFlip;
extern int pmode;
extern int pos_centre_f;
extern int pos_x_frequency;
extern int pos_y_smeter;
extern int resetTuningFlag;  // Experimental flag for ResetTuning() due to possible timing issues.  KF5N July 31, 2023
extern int secondaryMenuChoiceMade;
extern int selectedMapIndex;
extern int smeterLength;
extern int splitOn;
extern int switchFilterSideband;  //AFP 1-28-21
extern int syncEEPROM;
extern int termCursorXpos;
extern int x2;  //AFP
extern int xrState;
extern int zeta_help;
extern int zoom_sample_ptr;
extern int zoomIndex;

extern int updateDisplayFlag;
extern int updateDisplayCounter;

extern const int DEC2STATESIZE;
extern const int INT1_STATE_SIZE;
extern const int INT2_STATE_SIZE;
extern const int myInput;
extern const int pos_x_smeter;
extern const int waterfallBottom;
extern const int waterfallTop;
extern int wtf;

extern unsigned framesize;
extern unsigned nbits;
extern unsigned ring_buffsize;
extern unsigned tcr5;
extern unsigned tcr2div;

extern int32_t FFT_shift;
extern long long freqCorrectionFactor;
extern long long freqCorrectionFactorOld;  //AFP 09-21-22

extern int32_t IFFreq;  // IF (intermediate) frequency
extern int32_t IF_FREQ1;
extern int32_t O_iiSum19;
extern int32_t O_integrateCount19;
extern int32_t mainMenuIndex;
extern int32_t subMenuMaxOptions;
extern int32_t secondaryMenuIndex;  // -1 means haven't determined secondary menu
extern const uint32_t N_B;
extern const uint32_t N_DEC_B;
extern const uint32_t NR_add_counter;

extern uint32_t BUF_N_DF;
extern uint32_t FFT_length;
extern int32_t gapHistogram[];
extern uint32_t in_index;
extern uint32_t IQ_counter;
extern uint32_t MDR;
extern uint32_t N_BLOCKS;
extern uint32_t n_para;
extern uint32_t NR_X_pointer;
extern uint32_t NR_E_pointer;
extern uint32_t m_NumTaps;
extern uint32_t roomCount;    /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at the hot temperature.*/
extern uint32_t s_hotTemp;    /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at room temperature .*/
extern uint32_t s_hotCount;   /*!< The value of TEMPMON_TEMPSENSE0[TEMP_VALUE] at the hot temperature.*/
extern uint32_t s_roomC_hotC; /*!< The value of s_roomCount minus s_hotCount.*/
extern uint32_t T4_CPU_FREQUENCY;
extern uint32_t twinpeaks_counter;

extern long averageDit;
extern long averageDah;
extern long currentFreq;
extern unsigned long ditLength;
extern unsigned long transmitDitLength;  // JJP 8/19/23
extern unsigned long transmitDitUnshapedBlocks;
extern unsigned long transmitDahUnshapedBlocks;
extern long TxRxFreq;  // = centerFreq+NCOFreq  NCOFreq from FreqShift2()
extern long TxRxFreqOld;
extern long TxRxFreqDE;
extern long recClockFreq;  //  = TxRxFreq+IFFreq  IFFreq from FreqShift1()=48KHz
extern long CWRecFreq;     //  = TxRxFreq +/- 700Hz
extern uint32_t cwTimer;
extern long signalTime;
extern unsigned long ditTimerOn;
extern long DahTimer;
extern unsigned long cwTransmitDelay;  // ms to keep relay on after last atom read
extern long incrementValues[];
extern long lastFrequencies[][2];
extern long notchCenterBin;
extern long int n_clear;
extern long startTime;
extern long signalElapsedTime;
extern long spaceSpan;
extern long signalStart;
extern long signalEnd;  // Start-end of dit or dah
extern long spaceStart;
extern long spaceEnd;
extern long spaceElapsedTime;

extern uint32_t gapLength;
extern long gapEnd, gapStart;  // Time for noise measures

// This enum is used by an experimental Morse decoder.
enum states { state0,
              state1,
              state2,
              state3,
              state4,
              state5,
              state6 };
extern states decodeStates;

extern ulong samp_ptr;
extern unsigned long currentFreqs[];

extern uint64_t output12khz;

extern unsigned long long Clk2SetFreq;  // AFP 09-27-22
extern unsigned long long Clk1SetFreq;  // AFP 09-27-22

float32_t arm_atan2_f32(float32_t y, float32_t x);
float ApproxAtan(float z);
float ApproxAtan2(float y, float x);

extern float dcfRefLevel;
extern float DD4WH_RF_gain;
extern float s_hotT_ROOM; /*!< The value of s_hotTemp minus room temperature(25¡æ).*/

//====== SAM stuff AFP 11-02-22
extern float32_t a[3 * SAM_PLL_HILBERT_STAGES + 3];
extern float32_t b[3 * SAM_PLL_HILBERT_STAGES + 3];
extern float32_t c[3 * SAM_PLL_HILBERT_STAGES + 3];  // Filter c variables
extern float32_t c0[SAM_PLL_HILBERT_STAGES];
extern float32_t c1[SAM_PLL_HILBERT_STAGES];
extern float32_t d[3 * SAM_PLL_HILBERT_STAGES + 3];

extern float32_t a[];
extern float32_t abs_ring[];
extern float32_t abs_out_sample;
extern float32_t ai, bi, aq, bq;
extern float32_t ai_ps, bi_ps, aq_ps, bq_ps;
extern float32_t pll_fmax;

extern float32_t ANR_d[];
extern float32_t ANR_den_mult;
extern float32_t ANR_gamma;
extern float32_t ANR_lidx;
extern float32_t ANR_lidx_min;
extern float32_t ANR_lidx_max;
extern float32_t ANR_lincr;
extern float32_t ANR_ldecr;
extern float32_t ANR_ngamma;
extern float32_t ANR_two_mu;
extern float32_t ANR_w[];
extern float32_t attack_mult;
extern float32_t audio;
extern float32_t audiotmp;
extern float32_t audiou;
extern float32_t audioSpectBuffer[];
extern float32_t b[];
extern float32_t bass;
extern float32_t bin_BW;
extern float32_t bin;
extern float32_t biquad_lowpass1_state[];
extern float32_t biquad_lowpass1_coeffs[];
extern float32_t /*DMAMEM*/ buffer_spec_FFT[];
extern float32_t c[];
extern float32_t c0[];
extern float32_t c1[];
extern float32_t coefficient_set[];
extern float32_t corr[];
extern float32_t Cos;
extern float32_t cosBuffer2[];  //AFP 08-18-22
extern float32_t CPU_temperature;
extern float32_t cursorIncrementFraction;
extern float32_t d[];

extern float32_t dbm;
extern float32_t dbm_calibration;
extern float32_t dbm_old;
extern float32_t dbmhz;
extern float32_t decay_mult;
extern float32_t display_offset;
extern float32_t /*DMAMEM*/ FFT_buffer[];
extern float32_t /*DMAMEM*/ FFT_spec[];
extern float32_t /*DMAMEM*/ FFT_spec_old[];
extern float32_t dsI;
extern float32_t dsQ;
extern float32_t fast_backaverage;
extern float32_t fast_backmult;
extern float32_t fast_decay_mult;
extern float32_t farnsworthValue;
extern float32_t FFT_spec[];
extern float32_t FFT_spec_old[];
extern float32_t fil_out;
extern float32_t /*DMAMEM*/ FIR_Coef_I[];
extern float32_t /*DMAMEM*/ FIR_Coef_Q[];
extern float32_t /*DMAMEM*/ FIR_dec1_I_state[];
extern float32_t /*DMAMEM*/ FIR_dec2_I_state[];
extern float32_t /*DMAMEM*/ FIR_dec2_coeffs[];
extern float32_t /*DMAMEM*/ FIR_dec2_I_state[];
extern float32_t /*DMAMEM*/ FIR_dec2_Q_state[];
extern float32_t /*DMAMEM*/ FIR_int2_I_state[];
extern float32_t /*DMAMEM*/ FIR_int2_Q_state[];
extern float32_t /*DMAMEM*/ FIR_int1_coeffs[];
extern float32_t /*DMAMEM*/ FIR_int2_coeffs[];
extern float32_t /*DMAMEM*/ FIR_dec1_Q_state[];
extern float32_t /*DMAMEM*/ FIR_dec1_coeffs[];
extern float32_t /*DMAMEM*/ FIR_dec2_coeffs[];
extern float32_t /*DMAMEM*/ FIR_filter_mask[];
extern float32_t /*DMAMEM*/ FIR_int1_I_state[];
extern float32_t /*DMAMEM*/ FIR_int1_Q_state[];
extern float32_t /*DMAMEM*/ FIR_int2_I_state[];
extern float32_t /*DMAMEM*/ FIR_int2_Q_state[];
extern float32_t /*DMAMEM*/ Fir_Zoom_FFT_Decimate_I_state[];
extern float32_t /*DMAMEM*/ Fir_Zoom_FFT_Decimate_Q_state[];
extern float32_t /*DMAMEM*/ Fir_Zoom_FFT_Decimate_coeffs[];

// decimation with FIR lowpass for Zoom FFT
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I1;
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q1;
extern float32_t Fir_Zoom_FFT_Decimate_I1_state[];
extern float32_t Fir_Zoom_FFT_Decimate_Q1_state[];
extern float32_t Fir_Zoom_FFT_Decimate1_coeffs[12];

// decimation with FIR lowpass for Zoom FFT
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_I2;
extern arm_fir_decimate_instance_f32 Fir_Zoom_FFT_Decimate_Q2;
extern float32_t Fir_Zoom_FFT_Decimate_I2_state[];
extern float32_t Fir_Zoom_FFT_Decimate_Q2_state[];
extern float32_t Fir_Zoom_FFT_Decimate2_coeffs[12];

extern float32_t fixed_gain;
extern float32_t float_buffer_L[];
extern float32_t float_buffer_R[];
extern float32_t float_buffer_L_CW[];       //AFP 09-01-22
extern float32_t float_buffer_R_CW[];       //AFP 09-01-22
extern float32_t float_buffer_R_AudioCW[];  //AFP 10-18-22
extern float32_t float_buffer_L2[];
extern float32_t float_buffer_R2[];
extern float32_t float_buffer_R_AudioCW[];  //AFP 10-18-22
extern float32_t float_buffer_L_AudioCW[];  //AFP 10-18-22

extern float32_t hang_backaverage;
extern float32_t hang_backmult;
extern float32_t hang_decay_mult;
extern float32_t hang_thresh;
extern float32_t hang_level;
extern float32_t hangtime;
extern float32_t hh1;
extern float32_t hh2;
extern float32_t /*DMAMEM*/ iFFT_buffer[];
extern float32_t I_old;
extern float32_t I_sum;
extern float32_t IIR_biquad_Zoom_FFT_I_state[];
extern float32_t IIR_biquad_Zoom_FFT_Q_state[];
extern float32_t inv_max_input;
extern float32_t inv_out_target;

extern float32_t IQAmpCorrectionFactor[];
extern float32_t IQPhaseCorrectionFactor[];
extern float32_t IQXAmpCorrectionFactor[];
extern float32_t IQXPhaseCorrectionFactor[];

extern float32_t IQ_sum;
extern float32_t K_dirty;
extern float32_t K_est;
extern float32_t K_est_old;
extern float32_t K_est_mult;
extern float32_t last_dc_level;
extern float32_t /*DMAMEM*/ last_sample_buffer_L[];
extern float32_t /*DMAMEM*/ last_sample_buffer_R[];
extern float32_t L_BufferOffset[];
extern float32_t LMS_errsig1[];
extern float32_t LMS_NormCoeff_f32[];
extern float32_t LMS_nr_delay[];
extern float32_t LMS_StateF32[];
extern float32_t LP_Astop;
extern float32_t LP_Fpass;
extern float32_t LP_Fstop;
extern float32_t LPF_spectrum;
extern float32_t M_c1;
extern float32_t M_c2;
extern float32_t m_AttackAlpha;
extern float32_t m_AttackAvedbm;
extern float32_t m_DecayAvedbm;
extern float32_t m_DecayAlpha;
;
extern float32_t m_AverageMagdbm;
extern float32_t m_AttackAvedbmhz;
extern float32_t m_DecayAvedbmhz;
extern float32_t m_AverageMagdbmhz;
extern float32_t *mag_coeffs[];
extern float32_t max_gain;
extern float32_t max_input;
extern int calTypeFlag;
extern int calOnFlag;

extern float32_t midbass;
extern float32_t mid;
extern float32_t midtreble;
extern float32_t min_volts;

extern float32_t noiseThreshhold;
extern float32_t notches[];
extern float32_t /*DMAMEM*/ NR_FFT_buffer[];
extern float32_t NR_sum;
extern float32_t NR_KIM_K;
extern float32_t NR_onemalpha;
extern float32_t NR_onemtwobeta;
extern float32_t NR_onembeta;
extern float32_t NR_G_bin_m_1;
extern float32_t NR_G_bin_p_1;
extern float32_t NR_T;
extern float32_t NR_output_audio_buffer[];
extern float32_t NR_last_iFFT_result[];
extern float32_t NR_last_sample_buffer_L[];
extern float32_t NR_last_sample_buffer_R[];
extern float32_t NR_X[][3];
extern float32_t NR_E[][15];
extern float32_t NR_M[];
extern float32_t NR_Nest[][2];  //
extern float32_t NR_vk;
extern float32_t NR_lambda[];
extern float32_t NR_Gts[][2];
extern float32_t NR_G[];
extern float32_t NR_SNR_prio[];
extern float32_t NR_SNR_post[];
extern float32_t NR_SNR_post_pos;
extern float32_t NR_Hk_old[];
extern float32_t NR_VAD;
extern float32_t NR_VAD_thresh;
extern float32_t NR_long_tone[][2];
extern float32_t NR_long_tone_gain[];
extern float32_t NR_long_tone_alpha;
extern float32_t NR_long_tone_thresh;
extern float32_t NR_gain_smooth_alpha;
extern float32_t NR_temp_sum;
extern float32_t NB_thresh;
extern float32_t offsetDisplayDB;

extern float32_t onemfast_backmult;
extern float32_t onemhang_backmult;
extern float32_t out_sample[];
extern float32_t out_targ;
extern float32_t out_target;

extern float32_t P_dirty;
extern float32_t P_est;
extern float32_t P_est_old;
extern float32_t P_est_mult;
extern float32_t phaseLO;

extern float32_t pop_ratio;
extern float32_t Q_old;
extern float32_t Q_sum;
extern float32_t R_BufferOffset[];
extern float32_t ring[];
extern float32_t ring_max;
extern float32_t sample_meanL;
extern float32_t sample_meanR;
extern float32_t sample_meanLNew;
extern float32_t sample_meanRNew;
extern float32_t save_volts;
extern const float32_t volumeLog[101];
extern float32_t slope_constant;
extern float32_t spectrum_display_scale;  // 30.0
extern float32_t stereo_factor;
extern float32_t tau_attack;
extern float32_t tau_decay;
extern float32_t tau_fast_backaverage;
extern float32_t tau_fast_decay;
extern float32_t tau_hang_backmult;
extern float32_t tau_hang_decay;
extern float32_t teta1;
extern float32_t teta2;
extern float32_t teta3;
extern float32_t teta1_old;
extern float32_t teta2_old;
extern float32_t teta3_old;
extern float32_t tmp;
extern float32_t treble;
extern float32_t var_gain;
extern float32_t volts;
extern float32_t w;
extern float32_t wold;

extern float angl;
extern float bitSamplePeriod;
extern float bitSampleTimer;
extern float bsq, usq;
extern float cf1, cf2;
extern float dcfMean;
extern float dcfSum;
extern float lolim, hilim;
extern float partr, parti;
extern float pi;
extern float tau;
extern float temp;
extern float Tsample;
extern float xExpand;  //AFP
extern float x;

extern const float32_t nuttallWindow256[];

extern float32_t FFT_buffer[] __attribute__((aligned(4)));

extern float32_t float_buffer_L_3[];
extern float32_t float_buffer_R_3[];

extern const float32_t atanTable[];
extern const float32_t DF1;           // decimation factor
extern const float32_t DF2;           // decimation factor
extern const float32_t DF;            // decimation factor
extern const float32_t n_att;         // desired stopband attenuation
extern const float32_t n_desired_BW;  // desired max BW of the filters
extern const float32_t n_fpass1;
extern const float32_t n_fpass2;
extern const float32_t n_fstop1;
extern const float32_t n_fstop2;
extern const float32_t n_samplerate;  // samplerate before decimation

extern double elapsed_micros_idx_t;
extern double elapsed_micros_mean;
extern double elapsed_micros_sum;

//======================================== Function prototypes =========================================================

void AGC();
void AGCLoadValues();  // AGC fix.  G0ORX September 5, 2023
int AGCOptions();
void AGCPrep();
float32_t AlphaBetaMag(float32_t inphase, float32_t quadrature);
void AltNoiseBlanking(float *insamp, int Nsam, float *E);
void AMDemodAM();
void AMDecodeSAM();  // AFP 11-03-22
void AssignEEPROMObjectToVariable();

int BandOptions();
float BearingHeading(char *dxCallPrefix);
int BearingMaps();
void bmpDraw(const char *filename, int x, int y);
void ButtonBandDecrease();
void ButtonBandIncrease();
int ButtonDemod();
void ButtonDemodMode();
void ButtonFilter();
void ButtonFrequencyEntry();
void ButtonFreqIncrement();
void ButtonMenuIncrease();
void ButtonMenuDecrease();
void ButtonMode();
void ButtonNotchFilter();
void ButtonNR();
int ButtonSetNoiseFloor();
void ButtonZoom();

void CalcZoom1Magn();
void CalcFIRCoeffs(float *coeffs_I, int numCoeffs, float32_t fc, float32_t Astop, int type, float dfc, float Fsamprate);
void CalcCplxFIRCoeffs(float *coeffs_I, float *coeffs_Q, int numCoeffs, float32_t FLoCut, float32_t FHiCut, float SampleRate);
void CalcNotchBins();
void Calculatedbm();
void CaptureKeystrokes();
int CalibrateOptions();    // AFP 10-22-22, changed JJP 2/3/23
void CalibratePreamble();  // KF5N August 14, 2023
void CalibratePrologue();  // KF5N August 14, 2023
int CalibrateFrequency();
void CenterFastTune();
void ClearEEPROM();
void Codec_gain();
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
void ControlFilterF();
void CopyEEPROM();
int CreateMapList(char ptrMaps[10][50], int *count);
int CWOptions();

#define CW_SHAPING_NONE 0
#define CW_SHAPING_RISE 1
#define CW_SHAPING_FALL 2

void CW_ExciterIQData(int shaping);  // AFP 08-18-22

void Dah();
void DecodeIQ();
void DisplayClock();
void DisplaydbM();
void DisplayIncrementField();
void Dit();
void DoCWDecoding(int audioValue);
void DoCWReceiveProcessing();  //AFP 09-19-22
void DoExciterEQ();
void DoReceiveEQ();
void DoSignalHistogram(long val);
void DoGapHistogram(long val);
int DoSplitVFO();
void DoPaddleFlip();
void DoXmitCalibrate(int toneFreq);
void DoReceiveCalibrate();
void DrawActiveLetter(int row, int horizontalSpacer, int whichLetterIndex, int keyWidth, int keyHeight);
void DrawBandWidthIndicatorBar();  // AFP 03-27-22 Layers
void DrawFrequencyBarValue();
void DrawInfoWindowFrame();
void DrawKeyboard();
int DrawNewFloor(int floor);
void DrawNormalLetter(int row, int horizontalSpacer, int whichLetterIndex, int keyWidth, int keyHeight);
void DrawSMeterContainer();
void DrawSpectrumBandwidthInfo();
void DrawSpectrumDisplayContainer();
void DrawAudioSpectContainer();
int EEPROMOptions();
void EEPROMRead();
void EEPROMDataDefaults();
void EEPROMShow();
void EEPROMStartup();
void EEPROMStuffFavorites(unsigned long current[]);
void EEPROMWrite();
void EncoderFineTune();
void EncoderFilter();
void EncoderCenterTune();
void EncoderVolume();
int EqualizerRecOptions();
int EqualizerXmtOptions();
void EraseMenus();
void ErasePrimaryMenu();
void EraseSecondaryMenu();
void EraseSpectrumDisplayContainer();
void EraseSpectrumWindow();
void ExecuteButtonPress(int val);

void FilterBandwidth();
void FilterOverlay();
void FilterSetSSB();
int FindCountry(char *prefix);
int FirstTimeSDCard();
void FormatFrequency(long f, char *b);
int FrequencyOptions();
void FreqShift1();
void FreqShift2();
float goertzel_mag(int numSamples, int TARGET_FREQUENCY, int SAMPLING_RATE, float *data);
int GetEncoderValue(int minValue, int maxValue, int startValue, int increment, char prompt[]);
float GetEncoderValueLive(float minValue, float maxValue, float startValue, float increment, char prompt[]);  //AFP 10-22-22
void GetFavoriteFrequency();

float HaversineDistance(float dxLat, float dxLon);

int InitializeSDCard();
void InitializeDataArrays();
void InitFilterMask();
void InitLMSNoiseReduction();
void initTempMon(uint16_t freq, uint32_t lowAlarmTemp, uint32_t highAlarmTemp, uint32_t panicAlarmTemp);
void IQPhaseCorrection(float32_t *I_buffer, float32_t *Q_buffer, float32_t factor, uint32_t blocksize);
float32_t Izero(float32_t x);

void JackClusteredArrayMax(int32_t *array, int32_t elements, int32_t *maxCount, int32_t *maxIndex, int32_t *firstDit, int32_t spread);

void Kim1_NR();
void KeyOn();
void KeyRingOn();
void KeyTipOn();

void LetterSpace();
void LMSNoiseReduction(int16_t blockSize, float32_t *nrbuffer);
float32_t log10f_fast(float32_t X);

void MainTune();
int MicOptions();
int ModeOptions();
//DB2OO, 29-AUG-23: added
void MorseCharacterClear(void);
void MorseCharacterDisplay(char currentLetter);
void MyDelay(unsigned long millisWait);
void MyDrawFloat(float val, int decimals, int x, int y, char *buff);
float MSinc(int m, float fc);

void NoActiveMenu();
void NoiseBlanker(float32_t *inputsamples, float32_t *outputsamples);
int NROptions();

float PlotCalSpectrum(int x1, int cal_bins[2], int capture_bins);
void printFile(const char *filename);
void EnableButtonInterrupts();
int ProcessButtonPress(int valPin);
void ProcessEqualizerChoices(int EQType, char *title);
void ProcessIQData();
void ProcessIQData2(float toneFreq);

uint16_t read16(File &f);
uint32_t read32(File &f);
int ReadSelectedPushButton();
void RedrawDisplayScreen();
void ResetFlipFlops();
void ResetHistograms();
void ResetTuning();  // AFP 10-11-22
int RFOptions();
void ResetZoom(int zoomIndex1);  // AFP 11-06-22

int SampleOptions();
void SDUpdate();
int SDPresentCheck();
void SetCompressionLevel();
void SetCompressionRatio();
void SetCompressionAttack();
void SetCompressionRelease();
int MicGainSet();

void SaveAnalogSwitchValues();
int SDDataCheck();
void EEPROMDataDefaultsToSerial(const char *filename);
void SDEEPROMDataToSerial(const char *filename);
int SDEEPROMWriteDefaults();
void SelectCalFreq();   // AFP 10-18-22
void SelectCWFilter();  // AFP 10-18-22
void SelectCWOffset();  // KF5N December 13, 2023
extern "C" uint32_t set_arm_clock(uint32_t frequency);
void SetBand();
void SetBandRelay(int state);
void SetDecIntFilters();
void SetDitLength(int wpm);
void SetFavoriteFrequency();
void SetFreq();
int SetI2SFreq(int freq);
void SetIIRCoeffs(float32_t f0, float32_t Q, float32_t sample_rate, uint8_t filter_type);
void SetKeyType();
void SetKeyPowerUp();
void SetSideToneVolume();  // This function uses encoder to set sidetone volume.  KF5N August 29, 2023
long SetTransmitDelay();
void SetTransmitDitLength(int wpm);  // JJP 8/19/23
void SetupMode(int sideBand);
void SetupMyCompressors(boolean use_HP_filter, float knee_dBFS, float comp_ratio, float attack_sec, float release_sec);  //AFP 11-01-22 in DSP.cpp
int SetWPM();
void ShowAnalogGain();
void ShowBandwidth();
void ShowCurrentPowerSetting();
void ShowDecoderMessage();
void sineTone(int numCycles);
void initCWShaping();
int SpectrumOptions();

void TurnOffInitializingMessage();

void UpdateInfoWindow();

void SetFreqCal(int calFreqShift);
extern "C" {
  void sincosf(float err, float *s, float *c);
  void sincos(double err, double *s, double *c);
}
void ShowFrequency();
void ShowMenu(const char *menu[], int where);
void ShowName();
void ShowNotch();
void ShowSpectrum();
void ShowSpectrum2(int toneFreq);
void ShowSpectrumdBScale();
void ShowTempAndLoad();
void ShowTransmitReceiveStatus();
void BandInformation();
float32_t sign(float32_t x);
void SpectralNoiseReduction(void);
void SpectralNoiseReductionInit();
void Splash();
int SubmenuSelect(const char *options[], int numberOfChoices, int defaultStart);

void T4_rtc_set(unsigned long t);
float TGetTemp();

int Unused1();  // Placeholders for array of pointers to function
int Unused2();
int Unused3();
void UpdateAGCField();
void UpdateCompressionField();
void UpdateDecoderField();
void UpdateEEPROMVersionNumber();
void UpdateIncrementField();
void UpdateNoiseField();
void UpdateNotchField();
void UpdateNRField();
void UpdateSDIndicator(int present);
void UpdateVolumeField();
void UpdateWPMField();
void UpdateZoomField();

float VolumeToAmplification(int volume);
int VFOSelect();

void WaitforWRComplete();
int WhichOneToUse(char ptrMaps[][50], int count);
void WordSpace();
void writeClippedRect(int x, int y, int cx, int cy, uint16_t *pixels, bool waitForWRC);
inline void writeRect(int x, int y, int cx, int cy, uint16_t *pixels);

void Xanr();
int Xmit_IQ_Cal();  //AFP 09-21-22

void ZoomFFTPrep();
void ZoomFFTExe(uint32_t blockSize);

