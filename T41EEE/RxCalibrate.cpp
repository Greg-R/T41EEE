// Class Calibrate replaces Process2.cpp.  Greg KF5N June 16, 2024
// Class extensively modified to perform receive calibration only.  Greg KF5N August 2025

#include "SDT.h"

/*****
  Purpose: Load buffers used to modulate the transmitter during calibration.
          The epilogue must restore the buffers for normal operation!

   Parameter List:
      void

   Return value:
      void
 *****/
void RxCalibrate::loadCalToneBuffers(float toneFreq) {
  float theta;
  // This loop creates the sinusoidal waveform for the tone.
  for (int kf = 0; kf < 256; kf++) {
    theta = kf * 2.0 * PI * toneFreq / 24000;
    sinBuffer[kf] = sin(theta);
    cosBuffer[kf] = cos(theta);
  }
}


/*****
  Purpose: Plot calibration graphics (colored spectrum delimiter columns).
           New function, KF5N May 20, 2024.
  Parameter list:

  Return value:
    void
*****/
void RxCalibrate::plotCalGraphics() {
  tft.writeTo(L2);
  // Note, the spectrum bins begin at x = 2.
  tft.fillRect(rx_red_usb - capture_bins / 2 + 0, SPECTRUM_TOP_Y + 20, capture_bins, 341, DARK_RED);      // SPECTRUM_TOP_Y = 100, h = 135
  tft.fillRect(rx_blue_usb - capture_bins / 2 + 0, SPECTRUM_TOP_Y + 20, capture_bins, 341, RA8875_BLUE);  // h = SPECTRUM_HEIGHT + 3
  tft.writeTo(L1);
}


/*****
  Purpose: Run MakeFFTData() a few times to load and settle out buffers.  KF5N May 22, 2024
           Compute FFT in order to find maximum signal peak prior to beginning calibration.
  Parameter list:
    void

  Return value:
    void
*****/
void RxCalibrate::warmUpCal() {
  uint32_t index_of_max{ 0 };
  uint32_t count{ 0 };
  uint32_t i;
  // MakeFFTData() has to be called enough times for transients to settle out before computing FFT.
  //  delay(5000);
  for (i = 0; i < 128; i = i + 1) {
    fftActive = true;
    updateDisplayFlag = true;
    RxCalibrate::MakeFFTData();  // Note, FFT not called if buffers are not sufficiently filled.
    arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
    if (index_of_max > 380 and index_of_max < 388) {  // The peak is in the correct bin?
      count = count + 1;
    } else count = 0;       // Reset count in case of failure.
    if (count == 5) break;  // If five in a row, exit the loop.  Warm-up is complete.
  }
  updateDisplayFlag = true;    // This flag is used by the normal receiver process.
  fftActive = true;            // This is a flag local to this class.
  RxCalibrate::MakeFFTData();  // Now FFT will be calculated.
  updateDisplayFlag = false;
  fftActive = false;
  // Find peak of spectrum, which is 512 wide.  Use this to adjust spectrum peak to top of spectrum display.
  arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
  Serial.printf("RX rawSpectrumPeak = %d count = %d i = %d\n", rawSpectrumPeak, count, i);
  Serial.printf("RX index_of_max = %d\n", index_of_max);
  if (index_of_max < 380 or index_of_max > 388) Serial.printf("Problem with RX warmUpCal\n");
}


/*****
  Purpose: Print CW or SSB mode to display.

  Parameter list:
    void

  Return value:
    void
*****/
void RxCalibrate::PrintMode() {
  if (mode == 0) {
    tft.setCursor(left_text_edge + 120, 260);
    tft.print("CW");
  }
  if (mode == 1) {
    tft.setCursor(left_text_edge + 120, 260);
    tft.print("SSB");
  }
}


