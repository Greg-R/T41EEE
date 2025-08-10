// Class Calibrate replaces Process2.cpp.  Greg KF5N June 16, 2024

#pragma once

// Re-factoring into class Calibrate.  Greg KF5N June 15, 2024.
// Automatic calibration added.  Greg KF5N June 11, 2024
// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

//#include <vector>
#include <algorithm>

class TxCalibrate {
public:

int IQCalType;
int val;
float correctionIncrement;  //AFP 2-7-23
int userScale, userZoomIndex;
int transmitPowerLevelTemp, cwFreqOffsetTemp, calFreqTemp;
uint16_t base_y = 460;  // 247
int calTypeFlag = 0;
float adjdB = 0.0;
float adjdBold = 0.0;  // Used in exponential averager.  KF5N May 19, 2024
float adjdB_old = 0.0;
float adjdB_sample = 0.0;
float sineBuffer[512];   // Used to generate CW tone.
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
float32_t amplitude = 0;
float32_t phase = 0;
  float iOptimal = 1.0;
  float qOptimal = 0.0;
int32_t iDCoffset = 0;
int32_t qDCoffset = 0;
int mode = 0;
  MenuSelect task; 
  MenuSelect lastUsedTask = MenuSelect::DEFAULT;
    bool autoCal = false;
  bool refineCal = false;
//  bool shortCal = false;  // Is this the same as refineCal???
  bool radioCal = false;
    bool averageFlag = false;
      int averageCount = 0;
      bool saveToEeprom = false;
//        State state = State::warmup;  // Start calibration state machine in warmup state.

// Blue and red bar variables:
int32_t rx_blue_usb = 128;
int32_t rx_red_usb = 384;
enum class Cal_Mode{CW_RECEIVE, CW_TRANSMIT, CW_CARRIER, SSB_RECEIVE, SSB_TRANSMIT, SSB_CARRIER,};

int Zoom_FFT_M1;
//int Zoom_FFT_M2;
int zoom_sample_ptr = 0;
float32_t LPF_spectrum = 0.82;

std::vector<float> sub_vectorAmp = std::vector<float>(21);
std::vector<float> sub_vectorPhase = std::vector<float>(21);
std::vector<float> sub_vectorAmpResult = std::vector<float>(21);
std::vector<float> sub_vectorPhaseResult = std::vector<float>(21);
std::vector<float32_t> sweepVector = std::vector<float32_t>(101);
std::vector<float32_t> sweepVectorValue = std::vector<float32_t>(101);;

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

 State state;                    
//  enum class averagingState { refineAmp,
//                              refinePhase };

//averagingState avgState = averagingState::refineAmp;

//void loadCalToneBuffers();
void loadCalToneBuffers(float toneFreq);
void plotCalGraphics(int calType);
void MakeFFTData();
void warmUpCal();
void PrintMode();
void printCalType(bool autoCal, bool autoCalDone);
void CalibratePreamble(int setZoom);
void CalibrateEpilogue(bool radioCal, bool saveToEeprom);
//void DoReceiveCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom);
void DoXmitCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom);
void DoXmitCarrierCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom);
const char *calFreqs[2]{ "750 Hz", "3.0 kHz" };
//void SelectCalFreq();
void ShowSpectrum();
float PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins);
void RadioCal(int mode, bool refineCal);
void buttonTasks(bool radioCal, bool refineCal);
};