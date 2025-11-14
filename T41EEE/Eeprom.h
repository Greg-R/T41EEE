// EEPROM class

#pragma once

const int MAX_FAVORITES = 13;  // Max number of favorite frequencies stored in EEPROM

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