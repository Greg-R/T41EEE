// Button class
#pragma once

#define TOP_MENU_COUNT 11  // Changed to 10 after noise floor removed.  Greg KF5N February 14, 2025

class Button {

public:

  IntervalTimer buttonInterrupts;
  bool buttonInterruptsEnabled = false;
  int buttonRead = 0;
  int minPinRead = 1024;
  int secondaryMenuChoiceMade;

  bool save_last_frequency = false;  // Make this the default behavior.  Greg KF5N October 16, 2024.
  int directFreqFlag = 0;
  int32_t subMenuMaxOptions = 0;  // Holds the number of submenu options.
  uint32_t TxRxFreqOld = 0;
  bool audioState = MUTEAUDIO;

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
  int DrawNewFloor(int floor);
  void ExecuteModeChange();
  void ResetZoom(int zoomIndex1);
  void ButtonFrequencyEntry();
};