/*****
  Purpose: Print the calibration in progress to the display.  KF5N May 23, 2024
  
  Parameter list:
    void

  Return value:
    void
*****/
void RxCalibrate::printCalType(bool autoCal, bool autoCalDone) {
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED);
  tft.setCursor(left_text_edge, 260);
  tft.print("Receive");
  PrintMode();
  tft.setCursor(left_text_edge, 295);
  tft.print("Calibrate");
  if (autoCal) {
    tft.setCursor(left_text_edge, 330);
    tft.fillRect(left_text_edge, 330, 215, 40, RA8875_BLACK);
    if (autoCalDone) {
      tft.setTextColor(RA8875_GREEN);
      tft.print("Auto-Cal Mode");
    } else {
      tft.setTextColor(RA8875_RED);
      tft.print("Auto-Cal Mode");
    }
  } else {
    tft.setCursor(left_text_edge, 330);
    tft.fillRect(left_text_edge, 330, 215, 40, RA8875_BLACK);
    tft.print("Manual Mode");
  }
}


/*****
  Purpose: Set up prior to IQ calibrations.  New function.  KF5N August 14, 2023
  These things need to be saved here and restored in the epilogue function:
  Vertical scale in dB  (set to 10 dB during calibration)
  Zoom, set to 1X during receive calibration.
  Transmitter power, set to 5W during both calibrations.
   Parameter List:
      int setZoom   (This parameter should be 0 for receive (1X).

   Return value:
      void
 *****/
void RxCalibrate::CalibratePreamble(int setZoom) {
  //  cessb1.processorUsageMaxReset();
  controlAudioOut(ConfigData.audioOut, true);  // Mute all receiver audio.
  calOnFlag = true;
  exitManual = false;
  transmitPowerLevelTemp = ConfigData.transmitPowerLevel;  //AFP 05-11-23
  cwFreqOffsetTemp = ConfigData.CWOffset;
  // Remember the mode and state, and restore in the Epilogue.
  tempMode = bands.bands[ConfigData.currentBand].mode;
  tempState = radioState;
  // Calibrate requires upper or lower sideband.  Change if currently in an AM mode.  Put back in Epilogue.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    tempSideband = bands.bands[ConfigData.currentBand].sideband;
    // Use the last upper or lower sideband (CW, SSB, or FT8) during calibration.
    bands.bands[ConfigData.currentBand].sideband = ConfigData.lastSideband[ConfigData.currentBand];
  } else tempSideband = bands.bands[ConfigData.currentBand].sideband;
  ConfigData.CWOffset = 2;                   // 750 Hz for TX calibration.  Epilogue restores user selected offset.
  userZoomIndex = ConfigData.spectrum_zoom;  // Save the zoom index so it can be reset at the conclusion.  KF5N August 12, 2023
  zoomIndex = setZoom - 1;
  button.ButtonZoom();
  tft.fillRect(0, 272, 517, 399, RA8875_BLACK);  // Erase waterfall.  KF5N August 14, 2023

  tft.fillWindow();
  display.DrawSpectrumDisplayContainer();
  display.ShowFrequency();
  display.BandInformation();

  tft.writeTo(L2);  // Erase the bandwidth bar.  KF5N August 16, 2023
  tft.clearMemory();
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(left_text_edge, 175);
  tft.print("User1 - Gain/Phase");
  tft.setCursor(left_text_edge, 195);
  tft.print("User2 - Incr");
  tft.setCursor(left_text_edge, 215);
  tft.print("Zoom - Auto-Cal");
  tft.setCursor(left_text_edge, 235);
  tft.print("Filter - Refine-Cal");
  tft.setTextColor(RA8875_CYAN);
  ////  tft.fillRect(left_text_edge, 125, 100, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(left_text_edge + 90, 142);
  tft.setFontScale((enum RA8875tsize)1);
  tft.print("dB");
  tft.setTextColor(RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(left_text_edge, 125);
  tft.print("Incr = ");
  userScale = ConfigData.currentScale;  //  Remember user preference so it can be reset when done.  KF5N
  ConfigData.currentScale = 1;          //  Set vertical scale to 10 dB during calibration.  KF5N
  updateDisplayFlag = false;
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0;
  digitalWrite(MUTE, MUTEAUDIO);  //  Mute Audio  (HIGH=Mute)
  digitalWrite(RXTX, HIGH);       // Turn on transmitter.
  radioState = RadioState::RECEIVE_CALIBRATE_STATE;
  display.ShowTransmitReceiveStatus();
  rawSpectrumPeak = 0;
  SetAudioOperatingState(radioState);  // Do this last!  This clears the queues.
}


