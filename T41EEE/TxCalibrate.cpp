// Class TxCalibrate.  Greg KF5N July 10, 2024
// Re-factored August 2025.

// plotCalGraphics(int calType)
// warmUpCal()
// printCalType(bool autoCal, bool autoCalDone)
// CalibratePreamble(int setZoom)
// CalibrateEpilogue(bool radioCal, bool saveToEeprom)
// DoReceiveCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// DoXmitCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// DoXmitCarrierCalibrate(int mode, bool radioCal, bool refineCal, bool saveToEeprom)
// RadioCal(bool refineCal)
// MakeFFTData(int mode)
// ShowSpectrum(int mode)
// PlotCalSpectrum(int mode, int x1, int cal_bins[3], int capture_bins)

#include "SDT.h"


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
  Purpose: Run MakeFFTData() a few times to load and settle out buffers.  KF5N May 22, 2024
           Compute FFT in order to find maximum signal peak prior to beginning calibration.
  Parameter list:
    void

  Return value:
    void
*****/
void TxCalibrate::warmUpCal() {
  uint32_t index_of_max{ 0 };
  uint32_t count{ 0 };
  uint32_t i;
  // MakeFFTData() has to be called enough times for transients to settle out before computing FFT.
  for (i = 0; i < 128; i = i + 1) {
    fftActive = true;
    updateDisplayFlag = true;
    TxCalibrate::MakeFFTData();  // Note, FFT not called if buffers are not sufficiently filled.
    arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
    if(index_of_max > 251 and index_of_max < 260) {  // The peak is in the correct bin?
      count = count + 1;    
    }  else count = 0;  // Reset count in case of failure.
    if( count == 5) break;  // If five in a row, exit the loop.  Warm-up is complete.
  }
  updateDisplayFlag = true;
  fftActive = true;
  TxCalibrate::MakeFFTData();  // Now FFT will be calculated.
  updateDisplayFlag = false;
  // Find peak of spectrum, which is 512 wide.  Use this to adjust spectrum peak to top of spectrum display.
  arm_max_q15(pixelnew, 512, &rawSpectrumPeak, &index_of_max);
  Serial.printf("TX rawSpectrumPeak = %d count = %d i = %d\n", rawSpectrumPeak, count, i);
  Serial.printf("TX index_of_max = %d\n", index_of_max);
  if(index_of_max < 251 or index_of_max > 260) Serial.printf("Problem with TX warmUpCal\n");
}


/*****
  Purpose: Print CW or SSB to the display.  Greg KF5N August 2025
           Compute FFT in order to find maximum signal peak prior to beginning calibration.
  Parameter list:
    void

  Return value:
    void
*****/
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
controlAudioOut(ConfigData.audioOut, true);  // Mute all receiver audio.
  calOnFlag = true;  // Used for the special display during calibration and also high-dynamic range FFT.
  exitManual = false;
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
  } else tempSideband = bands.bands[ConfigData.currentBand].sideband;
  ConfigData.CWOffset = 2;                   // 750 Hz for TX calibration.  Epilogue restores user selected offset.
                                             //  userxmtMode = ConfigData.xmtMode;          // Store the user's mode setting.  KF5N July 22, 2023
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
  tft.setCursor(350, 175);
  tft.print("user1 - Gain/Phase");
  tft.setCursor(350, 195);
  tft.print("User2 - Incr");
  tft.setCursor(350, 215);
  tft.print("Zoom - Auto-Cal");
  tft.setCursor(350, 235);
  tft.print("Filter - Refine-Cal");
  tft.setTextColor(RA8875_CYAN);
  ////  tft.fillRect(350, 125, 100, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(440, 142);
  tft.setFontScale((enum RA8875tsize)1);
  tft.print("dB");
  tft.setTextColor(RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(350, 125);
  tft.print("Incr = ");
  userScale = ConfigData.currentScale;  //  Remember user preference so it can be reset when done.  KF5N
  ConfigData.currentScale = 1;          //  Set vertical scale to 10 dB during calibration.  KF5N
  updateDisplayFlag = false;
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0;
  digitalWrite(MUTE, MUTEAUDIO);  // Mute Audio.
  digitalWrite(RXTX, HIGH);       // Turn on transmitter.
  rawSpectrumPeak = 0;
  if (mode == 0) radioState = RadioState::CW_CALIBRATE_STATE;
  if (mode == 1) radioState = RadioState::SSB_CALIBRATE_STATE;
  display.ShowTransmitReceiveStatus();
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
//  radioState = tempState;
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0;
  calibrateFlag = 0;                                       // KF5N
  ConfigData.CWOffset = cwFreqOffsetTemp;                  // Return user selected CW offset frequency.
  sineTone(ConfigData.CWOffset + 6);                       // This function takes "number of cycles" which is the offset + 6.
  ConfigData.currentScale = userScale;                     //  Restore vertical scale to user preference.  KF5N
  ConfigData.transmitPowerLevel = transmitPowerLevelTemp;  // Restore the user's transmit power level setting.  KF5N August 15, 2023
  zoomIndex = userZoomIndex - 1;
  button.ButtonZoom();                                   // Restore the user's zoom setting.  Note that this function also modifies ConfigData.spectrum_zoom.
  if (TxCalibrate::saveToEeprom) eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
  tft.writeTo(L2);                                       // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
  calOnFlag = false;
  if (not TxCalibrate::radioCal) display.RedrawAll();  // Redraw everything!
  else tft.fillWindow();                                         // Clear the display in radioCal.
  fftOffset = 0;                                                 // Some reboots may be caused by large fftOffset values when Auto-Spectrum is on.
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  bands.bands[ConfigData.currentBand].sideband = tempSideband;
  lastState = RadioState::NOSTATE;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  radioState = tempState;
  SetAudioOperatingState(radioState);  // Restore state.
  powerUp = true;
}


