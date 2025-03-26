

// ShowMenu
// CalibrateOptions
// CWOptions
// Spectrum Options
// AGC Options
// Receive Equalizer Options
// SSB Options
// RF Options
// ConfigData Options
// CalData Options

#include "SDT.h"

int micChoice = 0;
int splitOn = 0;
int IQChoice = 0;


/*****
  Purpose: void ShowMenu()

  Parameter list:
    char *menuItem          pointers to the menu
    int where               0 is a primary menu, 1 is a secondary menu

  Return value;
    void
*****/
void ShowMenu(const char *menu[], int where) {
  tft.setFontScale((enum RA8875tsize)1);

  if (where == PRIMARY_MENU) {                                             // Should print on left edge of top line
    tft.fillRect(PRIMARY_MENU_X, MENUS_Y, 300, CHAR_HEIGHT, RA8875_BLUE);  // Top-left of display
    tft.setCursor(5, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);  // Primary Menu
  } else {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, 300, CHAR_HEIGHT, RA8875_GREEN);  // Right of primary display
    tft.setCursor(35, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);  // Secondary Menu
  }
  return;
}


// Updates by KF5N to CalibrateOptions() function.  Added SSB Carrier and SSB Transmit cal.  Greg KF5N July 10, 2024
// Updated receive calibration code to clean up graphics.  KF5N August 3, 2023
// ==============  AFP 10-22-22 ==================
/*****
  Purpose: Present the Calibrate options available and return the selection.
  This function is embedded in the mail receiver loop.  It gets called repeatedly during calibration.

  Parameter list:
    void

  Return value
   void
*****/
#ifdef QSE2
void CalibrateOptions() {
  int freqCorrectionFactorOld = 0;
  int32_t increment = 100L;
  MenuSelect menu;
  char freqCal[] = "Freq Cal: ";
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 30, CHAR_HEIGHT, RA8875_BLACK);

  // Select the type of calibration, and then skip this during the loop() function.
  // Note that some calibrate options run inside the loop() function!
  if (calibrateFlag == 0) {                                                                                                                                                                                                                                                                                                                  //    0             1           2               3                4              5             6              7                  8                  9               10                11               12                13               14               15              16          17           18
    const std::string IQOptions[]{ "Freq Cal", "CW PA Cal", "CW Rec Cal", "CW Carrier Cal", "CW Xmit Cal", "SSB PA Cal", "SSB Rec Cal", "SSB Carrier Cal", "SSB Transmit Cal", "CW Radio Cal", "CW Refine Cal", "SSB Radio Cal", "SSB Refine Cal", "dBm Level Cal", "DAC Offset CW", "DAC Offset SSB", "Btn Cal", "Btn Repeat", "Cancel" };  //AFP 10-21-22
    IQChoice = SubmenuSelect(IQOptions, 19, 0);                                                                                                                                                                                                                                                                                              //AFP 10-21-22
  }
  calibrateFlag = 1;
  switch (IQChoice) {

    case 0:  // Calibrate Frequency  - uses WWV
      CalData.freqCorrectionFactor = GetEncoderValueLive(-200000, 200000, CalData.freqCorrectionFactor, increment, freqCal, false);
      if (CalData.freqCorrectionFactor != freqCorrectionFactorOld) {
        si5351.set_correction(CalData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
        freqCorrectionFactorOld = CalData.freqCorrectionFactor;
      }
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X - 1, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT + 1, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 1:  // CW PA Cal
      CalData.CWPowerCalibrationFactor[ConfigData.currentBand] = GetEncoderValueLive(0.0, 1.0, CalData.CWPowerCalibrationFactor[ConfigData.currentBand], 0.01, (char *)"CW PA Cal: ", false);
      ConfigData.powerOutCW[ConfigData.currentBand] = sqrt(ConfigData.transmitPowerLevel / 20.0) * CalData.CWPowerCalibrationFactor[ConfigData.currentBand];
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.ConfigDataWrite();
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 2:                                                    // CW IQ Receive Cal - Gain and Phase
      cwcalibrater.DoReceiveCalibrate(0, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
      break;

    case 3:  // CW Xmit Carrier calibration.
      cwcalibrater.DoXmitCarrierCalibrate(0, false, false, true);
      break;

    case 4:                                                 // CW IQ Transmit Cal - Gain and Phase  //AFP 2-21-23
      cwcalibrater.DoXmitCalibrate(0, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
      break;

    case 5:  // SSB PA Cal
      CalData.SSBPowerCalibrationFactor[ConfigData.currentBand] = GetEncoderValueLive(0.0, 1.0, CalData.SSBPowerCalibrationFactor[ConfigData.currentBand], 0.01, (char *)"SSB PA Cal: ", false);
      ConfigData.powerOutSSB[ConfigData.currentBand] = sqrt(ConfigData.transmitPowerLevel / 20.0) * CalData.SSBPowerCalibrationFactor[ConfigData.currentBand];
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.CalDataWrite();
          eeprom.ConfigDataWrite();
          calibrateFlag = 0;
        }
      }
      break;  // Missing break.  KF5N August 12, 2023

    case 6:                                                    // SSB receive cal
      cwcalibrater.DoReceiveCalibrate(1, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
                                                               //      eeprom.CalDataWrite();                                   // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 7:  // SSB Carrier Cal
      ssbcalibrater.DoXmitCarrierCalibrate(false, false, true);
      //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 8:                                               // SSB Transmit cal
      ssbcalibrater.DoXmitCalibrate(false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
                                                          //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 9:  // CW fully automatic radio calibration.
      cwcalibrater.RadioCal(false);
      calibrateFlag = 0;
      //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 10:  // CW full automatic calibration refinement.
      cwcalibrater.RadioCal(true);
      calibrateFlag = 0;
      //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 11:  // SSB fully automatic radio calibration.
      ssbcalibrater.RadioCal(false);
      calibrateFlag = 0;
      break;

    case 12:  // SSB fully automatic calibration refinement.
      ssbcalibrater.RadioCal(true);
      calibrateFlag = 0;
      break;

    case 13:  // dBm level cal.  Was choose CW calibration tone frequency.
              //      calibrater.SelectCalFreq();
              //      calibrateFlag = 0;
      CalData.dBm_calibration = GetEncoderValueLive(0, 100, CalData.dBm_calibration, 1, (char *)"dBm Cal: ", false);
      if (CalData.dBm_calibration != freqCorrectionFactorOld) {
        //        si5351.set_correction(ConfigData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
        freqCorrectionFactorOld = CalData.dBm_calibration;
      }
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X - 1, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT + 1, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 14:  // Set DAC offset for CW carrier cancellation.
      CalData.dacOffsetCW = GetEncoderValueLiveQ15t(-5000, 5000, CalData.dacOffsetCW, 50, (char *)"DC Offset:", false);
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {
        if (menu == MenuSelect::MENU_OPTION_SELECT) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 15:  // Set DAC offset for SSB carrier cancellation.
      CalData.dacOffsetSSB = GetEncoderValueLiveQ15t(-5000, 5000, CalData.dacOffsetSSB, 50, (char *)"DC Offset:", false);
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {
        if (menu == MenuSelect::MENU_OPTION_SELECT) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      //      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 16:  // Calibrate buttons
      SaveAnalogSwitchValues();
      calibrateFlag = 0;
      RedrawDisplayScreen();
      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 17:  // Set button repeat rate
      CalData.buttonRepeatDelay = 1000 * GetEncoderValueLive(0, 5000, CalData.buttonRepeatDelay / 1000, 1, (char *)"Btn Repeat:  ", false);
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {
        if (menu == MenuSelect::MENU_OPTION_SELECT) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 18:  // Cancelled choice
//      RedrawDisplayScreen();
//      currentFreq = TxRxFreq = ConfigData.centerFreq + NCOFreq;
//      DrawBandWidthIndicatorBar();  // AFP 10-20-22
//      ShowFrequency();
//      BandInformation();
      calibrateFlag = 0;
      break;

    default:
      break;
  }
//  UpdateEqualizerField(ConfigData.receiveEQFlag, ConfigData.xmitEQFlag);
}
#else  // Not using QSE2 (No carrier calibration)
void CalibrateOptions() {
  int freqCorrectionFactorOld = 0;
  int32_t increment = 100L;
  MenuSelect menu;

  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 30, CHAR_HEIGHT, RA8875_BLACK);

  // Select the type of calibration, and then skip this during the loop() function.
  if (calibrateFlag == 0) {
    const std::string IQOptions[15]{ "Freq Cal", "CW PA Cal", "CW Rec Cal", "CW Xmit Cal", "SSB PA Cal", "SSB Rec Cal", "SSB Transmit Cal", "CW Radio Cal", "CW Refine Cal", "SSB Radio Cal", "SSB Refine Cal", "dBm Level Cal", "Btn Cal", "Btn Repeat", "Cancel" };  //AFP 10-21-22
    IQChoice = SubmenuSelect(IQOptions, 15, 0);                                                                                                                                                                                                                        //AFP 10-21-22
  }
  calibrateFlag = true;
  switch (IQChoice) {

    case 0:  // Calibrate Frequency  - uses WWV
      CalData.freqCorrectionFactor = GetEncoderValueLive(-200000, 200000, CalData.freqCorrectionFactor, increment, (char *)"Freq Cal: ", false);
      if (CalData.freqCorrectionFactor != freqCorrectionFactorOld) {
        si5351.set_correction(CalData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
        freqCorrectionFactorOld = CalData.freqCorrectionFactor;
      }
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X - 1, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT + 1, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = false;
        }
      }
      break;

    case 1:  // CW PA Cal
      CalData.CWPowerCalibrationFactor[ConfigData.currentBand] = GetEncoderValueLive(0.0, 1.0, CalData.CWPowerCalibrationFactor[ConfigData.currentBand], 0.01, (char *)"CW PA Cal: ", false);
      ConfigData.powerOutCW[ConfigData.currentBand] = sqrt(ConfigData.transmitPowerLevel / 20.0) * CalData.CWPowerCalibrationFactor[ConfigData.currentBand];
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.ConfigDataWrite();
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }

      break;

    case 2:                                                    // CW IQ Receive Cal - Gain and Phase
      cwcalibrater.DoReceiveCalibrate(0, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
                                                               //      eeprom.CalDataWrite();                             // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 3:                                                 // CW IQ Transmit Cal - Gain and Phase  //AFP 2-21-23
      cwcalibrater.DoXmitCalibrate(0, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
                                                            //      eeprom.CalDataWrite();                          // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 4:  // SSB PA Cal
      CalData.SSBPowerCalibrationFactor[ConfigData.currentBand] = GetEncoderValueLive(0.0, 1.0, CalData.SSBPowerCalibrationFactor[ConfigData.currentBand], 0.01, (char *)"SSB PA Cal: ", false);
      ConfigData.powerOutSSB[ConfigData.currentBand] = sqrt(ConfigData.transmitPowerLevel / 20.0) * CalData.SSBPowerCalibrationFactor[ConfigData.currentBand];
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.ConfigDataWrite();
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }

      break;  // Missing break.  KF5N August 12, 2023

    case 5:                                                    // SSB IQ Receive Cal - Gain and Phase
      cwcalibrater.DoReceiveCalibrate(1, false, false, true);  // This function was significantly revised.  KF5N August 16, 2023
                                                               //      eeprom.CalDataWrite();                             // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 6:
      ssbcalibrater.DoXmitCalibrate(false, false, true);  // SSB Transmit cal
                                                          //      eeprom.CalDataWrite();                        // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 7:  //  CW fully automatic radio calibration.
      cwcalibrater.RadioCal(false);
      calibrateFlag = 0;
      break;

    case 8:  // CW fully automatic calibration refinement.
      cwcalibrater.RadioCal(true);
      calibrateFlag = 0;
      break;

    case 9:  // SSB fully automatic radio calibration.
      ssbcalibrater.RadioCal(false);
      calibrateFlag = 0;
      break;

    case 10:  // SSB fully automatic calibration refinement.
      ssbcalibrater.RadioCal(true);
      calibrateFlag = 0;
      break;

    case 11:  // dBm level cal.  Was choose CW calibration tone frequency.
              //      calibrater.SelectCalFreq();
              //      calibrateFlag = 0;
      CalData.dBm_calibration = GetEncoderValueLive(0, 100, CalData.dBm_calibration, 1, (char *)"dBm Cal: ", false);
      if (CalData.dBm_calibration != freqCorrectionFactorOld) {
        //        si5351.set_correction(ConfigData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
        freqCorrectionFactorOld = CalData.dBm_calibration;
      }
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X - 1, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT + 1, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 12:  // Calibrate buttons
      SaveAnalogSwitchValues();
      calibrateFlag = 0;
      RedrawDisplayScreen();
//      ShowFrequency();
//      DrawFrequencyBarValue();
      eeprom.CalDataWrite();  // Save calibration numbers and configuration.  KF5N August 12, 2023
      break;

    case 13:  // Set button repeat rate
      CalData.buttonRepeatDelay = 1000 * GetEncoderValueLive(0, 5000, CalData.buttonRepeatDelay / 1000, 1, (char *)"Btn Repeat:  ", false);
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {
        if (menu == MenuSelect::MENU_OPTION_SELECT) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          eeprom.CalDataWrite();
          calibrateFlag = 0;
        }
      }
      break;

    case 14:  // Cancelled choice
      calibrateFlag = 0;
      break;

    default:
      break;
  }
}
#endif


// ==============  AFP 10-22-22 ==================
/*****
  Purpose: Present the CW options available to the user.  Change and store to ConfigData.

  Parameter list:
    void

  Return value
    void
*****/
void CWOptions()  // new option for Sidetone and Delay JJP 9/1/22
{
  // const char *cwChoices[]{ "Decode Sens", "CW Filter", "CW Offset", "WPM", "Sidetone Volume", "Key Type", "Paddle Flip", "Transmit Delay", "Cancel" };
  std::string cwChoices[]{ "Decode Sens", "CW Filter", "CW Offset", "WPM", "Sidetone Speaker", "Sidetone Headpho",
                           "Key Type", "Paddle Flip", "Transmit Delay", "Cancel" };
  int CWChoice = 0;
  uint32_t morseDecodeSensitivityOld = 0;
  //  uint32_t increment = 10;
  MenuSelect menu;

  if (morseDecodeAdjustFlag == false) {
    CWChoice = SubmenuSelectString(cwChoices, 10, 0);
    if (CWChoice == 0) morseDecodeAdjustFlag = true;  // Handle the special case of Morse decoder adjust; the loop must run.
  }

  switch (CWChoice) {

    case 0:  // Set Morse decoder sensitivity.
      ConfigData.morseDecodeSensitivity = GetEncoderValueLiveString(0, 10000, ConfigData.morseDecodeSensitivity, 100, cwChoices[CWChoice], false);
      if (ConfigData.morseDecodeSensitivity != morseDecodeSensitivityOld) morseDecodeSensitivityOld = ConfigData.morseDecodeSensitivity;
      menu = readButton();
      if (menu != MenuSelect::BOGUS_PIN_READ) {        // Any button press??
        if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X - 1, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT + 1, RA8875_BLACK);
          eeprom.ConfigDataWrite();
          morseDecodeAdjustFlag = false;
        }
      }
      break;

    case 1:              // CW Filter BW:      // AFP 10-18-22
      SelectCWFilter();  // in CWProcessing    // AFP 10-18-22
      break;             // AFP 10-18-22

    case 2:              // Select a preferred CW offset frequency.
      SelectCWOffset();  //  Located in CWProcessing.cpp
      break;

    case 3:  // WPM
      SetWPM();
      SetTransmitDitLength(ConfigData.currentWPM);  //Afp 09-22-22     // JJP 8/19/23
      break;

    case 4:  // Sidetone volume for speaker.
      SetSideToneVolume(true);
      break;

    case 5:  // Sidetone volume for headphone.
      SetSideToneVolume(false);
      break;

    case 6:          // Type of key:
      SetKeyType();  // Straight key or keyer? Stored in ConfigData.keyType.
      SetKeyPowerUp();
      UpdateWPMField();
      break;

    case 7:  // Flip paddles
      DoPaddleFlip();
      break;

    case 8:                // new function JJP 9/1/22
      SetTransmitDelay();  // Transmit relay hold delay
      break;

    default:  // Cancel
      break;
  }
}


