
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
    Serial.println(F("Failed to read configuration file."));
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
  ConfigData.spectrumNoiseFloor = doc["spectrumNoiseFloor"];  // This is a constant.  This does not need to be included in user data.
  ConfigData.centerTuneStep = doc["centerTuneStep"];
  ConfigData.fineTuneStep = doc["fineTuneStep"];
  ConfigData.transmitPowerLevel = doc["transmitPowerLevel"];
  ConfigData.xmtMode = doc["xmtMode"];
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
  ConfigData.sidetoneVolume = doc["sidetoneVolume"];
  ConfigData.cwTransmitDelay = doc["cwTransmitDelay"];
  ConfigData.activeVFO = doc["activeVFO"];
  ConfigData.currentBand = doc["currentBand"];
  ConfigData.currentBandA = doc["currentBandA"];
  ConfigData.currentBandB = doc["currentBandB"];
  ConfigData.currentFreqA = doc["currentFreqA"];
  ConfigData.currentFreqB = doc["currentFreqB"];
  ConfigData.freqCorrectionFactor = doc["freqCorrectionFactor"];
  for (int i = 0; i < 14; i++) ConfigData.equalizerRec[i] = doc["equalizerRec"][i];
  for (int i = 0; i < 14; i++) ConfigData.equalizerXmt[i] = doc["equalizerXmt"][i];
  ConfigData.equalizerXmt[0] = doc["equalizerXmt"][0];
  ConfigData.currentMicThreshold = doc["currentMicThreshold"];
  ConfigData.currentMicCompRatio = doc["currentMicCompRatio"];
  //ConfigData.currentMicAttack = doc["currentMicAttack"];
  //ConfigData.currentMicRelease = doc["currentMicRelease"];
  ConfigData.currentMicGain = doc["currentMicGain"];
  for (int i = 0; i < 18; i++) ConfigData.switchValues[i] = doc["switchValues"][i];
  ConfigData.LPFcoeff = doc["LPFcoeff"];
  ConfigData.NR_PSI = doc["NR_PSI"];
  ConfigData.NR_alpha = doc["NR_alpha"];
  ConfigData.NR_beta = doc["NR_beta"];
  ConfigData.omegaN = doc["omegaN"];
  ConfigData.pll_fmax = doc["pll_fmax"];
  ConfigData.powerOutCW[0] = doc["powerOutCW"][0];
  ConfigData.powerOutSSB[0] = doc["powerOutSSB"][0];
  for (int i = 0; i < 7; i++) ConfigData.CWPowerCalibrationFactor[i] = doc["CWPowerCalibrationFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.SSBPowerCalibrationFactor[i] = doc["SSBPowerCalibrationFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQCWRXAmpCorrectionFactor[i] = doc["IQCWRXAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQCWRXPhaseCorrectionFactor[i] = doc["IQCWRXPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQCWAmpCorrectionFactor[i] = doc["IQCWAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQCWPhaseCorrectionFactor[i] = doc["IQCWPhaseCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQSSBRXAmpCorrectionFactor[i] = doc["IQSSBRXAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQSSBRXPhaseCorrectionFactor[i] = doc["IQSSBRXPhaseCorrectionFactor"][i];  
  for (int i = 0; i < 7; i++) ConfigData.IQSSBAmpCorrectionFactor[i] = doc["IQSSBAmpCorrectionFactor"][i];
  for (int i = 0; i < 7; i++) ConfigData.IQSSBPhaseCorrectionFactor[i] = doc["IQSSBPhaseCorrectionFactor"][i];

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
  for (int i = 0; i < 7; i++) ConfigData.currentNoiseFloor[i] = doc["currentNoiseFloor"][i];
  ConfigData.compressorFlag = doc["compressorFlag"];
  ConfigData.xmitEQFlag = doc["xmitEQFlag"];
  ConfigData.receiveEQFlag = doc["receiveEQFlag"];
//  ConfigData.calFreq = doc["calFreq"];
  ConfigData.buttonThresholdPressed = doc["buttonThresholdPressed"] | 944;
  ConfigData.buttonThresholdReleased = doc["buttonThresholdReleased"] | 964;
  ConfigData.buttonRepeatDelay = doc["buttonRepeatDelay"] | 300000;
  ConfigData.autoGain = doc["autoGain"] | true;
  #ifdef QSE2
  for (int i = 0; i < 7; i++) ConfigData.iDCoffsetCW[i] = doc["iDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) ConfigData.qDCoffsetCW[i] = doc["qDCoffsetCW"][i];
  for (int i = 0; i < 7; i++) ConfigData.iDCoffsetSSB[i] = doc["iDCoffsetSSB"][i];
  for (int i = 0; i < 7; i++) ConfigData.qDCoffsetSSB[i] = doc["qDCoffsetSSB"][i];  
  ConfigData.dacOffsetCW = doc["dacOffsetCW"] | 0;
  ConfigData.dacOffsetSSB = doc["dacOffsetSSB"] | 0; 
  #endif
  ConfigData.CWradioCalComplete = doc["CWradioCalComplete"] | false;
  ConfigData.SSBradioCalComplete = doc["SSBradioCalComplete"] | false;

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
  doc["spectrumNoiseFloor"] = ConfigData.spectrumNoiseFloor;
  doc["centerTuneStep"] = ConfigData.centerTuneStep;
  doc["fineTuneStep"] = ConfigData.fineTuneStep;
  doc["transmitPowerLevel"] = ConfigData.transmitPowerLevel;
  doc["xmtMode"] = ConfigData.xmtMode;
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
  doc["sidetoneVolume"] = ConfigData.sidetoneVolume;
  doc["cwTransmitDelay"] = ConfigData.cwTransmitDelay;
  doc["activeVFO"] = ConfigData.activeVFO;
  doc["currentBand"] = ConfigData.currentBand;
  doc["currentBandA"] = ConfigData.currentBandA;
  doc["currentBandB"] = ConfigData.currentBandB;
  doc["currentFreqA"] = ConfigData.currentFreqA;
  doc["currentFreqB"] = ConfigData.currentFreqB;
  doc["freqCorrectionFactor"] = ConfigData.freqCorrectionFactor;
  for (int i = 0; i < 14; i++) doc["equalizerRec"][i] = ConfigData.equalizerRec[i];
  for (int i = 0; i < 14; i++) doc["equalizerXmt"][i] = ConfigData.equalizerXmt[i];
  doc["currentMicThreshold"] = ConfigData.currentMicThreshold;
  doc["currentMicCompRatio"] = ConfigData.currentMicCompRatio;
//  doc["currentMicAttack"] = ConfigData.currentMicAttack;
//  doc["currentMicRelease"] = ConfigData.currentMicRelease;
  doc["currentMicGain"] = ConfigData.currentMicGain;
  for (int i = 0; i < 18; i++) doc["switchValues"][i] = ConfigData.switchValues[i];
  doc["LPFcoeff"] = ConfigData.LPFcoeff;
  doc["NR_PSI"] = ConfigData.NR_PSI;
  doc["NR_alpha"] = ConfigData.NR_alpha;
  doc["NR_beta"] = ConfigData.NR_beta;
  doc["omegaN"] = ConfigData.omegaN;
  doc["pll_fmax"] = ConfigData.pll_fmax;
  for (int i = 0; i < 7; i++) doc["powerOutCW"][i] = ConfigData.powerOutCW[i];
  for (int i = 0; i < 7; i++) doc["powerOutSSB"][i] = ConfigData.powerOutSSB[i];
  for (int i = 0; i < 7; i++) {
    doc["CWPowerCalibrationFactor"][i] = ConfigData.CWPowerCalibrationFactor[i];
  }
  for (int i = 0; i < 7; i++) doc["SSBPowerCalibrationFactor"][i] = ConfigData.SSBPowerCalibrationFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXAmpCorrectionFactor"][i] =   ConfigData.IQCWRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWRXPhaseCorrectionFactor"][i] = ConfigData.IQCWRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWAmpCorrectionFactor"][i] =   ConfigData.IQCWAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQCWPhaseCorrectionFactor"][i] = ConfigData.IQCWPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXAmpCorrectionFactor"][i] =   ConfigData.IQSSBRXAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBRXPhaseCorrectionFactor"][i] = ConfigData.IQSSBRXPhaseCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBAmpCorrectionFactor"][i] =   ConfigData.IQSSBAmpCorrectionFactor[i];
  for (int i = 0; i < 7; i++) doc["IQSSBPhaseCorrectionFactor"][i] = ConfigData.IQSSBPhaseCorrectionFactor[i];  
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
  for (int i = 0; i < 7; i++) doc["currentNoiseFloor"][i] = ConfigData.currentNoiseFloor[i];
  doc["compressorFlag"] = ConfigData.compressorFlag;
  doc["xmitEQFlag"] = ConfigData.xmitEQFlag;
  doc["receiveEQFlag"] = ConfigData.receiveEQFlag;
//  doc["calFreq"] = ConfigData.calFreq;
  doc["buttonThresholdPressed"] = ConfigData.buttonThresholdPressed;
  doc["buttonThresholdReleased"] = ConfigData.buttonThresholdReleased;
  doc["buttonRepeatDelay"] = ConfigData.buttonRepeatDelay;
  doc["autoGain"] = ConfigData.autoGain;
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


