
#include "SDT.h"

// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

#include <vector>
//#include <valarray>
#include <algorithm>

int val;
float correctionIncrement;  //AFP 2-7-23
int userScale, userZoomIndex, userxmtMode;
int transmitPowerLevelTemp, cwFreqOffsetTemp;
uint16_t base_y = 460;  // 247
int calTypeFlag = 0;
int IQCalType;
void ProcessIQData2();
float adjdB = 0.0;
float adjdBold = 0.0;  // Used in exponential averager.  KF5N May 19, 2024
float adjdB_old = 0.0;
float adjdB_sample = 0.0;
int i;
q15_t rawSpectrumPeak = 0;
float adjdB_avg = 0.0;


/*****
  Purpose: Load buffers used to modulate the transmitter during calibration.
          The prologue must restore the buffers for normal operation!

   Parameter List:
      void

   Return value:
      void
 *****/
FLASHMEM void loadCalToneBuffers() {
  float theta;
  float32_t tones[2]{ 750.0, 3000.0 };
  // This loop creates the sinusoidal waveform for the tone.
  for (int kf = 0; kf < 256; kf++) {
    theta = kf * 2.0 * PI * tones[EEPROMData.calFreq] / 24000;
    sinBuffer[kf] = sin(theta);
    cosBuffer[kf] = cos(theta);
  }
}


/*****
  Purpose: Plot calibration graphics (colored spectrum delimiter columns).
           New function, KF5N May 20, 2024
  Parameter list:
    int calType

  Return value:
    void
*****/
void plotCalGraphics(int calType) {
  tft.writeTo(L2);
  if (calType == 0) {  // Receive Cal
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(445, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);     // SPECTRUM_TOP_Y = 100, h = 135
      tft.fillRect(304, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);  // h = SPECTRUM_HEIGHT + 3
    } else {                                                         // SPECTRUM_HEIGHT = 150 so h = 153
      tft.fillRect(50, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);
      tft.fillRect(188, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    }
  }
  if (calType == 1) {  // Transmit Cal
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(312, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    } else {
      if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {
        tft.fillRect(183, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);
        tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
      }
    }
  }
  if (calType == 2) {  // Carrier Cal
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(279, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    } else {
      if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {  //mode == DEMOD_LSB
        tft.fillRect(215, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);
        tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
      }
    }
  }
  tft.writeTo(L1);
}


/*****
  Purpose: Run ProcessIQData2() a few times to load and settle out buffers.  KF5N May 22, 2024
           Compute FFT in order to find maximum signal peak prior to beginning calibration.
  Parameter list:
    void

  Return value:
    void
*****/
void warmUpCal() {
  // Run ProcessIQData2() a few times to load and settle out buffers.  Compute FFT.  KF5N May 19, 2024
  uint32_t index_of_max;  // Not used, but required by arm_max_q15 function.
  for (int i = 0; i < 1024; i = i + 1) {
    updateDisplayFlag = true;  // Causes FFT to be calculated.
    ProcessIQData2();
  }
  updateDisplayFlag = false;
  // Find peak of spectrum, which is 512 wide.  Use this to adjust spectrum peak to top of spectrum display.
  arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
}


/*****
  Purpose: Print the calibration type to the display.  KF5N May 23, 2024
  
  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void printCalType(int IQCalType, bool autoCal) {
  const char *calName;
  const char *IQName[4] = { "Receive", "Transmit", "Carrier", "Calibrate" };
  tft.writeTo(L1);
  calName = IQName[calTypeFlag];
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED);
  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 0)) {
    tft.setCursor(35, 260);
    tft.print(calName);
    tft.setCursor(35, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(35, 330);
      tft.fillRect(35, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(35, 330);
      tft.fillRect(35, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 0)) {
    tft.setCursor(275, 260);
    tft.print(calName);
    tft.setCursor(275, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(275, 330);
      tft.fillRect(275, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(275, 330);
      tft.fillRect(275, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 1)) {
    tft.setCursor(30, 260);
    tft.print(calName);
    tft.setCursor(30, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 1)) {
    tft.setCursor(290, 260);
    tft.print(calName);
    tft.setCursor(290, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 2)) {
    tft.setCursor(30, 260);
    tft.print(calName);
    tft.setCursor(30, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 2)) {
    tft.setCursor(290, 260);
    tft.print(calName);
    tft.setCursor(290, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
}


/*****
  Purpose: Set up prior to IQ calibrations.  New function.  KF5N August 14, 2023
  These things need to be saved here and restored in the prologue function:
  Vertical scale in dB  (set to 10 dB during calibration)
  Zoom, set to 1X in receive and 4X in transmit calibrations.
  Transmitter power, set to 5W during both calibrations.
   Parameter List:
      int setZoom   (This parameter should be 0 for receive (1X) and 2 (4X) for transmit)

   Return value:
      void
 *****/
void CalibratePreamble(int setZoom) {
  calOnFlag = true;
  IQCalType = 0;
  radioState = CW_TRANSMIT_STRAIGHT_STATE;                 // KF5N
  transmitPowerLevelTemp = EEPROMData.transmitPowerLevel;  //AFP 05-11-23
  cwFreqOffsetTemp = EEPROMData.CWOffset;
  EEPROMData.CWOffset = 2;  // 750 Hz for TX calibration.  Prologue restores user selected offset.
  patchCord15.connect();    // Connect the I and Q output channels so the transmitter will work.
  patchCord16.connect();
  userxmtMode = EEPROMData.xmtMode;          // Store the user's mode setting.  KF5N July 22, 2023
  userZoomIndex = EEPROMData.spectrum_zoom;  // Save the zoom index so it can be reset at the conclusion.  KF5N August 12, 2023
  zoomIndex = setZoom - 1;
  loadCalToneBuffers();  // Restore in the prologue.
  ButtonZoom();
  tft.fillRect(0, 272, 517, 399, RA8875_BLACK);  // Erase waterfall.  KF5N August 14, 2023
  RedrawDisplayScreen();                         // Erase any existing spectrum trace data.
  tft.writeTo(L2);                               // Erase the bandwidth bar.  KF5N August 16, 2023
  tft.clearMemory();
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(350, 160);
  tft.print("user1 - Gain/Phase");
  tft.setCursor(350, 175);
  tft.print("User2 - Incr");
  tft.setCursor(350, 190);
  tft.print("Zoom - Auto-Cal");
  tft.setTextColor(RA8875_CYAN);
  tft.fillRect(350, 125, 100, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 125);
  tft.print("dB");
  tft.setCursor(350, 110);
  tft.print("Incr= ");
  userScale = EEPROMData.currentScale;  //  Remember user preference so it can be reset when done.  KF5N
  EEPROMData.currentScale = 1;          //  Set vertical scale to 10 dB during calibration.  KF5N
  updateDisplayFlag = false;
  digitalWrite(MUTE, LOW);  //turn off mute
  xrState = RECEIVE_STATE;
  T41State = CW_RECEIVE;
  patchCord1.disconnect();  // Disconnect microphone. modeSelectInExL replaced with patchcord.  KF5N March 11, 2024
  EEPROMData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  xrState = TRANSMIT_STATE;
  digitalWrite(MUTE, HIGH);  //  Mute Audio  (HIGH=Mute)
  digitalWrite(RXTX, HIGH);  // Turn on transmitter.
  ShowTransmitReceiveStatus();
  ShowSpectrumdBScale();
  rawSpectrumPeak = 0;
}


