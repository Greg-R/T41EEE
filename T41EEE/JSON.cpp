
#include "SDT.h"

// JSON format used to save and read from SD card.  This was derived from a JSON example from the ArduinoJSON library.

// Custom converters are needed for the mode states.
bool convertToJson(const RadioMode& src, JsonVariant dst) {
int state;
state = static_cast<int>(src);
//  char buf[32];
//  strftime(buf, sizeof(buf), "%FT%TZ", &src);
  return dst.set(state);
}

RadioMode convertFromJson(JsonVariantConst src, RadioMode& dst) {
int state;
state = src.as<int>();
return dst = static_cast<RadioMode>(state);
}

// Custom converters are needed for the audio states.
bool convertToJson(const AudioState& src, JsonVariant dst) {
int state;
state = static_cast<int>(src);
  return dst.set(state);
}

AudioState convertFromJson(JsonVariantConst src, AudioState& dst) {
int state;
state = src.as<int>();
return dst = static_cast<AudioState>(state);
}

// Custom converters are needed for the audio states.
bool convertToJson(const Sideband& src, JsonVariant dst) {
int state;
state = static_cast<int>(src);
  return dst.set(state);
}

Sideband convertFromJson(JsonVariantConst src, Sideband& dst) {
int state;
state = src.as<int>();
return dst = static_cast<Sideband>(state);
}


// Loads the ConfigData configuration from a file
FLASHMEM void JSON::loadConfiguration(const char *filename, config_t &ConfigData) {
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
    Serial.println(F("Failed to read calibration file."));
    return;
  }

  // Copy values from the JsonDocument to the ConfigData
  strlcpy(ConfigData.versionSettings, doc["versionSettings"] | "t41pp.0", 10);
  ConfigData.AGCMode = doc["AGCMode"];
  ConfigData.audioVolume = doc["audioVolume"];
  ConfigData.rfGainCurrent = doc["rfGainCurrent"];
  for (int i = 0; i < NUMBER_OF_BANDS; i++) ConfigData.rfGain[i] = doc["rfGain"][i];
  ConfigData.autoGain = doc["autoGain"];
  ConfigData.autoSpectrum = doc["autoSpectrum"];
//  ConfigData.spectrumNoiseFloor = doc["spectrumNoiseFloor"];  // This is a constant.  This does not need to be included in user data.
  ConfigData.centerTuneStep = doc["centerTuneStep"];
  ConfigData.fineTuneStep = doc["fineTuneStep"];
  ConfigData.transmitPowerLevel = doc["transmitPowerLevel"];