/*****
  Purpose: Show the list of scales for the spectrum divisions

  Parameter list:
    void

  Return value
    int           an index into displayScale[] array, or -1 on cancel
*****/
void SpectrumOptions() { /*
  dispSc displayScale[] =  //r *dbText,dBScale, baseOffset
  {
    {"20 dB/", 10.0, 24},
    {"10 dB/", 20.0, 10},  // JJP 7/14/23
  };
  */
  const std::string spectrumChoices[] = { "20 dB/unit", "10 dB/unit", "Cancel" };
  int spectrumSet = ConfigData.currentScale;  // JJP 7/14/23

  spectrumSet = SubmenuSelect(spectrumChoices, 3, spectrumSet);
  if (strcmp(spectrumChoices[spectrumSet].c_str(), "Cancel") == 0) {
    return;
  }
  ConfigData.currentScale = spectrumSet;  // Yep...
  eeprom.ConfigDataWrite();
  //  RedrawDisplayScreen();
  ShowSpectrumdBScale();
  lastState = RadioState::NOSTATE;  // Force update of operating state.
}


/*****
  Purpose: Select AGC option.

  Parameter list:
    void

  Return value
    void
*****
void AGCOptions() {
  const char *AGCChoices[] = { "AGC Off", "AGC Long", "AGC Slow", "AGC Medium", "AGC Fast", "Cancel" };  // G0ORX (Added Long) September 5, 2023

  ConfigData.AGCMode = SubmenuSelect(AGCChoices, 6, ConfigData.AGCMode);  // G0ORX
  if (ConfigData.AGCMode == 5) {
    return;
  }

  AGCLoadValues();         // G0ORX September 5, 2023
  ConfigData.ConfigDataWrite();    // ...save it
  UpdateAGCField();
}
void AGCOptions() {
//  const char *AGCChoices[] = { "AGC Off", "AGC Long", "AGC Slow", "AGC Medium", "AGC Fast", "Cancel" };  // G0ORX (Added Long) September 5, 2023
  const char *AGCChoices[] = { "AGC Off", "AGC On", "Cancel" };  // AGC revised.  Greg KF5N February 26, 2025

//  ConfigData.AGCMode = SubmenuSelect(AGCChoices, 6, ConfigData.AGCMode);  // G0ORX
    ConfigData.AGCMode = SubmenuSelect(AGCChoices, 3, ConfigData.AGCMode);  // AGC revised.  Greg KF5N February 26, 2025

  if (ConfigData.AGCMode == 2) {
    return;
  }
SetAudioOperatingState(radioState);

  eeprom.ConfigDataWrite();    // ...save it
  UpdateAGCField();
}
*/