/*****
  Purpose: Shut down and clean up after IQ calibrations.  New function.  KF5N August 14, 2023

   Parameter List:
      void

   Return value:
      void
 *****/
void CalibratePrologue() {
  digitalWrite(RXTX, LOW);  // Turn off the transmitter.
  updateDisplayFlag = false;
  xrState = RECEIVE_STATE;
  ShowTransmitReceiveStatus();
  T41State = CW_RECEIVE;
  // Clear queues to reduce transient.
  Q_in_L.clear();
  Q_in_R.clear();
  EEPROMData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  xrState = RECEIVE_STATE;
  calibrateFlag = 0;                       // KF5N
  EEPROMData.CWOffset = cwFreqOffsetTemp;  // Return user selected CW offset frequency.
  sineTone(EEPROMData.CWOffset + 6);       // This function takes "number of cycles" which is the offset + 6.
  //calFreqShift = 0;
  EEPROMData.currentScale = userScale;  //  Restore vertical scale to user preference.  KF5N
  ShowSpectrumdBScale();
  EEPROMData.xmtMode = userxmtMode;                        // Restore the user's floor setting.  KF5N July 27, 2023
  EEPROMData.transmitPowerLevel = transmitPowerLevelTemp;  // Restore the user's transmit power level setting.  KF5N August 15, 2023
  EEPROMWrite();                                           // Save calibration numbers and configuration.  KF5N August 12, 2023
  zoomIndex = userZoomIndex - 1;
  ButtonZoom();     // Restore the user's zoom setting.  Note that this function also modifies EEPROMData.spectrum_zoom.
  EEPROMWrite();    // Save calibration numbers and configuration.  KF5N August 12, 2023
  tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
  calOnFlag = false;
  RedrawDisplayScreen();
  IQChoice = 9;
  radioState = CW_RECEIVE_STATE;  // KF5N
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreq();         // Return Si5351 to normal operation mode.  KF5N
  lastState = 1111;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  return;
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the receive IQ.

   Parameter List:
      void

   Return value:
      void
 *****/
void DoReceiveCalibrate() {
  int task = -1;
  int lastUsedTask = -2;
  int calFreqTemp, calFreqShift;
  bool corrChange = false;
  float correctionIncrement = 0.01;
  calFreqTemp = EEPROMData.calFreq;
  EEPROMData.calFreq = 1;                                                     // Receive calibration currently must use 3 kHz.
  CalibratePreamble(0);                                                       // Set zoom to 1X.
  if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) calFreqShift = 24000;  //  LSB offset.  KF5N
  if (bands[EEPROMData.currentBand].mode == DEMOD_USB) calFreqShift = 24000;  //  USB offset.  KF5N
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(calFreqShift);
  calTypeFlag = 0;  // RX cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(correctionIncrement);
  bool autoCal = false;
  printCalType(calTypeFlag, autoCal);
  warmUpCal();

  enum class State { warmup,
                     state0,
                     initialSweepAmp,
                     initialSweepPhase,
                     refineAmp,
                     refinePhase,
                     average,
                     setOptimal,
                     exit };
  enum class averagingState { iChannel,
                              qChannel,
                              refineAmp,
                              refinePhase };
  State state = State::warmup;  // Start calibration state machine in warmup state.
  averagingState avgState = averagingState::iChannel;
  uint32_t warmup = 0;
  float maxSweepAmp = 0.2;
  float maxSweepPhase = 0.1;
  int count = 0;
  uint32_t index = 1;
  uint32_t adjdBMinIndex;
  int averageCount = 0;
  float iOptimal = 0;
  float qOptimal = 0;
  float increment = 0.001;
  std::vector<float32_t> sweepVector(401);
  std::vector<float32_t> sweepVectorValue(401);
  std::vector<float32_t> sub_vectorAmp(20);
  std::vector<float32_t> sub_vectorPhase(20);
  std::vector<float32_t> sub_vectorAmpResult(20);
  std::vector<float32_t> sub_vectorPhaseResult(20);

  bool averageFlag = false;
  std::vector<float>::iterator result;
  bool stopSweep = false;

  // Receive calibration loop
  while (true) {
    ShowSpectrum2();  //  Using 750 Hz or 3 kHz???
    val = ReadSelectedPushButton();
    if (val != BOGUS_PIN_READ) {
      val = ProcessButtonPress(val);
      if (val != lastUsedTask && task == -100) task = val;
      else task = BOGUS_PIN_READ;
    }
    switch (task) {
            // Activate automatic calibration.
      case ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(calTypeFlag, autoCal);
        state = State::warmup;
        break;
        // Toggle gain and phase
      case UNUSED_1:
        IQCalType = !IQCalType;
        break;
        // Toggle increment value
      case BEARING:  // UNUSED_2 is now called BEARING
        corrChange = !corrChange;
        if (corrChange == 1) {
          correctionIncrement = 0.001;  //AFP 2-7-23
        } else {                        //if (corrChange == 0)                   // corrChange is a toggle, so if not needed JJP 2/5/23
          correctionIncrement = 0.01;   //AFP 2-7-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(400, 110);
        tft.print(correctionIncrement, 3);
        break;
      case MENU_OPTION_SELECT:
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        IQChoice = 6;
        break;
      default:
        break;
    }                                     // End switch


    //  Begin automatic calibration state machine.
    if (autoCal) {
      switch (state) {
        case State::warmup:
          EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = 0.0 + maxSweepPhase;  //  Need to use these values during warmup
          EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 + maxSweepAmp;      //  so adjdB and adjdB_avg are high.
          warmup = warmup + 1;
          state = State::warmup;
          if (warmup == 10) state = State::state0;
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = 0.0;
          EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 - maxSweepAmp;  // Begin sweep at low end and move upwards.
          adjdB = 0;
          adjdB_avg = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index - 1] = EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand];

          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] + increment;  // Next one!
          // Go to Q channel when I channel sweep is finished.
          if (index > 20) {
            if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          }
          if (abs(EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] - 1.0) > maxSweepAmp || stopSweep) {  // Needs to be subtracted from 1.0.
            index = 1;
            IQCalType = 1;
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];
            iOptimal = sweepVectorValue[adjdBMinIndex];                                    // Set to the discovered minimum.
                                                                                           //              Serial.printf("The optimal amplitude = %.3f at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = -maxSweepPhase;  // Reset for next sweep.
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorAmp = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
        //    for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
        //      Serial.printf("Amp sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
        //    }
        //    for (uint32_t i = 0; i < sub_vectorAmp.size(); i = i + 1) {
        //      Serial.printf("sub_vectorAmp[%d] = %.3f\n", i, sub_vectorAmp[i]);
        //    }
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            stopSweep = false;
            state = State::initialSweepPhase;
            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
            //              }
            break;
          }
          index = index + 1;
          avgState = averagingState::iChannel;
          state = State::average;
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index - 1] = EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand];
          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          // Increment for the next measurement.
          EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] + increment;
          if (index > 20) {
            if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          }
          if (EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] > maxSweepPhase || stopSweep) {
            IQCalType = 0;
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            qOptimal = sweepVectorValue[adjdBMinIndex];                                                     // Set to the discovered minimum.
                                                                                                            //             Serial.printf("The optimal phase = %.3f at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
                                                                                                            //             EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 - maxSweepAmp;
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorPhase = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
        //    for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
        //      Serial.printf("Phase sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
        //    }

        //    for (uint32_t i = 0; i < sub_vectorPhase.size(); i = i + 1) {
        //      Serial.printf("sub_vectorPhase[%d] = %.3f\n", i, sub_vectorPhase[i]);
        //    }

            state = State::refineAmp;  // Proceed to refine the I channel.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            break;
          }
          index = index + 1;
          avgState = averagingState::qChannel;
          state = State::average;
          break;

        case State::refineAmp:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = sub_vectorAmp[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorAmpResult[index] = adjdB_avg;
            EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = sub_vectorAmp[index] + 0.001;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorAmp.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorAmpResult.begin(), sub_vectorAmpResult.end());
            adjdBMinIndex = std::distance(sub_vectorAmpResult.begin(), result);
            //            Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // iOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            iOptimal = sub_vectorAmp[adjdBMinIndex];
            EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = iOptimal;  // Set to optimal value before refining phase.
            state = State::refinePhase;
            IQCalType = 1;
            index = 0;
            averageFlag = false;

            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sub_vectorAmpResult[%d] = %.3f sub_VectorAmp[%d] = %.3f\n", i, sub_vectorAmpResult[i], i, sub_vectorAmp[i]);
            //              }

            break;
          }
          avgState = averagingState::refineAmp;
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = sub_vectorPhase[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorPhaseResult[index] = adjdB_avg;
            EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = sub_vectorPhase[index] + 0.001;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorPhase.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorPhaseResult.begin(), sub_vectorPhaseResult.end());
            adjdBMinIndex = std::distance(sub_vectorPhaseResult.begin(), result);
            //           Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // qOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            qOptimal = sub_vectorPhase[adjdBMinIndex];
            state = State::setOptimal;
            break;
          }
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 4) {
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          //  This may reduce accuracy of the calibration.
          if (count == 0 || count == 1) {  // Skip averaging for 1st and 2nd sweeps.
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            averageFlag = false;  // Don't average.  We are going for speed in the first sweeps.
            break;
          }

          averageCount = averageCount + 1;
          averageFlag = false;
          break;

        case State::setOptimal:
          EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = iOptimal;
          EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = qOptimal;
      //    Serial.printf("iOptimal = %.3f qOptimal = %.3f\n", iOptimal, qOptimal);
          state = State::exit;
          break;
        case State::exit:
          autoCal = false;
          printCalType(calTypeFlag, autoCal);
          break;
      }
    }  // end automatic calibration state machine

    if (task != -1) lastUsedTask = task;  //  Save the last used task.
    task = -100;                          // Reset task after it is used.
    if (IQCalType == 0) {                 // AFP 2-11-23
      EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand], correctionIncrement, (char *)"IQ Gain");
    } else {
      EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand], correctionIncrement, (char *)"IQ Phase");
    }
    if (IQChoice == 6) break;  // Exit the while loop.
  }                            // End while loop
  CalibratePrologue();
  EEPROMData.calFreq = calFreqTemp;  // Set back to the user selected calibration tone frequency.
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int toneFreqIndex

   Return value:
      void
 *****/
