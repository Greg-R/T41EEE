
#include "SDT.h"

// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

int val;
float correctionIncrement;  //AFP 2-7-23
int userScale, userZoomIndex, userxmtMode;
int transmitPowerLevelTemp, cwFreqOffsetTemp;
uint16_t base_y = SPECTRUM_BOTTOM;
int calTypeFlag = 0;
int IQCalType;


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
  calOnFlag = 1;
  //corrChange = false;
  //correctionIncrement = 0.01;  //AFP 2-7-23
  IQCalType = 0;
  radioState = CW_TRANSMIT_STRAIGHT_STATE;                 // KF5N
  transmitPowerLevelTemp = EEPROMData.transmitPowerLevel;  //AFP 05-11-23
  cwFreqOffsetTemp = EEPROMData.CWOffset;
  EEPROMData.CWOffset = 2;  // 750 Hz for TX calibration.  Prologue restores user selected offset.
  //EEPROMData.transmitPowerLevel = 5;  //AFP 02-09-23  Set to 5 watts as a precaution to protect the power amplifier in case it is connected.
  //EEPROMData.powerOutCW[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand];
  //  Serial.printf("preamble EEPROMData.powerOutCW = %f\n", EEPROMData.powerOutCW[EEPROMData.currentBand]);
  //modeSelectOutExL.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
  //modeSelectOutExR.gain(0, EEPROMData.powerOutCW[EEPROMData.currentBand]);  //AFP 10-21-22
  //modeSelectOutExL.gain(0, 1);               //AFP 10-21-22
  //modeSelectOutExR.gain(0, 1);               //AFP 10-21-22
  patchCord15.connect();  // Connect the I and Q output channels so the transmitter will work.
  patchCord16.connect();
  userxmtMode = EEPROMData.xmtMode;          // Store the user's mode setting.  KF5N July 22, 2023
  userZoomIndex = EEPROMData.spectrum_zoom;  // Save the zoom index so it can be reset at the conclusion.  KF5N August 12, 2023
  zoomIndex = setZoom - 1;
  loadCalToneBuffers();  // Restore in the prologue.
  ButtonZoom();
  RedrawDisplayScreen();  // Erase any existing spectrum trace data.
  tft.writeTo(L2);        // Erase the bandwidth bar.  KF5N August 16, 2023
  tft.clearMemory();
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(350, 160);
  tft.print("user1 - Gain/Phase");
  tft.setCursor(350, 175);
  tft.print("User2 - Incr");
  tft.setTextColor(RA8875_CYAN);
  tft.fillRect(350, 125, 100, tft.getFontHeight(), RA8875_BLACK);
  tft.fillRect(0, 272, 517, 399, RA8875_BLACK);  // Erase waterfall.  KF5N August 14, 2023
  tft.setCursor(400, 125);
  tft.print("dB");
  tft.setCursor(350, 110);
  tft.print("Incr= ");
  //  tft.setCursor(400, 110);
  //  tft.print(correctionIncrement, 3);
  userScale = EEPROMData.currentScale;  //  Remember user preference so it can be reset when done.  KF5N
  EEPROMData.currentScale = 1;          //  Set vertical scale to 10 dB during calibration.  KF5N
  updateDisplayFlag = 0;
  digitalWrite(MUTE, LOW);  //turn off mute
  xrState = RECEIVE_STATE;
  T41State = CW_RECEIVE;
//  modeSelectInR.gain(0, 1);
//  modeSelectInL.gain(0, 1);
//  modeSelectInExR.gain(0, 0);  2nd microphone channel not required.  KF5N March 11, 2024
//  modeSelectInExL.gain(0, 0);
  patchCord1.disconnect();  // Disconnect microphone. modeSelectInExL replaced with patchcord.  KF5N March 11, 2024