/*****
  Purpose: Shut down and clean up after IQ calibrations.  New function.  KF5N August 14, 2023

   Parameter List:
      void

   Return value:
      void
 *****/
void RxCalibrate::CalibrateEpilogue(bool radioCal, bool saveToEeprom) {
  /*
  Serial.printf("lastState=%d radioState=%d memory_used=%d memory_used_max=%d f32_memory_used=%d f32_memory_used_max=%d\n",
                lastState,
                radioState,
                (int)AudioStream::memory_used,
                (int)AudioStream::memory_used_max,
                (int)AudioStream_F32::f32_memory_used,
                (int)AudioStream_F32::f32_memory_used_max);
  AudioStream::memory_used_max = 0;
  AudioStream_F32::f32_memory_used_max = 0;
  Serial.printf("cessb1 max processor usage = %d\n", cessb1.processorUsageMax());
  */

  digitalWrite(RXTX, LOW);  // Turn off the transmitter.
  updateDisplayFlag = false;
  display.ShowTransmitReceiveStatus();
  // Clear queues to reduce transient.
  ADC_RX_I.end();
  ADC_RX_I.clear();
  ADC_RX_Q.end();
  ADC_RX_Q.clear();
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0;
  calibrateFlag = 0;                                       // KF5N
  ConfigData.CWOffset = cwFreqOffsetTemp;                  // Return user selected CW offset frequency.
  sineTone(ConfigData.CWOffset + 6);                       // This function takes "number of cycles" which is the offset + 6.
  ConfigData.currentScale = userScale;                     //  Restore vertical scale to user preference.  KF5N
                                                           ////  display.ShowSpectrumdBScale();
  ConfigData.transmitPowerLevel = transmitPowerLevelTemp;  // Restore the user's transmit power level setting.  KF5N August 15, 2023
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) bands.bands[ConfigData.currentBand].sideband = tempSideband;
  bands.bands[ConfigData.currentBand].mode = tempMode;
  zoomIndex = userZoomIndex - 1;
  button.ButtonZoom();                      // Restore the user's zoom setting.  Note that this function also modifies ConfigData.spectrum_zoom.
  if (saveToEeprom) eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
  tft.writeTo(L2);                          // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
  calOnFlag = false;
  if (not radioCal) display.RedrawAll();  // Redraw everything!
  else tft.fillWindow();                  // Clear the display.
  fftOffset = 0;                          // Some reboots may be caused by large fftOffset values when Auto-Spectrum is on.
  if ((MASTER_CLK_MULT_RX == 2) or (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  bands.bands[ConfigData.currentBand].sideband = tempSideband;  // Restore the sideband.
  radioState = tempState;
  lastState = RadioState::NOSTATE;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  SetAudioOperatingState(radioState);
  powerUp = true;  // Clip off transient.
  return;
}


/*****
  Purpose: Write the calibration factors to the CalData struct.

   Parameter List:
      float ichannel, float qchannel

   Return value:
      void
 *****/
void RxCalibrate::writeToCalData(float ichannel, float qchannel) {
  if (mode == 0) {  // CW
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
      CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
    }
  }
  if (mode == 1) {  // SSB
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
      CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
    }
  }
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the receiver IQ.

   Parameter List:
      int mode (0 is CW, 1 is SSB), bool radioCal, bool refineCal, bool saveToEeprom

   Return value:
      void
 *****/