//  ConfigData.xmtMode = doc["xmtMode"];
  ConfigData.audioOut = doc["audioOut"];
  ConfigData.nrOptionSelect = doc["nrOptionSelect"];
  ConfigData.currentScale = doc["currentScale"];
  ConfigData.spectrum_zoom = doc["spectrum_zoom"];
  ConfigData.CWFilterIndex = doc["CWFilterIndex"];
  ConfigData.paddleDit = doc["paddleDit"];
  ConfigData.paddleDah = doc["paddleDah"];
  ConfigData.decoderFlag = doc["decoderFlag"];
  ConfigData.morseDecodeSensitivity = doc["morseDecodeSensitivity"];
  ConfigData.keyType = doc["keyType"];
  ConfigData.currentWPM = doc["currentWPM"];
  ConfigData.CWOffset = doc["CWOffset"];  
  ConfigData.sidetoneSpeaker = doc["sidetoneSpeaker"];
  ConfigData.sidetoneHeadphone = doc["sidetoneHeadphone"];
  ConfigData.cwTransmitDelay = doc["cwTransmitDelay"];
  ConfigData.activeVFO = doc["activeVFO"];
  ConfigData.currentBand = doc["currentBand"];
  ConfigData.currentBandA = doc["currentBandA"];
  ConfigData.currentBandB = doc["currentBandB"];
  ConfigData.currentFreqA = doc["currentFreqA"];
  ConfigData.currentFreqB = doc["currentFreqB"];
  for (int i = 0; i < 14; i++) ConfigData.equalizerRec[i] = doc["equalizerRec"][i];
  for (int i = 0; i < 14; i++) ConfigData.equalizerXmt[i] = doc["equalizerXmt"][i];
  ConfigData.equalizerXmt[0] = doc["equalizerXmt"][0];
  ConfigData.micThreshold = doc["micThreshold"];
  ConfigData.micCompRatio = doc["micCompRatio"];
  //ConfigData.currentMicAttack = doc["currentMicAttack"];
  //ConfigData.currentMicRelease = doc["currentMicRelease"];
  ConfigData.micGain = doc["micGain"];
  ConfigData.LPFcoeff = doc["LPFcoeff"];
  ConfigData.NR_PSI = doc["NR_PSI"];
  ConfigData.NR_alpha = doc["NR_alpha"];
  ConfigData.NR_beta = doc["NR_beta"];
  ConfigData.omegaN = doc["omegaN"];
  ConfigData.pll_fmax = doc["pll_fmax"];
  ConfigData.powerOutCW[0] = doc["powerOutCW"][0];
  ConfigData.powerOutSSB[0] = doc["powerOutSSB"][0];
  for (int i = 0; i < 13; i++) ConfigData.favoriteFreqs[i] = doc["favoriteFreqs"][i];
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 2; j++) ConfigData.lastFrequencies[i][j] = doc["lastFrequencies"][i][j];
  }
  for (int i = 0; i < 7; i++) ConfigData.lastSideband[i] = doc["lastSideband"][i];
  ConfigData.centerFreq = doc["centerFreq"];
  //ConfigData.mapFileName  = doc["mapFileName"] | "Boston";
  strlcpy(ConfigData.mapFileName, doc["mapFileName"] | "Boston", 50);
  //ConfigData.myCall  = doc["myCall"];
//  strlcpy(ConfigData.myCall, doc["myCall"] | "Your Call", 10);
  //ConfigData.myTimeZone  = doc["myTimeZone"];
  strlcpy(ConfigData.myTimeZone, doc["myTimeZone"] | "EST", 10);
  ConfigData.separationCharacter = doc["separationCharacter"];
  ConfigData.paddleFlip = doc["paddleFlip"];
  ConfigData.sdCardPresent = doc["sdCardPresent"];
  ConfigData.myLong = doc["myLong"];
  ConfigData.myLat = doc["myLat"];

  ConfigData.compressorFlag = doc["compressorFlag"];
  ConfigData.xmitEQFlag = doc["xmitEQFlag"];
  ConfigData.receiveEQFlag = doc["receiveEQFlag"];
  ConfigData.autoGain = doc["autoGain"] | true;

  // How to copy strings:
  //  strlcpy(ConfigData.myCall,                  // <- destination
  //          doc["myCall"],  // <- source
  //          sizeof(ConfigData.myCall));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}