void DoXmitCalibrate(int toneFreqIndex) {
  enum class State { warmup,
                     state0,
                     initialSweepAmp,
                     initialSweepPhase,
                     refineAmp,
                     refinePhase,
                     average,
                     setOptimal,
                     exit };
  enum class averagingState { iChannel,
                              qChannel,
                              refineAmp,
                              refinePhase };
  int task = -1;
  int lastUsedTask = -2;
  int freqOffset;
  bool corrChange = false;
  float correctionIncrement = 0.01;
  State state = State::warmup;  // Start calibration state machine in warmup state.
  averagingState avgState = averagingState::iChannel;
  uint32_t warmup = 0;
  float maxSweepAmp = 0.2;
  float maxSweepPhase = 0.1;
  int count = 0;
  uint32_t index = 1;
  uint32_t adjdBMinIndex;
  int averageCount = 0;
  float iOptimal = 0;
  float qOptimal = 0;
  float increment = 0.001;
  std::vector<float32_t> sweepVector(401);
  std::vector<float32_t> sweepVectorValue(401);
  std::vector<float32_t> sub_vectorAmp(20);
  std::vector<float32_t> sub_vectorPhase(20);
  std::vector<float32_t> sub_vectorAmpResult(20);
  std::vector<float32_t> sub_vectorPhaseResult(20);

  bool autoCal = false;
  bool averageFlag = false;
  std::vector<float>::iterator result;
  bool stopSweep = false;

  if (toneFreqIndex == 0) {  // 750 Hz
    CalibratePreamble(4);    // Set zoom to 16X.
    freqOffset = 0;          // Calibration tone same as regular modulation tone.
  }
  if (toneFreqIndex == 1) {  // 3 kHz
    CalibratePreamble(2);    // Set zoom to 4X.
    freqOffset = 2250;       // Need 750 + 2250 = 3 kHz
  }
  calTypeFlag = 1;  // TX cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(correctionIncrement);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  printCalType(calTypeFlag, autoCal);
  warmUpCal();

  // Transmit Calibration Loop
  while (true) {
    ShowSpectrum2();
    val = ReadSelectedPushButton();
    if (val != BOGUS_PIN_READ) {
      val = ProcessButtonPress(val);
      if (val != lastUsedTask && task == -100) task = val;
      else task = BOGUS_PIN_READ;
    }
    switch (task) {
      // Activate automatic calibration.
      case ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(calTypeFlag, autoCal);
        state = State::warmup;
        break;
      // Toggle gain and phase
      case UNUSED_1:
        IQCalType = !IQCalType;
        break;
      // Toggle increment value
      case BEARING:  // UNUSED_2 is now called BEARING
        corrChange = !corrChange;
        if (corrChange == 1) {          // Toggle increment value
          correctionIncrement = 0.001;  // AFP 2-11-23
        } else {
          correctionIncrement = 0.01;  // AFP 2-11-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(400, 110);
        tft.print(correctionIncrement, 3);
        break;
      case MENU_OPTION_SELECT:  // Save values and exit calibration.
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        IQChoice = 6;  // AFP 2-11-23
        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal) {
      switch (state) {
        case State::warmup:
          EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = 0.0 + maxSweepPhase;  //  Need to use these values during warmup
          EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 + maxSweepAmp;      //  so adjdB and adjdB_avg are high.
          warmup = warmup + 1;
          state = State::warmup;
          if (warmup == 10) state = State::state0;
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = 0.0;
          EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 - maxSweepAmp;  // Begin sweep at low end and move upwards.
          adjdB = 0;
          adjdB_avg = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index - 1] = EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand];

          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] + increment;  // Next one!
          // Go to Q channel when I channel sweep is finished.
          if (index > 20) {
            if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          }
          if (abs(EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] - 1.0) > maxSweepAmp || stopSweep) {  // Needs to be subtracted from 1.0.
            index = 1;
            IQCalType = 1;
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];
            iOptimal = sweepVectorValue[adjdBMinIndex];                                    // Set to the discovered minimum.
                                                                                           //              Serial.printf("The optimal amplitude = %.3f at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = -maxSweepPhase;  // Reset for next sweep.
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorAmp = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
        //    for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
        //      Serial.printf("Amp sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
        //    }
        //    for (uint32_t i = 0; i < sub_vectorAmp.size(); i = i + 1) {
        //      Serial.printf("sub_vectorAmp[%d] = %.3f\n", i, sub_vectorAmp[i]);
        //    }
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            stopSweep = false;
            state = State::initialSweepPhase;
            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
            //              }
            break;
          }
          index = index + 1;
          avgState = averagingState::iChannel;
          state = State::average;
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index - 1] = EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand];
          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          // Increment for the next measurement.
          EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] + increment;
          if (index > 20) {
            if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          }
          if (EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] > maxSweepPhase || stopSweep) {
            IQCalType = 0;
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            qOptimal = sweepVectorValue[adjdBMinIndex];                                                     // Set to the discovered minimum.
                                                                                                            //             Serial.printf("The optimal phase = %.3f at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
                                                                                                            //             EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 - maxSweepAmp;
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorPhase = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
        //    for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
        //      Serial.printf("Phase sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
        //    }

        //    for (uint32_t i = 0; i < sub_vectorPhase.size(); i = i + 1) {
        //      Serial.printf("sub_vectorPhase[%d] = %.3f\n", i, sub_vectorPhase[i]);
        //    }

            state = State::refineAmp;  // Proceed to refine the I channel.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            break;
          }
          index = index + 1;
          avgState = averagingState::qChannel;
          state = State::average;
          break;

        case State::refineAmp:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = sub_vectorAmp[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorAmpResult[index] = adjdB_avg;
            EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = sub_vectorAmp[index] + 0.001;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorAmp.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorAmpResult.begin(), sub_vectorAmpResult.end());
            adjdBMinIndex = std::distance(sub_vectorAmpResult.begin(), result);
            //            Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // iOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            iOptimal = sub_vectorAmp[adjdBMinIndex];
            EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = iOptimal;  // Set to optimal value before refining phase.
            state = State::refinePhase;
            IQCalType = 1;
            index = 0;
            averageFlag = false;

            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sub_vectorAmpResult[%d] = %.3f sub_VectorAmp[%d] = %.3f\n", i, sub_vectorAmpResult[i], i, sub_vectorAmp[i]);
            //              }

            break;
          }
          avgState = averagingState::refineAmp;
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = sub_vectorPhase[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorPhaseResult[index] = adjdB_avg;
            EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = sub_vectorPhase[index] + 0.001;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorPhase.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorPhaseResult.begin(), sub_vectorPhaseResult.end());
            adjdBMinIndex = std::distance(sub_vectorPhaseResult.begin(), result);
            //           Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // qOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            qOptimal = sub_vectorPhase[adjdBMinIndex];
            state = State::setOptimal;
            break;
          }
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 4) {
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          //  This may reduce accuracy of the calibration.
          if (count == 0 || count == 1) {  // Skip averaging for 1st and 2nd sweeps.
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            averageFlag = false;  // Don't average.  We are going for speed in the first sweeps.
            break;
          }

          averageCount = averageCount + 1;
          averageFlag = false;
          break;

        case State::setOptimal:
          EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = iOptimal;
          EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = qOptimal;
      //    Serial.printf("iOptimal = %.3f qOptimal = %.3f\n", iOptimal, qOptimal);
          state = State::exit;
          break;
        case State::exit:
          autoCal = false;
          printCalType(calTypeFlag, autoCal);
          break;
      }
    }  // end automatic calibration state machine

    if (task != -1) lastUsedTask = task;  //  Save the last used task.
    task = -100;                          // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand], correctionIncrement, (char *)"IQ Gain X");
    } else {
      EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand], correctionIncrement, (char *)"IQ Phase X");
    }
    if (IQChoice == 6) break;  //  Exit the while loop.
  }                            // end while
  CalibratePrologue();
}  // End Transmit calibration


