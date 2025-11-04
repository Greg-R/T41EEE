// Button class
#pragma once

class Button {

public:

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

  const int32_t TOP_MENU_COUNT{ 11 };
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