void RxCalibrate::DoReceiveCalibrate(int calMode, bool radio, bool refine, bool toEeprom) {
  MenuSelect task = MenuSelect::DEFAULT;

  bool autoCal = false;

  RxCalibrate::mode = calMode;           // CW or SSB.  This is an object state variable.
  RxCalibrate::radioCal = radio;         // Initial calibration of all bands.
  RxCalibrate::refineCal = refine;       // Refinement (using existing values a starting point) calibration for all bands.
  RxCalibrate::saveToEeprom = toEeprom;  // Save to EEPROM

  loadCalToneBuffers(750.0);
  CalibratePreamble(0);      // Set zoom to 1X.
  int calFreqShift = 96000;  // Transmit frequency to 2 times IF, the image.
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(calFreqShift);
  plotCalGraphics();
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(left_text_edge + 60, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(left_text_edge + 60, 125);
  float increment = 0.002;  // Used in initial sweeps.
  tft.print(increment, 3);
  printCalType(autoCal, false);
  IQCalType = 0;                // Start with IG Gain calibration.
  warmUpCal();                  // Finds the peak of the FFT to adjust in display.
  State state = State::warmup;  // Start calibration state machine in warmup state.
  float maxSweepAmp = 0.2;
  float maxSweepPhase = 0.1;
  int averageCount = 0;
  float iOptimal = 1.0;
  float qOptimal = 0.0;
  std::vector<float32_t> sweepVector(201);
  std::vector<float32_t> sweepVectorValue(201);
  std::vector<float> sub_vectorAmp = std::vector<float>(21);
  std::vector<float> sub_vectorPhase = std::vector<float>(21);
  std::vector<float> sub_vectorAmpResult = std::vector<float>(21);
  std::vector<float> sub_vectorPhaseResult = std::vector<float>(21);
  int startTimer = 0;  // Used to time display of results.
  bool averageFlag = false;
  std::vector<float>::iterator result;
  // Get current values for amplitude and phase.  This is for refinement only.
  if (mode == 0) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }
  if (mode == 1) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }

  GetEncoderValueLive(-2.0, 2.0, phase, increment, (char *)"IQ Phase ", false, false);  // Show phase on display.

  if (radioCal) {
    autoCal = true;
    printCalType(autoCal, false);
    count = 0;
    warmup = 0;
    index = 1;
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Receive Calibration Loop
  while (true) {
    fftActive = true;
    RxCalibrate::ShowSpectrum();
    task = readButton();
    // Exit from manual calibration by button push.
    if (exitManual == true) {
      RxCalibrate::CalibrateEpilogue(radioCal, saveToEeprom);
      return;
    }
    //            if (exit) {
    //                Serial.printf("Audio memory usage = %d\n", AudioMemoryUsage());
    //        return;  //  exit variable set in buttonTasks() if button pushed by user.
    //        }
    //    if (refineCal) task = MenuSelect::FILTER;
    switch (task) {
      // Activate automatic calibration (initial calibration).
      case MenuSelect::ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
        state = State::warmup;
        averageFlag = false;
        break;
      // Automatic calibration using previously stored values (refine calibration).
      case MenuSelect::FILTER:  // 3rd row, 1st column button
        refineCal = true;
        autoCal = true;
        printCalType(autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
        averageFlag = false;
        state = State::warmup;
        break;
      // Toggle gain and phase adjustment in manual mode.
      case MenuSelect::UNUSED_1:
        if (IQCalType == 0) {
          IQCalType = 1;
          // Switching to phase, make gain white.
          GetEncoderValueLive(-2.0, 2.0, amplitude, increment, "IQ Gain ", true, false);
        } else {
          IQCalType = 0;
          // Switching go gain, make phase white.
          GetEncoderValueLive(-2.0, 2.0, phase, increment, "IQ Phase ", false, false);
        }
        break;
      // Toggle increment value
      case MenuSelect::BEARING:  // UNUSED_2 is now called BEARING
        corrChange = not corrChange;
        if (corrChange == true) {  // Toggle increment value
          increment = 0.001;       // AFP 2-11-23
        } else {
          increment = 0.01;  // AFP 2-11-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(left_text_edge + 60, 125, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(left_text_edge + 60, 125);
        tft.print(increment, 3);
        break;
      case MenuSelect::MENU_OPTION_SELECT:  // Save values and exit calibration.
        //        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        //        state = State::exit;
        //        autoCal = true;  // Set autoCal = true to access State::exit even during manual.
        exitManual = true;
        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal || radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(autoCal, false);
          count = 0;
          index = 0;
          IQCalType = 0;
          averageFlag = false;
          averageCount = 0;
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            phase = 0.0 + maxSweepPhase;    //  Need to use these values during warmup
            amplitude = 1.0 + maxSweepAmp;  //  so adjdB and adjdB_avg are forced upwards.
          }
          state = State::warmup;
          if (warmup == 16) state = State::state0;
          if (warmup == 16 && refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          IQCalType = 0;             // Start with IQ Gain.
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          phase = 0.0;
          amplitude = 1.0 - maxSweepAmp;                                                // Begin sweep at low end and move upwards.
                                                                                        ////          if (mode == 0)
          GetEncoderValueLive(-2.0, 2.0, phase, increment, "IQ Phase ", false, false);  // Display the phase value.
                                                                                        ////          if (mode == 1)
                                                                                        ////            GetEncoderValueLive(-2.0, 2.0, phase, increment, (char *)"IQ Phase", false);
          adjdB = 0;
          adjdB_avg = 0;
          index = 0;
          IQCalType = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index] = amplitude;
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for next measurement.
          amplitude = amplitude + increment;  // Next one!
          // Go to Q channel when I channel sweep is finished.
          if (abs(amplitude - 1.0) > maxSweepAmp) {                             // Needs to be subtracted from 1.0.
                                                                                ////            IQCalType = 1;                                                      // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            phase = -maxSweepPhase;                                             // The starting value for phase.
            amplitude = iOptimal;                                               // Reset for next sweep.
            // Update display to optimal value.
            GetEncoderValueLive(-2.0, 2.0, amplitude, increment, "IQ Gain ", true, false);
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 1;  // Prepare for phase.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = 0;
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            state = State::initialSweepPhase;  // Initial sweeps done; proceed to refine phase.
            break;
          }
          state = State::initialSweepAmp;  // This statement is not strictly necessary; leave here for clarity.
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index] = phase;
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for the next measurement.
          phase = phase + increment;
          if (phase > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);  // Input 2 is first right channel.
            qOptimal = sweepVectorValue[adjdBMinIndex];                  // Set to the discovered minimum.
            phase = qOptimal;                                            // Set to the discovered minimum.
            GetEncoderValueLive(-2.0, 2.0, phase, increment, "IQ Phase ", false, false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 0;
            adjdB = 0.0;
            state = State::refineAmp;  // Proceed to refine the gain channel.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            increment = 0.001;  // Use smaller increment in refinement.
            break;
          }
          state = State::initialSweepPhase;
          break;

        case State::refineAmp:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          amplitude = sub_vectorAmp[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorAmpResult[index] = adjdB_avg;
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorAmpResult.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorAmpResult.begin(), sub_vectorAmpResult.end());
            adjdBMinIndex = std::distance(sub_vectorAmpResult.begin(), result);
            // iOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            iOptimal = sub_vectorAmp[adjdBMinIndex];  // -.001;
            amplitude = iOptimal;                     // Set to optimal value before refining phase.
            GetEncoderValueLive(-2.0, 2.0, amplitude, increment, (char *)"IQ Gain ", true, true);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
            }
            IQCalType = 1;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 1 || count == 3) state = State::refinePhase;  // Alternate refinePhase and refineAmp.
            break;
          }
          //          avgState = averagingState::refineAmp;
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          phase = sub_vectorPhase[index];  // Starting value.
          // Don't record this until there is data.  So that will be AFTER this pass.
          if (averageFlag) {
            sub_vectorPhaseResult[index] = adjdB_avg;
            index = index + 1;
          }
          // Terminate when all values in sub_vectorAmp have been measured.
          if (index == sub_vectorPhaseResult.size()) {
            // Find the index of the minimum and record as iOptimal.
            result = std::min_element(sub_vectorPhaseResult.begin(), sub_vectorPhaseResult.end());
            adjdBMinIndex = std::distance(sub_vectorPhaseResult.begin(), result);
            // qOptimal is simply the value of sub_vectorAmp[adjdBMinIndex].
            index = 0;
            qOptimal = sub_vectorPhase[adjdBMinIndex];  // - .001;
            phase = qOptimal;
            GetEncoderValueLive(-2.0, 2.0, phase, increment, "IQ Phase ", false, false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 0;
            averageFlag = 0;
            averageCount = 0;
            count = count + 1;
            state = State::refineAmp;
            if (count == 2) state = State::refineAmp;
            if (count == 4) state = State::setOptimal;
            break;
          }
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 3) {
            if (IQCalType == 0) state = State::refineAmp;
            if (IQCalType == 1) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          averageCount = averageCount + 1;
          averageFlag = false;
          if (IQCalType == 0) state = State::refineAmp;
          if (IQCalType == 1) state = State::refinePhase;
          break;

        case State::setOptimal:
          count = 0;  // In case automatic calibration is run again.
          amplitude = iOptimal;
          phase = qOptimal;
          writeToCalData(amplitude, phase);
          state = State::exit;
          startTimer = static_cast<int>(milliTimer);  // Start result view timer.
          break;
        case State::exit:
          // Delay exit if in radio calibration to show calibration results for 5 seconds.
          if (radioCal) {
            printCalType(autoCal, true);
            if ((static_cast<int>(milliTimer) - startTimer) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              RxCalibrate::CalibrateEpilogue(radioCal, saveToEeprom);
              return;
            }
          } else {
            autoCal = false;  // Don't enter switch, but remain in manual loop.
            printCalType(autoCal, false);
          }
          break;
      }
    }  // end automatic calibration state machine

    //    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;  // Reset task after it is used.

    //  Read encoder and update values.  This is manual calibration.
    if (IQCalType == 0) amplitude = GetEncoderValueLive(-2.0, 2.0, amplitude, increment, "IQ Gain ", true, true);
    if (IQCalType == 1) phase = GetEncoderValueLive(-2.0, 2.0, phase, increment, "IQ Phase ", false, true);
    writeToCalData(amplitude, phase);
  }  // end while
}  // End Receive calibration


