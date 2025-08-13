// Class TxCalibrate.  Greg KF5N July 10, 2024

// loadCalToneBuffers(float toneFreq)
// plotCalGraphics(int calType)
// warmUpCal(int mode)
// printCalType(bool autoCal, bool autoCalDone)
// CalibratePreamble(int setZoom)
// CalibrateEpilogue(bool radioCal, bool saveToEeprom)
// DoReceiveCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// DoXmitCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// DoXmitCarrierCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// RadioCal(bool refineCal)
// ProcessIQData(int mode)
// ShowSpectrum(int mode)
// PlotCalSpectrum(int mode, int x1, int cal_bins[3], int capture_bins)

#include "SDT.h"


/*****
  Purpose: Load buffers used to modulate the transmitter during calibration.
          The epilogue must restore the buffers for normal operation!

   Parameter List:
      void

   Return value:
      void
 *****/
void TxCalibrate::loadCalToneBuffers(float toneFreq) {
  float theta;
  // This loop creates the sinusoidal waveform for the tone.
  for (int kf = 0; kf < 512; kf++) {
    theta = kf * 2.0 * PI * toneFreq / 24000;
    sineBuffer[kf] = sin(theta);
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
void TxCalibrate::plotCalGraphics(int calType) {
  //  int bar_width = 8;
  tft.writeTo(L2);

  if (calType == 1) {  // Transmit Cal
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      tft.fillRect(312, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    } else {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        tft.fillRect(183, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);
        tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
      }
    }
  }
  if (calType == 2) {  // Carrier Cal
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      tft.fillRect(279, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    } else {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {  //mode == DEMOD_LSB
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
void TxCalibrate::warmUpCal() {
  // Run ProcessIQData2() a few times to load and settle out buffers.  Compute FFT.  KF5N May 19, 2024
  uint32_t index_of_max;  // Not used, but required by arm_max_q15 function.
  for (int i = 0; i < 16; i = i + 1) {
    updateDisplayFlag = true;  // Causes FFT to be calculated.
    while (static_cast<uint32_t>(Q_in_L_Ex.available()) < 32 and static_cast<uint32_t>(ADC_RX_I.available()) < 32) {
      delay(1);
    }
    TxCalibrate::MakeFFTData();  // Note, FFT not called if buffers are not sufficiently filled.
  }
  updateDisplayFlag = false;
  // Find peak of spectrum, which is 512 wide.  Use this to adjust spectrum peak to top of spectrum display.
  arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
  Serial.printf("rawSpectrumPeak = %d\n", rawSpectrumPeak);
  //  rawSpectrumPeak = 193;
}


void TxCalibrate::PrintMode() {
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    if (mode == 0) {
      tft.setCursor(170, 260);
      tft.print("CW");
    }
    if (mode == 1) {
      tft.setCursor(170, 260);
      tft.print("SSB");
    }
  }
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    if (mode == 0) {
      tft.setCursor(425, 260);
      tft.print("CW");
    }
    if (mode == 1) {
      tft.setCursor(425, 260);
      tft.print("SSB");
    }
  }
}


/*****
  Purpose: Print the calibration type to the display.  KF5N May 23, 2024
  
  Parameter list:
    void
    REMOVE IQCalType parameter, not necessary here.  Also remove method variable!

  Return value:
    void
*****/
void TxCalibrate::printCalType(bool autoCal, bool autoCalDone) {
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED);
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) and (calTypeFlag == 1)) {
    tft.setCursor(30, 260);
    tft.print("Transmit");
    PrintMode();
    tft.setCursor(30, 295);
    tft.print("Calibrate");
    if (autoCal) {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) and (calTypeFlag == 1)) {
    tft.setCursor(290, 260);
    tft.print("Transmit");
    PrintMode();
    tft.setCursor(290, 295);
    tft.print("Calibrate");
    if (autoCal) {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) and (calTypeFlag == 2)) {
    tft.setCursor(30, 260);
    tft.print("Carrier");
    PrintMode();
    tft.setCursor(30, 295);
    tft.print("Calibrate");
    if (autoCal) {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(30, 330);
      tft.fillRect(30, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) and (calTypeFlag == 2)) {
    tft.setCursor(290, 260);
    tft.print("Carrier");
    PrintMode();
    tft.setCursor(290, 295);
    tft.print("Calibrate");
    if (autoCal) {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(290, 330);
      tft.fillRect(290, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
}


/*****
  Purpose: Set up prior to IQ calibrations.  New function.  KF5N August 14, 2023
  These things need to be saved here and restored in the epilogue function:
  Vertical scale in dB  (set to 10 dB during calibration)
  Zoom, set to 1X in receive and 4X in transmit calibrations.
  Transmitter power, set to 5W during both calibrations.
   Parameter List:
      int setZoom   (This parameter should be 0 for receive (1X) and 2 (4X) for transmit)

   Return value:
      void
 *****/
void TxCalibrate::CalibratePreamble(int setZoom) {
  cessb1.processorUsageMaxReset();
  calOnFlag = true;
  IQCalType = 0;
  transmitPowerLevelTemp = ConfigData.transmitPowerLevel;  //AFP 05-11-23
  cwFreqOffsetTemp = ConfigData.CWOffset;
  // Remember the mode and state, and restore in the Epilogue.
  tempMode = bands.bands[ConfigData.currentBand].mode;
  tempState = radioState;
  if (mode == 0)
    bands.bands[ConfigData.currentBand].mode = RadioMode::CW_MODE;
  else
    bands.bands[ConfigData.currentBand].mode = RadioMode::SSB_MODE;
  // Calibrate requires upper or lower sideband.  Change if currently in an AM mode.  Put back in Epilogue.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    tempSideband = bands.bands[ConfigData.currentBand].sideband;
    // Use the last upper or lower sideband.
    bands.bands[ConfigData.currentBand].sideband = ConfigData.lastSideband[ConfigData.currentBand];
  }
  ConfigData.CWOffset = 2;                   // 750 Hz for TX calibration.  Epilogue restores user selected offset.
                                             //  userxmtMode = ConfigData.xmtMode;          // Store the user's mode setting.  KF5N July 22, 2023
  userZoomIndex = ConfigData.spectrum_zoom;  // Save the zoom index so it can be reset at the conclusion.  KF5N August 12, 2023
  zoomIndex = setZoom - 1;
  button.ButtonZoom();
  tft.fillRect(0, 272, 517, 399, RA8875_BLACK);  // Erase waterfall.  KF5N August 14, 2023

  tft.fillWindow();
  DrawSpectrumDisplayContainer();
  ShowFrequency();
  BandInformation();

  tft.writeTo(L2);  // Erase the bandwidth bar.  KF5N August 16, 2023
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
  tft.setCursor(350, 205);
  tft.print("Filter - Refine-Cal");
  tft.setTextColor(RA8875_CYAN);
  tft.fillRect(350, 125, 100, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 140);
  tft.print("dB");
  tft.setCursor(350, 125);
  tft.print("Incr = ");
  userScale = ConfigData.currentScale;  //  Remember user preference so it can be reset when done.  KF5N
  ConfigData.currentScale = 1;          //  Set vertical scale to 10 dB during calibration.  KF5N
  updateDisplayFlag = false;
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  digitalWrite(MUTE, MUTEAUDIO);  //  Mute Audio  (HIGH=Mute)
  digitalWrite(RXTX, HIGH);       // Turn on transmitter.
  rawSpectrumPeak = 0;
  radioState = RadioState::SSB_CALIBRATE_STATE;
  ShowTransmitReceiveStatus();
  SetAudioOperatingState(radioState);  // Do this last!  This turns the queues on.
}


/*****
  Purpose: Shut down and clean up after IQ calibrations.  New function.  KF5N August 14, 2023

   Parameter List:
      void

   Return value:
      void
 *****/
void TxCalibrate::CalibrateEpilogue() {
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
 */

  digitalWrite(RXTX, LOW);  // Turn off the transmitter.
  updateDisplayFlag = false;
  SampleRate = SAMPLE_RATE_192K;  // Return to receiver sample rate.
  SetI2SFreq(SR[SampleRate].rate);
  InitializeDataArrays();  // Re-initialize the filters back to 192ksps.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) bands.bands[ConfigData.currentBand].sideband = tempSideband;
  bands.bands[ConfigData.currentBand].mode = tempMode;
  radioState = tempState;
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  calibrateFlag = 0;                                       // KF5N
  ConfigData.CWOffset = cwFreqOffsetTemp;                  // Return user selected CW offset frequency.
  sineTone(ConfigData.CWOffset + 6);                       // This function takes "number of cycles" which is the offset + 6.
  ConfigData.currentScale = userScale;                     //  Restore vertical scale to user preference.  KF5N
  ConfigData.transmitPowerLevel = transmitPowerLevelTemp;  // Restore the user's transmit power level setting.  KF5N August 15, 2023
  zoomIndex = userZoomIndex - 1;
  button.ButtonZoom();                      // Restore the user's zoom setting.  Note that this function also modifies ConfigData.spectrum_zoom.
  if (TxCalibrate::saveToEeprom) eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
  tft.writeTo(L2);                          // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
  calOnFlag = false;
  if (not TxCalibrate::radioCal) RedrawDisplayScreen();  // Redraw everything!
  else tft.fillWindow();                    // Clear the display.
  fftOffset = 0;                            // Some reboots may be caused by large fftOffset values when Auto-Spectrum is on.
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  lastState = RadioState::NOSTATE;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  powerUp = true;
}


//void TxCalibrate::buttonTasks(bool radioCal, bool refineCal) {  // Does this function need parameters?
void TxCalibrate::buttonTasks() {
  //  task = readButton(lastUsedTask);
  //TxCalibrate::radioCal = radioCal;
  //TxCalibrate::refineCal = refineCal;
  task = readButton();
  //  if (refineCal) task = MenuSelect::FILTER;  // Refine calibration.
  switch (task) {
    // Activate initial automatic calibration.
    case MenuSelect::ZOOM:  // 2nd row, 1st column button
      TxCalibrate::autoCal = true;
      TxCalibrate::refineCal = false;
      printCalType(TxCalibrate::autoCal, false);
      count = 0;
      warmup = 0;
      index = 0;
      IQCalType = 0;
      averageFlag = false;
      averageCount = 0;
      std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
      std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
      amplitude = 1.0;
      phase = 0.0;
      state = State::warmup;
      break;
    // Automatic calibration using previously stored values.
    case MenuSelect::FILTER:  // 3rd row, 1st column button
      TxCalibrate::autoCal = true;
      TxCalibrate::refineCal = true;
      printCalType(autoCal, false);
      count = 0;
      warmup = 0;
      index = 1;  // Why is index = 1???
      IQCalType = 0;
      std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
      std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
      //      amplitude = ;
      //      phase = ;
      averageFlag = false;
      state = State::warmup;
      break;
    // Toggle gain and phasei in manual mode.
    case MenuSelect::UNUSED_1:
      if (IQCalType == 0) IQCalType = 1;
      else IQCalType = 0;
      break;
    // Toggle increment value
    case MenuSelect::BEARING:  // UNUSED_2 is now called BEARING
      corrChange = not corrChange;
      if (corrChange == true) {       // Toggle increment value
        correctionIncrement = 0.001;  // AFP 2-11-23
      } else {
        correctionIncrement = 0.01;  // AFP 2-11-23
      }
      tft.setFontScale((enum RA8875tsize)0);
      tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
      tft.setCursor(405, 125);
      tft.print(correctionIncrement, 3);
      break;
    case MenuSelect::MENU_OPTION_SELECT:  // Save values and exit calibration.
//      tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
      CalibrateEpilogue();
      exitManual = true;
      break;
    default:
      break;
  }  // end switch
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int mode, bool radioCal, bool refineCal, bool saveToEeprom

   Return value:
      void
 *****/
void TxCalibrate::DoXmitCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom) {
  exitManual = false;
  int freqOffset;
  float correctionIncrement = 0.001;
  //  State state = State::warmup;  // Start calibration state machine in warmup state.
  float maxSweepAmp = 0.1;
  float maxSweepPhase = 0.1;
  float increment = 0.002;
  //  int averageCount = 0;
  IQCalType = 0;  // Begin with IQ gain optimization.
                  //  float iOptimal = 1.0;
                  //  float qOptimal = 0.0;
                  //  std::vector<float32_t> sweepVector(101);
                  //  std::vector<float32_t> sweepVectorValue(101);
  elapsedMillis fiveSeconds;
  int viewTime = 0;
  //  bool autoCal = false;
  //  bool refineCal = false;
  //  bool averageFlag = false;
  TxCalibrate::mode = mode;                  // CW or SSB
  TxCalibrate::radioCal = radioCal;          // Initial calibration of all bands.
  TxCalibrate::refineCal = refineCal;        // Refinement (using existing values a starting point) calibration for all bands.
  TxCalibrate::saveToEeprom = saveToEeprom;  // Save to EEPROM
  std::vector<float>::iterator result;

  //  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  TxCalibrate::CalibratePreamble(2);  // Set zoom to 4X.  Sample rate 48ksps.
  freqOffset = 0;                     // Calibration tone same as regular modulation tone.
  calTypeFlag = 1;                    // TX cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(correctionIncrement, 3);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) {
    ResetFlipFlops();
  } else {
    delay(1000);
  }
  SetFreqCal(freqOffset);
  printCalType(autoCal, false);
  // Get current values into the iOptimal and qOptimal amplitude and phase working variables.
  if (mode == 0) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  } else {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }
  // Run this so Phase shows from begining.  Get the value for the current sideband.
  GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, "IQ Phase", false);
  warmUpCal();

  if (radioCal) {
    autoCal = true;
    printCalType(autoCal, false);
    count = 0;
    warmup = 0;
    index = 1;  // Why is index = 1???
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Transmit Calibration Loop
  while (true) {
    //    Serial.printf("top of loop\n");
    // Serial.printf("amplitude = %f phase = %f\n", amplitude, phase);
//    TxCalibrate::ShowSpectrum();
    if (state == State::initialSweepAmp or state == State::initialSweepPhase) {
//      fftActive = false;
      TxCalibrate::MakeFFTData();
      TxCalibrate::MakeFFTData();
 //     TxCalibrate::MakeFFTData();
//      TxCalibrate::MakeFFTData();
//      TxCalibrate::MakeFFTData();
    }
    fftActive = true;
    TxCalibrate::ShowSpectrum();
    //    TxCalibrate::buttonTasks(radioCal, refineCal);  // This function takes care of button presses and resultant control of the rest of the process.
    // The buttons are polled by the while loop.
    TxCalibrate::buttonTasks();  // This takes care of manual calls to the initial or refinement calibrations.
    //  Begin automatic calibration state machine.
    if (autoCal or radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(autoCal, false);
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not TxCalibrate::refineCal) {
            phase = 0.0 + maxSweepPhase;    //  Need to use these values during warmup
            amplitude = 1.0 + maxSweepAmp;  //  so adjdB and adjdB_avg are forced upwards.
          }
          state = State::warmup;
          if (warmup == 10) state = State::state0;  // Proceed with initial calibration.
          if (warmup == 10 and TxCalibrate::refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          Serial.printf("Entered case refineCal\n");
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          IQCalType = 0;  // Start in IQ Gain.
          index = 0;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for initial calibration sweeps.  First sweep is amplitude (gain).
          phase = 0.0;                                                                    // Hold phase at 0.0 while amplitude sweeps.
          amplitude = 1.0 - maxSweepAmp;                                                  // Begin sweep at low end and move upwards.
          GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, "IQ Phase", false);  // Display phase value during amplitude sweep.
          adjdB = 0;
          adjdB_avg = 0;
          index = 0;
          IQCalType = 0;                   // IQ Gain
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          //         Serial.printf("Entered initialSweepAmp state\n");
          sweepVectorValue[index] = amplitude;
          sweepVector[index] = adjdB;
          // Increment for next measurement.
          index = index + 1;
          amplitude = amplitude + increment;  // Next one!
          // Done with initial sweep, move to initial sweep of phase.
          if (abs(amplitude - 1.0) > maxSweepAmp) {                             // Needs to be subtracted from 1.0.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
                                                                                //            Serial.printf("Init The optimal amplitude = %.3f at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            amplitude = iOptimal;                                               // Set amplitude to the discovered optimal value.
            Serial.printf("iOptimal = %f phase =%f\n", amplitude, phase);
            // Update display to optimal value.
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, "IQ Gain", true);
            //            Serial.printf("Stop at end of amplitude initial sweep.\n");
            //            Serial.printf("amplitude = %f phase = %f\n", amplitude, phase);
            //            delay(10000);
            //            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 1;           // Prepare for phase.
            phase = -maxSweepPhase;  // The starting value for phase.
            index = 0;
            //            averageFlag = false;
            //            averageCount = 0;
            //            count = 0;  // Used to switch from amplitude to phase refinement.
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            state = State::initialSweepPhase;  // Initial sweeps done; proceed to refine phase.
            break;
          }
          state = State::initialSweepAmp;
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index] = phase;
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for the next measurement.
          phase = phase + increment;
          if (phase > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            qOptimal = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            phase = qOptimal;                            // Set to the discovered minimum.

            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, "IQ Phase", false);
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 0;
            adjdB = 0.0;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            increment = 0.001;         // Use smaller increment in refinement.
            state = State::refineAmp;  // Proceed to refine the gain channel.
            break;
          }
          state = State::initialSweepPhase;
          break;

        case State::refineAmp:
          //          Serial.printf("Entered refineAmp state\n");
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
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, "IQ Gain", true);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
            }
            //            for(int i = 0; i < 21; i = i + 1) {
            //              Serial.printf("sub_vectorAmp[%d] = %f\n", i, sub_vectorAmp[i]);
            //            }
            IQCalType = 1;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 1 or count == 3) state = State::refinePhase;  // Alternate refinePhase and refineAmp.

            // if(count == 1) state = State::exit;

            break;
          }
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          phase = sub_vectorPhase[index];  // Starting value.
                                           //         state = State::exit;
                                           //         break;
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
            qOptimal = sub_vectorPhase[adjdBMinIndex];
            phase = qOptimal;
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, "IQ Phase", false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);
            }
            IQCalType = 0;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 2) state = State::refineAmp;
            if (count == 4) state = State::setOptimal;
            break;
          }
          //          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.  Used for refinement only.
          if (averageCount > 5) {
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
                      //          amplitude = iOptimal;  // Just use iOptimal!!!
                      //          phase = qOptimal;
          // Write the optimal values to the data structure.
          if (mode == 0) {
            if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
              CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand] = iOptimal;
              CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand] = qOptimal;
            } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
              CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand] = iOptimal;
              CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand] = qOptimal;
            }
          }
          if (mode == 1) {
            if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
              CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand] = iOptimal;
              CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand] = qOptimal;
            } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
              CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand] = iOptimal;
              CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand] = qOptimal;
            }
          }
          state = State::exit;
          viewTime = fiveSeconds;  // Start result view timer.
          break;
        case State::exit:
          if (radioCal) {
            printCalType(autoCal, true);
            if ((fiveSeconds - viewTime) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              CalibrateEpilogue();  // After 5 seconds.
              return;
            }
          }
          autoCal = false;
          refineCal = false;  // Reset refine cal.
          printCalType(autoCal, false);
          break;
      }
    }  // end automatic calibration state machine

    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;                            // Reset task after it is used.
                                                           //  Read encoder and update values.
    if (IQCalType == 0) amplitude = GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, "IQ Gain", true);
    if (IQCalType == 1) phase = GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, "IQ Phase", false);
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
    }
    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
    }

    if (exitManual) break;  //  Exit the while loop.
  }                   // end while

}  // End Transmit calibration


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int mode, bool radioCal, bool refineCal, bool saveToEeprom

   Return value:
      void
 *****/