void AGCOptions() {
  const std::string AGCChoices[] = { "AGC On", "AGC Off", "AGC Threshold", "Cancel" };  // G0ORX (Added Long) September 5, 2023
  int agcSet = 0;

  agcSet = SubmenuSelect(AGCChoices, 4, ConfigData.AGCMode);  // G0ORX


  switch (agcSet) {
    case 0:  // AGC On
      ConfigData.AGCMode = true;
      SetAudioOperatingState(radioState);
      UpdateAGCField();
      break;

    case 1:  // AGC Off
      ConfigData.AGCMode = false;
      SetAudioOperatingState(radioState);
      UpdateAGCField();
      break;

    case 2:  // Set AGC threshold
             //      ConfigData.AGCThreshold = static_cast<float32_t>(GetEncoderValue(-60, -20, ConfigData.AGCThreshold, 1, "AGC Threshold"));
             //      ConfigData.AGCThreshold = GetEncoderValueLiveString(-60.0, -20.0, ConfigData.AGCThreshold, 1.0, "AGC Thr ", false);
      ConfigData.AGCThreshold = static_cast<float32_t>(GetEncoderValue(-60, -20, ConfigData.AGCThreshold, 1, "AGC Threshold "));

      initializeAudioPaths();
      break;

    case 4:  // Cancel
      return;
      break;

    default:
      break;
  }
  eeprom.ConfigDataWrite();
}


