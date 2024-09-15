
//#include "SDT.h"
#include "EEPROMMemory.h"
#include "ArduinoJson.h"

/*****
  Purpose: To save the configuration data (working variables) to EEPROM.
           Skip 4 bytes to allow for the struct size variable.

  Parameter list:
    struct EEPROMData       pointer to the EEPROM structure

  Return value;
    void
*****/
void EEPROMMemory::EEPROMWrite() {
  EEPROM.put(EEPROM_BASE_ADDRESS + 4, EEPROMData);
}


/*****
  Purpose: This is nothing more than an alias for EEPROM.get(EEPROM_BASE_ADDRESS + 4, EEPROMData).

  Parameter list:
  None

  Return value;
    void
*****/
void EEPROMMemory::EEPROMRead() {
  EEPROM.get(EEPROM_BASE_ADDRESS + 4, EEPROMData);  // Read as one large chunk
}


/*****
  Purpose: Write the struct size stored to the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
void EEPROMMemory::EEPROMWriteSize(int structSize) {
  EEPROM.put(EEPROM_BASE_ADDRESS, structSize);  // Read as one large chunk
}


/*****
  Purpose: Read the struct size stored in the EEPROM.

  Parameter list:
  None

  Return value;
    void
*****/
int EEPROMMemory::EEPROMReadSize() {
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
void EEPROMMemory::EEPROMStuffFavorites(unsigned long current[]) {
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
void EEPROMMemory::SetFavoriteFrequency() {
  int index;
//  int val;
  MenuSelect menu;
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
    menu = readButton();
//    delay(150L);
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
void EEPROMMemory::GetFavoriteFrequency() {
  int index = 0;
//  int val;
  int currentBand2 = 0;
  MenuSelect menu;
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
    menu = readButton();
//    delay(150L);

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

void EEPROMMemory::EEPROMDataDefaults() {
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
void EEPROMMemory::EEPROMStartup() {
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


// JSON format used to save and read from SD card.  This was derived from a JSON example from the ArduinoJSON library.

// Custom converters are needed for the mode states.
bool EEPROMMemory::convertToJson(const RadioMode& src, JsonVariant dst) {
int state;
state = static_cast<int>(src);
//  char buf[32];
//  strftime(buf, sizeof(buf), "%FT%TZ", &src);

  return dst.set(state);
}


RadioMode EEPROMMemory::convertFromJson(JsonVariantConst src, RadioMode& dst) {
int state;
state = src.as<int>();
return dst = static_cast<RadioMode>(state);
}


// Loads the EEPROMData configuration from a file
void EEPROMMemory::loadConfiguration(const char *filename, config_t &EEPROMData) {
  // Open file for reading
  File file = SD.open(filename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  // StaticJsonDocument<512> doc;
  JsonDocument doc;  // This uses the heap.

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to read configuration file."));
    return;
  }

  // Copy values from the JsonDocument to the EEPROMData
  strlcpy(EEPROMData.versionSettings, doc["versionSettings"] | "t41pp.0", 10);
  EEPROMData.AGCMode = doc["AGCMode"];
  EEPROMData.audioVolume = doc["audioVolume"];
  EEPROMData.rfGainCurrent = doc["rfGainCurrent"];
  for (int i = 0; i < NUMBER_OF_BANDS; i++) EEPROMData.rfGain[i] = doc["rfGain"][i];
  EEPROMData.autoGain = doc["autoGain"];
  EEPROMData.autoSpectrum = doc["autoSpectrum"];
  EEPROMData.spectrumNoiseFloor = doc["spectrumNoiseFloor"];  // This is a constant.  This does not need to be included in user data.
  EEPROMData.centerTuneStep = doc["centerTuneStep"];
  EEPROMData.fineTuneStep = doc["fineTuneStep"];
  EEPROMData.transmitPowerLevel = doc["transmitPowerLevel"];
  EEPROMData.xmtMode = doc["xmtMode"];
  EEPROMData.nrOptionSelect = doc["nrOptionSelect"];
  EEPROMData.currentScale = doc["currentScale"];
  EEPROMData.spectrum_zoom = doc["spectrum_zoom"];
  EEPROMData.CWFilterIndex = doc["CWFilterIndex"];
  EEPROMData.paddleDit = doc["paddleDit"];
  EEPROMData.paddleDah = doc["paddleDah"];
  EEPROMData.decoderFlag = doc["decoderFlag"];
  EEPROMData.keyType = doc["keyType"];
  EEPROMData.currentWPM = doc["currentWPM"];
  EEPROMData.CWOffset = doc["CWOffset"];  
  EEPROMData.sidetoneVolume = doc["sidetoneVolume"];
  EEPROMData.cwTransmitDelay = doc["cwTransmitDelay"];
  EEPROMData.activeVFO = doc["activeVFO"];
  EEPROMData.currentBand = doc["currentBand"];
  EEPROMData.currentBandA = doc["currentBandA"];
  EEPROMData.currentBandB = doc["currentBandB"];
  EEPROMData.currentFreqA = doc["currentFreqA"];
  EEPROMData.currentFreqB = doc["currentFreqB"];
  EEPROMData.freqCorrectionFactor = doc["freqCorrectionFactor"];
  for (int i = 0; i < 14; i++) EEPROMData.equalizerRec[i] = doc["equalizerRec"][i];
  for (int i = 0; i < 14; i++) EEPROMData.equalizerXmt[i] = doc["equalizerXmt"][i];
  EEPROMData.equalizerXmt[0] = doc["equalizerXmt"][0];
  EEPROMData.currentMicThreshold = doc["currentMicThreshold"];
  EEPROMData.currentMicCompRatio = doc["currentMicCompRatio"];
  //EEPROMData.currentMicAttack = doc["currentMicAttack"];
  //EEPROMData.currentMicRelease = doc["currentMicRelease"];
  EEPROMData.currentMicGain = doc["currentMicGain"];
  for (int i = 0; i < 18; i++) EEPROMData.switchValues[i] = doc["switchValues"][i];
  EEPROMData.LPFcoeff = doc["LPFcoeff"];
  EEPROMData.NR_PSI = doc["NR_PSI"];
  EEPROMData.NR_alpha = doc["NR_alpha"];
  EEPROMData.NR_beta = doc["NR_beta"];
  EEPROMData.omegaN = doc["omegaN"];
  EEPROMData.pll_fmax = doc["pll_fmax"];
  EEPROMData.powerOutCW[0] = doc["powerOutCW"][0];
  EEPROMData.powerOutSSB[0] = doc["powerOutSSB"][0];
  for (int i = 0; i < 7; i++) EEPROMData.CWPowerCalibrationFactor[i] = doc["CWPowerCalibrationFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.SSBPowerCalibrationFactor[i] = doc["SSBPowerCalibrationFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQRXAmpCorrectionFactor[i] = doc["IQRXAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQRXPhaseCorrectionFactor[i] = doc["IQRXPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQCWAmpCorrectionFactor[i] = doc["IQCWAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQCWPhaseCorrectionFactor[i] = doc["IQCWPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQSSBAmpCorrectionFactor[i] = doc["IQSSBAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) EEPROMData.IQSSBPhaseCorrectionFactor[i] = doc["IQSSBPhaseCorrectionFactor"][i];

  for (int i = 0; i < 13; i++) EEPROMData.favoriteFreqs[i] = doc["favoriteFreqs"][i];
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 2; j++) EEPROMData.lastFrequencies[i][j] = doc["lastFrequencies"][i][j];
  }
  EEPROMData.centerFreq = doc["centerFreq"];
  //EEPROMData.mapFileName  = doc["mapFileName"] | "Boston";
  strlcpy(EEPROMData.mapFileName, doc["mapFileName"] | "Boston", 50);
  //EEPROMData.myCall  = doc["myCall"];
//  strlcpy(EEPROMData.myCall, doc["myCall"] | "Your Call", 10);
  //EEPROMData.myTimeZone  = doc["myTimeZone"];
  strlcpy(EEPROMData.myTimeZone, doc["myTimeZone"] | "EST", 10);
  EEPROMData.separationCharacter = doc["separationCharacter"];
  EEPROMData.paddleFlip = doc["paddleFlip"];
  EEPROMData.sdCardPresent = doc["sdCardPresent"];
  EEPROMData.myLong = doc["myLong"];
  EEPROMData.myLat = doc["myLat"];
  for (int i = 0; i < 7; i++) EEPROMData.currentNoiseFloor[i] = doc["currentNoiseFloor"][i];
  EEPROMData.compressorFlag = doc["compressorFlag"];
  EEPROMData.xmitEQFlag = doc["xmitEQFlag"];
  EEPROMData.receiveEQFlag = doc["receiveEQFlag"];
  EEPROMData.calFreq = doc["calFreq"];
  EEPROMData.buttonThresholdPressed = doc["buttonThresholdPressed"] | 944;
  EEPROMData.buttonThresholdReleased = doc["buttonThresholdReleased"] | 964;
  EEPROMData.buttonRepeatDelay = doc["buttonRepeatDelay"] | 300000;
  EEPROMData.autoGain = doc["autoGain"] | true;
  #ifdef QSE2
  for (int i = 0; i < 7; i++) EEPROMData.iDCoffsetCW[i] = doc["iDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) EEPROMData.qDCoffsetCW[i] = doc["qDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) EEPROMData.iDCoffsetSSB[i] = doc["iDCoffsetSSB"][i];
  for (int i = 0; i < 7; i++) EEPROMData.qDCoffsetSSB[i] = doc["qDCoffsetSSB"][i];  
  EEPROMData.dacOffsetCW = doc["dacOffsetCW"] | 0;
  EEPROMData.dacOffsetSSB = doc["dacOffsetSSB"] | 0; 
  #endif
  EEPROMData.CWradioCalComplete = doc["CWradioCalComplete"] | false;
  EEPROMData.SSBradioCalComplete = doc["SSBradioCalComplete"] | false;

  // How to copy strings:
  //  strlcpy(EEPROMData.myCall,                  // <- destination
  //          doc["myCall"],  // <- source
  //          sizeof(EEPROMData.myCall));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}


// Saves the configuration EEPROMData to a file or writes to serial.  toFile == true for file, false for serial.
void EEPROMMemory::saveConfiguration(const char *filename, const config_t &EEPROMData, bool toFile) {

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  //StaticJsonDocument<256> doc;  // This uses the stack.
  JsonDocument doc;  // This uses the heap.

  // Set the values in the document
  doc["versionSettings"] = EEPROMData.versionSettings;    // Fix for version not updating in JSON file.  KF5N March 18, 2024.
  doc["AGCMode"] = EEPROMData.AGCMode;
  doc["audioVolume"] = EEPROMData.audioVolume;
  doc["rfGainCurrent"] = EEPROMData.rfGainCurrent;
  for (int i = 0; i < NUMBER_OF_BANDS; i++) doc["rfGain"][i] = EEPROMData.rfGain[i];
  doc["autoGain"] = EEPROMData.autoGain;
  doc["autoSpectrum"] = EEPROMData.autoSpectrum;
  doc["spectrumNoiseFloor"] = EEPROMData.spectrumNoiseFloor;
  doc["centerTuneStep"] = EEPROMData.centerTuneStep;
  doc["fineTuneStep"] = EEPROMData.fineTuneStep;
  doc["transmitPowerLevel"] = EEPROMData.transmitPowerLevel;
  doc["xmtMode"] = EEPROMData.xmtMode;
  doc["nrOptionSelect"] = EEPROMData.nrOptionSelect;
  doc["currentScale"] = EEPROMData.currentScale;
  doc["spectrum_zoom"] = EEPROMData.spectrum_zoom;
  doc["CWFilterIndex"] = EEPROMData.CWFilterIndex;
  doc["paddleDit"] = EEPROMData.paddleDit;
  doc["paddleDah"] = EEPROMData.paddleDah;
  doc["decoderFlag"] = EEPROMData.decoderFlag;
  doc["keyType"] = EEPROMData.keyType;
  doc["currentWPM"] = EEPROMData.currentWPM;
  doc["CWOffset"] = EEPROMData.CWOffset;  
  doc["sidetoneVolume"] = EEPROMData.sidetoneVolume;
  doc["cwTransmitDelay"] = EEPROMData.cwTransmitDelay;
  doc["activeVFO"] = EEPROMData.activeVFO;
  doc["currentBand"] = EEPROMData.currentBand;
  doc["currentBandA"] = EEPROMData.currentBandA;
  doc["currentBandB"] = EEPROMData.currentBandB;
  doc["currentFreqA"] = EEPROMData.currentFreqA;
  doc["currentFreqB"] = EEPROMData.currentFreqB;
  doc["freqCorrectionFactor"] = EEPROMData.freqCorrectionFactor;
  for (int i = 0; i < 14; i++) doc["equalizerRec"][i] = EEPROMData.equalizerRec[i];
  for (int i = 0; i < 14; i++) doc["equalizerXmt"][i] = EEPROMData.equalizerXmt[i];
  doc["currentMicThreshold"] = EEPROMData.currentMicThreshold;
  doc["currentMicCompRatio"] = EEPROMData.currentMicCompRatio;
//  doc["currentMicAttack"] = EEPROMData.currentMicAttack;
//  doc["currentMicRelease"] = EEPROMData.currentMicRelease;
  doc["currentMicGain"] = EEPROMData.currentMicGain;
  for (int i = 0; i < 18; i++) doc["switchValues"][i] = EEPROMData.switchValues[i];
  doc["LPFcoeff"] = EEPROMData.LPFcoeff;
  doc["NR_PSI"] = EEPROMData.NR_PSI;
  doc["NR_alpha"] = EEPROMData.NR_alpha;
  doc["NR_beta"] = EEPROMData.NR_beta;
  doc["omegaN"] = EEPROMData.omegaN;
  doc["pll_fmax"] = EEPROMData.pll_fmax;
  for (int i = 0; i < 7; i++) doc["powerOutCW"][i] = EEPROMData.powerOutCW[i];
  for (int i = 0; i < 7; i++) doc["powerOutSSB"][i] = EEPROMData.powerOutSSB[i];
  for (int i = 0; i < 7; i++) {
    doc["CWPowerCalibrationFactor"][i] = EEPROMData.CWPowerCalibrationFactor[i];
  }
  for (int i = 0; i < 7; i++) doc["SSBPowerCalibrationFactor"][i] = EEPROMData.SSBPowerCalibrationFactor[i];
  for (int i = 0; i < 7; i++) doc["IQRXAmpCorrectionFactor"][i] =   EEPROMData.IQRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQRXPhaseCorrectionFactor"][i] = EEPROMData.IQRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactor"][i] =   EEPROMData.IQCWAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactor"][i] = EEPROMData.IQCWPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactor"][i] =   EEPROMData.IQSSBAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactor"][i] = EEPROMData.IQSSBPhaseCorrectionFactor[i];  
  for (int i = 0; i < 13; i++) doc["favoriteFreqs"][i] = EEPROMData.favoriteFreqs[i];
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 2; j++) doc["lastFrequencies"][i][j] = EEPROMData.lastFrequencies[i][j];
  }
  doc["centerFreq"] = EEPROMData.centerFreq;
  doc["mapFileName"] = EEPROMData.mapFileName;
//  doc["myCall"] = EEPROMData.myCall;
  doc["myTimeZone"] = EEPROMData.myTimeZone;
  doc["separationCharacter"] = EEPROMData.separationCharacter;
  doc["paddleFlip"] = EEPROMData.paddleFlip;
  doc["sdCardPresent"] = EEPROMData.sdCardPresent;
  doc["myLong"] = EEPROMData.myLong;
  doc["myLat"] = EEPROMData.myLat;
  for (int i = 0; i < 7; i++) doc["currentNoiseFloor"][i] = EEPROMData.currentNoiseFloor[i];
  doc["compressorFlag"] = EEPROMData.compressorFlag;
  doc["xmitEQFlag"] = EEPROMData.xmitEQFlag;
  doc["receiveEQFlag"] = EEPROMData.receiveEQFlag;
  doc["calFreq"] = EEPROMData.calFreq;
  doc["buttonThresholdPressed"] = EEPROMData.buttonThresholdPressed;
  doc["buttonThresholdReleased"] = EEPROMData.buttonThresholdReleased;
  doc["buttonRepeatDelay"] = EEPROMData.buttonRepeatDelay;
  doc["autoGain"] = EEPROMData.autoGain;
  #ifdef QSE2
  for (int i = 0; i < 7; i++) doc["iDCoffsetCW"][i] = EEPROMData.iDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetCW"][i] = EEPROMData.qDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["iDCoffsetSSB"][i] = EEPROMData.iDCoffsetSSB[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetSSB"][i] = EEPROMData.qDCoffsetSSB[i];
  doc["dacOffsetCW"] = EEPROMData.dacOffsetCW;
  doc["dacOffsetSSB"] = EEPROMData.dacOffsetSSB;
  #endif
  doc["CWradioCalComplete"] = EEPROMData.CWradioCalComplete;
  doc["SSBradioCalComplete"] = EEPROMData.SSBradioCalComplete;

  if (toFile) {
    // Delete existing file, otherwise EEPROMData is appended to the file
    SD.remove(filename);
    // Open file for writing
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }
    // Serialize JSON to file
    if (serializeJsonPretty(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    // Close the file
    file.close();
  } else {
    // Write to the serial port.

    serializeJsonPretty(doc, Serial);

  }
}


// Prints the content of a file to the Serial
FLASHMEM void EEPROMMemory::printFile(const char *filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