//void TxCalibrate::buttonTasks(bool radioCal, bool refineCal) {  // Does this function need parameters?
void TxCalibrate::buttonTasks() {
  task = readButton();
  //    exit = false;
  switch (task) {
    // Activate initial automatic calibration.
    case MenuSelect::ZOOM:  // 2nd row, 1st column button
      TxCalibrate::autoCal = true;
      TxCalibrate::refineCal = false;
      printCalType(TxCalibrate::autoCal, false);
      count = 0;
      warmup = 0;
      index = 0;
      //      IQCalType = 0;
      averageFlag = false;
      averageCount = 0;
      //      std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
      //      std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
      //      amplitude = 1.0;
      //      phase = 0.0;
      state = State::warmup;
      break;
    // Automatic calibration using previously stored values.
    case MenuSelect::FILTER:  // 3rd row, 1st column button
      TxCalibrate::autoCal = true;
      TxCalibrate::refineCal = true;
      printCalType(autoCal, false);
      count = 0;
      warmup = 0;
      index = 0;  // Why is index = 1???
                  //      IQCalType = 0;
                  //      std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
                  //      std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
      //      amplitude = ;
      //      phase = ;
      averageFlag = false;
      state = State::warmup;
      break;
    // Toggle gain and phasei in manual mode.
    case MenuSelect::UNUSED_1:
      if (IQCalType == 0) {
        IQCalType = 1;
        // Turn off red indication of active setting.
        if (calTypeFlag == 1) GetEncoderValueLive(-1.0, 1.0, amplitude, xmitIncrement, "IQ Gain ", true, false);
        if (calTypeFlag == 2) GetEncoderValueLive(-1.0, 1.0, iDCoffset, carrIncrement, "I Offset ", true, false);
      } else {
        IQCalType = 0;
        // Turn off red indication of active setting.
        if (calTypeFlag == 1) GetEncoderValueLive(-1.0, 1.0, phase, xmitIncrement, "IQ Phase ", false, false);
        if (calTypeFlag == 2) GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, false);
      }
      break;
    // Toggle increment value
    case MenuSelect::BEARING:  // UNUSED_2 is now called BEARING
      tft.setFontScale((enum RA8875tsize)0);
      tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
      tft.setCursor(405, 125);
      corrChange = not corrChange;
      if (calTypeFlag == 1) {
        if (corrChange == true) {  // Toggle increment value
          xmitIncrement = 0.001;   // AFP 2-11-23
        } else {
          xmitIncrement = 0.002;  // AFP 2-11-23
        }
        tft.print(xmitIncrement, 3);
        break;
      }
      if (calTypeFlag == 2) {
        if (corrChange == true) {  // Toggle increment value
          carrIncrement = 0.0005;
        } else {
          carrIncrement = 0.001;
        }
      }
      tft.print(carrIncrement, 4);
      break;
    case MenuSelect::MENU_OPTION_SELECT:  // Save values and exit from manual calibration.
      exitManual = true;
      break;
    default:
      break;
  }  // end switch
}


