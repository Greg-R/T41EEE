
#include "SDT.h"

/*****
  Purpose: To save the configuration data (working variables) to EEPROM.
           Skip 4 bytes to allow for the struct size variable.

  Parameter list:
    none

  Return value;
    void
*****/
void Eeprom::ConfigDataWrite() {
  EEPROM.put(EEPROM_BASE_ADDRESS + 4, ConfigData);
  Serial.printf("config write!\n");
}


/*****
  Purpose: This is nothing more than an alias for EEPROM.get(EEPROM_BASE_ADDRESS + 4, ConfigData).

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::ConfigDataRead() {
  EEPROM.get(EEPROM_BASE_ADDRESS + 4, ConfigData);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::ConfigDataWriteSize(int structSize) {
  EEPROM.put(EEPROM_BASE_ADDRESS, structSize);  // Read as one large chunk
}


/*****
  Purpose: To save the configuration data (working variables) to EEPROM.
           Skip 4 bytes to allow for the struct size variable.

  Parameter list:
   none

  Return value;
    void
*****/
void Eeprom::CalDataWrite() {
  EEPROM.put(CAL_BASE_ADDRESS + 4, CalData);
    Serial.printf("cal write!\n");
}


/*****
  Purpose: This is nothing more than an alias for EEPROM.get(CAL_BASE_ADDRESS + 4, ConfigData).

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::CalDataRead() {
  EEPROM.get(CAL_BASE_ADDRESS + 4, CalData);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::CalDataWriteSize(int structSize) {
  EEPROM.put(CAL_BASE_ADDRESS, structSize);  // Read as one large chunk
}


/*****
  Purpose: To save the bands array to EEPROM.
           Skip 4 bytes to allow for the struct size variable.

  Parameter list:
    none

  Return value;
    void
*****/
void Eeprom::BandsWrite() {
  EEPROM.put(BANDS_BASE_ADDRESS + 4, bands);
  Serial.printf("bands write!\n");
}


/*****
  Purpose: This is nothing more than an alias for EEPROM.get(BANDS_BASE_ADDRESS + 4, bands.bands[NUMBER_OF_BANDS]).

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::BandsRead() {
  EEPROM.get(BANDS_BASE_ADDRESS + 4, bands);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::BandsWriteSize(int structSize) {
  EEPROM.put(BANDS_BASE_ADDRESS, structSize);  // Read as one large chunk
}


/*****
  Purpose: Read the struct size stored in the EEPROM at the specified address.
           The ConfigData structure size is stored at 0.
  Parameter list:
  uint32_t address

  Return value;
    void
*****/
int Eeprom::EEPROMReadSize(uint32_t address ) {
  int structSize;
  EEPROM.get(address, structSize);  // Read as one large chunk
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
    current[i] = ConfigData.favoriteFreqs[i];
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
  tft.print(ConfigData.favoriteFreqs[index]);
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
      tft.print(ConfigData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

//    val = ReadSelectedPushButton();  // Read pin that controls all switches
//    val = ProcessButtonPress(val);
menu = readButton();
    delay(150L);
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      EraseMenus();
      ConfigData.favoriteFreqs[index] = TxRxFreq;
      //UpdateEEPROMSyncIndicator(0);       //  JJP 7/25/23
      if (ConfigData.activeVFO == VFO_A) {
        ConfigData.currentFreqA = TxRxFreq;
      } else {
        ConfigData.currentFreqB = TxRxFreq;
      }
      //      EEPROMWrite();
      SetFreq();
      BandInformation();
//      ShowBandwidth();
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
  tft.print(ConfigData.favoriteFreqs[index]);
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
      tft.print(ConfigData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

//    val = ReadSelectedPushButton();  // Read pin that controls all switches
//    val = ProcessButtonPress(val);
//    delay(150L);
    menu = readButton();

    if (ConfigData.centerFreq >= bands.bands[BAND_80M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_80M].fBandHigh) {
      currentBand2 = BAND_80M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_80M].fBandHigh && ConfigData.centerFreq <= 7000000L) {  // covers 5MHz WWV AFP 11-03-22
      currentBand2 = BAND_80M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_40M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_40M].fBandHigh) {
      currentBand2 = BAND_40M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_40M].fBandHigh && ConfigData.centerFreq <= 14000000L) {  // covers 10MHz WWV AFP 11-03-22
      currentBand2 = BAND_40M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_20M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_20M].fBandHigh) {
      currentBand2 = BAND_20M;
    } else if (ConfigData.centerFreq >= 14000000L && ConfigData.centerFreq <= 18000000L) {  // covers 15MHz WWV AFP 11-03-22
      currentBand2 = BAND_20M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_17M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_17M].fBandHigh) {
      currentBand2 = BAND_17M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_15M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_15M].fBandHigh) {
      currentBand2 = BAND_15M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_12M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_12M].fBandHigh) {
      currentBand2 = BAND_12M;
    } else if (ConfigData.centerFreq >= bands.bands[BAND_10M].fBandLow && ConfigData.centerFreq <= bands.bands[BAND_10M].fBandHigh) {
      currentBand2 = BAND_10M;
    }
    ConfigData.currentBand = currentBand2;

    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      switch (ConfigData.activeVFO) {
        case VFO_A:
          if (ConfigData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
            ConfigData.currentBandA = 0;                     // Yep. Roll to list front.
          }
          ConfigData.currentBandA = currentBand2;
          TxRxFreq = ConfigData.centerFreq + NCOFreq;
          ConfigData.lastFrequencies[ConfigData.currentBand][VFO_A] = TxRxFreq;
          break;

        case VFO_B:
          if (ConfigData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
            ConfigData.currentBandB = 0;                     // Yep. Roll to list front.
          }                                                  // Same for VFO B
          ConfigData.currentBandB = currentBand2;
          TxRxFreq = ConfigData.centerFreq + NCOFreq;
          ConfigData.lastFrequencies[ConfigData.currentBand][VFO_B] = TxRxFreq;
          break;
      }
    }
    if (menu == MenuSelect::MENU_OPTION_SELECT) {
      EraseSpectrumDisplayContainer();
//      currentMode = bands.bands[ConfigData.currentBand].mode;
      DrawSpectrumDisplayContainer();
      DrawFrequencyBarValue();
//      SetBandRelay();
      SetFreq();
      ShowFrequency();
      ShowSpectrumdBScale();
      EraseMenus();
      ResetTuning();
      FilterBandwidth();
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
//      digitalWrite(bandswitchPins[ConfigData.currentBand], LOW);
      SetFreq();
      ShowSpectrumdBScale();
      ShowSpectrum();
      //bands.bands[currentBand].mode = currentBand;
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

void Eeprom::ConfigDataDefaults() {
  struct config_t* defaultConfig = new config_t;  // Create a copy of the default configuration.
  ConfigData = *defaultConfig;                    // Copy the defaults to ConfigData struct.
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = ConfigData.centerFreq = ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO];
////  RedrawDisplayScreen();  //  Need to refresh display here.
}


