// Class Calibrate replaces Process2.cpp.  Greg KF5N June 16, 2024

#include "SDT.h"

/*****
  Purpose: Load buffers used to modulate the transmitter during calibration.
          The epilogue must restore the buffers for normal operation!

   Parameter List:
      void

   Return value:
      void
 *****/
void CWCalibrate::loadCalToneBuffers(float toneFreq) {
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
           New function, KF5N May 20, 2024
  Parameter list:
    int calType

  Return value:
    void
*****/
void CWCalibrate::plotCalGraphics(int calType) {
  tft.writeTo(L2);
  if (calType == 0) {  // Receive Cal
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      tft.fillRect(445, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);     // SPECTRUM_TOP_Y = 100, h = 135
      tft.fillRect(304, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);  // h = SPECTRUM_HEIGHT + 3
    } else {                                                         // SPECTRUM_HEIGHT = 150 so h = 153
      tft.fillRect(50, SPECTRUM_TOP_Y + 20, 20, 341, DARK_RED);
      tft.fillRect(188, SPECTRUM_TOP_Y + 20, 20, 341, RA8875_BLUE);
    }
  }
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
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
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
void CWCalibrate::warmUpCal(int mode) {
  // Run ProcessIQData2() a few times to load and settle out buffers.  Compute FFT.  KF5N May 19, 2024
  uint32_t index_of_max;  // Not used, but required by arm_max_q15 function.
  for (int i = 0; i < 128; i = i + 1) {
    updateDisplayFlag = true;  // Causes FFT to be calculated.
    while (static_cast<uint32_t>(ADC_RX_I.available()) < 16 and static_cast<uint32_t>(ADC_RX_Q.available()) < 16) {
      delay(1);
    }
    CWCalibrate::ProcessIQData2(mode);
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
void CWCalibrate::printCalType(int mode, int IQCalType, bool autoCal, bool autoCalDone) {
  const char *calName;
  const char *IQName[4] = { "Receive CW", "Transmit CW", "Carrier CW", "Calibrate" };
  tft.writeTo(L1);
  calName = IQName[calTypeFlag];
  if (mode == 1) calName = "Receive SSB";
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED);
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) and (calTypeFlag == 0)) {
    tft.setCursor(35, 260);
    tft.print(calName);
    tft.setCursor(35, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(35, 330);
      tft.fillRect(35, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(35, 330);
      tft.fillRect(35, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) and (calTypeFlag == 0)) {
    tft.setCursor(275, 260);
    tft.print(calName);
    tft.setCursor(275, 295);
    tft.print(IQName[3]);
    if (autoCal) {
      tft.setCursor(275, 330);
      tft.fillRect(275, 330, 215, 40, RA8875_BLACK);
      if (autoCalDone) {
        tft.setTextColor(RA8875_GREEN);
        tft.print("Auto-Cal Mode");
      } else {
        tft.setTextColor(RA8875_RED);
        tft.print("Auto-Cal Mode");
      }
    } else {
      tft.setCursor(275, 330);
      tft.fillRect(275, 330, 215, 40, RA8875_BLACK);
      tft.print("Manual Mode");
    }
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) and (calTypeFlag == 1)) {
    tft.setCursor(30, 260);
    tft.print(calName);
    tft.setCursor(30, 295);
    tft.print(IQName[3]);
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
    tft.print(calName);
    tft.setCursor(290, 295);
    tft.print(IQName[3]);
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
    tft.print(calName);
    tft.setCursor(30, 295);
    tft.print(IQName[3]);
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
    tft.print(calName);
    tft.setCursor(290, 295);
    tft.print(IQName[3]);
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
void CWCalibrate::CalibratePreamble(int setZoom) {
  cessb1.processorUsageMaxReset();
  calOnFlag = true;
  IQCalType = 0;
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
  tft.setCursor(400, 142);
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
  radioState = RadioState::CW_CALIBRATE_STATE;
  ShowTransmitReceiveStatus();
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
void CWCalibrate::CalibrateEpilogue(bool radioCal, bool saveToEeprom) {
  Serial.printf("CW epilogue\n");
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
  ShowTransmitReceiveStatus();
  // Clear queues to reduce transient.
  ADC_RX_I.end();
  ADC_RX_I.clear();
  ADC_RX_Q.end();
  ADC_RX_Q.clear();
  ConfigData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  calibrateFlag = 0;                       // KF5N
  ConfigData.CWOffset = cwFreqOffsetTemp;  // Return user selected CW offset frequency.
  sineTone(ConfigData.CWOffset + 6);       // This function takes "number of cycles" which is the offset + 6.
  ConfigData.currentScale = userScale;     //  Restore vertical scale to user preference.  KF5N
  ShowSpectrumdBScale();
  ConfigData.transmitPowerLevel = transmitPowerLevelTemp;  // Restore the user's transmit power level setting.  KF5N August 15, 2023
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) bands.bands[ConfigData.currentBand].sideband = tempSideband;
  bands.bands[ConfigData.currentBand].mode = tempMode;
  radioState = tempState;
  zoomIndex = userZoomIndex - 1;
  button.ButtonZoom();                      // Restore the user's zoom setting.  Note that this function also modifies ConfigData.spectrum_zoom.
  if (saveToEeprom) eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
  tft.writeTo(L2);                          // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
  calOnFlag = false;
  if (not radioCal) RedrawDisplayScreen();  // Redraw everything!
  else tft.fillWindow();                    // Clear the display.
  fftOffset = 0;                            // Some reboots may be caused by large fftOffset values when Auto-Spectrum is on.
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  lastState = RadioState::NOSTATE;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  powerUp = true;                   // Clip off transient.
  return;
}


/*****
  Purpose: Combined input/output for the purpose of calibrating the receiver IQ.

   Parameter List:
      int mode (0 is CW, 1 is SSB), bool radioCal, bool shortCal, bool saveToEeprom

   Return value:
      void
 *****/
void CWCalibrate::DoReceiveCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom) {
  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  int calFreqShift;
  float correctionIncrement = 0.001;
  bool autoCal = false;
  bool refineCal = false;
  loadCalToneBuffers(3000.0);
  CalibratePreamble(0);                                                                       // Set zoom to 1X.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) calFreqShift = 24000;  //  LSB offset.  KF5N
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) calFreqShift = 24000;  //  USB offset.  KF5N
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(calFreqShift);
  calTypeFlag = 0;  // RX cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(correctionIncrement, 3);
  printCalType(mode, calTypeFlag, autoCal, false);
  warmUpCal(mode);
  State state = State::warmup;  // Start calibration state machine in warmup state.
  float maxSweepAmp = 0.2;
  float maxSweepPhase = 0.1;
  float increment = 0.002;
  int averageCount = 0;
  float iOptimal = 1.0;
  float qOptimal = 0.0;
  std::vector<float32_t> sweepVector(301);
  std::vector<float32_t> sweepVectorValue(301);
  elapsedMillis fiveSeconds;
  int viewTime = 0;
  bool averageFlag = false;
  std::vector<float>::iterator result;

  plotCalGraphics(calTypeFlag);
  printCalType(mode, calTypeFlag, autoCal, false);
  // Run this so Phase shows from begining.  Get the current values assigned to the local variables.
  if (mode == 0) {  // CW
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      GetEncoderValueLive(-2.0, 2.0, CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand], correctionIncrement, (char *)"IQ Phase", false);
      amplitude = CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand];
      phase = CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      GetEncoderValueLive(-2.0, 2.0, CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand], correctionIncrement, (char *)"IQ Phase", false);
      amplitude = CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand];
      phase = CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }
  if (mode == 1) {  // SSB
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
      GetEncoderValueLive(-2.0, 2.0, CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand], correctionIncrement, (char *)"IQ Phase", false);
      amplitude = CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand];
      phase = CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
    } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      GetEncoderValueLive(-2.0, 2.0, CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand], correctionIncrement, (char *)"IQ Phase", false);
      amplitude = CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand];
      phase = CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
    }
  }

  warmUpCal(mode);

  if (radioCal) {
    autoCal = true;
    printCalType(mode, calTypeFlag, autoCal, false);
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
    // Update values before calculating spectrum.
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
        CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
      }
    }
    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
        CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
      }
    }
    CWCalibrate::ShowSpectrum2(mode);
    task = readButton(lastUsedTask);
    if (shortCal) task = MenuSelect::FILTER;
    switch (task) {
      // Activate automatic calibration.
      case MenuSelect::ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
        state = State::warmup;
        averageFlag = false;
        break;
      // Automatic calibration using previously stored values.
      case MenuSelect::FILTER:  // 3rd row, 1st column button
        shortCal = false;
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);

        if (mode == 0) {
          if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
            iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand];
            ;
            qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
          } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
            iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand];
            ;
            qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
          }
        }
        if (mode == 1) {
          if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
            iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand];
            ;
            qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
          } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
            iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand];
            ;
            qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
          }
        }
        state = State::warmup;
        averageFlag = false;
        refineCal = true;
        break;
      // Toggle gain and phase
      case MenuSelect::UNUSED_1:
        IQCalType = !IQCalType;
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
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        CWCalibrate::CalibrateEpilogue(radioCal, saveToEeprom);
        return;
        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal || radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(mode, calTypeFlag, autoCal, false);
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
          if (warmup == 10) state = State::state0;
          if (warmup == 10 && refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          IQCalType = 0;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          phase = 0.0;
          amplitude = 1.0 - maxSweepAmp;  // Begin sweep at low end and move upwards.
          if (mode == 0)
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);  // Display the phase value.
          if (mode == 1)
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
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
            IQCalType = 1;                                                      // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            phase = -maxSweepPhase;                                             // The starting value for phase.
            amplitude = iOptimal;                                               // Reset for next sweep.
            // Update display to optimal value.
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
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
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
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
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
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
          avgState = averagingState::refineAmp;
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
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
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
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 3) {
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          averageCount = averageCount + 1;
          averageFlag = false;
          if (avgState == averagingState::refineAmp) state = State::refineAmp;
          if (avgState == averagingState::refinePhase) state = State::refinePhase;
          break;

        case State::setOptimal:
          count = 0;  // In case automatic calibration is run again.
          amplitude = iOptimal;
          phase = qOptimal;

          if (mode == 0) {
            if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
              iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand];
              ;
              qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
            } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
              iOptimal = amplitude = CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand];
              ;
              qOptimal = phase = CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
            }
          }
          if (mode == 1) {
            if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
              iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand];
              ;
              qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand];
            } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
              iOptimal = amplitude = CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand];
              ;
              qOptimal = phase = CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand];
            }
          }

          state = State::exit;
          viewTime = fiveSeconds;  // Start result view timer.
          break;
        case State::exit:
          if (radioCal) {
            printCalType(mode, calTypeFlag, autoCal, true);
            if ((fiveSeconds - viewTime) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              CalibrateEpilogue(radioCal, saveToEeprom);
              return;
            }
          }
          autoCal = false;
          refineCal = false;  // Reset refine cal.
          printCalType(mode, calTypeFlag, autoCal, false);
          break;
      }
    }  // end automatic calibration state machine

    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;                            // Reset task after it is used.
    //  Read encoder and update values.

    if (IQCalType == 0) {
      amplitude = GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
      if (mode == 0) {  // CW
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
          CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
          CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      }
      if (mode == 1) {  // SSB
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
          CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
        else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
          CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
      }
    } else {
      phase = GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
      if (mode == 0) {  // CW
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
          CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
        else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
          CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
      }
      if (mode == 1) {  // SSB
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
          CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
        else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
          CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
      }
    }
  }  // end while
}  // End Receive calibration


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int toneFreqIndex, bool radioCal, bool shortCal

   Return value:
      void
 *****/
