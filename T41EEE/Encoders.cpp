//#include <charconv>

#include "SDT.h"

float adjustVolEncoder;

#ifdef FAST_TUNE
bool FastTune = true;                    //  HMB
uint32_t FT_last_time;                   // millis() of last fine tune step   HMB
bool FT_ON = false;                      // In fast tunung mode HMB
int FT_step_counter = 0;                 // how many fast steps have there been continuously HMB
int last_FT_step_size = 1;               // so can go back HMB
const unsigned long FT_on_ms = 30;       // time between FTsteps below which increases the step size
const unsigned long FT_cancel_ms = 400;  // time between steps above which FT is cancelled
const int FT_trig = 4;                   // number of short steps to trigger fast tune,
const int FT_step = 500;                 // Hz step in Fast Tune
#endif


/*****
  Purpose: Audio filter adjust with encoder.
           This function runs only if the encoder has been rotated.
  Parameter list:
    void
  Return value;
    void
*****/
// A major change to this function.  It only sets FHiCut and FLoCut variables in the bands2 struct/array.
// No other functions are performed.  Filter settings, demodulation, and graphics changes are delegated to other functions.
// This version limits the result to an fhigh and an flow number.  Note that this function works in concert with EncoderFilter()
// which is attached to an interrupt.  Graphics changes are handled by UpdateAudioGraphics() in Display.cpp.
void FilterSetSSB() {
  int32_t filter_change;
  int filterWidth = static_cast<int>((bands.bands[ConfigData.currentBand].FHiCut - bands.bands[ConfigData.currentBand].FLoCut) / 1000.0 * pixel_per_khz);
  if (filter_pos != last_filter_pos) {  // This decision is required as this function is required to be used in many locations.  KF5N April 21, 2024
    if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) display.BandInformation();
    filter_change = (filter_pos - last_filter_pos);
    if (filter_change >= 1) {
      filterWidth--;  // filterWidth is used in graphics only!
      if (filterWidth < 10)
        filterWidth = 10;
    }
    if (filter_change <= -1) {
      filterWidth++;
      if (filterWidth > 100)
        filterWidth = 50;
    }
    last_filter_pos = filter_pos;
    // Change the FLoCut and FhiCut variables which adjust the DSP filters.

    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      if (switchFilterSideband == true) {  // Adjust and limit FLoCut
        bands.bands[ConfigData.currentBand].FLoCut = bands.bands[ConfigData.currentBand].FLoCut + filterEncoderMove * 100 * ENCODER_FACTOR;
        // Don't allow FLoCut to be less than 100 Hz below FHiCut.
        if (bands.bands[ConfigData.currentBand].FLoCut >= (bands.bands[ConfigData.currentBand].FHiCut - 100)) bands.bands[ConfigData.currentBand].FLoCut = bands.bands[ConfigData.currentBand].FHiCut - 100;
      } else {  // Adjust and limit FHiCut.
        bands.bands[ConfigData.currentBand].FHiCut = bands.bands[ConfigData.currentBand].FHiCut + filterEncoderMove * 100 * ENCODER_FACTOR;
        // Don't allow FHiCut to be less than 100 Hz above FLoCut.
        if (bands.bands[ConfigData.currentBand].FHiCut <= (bands.bands[ConfigData.currentBand].FLoCut + 100)) bands.bands[ConfigData.currentBand].FHiCut = bands.bands[ConfigData.currentBand].FLoCut + 100;
      }
    }

    if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
      bands.bands[ConfigData.currentBand].FAMCut = bands.bands[ConfigData.currentBand].FAMCut + filterEncoderMove * 100 * ENCODER_FACTOR;
    }

    FilterBandwidth();
    volumeChangeFlag = true;
  }

  display.UpdateAudioGraphics();    // Redraw Morse decoder graphics because they get erased due to filter graphics updates.
  display.DrawFrequencyBarValue();  // This calls ShowBandwidth().  YES, this function is useful here.
  display.DrawBandWidthIndicatorBar();

//  Serial.printf("FLoCut = %d FHiCut = %d\n", bands.bands[ConfigData.currentBand].FLoCut, bands.bands[ConfigData.currentBand].FHiCut);
}


