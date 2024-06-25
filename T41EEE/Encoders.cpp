
#include "SDT.h"

float adjustVolEncoder;

/*****
  Purpose: Audio filter adjust with encoder.  This function is called from ShowSpectrum() in Display.cpp.
           This function runs only if the encoder has been rotated.
  Parameter list:
    void
  Return value;
    void
    Modified AFP21-12-15
*****/
void FilterSetSSB() {
uint32_t filter_change;
  if (filter_pos != last_filter_pos) {  // This decision is required as this function is required to be used in many locations.  KF5N April 21, 2024
    tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
    tft.clearMemory();
    if(EEPROMData.xmtMode == CW_MODE) BandInformation(); 
    tft.fillRect((MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2 - filterWidth, SPECTRUM_TOP_Y + 17, filterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  // Erase old filter background
    filter_change = (filter_pos - last_filter_pos);
    if (filter_change >= 1) {
      filterWidth--;           // filterWidth is used in graphics only!
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
    switch (bands[EEPROMData.currentBand].mode) {
      case DEMOD_LSB:
        if (switchFilterSideband == false)  // LSB "0" = normal, "1" means change opposite filter.  ButtonFilter() function swaps this.
        {  // Adjust FLoCut and limit FLoCut based on the current frequency of FHiCut.
          bands[EEPROMData.currentBand].FLoCut = bands[EEPROMData.currentBand].FLoCut - filterEncoderMove * 100 * ENCODER_FACTOR;  
          // Don't allow FLoCut to be less than 100 Hz below FHiCut.
          if(bands[EEPROMData.currentBand].FLoCut >= (bands[EEPROMData.currentBand].FHiCut - 100)) bands[EEPROMData.currentBand].FLoCut = bands[EEPROMData.currentBand].FHiCut - 100;          
        } else if (switchFilterSideband == true) {  // Adjust and limit FHiCut.
          bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FHiCut - filterEncoderMove * 100 * ENCODER_FACTOR;
          if(bands[EEPROMData.currentBand].FHiCut >= -100) bands[EEPROMData.currentBand].FHiCut = -100;  // Don't allow FLoCut to go above -100.
          // Don't allow FHiCut to be less than 100 Hz above FLoCut.
          if(bands[EEPROMData.currentBand].FHiCut <= (bands[EEPROMData.currentBand].FLoCut + 100)) bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FLoCut + 100;
        }
        FilterBandwidth();
        break;
      case DEMOD_USB:
      if (switchFilterSideband == false)
 {  // Adjust and limit FHiCut.
          bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FHiCut + filterEncoderMove * 100 * ENCODER_FACTOR;
          // Don't allow FHiCut to be less than 100 Hz above FLoCut.
          if(bands[EEPROMData.currentBand].FHiCut <= (bands[EEPROMData.currentBand].FLoCut + 100)) bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FLoCut + 100;
        }        
        else if         (switchFilterSideband == true)
        {  // Adjust FLoCut and limit FLoCut based on the current frequency of FHiCut.
          bands[EEPROMData.currentBand].FLoCut = bands[EEPROMData.currentBand].FLoCut + filterEncoderMove * 100 * ENCODER_FACTOR;  
          // Don't allow FLoCut to go below 100.
          if(bands[EEPROMData.currentBand].FLoCut <= 100) bands[EEPROMData.currentBand].FLoCut = 100;  
          // Don't allow FLoCut to be less than 100 Hz below FHiCut.
          if(bands[EEPROMData.currentBand].FLoCut >= (bands[EEPROMData.currentBand].FHiCut - 100)) bands[EEPROMData.currentBand].FLoCut = bands[EEPROMData.currentBand].FHiCut - 100;          
        } 
        FilterBandwidth();
        break;
      case DEMOD_AM:
        bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
        bands[EEPROMData.currentBand].FLoCut = -bands[EEPROMData.currentBand].FHiCut;
        FilterBandwidth();
//        InitFilterMask();  This function is called by FilterBandwidth().  Greg KF5N April 21, 2024
        break;
      case DEMOD_SAM:  // AFP 11-03-22
        bands[EEPROMData.currentBand].FHiCut = bands[EEPROMData.currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
        bands[EEPROMData.currentBand].FLoCut = -bands[EEPROMData.currentBand].FHiCut;
        FilterBandwidth();
//        InitFilterMask();
        break;
    }
      volumeChangeFlag = true;
  }
    // =============  AFP 10-27-22

    //ControlFilterF();
//    Menu2 = MENU_F_LO_CUT;  // set Menu2 to MENU_F_LO_CUT
//    FilterBandwidth();  // Do any of these functions do anything???
//    ShowBandwidth();
//  Which function adjusts the frequency limit bars in the audio spectrum display???
//    DrawFrequencyBarValue();  // This calls ShowBandwidth().  YES, this function is useful here.
//    UpdateDecoderField();   // Redraw Morse decoder graphics because they get erased due to filter graphics updates.

//  The following code was moved from ShowSpectrum() in Display.cpp.
        int filterLoPositionMarker{0};
        int filterHiPositionMarker{0};
        int temp{0};
        filterLoPositionMarker = map(bands[EEPROMData.currentBand].FLoCut, 0, 6000, 0, 256);
        filterHiPositionMarker = map(bands[EEPROMData.currentBand].FHiCut, 0, 6000, 0, 256);
        // Flip positions if LSB so that correct delimiter is highlighted.
        if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
           temp = filterLoPositionMarker;
           filterLoPositionMarker = filterHiPositionMarker;
           filterHiPositionMarker = temp;
        }
        //Draw Filter indicator lines on audio plot to Layer 2.
        tft.writeTo(L2);
        if(not switchFilterSideband) {
        tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_LIGHT_GREY);
        tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_RED);
        } else {
        tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_RED);
        tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_LIGHT_GREY);
        }

    tft.writeTo(L1);    
    DrawFrequencyBarValue();  // This calls ShowBandwidth().  YES, this function is useful here.
    UpdateDecoderField();   // Redraw Morse decoder graphics because they get erased due to filter graphics updates.
    DrawBandWidthIndicatorBar();
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
  //  long oldFreq    = EEPROMData.centerFreq;

  unsigned char result = tuneEncoder.process();  // Read the encoder

  if (result == 0)  // Nothing read
    return;

  if (EEPROMData.xmtMode == CW_MODE && EEPROMData.decoderFlag) {  // No reason to reset if we're not doing decoded CW AFP 09-27-22
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

  EEPROMData.centerFreq += (EEPROMData.centerTuneStep * tuneChange);  // tune the master vfo
  if(EEPROMData.centerFreq < 300000) EEPROMData.centerFreq = 300000;
  TxRxFreq = EEPROMData.centerFreq + NCOFreq;
  EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO] = TxRxFreq;
  SetFreq();  //  Change to receiver tuning process.  KF5N July 22, 2023
  //currentFreqA= EEPROMData.centerFreq + NCOFreq;
  DrawBandWidthIndicatorBar();  // AFP 10-20-22
  //FilterOverlay(); // AFP 10-20-22
  ShowFrequency();
  BandInformation();
}