/*****
  Purpose: To process the graphics for the 14 chan equalizer option

  Parameter list:
    int array[]         The array to fill in.  0 is receive, 1 is transmit.
    char *title         The equalizer being set.
  Return value
    void
*****/
void ProcessEqualizerChoices(int EQType, char *title) {
  //  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
  //  }

  std::string rXeqFreq[14]{ " 200", " 250", " 315", " 400", " 500", " 630", " 800", "1000", "1250", "1600", "2000", "2500", "3150", "4000" };
  std::string tXeqFreq[14]{ "  50", "  71", " 100", " 141", " 200", " 283", " 400", " 566", " 800", "1131", "1600", "2263", "3200", "4526" };

  int yLevel[EQUALIZER_CELL_COUNT];  // EQUALIZER_CELL_COUNT 14

  int columnIndex;
  int iFreq;
  int newValue;
  int xOrigin = 50;
  int xOffset;
  int yOrigin = 50;
  int wide = 700;
  int high = 300;
  int barWidth = 46;
  int barTopY;
  int barBottomY;
  MenuSelect menu = MenuSelect::DEFAULT;

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    if (EQType == 0) {
      yLevel[iFreq] = ConfigData.equalizerRec[iFreq];
    } else {
      if (EQType == 1) {
        yLevel[iFreq] = (ConfigData.equalizerXmt[iFreq] * 10) + 100;
      }
    }
  }
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  tft.fillWindow(RA8875_BLACK);

  tft.fillRect(xOrigin - 50, yOrigin - 25, wide + 50, high + 50, RA8875_BLACK);  // Clear data area
  tft.setTextColor(RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setCursor(200, 0);
  tft.print(title);

  tft.drawRect(xOrigin - 4, yOrigin, wide + 4, high, RA8875_BLUE);
  tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED);  // Print center zero line center
  tft.setFontScale((enum RA8875tsize)0);

  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + tft.getFontHeight());
  tft.print("+12");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + (high / 2) - tft.getFontHeight());
  tft.print(" 0");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + high - tft.getFontHeight() * 2);
  tft.print("-12");

  barTopY = yOrigin + (high / 2);                // 50 + (300 / 2) = 200
  barBottomY = barTopY + DEFAULT_EQUALIZER_BAR;  // Default 200 + 100

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    tft.fillRect(xOrigin + (barWidth + 4) * iFreq, barTopY - (yLevel[iFreq] - DEFAULT_EQUALIZER_BAR), barWidth, yLevel[iFreq], RA8875_CYAN);
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq, yOrigin + high - tft.getFontHeight() * 2);
    if (EQType == 0) tft.print(rXeqFreq[iFreq].c_str()); else tft.print(tXeqFreq[iFreq].c_str());
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq + tft.getFontWidth() * 1.5, yOrigin + high + tft.getFontHeight() * 2);
if (EQType == 0) tft.print(yLevel[iFreq]);
if (EQType == 1) tft.print(ConfigData.equalizerXmt[iFreq]);
  }

  columnIndex = 0;  // Get ready to set values for columns
  newValue = 0;
  while (columnIndex < EQUALIZER_CELL_COUNT) {
    xOffset = xOrigin + (barWidth + 4) * columnIndex;  // Just do the math once
                                                       //    tft.fillRect(xOffset,                               // Indent to proper bar... Removed this rectangle.  Seems unnecessary.  KF5N November 12, 2023
                                                       //                 barBottomY - yLevel[columnIndex] - 1,  // Start at red line
                                                       //                 barBottomY - 1,
                                                       //                 barWidth,                              // Set bar width
                                                       //                 newValue + 1,                          // Erase old bar
                                                       //                 -100,
                                                       //                 RA8875_BLACK);

    tft.fillRect(xOffset,                           // Indent to proper bar...
                 barBottomY - yLevel[columnIndex],  // Start at red line
                 barWidth,                          // Set bar width
                 yLevel[columnIndex],               // Draw new bar
                 RA8875_MAGENTA);
    while (true) {
      newValue = yLevel[columnIndex];  // Get current value
      if (filterEncoderMove != 0) {

        tft.fillRect(xOffset,                    // Indent to proper bar...
                     barBottomY - newValue - 1,  // Start at red line
                     barWidth,                   // Set bar width
                     yLevel[columnIndex] + 1,               // Erase old bar
                     RA8875_BLACK);
//        newValue += (PIXELS_PER_EQUALIZER_DELTA * filterEncoderMove);  // Find new bar height. OK since filterEncoderMove equals 1 or -1. PIXELS_PER_EQUALIZER_DELTA = 10
          yLevel[columnIndex] += (PIXELS_PER_EQUALIZER_DELTA * filterEncoderMove);
        tft.fillRect(xOffset,                                          // Indent to proper bar...
                     barBottomY - yLevel[columnIndex],                            // Start at red line
                     barWidth,                                         // Set bar width
                     yLevel[columnIndex],                                         // Draw new bar
                     RA8875_MAGENTA);
//        yLevel[columnIndex] = newValue;

        tft.fillRect(xOffset + tft.getFontWidth() * 1.5 - 1, yOrigin + high + tft.getFontHeight() * 2,  // Update bottom number
                     barWidth, CHAR_HEIGHT, RA8875_BLACK);
        tft.setCursor(xOffset + tft.getFontWidth() * 1.5, yOrigin + high + tft.getFontHeight() * 2);
        if(EQType == 0) tft.print(yLevel[columnIndex]); else tft.print((yLevel[columnIndex] - 100) / 10);
        if (yLevel[columnIndex] < DEFAULT_EQUALIZER_BAR) {  // Repaint red center line if erased
          tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED);  // Clear hole in display center
        }
      }
      filterEncoderMove = 0;
      menu = readButton();  // Read the ladder value
      if (menu != MenuSelect::BOGUS_PIN_READ) {

        tft.fillRect(xOffset,                // Indent to proper bar...
                     barBottomY - newValue,  // Start at red line
                     barWidth,               // Set bar width
                     newValue,               // Draw new bar
                     RA8875_GREEN);

        if (EQType == 0) {
          ConfigData.equalizerRec[columnIndex] = newValue;
        } else {
          if (EQType == 1) {
            ConfigData.equalizerXmt[columnIndex] = (yLevel[columnIndex] - 100) / 10;
          }
        }

        filterEncoderMove = 0;
        columnIndex++;
        break;
      }  // end inner while
    }    // end outer while
    eeprom.ConfigDataWrite();
  }
  RedrawDisplayScreen();
  lastState = RadioState::NOSTATE;  // Force update of operating state.
}