/*****
  Purpose: EncoderCenterTune.  This is "coarse" tuning.
  Parameter list:
    void
  Return value;
    void
*****/
void EncoderCenterTune() {
  long tuneChange = 0L;

  unsigned char result = tuneEncoder.process();  // Read the encoder

  if (result == 0)  // Nothing read
    return;

  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE && ConfigData.decoderFlag) {  // No reason to reset if we're not doing decoded CW AFP 09-27-22
    ResetHistograms();
  }

  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      tuneChange = 1L;
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      tuneChange = -1L;
      break;
  }

  ConfigData.centerFreq += (ConfigData.centerTuneStep * tuneChange);  // tune the master vfo
  if (ConfigData.centerFreq < 300000) ConfigData.centerFreq = 300000;
  TxRxFreq = ConfigData.centerFreq + NCOFreq;
  ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO] = TxRxFreq;
  SetFreq();                    //  Change to receiver tuning process.  KF5N July 22, 2023
  display.DrawBandWidthIndicatorBar();  // AFP 10-20-22
  display.ShowFrequency();
  display.BandInformation();
}


/*****
  Purpose: Encoder volume control.  Sets ConfigData.audioVolume between 0 and 100.

  Parameter list:
    void

  Return value;
    void
*****/
void EncoderVolume()  //============================== AFP 10-22-22  Begin new
{
  char result;
//  int increment [[maybe_unused]] = 0;

  result = volumeEncoder.process();  // Read the encoder

  if (result == 0) {  // Nothing read
    return;
  }
  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      adjustVolEncoder = 1;
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      adjustVolEncoder = -1;
      break;
  }

if(ConfigData.audioOut == AudioState::SPEAKER) {
  ConfigData.speakerVolume += adjustVolEncoder;
  if (ConfigData.speakerVolume > 100) {
    ConfigData.speakerVolume = 100;
  } else {
    if (ConfigData.speakerVolume < 0)
      ConfigData.speakerVolume = 0;
  }
}
if(ConfigData.audioOut == AudioState::HEADPHONE) {  
  ConfigData.headphoneVolume += adjustVolEncoder;
  if (ConfigData.headphoneVolume > 100) {
    ConfigData.headphoneVolume = 100;
  } else {
    if (ConfigData.headphoneVolume < 0)
      ConfigData.headphoneVolume = 0;
  }
}

  volumeChangeFlag = true;  // Need this because of unknown timing in display updating.
}


/*****
  Purpose: Use the encoder to change the value of a number in some other function.
           This function does not have a while loop, thus it must be used inside
           some other loop.

  Parameter list:
    int minValue                The lowest value allowed.
    int maxValue                The largest value allowed.
    int startValue              The starting numeric value.
    int increment               The amount by which each increment changes the value.
    std:string prompt[]         The input textual prompt.
    bool left                   Put in left or right slot at top of display.
    bool colorRed               If true, the revised value is displayed in red.
  Return value;
    int                         The revised value.
*****/
float GetEncoderValueLive(float minValue, float maxValue, float startValue, float increment, std::string prompt, bool left, bool colorRed)
{
  float currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  if (left) tft.fillRect(150, 8, 95, 22, RA8875_BLACK);
  else tft.fillRect(425, 8, 95, 22, RA8875_BLACK);  // Increased rectangle size to full erase value.  KF5N August 12, 2023
  if (left) tft.setCursor(0, 1);
  else tft.setCursor(260, 1);
  tft.print(prompt.c_str());
if (left) tft.setCursor(149, 1);
  else tft.setCursor(425, 1);
  if(colorRed) tft.setTextColor(RA8875_RED);  // Make value red if active.
  if (abs(startValue) > 2) {  // Note sure where this restriction came from.
    tft.print(startValue, 0);
  } else {
    if(increment < 0.001) tft.print(startValue, 4);
    else tft.print(startValue, 3);
  }
  tft.setTextColor(RA8875_WHITE);
  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

    if (left) tft.setCursor(149, 1);
    else tft.setCursor(425, 1);
      if(colorRed) tft.setTextColor(RA8875_RED);  // Make value red if active.
    if (abs(startValue) > 2) {
      tft.print(startValue, 0);
    } else {
    if(increment < 0.001) tft.print(startValue, 4);
    else tft.print(startValue, 3);
    }
    tft.setTextColor(RA8875_WHITE);
    filterEncoderMove = 0;
  }
  return currentValue;
}