//  modeSelectOutL.gain(0, 1);
//  modeSelectOutR.gain(0, 1);
//  modeSelectOutL.gain(1, 0);
//  modeSelectOutR.gain(1, 0);
//  modeSelectOutExL.gain(0, 1);
//  modeSelectOutExR.gain(0, 1);
  EEPROMData.centerFreq = TxRxFreq;
  NCOFreq = 0L;
  xrState = TRANSMIT_STATE;
  digitalWrite(MUTE, HIGH);  //  Mute Audio  (HIGH=Mute)
  digitalWrite(RXTX, HIGH);  // Turn on transmitter.
  ShowTransmitReceiveStatus();
  ShowSpectrumdBScale();
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
  updateDisplayFlag = 0;
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
  RedrawDisplayScreen();
  IQChoice = 9;
  calOnFlag = 0;
  radioState = CW_RECEIVE_STATE;  // KF5N
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreq();         // Return Si5351 to normal operation mode.  KF5N
  lastState = 1111;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 16, 2023
  return;
}




/*****
  Purpose: Combined input/ output for the purpose of calibrating the receive IQ

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
                                                                              //  if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) calFreqShift = 24000 - 2000;  //  LSB offset.  KF5N
                                                                              //  if (bands[EEPROMData.currentBand].mode == DEMOD_USB) calFreqShift = 24000 + 2250;  //  USB offset.  KF5N
  if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) calFreqShift = 24000;  //  LSB offset.  KF5N
  if (bands[EEPROMData.currentBand].mode == DEMOD_USB) calFreqShift = 24000;  //  USB offset.  KF5N
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(calFreqShift);
  calTypeFlag = 0;  // RX cal

  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(correctionIncrement);

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
        //        EEPROMData.EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand];
        //        EEPROMData.EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand];
        IQChoice = 6;
        break;
      default:
        break;
    }                                     // End switch
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
  Purpose: Combined input/ output for the purpose of calibrating the transmit IQ

   Parameter List:
      void

   Return value:
      void
 *****/
void DoXmitCalibrate(int toneFreqIndex) {
  int task = -1;
  int lastUsedTask = -2;
  int freqOffset;
  bool corrChange = false;
  float correctionIncrement = 0.01;

  if (toneFreqIndex == 0) {  // 750 Hz
    CalibratePreamble(4);    // Set zoom to 16X.
    freqOffset = 0;          // Calibration tone same as regular modulation tone.
  }
  if (toneFreqIndex == 1) {  // 3 kHz
    CalibratePreamble(2);    // Set zoom to 4X.
    freqOffset = 2250;       // Need 750 + 2250 = 3 kHz
  }
  calTypeFlag = 1;  // TX cal

  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(correctionIncrement);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  tft.writeTo(L1);
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
        //        EEPROMData.EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand];
        //        EEPROMData.EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand];
        IQChoice = 6;  // AFP 2-11-23
        break;
      default:
        break;
    }                                     // end switch
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
}


/*****
  Purpose: Combined input/ output for the purpose of calibrating the transmit IQ

   Parameter List:
      void

   Return value:
      void
 *****/