/*****
  Purpose: Receive EQ set

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
void EqualizerRecOptions() {
  const std::string RecEQChoices[] = { "RX EQ On", "RX EQ Off", "RX EQSet", "Cancel" };  // Add code practice oscillator
  int EQChoice = 0;

  EQChoice = SubmenuSelect(RecEQChoices, 4, 0);

  switch (EQChoice) {
    case 0:
      ConfigData.receiveEQFlag = true;
      button.ExecuteModeChange();
      break;
    case 1:
      ConfigData.receiveEQFlag = false;
      button.ExecuteModeChange();
      break;
    case 2:
      ProcessEqualizerChoices(0, (char *)"Receive Equalizer");
      break;
    case 3:
      break;
  }
  eeprom.ConfigDataWrite();
//  RedrawDisplayScreen();
  UpdateEqualizerField(ConfigData.receiveEQFlag, ConfigData.xmitEQFlag);
}


/*****
  Purpose: Xmit EQ options

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
void EqualizerXmtOptions() {
  const std::string XmtEQChoices[] = { "TX EQ On", "TX EQ Off", "TX EQSet", "Cancel" };  // Add code practice oscillator
  int EQChoice = 0;

  EQChoice = SubmenuSelect(XmtEQChoices, 4, 0);

  switch (EQChoice) {
    case 0:
      ConfigData.xmitEQFlag = true;
      break;
    case 1:
      ConfigData.xmitEQFlag = false;
      break;
    case 2:
      ProcessEqualizerChoices(1, (char *)"Transmit Equalizer");
      break;
    case 3:  // Do nothing and exit.
      break;
  }
  eeprom.ConfigDataWrite();
//  RedrawDisplayScreen();
  UpdateEqualizerField(ConfigData.receiveEQFlag, ConfigData.xmitEQFlag);
}



/*****
  Purpose: Set options for the SSB exciter.

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
void SSBOptions()  // AFP 09-22-22 All new
{
  static int micChoice = 0;
  //  const char *micChoices[] = { "Mic Comp On", "Mic Comp Off", "Set Threshold", "Set Comp_Ratio", "Set Attack", "Set Decay", "Cancel" };
  const std::string micChoices[] = { "CESSB", "SSB", "FT8", "Comp On", "Comp Off", "Mic Gain", "Comp Threshold", "Comp Ratio", "Cancel" };

  micChoice = SubmenuSelect(micChoices, 9, micChoice);
  switch (micChoice) {

    case 0:  // CESSB on
      ConfigData.cessb = true;
      cessb1.setProcessing(ConfigData.cessb);
      Serial.printf("processing = %d", cessb1.getProcessing());
      //    ConfigData.ConfigDataWrite();
      BandInformation();
      break;

    case 1:  // SSB Data on
      ConfigData.cessb = false;
      cessb1.setProcessing(ConfigData.cessb);
      Serial.printf("processing = %d", cessb1.getProcessing());
      //    ConfigData.ConfigDataWrite();
      BandInformation();
      break;

    case 2:  // FT8
      ConfigData.cessb = false;
      cessb1.setProcessing(ConfigData.cessb);
      Serial.printf("processing = %d", cessb1.getProcessing());
      //    ConfigData.ConfigDataWrite();
      BandInformation();
      break;

    case 3:  // Compressor On
      ConfigData.compressorFlag = true;
      UpdateCompressionField();
      //    ConfigData.ConfigDataWrite();
      break;

    case 4:  // Compressor Off
      ConfigData.compressorFlag = false;
      UpdateCompressionField();
      //    ConfigData.ConfigDataWrite();
      break;

    case 5:  // Adjust mic gain in dB.  Default 0 db.
      MicGainSet();
      break;

    case 6:  // Set compression ratio.  Default -10 dB.
      SetCompressionThreshold();
      UpdateCompressionField();
      break;

    case 7:  // Set compressor threshold.  Default 100.0.
      SetCompressionRatio();
      UpdateCompressionField();
      break;

    case 8:  // Cancel
      return;
      break;

    default:
      return;
      break;
  }
  updateMic();
  eeprom.ConfigDataWrite();
}


/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value12
    int           an index into the band array
*****/
void RFOptions() {
  //  const char *rfOptions[] = { "TX Power Set", "RF Gain Set", "RF Auto-Gain On", "RF Auto-Gain Off", "Auto-Spectrum On", "AutoSpectrum Off", "Cancel" };
  const std::string rfOptions[] = { "TX Power Set", "RF Gain Set", "RF Auto-Gain On", "RF Auto-Gain Off", "Auto-Spectrum On", "AutoSpectrum Off", "Cancel" };
  int rfSet = 0;
  rfSet = SubmenuSelect(rfOptions, 7, rfSet);

  switch (rfSet) {
    case 0:  // TX Power Set.  AFP 10-21-22
      ConfigData.transmitPowerLevel = static_cast<float32_t>(GetEncoderValue(1, 20, ConfigData.transmitPowerLevel, 1, "Power: "));
      // When the transmit power level is set, this means ALL of the power coefficients must be revised!
      // powerOutCW and powerOutSSB must be updated.
      initPowerCoefficients();
      eeprom.ConfigDataWrite();  //AFP 10-21-22
      BandInformation();
      break;

    case 1:  // Manual gain set.
      ConfigData.rfGain[ConfigData.currentBand] = GetEncoderValue(-60, 20, ConfigData.rfGain[ConfigData.currentBand], 5, (char *)"RF Gain dB: ");
      eeprom.ConfigDataWrite();
      break;

    case 2:  // Auto-Gain On
      ConfigData.autoGain = true;
      ConfigData.autoSpectrum = false;  // Make sure Auto-Spectrum is off.
                                        //      fftOffset = 0;
      ShowAutoStatus();
      eeprom.ConfigDataWrite();
      break;

    case 3:  // Auto-Gain Off
      ConfigData.autoGain = false;
      ShowAutoStatus();
      eeprom.ConfigDataWrite();
      break;

    case 4:  // Auto-Spectrum On
      ConfigData.autoSpectrum = true;
      ConfigData.autoGain = false;  // Make sure Auto-Gain is off.
      ShowAutoStatus();
      eeprom.ConfigDataWrite();
      break;

    case 5:  // Auto-Spectrum Off
      ConfigData.autoSpectrum = false;
      //      fftOffset = 0;
      ShowAutoStatus();
      eeprom.ConfigDataWrite();
      break;

    default:  // Cancel
      break;
  }
}


