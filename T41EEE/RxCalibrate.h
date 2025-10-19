
#pragma once

// Re-factoring into class Calibrate.  Greg KF5N June 15, 2024.
// Automatic calibration added.  Greg KF5N June 11, 2024
// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023
// Re-factored into class RxCalibrate.cpp.  This class does receive calibrate only.

//#include <vector>
#include <algorithm>

class RxCalibrate {
public:

// Blue and red bar variables.  These should be 256 (bins) apart.
// These variables identify the center of the bars, not the left edge.
int32_t rx_blue_usb = 385;  // Image frequency.  383
int32_t rx_red_usb = 129;   // Receive frequency. 127
int32_t capture_bins = 32;      // Make bar_width divisible by 2.
int32_t left_text_edge = 165;

uint32_t IQCalType = 0;  // 0 is IQ Gain; 1 is Phase.
  bool refineCal = false;
  bool radioCal = false;
  bool averageFlag = false;  
  bool saveToEeprom = false;
int val;
//float increment = 0.002;
int userScale, userZoomIndex, userxmtMode;
int transmitPowerLevelTemp, cwFreqOffsetTemp, calFreqTemp;
uint16_t base_y = 460;  // 247
int calTypeFlag = 0;
float adjdB = 0.0;
float adjdBold = 0.0;  // Used in exponential averager.  KF5N May 19, 2024
float adjdB_avg = 0.0;
uint32_t adjdBMinIndex;
float32_t amplitude = 0.0;
float32_t phase = 0.0;
q15_t rawSpectrumPeak = 0;
uint32_t index = 0;
uint32_t count = 0;
uint32_t warmup = 0;
bool exitManual = false;
bool corrChange = false;
bool fftActive = false;
bool fftSuccess = false;
elapsedMillis milliTimer;
int mode;
Sideband tempSideband;
RadioMode tempMode;
RadioState tempState;

std::vector<float> sub_vectorAmp = std::vector<float>(21);
std::vector<float> sub_vectorPhase = std::vector<float>(21);
std::vector<float> sub_vectorAmpResult = std::vector<float>(21);
std::vector<float> sub_vectorPhaseResult = std::vector<float>(21);

  enum class State { warmup,
                     refineCal,
                     state0,
                     initialSweepAmp,
                     initialSweepPhase,
                     refineAmp,
                     refinePhase,
                     average,
                     setOptimal,
                     exit };

void loadCalToneBuffers(float toneFreq);
void plotCalGraphics();
void PrintMode();
void MakeFFTData();
void warmUpCal();
void printCalType(bool autoCal, bool autoCalDone);
void CalibratePreamble(int setZoom);
void CalibrateEpilogue(bool radioCal, bool saveToEeprom);
void DoReceiveCalibrate(int calMode, bool radio, bool refine, bool toEeprom);  // Mode determines CW versus SSB.
void ShowSpectrum();
float PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins);
void writeToCalData(float ichannel, float qchannel);
};