/*****
  Purpose: To load into active memory the default settings for EEPROM variables.

  Parameter list:
    none

  Return value;
    void
*****/

void Eeprom::CalDataDefaults() {
  struct calibration_t* defaultCal = new calibration_t;  // Create a copy of the default configuration.
  CalData = *defaultCal;                    // Copy the defaults to ConfigData struct.
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = ConfigData.centerFreq = ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO];
////  RedrawDisplayScreen();  //  Need to refresh display here.
}


/*****
  Purpose: Manage EEPROM memory at radio start-up.

  Parameter list:
    void

  Return value;
    void
*****/
void Eeprom::EEPROMStartup() {
  int ConfigDataEEPROMSize;
  int ConfigDataStackSize;
  int CalDataEEPROMSize;
  int CalDataStackSize;
  int BandsEEPROMSize;
  int BandsStackSize;

  //  Determine if the struct ConfigData is compatible (same size) with the one stored in EEPROM.

  ConfigDataEEPROMSize = EEPROMReadSize(EEPROM_BASE_ADDRESS);
  ConfigDataStackSize = sizeof(ConfigData);

  CalDataEEPROMSize = EEPROMReadSize(CAL_BASE_ADDRESS);
  CalDataStackSize = sizeof(CalData);

  BandsEEPROMSize = EEPROMReadSize(BANDS_BASE_ADDRESS);
  BandsStackSize = sizeof(bands);
//  Serial.printf("BandsStackSize = %d\n", BandsStackSize);

  // For minor revisions to the code, we don't want to overwrite the EEPROM.
  // We will assume the switch matrix and other items are calibrated or configured by the user, and are not to be lost.
  // However, if the ConfigData or the CalData struct changes, it is necessary to overwrite the EEPROM with the new struct.
  // This decision is made by using a simple size comparison.  This is not fool-proof, but it will probably
  // work most of the time.  The users should be instructed to always save the EEPROM (ConfigData and CalData) to SD for later recovery
  // of their calibration and configuration settings.
  // If all else fails, then the user should execute a FLASH erase.  The configuration and calibration can then be read from the SD card.

  // The case where struct sizes are the same, indicating no changes to the struct.  Nothing more to do, return.
  if (ConfigDataEEPROMSize == ConfigDataStackSize and CalDataEEPROMSize == CalDataStackSize and BandsEEPROMSize == BandsStackSize) {
//    Serial.printf("Got to stack versus EEPROM comparison\n");
//    Serial.printf("ConfigDataEEPROMSize = %d ConfigDataStackSize = %d\n", ConfigDataEEPROMSize, ConfigDataStackSize);
//    Serial.printf("CalDataEEPROMSize = %d CalDataStackSize = %d\n", CalDataEEPROMSize, CalDataStackSize);
    ConfigDataRead();  // Read the ConfigData into active memory.
    CalDataRead();  // Read the CalData into active memory.
    BandsRead();    // Read the bands array into active memory.
    return;        // Done, begin radio operation.
  }

  // If the flow proceeds here, it is time to initialize some things.
  // The rest of the code will require a switch matrix calibration, and will write the ConfigData struct to EEPROM.

  SaveAnalogSwitchValues();          // Calibrate the switch matrix.
  ConfigDataWriteSize(ConfigDataStackSize);  // Write the size of the struct to EEPROM.
  ConfigDataWrite();  // Write the ConfigData struct to non-volatile memory.
  CalDataWriteSize(CalDataStackSize);  // Write the size of the struct to EEPROM.
  CalDataWrite();  // Write the ConfigData struct to non-volatile memory.
  BandsWriteSize(BandsStackSize);  // Write the size of the bands array to EEPROM.
  BandsWrite();  // Write the bands array to non-volatile memory.

//  Serial.printf("")
}