void TxCalibrate::writeToCalData(float ichannel, float qchannel) {
  if (mode == 0) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand] = ichannel;
      CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand] = qchannel;
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
      CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand] = ichannel;
    CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand] = qchannel;
  }
  if (mode == 1) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand] = ichannel;
      CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand] = qchannel;
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
      CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand] = ichannel;
    CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand] = qchannel;
  }
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int mode, bool radioCal, bool refineCal, bool saveToEeprom

   Return value:
      void
 *****/
void TxCalibrate::DoXmitCalibrate(int calMode, bool radio, bool refine, bool toEeprom) {
  int freqOffset = 0;  // Calibration tone same as regular modulation tone.
  //  float correctionIncrement = 0.001;
  //  State state = State::warmup;  // Start calibration state machine in warmup state.
  float maxSweepAmp = 0.2;
  float maxSweepPhase = 0.1;
  xmitIncrement = 0.002;  // Coarse increment used during initial amplitude calibration.
  //  int averageCount = 0;
  IQCalType = 0;  // Begin with IQ gain optimization.
                  //  float iOptimal = 1.0;
                  //  float qOptimal = 0.0;
  std::vector<float32_t> sweepVector(201);
  std::vector<float32_t> sweepVectorValue(201);
  std::vector<float32_t> sub_vectorAmp = std::vector<float>(21);  // Can these arrays be commonized?
  std::vector<float32_t> sub_vectorPhase = std::vector<float>(21);
  std::vector<float> sub_vectorAmpResult = std::vector<float>(21);
  std::vector<float> sub_vectorPhaseResult = std::vector<float>(21);
  elapsedMillis fiveSeconds;
  int startTimer = 0;
  TxCalibrate::autoCal = false;
  //  bool refineCal = false;
  //  bool averageFlag = false;
  TxCalibrate::mode = calMode;           // CW or SSB.  This is an object state variable.
  TxCalibrate::radioCal = radio;         // Initial calibration of all bands.
  TxCalibrate::refineCal = refine;       // Refinement (using existing values a starting point) calibration for all bands.
  TxCalibrate::saveToEeprom = toEeprom;  // Save to EEPROM
  std::vector<float>::iterator result;

  //  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  TxCalibrate::CalibratePreamble(2);  // Set zoom to 4X.  Sample rate 48ksps.                  
  calTypeFlag = 1;                    // TX cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(xmitIncrement, 3);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) {
    ResetFlipFlops();  // This has a delay.
  } else {
    delay(1000);  // Delay for V10/V11 to allow transients to settle.
  }
  SetFreqCal(freqOffset);
  printCalType(autoCal, false);
  // Get current values into the iOptimal and qOptimal amplitude and phase working variables.
  // This is only useful for refinement.
  if (mode == 0) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  } else if (mode == 1) {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      iOptimal = amplitude = CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      iOptimal = amplitude = CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand];
      qOptimal = phase = CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }
  // Run this so Phase shows from beginning.  Get the value for the current sideband.
  GetEncoderValueLive(-2.0, 2.0, phase, xmitIncrement, "IQ Phase ", false, false);
  warmUpCal();

  if (radioCal) {
    autoCal = true;
    printCalType(autoCal, false);
    count = 0;
    warmup = 0;
    index = 0;  // Why is index = 1???
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Transmit Calibration Loop
  while (true) {
    fftActive = true;
    TxCalibrate::ShowSpectrum();
    // This function takes care of button presses and resultant control of the rest of the process.
    // The buttons are polled by the while loop.
    TxCalibrate::buttonTasks();  // This takes care of manual calls to the initial or refinement calibrations.
                                 // Exit from manual calibration by button push.
    if (exitManual == true) {
      TxCalibrate::CalibrateEpilogue();
      return;
    }
    //  Begin automatic calibration state machine.
    if (autoCal or radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(autoCal, false);
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            phase = 0.0 + maxSweepPhase;    //  Need to use these values during warmup
            amplitude = 1.0 + maxSweepAmp;  //  so adjdB and adjdB_avg are forced upwards.
          }
          state = State::warmup;
          if (warmup == 16) state = State::state0;  // Proceed with initial calibration.
          if (warmup == 16 and refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * static_cast<float32_t>(i));  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * static_cast<float32_t>(i));  // The next array to sweep.
          }
          IQCalType = 0;  // Start in IQ Gain.
          index = 0;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for initial calibration sweeps.  First sweep is amplitude (gain).
          phase = 0.0;                                                                     // Hold phase at 0.0 while amplitude sweeps.
          amplitude = 1.0 - maxSweepAmp;                                                   // Begin sweep at low end and move upwards.
          GetEncoderValueLive(-2.0, 2.0, phase, xmitIncrement, "IQ Phase ", false, false);  // Display phase value during amplitude sweep.
          adjdB = 0;
          adjdB_avg = 0;
          index = 0;
          IQCalType = 0;                   // IQ Gain
          xmitIncrement = 0.002;  // Reset in case initial cal is run twice.
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
        TxCalibrate::ShowSpectrum();  // This slows down the loop to improve accuracy.
          sweepVectorValue[index] = amplitude;
          sweepVector[index] = adjdB;
          // Increment for next measurement.
          index = index + 1;
          amplitude = amplitude + xmitIncrement;  // Next one!
          // Done with initial sweep, move to initial sweep of phase.
          if (abs(amplitude - 1.0) > maxSweepAmp) {                             // Needs to be subtracted from 1.0.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            amplitude = iOptimal;                                               // Set amplitude to the discovered optimal value.
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-2.0, 2.0, amplitude, xmitIncrement, "IQ Gain ", true, false);
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * static_cast<float32_t>(i));
            }
            IQCalType = 1;           // Prepare for phase.
            phase = -maxSweepPhase;  // The starting value for phase.
            index = 0;
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            xmitIncrement = 0.001;  // The initial increment can be reduced because the sweep range is half.
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
          phase = phase + xmitIncrement;
          if (phase > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            qOptimal = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            phase = qOptimal;                            // Set to the discovered minimum.
                                                         // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-2.0, 2.0, phase, xmitIncrement, "IQ Phase ", false, false);
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * static_cast<float32_t>(i));
            }
            IQCalType = 0;
            adjdB = 0.0;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            xmitIncrement = 0.001;     // Use smaller increment in refinement.
            state = State::refineAmp;  // Proceed to refine the gain channel.
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
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-2.0, 2.0, amplitude, xmitIncrement, "IQ Gain ", true, false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
            }
            IQCalType = 1;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 1 or count == 3) state = State::refinePhase;  // Alternate refinePhase and refineAmp.
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
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-2.0, 2.0, phase, xmitIncrement, "IQ Phase ", false, false);
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
          // Write the optimal values to the data structure.
          writeToCalData(iOptimal, qOptimal);
          state = State::exit;
          startTimer = static_cast<int>(milliTimer);  // Start result view timer.
          break;
        case State::exit:
          // Delay exit if in radio calibration to show calibration results for 5 seconds, and then exit.
          if (radioCal) {
            printCalType(autoCal, true);
            if ((static_cast<int>(milliTimer) - startTimer) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {  // Clean up and return.
              TxCalibrate::CalibrateEpilogue();
              return;
            }
            //  Not in radioCal, but was in autoCal, and now need to return to manual cal.
          } else {
            autoCal = false;  // Don't enter switch, but remain in manual loop.
            printCalType(autoCal, false);
          }
          break;
      }
    }  // end automatic calibration state machine

    //    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;  // Reset task after it is used.
                                 //  Read encoder and update values.
    if (IQCalType == 0) amplitude = GetEncoderValueLive(-2.0, 2.0, amplitude, xmitIncrement, "IQ Gain ", true, true);
    if (IQCalType == 1) phase = GetEncoderValueLive(-2.0, 2.0, phase, xmitIncrement, "IQ Phase ", false, true);
    writeToCalData(amplitude, phase);

  }  // end while
}  // End Transmit calibration


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int mode, bool radioCal, bool refineCal, bool saveToEeprom

   Return value:
      void
 *****/