/*****
  Purpose: This option reverses the dit and dah paddles on the keyer

  Parameter list:
    void

  Return value
    void
*****/
void DoPaddleFlip() {
  const char *paddleState[] = { "Right paddle = dah", "Right paddle = dit" };
  int choice, lastChoice;
  MenuSelect pushButtonSwitchIndex;
  //  int valPin;

  ConfigData.paddleDah = KEYER_DAH_INPUT_RING;  // Defaults
  ConfigData.paddleDit = KEYER_DIT_INPUT_TIP;
  choice = lastChoice = 0;

  tft.setTextColor(RA8875_BLACK);
  tft.fillRect(SECONDARY_MENU_X - 100, MENUS_Y, EACH_MENU_WIDTH + 100, CHAR_HEIGHT, RA8875_GREEN);
  tft.setCursor(SECONDARY_MENU_X - 93, MENUS_Y + 1);
  tft.print(paddleState[choice]);  // Show the default (right paddle = dah

  while (true) {
    delay(150L);
    //    valPin = ReadSelectedPushButton();                     // Poll buttons
    //    if (valPin != -1) {                                    // button was pushed
    pushButtonSwitchIndex = readButton();  // Winner, winner...chicken dinner!
    if (pushButtonSwitchIndex == MenuSelect::MAIN_MENU_UP || pushButtonSwitchIndex == MenuSelect::MAIN_MENU_DN) {
      choice = !choice;  // Reverse the last choice
      tft.fillRect(SECONDARY_MENU_X - 100, MENUS_Y, EACH_MENU_WIDTH + 100, CHAR_HEIGHT, RA8875_GREEN);
      tft.setCursor(SECONDARY_MENU_X - 93, MENUS_Y + 1);
      tft.print(paddleState[choice]);
    }
    if (pushButtonSwitchIndex == MenuSelect::MENU_OPTION_SELECT) {  // Made a choice??
      if (choice) {                                                 // Means right-paddle dit
        ConfigData.paddleDit = KEYER_DAH_INPUT_RING;
        ConfigData.paddleDah = KEYER_DIT_INPUT_TIP;
        ConfigData.paddleFlip = 1;  // KD0RC
      } else {
        ConfigData.paddleDit = KEYER_DIT_INPUT_TIP;
        ConfigData.paddleDah = KEYER_DAH_INPUT_RING;
        ConfigData.paddleFlip = 0;  // KD0RC
      }
      EraseMenus();
      UpdateWPMField();  // KD0RC
      break;
    }
  }
  eeprom.ConfigDataWrite();
}


/*****
  Purpose: Used to change the currently active VFO

  Parameter list:
    void

  Return value
    int             // the currently active VFO, A = 1, B = 0
*****/
void VFOSelect() {
  const std::string VFOOptions[] = { "VFO A", "VFO B", "VFO Split", "Cancel" };
  int toggle;
  int choice, lastChoice;

  choice = lastChoice = toggle = ConfigData.activeVFO;
  splitOn = 0;

  tft.setTextColor(RA8875_BLACK);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);
  tft.setCursor(SECONDARY_MENU_X + 7, MENUS_Y + 1);
  tft.print(VFOOptions[choice].c_str());  // Show the default (right paddle = dah

  choice = SubmenuSelect(VFOOptions, 4, 0);
  delay(10);
  NCOFreq = 0L;
  switch (choice) {
    case VFO_A:  // VFO A
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqA;
      ConfigData.activeVFO = VFO_A;
      ConfigData.currentBand = ConfigData.currentBandA;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);  // Erase split message
      splitOn = 0;
      break;

    case VFO_B:  // VFO B
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqB;
      ConfigData.activeVFO = VFO_B;
      ConfigData.currentBand = ConfigData.currentBandB;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);  // Erase split message
      splitOn = 0;
      break;

    case VFO_SPLIT:  // Split
      DoSplitVFO();
      splitOn = 1;
      break;

    default:  // Cancel
      break;
  }
  bands.bands[ConfigData.currentBand].freq = TxRxFreq;
