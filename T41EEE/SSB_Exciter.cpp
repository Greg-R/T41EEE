
#include "SDT.h"

//int micGainChoice;

// This function sets the microphone gain and compressor parameters.  Greg KF5N March 9, 2025.
void updateMic() {

  micGain.setGain_dB(ConfigData.micGain);  // Set the microphone gain.

  struct compressionCurve crv = { -3.0, 0.0,  // margin, offset
                                  { 0.0, -10.0, ConfigData.micThreshold, -1000.0f, -1000.0f },  
                                  { 10.0, ConfigData.micCompRatio, 1.0f, 1.0, 1.0 } };

  int16_t delaySize = 256;                    // Any power of 2, i.e., 256, 128, 64, etc.
  compressor1.setDelayBufferSize(delaySize);  // Improves transient response of compressor.
  compressor1.setAttackReleaseSec(0.005f, 2.0f);  // Same as used in Tiny Ten by Bob W7PUA.
  compressor1.setCompressionCurve(&crv);
  compressor1.begin();
}


/*****
  Purpose: Retrieve I and Q samples from the Open Audio Library CESSB object at 48ksps.
           Apply calibration factors and scale for power.  Push the modified I and Q
           back into the Teensy audio system to drive the Audio Adapter outputs which
           are connected to the QSE.

  Parameter list: none

  Return value;
    void
    Notes:
    There are several actions in this function
    1.  Read in the I and Q data from the CESSB object.  16 blocks of 128 samples each.
    2.  Apply magnitude and phase calibration factors.
    3.  Scale for power.
    4.  Push the blocks back to the Teensy audio system to the Audio Adapter outputs
        at 48ksps.
*****/

void ExciterIQData() {
  uint32_t N_BLOCKS_EX = N_B_EX;
  float32_t powerScale;

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are  read into two arrays in blocks of 128 up to N_BLOCKS.  The arrays are
        of size BUFFER_SIZE*N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_L / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_L = 512
        BUFFER_SIZE * N_BLOCKS = 2048 samples
     **********************************************************************************/
  // are there at least N_BLOCKS buffers in each channel available ?
  if ((uint32_t)Q_in_L_Ex.available() > N_BLOCKS_EX) {

    // get audio samples from the audio  buffers and convert them to float
    // read in 32 blocks á 128 samples in I and Q
    for (unsigned i = 0; i < N_BLOCKS_EX; i++) {

      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
      arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // Right channel not used.  KF5N March 11, 2024
      Q_in_L_Ex.freeBuffer();
      Q_in_R_Ex.freeBuffer();  // Right channel not used.  KF5N March 11, 2024
    }

    // Set the sideband.
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

    // Apply amplitude and phase corrections.  FT8 uses CW corrections and is always USB.
    if(bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    if(bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE)
    cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand], CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand], 0.0);
    else if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE)
    cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
    } else if(bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    if(bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE)
    cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
    else if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE)
    cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
    }

    //  This is the correct place in the data flow to inject the scaling for power.
#ifdef QSE2
    powerScale = 2.0 * ConfigData.powerOutSSB[ConfigData.currentBand];
#else
    powerScale = 1.4 * ConfigData.powerOutSSB[ConfigData.currentBand];
#endif

    arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048);  //Scale to compensate for losses in Interpolation
    arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

    /**********************************************************************************  AFP 12-31-20
      CONVERT TO INTEGER AND PLAY AUDIO
    **********************************************************************************/
    q15_t q15_buffer_LTemp[2048];  // KF5N
    q15_t q15_buffer_RTemp[2048];  // KF5N

    arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);
    arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);
#ifdef QSE2
    if(bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
    arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_LTemp, 2048);  // Carrier suppression offset.
    arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_RTemp, 2048);
    } else if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {
    arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, 2048);  // Carrier suppression offset.
    arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, 2048);      
    }
#endif
    Q_out_L_Ex.play(q15_buffer_LTemp, 2048);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
    Q_out_R_Ex.play(q15_buffer_RTemp, 2048);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.
  }
}

/*****
  Purpose: Set the current band relay ON or OFF.  Reduce relay cycling.  Greg KF5N March 24, 2025

  Parameter list:
    void

  Return value;
    void
*****/
void SetBandRelay() {
  // There are 4 physical relays in the case of the V10/V11 LPF board.
  for (int i = 0; i < 5; i = i + 1) {
    if(i == ConfigData.currentBand) {
    digitalWrite(bandswitchPins[ConfigData.currentBand], HIGH);
    }
    else { 
      if(bandswitchPins[i] != bandswitchPins[ConfigData.currentBand])  // Skip if the pins are the same.
      digitalWrite(bandswitchPins[i], LOW);  // Set band relay low.
    }
  }
}


/*****
  Purpose: Allow user to set the microphone compression level.
           A typical value is -20 dB.
  Parameter list:
    void

  Return value;
    void
*****/
void SetCompressionThreshold() {
  MenuSelect menu, lastUsedTask = MenuSelect::DEFAULT;

  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 48, MENUS_Y + 1);
  tft.print("Comp Thresh dB:");
  tft.setCursor(SECONDARY_MENU_X + 195, MENUS_Y + 1);
  tft.print(ConfigData.micThreshold, 0);

  while (true) {
    if (filterEncoderMove != 0) {
      ConfigData.micThreshold += ((float)filterEncoderMove);
      if (ConfigData.micThreshold < -60)
        ConfigData.micThreshold = -60;
      else if (ConfigData.micThreshold > 0)  // 100% max
        ConfigData.micThreshold = 0;

      tft.fillRect(SECONDARY_MENU_X + 195, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 195, MENUS_Y + 1);
      tft.print(ConfigData.micThreshold, 0);
      filterEncoderMove = 0;
    }
    /*
    val = ReadSelectedPushButton();  // Read pin that controls all switches
    menu = ProcessButtonPress(val);
    delay(150L);
    */
    menu = readButton(lastUsedTask);
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      updateMic();
      eeprom.ConfigDataWrite();
      UpdateCompressionField();
      break;
    }
  }
  EraseMenus();
}