/*****
  Purpose: Manually tuned cancellation of the undesired transmitter carrier output.

   Parameter List:
      int toneFreqIndex

   Return value:
      void
 *****/
#ifdef QSE2
void DoXmitCarrierCalibrate(int toneFreqIndex) {
  enum class State { warmup,
                     state0,
                     initialSweepAmp,
                     initialSweepPhase,
                     refineAmp,
                     refinePhase,
                     average,
                     setOptimal,
                     exit };
  enum class averagingState { iChannel,
                              qChannel,
                              refineAmp,
                              refinePhase };
  int task = -1;
  int lastUsedTask = -2;
  int freqOffset;
  bool corrChange = true;
  uint32_t warmup = 0;
  int count = 0;
  uint32_t adjdBMinIndex;
  int averageCount = 0;
  int maxSweep = 400;
  int iOptimal = 0;
  int qOptimal = 0;
  int increment = 5;
  bool autoCal = false;
  State state = State::warmup;  // Start calibration state machine in warmup state.
  averagingState avgState = averagingState::iChannel;

  uint32_t index = 1;

  std::vector<float> sweepVector(401);
  std::vector<int> sweepVectorValue(401);
  std::vector<int> sub_vectorAmp(20);
  std::vector<int> sub_vectorPhase(20);
  std::vector<float> sub_vectorAmpResult(20);
  std::vector<float> sub_vectorPhaseResult(20);

  bool averageFlag = false;
  std::vector<float>::iterator result;
  bool stopSweep = false;

  if (toneFreqIndex == 0) {  // 750 Hz
    CalibratePreamble(4);    // Set zoom to 16X.
    freqOffset = 0;          // Calibration tone same as regular modulation tone.
  }
  if (toneFreqIndex == 1) {  // 3 kHz
    CalibratePreamble(2);    // Set zoom to 4X.
    freqOffset = 2250;       // Need 750 + 2250 = 3 kHz
  }
  calTypeFlag = 2;  // TX carrier calibration.
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(increment);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  printCalType(calTypeFlag, autoCal);
  warmUpCal();

  // Carrier Calibration Loop.  This is independent of loop().
  while (true) {
    ShowSpectrum2();
    val = ReadSelectedPushButton();
    if (val != BOGUS_PIN_READ) {
      val = ProcessButtonPress(val);
      if (val != lastUsedTask && task == -100) task = val;
      else task = BOGUS_PIN_READ;
    }
    switch (task) {
      // Activate automatic calibration.
      case ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(calTypeFlag, autoCal);
        state = State::warmup;
        break;
      // Toggle gain and phase
      case UNUSED_1:
        IQCalType = !IQCalType;
        break;
      // Toggle increment value
      case BEARING:  // UNUSED_2 is now called BEARING
        corrChange = not corrChange;
        if (corrChange == true) {  // Toggle increment value
          increment = 10;          // AFP 2-11-23
        } else {
          increment = 1;  // AFP 2-11-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(400, 110);
        tft.print(increment);
        break;
      case MENU_OPTION_SELECT:  // Save values and exit calibration.
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        IQChoice = 6;  // AFP 2-11-23
        break;
      default:
        break;
    }  // end switch


    //  Begin automatic calibration state machine.
    if (autoCal) {
      switch (state) {
        case State::warmup:
          EEPROMData.qDCoffset[EEPROMData.currentBand] = maxSweep;  //  Need to use these values during warmup
          EEPROMData.iDCoffset[EEPROMData.currentBand] = maxSweep;      //  so adjdB and adjdB_avg are high.
          warmup = warmup + 1;
          state = State::warmup;
          if (warmup == 10) state = State::state0;
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          EEPROMData.qDCoffset[EEPROMData.currentBand] = 0;
          EEPROMData.iDCoffset[EEPROMData.currentBand] = -maxSweep;  // Begin sweep at low end and move upwards.
          adjdB = 0;
          adjdB_avg = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index - 1] = EEPROMData.iDCoffset[EEPROMData.currentBand];

          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          EEPROMData.iDCoffset[EEPROMData.currentBand] = EEPROMData.iDCoffset[EEPROMData.currentBand] + increment;  // Next one!
          // Go to Q channel when I channel sweep is finished.
//          if (index > 20) {
//            if (sweepVector[index - 1] > (sweepVector[index - 11] + 1.0)) stopSweep = true;  // Stop sweep if past the null.
//          }
          if (abs(EEPROMData.iDCoffset[EEPROMData.currentBand]) > maxSweep) {  // Needs to be subtracted from 1.0.
            index = 1;
            IQCalType = 1;
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            EEPROMData.iDCoffset[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];
            iOptimal = sweepVectorValue[adjdBMinIndex];                                    // Set to the discovered minimum.
          //  Serial.printf("The optimal iDCoffset = %d at index %d with value %.3f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            EEPROMData.qDCoffset[EEPROMData.currentBand] = -maxSweep;  // Reset for next sweep.
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorAmp = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
         //   for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
         //     Serial.printf("Amp sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
         //   }
         //   for (uint32_t i = 0; i < sub_vectorAmp.size(); i = i + 1) {
         //     Serial.printf("sub_vectorAmp[%d] = %d\n", i, sub_vectorAmp[i]);
         //   }
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0);
            stopSweep = false;
            state = State::initialSweepPhase;
            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
            //              }
            break;
          }
          index = index + 1;
          avgState = averagingState::iChannel;
          state = State::average;
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index - 1] = EEPROMData.qDCoffset[EEPROMData.currentBand];
          if (averageFlag) sweepVector[index - 1] = adjdB_avg;
          else sweepVector[index - 1] = adjdB;
          // Increment for the next measurement.
          EEPROMData.qDCoffset[EEPROMData.currentBand] = EEPROMData.qDCoffset[EEPROMData.currentBand] + increment;
//          if (index > 20) {
//            if (sweepVector[index - 1] > (sweepVector[index - 11] + 2.0)) stopSweep = true;  // Stop sweep if past the null.
//          }
          if (EEPROMData.qDCoffset[EEPROMData.currentBand] > maxSweep) {
            IQCalType = 0;
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            EEPROMData.qDCoffset[EEPROMData.currentBand] = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            qOptimal = sweepVectorValue[adjdBMinIndex];                                                     // Set to the discovered minimum.
        //    Serial.printf("The optimal qDCoffset = %d at index %d with value %.3f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
                                                                                                            //             EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = 1.0 - maxSweepAmp;
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            sub_vectorPhase = { sweepVectorValue.begin() + adjdBMinIndex - 10, sweepVectorValue.begin() + adjdBMinIndex + 10 };
//            for (uint32_t i = 0; i < sweepVectorValue.size(); i = i + 1) {
//              Serial.printf("Phase sweepVectorValue[%d] = %.3f sweepVector[%d] = %.3f\n", i, sweepVectorValue[i], i, sweepVector[i]);
//            }

          //  for (uint32_t i = 0; i < sub_vectorPhase.size(); i = i + 1) {
          //    Serial.printf("sub_vectorPhase[%d] = %d\n", i, sub_vectorPhase[i]);
          //  }

            state = State::refineAmp;  // Proceed to refine the I channel.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            break;
          }
          index = index + 1;
          avgState = averagingState::qChannel;
          state = State::average;
          break;

        case State::refineAmp:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.iDCoffset[EEPROMData.currentBand] = sub_vectorAmp[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorAmpResult[index] = adjdB_avg;
            EEPROMData.iDCoffset[EEPROMData.currentBand] = sub_vectorAmp[index] + increment;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorAmp.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorAmpResult.begin(), sub_vectorAmpResult.end());
            adjdBMinIndex = std::distance(sub_vectorAmpResult.begin(), result);
            //            Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // iOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            iOptimal = sub_vectorAmp[adjdBMinIndex];
            EEPROMData.iDCoffset[EEPROMData.currentBand] = iOptimal;  // Set to optimal value before refining phase.
            state = State::refinePhase;
            IQCalType = 1;
            index = 0;
            averageFlag = false;

            //              for(int i = 0; i < 40; i = i + 1) {
            //               Serial.printf("sub_vectorAmpResult[%d] = %.3f sub_VectorAmp[%d] = %.3f\n", i, sub_vectorAmpResult[i], i, sub_vectorAmp[i]);
            //              }

            break;
          }
          avgState = averagingState::refineAmp;
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          EEPROMData.qDCoffset[EEPROMData.currentBand] = sub_vectorPhase[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorPhaseResult[index] = adjdB_avg;
            EEPROMData.qDCoffset[EEPROMData.currentBand] = sub_vectorPhase[index] + increment;  // Measure and record adjdB_avg at this value of correction factor.
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorPhase.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorPhaseResult.begin(), sub_vectorPhaseResult.end());
            adjdBMinIndex = std::distance(sub_vectorPhaseResult.begin(), result);
            //           Serial.printf("*result = %.3f adjdBMinIndex = %d\n", *result, adjdBMinIndex);
            // qOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            qOptimal = sub_vectorPhase[adjdBMinIndex];
            state = State::setOptimal;
            break;
          }
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 8) {
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          //  This may reduce accuracy of the calibration.
          if (count == 0 || count == 1) {  // Skip averaging for 1st and 2nd sweeps.
            if (avgState == averagingState::iChannel) state = State::initialSweepAmp;
            if (avgState == averagingState::qChannel) state = State::initialSweepPhase;
            averageFlag = false;  // Don't average.  We are going for speed in the first sweeps.
            break;
          }

          averageCount = averageCount + 1;
          averageFlag = false;
          break;

        case State::setOptimal:
          EEPROMData.iDCoffset[EEPROMData.currentBand] = iOptimal;
          EEPROMData.qDCoffset[EEPROMData.currentBand] = qOptimal;
        //  Serial.printf("iOptimal = %d qOptimal = %d\n", iOptimal, qOptimal);
          state = State::exit;
          break;
        case State::exit:
          autoCal = false;
          printCalType(calTypeFlag, autoCal);
          break;
      }
    }  // end automatic calibration state machine


    if (task != -1) lastUsedTask = task;  //  Save the last used task.
    task = -100;                          // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      EEPROMData.iDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.iDCoffset[EEPROMData.currentBand], increment, (char *)"I Offset:");
    } else {
      EEPROMData.qDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.qDCoffset[EEPROMData.currentBand], increment, (char *)"Q Offset:");
    }
    if (IQChoice == 6) break;  //  Exit the while loop.

  }  // end while
  CalibratePrologue();
}
#endif


