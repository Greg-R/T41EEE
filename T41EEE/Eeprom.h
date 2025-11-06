// Class Calibrate replaces Process2.cpp.  Greg KF5N June 16, 2024

#pragma once

// Re-factoring into class Calibrate.  Greg KF5N June 15, 2024.
// Automatic calibration added.  Greg KF5N June 11, 2024
// Updates to DoReceiveCalibration() and DoXmitCalibrate() functions by KF5N.  July 20, 2023
// Updated PlotCalSpectrum() function to clean up graphics.  KF5N August 3, 2023
// Major clean-up of calibration.  KF5N August 16, 2023

const int MAX_FAVORITES = 13;  // Max number of favorite frequencies stored in EEPROM
//#define EEPROM_BASE_ADDRESS 0U

class Eeprom {
public:

void ConfigDataWrite();
void ConfigDataRead();
void ConfigDataRead(config_t &configStruct);
void ConfigDataWriteSize(uint32_t structSize);
void CalDataWrite();
void CalDataRead();
void CalDataRead(calibration_t &calStruct);
void CalDataWriteSize(uint32_t structSize);
void BandsWrite();
void BandsRead();
void BandsWriteSize(uint32_t structSize);

int EEPROMReadSize(uint32_t address);
void EEPROMStuffFavorites(uint32_t current[]);
void SetFavoriteFrequency();
void GetFavoriteFrequency();
void CalDataDefaults();
void ConfigDataDefaults();
void EEPROMStartup();

private:
const uint32_t EEPROM_BASE_ADDRESS = 0;
const uint32_t CAL_BASE_ADDRESS = 1024;
const uint32_t BANDS_BASE_ADDRESS = 2048;
};