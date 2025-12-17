// Button class

#pragma once

class Button {

public:

  // Initialize the object with user selected tuning increments specified by the user in MyConfigurationFile.h.
  Button(std::vector<uint32_t>& fineTuneArray, std::vector<uint32_t>& centerTuneArray)
    : fineTuneArray(fineTuneArray), centerTuneArray(centerTuneArray) {}

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
  void InputParameterButton(const std::string parameterName, std::vector<std::string>selectionList, uint32_t &parameter);
  void InputParameterEncoder(int32_t minValue, int32_t maxValue, uint32_t increment, const std::string parameterName, int32_t &parameter);

private:

  std::vector<uint32_t>& fineTuneArray;
  std::vector<uint32_t>& centerTuneArray;
  std::vector<uint32_t>::iterator result;
  std::vector<std::string>::iterator stringIterator;
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