/*****
  Purpose: Manually tuned cancellation of the undesired transmitter carrier output.

   Parameter List:
      int toneFreqIndex

   Return value:
      void
 *****
#ifdef QSE2
void DoXmitCarrierCalibrate(int toneFreqIndex) {
  enum class State { warmup,
                     state0,
                     state1,
                     state2,
                     average,
                     setOptimal,
                     exit };
  enum class averagingState { iChannel,
                              qChannel };
  int task = -1;
  int lastUsedTask = -2;
  int freqOffset;
  bool corrChange = true;
  State state = State::warmup;  // Start in warmup.
  averagingState avgState = averagingState::iChannel;
  uint32_t warmup = 0;
  int count = 0;
  int index = 1;
  float adjdBMin;
  uint32_t adjdBMinIndex;
  int averageCount = 0;
  int maxSweep = 400;
  int iOptimal = 0;
  int qOptimal = 0;
  int increment = 5;
  int points = (maxSweep * 2) / increment;
  float sweepArray[1001] = { 0 };
  int sweepArrayOffset[1001] = { 0 };
  bool autoCal = false;
  bool averageFlag = false;

  if (toneFreqIndex == 0) {  // 750 Hz
    CalibratePreamble(4);    // Set zoom to 16X.
    freqOffset = 0;          // Calibration tone same as regular modulation tone.
  }
  if (toneFreqIndex == 1) {  // 3 kHz
    CalibratePreamble(2);    // Set zoom to 4X.
    freqOffset = 2250;       // Need 750 + 2250 = 3 kHz
  }
  calTypeFlag = 2;  // TX carrier calibration.
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(increment);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  printCalType(calTypeFlag, autoCal);
  warmUpCal();

  // Carrier Calibration Loop.  This is independent of loop().
  while (true) {
    ShowSpectrum2();
    val = ReadSelectedPushButton();
    if (val != BOGUS_PIN_READ) {
      val = ProcessButtonPress(val);
      if (val != lastUsedTask && task == -100) task = val;
      else task = BOGUS_PIN_READ;
    }
    switch (task) {
      // Activate automatic calibration.
      case ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(calTypeFlag, autoCal);
        state = State::warmup;
        break;
      // Toggle gain and phase
      case UNUSED_1:
        IQCalType = !IQCalType;
        break;
      // Toggle increment value
      case BEARING:  // UNUSED_2 is now called BEARING
        corrChange = not corrChange;
        if (corrChange == true) {  // Toggle increment value
          increment = 10;          // AFP 2-11-23
        } else {
          increment = 1;  // AFP 2-11-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(400, 110);
        tft.print(increment);
        break;
      case MENU_OPTION_SELECT:  // Save values and exit calibration.
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        IQChoice = 6;  // AFP 2-11-23
        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal) {
      switch (state) {
        case State::warmup:
          EEPROMData.qDCoffset[EEPROMData.currentBand] = maxSweep;  // Force I and Q to extreme edge.
          EEPROMData.iDCoffset[EEPROMData.currentBand] = maxSweep;
          warmup = warmup + 1;
          state = State::warmup;
          if (warmup == 10) {
            state = State::state0;
            for (int i = 0; i < 1001; i = i + 1) sweepArray[index - 1] = 0;  // Clear the result array.  Probably not necessary.
          }
          break;
        case State::state0:
          // Starting values for sweeps.
          EEPROMData.qDCoffset[EEPROMData.currentBand] = 0;
          EEPROMData.iDCoffset[EEPROMData.currentBand] = -maxSweep;
          adjdB_avg = 0;
          adjdB = 0;
          state = State::state1;  // Let this fall through.
        case State::state1:
          sweepArrayOffset[index - 1] = EEPROMData.iDCoffset[EEPROMData.currentBand];
          if (averageFlag) sweepArray[index - 1] = adjdB_avg;                                                       // Use average value after several passes.
          else sweepArray[index - 1] = adjdB;                                                                       // Use instantaneous value when averaging is off.
          EEPROMData.iDCoffset[EEPROMData.currentBand] = EEPROMData.iDCoffset[EEPROMData.currentBand] + increment;  // Next one!

          // Stop sweep if null has been passed.  Look behind by 10 steps.
          if (index > 11) {
            if ((sweepArray[index - 1] - sweepArray[index - 10]) > 2.0) {  // If the undesired signal increase by 2 dB, the null has been passed.
              Serial.println("Early cut-out activated");
              state = State::state2;
              index = 1;
              IQCalType = 1;
              arm_min_f32(sweepArray, points, &adjdBMin, &adjdBMinIndex);
              EEPROMData.iDCoffset[EEPROMData.currentBand] = sweepArrayOffset[adjdBMinIndex];
              iOptimal = sweepArrayOffset[adjdBMinIndex];  // Set to the discovered minimum.
              Serial.printf("The optimal iOffset = %d at index %d with value %.1f count = %d\n", sweepArrayOffset[adjdBMinIndex], adjdBMinIndex, adjdBMin, count);
              EEPROMData.qDCoffset[EEPROMData.currentBand] = -maxSweep;  // Reset for next sweep.  Start at negative edge and sweep up.
              count = count + 1;
              if (count == 4) state = State::setOptimal;
              break;
            }
          }


          // Go to Q channel when I channel sweep is finished.
          if (EEPROMData.iDCoffset[EEPROMData.currentBand] == maxSweep) {
            state = State::state2;
            index = 1;
            IQCalType = 1;
            arm_min_f32(sweepArray, points, &adjdBMin, &adjdBMinIndex);
            EEPROMData.iDCoffset[EEPROMData.currentBand] = sweepArrayOffset[adjdBMinIndex];
            iOptimal = sweepArrayOffset[adjdBMinIndex];  // Set to the discovered minimum.
            Serial.printf("The optimal iOffset = %d at index %d with value %.1f count = %d\n", sweepArrayOffset[adjdBMinIndex], adjdBMinIndex, adjdBMin, count);
            EEPROMData.qDCoffset[EEPROMData.currentBand] = -maxSweep;  // Reset for next sweep.  Start at negative edge and sweep up.
            count = count + 1;
            if (count == 4) state = State::setOptimal;
            break;
          }
          index = index + 1;
          avgState = averagingState::iChannel;
          state = State::average;
          break;
        case State::state2:
          sweepArrayOffset[index - 1] = EEPROMData.qDCoffset[EEPROMData.currentBand];
          if (averageFlag) sweepArray[index - 1] = adjdB_avg;
          else sweepArray[index - 1] = adjdB;  // Use instantaneous value when averaging is off.
          EEPROMData.qDCoffset[EEPROMData.currentBand] = EEPROMData.qDCoffset[EEPROMData.currentBand] + increment;
          if (EEPROMData.qDCoffset[EEPROMData.currentBand] == maxSweep) {
            state = State::state1;
            index = 1;
            IQCalType = 0;
            arm_min_f32(sweepArray, points, &adjdBMin, &adjdBMinIndex);
            EEPROMData.qDCoffset[EEPROMData.currentBand] = sweepArrayOffset[adjdBMinIndex];  // Set to the discovered minimum.
            qOptimal = sweepArrayOffset[adjdBMinIndex];                                      // Set to the discovered minimum.
            Serial.printf("The optimal qOffset = %d at index %d with value %.1f count = %d\n", sweepArrayOffset[adjdBMinIndex], adjdBMinIndex, adjdBMin, count);
            EEPROMData.iDCoffset[EEPROMData.currentBand] = -maxSweep;
            count = count + 1;
            if (count == 4) state = State::setOptimal;
            break;
          }
          index = index + 1;
          avgState = averagingState::qChannel;
          state = State::average;
          break;
        case State::average:       // Stay in this state while averaging is in progress.
          if (averageCount > 8) {  // Exit and deliver adjdB_avg measurement.
            if (avgState == averagingState::iChannel) state = State::state1;
            if (avgState == averagingState::qChannel) state = State::state2;
            averageFlag = true;
            averageCount = 0;
            break;
          }
          //  This may reduce accuracy of the calibration.
          if (count == 0 || count == 1) {  // Skip averaging for 1st and 2nd sweeps.
            if (avgState == averagingState::iChannel) state = State::state1;
            if (avgState == averagingState::qChannel) state = State::state2;
            averageFlag = false;
            break;
          }
          averageFlag = true;
          averageCount = averageCount + 1;
          break;
        case State::setOptimal:
          EEPROMData.iDCoffset[EEPROMData.currentBand] = iOptimal;
          EEPROMData.qDCoffset[EEPROMData.currentBand] = qOptimal;
          state = State::exit;
          break;
        case State::exit:
          autoCal = false;
          warmup = 0;  // In case calibration is run again.
          printCalType(calTypeFlag, autoCal);
          break;
      }
    }  // end automatic calibration state machine


    if (task != -1) lastUsedTask = task;  //  Save the last used task.
    task = -100;                          // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      EEPROMData.iDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.iDCoffset[EEPROMData.currentBand], increment, (char *)"I Offset:");
    } else {
      EEPROMData.qDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.qDCoffset[EEPROMData.currentBand], increment, (char *)"Q Offset:");
    }
    if (IQChoice == 6) break;  //  Exit the while loop.

  }  // end while
  CalibratePrologue();
}
#endif
*/