//  q15_t iDCoffset;
//  q15_t qDCoffset;
#ifdef QSE2
void DoXmitCarrierCalibrate(int toneFreqIndex) {
  int task = -1;
  int lastUsedTask = -2;
  int freqOffset;
  bool corrChange = true;
  int increment = 10;

  if (toneFreqIndex == 0) {  // 750 Hz
    CalibratePreamble(4);    // Set zoom to 16X.
    freqOffset = 0;          // Calibration tone same as regular modulation tone.
  }
  if (toneFreqIndex == 1) {  // 3 kHz
    CalibratePreamble(2);    // Set zoom to 4X.
    freqOffset = 2250;       // Need 750 + 2250 = 3 kHz
  }
  calTypeFlag = 2;  // TX carrier calibration.

  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(400, 110, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(400, 110);
  tft.print(increment);
  if ((MASTER_CLK_MULT_RX == 2) || (MASTER_CLK_MULT_TX == 2)) ResetFlipFlops();
  SetFreqCal(freqOffset);
  tft.writeTo(L1);
  // Transmit Calibration Loop.  This is independent of loop().
  while (true) {
    ShowSpectrum2();
    val = ReadSelectedPushButton();
    if (val != BOGUS_PIN_READ) {
      val = ProcessButtonPress(val);
      if (val != lastUsedTask && task == -100) task = val;
      else task = BOGUS_PIN_READ;
    }
    switch (task) {
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
        //        EEPROMData.EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQAmpCorrectionFactor[EEPROMData.currentBand];
        //        EEPROMData.EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand] = EEPROMData.IQPhaseCorrectionFactor[EEPROMData.currentBand];
        IQChoice = 6;  // AFP 2-11-23
        break;

      default:
        break;
    }                                     // end switch
    if (task != -1) lastUsedTask = task;  //  Save the last used task.
    task = -100;                          // Reset task after it is used.
    //  Read encoder and update values.
    if (IQCalType == 0) {
      EEPROMData.iDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.iDCoffset[EEPROMData.currentBand], increment, (char *)"I Offset:");
    } else {
      EEPROMData.qDCoffset[EEPROMData.currentBand] = (q15_t)GetEncoderValueLiveQ15t(-1000, 1000, EEPROMData.qDCoffset[EEPROMData.currentBand], increment, (char *)"Q Offset:");
    }
    if (IQChoice == 6) break;  //  Exit the while loop.
  }                            // end while
  CalibratePrologue();
}
#endif

/*****
  Purpose: Signal processing for the purpose of calibrating the transmit IQ

   Parameter List:
      int toneFreq, index of an array of tone frequencies.

   Return value:
      void
 *****/
void ProcessIQData2(int toneFreqIndex) {
  //  float bandOutputFactor;                                          // AFP 2-11-23
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
  powerScale = 30.0 * EEPROMData.powerOutCW[EEPROMData.currentBand];
  //  192KHz effective sample rate here
  arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048);  //Scale to compensate for losses in Interpolation
  arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

  // This code block was introduced after TeensyDuino 1.58 appeared.  It doesn't use a for loop, but processes the entire 2048 buffer in one pass.
  // are there at least N_BLOCKS buffers in each channel available ?
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

    usec = 0;
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
  float adjdB = 0.0;
  int capture_bins = 10;  // Sets the number of bins to scan for signal peak.

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
  //  if (calTypeFlag == 2 && bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
  //    cal_bins[0] = 257;  // Carrier
  //    cal_bins[1] = 289;
  //    cal_bins[2]
  //  }  // Transmit calibration, LSB.  KF5N
  //  if (calTypeFlag == 2 && bands[EEPROMData.currentBand].mode == DEMOD_USB) {
  //    cal_bins[0] = 193;
  //    cal_bins[1] = 257;  // Carrier
  //  }  // Transmit calibration, USB.  KF5N

  // Draw vertical markers for the reference, undesired sideband locations, and centerline.  For debugging only!
  //    tft.drawFastVLine(cal_bins[0], SPECTRUM_TOP_Y, h, RA8875_GREEN);
  //    tft.drawFastVLine(cal_bins[1], SPECTRUM_TOP_Y, h, RA8875_GREEN);
  //    tft.drawFastVLine(cal_bins[2], SPECTRUM_TOP_Y, h, RA8875_GREEN);
  //    int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2;    //  (512 + 3)/2 = 257.5
  //    tft.drawFastVLine(centerLine, SPECTRUM_TOP_Y, h, RA8875_GREEN);  // Draws centerline on spectrum display

  //  There are 2 for-loops, one for the reference signal and another for the undesired sideband.
  if (calTypeFlag == 0) {
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
  }

  // Plot carrier during transmit cal, do not return a dB value:
  //  if (calTypeFlag == 1)
  //  for (x1 = cal_bins[0] + 20; x1 < cal_bins[1] - 20; x1++) PlotCalSpectrum(x1, cal_bins, capture_bins);
  if (calTypeFlag == 1) {
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) PlotCalSpectrum(x1, cal_bins, capture_bins);  // Carrier
  }
  if (calTypeFlag == 2) {
    for (x1 = cal_bins[0] - capture_bins; x1 < cal_bins[0] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[1] - capture_bins; x1 < cal_bins[1] + capture_bins; x1++) adjdB = PlotCalSpectrum(x1, cal_bins, capture_bins);
    for (x1 = cal_bins[2] - capture_bins; x1 < cal_bins[2] + capture_bins; x1++) PlotCalSpectrum(x1, cal_bins, capture_bins);  // Undesired sideband
  }

  // Finish up:
  //= AFP 2-11-23
  tft.fillRect(350, 125, 50, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(350, 125);  // 350, 125
  tft.print(adjdB, 1);

  //  At least a partial waterfall is necessary.  It seems to provide some important timing function.  KF5N August 14, 2023
  tft.BTE_move(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE, MAX_WATERFALL_WIDTH, MAX_WATERFALL_ROWS - 2, WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, 1, 2);
  while (tft.readStatus())
    ;
}