void CWCalibrate::DoXmitCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom) {
  int freqOffset;
  float correctionIncrement = 0.001;
  CWCalibrate::State state = State::warmup;  // Start calibration state machine in warmup state.
  float maxSweepAmp = 0.1;
  float maxSweepPhase = 0.1;
  float increment = 0.002;
  int averageCount = 0;
  IQCalType = 0;  // Begin with gain optimization.
  float iOptimal = 1.0;
  float qOptimal = 0.0;
  std::vector<float32_t> sweepVector(351);
  std::vector<float32_t> sweepVectorValue(351);
  elapsedMillis fiveSeconds;
  int viewTime = 0;
  bool autoCal = false;
  bool refineCal = false;
  bool averageFlag = false;
  std::vector<float>::iterator result;
  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  CWCalibrate::CalibratePreamble(4);  // Set zoom to 16X.
  freqOffset = 0;                     // Calibration tone same as regular modulation tone.
  loadCalToneBuffers(750.0);
  calTypeFlag = 1;  // TX cal
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
  printCalType(mode, calTypeFlag, autoCal, false);
  // Get current values into the amplitude and phase working variables:
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    amplitude = CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand];
    phase = CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand];
  } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    amplitude = CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand];
    phase = CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand];
  }
  // Run this so Phase shows from begining.
  GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
  warmUpCal(mode);

  if (radioCal) {
    autoCal = true;
    printCalType(mode, calTypeFlag, autoCal, false);
    count = 0;
    warmup = 0;
    index = 1;
    IQCalType = 0;
    std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
    std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
    state = State::warmup;
  }

  // Transmit Calibration Loop
  while (true) {
    CWCalibrate::ShowSpectrum2(mode);
    task = readButton(lastUsedTask);
    if (shortCal) task = MenuSelect::FILTER;
    switch (task) {
      // Activate automatic calibration.
      case MenuSelect::ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 0;
        IQCalType = 0;
        averageFlag = false;
        averageCount = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
        iOptimal = amplitude;
        qOptimal = phase;
        state = State::warmup;
        break;
      // Automatic calibration using previously stored values.
      case MenuSelect::FILTER:  // 3rd row, 1st column button
        shortCal = false;
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        //stopSweep = false;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
        iOptimal = amplitude;
        qOptimal = phase;
        state = State::warmup;
        averageFlag = false;
        refineCal = true;
        break;
      // Toggle gain and phase
      case MenuSelect::UNUSED_1:
        IQCalType = !IQCalType;
        break;
      // Toggle increment value
      case MenuSelect::BEARING:  // UNUSED_2 is now called BEARING
        corrChange = !corrChange;
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
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
        CWCalibrate::CalibrateEpilogue(radioCal, saveToEeprom);
        return;
        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal || radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(mode, calTypeFlag, autoCal, false);
          //  stopSweep = false;
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            phase = 0.0 + maxSweepPhase;    //  Need to use these values during warmup
            amplitude = 1.0 + maxSweepAmp;  //  so adjdB and adjdB_avg are forced upwards.
          }
          state = State::warmup;
          if (warmup == 10) state = State::state0;
          if (warmup == 10 && refineCal) state = State::refineCal;
          break;
        case State::refineCal:
          // Prep the refinement arrays based on saved values.
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorAmp[i] = (iOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          for (int i = 0; i < 21; i = i + 1) {
            sub_vectorPhase[i] = (qOptimal - 10 * 0.001) + (0.001 * i);  // The next array to sweep.
          }
          IQCalType = 0;
          state = State::refineAmp;  // Skip the initial sweeps.
          break;
        case State::state0:
          // Starting values for sweeps.  First sweep is amplitude (gain).
          phase = 0.0;
          amplitude = 1.0 - maxSweepAmp;  // Begin sweep at low end and move upwards.
          GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
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
            IQCalType = 1;                                                      // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());  // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);         // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                         // Set to the discovered minimum.
            phase = -maxSweepPhase;                                             // The starting value for phase.
            amplitude = iOptimal;                                               // Reset for next sweep.
            // Update display to optimal value.
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
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
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
            // Save the sub_vector which will be used to refine the optimal result.
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
            GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
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
          avgState = averagingState::refineAmp;
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
            GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
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
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 3) {
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          averageCount = averageCount + 1;
          averageFlag = false;
          if (avgState == averagingState::refineAmp) state = State::refineAmp;
          if (avgState == averagingState::refinePhase) state = State::refinePhase;
          break;

        case State::setOptimal:
          count = 0;  // In case automatic calibration is run again.
          amplitude = iOptimal;
          phase = qOptimal;
          if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
            CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
            CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
          } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
            CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
            CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
          }
          state = State::exit;
          viewTime = fiveSeconds;  // Start result view timer.
          break;

        case State::exit:
          if (radioCal) {
            printCalType(mode, calTypeFlag, autoCal, true);
            if ((fiveSeconds - viewTime) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              CalibrateEpilogue(radioCal, saveToEeprom);
              return;
            }
          }
          autoCal = false;
          refineCal = false;  // Reset refine cal.
          printCalType(mode, calTypeFlag, autoCal, false);
          break;
      }
    }  // end automatic calibration state machine

    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;                            // Reset task after it is used.
    //  Read encoder and update values within the while loop.
    if (IQCalType == 0) {
      amplitude = GetEncoderValueLive(-2.0, 2.0, amplitude, correctionIncrement, (char *)"IQ Gain", true);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand] = amplitude;
      else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand] = amplitude;
    } else {
      phase = GetEncoderValueLive(-2.0, 2.0, phase, correctionIncrement, (char *)"IQ Phase", false);
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER)
        CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand] = phase;
      else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
        CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand] = phase;
    }
  }  // end while
}  // End Transmit calibration


