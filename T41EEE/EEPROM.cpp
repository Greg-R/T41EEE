#ifndef BEENHERE
#include "SDT.h"
#endif

//DB2OO, 29-AUG-23: Don't use the overall VERSION for the EEPROM structure version information, but use a combination of an EEPROM_VERSION with the size of the EEPROMData variable.
// The "EEPROM_VERSION" should only be changed, if the structure config_t EEPROMData has changed!
// For V049.1 the new version in EEPROM will be "V049_808", for V049.2 it will be "V049_812"
#define EEPROM_VERSION  "T41EEE.0"
static char version_size[10];

/*****
  Purpose: void EEPROMSetVersion()

  Parameter list:

  Return value;
    char* pointer to EEPROM version string of the form "V049_808"
*****/
static char* EEPROMSetVersion(void)
{
    size_t  l;
  strncpy(version_size, EEPROM_VERSION, sizeof(version_size));
  l = strlen(version_size);
  //enough space to append '_' and 4 characters for size
  if ((sizeof(version_size)-l) > 5) {
    version_size[l] = '_';
    itoa(sizeof(EEPROMData), version_size+l+1, 10);
  }
  return version_size;
}

/*****
  Purpose: void EEPROMRead()

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
void EEPROMRead() {
  int i;
  int v049_version=0; //DB2OO, 10-SEP-23
#define MORSE_STRING_DISPLAY(s)  {size_t j; for (j=0;j<strlen(s);j++) MorseCharacterDisplay(s[j]);}  

  //DB2OO, 25-AUG-23: don't read the EEPROM before you are sure, that it is in T41-SDT format!!
  //DB2OO, 25-AUG-23: first read only the version string and compare it with the current version. The version string must also be at the beginning of the EEPROMData structure!
  for (i=0; i<10; i++) { EEPROMData.versionSettings[i]= EEPROM.read(EEPROM_BASE_ADDRESS+i); }
#ifdef DEBUG 
    //display version in EEPROM in last line of display
    MORSE_STRING_DISPLAY("EEPROMVersion ");
    if (strlen(versionSettings) <10) {
      MORSE_STRING_DISPLAY(versionSettings);
    }else {
      MORSE_STRING_DISPLAY("<<INVALID>>");
    }
    MyDelay(1000);
#endif
  //Do we have V049.1 or V049.2 structure in EEPROM?
  if (strcmp("V049.1", EEPROMData.versionSettings) == 0) v049_version=1;
  if (strcmp("V049.2", EEPROMData.versionSettings) == 0) v049_version=2;

  if (v049_version > 0) {
     //DB2OO, 29-AUG-23: allow "V049.1" or "V049.2" instead of the Version with size for a migration to the new format
     strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // set new version format
    for (i=0; i<10; i++) { EEPROM.write(EEPROM_BASE_ADDRESS+i, EEPROMData.versionSettings[i]); }
  }
  if (strncmp(EEPROMSetVersion(), EEPROMData.versionSettings, 10) != 0) {
    //Different version in EEPROM: set the EEPROM values for THIS version
#ifdef DEBUG  
    const char *wrong_ver = "EEPROMRead(): different version, calling EEPROMSaveDefaults2()";
    MORSE_STRING_DISPLAY(wrong_ver);
    Serial.println(wrong_ver);
    MyDelay(1000);
#endif    
  //  EEPROMSaveDefaults2();
    // and write it into the EEPROM
    EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);
    // after this we will read the default values for this version
  } else {
#ifdef DEBUG  
    MORSE_STRING_DISPLAY("-->Reading EEPROM content...");
    MyDelay(1000);
#endif
  }
#ifdef DEBUG
  //clear the Morse character buffer
  MorseCharacterClear();
#endif  
  EEPROM.get(EEPROM_BASE_ADDRESS, EEPROMData);  // Read as one large chunk
}


/*****
  Purpose: To save the configuration data (working variables) to EEPROM

  Parameter list:
    struct EEPROMData       pointer to the EEPROM structure

  Return value;
    void
*****/
void EEPROMWrite() {
  EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);
}