#ifdef QSE2
void TxCalibrate::DoXmitCarrierCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom) {
  //  int task = -1;
  //  int lastUsedTask = -2;
  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  bool exit = false;
  int freqOffset;
  int correctionIncrement = 10;
  State state = State::warmup;  // Start calibration state machine in warmup state.
  int maxSweepAmp = 450;
  int maxSweepPhase = 450;
  int increment = 5;
  //  uint32_t adjdBMinIndex;
  int averageCount = 0;
  int iOptimal = 0;
  int qOptimal = 0;
  std::vector<float> sweepVector(181);
  std::vector<int> sweepVectorValue(181);
  std::vector<int> sub_vectorIoffset = std::vector<int>(21);
  std::vector<int> sub_vectorQoffset = std::vector<int>(21);
  elapsedMillis fiveSeconds;
  int viewTime = 0;

  std::vector<float>::iterator result;
  TxCalibrate::mode = mode;
  TxCalibrate::CalibratePreamble(2);  // Set zoom to 16X.
  freqOffset = 0;                     // Calibration tone same as regular modulation tone.
  calTypeFlag = 2;                    // Carrier calibration
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(correctionIncrement);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  radioState = RadioState::SSB_TRANSMIT_STATE;
  SetFreqCal(freqOffset);
  printCalType(autoCal, false);
  // Get current values into the iDCoffset and qDCoffset working variables.

  if (mode == 0) {
    iDCoffset = CalData.iDCoffsetCW[ConfigData.currentBand];
    qDCoffset = CalData.qDCoffsetCW[ConfigData.currentBand];
  }
  if (mode == 1) {
    iDCoffset = CalData.iDCoffsetSSB[ConfigData.currentBand];
    qDCoffset = CalData.qDCoffsetSSB[ConfigData.currentBand];
  }

  // Run this so Phase shows from begining.
  if (mode == 0)
    GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, "Q Offset", false);
  if (mode == 1)
    GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetSSB[ConfigData.currentBand], correctionIncrement, "Q Offset", false);
  warmUpCal();
  //  delay(5000);

  if (radioCal) {
    autoCal = true;
    printCalType(autoCal, false);
    count = 0;
    warmup = 0;
    index = 0;
    //  stopSweep = false;
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Transmit Calibration Loop
  while (true) {
    TxCalibrate::ShowSpectrum();
    task = readButton(lastUsedTask);           //  Return the button push.
    if (refineCal) task = MenuSelect::FILTER;  // Jump to refineCal.
    switch (task) {
      // Activate initial automatic calibration.
      case MenuSelect::ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        averageFlag = false;
        averageCount = 0;
        //stopSweep = false;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0);
        // IS THIS STEP REALLY NECESSARY?
        /*
        if(mode == 0) {
        iOptimal = CalData.iDCoffsetCW[ConfigData.currentBand];
        qOptimal = CalData.qDCoffsetCW[ConfigData.currentBand];
        }
                if(mode == 1) {
        iOptimal = CalData.iDCoffsetSSB[ConfigData.currentBand];
        qOptimal = CalData.qDCoffsetSSB[ConfigData.currentBand];
        }
        */
        iOptimal = 0;
        qOptimal = 0;
        state = State::warmup;
        break;
      // Automatic calibration using previously stored values (refine).
      case MenuSelect::FILTER:  // 3rd row, 1st column button
        autoCal = true;
        printCalType(autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        //stopSweep = false;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0);
        // IS THIS STEP REALLY NECESSARY?
        if (mode == 0) {
          iOptimal = CalData.iDCoffsetCW[ConfigData.currentBand];
          qOptimal = CalData.qDCoffsetCW[ConfigData.currentBand];
        }
        if (mode == 1) {
          iOptimal = CalData.iDCoffsetSSB[ConfigData.currentBand];
          qOptimal = CalData.qDCoffsetSSB[ConfigData.currentBand];
        }
        state = State::warmup;
        averageFlag = false;
        refineCal = true;
        break;
      // Toggle gain and phase
      case MenuSelect::UNUSED_1:
        if (IQCalType == 0) IQCalType = 1;
        else IQCalType = 0;
        break;
      // Toggle increment value
      case MenuSelect::BEARING:  // UNUSED_2 is now called BEARING
        corrChange = !corrChange;
        if (corrChange == true) {    // Toggle increment value
          correctionIncrement = 10;  // AFP 2-11-23
        } else {
          correctionIncrement = 5;  // AFP 2-11-23
        }
        tft.setFontScale((enum RA8875tsize)0);
        tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
        tft.setCursor(405, 125);
        tft.print(correctionIncrement);
        break;
      case MenuSelect::MENU_OPTION_SELECT:  // Save values and exit calibration.
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        CalibrateEpilogue();
        return;
        //        exit = true;
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
          //  stopSweep = false;
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            if (mode == 0) {
              CalData.qDCoffsetCW[ConfigData.currentBand] = maxSweepPhase;  //  Need to use these values during warmup
              CalData.iDCoffsetCW[ConfigData.currentBand] = maxSweepAmp;    //  so adjdB and adjdB_avg are forced upwards.
            }
            if (mode == 1) {
              CalData.qDCoffsetSSB[ConfigData.currentBand] = maxSweepPhase;  //  Need to use these values during warmup
              CalData.iDCoffsetSSB[ConfigData.currentBand] = maxSweepAmp;    //  so adjdB and adjdB_avg are forced upwards.
            }
          }
          state = State::warmup;
          if (warmup == 10) state = State::state0;
          if (warmup == 10 && refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorIoffset[i] = (iOptimal - 10 * 5) + (5 * i);  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorQoffset[i] = (qOptimal - 10 * 5) + (5 * i);  // The next array to sweep.
          }
          IQCalType = 0;  // Start with the I offset.
          refineCal = false;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is the I offset.
          //          CalData.qDCoffsetSSB[ConfigData.currentBand] = 0;
          //          CalData.iDCoffsetSSB[ConfigData.currentBand] = -maxSweepAmp;  // Begin sweep at low end and move upwards.
          qDCoffset = 0;
          iDCoffset = -maxSweepAmp;  // Begin sweep at low end and move upwards.
          GetEncoderValueLiveQ15t(1000, 1000, qDCoffset, correctionIncrement, "Q Offset", false);
          adjdB = 0;
          adjdB_avg = 0;
          index = 0;
          IQCalType = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index] = iDCoffset;
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for next measurement.
          //          CalData.iDCoffsetSSB[ConfigData.currentBand] = CalData.iDCoffsetSSB[ConfigData.currentBand] + increment;  // Next one!
          iDCoffset = iDCoffset + increment;
          // Go to Q channel when I channel sweep is finished.
          //    if (index > 20) {
          //      if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          //    }
          if (abs(iDCoffset - 1.0) > maxSweepAmp) {                             // Needs to be subtracted from 1.0.
            IQCalType = 1;                                                      // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            // Serial.printf("Init The optimal I offset = %d at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            qDCoffset = -maxSweepPhase;  // The starting value for phase.
            iDCoffset = iOptimal;        // Reset for next sweep.
            // Update display to optimal value.
            GetEncoderValueLiveQ15t(-1000, 1000, iDCoffset, correctionIncrement, "I Offset", true);
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10 * 5) + (5 * i);
            }
            //          stopSweep = false;
            IQCalType = 1;  // Prepare for phase.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = 0;
            adjdB = 0;
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            state = State::initialSweepPhase;  // Initial I sweep done; proceed to initial Q sweep.
            break;
          }
          state = State::initialSweepAmp;
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index] = qDCoffset;
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for the next measurement.
          //          CalData.qDCoffsetSSB[ConfigData.currentBand] = CalData.qDCoffsetSSB[ConfigData.currentBand] + increment;
          qDCoffset = qDCoffset + increment;
          //     if (index > 20) {
          //       if (sweepVector[index - 1] > (sweepVector[index - 11] + 3.0)) stopSweep = true;  // Stop sweep if past the null.
          //     }
          if (qDCoffset > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            qOptimal = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            qDCoffset = qOptimal;                        // Set to the discovered minimum.
            GetEncoderValueLiveQ15t(-1000, 1000, qDCoffset, correctionIncrement, "Q Offset", false);
            //            Serial.printf("Init The optimal Q offset = %d at index %d with value %.1f count = %d\n", sweepVectorValue[adjdBMinIndex], adjdBMinIndex, *result, count);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorQoffset[i] = (qOptimal - 10 * 5) + (5 * i);
            }
            IQCalType = 0;
            adjdB = 0.0;
            state = State::refineAmp;  // Proceed to refine the gain channel.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            increment = 5;  // Use smaller increment in refinement.
            break;
          }
          state = State::initialSweepPhase;
          break;

        case State::refineAmp:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          iDCoffset = sub_vectorIoffset[index];  // Starting value.
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
            iOptimal = sub_vectorIoffset[adjdBMinIndex];  // -.001;
            iDCoffset = iOptimal;                         // Set to optimal value before refining phase.
            GetEncoderValueLiveQ15t(-1000, 1000, iDCoffset, correctionIncrement, "I Offset", true);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10 * 5) + (5 * i);  // The next array to sweep.
            }
            // Serial.printf("Refine The optimal I offset = %d at index %d with value %.1f\n", iOptimal, adjdBMinIndex, *result);
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
          qDCoffset = sub_vectorQoffset[index];  // Starting value.
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
            qOptimal = sub_vectorQoffset[adjdBMinIndex];  // - .001;
            qDCoffset = qOptimal;
            GetEncoderValueLiveQ15t(-1000, 1000, qDCoffset, correctionIncrement, "Q Offset", false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorQoffset[i] = (qOptimal - 10 * 5) + (5 * i);
            }
            // Serial.printf("Refine The optimal Q offset = %d at index %d with value %.1f\n", qOptimal, adjdBMinIndex, *result);
            IQCalType = 0;
            averageFlag = 0;
            averageCount = 0;
            count = count + 1;
            state = State::refineAmp;
            if (count == 2) state = State::refineAmp;
            if (count == 4) state = State::setOptimal;
            break;
          }
          //          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 8) {
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
          if (mode == 0) {
            CalData.iDCoffsetCW[ConfigData.currentBand] = iOptimal;
            CalData.qDCoffsetCW[ConfigData.currentBand] = qOptimal;
          }
          if (mode == 1) {
            CalData.iDCoffsetSSB[ConfigData.currentBand] = iOptimal;
            CalData.qDCoffsetSSB[ConfigData.currentBand] = qOptimal;
          }
          state = State::exit;
          viewTime = fiveSeconds;  // Start result view timer.
          break;
        case State::exit:
          if (radioCal) {
            printCalType(autoCal, true);
            if ((fiveSeconds - viewTime) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              TxCalibrate::CalibrateEpilogue();
              return;
            }
          }
          autoCal = false;
          refineCal = false;  // Reset refine cal.
          printCalType(autoCal, false);
          break;
      }
    }  // end automatic calibration state machine

    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;                            // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      iDCoffset = GetEncoderValueLiveQ15t(-1000, 1000, iDCoffset, correctionIncrement, "I Offset", true);
      if (mode == 0) {
        CalData.iDCoffsetCW[ConfigData.currentBand] = iDCoffset;
      }
      if (mode == 1) {
        CalData.qDCoffsetSSB[ConfigData.currentBand] = iDCoffset;
      }
    }
    if (IQCalType == 1) {
      qDCoffset = GetEncoderValueLiveQ15t(-1000, 1000, qDCoffset, correctionIncrement, "Q Offset", false);
      if (mode == 0) {
        CalData.qDCoffsetCW[ConfigData.currentBand] = qDCoffset;
      }
      if (mode == 1) {
        CalData.qDCoffsetSSB[ConfigData.currentBand] = qDCoffset;
      }
    }
    //    if (IQChoice == 6) break;  //  Exit the while loop.
    if (exit) break;
  }  // end while

}  // End carrier calibration
#endif