/*****
  Purpose: Signal processing for the purpose of calibration.
           This operates at 192ksps and is therefore not compatible with TX calibration.

   Parameter List:
      none

   Return value:
      void
 *****/
void RxCalibrate::MakeFFTData() {
  float rfGainValue, powerScale;  // AFP 2-11-23.  Greg KF5N February 13, 2023
                                  //  float recBandFactor[7] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };  // AFP 2-11-23  KF5N uniform values

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are read into two arrays sp_L and sp_R in blocks of 128 up to N_BLOCKS.  The arrays are
        of size BUFFER_SIZE * N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_LENGTH / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_LENGTH = 512
        BUFFER_SIZE*N_BLOCKS = 2048 samples
     **********************************************************************************/
  // Generate I and Q for receive calibration.  Greg Raven KF5N October 2025
  arm_scale_f32(cosBuffer, 0.20, float_buffer_L_EX, 256);  // AFP 2-11-23 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32(sinBuffer, 0.20, float_buffer_R_EX, 256);

  // Transmitter calibration parameters.  These will be approximations, and good enough for this purpose.
  if (mode == 0) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      arm_scale_f32(float_buffer_L_EX, -CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L_EX, 256);       //Adjust level of L buffer // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand], 256);  // Adjust phase
    }
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      arm_scale_f32(float_buffer_L_EX, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L_EX, 256);  // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 256);
    }
  }
  if (mode == 1) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      arm_scale_f32(float_buffer_L_EX, -CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L_EX, 256);       //Adjust level of L buffer // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand], 256);  // Adjust phase
    }
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      arm_scale_f32(float_buffer_L_EX, CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L_EX, 256);  // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand], 256);
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
  powerScale = 40.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#else
  powerScale = 30.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#endif

  //  192KHz effective sample rate here
  arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048);  //Scale to compensate for losses in Interpolation
  arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

  // This code block was introduced after TeensyDuino 1.58 appeared.  It doesn't use a for loop, but processes the entire 2048 buffer in one pass.
  // Revised I and Q calibration signal generation using large buffers.  Greg KF5N June 4 2023
  //  if (static_cast<uint32_t>(ADC_RX_I.available()) > 16 && static_cast<uint32_t>(ADC_RX_Q.available()) > 16) {  // Audio Record Queues!!!
  //  q15_t q15_buffer_LTemp[2048];  //KF5N
  //  q15_t q15_buffer_RTemp[2048];  //KF5N

  //  arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
  //  arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);