// Automatic calibration
void autoCal() {
}


/*****
  Purpose: Signal processing for the purpose of calibration.

   Parameter List:
      int toneFreq, index of an array of tone frequencies.

   Return value:
      void
 *****/
void ProcessIQData2() {
  float rfGainValue, powerScale;                                   // AFP 2-11-23.  Greg KF5N February 13, 2023
  float recBandFactor[7] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };  // AFP 2-11-23  KF5N uniform values

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are read into two arrays sp_L and sp_R in blocks of 128 up to N_BLOCKS.  The arrarys are
        of size BUFFER_SIZE * N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_LENGTH / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_LENGTH = 512
        BUFFER_SIZE*N_BLOCKS = 2024 samples
     **********************************************************************************/
  // Generate I and Q for the transmit or receive calibration.  KF5N                                  // KF5N
  arm_scale_f32(cosBuffer, 0.20, float_buffer_L_EX, 256);  // AFP 2-11-23 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32(sinBuffer, 0.20, float_buffer_R_EX, 256);  // AFP 2-11-23 Sidetone = 3000

  if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
    arm_scale_f32(float_buffer_L_EX, -EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L_EX, 256);       //Adjust level of L buffer // AFP 2-11-23
    IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand], 256);  // Adjust phase
  } else {
    if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {
      arm_scale_f32(float_buffer_L_EX, EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L_EX, 256);  // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand], 256);
    }
  }
  //24KHz effective sample rate here
  arm_fir_interpolate_f32(&FIR_int1_EX_I, float_buffer_L_EX, float_buffer_LTemp, 256);
  arm_fir_interpolate_f32(&FIR_int1_EX_Q, float_buffer_R_EX, float_buffer_RTemp, 256);

  // interpolation-by-4,  48KHz effective sample rate here
  arm_fir_interpolate_f32(&FIR_int2_EX_I, float_buffer_LTemp, float_buffer_L_EX, 512);
  arm_fir_interpolate_f32(&FIR_int2_EX_Q, float_buffer_RTemp, float_buffer_R_EX, 512);

  //  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
  powerScale = 40.0 * EEPROMData.powerOutCW[EEPROMData.currentBand];