// Automatic calibration of all bands.  Greg KF5N June 4, 2024
void TxCalibrate::RadioCal(int mode, bool refineCal) {
  std::vector<int> ham_bands = { BAND_80M, BAND_40M, BAND_20M, BAND_17M, BAND_15M, BAND_12M, BAND_10M };
  mode = mode;
  // Warn the user if the radio is not calibrated and refine cal is attempted.
  if (((mode == 0) and refineCal and not CalData.CWradioCalComplete) or ((mode == 1) and refineCal and not CalData.SSBradioCalComplete)) {
    tft.setFontScale((enum RA8875tsize)2);
    tft.setTextColor(RA8875_RED);
    tft.setCursor(20, 300);
    tft.print("RADIO NOT CALIBRATED");
    return;
  }

  for (int band : ham_bands) {
    button.BandSet(band);
    ConfigData.currentBand = ConfigData.currentBandA = band;
    SetBandRelay();
    TxRxFreq = calFrequencies[band][mode];  // [band][cw 0, ssb 1]
    Serial.printf("TxRxFreq = %d\n", TxRxFreq);
    Serial.printf("band = %d\n", band);
    Serial.printf("mode = %d\n", mode);
    Serial.printf("calFrequencies[band][mode] = %d\n", calFrequencies[band][mode]);
    ConfigData.centerFreq = TxRxFreq;
    ShowFrequency();
    SetFreq();
    if (band < 2)
      bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;
    else
      bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;
    rxcalibrater.DoReceiveCalibrate(mode, true, refineCal, false);
    TxCalibrate::DoXmitCalibrate(mode, true, refineCal, false);
    bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;
    rxcalibrater.DoReceiveCalibrate(mode, true, refineCal, false);
    TxCalibrate::DoXmitCalibrate(mode, true, refineCal, false);
#ifdef QSE2
    TxCalibrate::DoXmitCarrierCalibrate(mode, true, refineCal, false);
#endif
  }

  // Set flag for initial calibration completed.
  if (mode == 0) CalData.CWradioCalComplete = true;
  if (mode == 1) CalData.SSBradioCalComplete = true;
  eeprom.CalDataWrite();
  RedrawDisplayScreen();
}