#ifdef QSE2
void TxCalibrate::DoXmitCarrierCalibrate(int calMode, bool radio, bool refine, bool toEeprom) {
  float32_t maxSweepAmp = 0.1;
  float32_t maxSweepPhase = 0.1;
  carrIncrement = 0.0010;  // Initial carrier increment.
  IQCalType = 0;           // Begin with I channel offset.
  int averageCount = 0;
  TxCalibrate::mode = calMode;  // CW or SSB
  TxCalibrate::radioCal = radio;
  TxCalibrate::refineCal = refine;
  TxCalibrate::saveToEeprom = toEeprom;     // Save to EEPROM
  std::vector<float32_t> sweepVector(201);  // 0 + 450 * 2 / 5
  std::vector<float32_t> sweepVectorValue(201);
  std::vector<float32_t> sub_vectorIoffset = std::vector<float32_t>(21);
  std::vector<float32_t> sub_vectorQoffset = std::vector<float32_t>(21);
  std::vector<float> sub_vectorAmpResult = std::vector<float>(21);
  std::vector<float> sub_vectorPhaseResult = std::vector<float>(21);
  std::vector<float>::iterator result;
  int startTimer = 0;
  TxCalibrate::CalibratePreamble(2);  // Set zoom to 4X.  Note this is using 48ksps sample rate.
  int freqOffset = 0;                 // Calibration tone same as regular modulation tone.
  calTypeFlag = 2;                    // Carrier calibration
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(carrIncrement, 4);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  radioState = RadioState::SSB_TRANSMIT_STATE;
  SetFreqCal(freqOffset);
  printCalType(autoCal, false);
  // Get current values into the iDCoffset and qDCoffset working variables.
  // This is required for refinement.
  if (mode == 0) {
    iDCoffset = CalData.iDCoffsetCW[ConfigData.currentBand];
    qDCoffset = CalData.qDCoffsetCW[ConfigData.currentBand];
  }
  if (mode == 1) {
    iDCoffset = CalData.iDCoffsetSSB[ConfigData.currentBand];
    qDCoffset = CalData.qDCoffsetSSB[ConfigData.currentBand];
  }
  // Run this so Q offset shows from begining.
  GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, false);
  warmUpCal();

  if (radioCal) {
    autoCal = true;
    printCalType(autoCal, false);
    count = 0;
    warmup = 0;
    index = 0;
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Transmit Calibration Loop
  while (true) {
    fftActive = true;
    TxCalibrate::ShowSpectrum();
    TxCalibrate::buttonTasks();  // This takes care of manual calls to the initial or refinement calibrations.
    // Exit from manual calibration by button push.
    if (exitManual == true) {
      TxCalibrate::CalibrateEpilogue();
      return;
    }
    //  Begin automatic calibration state machine.
    if (autoCal or radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(autoCal, false);
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            qDCoffset = maxSweepPhase;  //  Need to use these values during warmup
            iDCoffset = maxSweepAmp;    //  so adjdB and adjdB_avg are forced upwards.
          }
          state = State::warmup;
          if (warmup == 16) state = State::state0;
          if (warmup == 16 && refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (uint32_t i = 0; i < sub_vectorIoffset.size(); i = i + 1) {
            sub_vectorIoffset[i] = (iDCoffset - 10 * 0.0005) + (0.0005 * static_cast<float32_t>(i));  // The next array to sweep.
          }
          for (uint32_t i = 0; i < sub_vectorQoffset.size(); i = i + 1) {
            sub_vectorQoffset[i] = (qDCoffset - 10 * 0.0005) + (0.0005 * static_cast<float32_t>(i));  // The next array to sweep.
          }
          IQCalType = 0;             // Start with the I offset.
          carrIncrement = 0.0005;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          qDCoffset = 0.0;
          iDCoffset = -maxSweepAmp;  // Begin sweep at low end and move upwards.
          GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, false);
          index = 0;
          IQCalType = 0;
          carrIncrement = 0.0010;  // Reset in case initial cal is run twice.
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index] = iDCoffset;  // Starting at -maxSweepAmp.
          sweepVector[index] = adjdB;           // Already computed in warm-up for index 0.
          index = index + 1;
          // Increment for next measurement.
          iDCoffset = iDCoffset + carrIncrement;
          // Go to Q channel when I channel sweep is finished.
          if (iDCoffset > maxSweepAmp) {
            IQCalType = 1;                                                      // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            iDCoffset = iOptimal;                                               // Reset for next sweep.
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-1.0, 1.0, iDCoffset, carrIncrement, "I Offset ", true, false);
            // Save the sub_vector which will be used to refine the optimal result.
            for (uint32_t i = 0; i < sub_vectorIoffset.size(); i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10.0 * 0.0005) + (0.0005 * static_cast<float32_t>(i));
            }
            IQCalType = 1;  // Prepare for phase.
            index = 0;
            averageFlag = false;
            averageCount = 0;
            //      count = 0;
            adjdB = 0;
            // Clear the vector before moving to phase.
            std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
            std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
            qDCoffset = -maxSweepPhase;        // The starting value for phase.
            state = State::initialSweepPhase;  // Initial I channel sweep done; proceed to initial Q sweep.
            break;
          }
          state = State::initialSweepAmp;  // Continue sweeping.
          break;

        case State::initialSweepPhase:
          sweepVectorValue[index] = qDCoffset;  // Start at -maxSweepPhase.
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for the next measurement.
          qDCoffset = qDCoffset + carrIncrement;
          if (qDCoffset > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            qOptimal = sweepVectorValue[adjdBMinIndex];  // Set to the discovered minimum.
            qDCoffset = qOptimal;                        // Set to the discovered minimum.
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, false);
            for (uint32_t i = 0; i < sub_vectorQoffset.size(); i = i + 1) {
              sub_vectorQoffset[i] = (qOptimal - 10.0 * 0.0005) + (0.0005 * static_cast<float32_t>(i));
            }
            IQCalType = 0;
            adjdB = 0.0;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            carrIncrement = 0.0005;    // Use smaller increment in refinement.
            count = 0;                 // This variable is used to switch back and forth between I and Q refinement.
            state = State::refineAmp;  // Proceed to refine the gain channel.
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
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-1.0, 1.0, iDCoffset, carrIncrement, "I Offset ", true, false);
            for (uint32_t i = 0; i < sub_vectorIoffset.size(); i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10 * 0.0005) + (0.0005 * static_cast<float32_t>(i));  // The next array to sweep.
            }
            IQCalType = 1;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 1 || count == 3) state = State::refinePhase;  // Alternate refinePhase and refineAmp.
            break;
          }
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
            // Update display to optimal value and change from red to white.
            GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, false);
            for (uint32_t i = 0; i < sub_vectorQoffset.size(); i = i + 1) {
              sub_vectorQoffset[i] = (qOptimal - 10 * 0.0005) + (0.0005 * static_cast<float32_t>(i));
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
          startTimer = static_cast<int>(milliTimer);  // Start result view timer.
          break;
        case State::exit:
          // Delay exit if in radio calibration to show calibration results for 5 seconds and then exit.
          if (radioCal) {
            printCalType(autoCal, true);
            if ((static_cast<int>(milliTimer) - startTimer) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              TxCalibrate::CalibrateEpilogue();
              return;
            }
          } else {
            autoCal = false;  // Go back to manual mode.
            printCalType(autoCal, false);
          }
          break;
        default:
          break;
      }
    }  // end automatic calibration state machine

    //    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;  // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) iDCoffset = GetEncoderValueLive(-1.0, 1.0, iDCoffset, carrIncrement, "I Offset ", true, true);
    if (IQCalType == 1) qDCoffset = GetEncoderValueLive(-1.0, 1.0, qDCoffset, carrIncrement, "Q Offset ", false, true);
    if (mode == 0) {
      CalData.iDCoffsetCW[ConfigData.currentBand] = iDCoffset;
      CalData.qDCoffsetCW[ConfigData.currentBand] = qDCoffset;
    }
    if (mode == 1) {
      CalData.iDCoffsetSSB[ConfigData.currentBand] = iDCoffset;
      CalData.qDCoffsetSSB[ConfigData.currentBand] = qDCoffset;
    }

  }  // end while
}  // End carrier calibration

