// Class Calibrate replaces Process2.cpp.  Greg KF5N June 16, 2024

#pragma once

// Re-factoring into class Calibrate.  Greg KF5N June 15, 2024.
// Automatic calibration added.  Greg KF5N June 11, 2024
// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

//#include <vector>
#include <algorithm>

class CWCalibrate {
public:

int IQCalType;
int val;
float correctionIncrement;  //AFP 2-7-23
int userScale, userZoomIndex, userxmtMode;
int transmitPowerLevelTemp, cwFreqOffsetTemp, calFreqTemp;
uint16_t base_y = 460;  // 247
int calTypeFlag = 0;
float adjdB = 0.0;
float adjdBold = 0.0;  // Used in exponential averager.  KF5N May 19, 2024
float adjdB_old = 0.0;
float adjdB_sample = 0.0;
float32_t amplitude = 0.0;
float32_t phase = 0.0;
q15_t rawSpectrumPeak = 0;
float adjdB_avg = 0.0;
uint32_t index = 0;
uint32_t count = 0;
uint32_t warmup = 0;
uint32_t adjdBMinIndex;
bool corrChange = false;
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
  enum class averagingState { refineAmp,
                              refinePhase };

averagingState avgState = averagingState::refineAmp;

void loadCalToneBuffers(float toneFreq);
void plotCalGraphics(int calType);
void ProcessIQData2(int mode);
void warmUpCal(int mode);
void printCalType(int mode, int IQCalType, bool autoCal, bool autoCalDone);
void CalibratePreamble(int setZoom);
void CalibrateEpilogue(bool radioCal, bool saveToEeprom);
void DoReceiveCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom);  // Mode determines CW versus SSB.
void DoXmitCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom);
#ifdef QSE2
void DoXmitCarrierCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom);
#endif
void ShowSpectrum2(int mode);
float PlotCalSpectrum(int mode, int x1, int cal_bins[3], int capture_bins);
void RadioCal(bool refineCal);
};