/*****
  Purpose: Signal processing for the purpose of calibration.  FFT only, no audio!

   Parameter List:
      

   Return value:
      void
 *****/
void TxCalibrate::MakeFFTData() {
  float32_t rfGainValue;                                               // AFP 2-11-23.  Greg KF5N February 13, 2023
  float32_t recBandFactor[7] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };  // AFP 2-11-23  KF5N uniform values

  uint32_t dataWidth = 2048;  // was 2048
  float32_t powerScale;

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are  read into two arrays sp_L and sp_R in blocks of 128 up to N_BLOCKS.  The arrays are
        of size BUFFER_SIZE*N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_L / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_L = 512
        BUFFER_SIZE*N_BLOCKS = 2048 samples
     **********************************************************************************/
  //     Serial.printf("Q_in_L_Ex.available = %d\n", Q_in_L_Ex.available());
  // Are there at least N_BLOCKS buffers in each channel available ?
  if (static_cast<uint32_t>(Q_in_L_Ex.available()) > 16 and static_cast<uint32_t>(Q_in_R_Ex.available()) > 16) {
    //     Serial.printf("Q_in_L_Ex.available = %d\n", Q_in_L_Ex.available());
    // get audio samples from the audio  buffers and convert them to float
    // read in 16 blocks of 128 samples in I and Q
    for (unsigned i = 0; i < 16; i++) {

      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // Right channel not used.  KF5N March 11, 2024
      Q_in_L_Ex.freeBuffer();
      Q_in_R_Ex.freeBuffer();
    }

    // Set the sideband.
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

    // Apply amplitude and phase corrections.
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBandA], CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBandA], 0.0);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBandA], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBandA], 0.0);
    }
    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBandA], CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBandA], 0.0);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBandA], CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBandA], 0.0);
    }

    //  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
    powerScale = 2.0 * ConfigData.powerOutSSB[ConfigData.currentBand];