/*****
  Purpose: Combined input/output for the purpose of calibrating the transmit IQ.

   Parameter List:
      int toneFreqIndex

   Return value:
      void
 *****/
#ifdef QSE2
void CWCalibrate::DoXmitCarrierCalibrate(int mode, bool radioCal, bool shortCal, bool saveToEeprom) {
  MenuSelect task, lastUsedTask = MenuSelect::DEFAULT;
  int freqOffset;
  int correctionIncrement = 10;
  State state = State::warmup;  // Start calibration state machine in warmup state.
  int maxSweepAmp = 450;
  int maxSweepPhase = 450;
  int increment = 5;
  int averageCount = 0;
  int iOptimal = 0;
  int qOptimal = 0;
  std::vector<float> sweepVector(181);
  std::vector<int> sweepVectorValue(181);
  std::vector<int> sub_vectorIoffset = std::vector<int>(21);
  std::vector<int> sub_vectorQoffset = std::vector<int>(21);
  elapsedMillis fiveSeconds;
  int viewTime = 0;
  bool autoCal = false;
  bool refineCal = false;
  bool averageFlag = false;
  std::vector<float>::iterator result;
  CWCalibrate::CalibratePreamble(4);  // Set zoom to 16X.
  freqOffset = 0;                     // Calibration tone same as regular modulation tone.
  loadCalToneBuffers(750.0);
  calTypeFlag = 2;  // Carrier cal
  plotCalGraphics(calTypeFlag);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(405, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(405, 125);
  tft.print(correctionIncrement);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  printCalType(mode, calTypeFlag, autoCal, false);
  // Get current values into the amplitude and phase working variables.
  // Here this is only used to get the correct sideband phasing during carrier calibration.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    amplitude = CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand];
    phase = CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand];
  } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    amplitude = CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand];
    phase = CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand];
  }
  // Run this so Phase shows from begining.
  GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"Q Offset", false);
  warmUpCal(mode);

  if (radioCal) {
    autoCal = true;
    printCalType(mode, calTypeFlag, autoCal, false);
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
    CWCalibrate::ShowSpectrum2(mode);
    task = readButton(lastUsedTask);
    if (shortCal) task = MenuSelect::FILTER;  // Jump to refineCal.
    switch (task) {
      // Activate automatic calibration.
      case MenuSelect::ZOOM:  // 2nd row, 1st column button
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 1;
        IQCalType = 0;
        averageFlag = false;
        averageCount = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0);
        iOptimal = CalData.iDCoffsetCW[ConfigData.currentBand];
        qOptimal = CalData.qDCoffsetCW[ConfigData.currentBand];
        state = State::warmup;
        break;
      // Automatic calibration using previously stored values.
      case MenuSelect::FILTER:  // 3rd row, 1st column button
        shortCal = false;
        autoCal = true;
        printCalType(mode, calTypeFlag, autoCal, false);
        count = 0;
        warmup = 0;
        index = 0;
        IQCalType = 0;
        std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0);
        std::fill(sweepVector.begin(), sweepVector.end(), 0);
        iOptimal = CalData.iDCoffsetCW[ConfigData.currentBand];
        qOptimal = CalData.qDCoffsetCW[ConfigData.currentBand];
        state = State::warmup;
        averageFlag = false;
        refineCal = true;
        break;
      // Toggle gain and phase
      case MenuSelect::UNUSED_1:
        IQCalType = !IQCalType;
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
        CWCalibrate::CalibrateEpilogue(radioCal, saveToEeprom);
        return;

        break;
      default:
        break;
    }  // end switch

    //  Begin automatic calibration state machine.
    if (autoCal || radioCal) {
      switch (state) {
        case State::warmup:
          autoCal = true;
          printCalType(mode, calTypeFlag, autoCal, false);
          std::fill(sweepVectorValue.begin(), sweepVectorValue.end(), 0.0);
          std::fill(sweepVector.begin(), sweepVector.end(), 0.0);
          warmup = warmup + 1;
          if (not refineCal) {
            CalData.qDCoffsetCW[ConfigData.currentBand] = maxSweepPhase;  //  Need to use these values during warmup
            CalData.iDCoffsetCW[ConfigData.currentBand] = maxSweepAmp;    //  so adjdB and adjdB_avg are forced upwards.
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
          CalData.qDCoffsetCW[ConfigData.currentBand] = 0;
          CalData.iDCoffsetCW[ConfigData.currentBand] = -maxSweepAmp;  // Begin sweep at low end and move upwards.
          GetEncoderValueLiveQ15t(1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"Q Offset", false);
          adjdB = 0;
          adjdB_avg = 0;
          index = 0;
          IQCalType = 0;
          state = State::initialSweepAmp;  // Let this fall through.

        case State::initialSweepAmp:
          sweepVectorValue[index] = CalData.iDCoffsetCW[ConfigData.currentBand];
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for next measurement.
          CalData.iDCoffsetCW[ConfigData.currentBand] = CalData.iDCoffsetCW[ConfigData.currentBand] + increment;  // Next one!
          // Go to Q channel when I channel sweep is finished.
          if (abs(CalData.iDCoffsetCW[ConfigData.currentBand] - 1.0) > maxSweepAmp) {  // Needs to be subtracted from 1.0.
            IQCalType = 1;                                                             // Get ready for phase.
            result = std::min_element(sweepVector.begin(), sweepVector.end());         // Value of the minimum.
            adjdBMinIndex = std::distance(sweepVector.begin(), result);                // Find the index.
            iOptimal = sweepVectorValue[adjdBMinIndex];                                // Set to the discovered minimum.
            CalData.qDCoffsetCW[ConfigData.currentBand] = -maxSweepPhase;              // The starting value for phase.
            CalData.iDCoffsetCW[ConfigData.currentBand] = iOptimal;                    // Reset for next sweep.
            // Update display to optimal value.
            GetEncoderValueLiveQ15t(-1000, 1000, CalData.iDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"I Offset", true);
            count = count + 1;
            // Save the sub_vector which will be used to refine the optimal result.
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10 * 5) + (5 * i);
            }
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
          sweepVectorValue[index] = CalData.qDCoffsetCW[ConfigData.currentBand];
          sweepVector[index] = adjdB;
          index = index + 1;
          // Increment for the next measurement.
          CalData.qDCoffsetCW[ConfigData.currentBand] = CalData.qDCoffsetCW[ConfigData.currentBand] + increment;
          if (CalData.qDCoffsetCW[ConfigData.currentBand] > maxSweepPhase) {
            result = std::min_element(sweepVector.begin(), sweepVector.end());
            adjdBMinIndex = std::distance(sweepVector.begin(), result);
            qOptimal = sweepVectorValue[adjdBMinIndex];              // Set to the discovered minimum.
            CalData.qDCoffsetCW[ConfigData.currentBand] = qOptimal;  // Set to the discovered minimum.
            GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"Q Offset", false);
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
          CalData.iDCoffsetCW[ConfigData.currentBand] = sub_vectorIoffset[index];  // Starting value.
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
            iOptimal = sub_vectorIoffset[adjdBMinIndex];             // -.001;
            CalData.iDCoffsetCW[ConfigData.currentBand] = iOptimal;  // Set to optimal value before refining phase.
            GetEncoderValueLiveQ15t(-1000, 1000, CalData.iDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"I Offset", true);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorIoffset[i] = (iOptimal - 10 * 5) + (5 * i);  // The next array to sweep.
            }
            IQCalType = 1;
            index = 0;
            averageFlag = false;
            averageCount = 0;
            count = count + 1;
            if (count == 1 || count == 3) state = State::refinePhase;  // Alternate refinePhase and refineAmp.
            break;
          }
          avgState = averagingState::refineAmp;
          state = State::average;
          break;

        case State::refinePhase:
          // Now sweep over the entire sub_vectorAmp array with averaging on. index starts at 0.
          CalData.qDCoffsetCW[ConfigData.currentBand] = sub_vectorQoffset[index];  // Starting value.
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
            CalData.qDCoffsetCW[ConfigData.currentBand] = qOptimal;
            GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"Q Offset", false);
            for (int i = 0; i < 21; i = i + 1) {
              sub_vectorQoffset[i] = (qOptimal - 10 * 5) + (5 * i);
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
          avgState = averagingState::refinePhase;
          state = State::average;
          break;

        case State::average:  // Stay in this state while averaging is in progress.
          if (averageCount > 5) {
            if (avgState == averagingState::refineAmp) state = State::refineAmp;
            if (avgState == averagingState::refinePhase) state = State::refinePhase;
            averageCount = 0;
            averageFlag = true;  // Averaging is complete!
            break;
          }
          averageCount = averageCount + 1;
          averageFlag = false;
          if (avgState == averagingState::refineAmp) state = State::refineAmp;
          if (avgState == averagingState::refinePhase) state = State::refinePhase;
          break;

        case State::setOptimal:
          count = 0;  // In case automatic calibration is run again.
          CalData.iDCoffsetCW[ConfigData.currentBand] = iOptimal;
          CalData.qDCoffsetCW[ConfigData.currentBand] = qOptimal;
          state = State::exit;
          viewTime = fiveSeconds;  // Start result view timer.
          break;
        case State::exit:
          if (radioCal) {
            printCalType(mode, calTypeFlag, autoCal, true);
            if ((fiveSeconds - viewTime) < 5000) {  // Show calibration result for 5 seconds at conclusion during Radio Cal.
              state = State::exit;
              break;
            } else {
              CalibrateEpilogue(radioCal, saveToEeprom);
              return;
            }
          }
          autoCal = false;
          refineCal = false;  // Reset refine cal.
          printCalType(mode, calTypeFlag, autoCal, false);
          break;
      }
    }  // end automatic calibration state machine

    if (task != MenuSelect::DEFAULT) lastUsedTask = task;  //  Save the last used task.
    task = MenuSelect::DEFAULT;                            // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      CalData.iDCoffsetCW[ConfigData.currentBand] = GetEncoderValueLiveQ15t(-1000, 1000, CalData.iDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"I Offset", true);
    } else {
      CalData.qDCoffsetCW[ConfigData.currentBand] = GetEncoderValueLiveQ15t(-1000, 1000, CalData.qDCoffsetCW[ConfigData.currentBand], correctionIncrement, (char *)"Q Offset", false);
    }
  }  // end while
}  // End carrier calibration
#endif


