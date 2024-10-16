
#include "SDT.h"

/*****
  Purpose: To save the configuration data (working variables) to EEPROM.
           Skip 4 bytes to allow for the struct size variable.

  Parameter list:
    struct EEPROMData       pointer to the EEPROM structure

  Return value;
    void
*****/
void Eeprom::EEPROMWrite() {
  EEPROM.put(EEPROM_BASE_ADDRESS + 4, EEPROMData);
}


/*****
  Purpose: This is nothing more than an alias for EEPROM.get(EEPROM_BASE_ADDRESS + 4, EEPROMData).

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::EEPROMRead() {
  EEPROM.get(EEPROM_BASE_ADDRESS + 4, EEPROMData);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::EEPROMWriteSize(int structSize) {
  EEPROM.put(EEPROM_BASE_ADDRESS, structSize);  // Read as one large chunk
}


/*****
  Purpose: Read the struct size stored in the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
int Eeprom::EEPROMReadSize() {
  int structSize;
  EEPROM.get(EEPROM_BASE_ADDRESS, structSize);  // Read as one large chunk
  return structSize;
}


/*****
  Purpose: Read default favorite frequencies

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
void Eeprom::EEPROMStuffFavorites(unsigned long current[]) {
  int i;
  for (i = 0; i < MAX_FAVORITES; i++) {
    current[i] = EEPROMData.favoriteFreqs[i];
  }
}


/*****
  Purpose: Used to save a favortie frequency to EEPROM

  Parameter list:

  Return value;
    void

  CAUTION: This code assumes you have set the curently active VFO frequency to the new
           frequency you wish to save. You them use the menu encoder to scroll through
           the current list of stored frequencies. Stop on the one that you wish to
           replace and press Select to save in EEPROM. The currently active VFO frequency
           is then stored to EEPROM.
*****/
void Eeprom::SetFavoriteFrequency() {
  int index;
  MenuSelect menu = MenuSelect::DEFAULT;
  tft.setFontScale((enum RA8875tsize)1);
  index = 0;
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
  tft.print(EEPROMData.favoriteFreqs[index]);
  while (true) {
    if (filterEncoderMove != 0) {  // Changed encoder?
      index += filterEncoderMove;  // Yep
      if (index < 0) {
        index = MAX_FAVORITES - 1;  // Wrap to last one
      } else {
        if (index > MAX_FAVORITES)
          index = 0;  // Wrap to first one
      }
      tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
      tft.print(EEPROMData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

//    val = ReadSelectedPushButton();  // Read pin that controls all switches
//    val = ProcessButtonPress(val);
menu = readButton();
    delay(150L);
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      EraseMenus();
      EEPROMData.favoriteFreqs[index] = TxRxFreq;
      //UpdateEEPROMSyncIndicator(0);       //  JJP 7/25/23
      if (EEPROMData.activeVFO == VFO_A) {
        EEPROMData.currentFreqA = TxRxFreq;
      } else {
        EEPROMData.currentFreqB = TxRxFreq;
      }
      //      EEPROMWrite();
      SetFreq();
      BandInformation();
      ShowBandwidth();
      FilterBandwidth();
      ShowFrequency();
      break;
    }
  }
}


/*****
  Purpose: Used to fetch a favortie frequency as stored in EEPROM. It then copies that
           frequency to the currently active VFO

  Parameter list:

  Return value;
    void
*****/
void Eeprom::GetFavoriteFrequency() {
  int index = 0;
//  int val;
MenuSelect menu = MenuSelect::DEFAULT;
  int currentBand2 = 0;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
  tft.print(EEPROMData.favoriteFreqs[index]);
  while (true) {
    if (filterEncoderMove != 0) {  // Changed encoder?
      index += filterEncoderMove;  // Yep
      if (index < 0) {
        index = MAX_FAVORITES - 1;  // Wrap to last one
      } else {
        if (index > MAX_FAVORITES)
          index = 0;  // Wrap to first one
      }
      tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
      tft.print(EEPROMData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

//    val = ReadSelectedPushButton();  // Read pin that controls all switches
//    val = ProcessButtonPress(val);
//    delay(150L);
    menu = readButton();

    if (EEPROMData.centerFreq >= bands[BAND_80M].fBandLow && EEPROMData.centerFreq <= bands[BAND_80M].fBandHigh) {
      currentBand2 = BAND_80M;
    } else if (EEPROMData.centerFreq >= bands[BAND_80M].fBandHigh && EEPROMData.centerFreq <= 7000000L) {  // covers 5MHz WWV AFP 11-03-22
      currentBand2 = BAND_80M;
    } else if (EEPROMData.centerFreq >= bands[BAND_40M].fBandLow && EEPROMData.centerFreq <= bands[BAND_40M].fBandHigh) {
      currentBand2 = BAND_40M;
    } else if (EEPROMData.centerFreq >= bands[BAND_40M].fBandHigh && EEPROMData.centerFreq <= 14000000L) {  // covers 10MHz WWV AFP 11-03-22
      currentBand2 = BAND_40M;
    } else if (EEPROMData.centerFreq >= bands[BAND_20M].fBandLow && EEPROMData.centerFreq <= bands[BAND_20M].fBandHigh) {
      currentBand2 = BAND_20M;
    } else if (EEPROMData.centerFreq >= 14000000L && EEPROMData.centerFreq <= 18000000L) {  // covers 15MHz WWV AFP 11-03-22
      currentBand2 = BAND_20M;
    } else if (EEPROMData.centerFreq >= bands[BAND_17M].fBandLow && EEPROMData.centerFreq <= bands[BAND_17M].fBandHigh) {
      currentBand2 = BAND_17M;
    } else if (EEPROMData.centerFreq >= bands[BAND_15M].fBandLow && EEPROMData.centerFreq <= bands[BAND_15M].fBandHigh) {
      currentBand2 = BAND_15M;
    } else if (EEPROMData.centerFreq >= bands[BAND_12M].fBandLow && EEPROMData.centerFreq <= bands[BAND_12M].fBandHigh) {
      currentBand2 = BAND_12M;
    } else if (EEPROMData.centerFreq >= bands[BAND_10M].fBandLow && EEPROMData.centerFreq <= bands[BAND_10M].fBandHigh) {
      currentBand2 = BAND_10M;
    }
    EEPROMData.currentBand = currentBand2;

    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      switch (EEPROMData.activeVFO) {
        case VFO_A:
          if (EEPROMData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
            EEPROMData.currentBandA = 0;                     // Yep. Roll to list front.
          }
          EEPROMData.currentBandA = currentBand2;
          TxRxFreq = EEPROMData.centerFreq + NCOFreq;
          EEPROMData.lastFrequencies[EEPROMData.currentBand][VFO_A] = TxRxFreq;
          break;

        case VFO_B:
          if (EEPROMData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
            EEPROMData.currentBandB = 0;                     // Yep. Roll to list front.
          }                                                  // Same for VFO B
          EEPROMData.currentBandB = currentBand2;
          TxRxFreq = EEPROMData.centerFreq + NCOFreq;
          EEPROMData.lastFrequencies[EEPROMData.currentBand][VFO_B] = TxRxFreq;
          break;
      }
    }
    if (menu == MenuSelect::MENU_OPTION_SELECT) {
      EraseSpectrumDisplayContainer();
//      currentMode = bands[EEPROMData.currentBand].mode;
      DrawSpectrumDisplayContainer();
      DrawFrequencyBarValue();
      SetBand();
      SetFreq();
      ShowFrequency();
      ShowSpectrumdBScale();
      EraseMenus();
      ResetTuning();
      FilterBandwidth();
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
      digitalWrite(bandswitchPins[EEPROMData.currentBand], LOW);
      SetFreq();
      ShowSpectrumdBScale();
      ShowSpectrum();
      //bands[currentBand].mode = currentBand;
      return;
    }
  }
}


/*****
  Purpose: To load into active memory the default settings for EEPROM variables.

  Parameter list:
    none

  Return value;
    void
*****/

void Eeprom::EEPROMDataDefaults() {
  struct config_t* defaultConfig = new config_t;  // Create a copy of the default configuration.
  EEPROMData = *defaultConfig;                    // Copy the defaults to EEPROMData struct.
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = EEPROMData.centerFreq = EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO];
  RedrawDisplayScreen();  //  Need to refresh display here.
}


/*****
  Purpose: Manage EEPROM memory at radio start-up.

  Parameter list:
    void

  Return value;
    void
*****/
void Eeprom::EEPROMStartup() {
  int eepromStructSize;
  int stackStructSize;
  //  Determine if the struct EEPROMData is compatible (same size) with the one stored in EEPROM.

  eepromStructSize = EEPROMReadSize();
  stackStructSize = sizeof(EEPROMData);

  // For minor revisions to the code, we don't want to overwrite the EEPROM.
  // We will assume the switch matrix and other items are calibrated by the user, and not to be lost.
  // However, if the EEPROMData struct changes, it is necessary to overwrite the EEPROM with the new struct.
  // This decision is made by using a simple size comparison.  This is not fool-proof, but it will probably
  // work most of the time.  The users should be instructed to always save the EEPROM to SD for later recovery
  // of their calibration and custom settings.
  // If all else fails, then the user should execute a FLASH erase.

  // The case where struct sizes are the same, indicating no changes to the struct.  Nothing more to do, return.
  if (eepromStructSize == stackStructSize) {
    EEPROMRead();  // Read the EEPROM data into active memory.
    return;        // Done, begin radio operation.
  }

  // If the flow proceeds here, it is time to initialize some things.
  // The rest of the code will require a switch matrix calibration, and will write the EEPROMData struct to EEPROM.

  SaveAnalogSwitchValues();          // Calibrate the switch matrix.
  EEPROMWriteSize(stackStructSize);  // Write the size of the struct to EEPROM.

  EEPROMWrite();  // Write the EEPROMData struct to non-volatile memory.
}