#else
  powerScale = 30.0 * EEPROMData.powerOutCW[EEPROMData.currentBand];
#endif

  //  192KHz effective sample rate here
  arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048);  //Scale to compensate for losses in Interpolation
  arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

  // This code block was introduced after TeensyDuino 1.58 appeared.  It doesn't use a for loop, but processes the entire 2048 buffer in one pass.
  // Are there at least N_BLOCKS buffers in each channel available ?
  if ((uint32_t)Q_in_L.available() > N_BLOCKS && (uint32_t)Q_in_R.available() > N_BLOCKS) {  // Audio Record Queues!!!

    // Revised I and Q calibration signal generation using large buffers.  Greg KF5N June 4 2023
    q15_t q15_buffer_LTemp[2048];  //KF5N
    q15_t q15_buffer_RTemp[2048];  //KF5N
    Q_out_L_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
    Q_out_R_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
    arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
    arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);

#ifdef QSE2
    arm_offset_q15(q15_buffer_LTemp, EEPROMData.iDCoffset[EEPROMData.currentBand] + 1260, q15_buffer_LTemp, 2048);
    arm_offset_q15(q15_buffer_RTemp, EEPROMData.qDCoffset[EEPROMData.currentBand] + 1260, q15_buffer_RTemp, 2048);