/*****
  Purpose: Use the encoder to change the value of a number in some other function.
           This function does not have a while loop.  Thus it must be used inside
           some other loop.

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
float GetEncoderValueLiveString(float minValue, float maxValue, float startValue, float increment, std::string prompt, bool left)  //AFP 10-22-22
{
  float currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  if (left) tft.fillRect(160, 0, 85, CHAR_HEIGHT, RA8875_BLACK);
  else tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK);  // Increased rectangle size to full erase value.  KF5N August 12, 2023
  if (left) tft.setCursor(0, 1);
  else tft.setCursor(257, 1);
  tft.print(prompt.c_str());
  if (left) tft.setCursor(160, 1);
  else tft.setCursor(440, 1);
  if (abs(startValue) > 2) {
    tft.print(startValue, 0);
  } else {
    tft.print(startValue, 3);
  }
  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

    if (left) tft.setCursor(160, 1);
    else tft.setCursor(440, 1);
    if (abs(startValue) > 2) {
      tft.print(startValue, 0);
    } else {
      tft.print(startValue, 3);
    }
    filterEncoderMove = 0;
  }
  return currentValue;
}


/*****
  Purpose: Use the encoder to change the value of a number in some other function
           This function does not have a while loop.  Thus it must be used inside
           some other loop.

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    std::string prompt               the input prompt
    bool left                   true to print left (IQ Gain or IDC Offset)
  Return value;
    int                         the new value
*****/
q15_t GetEncoderValueLiveQ15t(int minValue, int maxValue, int startValue, int increment, std::string prompt, bool left)  //AFP 10-22-22
{
  int currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  if (left) tft.fillRect(160, 0, 85, CHAR_HEIGHT, RA8875_BLACK);
  else tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK);  // Increased rectangle size to full erase value.  KF5N August 12, 2023
  if (left) tft.setCursor(0, 1);
  else tft.setCursor(257, 1);
  tft.print(prompt.c_str());
  if (left) tft.setCursor(160, 1);
  else tft.setCursor(440, 1);
  if (abs(startValue) > 2) {
    tft.print(startValue);
  } else {
    tft.print(startValue);
  }

  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

    if (left) tft.setCursor(160, 1);
    else tft.setCursor(440, 1);
    if (abs(startValue) > 2) {
      tft.print(startValue);
    } else {
      tft.print(startValue);
    }
    filterEncoderMove = 0;
  }
  return static_cast<q15_t>(currentValue);
}


/*****
  Purpose: Use the encoder to change the value of a number in some other function.
           This function has a while loop, and it can be used independently.

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
int GetEncoderValue(int minValue, int maxValue, int startValue, int increment, std::string prompt) {
  int currentValue = startValue;
  MenuSelect menu;

  tft.setFontScale((enum RA8875tsize)1);

  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 0, 280, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(257, 1);
  tft.print(prompt.c_str());
  tft.setCursor(470, 1);
  tft.print(startValue);

  while (true) {
    if (filterEncoderMove != 0) {
      currentValue += filterEncoderMove * increment;  // Bump up or down...
      if (currentValue < minValue)
        currentValue = minValue;
      else if (currentValue > maxValue)
        currentValue = maxValue;

      tft.fillRect(465, 0, 65, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(470, 1);
      tft.print(currentValue);
      filterEncoderMove = 0;
    }

    menu = readButton();                           // Use ladder value to get menu choice
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      return currentValue;
    }
  }
}


/*****
  Purpose: Allows quick setting of WPM without going through a menu

  Parameter list:
    void

  Return value;
    int           the current WPM
*****/
int SetWPM() {
  //  int val;
  MenuSelect menu;
  long lastWPM = ConfigData.currentWPM;

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
  tft.print("current WPM:");
  tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
  tft.print(ConfigData.currentWPM);

  while (true) {
    if (filterEncoderMove != 0) {                  // Changed encoder?
      ConfigData.currentWPM += filterEncoderMove;  // Yep
      lastWPM = ConfigData.currentWPM;
      if (lastWPM < 5)  // Set minimum keyer speed to 5 wpm.  KF5N August 20, 2023
        lastWPM = 5;
      else if (lastWPM > MAX_WPM)
        lastWPM = MAX_WPM;

      tft.fillRect(SECONDARY_MENU_X + 200, MENUS_Y + 1, 50, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
      tft.print(lastWPM);
      filterEncoderMove = 0;
    }

    menu = readButton();
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      ConfigData.currentWPM = lastWPM;
      display.UpdateWPMField();
      break;
    }
  }
  eeprom.ConfigDataWrite();
  tft.setTextColor(RA8875_WHITE);
  display.EraseMenus();
  return ConfigData.currentWPM;
}