#endif


// Automatic calibration of all bands.  Greg KF5N June 4, 2024
void TxCalibrate::RadioCal(int mode, bool refineCal) {
  std::vector<int> ham_bands = { BAND_80M, BAND_40M, BAND_20M, BAND_17M, BAND_15M, BAND_12M, BAND_10M };

  // Warn the user if the radio is not calibrated and refine cal is attempted.
  if (((mode == 0) and refineCal and not CalData.CWradioCalComplete) or ((mode == 1) and refineCal and not CalData.SSBradioCalComplete)) {
    tft.setFontScale((enum RA8875tsize)2);
    tft.setTextColor(RA8875_RED);
    tft.setCursor(20, 300);
    tft.print("RADIO NOT CALIBRATED");
    return;
  }
// Calibrate all bands.
  for (int band : ham_bands) {
    button.BandSet(band);
    ConfigData.currentBand = ConfigData.currentBandA = band;
//    SetBandRelay();  Done in ExecuteModeChange()
    TxRxFreq = calFrequencies[band][mode];  // [band][cw 0, ssb 1]
    ConfigData.centerFreq = TxRxFreq;
    display.ShowFrequency();
    display.BandInformation();
    SetFreq();
    SetBandRelay();
//    button.ExecuteModeChange();
    if (band < 2) {
      bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Calibrate lower sideband for 80M and 40M.
      rxcalibrater.DoReceiveCalibrate(mode, true, refineCal, false);
      txcalibrater.DoXmitCalibrate(mode, true, refineCal, false);
    } else
      bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;

    bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;
    rxcalibrater.DoReceiveCalibrate(mode, true, refineCal, false);  // Include 80M and 40M due to FT8.
    txcalibrater.DoXmitCalibrate(mode, true, refineCal, false);
#ifdef QSE2
    txcalibrater.DoXmitCarrierCalibrate(mode, true, refineCal, false);
#endif
  }

  // Set flag for initial calibration completed.
  if (mode == 0) CalData.CWradioCalComplete = true;
  if (mode == 1) CalData.SSBradioCalComplete = true;
  eeprom.CalDataWrite();
  display.RedrawAll();
}