/*****
  Purpose: Encoder volume control.  Sets EEPROMData.audioVolume between 0 and 100.

  Parameter list:
    void

  Return value;
    void
*****/
void EncoderVolume()  //============================== AFP 10-22-22  Begin new
{
  char result;
  int increment [[maybe_unused]] = 0;

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
  EEPROMData.audioVolume += adjustVolEncoder; 

  if (EEPROMData.audioVolume > 100) {
    EEPROMData.audioVolume = 100;
  } else  {
    if (EEPROMData.audioVolume < 0) 
      EEPROMData.audioVolume = 0;
  }

  volumeChangeFlag = true;  // Need this because of unknown timing in display updating.
}  //============================== AFP 10-22-22  End new


/*****
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
float GetEncoderValueLive(float minValue, float maxValue, float startValue, float increment, char prompt[], bool left)  //AFP 10-22-22
{
  float currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  if(left) tft.fillRect(160, 0, 85, CHAR_HEIGHT, RA8875_BLACK); else tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK); // Increased rectangle size to full erase value.  KF5N August 12, 2023
  if(left)  tft.setCursor(0, 1); else tft.setCursor(257, 1);
  tft.print(prompt);
  if(left)  tft.setCursor(160, 1); else  tft.setCursor(440, 1);
  if (abs(startValue) > 2) {
    tft.print(startValue, 0);
  } else {
    tft.print(startValue, 3);
  }
  //while (true) {
  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

  //  tft.fillRect(449, 0, 90, CHAR_HEIGHT, RA8875_BLACK);  // This is not required. KF5N August 12, 2023
   if(left) tft.setCursor(160, 1); else tft.setCursor(440, 1);
    if (abs(startValue) > 2) {
      tft.print(startValue, 0);
    } else {
      tft.print(startValue, 3);
    }
    filterEncoderMove = 0;
  }
  //tft.setTextColor(RA8875_WHITE);
  return currentValue;
}


/*****
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
q15_t GetEncoderValueLiveQ15t(int minValue, int maxValue, int startValue, int increment, char prompt[], bool left)  //AFP 10-22-22
{
  int currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  if(left) tft.fillRect(160, 0, 85, CHAR_HEIGHT, RA8875_BLACK); else tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK); // Increased rectangle size to full erase value.  KF5N August 12, 2023
  if(left)  tft.setCursor(0, 1); else tft.setCursor(257, 1);
  tft.print(prompt);
  if(left)  tft.setCursor(160, 1); else  tft.setCursor(440, 1);
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

  //  tft.fillRect(449, 0, 90, CHAR_HEIGHT, RA8875_BLACK);  // This is not required. KF5N August 12, 2023
    if(left) tft.setCursor(160, 1); else tft.setCursor(440, 1);
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
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
int GetEncoderValue(int minValue, int maxValue, int startValue, int increment, char prompt[]) {
  int currentValue = startValue;
  int val;

  tft.setFontScale((enum RA8875tsize)1);

  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 0, 280, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(257, 1);
  tft.print(prompt);
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

    val = ReadSelectedPushButton();  // Read the ladder value
    //MyDelay(100L); //AFP 09-22-22
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);    // Use ladder value to get menu choice
      if (val == MENU_OPTION_SELECT) {  // Make a choice??
        return currentValue;
      }
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
  int val;
  long lastWPM = EEPROMData.currentWPM;

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
  tft.print("current WPM:");
  tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
  tft.print(EEPROMData.currentWPM);

  while (true) {
    if (filterEncoderMove != 0) {       // Changed encoder?
      EEPROMData.currentWPM += filterEncoderMove;  // Yep
      lastWPM = EEPROMData.currentWPM;
      if (lastWPM < 5)    // Set minimum keyer speed to 5 wpm.  KF5N August 20, 2023
        lastWPM = 5;
      else if (lastWPM > MAX_WPM)
        lastWPM = MAX_WPM;

      tft.fillRect(SECONDARY_MENU_X + 200, MENUS_Y + 1, 50, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
      tft.print(lastWPM);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      EEPROMData.currentWPM = lastWPM;
      //EEPROMData.EEPROMData.currentWPM = EEPROMData.currentWPM;
      UpdateWPMField();
      break;
    }
  }
  EEPROMWrite();
  tft.setTextColor(RA8875_WHITE);
  EraseMenus();
  return EEPROMData.currentWPM;
}


/*****
  Purpose: Determines how long the transmit relay remains on after last CW atom is sent.

  Parameter list:
    void

  Return value;
    long            the delay length in milliseconds
*****/
long SetTransmitDelay()  // new function JJP 9/1/22
{
  int val;
  long lastDelay = EEPROMData.cwTransmitDelay;
  long increment = 250;  // Means a quarter second change per detent

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X - 150, MENUS_Y, EACH_MENU_WIDTH + 150, CHAR_HEIGHT, RA8875_MAGENTA);  // scoot left cuz prompt is long
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 149, MENUS_Y + 1);
  tft.print("current delay:");
  tft.setCursor(SECONDARY_MENU_X + 79, MENUS_Y + 1);
  tft.print(EEPROMData.cwTransmitDelay);

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

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    //MyDelay(150L);  //ALF 09-22-22
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      EEPROMData.cwTransmitDelay = lastDelay;
      //EEPROMData.EEPROMData.cwTransmitDelay = EEPROMData.cwTransmitDelay;
      EEPROMWrite();
      break;
    }
  }
  tft.setTextColor(RA8875_WHITE);
  EraseMenus();
  return EEPROMData.cwTransmitDelay;
}


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
  NCOFreq = NCOFreq + EEPROMData.fineTuneStep * fineTuneEncoderMove;  // Increment NCOFreq per encoder movement.
  centerTuneFlag = 1;   // This is used in Process.cpp.  Greg KF5N May 16, 2024
  // ============  AFP 10-28-22
  if (EEPROMData.activeVFO == VFO_A) {
    EEPROMData.currentFreqA = EEPROMData.centerFreq + NCOFreq;  //AFP 10-05-22
    EEPROMData.lastFrequencies[EEPROMData.currentBand][0] = EEPROMData.currentFreqA;
  } else {
    EEPROMData.currentFreqB = EEPROMData.centerFreq + NCOFreq;  //AFP 10-05-22
    EEPROMData.lastFrequencies[EEPROMData.currentBand][1] = EEPROMData.currentFreqB;
  }
  // ===============  Recentering at band edges ==========
  if (EEPROMData.spectrum_zoom != 0) {
    if (NCOFreq >= static_cast<int32_t>((95000 / (1 << EEPROMData.spectrum_zoom))) || NCOFreq < static_cast<int32_t>((-93000 / (1 << EEPROMData.spectrum_zoom)))) {  // 47500 with 2x zoom.
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
  TxRxFreq = EEPROMData.centerFreq + NCOFreq;  // KF5N
}


// This function is attached to interrupts (in the .ino file).
  void EncoderFilter() {
  char result;
  result = filterEncoder.process();  // Read the encoder

  if (result == 0) {
    //    filterEncoderMove = 0;// Nothing read
    return;
  }

  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      filterEncoderMove = 1;
      //filter_pos = last_filter_pos - 5 * filterEncoderMove;  // AFP 10-22-22
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      filterEncoderMove = -1;
      // filter_pos = last_filter_pos - 5 * filterEncoderMove;   // AFP 10-22-22
      break;
  }
  if (calibrateFlag == 0) {                                // AFP 10-22-22
    filter_pos = last_filter_pos - 5 * filterEncoderMove;  // AFP 10-22-22.  Hmmm.  Why multiply by 5???  Greg KF5N April 21, 2024
  }                                                        // AFP 10-22-22   filter_pos is allowed to go negative.  This may be a problem.
}