/*****
  Purpose: Read default favorite frequencies

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
void EEPROMStuffFavorites(unsigned long current[]) {
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
FLASHMEM void SetFavoriteFrequency() {
  int index;
  int val;
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

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      EraseMenus();
      EEPROMData.favoriteFreqs[index] = TxRxFreq;
      syncEEPROM = 0;  // SD EEPROM different that memory EEPROM
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
FLASHMEM void GetFavoriteFrequency() {
  int index = 0;
  int val;
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

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);

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

    if (val == MENU_OPTION_SELECT) {  // Make a choice??
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
          }                                       // Same for VFO B
          EEPROMData.currentBandB = currentBand2;
          TxRxFreq = EEPROMData.centerFreq + NCOFreq;
          EEPROMData.lastFrequencies[EEPROMData.currentBand][VFO_B] = TxRxFreq;
          break;
      }
    }
    if (val == MENU_OPTION_SELECT) {
      EraseSpectrumDisplayContainer();
      currentMode = bands[EEPROMData.currentBand].mode;
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
    struct defaultConfig       pointer to the default EEPROMData structure

  Return value;
    void
*****/

void EEPROMDataDefaults() {
struct config_t* defaultConfig = new config_t;  // Create a copy of the default configuration.
EEPROMData = *defaultConfig;             // Copy the defaults to EEPROMData struct.
// Initialize the frequency setting based on the last used frequency stored to EEPROM.
TxRxFreq = EEPROMData.centerFreq = EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO];
RedrawDisplayScreen();  //  Need to refresh display here.
}


/*****
  Purpose: Update the version number only in EEPROM

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateEEPROMVersionNumber() {
  strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // Copy the latest version to EEPROM
}


/*****
  Purpose: Writes EEPROMData defaults to the Serial monitor.

  Parameter list:
   const char *filename

  Return value;
    int               0 = SD is untouched, 1 = has data
*****/
void EEPROMDataToSerial(const char *filename) {

  Serial.println("Current EEPROMData follows:");

  Serial.println("End of current EEPROMData");
}


/*****
  Purpose: Clears the first 1K of emulated EEPROM to 0xff

  Parameter list:
    void

  Return value;
    void
*****/
void ClearEEPROM() {
  int i;
  for (i = 0; i < 1000; i++) {
    EEPROM.write(i, 0xFF);
  }
}



/*****
  Purpose: Read the EEPROM from: a) EEPROM memory, b) SD card memory, or c) defaults

  Parameter list:
    void

  Return value;
    void
*****/
void EEPROMStartup() {
  EEPROMRead();  // Read current stored data

  if (strcmp(EEPROMData.versionSettings, EEPROMSetVersion()) == 0) {  // Are the versions the same?
    return;                                                // Yep. Go home and don't mess with the EEPROM
  }
  strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // Nope, this is a new the version, so copy new version title to EEPROM
                                                //                                                                     Check if calibration has not been done and/or switch values are wonky, okay to use defaults
                                                //                                                                     If the Teensy is unused, these EEPROM values are 0xFF or perhaps cleared to 0.

//  if (EEPROMData.switchValues[9] < 440 || EEPROMData.switchValues[9] > 480) {
//    EEPROMSaveDefaults2();     // At least give them some starting values
//    EEPROMData.switchValues[9] = 0;  // This will force the next code block to set the switch values.
//  }
//  if (EEPROMData.switchValues[9] < 440 || EEPROMData.switchValues[9] > 480) {  // If the Teensy is unused, these EEPROM values are 0xFF or perhaps cleared to 0.
    SaveAnalogSwitchValues();                                      // In that case, we need to set the switch values.
//  }
  //                                                                     If we get here, the switch values have been set, either previously or by the call to
  //                                                                     SaveAnalogSwitchValues() as has the rest of the EEPROM data. This avoids recalibration.
//  EEPROM.put(0, EEPROMData);  // This rewrites the entire EEPROM struct as defined in SDT.h
//  EEPROMRead();               // Read the EEPROM data, including new switch values. This also resets working variables

#ifdef DEBUG1
  SDEEPROMDump();  // Call this to observe EEPROM struct data
#endif
}