#else
    powerScale = 1.4 * ConfigData.powerOutSSB[ConfigData.currentBand];
#endif

    arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, dataWidth);
    arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, dataWidth);

    /**********************************************************************************  AFP 12-31-20
      CONVERT TO INTEGER AND PLAY AUDIO
    **********************************************************************************/
    q15_t q15_buffer_LTemp[dataWidth];  // KF5N
    q15_t q15_buffer_RTemp[dataWidth];  // KF5N

    arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
    arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);
#ifdef QSE2
    if (mode == 0) {
      arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, dataWidth);  // Carrier suppression offset.
      arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, dataWidth);
    }
    if (mode == 1) {
      arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_LTemp, dataWidth);  // Carrier suppression offset.
      arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_RTemp, dataWidth);
    }
#endif
    Q_out_L_Ex.play(q15_buffer_LTemp, dataWidth);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
    Q_out_R_Ex.play(q15_buffer_RTemp, dataWidth);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.
  } else {
    fftSuccess = false;  // Not enough transmit data.
    Serial.printf("FFT failure!\n");
    //    return;
  }
  // End of transmit code.  Begin receive code.

  // Get audio samples from the audio  buffers and convert them to float.
  // Read in 16 blocks of 128 samples in I and Q if available.
  if (static_cast<uint32_t>(ADC_RX_I.available()) > 16 and static_cast<uint32_t>(ADC_RX_Q.available()) > 16) {
    for (unsigned i = 0; i < N_BLOCKS; i++) {
      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float(ADC_RX_Q.readBuffer(), &float_buffer_L[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      arm_q15_to_float(ADC_RX_I.readBuffer(), &float_buffer_R[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      ADC_RX_I.freeBuffer();
      ADC_RX_Q.freeBuffer();
    }

    rfGainValue = pow(10, static_cast<float32_t>(ConfigData.rfGain[ConfigData.currentBand]) / 20);  //AFP 2-11-23
    arm_scale_f32(float_buffer_L, rfGainValue, float_buffer_L, 2048);                               //AFP 2-11-23
    arm_scale_f32(float_buffer_R, rfGainValue, float_buffer_R, 2048);                               //AFP 2-11-23

    /**********************************************************************************  AFP 12-31-20
      Scale the data buffers by the RFgain value defined in bands.bands[ConfigData.currentBand] structure
    **********************************************************************************/
    arm_scale_f32(float_buffer_L, recBandFactor[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 2-11-23
    arm_scale_f32(float_buffer_R, recBandFactor[ConfigData.currentBand], float_buffer_R, 2048);  //AFP 2-11-23

    // Manual IQ amplitude correction (receive only)
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand], 2048);
      } else {
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
          arm_scale_f32(float_buffer_L, CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 04-14-22 KF5N changed sign
          IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand], 2048);
        }
      }
    }

    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand], 2048);
      } else {
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
          arm_scale_f32(float_buffer_L, CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 04-14-22 KF5N changed sign
          IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand], 2048);
        }
      }
    }
    //    }
    /* Manual IQ amplitude correction
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, -CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      } else {
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
          arm_scale_f32(float_buffer_L, -CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
          IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
        }
      }
      */

    FreqShift1();  // 48 kHz shift

    // This process started because there are 2048 samples available.  Perform FFT.
    updateDisplayFlag = true;
    //      Serial.printf("Call ZoomFFTExe\n");
    if(fftActive) ZoomFFTExe(2048);
    fftSuccess = true;
  }  // End of receive code
  else {
    fftSuccess = false;  // Insufficient receive buffers to make FFT.  Do not plot FFT data!
    Serial.printf("FFT failed!\n");
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
void TxCalibrate::ShowSpectrum()  //AFP 2-10-23
{
  int x1 = 0;
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

  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    cal_bins[0] = 257;  // LSB
    cal_bins[1] = 289;  // Carrier
    cal_bins[2] = 322;  // Undesired sideband
  }                     // Transmit and Carrier calibration, LSB.  KF5N
  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    cal_bins[0] = 257;  // USB
    cal_bins[1] = 225;  // Carrier
    cal_bins[2] = 193;  // Undesired sideband
  }                     // Transmit and Carrier calibration, USB.  KF5N

  //    TxCalibrate::MakeFFTData();
  // Don't plot if MakeFFTData() was not succesful.
  ////    if(fftSuccess == false) return;

  // Plot carrier during transmit cal, do not return a dB value:
  if (calTypeFlag == 1) {  // Transmit cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) adjdB = TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);  // Carrier
  }
  if (calTypeFlag == 2) {  // Carrier cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) TxCalibrate::PlotCalSpectrum(x1, cal_bins, capture_bins);  // Undesired sideband
  }

  tft.setCursor(350, 142);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(350, 142, 50, tft.getFontHeight(), RA8875_BLACK);  // Erase old adjdB number.                                       // 350, 125
  tft.print(adjdB, 1);

  ////  delay(5);  // This requires "tuning" in order to get best response.
}