// Automatic calibration of all bands.  Greg KF5N June 4, 2024
void CWCalibrate::RadioCal(bool refineCal) {
  // Warn the user if the radio is not calibrated and refine cal is attempted.
  if (refineCal && not CalData.CWradioCalComplete) {
    tft.setFontScale((enum RA8875tsize)2);
    tft.setTextColor(RA8875_RED);
    tft.setCursor(20, 300);
    tft.print("RADIO NOT CALIBRATED");
    return;
  }

  //  80M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_80M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[0][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  40M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_40M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[1][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  20M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_20M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[2][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  17M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_17M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[3][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  15M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_15M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[4][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  12M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_12M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[5][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  //  10M
  ConfigData.currentBand = ConfigData.currentBandA = BAND_10M;
  SetBandRelay();
  TxRxFreq = CalData.calFrequencies[6][1];
  ConfigData.centerFreq = TxRxFreq;
  ShowFrequency();
  SetFreq();
  bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;  // Do LSB first.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
  bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // For FT8.
  CWCalibrate::DoXmitCalibrate(0, true, refineCal, false);
  CWCalibrate::DoReceiveCalibrate(0, true, refineCal, false);
#ifdef QSE2
  CWCalibrate::DoXmitCarrierCalibrate(0, true, refineCal, false);
#endif

  // Set flag for initial calibration completed.
  CalData.CWradioCalComplete = true;
  eeprom.CalDataWrite();
  RedrawDisplayScreen();
}