#endif

    Q_out_L_Ex.play(q15_buffer_LTemp, 2048);
    Q_out_R_Ex.play(q15_buffer_RTemp, 2048);
    Q_out_L_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
    Q_out_R_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);

    // End of transmit code.  Begin receive code.

    // get audio samples from the audio  buffers and convert them to float
    // read in 32 blocks á 128 samples in I and Q
    for (unsigned i = 0; i < N_BLOCKS; i++) {
      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float(Q_in_R.readBuffer(), &float_buffer_L[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      arm_q15_to_float(Q_in_L.readBuffer(), &float_buffer_R[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      Q_in_L.freeBuffer();
      Q_in_R.freeBuffer();
    }

    rfGainValue = pow(10, (float)EEPROMData.rfGain[EEPROMData.currentBand] / 20);        //AFP 2-11-23
    arm_scale_f32(float_buffer_L, rfGainValue, float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23
    arm_scale_f32(float_buffer_R, rfGainValue, float_buffer_R, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23

    /**********************************************************************************  AFP 12-31-20
      Scale the data buffers by the RFgain value defined in bands[EEPROMData.currentBand] structure
    **********************************************************************************/
    arm_scale_f32(float_buffer_L, recBandFactor[EEPROMData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23
    arm_scale_f32(float_buffer_R, recBandFactor[EEPROMData.currentBand], float_buffer_R, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23

    // Manual IQ amplitude correction
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      arm_scale_f32(float_buffer_L, -EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
      IQPhaseCorrection(float_buffer_L, float_buffer_R, EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand], BUFFER_SIZE * N_BLOCKS);
    } else {
      if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {
        arm_scale_f32(float_buffer_L, -EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
        IQPhaseCorrection(float_buffer_L, float_buffer_R, EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand], BUFFER_SIZE * N_BLOCKS);
      }
    }
    FreqShift1();  // Why done here? KF5N

    if (EEPROMData.spectrum_zoom == SPECTRUM_ZOOM_1) {  // && display_S_meter_or_spectrum_state == 1)
      zoom_display = 1;
      CalcZoom1Magn();  //AFP Moved to display function
    }

    // ZoomFFTExe is being called too many times in Calibration.  Should be called ONLY at the start of each sweep.
    if (EEPROMData.spectrum_zoom != SPECTRUM_ZOOM_1) {
      //AFP  Used to process Zoom>1 for display
      ZoomFFTExe(BUFFER_SIZE * N_BLOCKS);
    }
  }
}


/*****
  Purpose: Show Spectrum display modified for IQ calibration.
           This is similar to the function used for normal reception, however, it has
           been simplified and streamlined for calibration.

  Parameter list:
    void

  Return value;
    void
*****/
void ShowSpectrum2()  //AFP 2-10-23
{
  int x1 = 0;
  //  float adjdB = 0.0;
  int capture_bins = 8;  // Sets the number of bins to scan for signal peak.

  pixelnew[0] = 0;
  pixelnew[1] = 0;
  pixelold[0] = 0;
  pixelold[1] = 0;

  //  This is the "spectra scanning" for loop.  During calibration, only small areas of the spectrum need to be examined.
  //  If the entire 512 wide spectrum is used, the calibration loop will be slow and unresponsive.
  //  The scanning areas are determined by receive versus transmit calibration, and LSB or USB.  Thus there are 4 different scanning zones.
  //  All calibrations use a 0 dB reference signal and an "undesired sideband" signal which is to be minimized relative to the reference.
  //  Thus there is a target "bin" for the reference signal and another "bin" for the undesired sideband.
  //  The target bin locations are used by the for-loop to sweep a small range in the FFT.  A maximum finding function finds the peak signal strength.
  int cal_bins[3] = { 0, 0, 0 };
  if (calTypeFlag == 0 && bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
    cal_bins[0] = 315;
    cal_bins[1] = 455;
  }  // Receive calibration, LSB.  KF5N
  if (calTypeFlag == 0 && bands[EEPROMData.currentBand].mode == DEMOD_USB) {
    cal_bins[0] = 59;
    cal_bins[1] = 199;
  }  // Receive calibration, USB.  KF5N
  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
    cal_bins[0] = 257;  // LSB
    cal_bins[1] = 289;  // Carrier
    cal_bins[2] = 322;  // Undesired sideband
  }                     // Transmit and Carrier calibration, LSB.  KF5N
  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands[EEPROMData.currentBand].mode == DEMOD_USB) {
    cal_bins[0] = 257;  // USB
    cal_bins[1] = 225;  // Carrier
    cal_bins[2] = 193;  // Undesired sideband
  }                     // Transmit and Carrier calibration, LSB.  KF5N

  //  There are 2 for-loops, one for the reference signal and another for the undesired sideband.
  if (calTypeFlag == 0) {  // Receive cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
  }

  // Plot carrier during transmit cal, do not return a dB value:
  if (calTypeFlag == 1) {  // Transmit cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) PlotCalSpectrum(x1, cal_bins, capture_bins);  // Carrier
  }
  if (calTypeFlag == 2) {  // Carrier cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) PlotCalSpectrum(x1, cal_bins, capture_bins);  // Undesired sideband
  }

  // Finish up:
  //= AFP 2-11-23
  tft.fillRect(350, 125, 50, tft.getFontHeight(), RA8875_BLACK);  // Erase old adjdB number.
  tft.setCursor(350, 125);                                        // 350, 125
  tft.print(adjdB, 1);

  delay(5);  // This requires "tuning" in order to get best response.
}


/*****
  Purpose:  Plot Calibration Spectrum   //  KF5N 7/2/2023
            This function plots a partial spectrum during calibration only.
            This is intended to increase the efficiency and therefore the responsiveness of the calibration encoder.
            This function is called by ShowSpectrum2() in two for-loops.  One for-loop is for the reference signal,
            and the other for-loop is for the undesired sideband.
  Parameter list:
    int x1, where x1 is the FFT bin.
    cal_bins[3], locations of the desired and undesired signals
    capture_bins, width of the bins used to display the signals
  Return value;
    float, returns the adjusted value in dB
*****/
float PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins) {
  float adjdB = 0.0;
  int16_t adjAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  int16_t refAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  float alpha = 0.01;

  uint32_t index_of_max;  // This variable is not currently used, but it is required by the ARM max function.  KF5N
  int y_new_plot, y1_new_plot, y_old_plot, y_old2_plot;

  // The FFT should be performed only at the beginning of the sweep, and buffers must be full.
  if (x1 == (cal_bins[0] - capture_bins)) {  // Set flag at revised beginning.  KF5N
    updateDisplayFlag = true;                // This flag is used in ZoomFFTExe().
    ShowBandwidth();                         // Without this call, the calibration value in dB will not be updated.  KF5N
  } else updateDisplayFlag = false;          //  Do not save the the display data for the remainder of the sweep.

  ProcessIQData2();  // Call the Audio process from within the display routine to eliminate conflicts with drawing the spectrum.

  y_new = pixelnew[x1];
  y1_new = pixelnew[x1 - 1];
  y_old = pixelold[x1];
  y_old2 = pixelold[x1 - 1];

  // Find the maximums of the desired and undesired signals.

  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 0)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 0)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
  }

  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 1)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[2] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 1)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[2] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
  }

  if ((bands[EEPROMData.currentBand].mode == DEMOD_LSB) && (calTypeFlag == 2)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands[EEPROMData.currentBand].mode == DEMOD_USB) && (calTypeFlag == 2)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
  }

  //=== // AFP 2-11-23
  //  if (y_new > base_y) y_new = base_y;
  //  if (y_old > base_y) y_old = base_y;
  //  if (y_old2 > base_y) y_old2 = base_y;
  //  if (y1_new > base_y) y1_new = base_y;

  //  if (y_new < -140) y_new = -140;  // Bottom of expanded calibration spectrum display.
  //  if (y_old < -140) y_old = -140;
  //  if (y_old2 < -140) y_old2 = -140;
  //  if (y1_new < -140) y1_new = -140;

  y_old2_plot = 135 + (-y_old2 + rawSpectrumPeak);
  y_old_plot = 135 + (-y_old + rawSpectrumPeak);
  y1_new_plot = 135 + (-y1_new + rawSpectrumPeak);
  y_new_plot = 135 + (-y_new + rawSpectrumPeak);

  if (y_new_plot > base_y) y_new_plot = base_y;
  if (y_old_plot > base_y) y_old_plot = base_y;
  if (y_old2_plot > base_y) y_old2_plot = base_y;
  if (y1_new_plot > base_y) y1_new_plot = base_y;

  // Erase the old spectrum and draw the new spectrum.
  tft.drawLine(x1, y_old2_plot, x1, y_old_plot, RA8875_BLACK);   // Erase old...
  tft.drawLine(x1, y1_new_plot, x1, y_new_plot, RA8875_YELLOW);  // Draw new

  pixelCurrent[x1] = pixelnew[x1];  //  This is the actual "old" spectrum! Copied to pixelold by the FFT function.

  adjdB = ((float)adjAmplitude - (float)refAmplitude) / (1.95 * 2.0);  // Cast to float and calculate the dB level.  Needs further refinement for accuracy.  KF5N
  adjdB_avg = adjdB * alpha + adjdBold * (1.0 - alpha);                // Exponential average.
                                                                       //  adjdB_avg = adjdB;     // TEMPORARY EXPERIMENT
  adjdBold = adjdB_avg;
  adjdB_sample = adjdB;
  if (bands[EEPROMData.currentBand].mode == DEMOD_USB && not(calTypeFlag == 0)) adjdB_avg = -adjdB_avg;  // Flip sign for USB only for TX cal.

  tft.writeTo(L1);
  return adjdB_avg;
  //    return adjdB;  // TEMPORARY EXPERIMENT
}


/*****
  Purpose: Function pointer to select the transmit calibration tone frequency.  Possible values:
           0 = 750 Hz
           1 = 3 kHz

  Parameter list:
    void

  Return value:
    void
*****/
const char *calFreqs[2]{ "750 Hz", "3.0 kHz" };
FLASHMEM void SelectCalFreq() {
  EEPROMData.calFreq = SubmenuSelect(calFreqs, 2, EEPROMData.calFreq);  // Returns the index of the array.
  //  RedrawDisplayScreen();  Kills the bandwidth graphics in the audio display window, remove. KF5N July 30, 2023
  // Clear the current CW filter graphics and then restore the bandwidth indicator bar.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  RedrawDisplayScreen();
}
