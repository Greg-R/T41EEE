
#include "SDT.h"

class JSON {
public:

//bool convertToJson(const RadioMode& src, JsonVariant dst);
//RadioMode convertFromJson(JsonVariantConst src, RadioMode& dst);

//RadioMode JSON::convertFromJson(JsonVariantConst src, RadioMode& dst)

void loadConfiguration(const char *filename, config_t &ConfigData);
void saveConfiguration(const char *filename, const config_t &ConfigData, bool toFile);
void printFile(const char *filename);

};