/*****
  Purpose: Signal processing for the purpose of calibration.  FFT only, no audio!

   Parameter List:
      

   Return value:
      void
 *****/
void TxCalibrate::MakeFFTData() {
  float32_t rfGainValue;                                               // AFP 2-11-23.  Greg KF5N February 13, 2023
//  float32_t recBandFactor[7] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };  // AFP 2-11-23  KF5N uniform values

  uint32_t dataWidth = 2048;  // was 2048
  float32_t powerScale = 0;

  float32_t* iBuffer = nullptr;  // I and Q pointers needed for one-time read of record queues.
  float32_t* qBuffer = nullptr;

// Read incoming I and Q audio blocks from the SSB exciter.
  // Are there at least N_BLOCKS buffers in each channel available ?
  if (static_cast<uint32_t>(Q_in_L_Ex.available()) > 15 and static_cast<uint32_t>(Q_in_R_Ex.available()) > 15) {
    for (unsigned i = 0; i < 16; i++) {

      iBuffer = Q_in_L_Ex.readBuffer();
      qBuffer = Q_in_R_Ex.readBuffer();
      std::copy(iBuffer, iBuffer + 128, &float_buffer_L_EX[128 * i]);
      std::copy(qBuffer, qBuffer + 128, &float_buffer_R_EX[128 * i]);

      Q_in_L_Ex.freeBuffer();
      Q_in_R_Ex.freeBuffer();
    }
    
//  if (static_cast<uint32_t>(Q_in_L_Ex.available()) > 15 and static_cast<uint32_t>(Q_in_R_Ex.available()) > 15) {
//        for (unsigned i = 0; i < 16; i++) {
//      arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
//      arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // Right channel not used.  KF5N March 11, 2024
//      Q_in_L_Ex.freeBuffer();
//      Q_in_R_Ex.freeBuffer();
 //   }


    // Set the sideband.
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

    // Apply amplitude and phase corrections.
    if (TxCalibrate::mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBandA], CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBandA], 0.0);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBandA], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBandA], 0.0);
    }
    if (TxCalibrate::mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBandA], CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBandA], 0.0);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBandA], CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBandA], 0.0);
    }

    //  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
    if (mode == 0) powerScale = 2.0 * ConfigData.powerOutCW[ConfigData.currentBand];
    if (mode == 1) powerScale = 2.0 * ConfigData.powerOutSSB[ConfigData.currentBand];