#ifdef QSE2
  if (mode == 0) {
    arm_offset_f32(float_buffer_L_EX, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_L_EX, 2048);
    arm_offset_f32(float_buffer_R_EX, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_R_EX, 2048);
  }
  if (mode == 1) {
    arm_offset_f32(float_buffer_L_EX, CalData.iDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, float_buffer_L_EX, 2048);  // Carrier suppression offset.
    arm_offset_f32(float_buffer_R_EX, CalData.qDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, float_buffer_R_EX, 2048);
  }
#endif

  Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
  Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
  //  Q_out_L_Ex.play(q15_buffer_LTemp, 2048);
  //  Q_out_R_Ex.play(q15_buffer_RTemp, 2048);
  Q_out_L_Ex.play(float_buffer_L_EX, 2048);
  Q_out_R_Ex.play(float_buffer_R_EX, 2048);


  // }   // End of transmit code.  Begin receive code.

  // Get I16 audio blocks from the record queues and convert them to float.
  // Read in 16 blocks of 128 samples in I and Q if available.
  if (static_cast<uint32_t>(ADC_RX_I.available()) > 16 and static_cast<uint32_t>(ADC_RX_Q.available()) > 16) {
    for (unsigned i = 0; i < 16; i++) {
      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float(ADC_RX_Q.readBuffer(), &float_buffer_L[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      arm_q15_to_float(ADC_RX_I.readBuffer(), &float_buffer_R[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      ADC_RX_I.freeBuffer();
      ADC_RX_Q.freeBuffer();
    }

    rfGainValue = pow(10, (float)ConfigData.rfGain[ConfigData.currentBand] / 20);        //AFP 2-11-23
    arm_scale_f32(float_buffer_L, rfGainValue, float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23
    arm_scale_f32(float_buffer_R, rfGainValue, float_buffer_R, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23

    /**********************************************************************************  AFP 12-31-20
      Scale the data buffers by the RFgain value defined in bands.bands[ConfigData.currentBand] structure
    **********************************************************************************/
    //    arm_scale_f32(float_buffer_L, recBandFactor[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23
    //    arm_scale_f32(float_buffer_R, recBandFactor[ConfigData.currentBand], float_buffer_R, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23

    // IQ amplitude and phase correction.  Mode 0 is for CW and Mode 1 is for SSB.
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      }
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        arm_scale_f32(float_buffer_L, CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      }
    }
    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      }
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        arm_scale_f32(float_buffer_L, CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      }
    }

    // This process started because there are 2048 samples available.  Perform FFT.
    updateDisplayFlag = true;
    if (fftActive) CalcZoom1Magn();  // Receiver calibration uses 1X zoom.
    FreqShift1();                    // 48 kHz shift
    fftSuccess = true;
  }  // End of receive code
  else {
    fftSuccess = false;  // Insufficient receive buffers to make FFT.  Do not plot FFT data!
    Serial.printf("FFT failed!\n");
  }
}  // end MakeFFTData()


/*****
  Purpose: Show Spectrum display modified for IQ calibration.
           This is similar to the function used for normal reception, however, it has
           been simplified and streamlined for calibration.

  Parameter list:
    void

  Return value;
    void
*****/
void RxCalibrate::ShowSpectrum()  //AFP 2-10-23
{
  int x1 = 0;
  //  int capture_bins = 8;  // Sets the number of bins to scan for signal peak.

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
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    cal_bins[0] = rx_blue_usb;  // was 315
    cal_bins[1] = rx_red_usb;
  }  // Receive calibration, LSB.  KF5N
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    cal_bins[0] = rx_blue_usb;  // was 315
    cal_bins[1] = rx_red_usb;
  }  // Receive calibration, USB.  KF5N

  //  There are 2 for-loops, one for the reference signal and another for the undesired sideband.
  for (x1 = cal_bins[0] - capture_bins / 2; x1 < cal_bins[0] + capture_bins / 2; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
  for (x1 = cal_bins[1] - capture_bins / 2; x1 < cal_bins[1] + capture_bins / 2; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);

  tft.setCursor(left_text_edge, 142);
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(left_text_edge, 142, 80, tft.getFontHeight(), RA8875_BLACK);  // Erase old adjdB number.                                       // 350, 125
  tft.print(adjdB, 1);
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
float RxCalibrate::PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins) {
  int16_t adjAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  int16_t refAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  float alpha = 0.01;

  uint32_t index_of_max;  // This variable is not currently used, but it is required by the ARM max function.  KF5N
  int y_new_plot, y1_new_plot, y_old_plot, y_old2_plot;

  // The FFT should be performed only at the beginning of the sweep, and buffers must be full.
  if (x1 == (cal_bins[0] - capture_bins / 2)) {  // Set flag at revised beginning.  KF5N
    updateDisplayFlag = true;                    // This flag is used in ZoomFFTExe().
    RxCalibrate::MakeFFTData();                  // Compute FFT and draw it on the display.
  } else updateDisplayFlag = false;              //  Do not save the the display data for the remainder of the sweep.

  // Call the Audio process from within the display routine to eliminate conflicts with drawing the spectrum.

  y_new = pixelnew[x1];
  y1_new = pixelnew[x1 - 1];
  y_old = pixelold[x1];
  y_old2 = pixelold[x1 - 1];

  // Find the maximums of the desired and undesired signals.

  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins / 2)], capture_bins, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins / 2)], capture_bins, &adjAmplitude, &index_of_max);
  }
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins / 2)], capture_bins, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins / 2)], capture_bins, &adjAmplitude, &index_of_max);
  }

  y_old2_plot = 135 + (-y_old2 + rawSpectrumPeak);
  y_old_plot = 135 + (-y_old + rawSpectrumPeak);
  y1_new_plot = 135 + (-y1_new + rawSpectrumPeak);
  y_new_plot = 135 + (-y_new + rawSpectrumPeak);

  // Prevent spectrum from going above the top of the spectrum area.  KF5N
  if (y_new_plot < 120) y_new_plot = 120;
  if (y1_new_plot < 120) y1_new_plot = 120;
  if (y_old_plot < 120) y_old_plot = 120;
  if (y_old2_plot < 120) y_old2_plot = 120;

  // The prevents spectrum from going below lower limit.
  if (y_new_plot > base_y) y_new_plot = base_y;
  if (y_old_plot > base_y) y_old_plot = base_y;
  if (y_old2_plot > base_y) y_old2_plot = base_y;
  if (y1_new_plot > base_y) y1_new_plot = base_y;

  // Erase the old spectrum and draw the new spectrum.
  tft.drawLine(x1 + 0, y_old2_plot, x1 + 0, y_old_plot, RA8875_BLACK);   // Erase old...
  tft.drawLine(x1 + 0, y1_new_plot, x1 + 0, y_new_plot, RA8875_YELLOW);  // Draw new

  //  pixelCurrent[x1] = pixelnew[x1];  //  This is the actual "old" spectrum! Copied to pixelold by the FFT function.

  adjdB = (static_cast<float>(adjAmplitude) - static_cast<float>(refAmplitude)) / (1.95 * 2.0);  // Cast to float and calculate the dB level.  Needs further refinement for accuracy.  KF5N
  adjdB_avg = adjdB * alpha + adjdBold * (1.0 - alpha);                                          // Exponential average.

  adjdBold = adjdB_avg;

  tft.writeTo(L1);
  return adjdB_avg;
}
