
// Button class
#pragma once

#include <vector>

class Button {

public:

// Initialize the object with user selected tuning increments specified by the user in MyConfigurationFile.h.
  Button(std::vector<uint32_t>& fineTuneArray, std::vector<uint32_t>& centerTuneArray): fineTuneArray{fineTuneArray}, centerTuneArray{centerTuneArray} {};
//Button() {
//std::vector<uint32_t> centerTuneArray { 1000, 10000, 100000, 1000000 };
//std::vector<uint32_t> fineTuneArray { 10, 20, 50, 100, 200, 500 };
//};

  bool buttonInterruptsEnabled = false;
  void EnableButtonInterrupts();
  MenuSelect ProcessButtonPress(int valPin);
  int ReadSelectedPushButton();
  void ExecuteButtonPress(MenuSelect val);
  void ButtonCenterFreqIncrement();
  void ButtonFineFreqIncrement();
  void ButtonMenuIncrease();
  void ButtonMenuDecrease();
  void ButtonBandIncrease();
  void ButtonBandDecrease();
  void BandSet(int band);
  void ButtonZoom();
  void ButtonFilter();
  void ButtonSelectSideband();
  void ButtonMode();
  void ButtonNR();
  void ButtonNotchFilter();
  void ButtonMuteAudio();
  void ExecuteModeChange();
  void ButtonFrequencyEntry();

private:

//std::vector<uint32_t>& fineTuneArray;
//std::vector<uint32_t>& centerTuneArray;  // k3pto
std::vector<uint32_t> centerTuneArray;
std::vector<uint32_t> fineTuneArray;
std::vector<uint32_t>::iterator result;
  const int32_t TOP_MENU_COUNT{ 10 };
  IntervalTimer buttonInterrupts;
  int buttonRead = 0;
  int minPinRead = 1024;
  int secondaryMenuChoiceMade;
  bool save_last_frequency = false;  // Make this the default behavior.  Greg KF5N October 16, 2024.
  int directFreqFlag = 0;
  int32_t subMenuMaxOptions = 0;  // Holds the number of submenu options.
  uint32_t TxRxFreqOld = 0;
  bool audioState = MUTEAUDIO;
};