//  SetBand();           // KF5N July 12, 2023
  SetBandRelay();  // Required when switching VFOs. KF5N July 12, 2023
  SetFreq();
//  RedrawDisplayScreen();
//  BandInformation();
//  ShowBandwidth();
  FilterBandwidth();
  tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);  // delete old digit
  tft.fillRect(FREQUENCY_X, FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);        // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
//  ShowFrequency();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) BandInformation();
//  DrawBandWidthIndicatorBar();
//  DrawFrequencyBarValue();
//  UpdateDecoderField();
}


/*****
  Purpose: Allow user to set current user configuration values or restore default settings.

  Parameter list:
    void

  Return value
    void
*****/
void ConfigDataOptions() {  //           0               1                2               3               4                  5                  6                  7                  8              9           10
  const std::string ConfigDataOpts[] = { "Save Current", "Load Defaults", "Get Favorite", "Set Favorite", "Copy Config->SD", "Copy SD->Config", "Config->Serial", "Default->Serial", "Stack->Serial", "SD->Serial", "Cancel" };
  int defaultOpt = 0;
  config_t tempConfig;     // A temporary config_t struct to copy ConfigData data into.
  config_t defaultConfig;  // The configuration defaults.
  defaultOpt = SubmenuSelect(ConfigDataOpts, 11, defaultOpt);
  switch (defaultOpt) {
    case 0:  // Save current ConfigData struct to ConfigData non-volatile memory.  Also save the bands struct at the same time!
      eeprom.ConfigDataWrite();
      eeprom.BandsWrite();
      break;

    case 1:
      eeprom.ConfigDataDefaults();  // Restore defaults to ConfigData struct and refresh display.
      break;

    case 2:
      eeprom.GetFavoriteFrequency();  // Get a stored frequency and store in active VFO
      break;

    case 3:
      eeprom.SetFavoriteFrequency();  // Set favorites
      break;

    case 4:                                                      // Copy ConfigData->SD.
      EEPROM.get(EEPROM_BASE_ADDRESS + 4, tempConfig);           // Read as one large chunk
      json.saveConfiguration(configFilename, tempConfig, true);  // Save ConfigData struct to SD
      break;

    case 5:                                                // Copy SD->ConfigData
      json.loadConfiguration(configFilename, ConfigData);  // Copy from SD to struct in active memory (on the stack) ConfigData.
      eeprom.ConfigDataWrite();                            // Write to ConfigData non-volatile memory.
      initUserDefinedStuff();                              // Various things must be initialized.  This is normally done in setup().  KF5N February 21, 2024
      tft.writeTo(L2);                                     // This is specifically to clear the bandwidth indicator bar.  KF5N August 7, 2023
      tft.clearMemory();
      tft.writeTo(L1);
      RedrawDisplayScreen();  // Assume there are lots of changes and do a heavy-duty refresh.  KF5N August 7, 2023
      break;

    case 6:  // ConfigData->Serial
      {
        Serial.println(F("\nBegin ConfigData from ConfigData"));
        // Don't want to overwrite the stack.  Need a temporary struct, read the ConfigData data into that.
        config_t ConfigData_temp;
        EEPROM.get(EEPROM_BASE_ADDRESS + 4, ConfigData_temp);
        json.saveConfiguration(configFilename, ConfigData_temp, false);  // Write the temporary struct to the serial monitor.
        Serial.println(F("\nEnd ConfigData from ConfigData\n"));
      }
      break;

    case 7:  // Defaults->Serial
      Serial.println(F("\nBegin ConfigData defaults"));
      json.saveConfiguration(configFilename, defaultConfig, false);  // Write default ConfigData struct to the Serial monitor.
      Serial.println(F("\nEnd ConfigData defaults\n"));
      break;

    case 8:  // Current->Serial
      Serial.println(F("Begin ConfigData on the stack"));
      json.saveConfiguration(configFilename, ConfigData, false);  // Write current ConfigData struct to the Serial monitor.
      Serial.println(F("\nEnd ConfigData on the stack\n"));
      break;

    case 9:  // SDConfigData->Serial
      Serial.println(F("Begin ConfigData on the SD card"));
      json.printFile(configFilename);  // Write SD card ConfigData struct to the Serial monitor.
      Serial.println(F("End ConfigData on the SD card\n"));
      break;

    default:
      defaultOpt = -1;  // No choice made
      break;
  }
  //  return defaultOpt;
}