/*****
  Purpose:  Plot Calibration Spectrum   //  KF5N 7/2/2023
            This function plots a partial spectrum during calibration only.
            This is intended to increase the efficiency and therefore the responsiveness of the calibration encoder.
            This function is called by ShowSpectrum2() in two for-loops.  One for-loop is for the reference signal,
            and the other for-loop is for the undesired sideband.
  Parameter list:
    int x1, where x1 is the FFT bin.
    cal_bins[2] locations of the desired and undesired signals
    capture_bins width of the bins used to display the signals
  Return value;
    float returns the adjusted value in dB
*****/
float PlotCalSpectrum(int x1, int cal_bins[3], int capture_bins) {
  float adjdB = 0.0;
  int16_t adjAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  int16_t refAmplitude = 0;  // Was float; cast to float in dB calculation.  KF5N
  uint32_t index_of_max;     // This variable is not currently used, but it is required by the ARM max function.  KF5N

  // The FFT should be performed only at the beginning of the sweep, and buffers must be full.
  if (x1 == (cal_bins[0] - capture_bins)) {  // Set flag at revised beginning.  KF5N
    updateDisplayFlag = 1;                   // This flag is used in ZoomFFTExe().
    ShowBandwidth();                         // Without this call, the calibration value in dB will not be updated.  KF5N
  } else updateDisplayFlag = 0;              //  Do not save the the display data for the remainder of the

  ProcessIQData2(EEPROMData.calFreq);  // Call the Audio process from within the display routine to eliminate conflicts with drawing the spectrum.

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
  if (y_new > base_y) y_new = base_y;
  if (y_old > base_y) y_old = base_y;
  if (y_old2 > base_y) y_old2 = base_y;
  if (y1_new > base_y) y1_new = base_y;

  if (y_new < 0) y_new = 0;
  if (y_old < 0) y_old = 0;
  if (y_old2 < 0) y_old2 = 0;
  if (y1_new < 0) y1_new = 0;

  // Erase the old spectrum and draw the new spectrum.
  tft.drawLine(x1, EEPROMData.spectrumNoiseFloor - y_old2, x1, EEPROMData.spectrumNoiseFloor - y_old, RA8875_BLACK);   // Erase old...
  tft.drawLine(x1, EEPROMData.spectrumNoiseFloor - y1_new, x1, EEPROMData.spectrumNoiseFloor - y_new, RA8875_YELLOW);  // Draw new
  pixelCurrent[x1] = pixelnew[x1];                                                                                     //  This is the actual "old" spectrum! Copied to pixelold by the FFT function.

  adjdB = ((float)adjAmplitude - (float)refAmplitude) / 1.95;  // Cast to float and calculate the dB level.  KF5N
  if(bands[EEPROMData.currentBand].mode == DEMOD_USB && not (calTypeFlag == 0)) adjdB = -adjdB;  // Flip sign for USB only for TX cal.

  if (calTypeFlag == 0) {                                      // Receive Cal
                                                               //    adjdB = ((float)adjAmplitude - (float)refAmplitude) / 1.95;
    tft.writeTo(L2);
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(445, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);     // SPECTRUM_TOP_Y = 100
      tft.fillRect(304, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);  // h = SPECTRUM_HEIGHT + 3
    } else {                                                           // SPECTRUM_HEIGHT = 150 so h = 153
      tft.fillRect(50, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);
      tft.fillRect(188, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);
    }
  }
  if (calTypeFlag == 1) {  //Transmit Cal
                           //    adjdB = ((float)adjAmplitude - (float)refAmplitude) / 1.95;  // Cast to float and calculate the dB level.  KF5N
    tft.writeTo(L2);
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(312, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);
    } else {
      if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {  //mode == DEMOD_LSB
        tft.fillRect(183, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);
        tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);
      }
    }
  }
  if (calTypeFlag == 2) {  //Transmit Cal
    tft.writeTo(L2);
    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
      tft.fillRect(279, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);  // Adjusted height due to other graphics changes.  KF5N August 3, 2023
      tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);
    } else {
      if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {  //mode == DEMOD_LSB
        tft.fillRect(215, SPECTRUM_TOP_Y + 20, 20, h - 6, DARK_RED);
        tft.fillRect(247, SPECTRUM_TOP_Y + 20, 20, h - 6, RA8875_BLUE);
      }
    }
  }
  tft.writeTo(L1);
  return adjdB;
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
void SelectCalFreq() {
  EEPROMData.calFreq = SubmenuSelect(calFreqs, 2, EEPROMData.calFreq);  // Returns the index of the array.
  //  RedrawDisplayScreen();  Kills the bandwidth graphics in the audio display window, remove. KF5N July 30, 2023
  // Clear the current CW filter graphics and then restore the bandwidth indicator bar.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  RedrawDisplayScreen();
}


