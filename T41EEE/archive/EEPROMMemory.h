// Class EEPROM replaces EEPROM.cpp and JSON.CPP.  Greg KF5N September 14, 2024

//#pragma once

#include "SDT.h"

// Re-factoring into class EEPROM.  Greg KF5N September 14, 2024.


class EEPROMMemory {
public:

const int MAX_FAVORITES = 13;  // Max number of favorite frequencies stored in EEPROM
int16_t currentMode;

void EEPROMWrite();
void EEPROMRead();
void EEPROMWriteSize(int structSize);
int EEPROMReadSize();
void EEPROMStuffFavorites(unsigned long current[]);
void SetFavoriteFrequency();
void GetFavoriteFrequency();
void EEPROMDataDefaults();
void EEPROMStartup();
bool convertToJson(const RadioMode& src, JsonVariant dst);
RadioMode convertFromJson(JsonVariantConst src, RadioMode& dst);
void loadConfiguration(const char *filename, config_t &EEPROMData);
void saveConfiguration(const char *filename, const config_t &EEPROMData, bool toFile);
void printFile(const char *filename);

};