/*****
  Purpose:  Plot Calibration Spectrum   //  KF5N 7/2/2023
            This function plots a partial spectrum during calibration only.
            This is intended to increase the efficiency and therefore the responsiveness of the calibration encoder.
            This function is called by ShowSpectrum() in two for-loops.  One for-loop is for the reference signal,
            and the other for-loop is for the undesired sideband.
  Parameter list:
    int x1, where x1 is the FFT bin.
    cal_bins[3], locations of the desired and undesired signals
    capture_bins, width of the bins used to display the signals
  Return value;
    float, returns the adjusted value in dB
*****/
float TxCalibrate::PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins) {
  //  float adjdB = 0.0;
  int16_t adjAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  int16_t refAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  float alpha = 0.01;

  uint32_t index_of_max;  // This variable is not currently used, but it is required by the ARM max function.  KF5N
  int y_new_plot, y1_new_plot, y_old_plot, y_old2_plot;

  //   The FFT should be performed only at the beginning of the sweep, and buffers must be full.
  if (x1 == (cal_bins[0] - capture_bins)) {  // Set flag at beginning of sweep.  FFT is calculated if enough data is available.  KF5N
    updateDisplayFlag = true;                // This flag is used in ZoomFFTExe().  When true the FFT is performed, when false skipped.
                                             // This is going to acquire 2048 samples of I and Q from the receiver, and then perform the FFT.
                                             // The result will be a 512 wide array of FFT bin levels.
    TxCalibrate::MakeFFTData();
  } else updateDisplayFlag = false;  //  Do not save the the display data for the remainder of the sweep.


  y_new = pixelnew[x1];
  y1_new = pixelnew[x1 - 1];
  y_old = pixelold[x1];
  y_old2 = pixelold[x1 - 1];

  // Find the maximums of the desired and undesired signals so that dB can be calculated.
  // This is done on sub-arrays of the FFT bins for efficiency.
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) && (calTypeFlag == 1)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[2] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) && (calTypeFlag == 1)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[2] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
  }

  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) && (calTypeFlag == 2)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) && (calTypeFlag == 2)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
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
  tft.drawLine(x1, y_old2_plot, x1, y_old_plot, RA8875_BLACK);   // Erase old...
  tft.drawLine(x1, y1_new_plot, x1, y_new_plot, RA8875_YELLOW);  // Draw new

  pixelCurrent[x1] = pixelnew[x1];  //  This is the actual "old" spectrum! Copied to pixelold by the FFT function.

  adjdB = (static_cast<float32_t>(adjAmplitude) - static_cast<float32_t>(refAmplitude)) / (1.95 * 2.0);          // Cast to float and calculate the dB level.  Needs further refinement for accuracy.  KF5N
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER && not(calTypeFlag == 0)) adjdB = -adjdB;  // Flip sign for USB only for TX cal.
  adjdB_avg = adjdB * alpha + adjdBold * (1.0 - alpha);                                                          // Exponential average.
                                                                                                                 //  adjdB_avg = adjdB;     // TEMPORARY EXPERIMENT
  adjdBold = adjdB_avg;
  adjdB_sample = adjdB;
  //    if (bands.bands[ConfigData.currentBand].mode == DEMOD_USB && not(calTypeFlag == 0)) adjdB_avg = -adjdB_avg;  // Flip sign for USB only for TX cal.

  tft.writeTo(L1);
  return adjdB_avg;
}