/*****
  Purpose: Determines how long the transmit relay remains on after last CW atom is sent.

  Parameter list:
    void

  Return value;
    long            the delay length in milliseconds
*****/
uint32_t SetTransmitDelay()
{
  MenuSelect menu;
  long lastDelay = ConfigData.cwTransmitDelay;
  long increment = 250;  // Means a quarter second change per detent

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X - 150, MENUS_Y, EACH_MENU_WIDTH + 150, CHAR_HEIGHT, RA8875_MAGENTA);  // scoot left cuz prompt is long
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 149, MENUS_Y + 1);
  tft.print("current delay:");
  tft.setCursor(SECONDARY_MENU_X + 79, MENUS_Y + 1);
  tft.print(ConfigData.cwTransmitDelay);

  while (true) {
    if (filterEncoderMove != 0) {                  // Changed encoder?
      lastDelay += filterEncoderMove * increment;  // Yep
      if (lastDelay < 0L)
        lastDelay = 250L;

      tft.fillRect(SECONDARY_MENU_X + 80, MENUS_Y + 1, 200, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 79, MENUS_Y + 1);
      tft.print(lastDelay);
      filterEncoderMove = 0;
    }

    menu = readButton();
    if (menu == MenuSelect::MENU_OPTION_SELECT) {  // Make a choice??
      ConfigData.cwTransmitDelay = lastDelay;
      eeprom.ConfigDataWrite();
      break;
    }
  }
  tft.setTextColor(RA8875_WHITE);
  display.EraseMenus();
  return ConfigData.cwTransmitDelay;
}