#else
    if (mode == 0) powerScale = 1.4 * ConfigData.powerOutCW[ConfigData.currentBand];
    if (mode == 1) powerScale = 1.4 * ConfigData.powerOutSSB[ConfigData.currentBand];
#endif

    arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, dataWidth);
    arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, dataWidth);

// Temporarily bypass ***************************************************************************  AFP 12-31-20
//      CONVERT TO INTEGER AND PLAY AUDIO
//    **********************************************************************************
//    q15_t q15_buffer_LTemp[dataWidth];  // KF5N
//    q15_t q15_buffer_RTemp[dataWidth];  // KF5N


//    arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
//    arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);
#ifdef QSE2
    if (TxCalibrate::mode == 0) {
      arm_offset_f32(float_buffer_L_EX, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_L_EX, dataWidth);  // Carrier suppression offset.
      arm_offset_f32(float_buffer_R_EX, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_R_EX, dataWidth);
    }
    if (TxCalibrate::mode == 1) {
      arm_offset_f32(float_buffer_L_EX, CalData.iDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, float_buffer_L_EX, dataWidth);  // Carrier suppression offset.
      arm_offset_f32(float_buffer_R_EX, CalData.qDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, float_buffer_R_EX, dataWidth);
    }
#endif
    //
    //    Q_out_L_Ex.play(q15_buffer_LTemp, dataWidth);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
    //    Q_out_R_Ex.play(q15_buffer_RTemp, dataWidth);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.
    Q_out_L_Ex.play(float_buffer_L_EX, dataWidth);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
    Q_out_R_Ex.play(float_buffer_R_EX, dataWidth);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.
  } else {
    fftSuccess = false;  // Not enough transmit data.
//    Serial.printf("Failed to get enough I and Q transmitter data!\n");
  }
  // End of transmit code.  Begin receive code.

  // Get audio samples from the audio  buffers and convert them to float.
  // Read in 16 blocks of 128 samples in I and Q if available.
  if (static_cast<uint32_t>(ADC_RX_I.available()) > 15 and static_cast<uint32_t>(ADC_RX_Q.available()) > 15) {
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
//    arm_scale_f32(float_buffer_L, recBandFactor[ConfigData.currentBand], float_buffer_L, 2048);  //AFP 2-11-23
//    arm_scale_f32(float_buffer_R, recBandFactor[ConfigData.currentBand], float_buffer_R, 2048);  //AFP 2-11-23

    // Manual IQ amplitude and phase correction (receive only).
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

    FreqShift1();  // 12 kHz shift

    // This process started because there are 2048 samples available.  Perform FFT.
    updateDisplayFlag = true;
    if (fftActive) ZoomFFTExe(2048);
    fftSuccess = true;
  }  // End of receive code
  else {
    fftSuccess = false;  // Insufficient receive buffers to make FFT.  Do not plot FFT data!
//    Serial.printf("FFT failed due to insufficient I and Q receive data!\n");
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
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(350, 142, 78, tft.getFontHeight(), RA8875_BLACK);  // Erase old adjdB number.                                       // 350, 125
  tft.print(adjdB, 1);

}  // end ShowSpectrum()


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

//  pixelCurrent[x1] = pixelnew[x1];  //  This is the actual "old" spectrum! Copied to pixelold by the FFT function.

  adjdB = (static_cast<float32_t>(adjAmplitude) - static_cast<float32_t>(refAmplitude)) / (1.95 * 2.0);          // Cast to float and calculate the dB level.  Needs further refinement for accuracy.  KF5N
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER && not(calTypeFlag == 0)) adjdB = -adjdB;  // Flip sign for USB only for TX cal.
  adjdB_avg = adjdB * alpha + adjdBold * (1.0 - alpha);                                                          // Exponential average.
                                                                                                                 //  adjdB_avg = adjdB;     // TEMPORARY EXPERIMENT
  adjdBold = adjdB_avg;
  //adjdB_sample = adjdB;
  //    if (bands.bands[ConfigData.currentBand].mode == DEMOD_USB && not(calTypeFlag == 0)) adjdB_avg = -adjdB_avg;  // Flip sign for USB only for TX cal.

  tft.writeTo(L1);
  return adjdB_avg;
}  // end PlotCalSpectrum(. . .)