// Saves the configuration ConfigData to a file or writes to serial.  toFile == true for file, false for serial.
FLASHMEM void JSON::saveConfiguration(const char *filename, const config_t &ConfigData, bool toFile) {

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  //StaticJsonDocument<256> doc;  // This uses the stack.
  JsonDocument doc;  // This uses the heap.

  // Set the values in the document
  doc["versionSettings"] = ConfigData.versionSettings;    // Fix for version not updating in JSON file.  KF5N March 18, 2024.
  doc["AGCMode"] = ConfigData.AGCMode;
  doc["audioVolume"] = ConfigData.audioVolume;
  doc["rfGainCurrent"] = ConfigData.rfGainCurrent;
  for (int i = 0; i < NUMBER_OF_BANDS; i++) doc["rfGain"][i] = ConfigData.rfGain[i];
  doc["autoGain"] = ConfigData.autoGain;
  doc["autoSpectrum"] = ConfigData.autoSpectrum;
//  doc["spectrumNoiseFloor"] = ConfigData.spectrumNoiseFloor;
  doc["centerTuneStep"] = ConfigData.centerTuneStep;
  doc["fineTuneStep"] = ConfigData.fineTuneStep;
  doc["transmitPowerLevel"] = ConfigData.transmitPowerLevel;
//  doc["xmtMode"] = ConfigData.xmtMode;
  doc["audioOut"] = ConfigData.audioOut;
  doc["nrOptionSelect"] = ConfigData.nrOptionSelect;
  doc["currentScale"] = ConfigData.currentScale;
  doc["spectrum_zoom"] = ConfigData.spectrum_zoom;
  doc["CWFilterIndex"] = ConfigData.CWFilterIndex;
  doc["paddleDit"] = ConfigData.paddleDit;
  doc["paddleDah"] = ConfigData.paddleDah;
  doc["decoderFlag"] = ConfigData.decoderFlag;
  doc["morseDecodeSensitivity"] = ConfigData.morseDecodeSensitivity;
  doc["keyType"] = ConfigData.keyType;
  doc["currentWPM"] = ConfigData.currentWPM;
  doc["CWOffset"] = ConfigData.CWOffset;  
  doc["sidetoneSpeaker"] = ConfigData.sidetoneSpeaker;
  doc["sidetoneHeadphone"] = ConfigData.sidetoneHeadphone;  
  doc["cwTransmitDelay"] = ConfigData.cwTransmitDelay;
  doc["activeVFO"] = ConfigData.activeVFO;
  doc["currentBand"] = ConfigData.currentBand;
  doc["currentBandA"] = ConfigData.currentBandA;
  doc["currentBandB"] = ConfigData.currentBandB;
  doc["currentFreqA"] = ConfigData.currentFreqA;
  doc["currentFreqB"] = ConfigData.currentFreqB;
//  doc["freqCorrectionFactor"] = ConfigData.freqCorrectionFactor;
  for (int i = 0; i < 14; i++) doc["equalizerRec"][i] = ConfigData.equalizerRec[i];
  for (int i = 0; i < 14; i++) doc["equalizerXmt"][i] = ConfigData.equalizerXmt[i];
  doc["micThreshold"] = ConfigData.micThreshold;
  doc["micCompRatio"] = ConfigData.micCompRatio;
//  doc["currentMicAttack"] = ConfigData.currentMicAttack;
//  doc["currentMicRelease"] = ConfigData.currentMicRelease;
  doc["micGain"] = ConfigData.micGain;
//  for (int i = 0; i < 18; i++) doc["switchValues"][i] = ConfigData.switchValues[i];
  doc["LPFcoeff"] = ConfigData.LPFcoeff;
  doc["NR_PSI"] = ConfigData.NR_PSI;
  doc["NR_alpha"] = ConfigData.NR_alpha;
  doc["NR_beta"] = ConfigData.NR_beta;
  doc["omegaN"] = ConfigData.omegaN;
  doc["pll_fmax"] = ConfigData.pll_fmax;
  for (int i = 0; i < 7; i++) doc["powerOutCW"][i] = ConfigData.powerOutCW[i];
  for (int i = 0; i < 7; i++) doc["powerOutSSB"][i] = ConfigData.powerOutSSB[i];
  /*
  for (int i = 0; i < 7; i++) {doc["CWPowerCalibrationFactor"][i] = ConfigData.CWPowerCalibrationFactor[i];}
  for (int i = 0; i < 7; i++) doc["SSBPowerCalibrationFactor"][i] = ConfigData.SSBPowerCalibrationFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXAmpCorrectionFactor"][i] =   ConfigData.IQCWRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXPhaseCorrectionFactor"][i] = ConfigData.IQCWRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactor"][i] =   ConfigData.IQCWAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactor"][i] = ConfigData.IQCWPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXAmpCorrectionFactor"][i] =   ConfigData.IQSSBRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXPhaseCorrectionFactor"][i] = ConfigData.IQSSBRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactor"][i] =   ConfigData.IQSSBAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactor"][i] = ConfigData.IQSSBPhaseCorrectionFactor[i];
  */
  for (int i = 0; i < 13; i++) doc["favoriteFreqs"][i] = ConfigData.favoriteFreqs[i];
  for (int i = 0; i < 7; i++) {
  for (int j = 0; j < 2; j++) doc["lastFrequencies"][i][j] = ConfigData.lastFrequencies[i][j];
  }
  for (int i = 0; i < 7; i++) doc["lastSideband"][i] = ConfigData.lastSideband[i];
  doc["centerFreq"] = ConfigData.centerFreq;
  doc["mapFileName"] = ConfigData.mapFileName;
//  doc["myCall"] = ConfigData.myCall;
  doc["myTimeZone"] = ConfigData.myTimeZone;
  doc["separationCharacter"] = ConfigData.separationCharacter;
  doc["paddleFlip"] = ConfigData.paddleFlip;
  doc["sdCardPresent"] = ConfigData.sdCardPresent;
  doc["myLong"] = ConfigData.myLong;
  doc["myLat"] = ConfigData.myLat;
  
  doc["compressorFlag"] = ConfigData.compressorFlag;
  doc["xmitEQFlag"] = ConfigData.xmitEQFlag;
  doc["receiveEQFlag"] = ConfigData.receiveEQFlag;
//  doc["calFreq"] = ConfigData.calFreq;
//  doc["buttonThresholdPressed"] = ConfigData.buttonThresholdPressed;
//  doc["buttonThresholdReleased"] = ConfigData.buttonThresholdReleased;
//  doc["buttonRepeatDelay"] = ConfigData.buttonRepeatDelay;
  doc["autoGain"] = ConfigData.autoGain;
  /*
  #ifdef QSE2
  for (int i = 0; i < 7; i++) doc["iDCoffsetCW"][i] = ConfigData.iDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetCW"][i] = ConfigData.qDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["iDCoffsetSSB"][i] = ConfigData.iDCoffsetSSB[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetSSB"][i] = ConfigData.qDCoffsetSSB[i];
  doc["dacOffsetCW"] = ConfigData.dacOffsetCW;
  doc["dacOffsetSSB"] = ConfigData.dacOffsetSSB;
  #endif
  doc["CWradioCalComplete"] = ConfigData.CWradioCalComplete;
  doc["SSBradioCalComplete"] = ConfigData.SSBradioCalComplete;
*/
  if (toFile) {
    // Delete existing file, otherwise ConfigData is appended to the file
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


// Loads the ConfigData configuration from a file
FLASHMEM void JSON::loadCalibration(const char *filename, calibration_t &CalData) {
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
    Serial.println(F("Failed to read calibration file."));
    return;
  }

  // Copy values from the JsonDocument to the CalData

  CalData.freqCorrectionFactor = doc["freqCorrectionFactor"];

//  ConfigData.powerOutCW[0] = doc["powerOutCW"][0];
//  ConfigData.powerOutSSB[0] = doc["powerOutSSB"][0];
  for (int i = 0; i < 7; i++) CalData.CWPowerCalibrationFactor[i] = doc["CWPowerCalibrationFactor"][i];
  for (int i = 0; i < 7; i++) CalData.SSBPowerCalibrationFactor[i] = doc["SSBPowerCalibrationFactor"][i];
/*
  for (int i = 0; i < 7; i++) CalData.IQCWRXAmpCorrectionFactor[i] = doc["IQCWRXAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWRXPhaseCorrectionFactor[i] = doc["IQCWRXPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWAmpCorrectionFactor[i] = doc["IQCWAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWPhaseCorrectionFactor[i] = doc["IQCWPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXAmpCorrectionFactor[i] = doc["IQSSBRXAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXPhaseCorrectionFactor[i] = doc["IQSSBRXPhaseCorrectionFactor"][i];  
  for (int i = 0; i < 7; i++) CalData.IQSSBAmpCorrectionFactor[i] = doc["IQSSBAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBPhaseCorrectionFactor[i] = doc["IQSSBPhaseCorrectionFactor"][i];
*/
  for (int i = 0; i < 7; i++) CalData.IQCWRXAmpCorrectionFactorLSB[i] = doc["IQCWRXAmpCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWRXPhaseCorrectionFactorLSB[i] = doc["IQCWRXPhaseCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWAmpCorrectionFactorLSB[i] = doc["IQCWAmpCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWPhaseCorrectionFactorLSB[i] = doc["IQCWPhaseCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXAmpCorrectionFactorLSB[i] = doc["IQSSBRXAmpCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXPhaseCorrectionFactorLSB[i] = doc["IQSSBRXPhaseCorrectionFactorLSB"][i];  
  for (int i = 0; i < 7; i++) CalData.IQSSBAmpCorrectionFactorLSB[i] = doc["IQSSBAmpCorrectionFactorLSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBPhaseCorrectionFactorLSB[i] = doc["IQSSBPhaseCorrectionFactorLSB"][i];

  for (int i = 0; i < 7; i++) CalData.IQCWRXAmpCorrectionFactorUSB[i] = doc["IQCWRXAmpCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWRXPhaseCorrectionFactorUSB[i] = doc["IQCWRXPhaseCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWAmpCorrectionFactorUSB[i] = doc["IQCWAmpCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQCWPhaseCorrectionFactorUSB[i] = doc["IQCWPhaseCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXAmpCorrectionFactorUSB[i] = doc["IQSSBRXAmpCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBRXPhaseCorrectionFactorUSB[i] = doc["IQSSBRXPhaseCorrectionFactorUSB"][i];  
  for (int i = 0; i < 7; i++) CalData.IQSSBAmpCorrectionFactorUSB[i] = doc["IQSSBAmpCorrectionFactorUSB"][i];
  for (int i = 0; i < 7; i++) CalData.IQSSBPhaseCorrectionFactorUSB[i] = doc["IQSSBPhaseCorrectionFactorUSB"][i];

  CalData.buttonThresholdPressed = doc["buttonThresholdPressed"] | 944;
  CalData.buttonThresholdReleased = doc["buttonThresholdReleased"] | 964;
  CalData.buttonRepeatDelay = doc["buttonRepeatDelay"] | 300000;
  for (int i = 0; i < 18; i++) CalData.switchValues[i] = doc["switchValues"][i];

  #ifdef QSE2
  for (int i = 0; i < 7; i++) CalData.iDCoffsetCW[i] = doc["iDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) CalData.qDCoffsetCW[i] = doc["qDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) CalData.iDCoffsetSSB[i] = doc["iDCoffsetSSB"][i];
  for (int i = 0; i < 7; i++) CalData.qDCoffsetSSB[i] = doc["qDCoffsetSSB"][i];  
  CalData.dacOffsetCW = doc["dacOffsetCW"] | 0;
  CalData.dacOffsetSSB = doc["dacOffsetSSB"] | 0; 
  #endif
  CalData.CWradioCalComplete = doc["CWradioCalComplete"] | false;
  CalData.SSBradioCalComplete = doc["SSBradioCalComplete"] | false;
  CalData.dBm_calibration = doc["dBm_calibration"];

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}


// Saves the configuration CalData to a file or writes to serial.  toFile == true for file, false for serial.
FLASHMEM void JSON::saveCalibration(const char *filename, const calibration_t &CalData, bool toFile) {

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  //StaticJsonDocument<256> doc;  // This uses the stack.
  JsonDocument doc;  // This uses the heap.

  // Set the values in the document

  doc["freqCorrectionFactor"] = CalData.freqCorrectionFactor;

//  for (int i = 0; i < 7; i++) doc["powerOutCW"][i] = ConfigData.powerOutCW[i];
//  for (int i = 0; i < 7; i++) doc["powerOutSSB"][i] = ConfigData.powerOutSSB[i];
  for (int i = 0; i < 7; i++) doc["CWPowerCalibrationFactor"][i] = CalData.CWPowerCalibrationFactor[i];
  for (int i = 0; i < 7; i++) doc["SSBPowerCalibrationFactor"][i] = CalData.SSBPowerCalibrationFactor[i];
/*
  for (int i = 0; i < 7; i++) doc["IQCWRXAmpCorrectionFactor"][i] =   CalData.IQCWRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXPhaseCorrectionFactor"][i] = CalData.IQCWRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactor"][i] =   CalData.IQCWAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactor"][i] = CalData.IQCWPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXAmpCorrectionFactor"][i] =   CalData.IQSSBRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXPhaseCorrectionFactor"][i] = CalData.IQSSBRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactor"][i] =   CalData.IQSSBAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactor"][i] = CalData.IQSSBPhaseCorrectionFactor[i];  
*/
  for (int i = 0; i < 7; i++) doc["IQCWRXAmpCorrectionFactorLSB"][i] =   CalData.IQCWRXAmpCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXPhaseCorrectionFactorLSB"][i] = CalData.IQCWRXPhaseCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactorLSB"][i] =   CalData.IQCWAmpCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactorLSB"][i] = CalData.IQCWPhaseCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXAmpCorrectionFactorLSB"][i] =   CalData.IQSSBRXAmpCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXPhaseCorrectionFactorLSB"][i] = CalData.IQSSBRXPhaseCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactorLSB"][i] =   CalData.IQSSBAmpCorrectionFactorLSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactorLSB"][i] = CalData.IQSSBPhaseCorrectionFactorLSB[i];

  for (int i = 0; i < 7; i++) doc["IQCWRXAmpCorrectionFactorUSB"][i] =   CalData.IQCWRXAmpCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXPhaseCorrectionFactorUSB"][i] = CalData.IQCWRXPhaseCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactorUSB"][i] =   CalData.IQCWAmpCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactorUSB"][i] = CalData.IQCWPhaseCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXAmpCorrectionFactorUSB"][i] =   CalData.IQSSBRXAmpCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXPhaseCorrectionFactorUSB"][i] = CalData.IQSSBRXPhaseCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactorUSB"][i] =   CalData.IQSSBAmpCorrectionFactorUSB[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactorUSB"][i] = CalData.IQSSBPhaseCorrectionFactorUSB[i];

  doc["buttonThresholdPressed"] = CalData.buttonThresholdPressed;
  doc["buttonThresholdReleased"] = CalData.buttonThresholdReleased;
  doc["buttonRepeatDelay"] = CalData.buttonRepeatDelay;
  for (int i = 0; i < 18; i++) doc["switchValues"][i] = CalData.switchValues[i];

  #ifdef QSE2
  for (int i = 0; i < 7; i++) doc["iDCoffsetCW"][i] = CalData.iDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetCW"][i] = CalData.qDCoffsetCW[i];
  for (int i = 0; i < 7; i++) doc["iDCoffsetSSB"][i] = CalData.iDCoffsetSSB[i];
  for (int i = 0; i < 7; i++) doc["qDCoffsetSSB"][i] = CalData.qDCoffsetSSB[i];
  doc["dacOffsetCW"] = CalData.dacOffsetCW;
  doc["dacOffsetSSB"] = CalData.dacOffsetSSB;
  #endif
  doc["CWradioCalComplete"] = CalData.CWradioCalComplete;
  doc["SSBradioCalComplete"] = CalData.SSBradioCalComplete;
  doc["dBm_calibration"] = CalData.dBm_calibration;

  if (toFile) {
    // Delete existing file, otherwise ConfigData is appended to the file
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
void JSON::printFile(const char *filename) {
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


