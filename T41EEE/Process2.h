//#include "SDT.h"
#pragma once
// Automatic calibration added.  Greg KF5N June 11, 2024
// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

#include <vector>
//#include <valarray>
#include <algorithm>

class Calibrate {
public:

int IQCalType;
int val;
float correctionIncrement;  //AFP 2-7-23
int userScale, userZoomIndex, userxmtMode;
int transmitPowerLevelTemp, cwFreqOffsetTemp;
uint16_t base_y = 460;  // 247
int calTypeFlag = 0;
float adjdB = 0.0;
float adjdBold = 0.0;  // Used in exponential averager.  KF5N May 19, 2024
float adjdB_old = 0.0;
float adjdB_sample = 0.0;
int i;
q15_t rawSpectrumPeak = 0;
float adjdB_avg = 0.0;

void loadCalToneBuffers();
void plotCalGraphics(int calType);
void ProcessIQData2();
void warmUpCal();
void printCalType(int IQCalType, bool autoCal, bool autoCalDone);
void CalibratePreamble(int setZoom);
void CalibratePrologue();
void DoReceiveCalibrate(bool radioCal, bool shortCal);
void DoXmitCalibrate(int toneFreqIndex, bool radioCal, bool shortCal);
const char *calFreqs[2]{ "750 Hz", "3.0 kHz" };
void SelectCalFreq();
void ShowSpectrum2();
float PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins);
void RadioCal(bool refineCal);
};