/*****
  Purpose: Allow user to set restore calibration values or restore default settings.

  Parameter list:
    void

  Return value
    void
*****/
void CalDataOptions() {  //           0               1                2               3               4               5                  6               7          8
  const std::string CalDataOpts[] = { "Save Current", "Load Defaults", "Copy Cal->SD", "Copy SD->Cal", "Cal->Serial", "Default->Serial", "Stack->Serial", "SD->Serial", "Cancel" };
  int defaultOpt = 0;
  calibration_t tempCal;     // A temporary calibration_t struct to copy CalData data into.
  calibration_t defaultCal;  // The configuration defaults.
  defaultOpt = SubmenuSelect(CalDataOpts, 9, defaultOpt);
  switch (defaultOpt) {
    case 0:  // Save current CalData struct to ConfigData non-volatile memory.
      eeprom.CalDataWrite();
      break;

    case 1:
      eeprom.CalDataDefaults();  // Restore defaults to CalData struct and refresh display.
      break;

    case 2:                                              // Copy CalData->SD.
      EEPROM.get(CAL_BASE_ADDRESS + 4, tempCal);         // Read as one large chunk
      json.saveCalibration(calFilename, tempCal, true);  // Save ConfigData struct to SD
      break;

    case 3:                                        // Copy SD->CalData
      json.loadCalibration(calFilename, CalData);  // Copy from SD to struct in active memory (on the stack) ConfigData.
      eeprom.CalDataWrite();                       // Write to ConfigData non-volatile memory.
      initUserDefinedStuff();                      // Various things must be initialized.  This is normally done in setup().  KF5N February 21, 2024
      tft.writeTo(L2);                             // This is specifically to clear the bandwidth indicator bar.  KF5N August 7, 2023
      tft.clearMemory();
      tft.writeTo(L1);
      RedrawDisplayScreen();  // Assume there are lots of changes and do a heavy-duty refresh.  KF5N August 7, 2023
      break;

    case 4:  // CalData->Serial
      {
        Serial.println(F("\nBegin CalData from CalData"));
        // Don't want to overwrite the stack.  Need a temporary struct, read the CalData data into that.
        calibration_t CalData_temp;
        EEPROM.get(CAL_BASE_ADDRESS + 4, CalData_temp);
        json.saveCalibration(calFilename, CalData_temp, false);  // Write the temporary struct to the serial monitor.
        Serial.println(F("\nEnd CalData from CalData\n"));
      }
      break;

    case 5:  // Defaults->Serial
      Serial.println(F("\nBegin CalData defaults"));
      json.saveCalibration(calFilename, defaultCal, false);  // Write default CalData struct to the Serial monitor.
      Serial.println(F("\nEnd CalData defaults\n"));
      break;

    case 6:  // Current->Serial
      Serial.println(F("Begin CalData on the stack"));
      json.saveCalibration(calFilename, CalData, false);  // Write current CalData struct to the Serial monitor.
      Serial.println(F("\nEnd CalData on the stack\n"));
      break;

    case 7:  // SD CalData->Serial
      Serial.println(F("Begin CalData on the SD card"));
      json.printFile(calFilename);  // Write SD card CalData struct to the Serial monitor.
      Serial.println(F("End CalData on the SD card\n"));
      break;

    default:
      defaultOpt = -1;  // No choice made
      break;
  }
  //  return defaultOpt;
}


/*****
  Purpose: To select an option from a submenu

  Parameter list:
    char *options[]           submenus
    int numberOfChoices       choices available
    int defaultState          the starting option

  Return value
    int           an index into the band array
*****/
//int SubmenuSelect(const char *options[], int numberOfChoices, int defaultStart) {
int SubmenuSelect(const std::string options[], int numberOfChoices, int defaultStart) {
  int refreshFlag = 0;
  MenuSelect menu;
  int encoderReturnValue;

  tft.setTextColor(RA8875_BLACK);
  encoderReturnValue = defaultStart;  // Start the options using this option

  tft.setFontScale((enum RA8875tsize)1);
  if (refreshFlag == 0) {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
    tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
    tft.print(options[encoderReturnValue].c_str());  // Secondary Menu
    refreshFlag = 1;
  }
  //  delay(150L);

  while (true) {
    menu = readButton();                       // Read the ladder value
                                               //    delay(150L);
                                               //    if (val != -1 && val < (ConfigData.switchValues[0] + WIGGLE_ROOM)) {
                                               //      menu = ProcessButtonPress(val);  // Use ladder value to get menu choice
    if (menu != MenuSelect::BOGUS_PIN_READ) {  // Valid choice?
      switch (menu) {
        case MenuSelect::MENU_OPTION_SELECT:  // They made a choice
          tft.setTextColor(RA8875_WHITE);
          EraseMenus();
          return encoderReturnValue;
          break;

        case MenuSelect::MAIN_MENU_UP:
          encoderReturnValue++;
          if (encoderReturnValue >= numberOfChoices)
            encoderReturnValue = 0;
          break;

        case MenuSelect::MAIN_MENU_DN:
          encoderReturnValue--;
          if (encoderReturnValue < 0)
            encoderReturnValue = numberOfChoices - 1;
          break;

        default:
          encoderReturnValue = -1;  // An error selection
          break;
      }
      if (encoderReturnValue != -1) {
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
        tft.setTextColor(RA8875_BLACK);
        tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
        tft.print(options[encoderReturnValue].c_str());
        //          delay(50L);
        refreshFlag = 0;
      }
    }
  }
}


/*****
  Purpose: To select an option from a submenu

  Parameter list:
    string options[]           submenus
    int numberOfChoices       choices available
    int defaultState          the starting option

  Return value
    int           an index into the band array
*****/
int SubmenuSelectString(std::string options[], int numberOfChoices, int defaultStart) {
  int refreshFlag = 0;
  MenuSelect menu;
  int encoderReturnValue;

  tft.setTextColor(RA8875_BLACK);
  encoderReturnValue = defaultStart;  // Start the options using this option

  tft.setFontScale((enum RA8875tsize)1);
  if (refreshFlag == 0) {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
    tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
    tft.print(options[encoderReturnValue].c_str());  // Secondary Menu
    refreshFlag = 1;
  }
  //  delay(150L);

  while (true) {
    menu = readButton();                       // Read the ladder value
                                               //    delay(150L);
                                               //    if (val != -1 && val < (ConfigData.switchValues[0] + WIGGLE_ROOM)) {
                                               //      menu = ProcessButtonPress(val);  // Use ladder value to get menu choice
    if (menu != MenuSelect::BOGUS_PIN_READ) {  // Valid choice?
      switch (menu) {
        case MenuSelect::MENU_OPTION_SELECT:  // They made a choice
          tft.setTextColor(RA8875_WHITE);
          EraseMenus();
          return encoderReturnValue;
          break;

        case MenuSelect::MAIN_MENU_UP:
          encoderReturnValue++;
          if (encoderReturnValue >= numberOfChoices)
            encoderReturnValue = 0;
          break;

        case MenuSelect::MAIN_MENU_DN:
          encoderReturnValue--;
          if (encoderReturnValue < 0)
            encoderReturnValue = numberOfChoices - 1;
          break;

        default:
          encoderReturnValue = -1;  // An error selection
          break;
      }
      if (encoderReturnValue != -1) {
        tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
        tft.setTextColor(RA8875_BLACK);
        tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
        tft.print(options[encoderReturnValue].c_str());
        //          delay(50L);
        refreshFlag = 0;
      }
    }
  }
}