// Consolidate transmit code into a function?
/*
FASTRUN void playTransmitData() {
//  if ((uint32_t)Q_in_L.available() > N_BLOCKS && (uint32_t)Q_in_R.available() > N_BLOCKS) {
  //  Try for loop from much earlier versions to see if this code block can be commonized.
    for (unsigned  i = 0; i < 16; i++) {  //N_BLOCKS_EX=16  BUFFER_SIZE=128 16x128=2048
      // Assign pointers to the Teensy Audio buffers.  The data will be played via the Teensy audio system.
      // Q_out_L_Ex.getBuffer()  This returns a pointer to a 128 size buffer.  Use this in arm functions to process streaming data.

      // Transmitter I and Q.  Cast the array from float to q15_t.  q15_t is equivalent to int16_t.
      arm_float_to_q15 (&float_buffer_L_EX[BUFFER_SIZE * i], Q_out_L_Ex.getBuffer(), BUFFER_SIZE);  // source, destination, number of samples
      arm_float_to_q15 (&float_buffer_R_EX[BUFFER_SIZE * i], Q_out_R_Ex.getBuffer(), BUFFER_SIZE);

      // Sidetone.  Only one channel is used.  Cast the array from float to q15_t.
      arm_float_to_q15 (&float_buffer_LTemp[BUFFER_SIZE * i], Q_out_L.getBuffer(), BUFFER_SIZE);  // source, destination, number of samples

    // Inject the DC offset from carrier calibration.  There is an ARM function for this.
      arm_offset_q15(Q_out_L_Ex.getBuffer(), EEPROMData.iDCoffset[EEPROMData.currentBand] + 1900, Q_out_L_Ex.getBuffer(), 128);
      arm_offset_q15(Q_out_R_Ex.getBuffer(), EEPROMData.qDCoffset[EEPROMData.currentBand] + 1900, Q_out_R_Ex.getBuffer(), 128);

      Q_out_L_Ex.playBuffer(); // Transmitter
      Q_out_R_Ex.playBuffer(); // Transmitter
      Q_out_L.playBuffer();    // Sidetone, play L channel only.
    }
  }
*/