/*****
  Purpose: Signal processing for the purpose of calibration.

   Parameter List:
      int toneFreq, index of an array of tone frequencies.

   Return value:
      void
 *****/
void CWCalibrate::ProcessIQData2(int mode) {
  float rfGainValue, powerScale;                                   // AFP 2-11-23.  Greg KF5N February 13, 2023
  float recBandFactor[7] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };  // AFP 2-11-23  KF5N uniform values

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are read into two arrays sp_L and sp_R in blocks of 128 up to N_BLOCKS.  The arrays are
        of size BUFFER_SIZE * N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_LENGTH / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_LENGTH = 512
        BUFFER_SIZE*N_BLOCKS = 2048 samples
     **********************************************************************************/
  // Generate I and Q for the transmit or receive calibration.  KF5N                                  // KF5N
  arm_scale_f32(cosBuffer, 0.20, float_buffer_L_EX, 256);  // AFP 2-11-23 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32(sinBuffer, 0.20, float_buffer_R_EX, 256);  // AFP 2-11-23 Sidetone = 3000

  //if(mode == 0) {
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    arm_scale_f32(float_buffer_L_EX, -amplitude, float_buffer_L_EX, 256);  //Adjust level of L buffer // AFP 2-11-23
    IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, phase, 256);   // Adjust phase
  } else {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      arm_scale_f32(float_buffer_L_EX, amplitude, float_buffer_L_EX, 256);  // AFP 2-11-23
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, phase, 256);
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
  if (static_cast<uint32_t>(ADC_RX_I.available()) > 16 && static_cast<uint32_t>(ADC_RX_Q.available()) > 16) {  // Audio Record Queues!!!
    q15_t q15_buffer_LTemp[2048];                                                                              //KF5N
    q15_t q15_buffer_RTemp[2048];                                                                              //KF5N
    Q_out_L_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
    Q_out_R_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
    arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
    arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);

