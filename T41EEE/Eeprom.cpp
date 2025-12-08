#include <memory>
// EEPROM class


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
  Purpose: This is nothing more than an alias for EEPROM.get(EEPROM_BASE_ADDRESS + 4, ConfigData).
  This is a version with a different signature.  Used only once, in MenuProc.cpp.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::ConfigDataRead(config_t& configStruct) {
  EEPROM.get(EEPROM_BASE_ADDRESS + 4, configStruct);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::ConfigDataWriteSize(uint32_t structSize) {
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
  Purpose: This is nothing more than an alias for EEPROM.get(EEPROM_BASE_ADDRESS + 4, ConfigData).
  This is a version with a different signature.  Used only once, in MenuProc.cpp.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::CalDataRead(calibration_t& calStruct) {
  EEPROM.get(CAL_BASE_ADDRESS + 4, calStruct);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void Eeprom::CalDataWriteSize(uint32_t structSize) {
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
void Eeprom::BandsWriteSize(uint32_t structSize) {
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
int Eeprom::EEPROMReadSize(uint32_t address) {
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

    menu = readButton();
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      display.EraseMenus();
      ConfigData.favoriteFreqs[index] = TxRxFreq;
      if (ConfigData.activeVFO == VFO_A) {
        ConfigData.currentFreqA = TxRxFreq;
      } else {
        ConfigData.currentFreqB = TxRxFreq;
      }
      SetFreq();
      display.BandInformation();
      FilterBandwidth();
      display.ShowFrequency();
      break;
    }
  }
}


/*****
  Purpose: Used to fetch a favorite frequency as stored in EEPROM. It then copies that
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
      display.EraseSpectrumDisplayContainer();
      display.DrawSpectrumDisplayContainer();
      display.DrawFrequencyBarValue();
      SetFreq();
      display.ShowFrequency();
      display.ShowSpectrumdBScale();
      display.EraseMenus();
      ResetTuning();
      FilterBandwidth();
      display.BandInformation();
      NCOFreq = 0L;
      display.DrawBandWidthIndicatorBar();  // AFP 10-20-22
      SetFreq();
      display.ShowSpectrumdBScale();
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
//  struct config_t* defaultConfig = new config_t;  // Create a copy of the default configuration.
std::unique_ptr<config_t> defaultConfig = std::make_unique<config_t>();
  ConfigData = defaultConfig;                    // Copy the defaults to ConfigData struct.
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = ConfigData.centerFreq = ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO];
//  delete defaultConfig;
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
  CalData = *defaultCal;                                 // Copy the defaults to ConfigData struct.
  // Initialize the frequency setting based on the last used frequency stored to EEPROM.
  TxRxFreq = ConfigData.centerFreq = ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO];
}


/*****
  Purpose: Manage EEPROM memory at radio start-up.

  Parameter list:
    void

  Return value;
    void
*****/
void Eeprom::EEPROMStartup() {
  int ConfigDataEEPROMSize{ 0 };
  int ConfigDataStackSize{ 0 };
  int CalDataEEPROMSize{ 0 };
  int CalDataStackSize{ 0 };
  int BandsEEPROMSize{ 0 };
  int BandsStackSize{ 0 };
  std::unique_ptr<config_t> defaultConfig = std::make_unique<config_t>();

  //  Determine if the structs ConfigData, CalData, and bands are compatible (same size) with the one stored in EEPROM.

  ConfigDataEEPROMSize = EEPROMReadSize(EEPROM_BASE_ADDRESS);
  ConfigDataStackSize = sizeof(ConfigData);

  CalDataEEPROMSize = EEPROMReadSize(CAL_BASE_ADDRESS);
  CalDataStackSize = sizeof(CalData);

  BandsEEPROMSize = EEPROMReadSize(BANDS_BASE_ADDRESS);
  BandsStackSize = sizeof(bands);

  // For minor revisions to the code, we don't want to overwrite the EEPROM.
  // We will assume the switch matrix and other items are calibrated or configured by the user, and are not to be lost.
  // However, if the ConfigData, CalData or bands structs change, it is necessary to overwrite the EEPROM with the new struct(s).
  // This decision is made by using a simple size comparison.  This is not fool-proof, but it will probably
  // work most of the time.  The users should be instructed to always save the EEPROM (ConfigData and CalData) to SD for later recovery
  // of their calibration and configuration settings.  Note: the bands struct is saved along with ConfigData.
  // If all else fails, then the user should execute a FLASH erase.  The configuration and calibration can then be read from the SD card.

  // The case where struct sizes are the same, indicating no changes to the struct.  Nothing more to do, return.
  if (ConfigDataEEPROMSize == ConfigDataStackSize and CalDataEEPROMSize == CalDataStackSize and BandsEEPROMSize == BandsStackSize) {

    ConfigDataRead();  // Read the ConfigData into active memory.
    CalDataRead();     // Read the CalData into active memory.
    BandsRead();       // Read the bands array into active memory.

    // Handle the special case of overwrite when none of the structs have changed size.  This can happen when a user wants to
    // upgrade without executing a FLASH erase.  The only thing which will be updated in this case is the version.
    //    eeprom.ConfigDataRead(tempConfig);                                        // Read the default struct, which has the most recent version (a char array).
    if (strcmp(defaultConfig->versionSettings, ConfigData.versionSettings) == 0) {
      return;  // Versions the same, done.
    } else {
      for (int i = 0; i < 10; i = i + 1) {
        ConfigData.versionSettings[i] = defaultConfig->versionSettings[i];  // Set to the new version.
      }
      ConfigDataWrite();  // Write to EEPROM.
    }
    return;  // Done, begin radio operation.
  }

  // If the flow proceeds here, it is time to initialize some things.
  // The rest of the code will require a switch matrix calibration, and will write the ConfigData struct to EEPROM.

  SaveAnalogSwitchValues();                  // Calibrate the switch matrix.
  ConfigDataWriteSize(ConfigDataStackSize);  // Write the size of the struct to EEPROM.
  ConfigDataWrite();                         // Write the ConfigData struct to non-volatile memory.
  CalDataWriteSize(CalDataStackSize);        // Write the size of the struct to EEPROM.
  CalDataWrite();                            // Write the ConfigData struct to non-volatile memory.
  BandsWriteSize(BandsStackSize);            // Write the size of the bands array to EEPROM.
  BandsWrite();                              // Write the bands array to non-volatile memory.
}


/*****
  Purpose: Initialize the SD card

  Parameter list:
    void

  Return value;
    int                   0 if cannot initialize, 1 otherwise
*****/
int Eeprom::InitializeSDCard() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED, RA8875_BLACK);
  tft.setCursor(100, 240);
  if (!SD.begin(chipSelect)) {
    tft.print("SD card cannot be initialized.");
    delay(2000L);  // Given them time to read it.
    return 0;
  }

  return 1;
}