#ifdef FAST_TUNE
/*****
  Purpose: Fine frequency tune control with variable speed by Harry Brash GM3RVL.  October 30, 2024

  Parameter list:
    void

  Return value;
    void
*****/
void EncoderFineTune() {
  char result;
  unsigned long MS_temp;   // HMB
  unsigned long FT_delay;  // HMB

  result = fineTuneEncoder.process();  // Read the encoder
  if (result == 0) {                   // Nothing read
    fineTuneEncoderMove = 0L;
    return;
  }
  if (result == DIR_CW) {  // 16 = CW, 32 = CCW
    fineTuneEncoderMove = 1L;
  } else {
    fineTuneEncoderMove = -1L;
  }
  MS_temp = millis();  // HMB...

  FT_delay = MS_temp - FT_last_time;
  FT_last_time = MS_temp;

  if (FT_ON) {  // Check if FT should be cancelled (FT_delay>=FT_cancel_ms)
    if (FT_delay >= FT_cancel_ms) {
      FT_ON = false;
      ConfigData.fineTuneStep = last_FT_step_size;
    }
  } else {  //  FT is off so check for short delays
    if (FT_delay <= FT_on_ms) {
      FT_step_counter += 1;
    }
    if (FT_step_counter >= FT_trig) {
      last_FT_step_size = ConfigData.fineTuneStep;
      ConfigData.fineTuneStep = FT_step;  // Set in SDT.h
      FT_step_counter = 0;
      FT_ON = true;
    }
  }

  NCOFreq = NCOFreq + ConfigData.fineTuneStep * fineTuneEncoderMove;  // Increment NCOFreq per encoder movement.
  centerTuneFlag = 1;                                                 // This is used in Process.cpp.  Greg KF5N May 16, 2024
  // ============  AFP 10-28-22
  if (ConfigData.activeVFO == VFO_A) {
    ConfigData.currentFreqA = ConfigData.centerFreq + NCOFreq;  //AFP 10-05-22
    ConfigData.lastFrequencies[ConfigData.currentBand][0] = ConfigData.currentFreqA;
  } else {
    ConfigData.currentFreqB = ConfigData.centerFreq + NCOFreq;  //AFP 10-05-22
    ConfigData.lastFrequencies[ConfigData.currentBand][1] = ConfigData.currentFreqB;
  }
  // ===============  Recentering at band edges ==========
  if (ConfigData.spectrum_zoom != 0) {
    if (NCOFreq >= static_cast<int32_t>((95000 / (1 << ConfigData.spectrum_zoom))) || NCOFreq < static_cast<int32_t>((-93000 / (1 << ConfigData.spectrum_zoom)))) {  // 47500 with 2x zoom.
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  } else {
    if (NCOFreq > 142000 || NCOFreq < -43000) {  // Offset tuning window in zoom 1x
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  }
  fineTuneEncoderMove = 0L;
  TxRxFreq = ConfigData.centerFreq + NCOFreq;  // KF5N
}
#else
/*****
  Purpose: Fine frequency tune control.

  Parameter list:
    void

  Return value;
    void
*****/
void EncoderFineTune() {
  char result;

  result = fineTuneEncoder.process();  // Read the encoder
  if (result == 0) {                   // Nothing read
    fineTuneEncoderMove = 0L;
    return;
  } else {
    if (result == DIR_CW) {  // 16 = CW, 32 = CCW
      fineTuneEncoderMove = 1L;
    } else {
      fineTuneEncoderMove = -1L;
    }
  }
  NCOFreq = NCOFreq + ConfigData.fineTuneStep * fineTuneEncoderMove;  // Increment NCOFreq per encoder movement.
  centerTuneFlag = 1;                                                 // This is used in Process.cpp.  Greg KF5N May 16, 2024
  // ============  AFP 10-28-22
  if (ConfigData.activeVFO == VFO_A) {
    ConfigData.currentFreqA = ConfigData.centerFreq + NCOFreq;  //AFP 10-05-22
    ConfigData.lastFrequencies[ConfigData.currentBand][0] = ConfigData.currentFreqA;
  } else {
    ConfigData.currentFreqB = ConfigData.centerFreq + NCOFreq;  //AFP 10-05-22
    ConfigData.lastFrequencies[ConfigData.currentBand][1] = ConfigData.currentFreqB;
  }
  // ===============  Recentering at band edges ==========
  if (ConfigData.spectrum_zoom != 0) {
    if (NCOFreq >= static_cast<int32_t>((95000 / (1 << ConfigData.spectrum_zoom))) || NCOFreq < static_cast<int32_t>((-93000 / (1 << ConfigData.spectrum_zoom)))) {  // 47500 with 2x zoom.
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  } else {
    if (NCOFreq > 142000 || NCOFreq < -43000) {  // Offset tuning window in zoom 1x
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  }
  fineTuneEncoderMove = 0L;
  TxRxFreq = ConfigData.centerFreq + NCOFreq;  // KF5N
}
#endif


// This function is attached to interrupts (in the .ino file).
void EncoderFilter() {
  char result;
  result = filterEncoder.process();  // Read the encoder

  if (result == 0) {
    //    filterEncoderMove = 0;// Nothing read
    return;
  }

  switch (result) {
    case DIR_CW:  // Turned it clockwise
      filterEncoderMove = 1;
      //filter_pos = last_filter_pos - 5 * filterEncoderMove;  // AFP 10-22-22
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      filterEncoderMove = -1;
      // filter_pos = last_filter_pos - 5 * filterEncoderMove;   // AFP 10-22-22
      break;
  }
  if (calibrateFlag == false and morseDecodeAdjustFlag == false) {  // This is done so that filter adjustment is not affected during these operations.
    filter_pos = last_filter_pos - 5 * filterEncoderMove;           // AFP 10-22-22.  Hmmm.  Why multiply by 5???  Greg KF5N April 21, 2024
  }                                                                 // AFP 10-22-22   filter_pos is allowed to go negative.  This may be a problem.
}