#ifdef QSE2
    arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, 2048);
    arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, 2048);
#endif

    Q_out_L_Ex.play(q15_buffer_LTemp, 2048);
    Q_out_R_Ex.play(q15_buffer_RTemp, 2048);
    Q_out_L_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
    Q_out_R_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);

    // End of transmit code.  Begin receive code.

    // get audio samples from the audio  buffers and convert them to float
    // read in 32 blocks á 128 samples in I and Q if available.

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
    arm_scale_f32(float_buffer_L, recBandFactor[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23
    arm_scale_f32(float_buffer_R, recBandFactor[ConfigData.currentBand], float_buffer_R, BUFFER_SIZE * N_BLOCKS);  //AFP 2-11-23

    // Manual IQ amplitude correction
    if (mode == 0) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, -CalData.IQCWRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorLSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      } else {
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
          arm_scale_f32(float_buffer_L, -CalData.IQCWRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
          IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQCWRXPhaseCorrectionFactorUSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
        }
      }
    }

    if (mode == 1) {
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        arm_scale_f32(float_buffer_L, -CalData.IQSSBRXAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22
        IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorLSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
      } else {
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
          arm_scale_f32(float_buffer_L, -CalData.IQSSBRXAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L, BUFFER_SIZE * N_BLOCKS);  //AFP 04-14-22 KF5N changed sign
          IQPhaseCorrection(float_buffer_L, float_buffer_R, CalData.IQSSBRXPhaseCorrectionFactorUSB[ConfigData.currentBand], BUFFER_SIZE * N_BLOCKS);
        }
      }
    }

    FreqShift1();  // Why done here? KF5N

    if (ConfigData.spectrum_zoom == SPECTRUM_ZOOM_1) {  // && display_S_meter_or_spectrum_state == 1)
      zoom_display = 1;
      CalcZoom1Magn();  //AFP Moved to display function
    }

    // ZoomFFTExe is being called too many times in Calibration.  Should be called ONLY at the start of each sweep.
    if (ConfigData.spectrum_zoom != SPECTRUM_ZOOM_1) {
      //AFP  Used to process Zoom>1 for display
      ZoomFFTExe(BUFFER_SIZE * N_BLOCKS);
    }
  }  // End of receive code.
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
void CWCalibrate::ShowSpectrum2(int mode)  //AFP 2-10-23
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
  if (calTypeFlag == 0 && bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    cal_bins[0] = 315;
    cal_bins[1] = 455;
  }  // Receive calibration, LSB.  KF5N
  if (calTypeFlag == 0 && bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    cal_bins[0] = 59;
    cal_bins[1] = 199;
  }  // Receive calibration, USB.  KF5N
  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    cal_bins[0] = 257;  // LSB
    cal_bins[1] = 289;  // Carrier
    cal_bins[2] = 322;  // Undesired sideband
  }                     // Transmit and Carrier calibration, LSB.  KF5N
  if ((calTypeFlag == 1 || calTypeFlag == 2) && bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    cal_bins[0] = 257;  // USB
    cal_bins[1] = 225;  // Carrier
    cal_bins[2] = 193;  // Undesired sideband
  }                     // Transmit and Carrier calibration, LSB.  KF5N

  //  There are 2 for-loops, one for the reference signal and another for the undesired sideband.
  if (calTypeFlag == 0) {  // Receive cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
  }

  // Plot carrier during transmit cal, do not return a dB value:
  if (calTypeFlag == 1) {  // Transmit cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) PlotCalSpectrum(mode, x1, cal_bins, capture_bins);  // Carrier
  }
  if (calTypeFlag == 2) {  // Carrier cal
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(mode, x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) PlotCalSpectrum(mode, x1, cal_bins, capture_bins);  // Undesired sideband
  }

  tft.setCursor(350, 142);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(350, 142, 50, tft.getFontHeight(), RA8875_BLACK);  // Erase old adjdB number.                                       // 350, 125
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
float CWCalibrate::PlotCalSpectrum(int mode, int x1, int cal_bins[3], int capture_bins) {
  int16_t adjAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  int16_t refAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  float alpha = 0.01;

  uint32_t index_of_max;  // This variable is not currently used, but it is required by the ARM max function.  KF5N
  int y_new_plot, y1_new_plot, y_old_plot, y_old2_plot;

  // The FFT should be performed only at the beginning of the sweep, and buffers must be full.
  if (x1 == (cal_bins[0] - capture_bins)) {  // Set flag at revised beginning.  KF5N
    updateDisplayFlag = true;                // This flag is used in ZoomFFTExe().
  } else updateDisplayFlag = false;          //  Do not save the the display data for the remainder of the sweep.

  CWCalibrate::ProcessIQData2(mode);  // Call the Audio process from within the display routine to eliminate conflicts with drawing the spectrum.

  y_new = pixelnew[x1];
  y1_new = pixelnew[x1 - 1];
  y_old = pixelold[x1];
  y_old2 = pixelold[x1 - 1];

  // Find the maximums of the desired and undesired signals.

  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) && (calTypeFlag == 0)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
  }
  if ((bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) && (calTypeFlag == 0)) {
    arm_max_q15(&pixelnew[(cal_bins[0] - capture_bins)], capture_bins * 2, &adjAmplitude, &index_of_max);
    arm_max_q15(&pixelnew[(cal_bins[1] - capture_bins)], capture_bins * 2, &refAmplitude, &index_of_max);
  }

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

  adjdB = ((float)adjAmplitude - (float)refAmplitude) / (1.95 * 2.0);                                            // Cast to float and calculate the dB level.  Needs further refinement for accuracy.  KF5N
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER && not(calTypeFlag == 0)) adjdB = -adjdB;  // Flip sign for USB only for TX cal.
  adjdB_avg = adjdB * alpha + adjdBold * (1.0 - alpha);                                                          // Exponential average.
                                                                                                                 //  adjdB_avg = adjdB;     // TEMPORARY EXPERIMENT
  adjdBold = adjdB_avg;
  adjdB_sample = adjdB;

  tft.writeTo(L1);
  return adjdB_avg;
}