/*****
  Purpose: Allow user to set the microphone compression ratio.
           A typical value is in the 10 to 100 range.
  Parameter list:
    void

  Return value;
    void
*****/
void SetCompressionRatio() {
  MenuSelect menu, lastUsedTask = MenuSelect::DEFAULT;

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 48, MENUS_Y + 1);
  tft.print("Comp Ratio:");
  tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
  tft.print(ConfigData.micCompRatio, 0);

  while (true) {
    if (filterEncoderMove != 0) {
      ConfigData.micCompRatio += ((float)filterEncoderMove * 1.0);
      if (ConfigData.micCompRatio > 1000)
        ConfigData.micCompRatio = 1000;
      else if (ConfigData.micCompRatio < 1)  // 100% max
        ConfigData.micCompRatio = 1;

      tft.fillRect(SECONDARY_MENU_X + 180, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
      tft.print(ConfigData.micCompRatio, 0);
      filterEncoderMove = 0;
    }
menu = readButton(lastUsedTask);
/*
    val = ReadSelectedPushButton();  // Read pin that controls all switches
    menu = ProcessButtonPress(val);
    delay(150L);
    */

    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      // ConfigData.ConfigData.micCompRatio = ConfigData.micCompRatio;
      updateMic();  // This updates the compression ratio and the threshold.
      eeprom.ConfigDataWrite();
      break;
    }
  }
  EraseMenus();
}


/*****
  Purpose: Set microphone gain.  The default is 0 dB.

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
void MicGainSet() {
  MenuSelect menu, lastUsedTask = MenuSelect::DEFAULT;
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 48, MENUS_Y + 1);
  tft.print("Mic Gain dB:");
  tft.setCursor(SECONDARY_MENU_X + 160, MENUS_Y + 1);
  tft.print(ConfigData.micGain, 1);
  while (true) {
    if (filterEncoderMove != 0) {
      ConfigData.micGain += ((float)filterEncoderMove);
      if (ConfigData.micGain < -20)
        ConfigData.micGain = -20;
      else if (ConfigData.micGain > 20)  // 100% max
        ConfigData.micGain = 20;
      tft.fillRect(SECONDARY_MENU_X + 160, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 160, MENUS_Y + 1);
      tft.print(ConfigData.micGain, 1);
      filterEncoderMove = 0;
    }
    /*
    val = ReadSelectedPushButton();
    menu = ProcessButtonPress(val);
    */
    menu = readButton(lastUsedTask);
    if (menu == MenuSelect::MENU_OPTION_SELECT) {
      updateMic();  // Update the Open Audio compressor and microphone gain.
      eeprom.ConfigDataWrite();
      break;
    }
  }
}



/*****
  Purpose: Allow user to set the mic Attack in sec

  Parameter list:
    void

  Return value;
    void
*****
void SetCompressionAttack()
{
  int val;

  tft.setFontScale( (enum RA8875tsize) 1);

  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X  - 48, MENUS_Y + 1);
  tft.print("Attack Sec:");
  tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
  tft.print(ConfigData.currentMicAttack, 1);

  while (true) {
    if (filterEncoderMove != 0) {
      ConfigData.currentMicAttack += ((float) filterEncoderMove * 0.1);
      if (ConfigData.currentMicAttack > 10)
        ConfigData.currentMicAttack = 10;
      else if (ConfigData.currentMicAttack < .1)                 // 100% max
        ConfigData.currentMicAttack = .1;

      tft.fillRect(SECONDARY_MENU_X + 180, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
      tft.print(ConfigData.currentMicAttack, 1);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();                                  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    delay(150L);

    if (val == MENU_OPTION_SELECT) {                             // Make a choice??
      //ConfigData.ConfigData.currentMicAttack = ConfigData.currentMicAttack;
      EEPROMWrite();

      break;
    }
  }
  EraseMenus();
}
*/

/*****
  Purpose: Allow user to set the mic compression ratio

  Parameter list:
    void

  Return value;
    void
*****
void SetCompressionRelease()
{
  int val;

  tft.setFontScale( (enum RA8875tsize) 1);

  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X  - 48, MENUS_Y + 1);
  tft.print("Decay Sec:");
  tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
  tft.print(ConfigData.currentMicRelease, 1);

  while (true) {
    if (filterEncoderMove != 0) {
      ConfigData.currentMicRelease += ((float) filterEncoderMove * 0.1);
      if (ConfigData.currentMicRelease > 10)
        ConfigData.currentMicRelease = 10;
      else if (ConfigData.currentMicRelease < 0.1)                 // 100% max
        ConfigData.currentMicRelease = 0.1;

      tft.fillRect(SECONDARY_MENU_X + 180, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
      tft.print(ConfigData.currentMicRelease, 1);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();                                  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    delay(150L);

    if (val == MENU_OPTION_SELECT) {                             // Make a choice??
      //ConfigData.ConfigData.micCompRatio = ConfigData.micCompRatio;
      EEPROMWrite();

      break;
    }
  }
  EraseMenus();
}
*/