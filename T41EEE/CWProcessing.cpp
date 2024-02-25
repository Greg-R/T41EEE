
#include "SDT.h"

#define HISTOGRAM_ELEMENTS 750

float32_t aveCorrResultR;  // Used in running averages; must not be inside function.
float32_t aveCorrResultL;
//float32_t float_Corr_BufferR[511] = {0};  // DMAMEM may cause problems with these two buffers and CW decode.
//float32_t float_Corr_BufferL[511] = {0};
float32_t* float_Corr_BufferR = new float32_t[511];
float32_t* float_Corr_BufferL = new float32_t[511];
float CWLevelTimer;
float CWLevelTimerOld;
float goertzelMagnitude;
char decodeBuffer[33] = {0};  // The buffer for holding the decoded characters.  Increased to 33.  KF5N October 29, 2023
int endGapFlag = 0;
int topGapIndex;
int topGapIndexOld;
//int32_t gapHistogram[HISTOGRAM_ELEMENTS];  // DMAMEM???
//int32_t signalHistogram[HISTOGRAM_ELEMENTS];
int32_t* gapHistogram = new int32_t[HISTOGRAM_ELEMENTS];
int32_t* signalHistogram = new int32_t[HISTOGRAM_ELEMENTS];
long valRef1;
long valRef2;
long gapRef1;
int valFlag = 0;
long signalStartOld = 0;
long aveDitLength = 80;
long aveDahLength = 200;
float thresholdGeometricMean = 140.0;  // This changes as decoder runs
float thresholdArithmeticMean;
int dahLength;
int gapAtom;  // Space between atoms
int gapChar;  // Space between characters
long signalElapsedTime;
long signalStart;
long signalEnd;  // Start-end of dit or dah
uint32_t gapLength;
float freq[4] = { 562.5, 656.5, 750.0, 843.75 };

// This enum is used by an experimental Morse decoder.
enum states { state0,
              state1,
              state2,
              state3,
              state4,
              state5,
              state6 };
states decodeStates = state0;

//=================  AFP10-18-22 ================
/*****
  Purpose: Select CW Filter. EEPROMData.CWFilterIndex has these values:
           0 = 840Hz
           1 = 1kHz
           2 = 1.3kHz
           3 = 1.8kHz
           4 = 2kHz
           5 = Off

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void SelectCWFilter() {
  const char *CWFilter[] = { "0.8kHz", "1.0kHz", "1.3kHz", "1.8kHz", "2.0kHz", " Off " };
  EEPROMData.CWFilterIndex = SubmenuSelect(CWFilter, 6, 0);  // CWFilter is an array of strings.
  // Clear the current CW filter graphics and then restore the bandwidth indicator bar.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  BandInformation();
  DrawBandWidthIndicatorBar();
  UpdateDecoderField();
}


/*****
  Purpose: Select CW tone frequency.

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void SelectCWOffset() {
  const char *CWOffsets[] = { "562.5 Hz", "656.5 Hz", "750 Hz", "843.75 Hz", " Cancel " };
  const int numCycles[4] = { 6, 7, 8, 9 };
  EEPROMData.CWOffset = SubmenuSelect(CWOffsets, 5, EEPROMData.CWOffset);  // CWFilter is an array of strings.
  // Now generate the values for the buffer which is used to create the CW tone.  The values are discrete because there must be whole cycles.
  if (EEPROMData.CWOffset < 4) sineTone(numCycles[EEPROMData.CWOffset]);
  EEPROMWrite();  // Save to EEPROM.
  // Clear the current CW filter graphics and then restore the bandwidth indicator bar.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  RedrawDisplayScreen();
}


//=================  AFP10-18-22 ================
/*****
  Purpose: to process CW specific signals

  Parameter list:
    void

  Return value:
    void

*****/
void DoCWReceiveProcessing() {  // All New AFP 09-19-22
  float goertzelMagnitude1;
  float goertzelMagnitude2;
  float32_t aveCorrResult;
  float32_t corrResultR;
  uint32_t corrResultIndexR;
  float32_t corrResultL;
  uint32_t corrResultIndexL;
  float32_t combinedCoeff;                          //AFP 02-06-22
  int audioTemp;                                    // KF5N
//  float freq[4] = { 562.5, 656.5, 750.0, 843.75 };  // User selectable CW offset frequencies.

  arm_fir_f32(&FIR_CW_DecodeL, float_buffer_L, float_buffer_L_CW, 256);  // AFP 10-25-22  Park McClellan FIR filter const Group delay
  arm_fir_f32(&FIR_CW_DecodeR, float_buffer_R, float_buffer_R_CW, 256);  // AFP 10-25-22

  if (EEPROMData.decoderFlag) {  // JJP 7/20/23

    // ----------------------  Correlation calculation  AFP 02-04-22 -------------------------
    //Calculate correlation between calc sine and incoming signal

    arm_correlate_f32(float_buffer_R_CW, 256, sinBuffer, 256, float_Corr_BufferR);
    arm_max_f32(float_Corr_BufferR, 511, &corrResultR, &corrResultIndexR);
    // Running average of corr coeff. R
    aveCorrResultR = .7 * corrResultR + .3 * aveCorrResultR;
    arm_correlate_f32(float_buffer_L_CW, 256, sinBuffer, 256, float_Corr_BufferL);
    // Get max value of correlation
    arm_max_f32(float_Corr_BufferL, 511, &corrResultL, &corrResultIndexL);
    // Running average of corr coeff. L
    aveCorrResultL = .7 * corrResultL + .3 * aveCorrResultL;
    aveCorrResult = (corrResultR + corrResultL) / 2;
    // Calculate Goertzel Mahnitude of incomming signal
    goertzelMagnitude1 = goertzel_mag(256, freq[EEPROMData.CWOffset], 24000, float_buffer_L_CW);  //AFP 10-25-22
    goertzelMagnitude2 = goertzel_mag(256, freq[EEPROMData.CWOffset], 24000, float_buffer_R_CW);  //AFP 10-25-22
    goertzelMagnitude = (goertzelMagnitude1 + goertzelMagnitude2) / 2;
    //Combine Correlation and Gowetzel Coefficients
    combinedCoeff = 200 * aveCorrResult * goertzelMagnitude;
//    Serial.printf("combinedCoeff = %f\n", combinedCoeff);  // Use this to tune decoder.
    // ==========  Changed CW decode "lock" indicator
    if (combinedCoeff > 50) {  // AFP 10-26-22
      tft.fillRect(700, 442, 15, 15, RA8875_GREEN);
    } else if (combinedCoeff < 50) {  // AFP 10-26-22
      CWLevelTimer = millis();
      if (CWLevelTimer - CWLevelTimerOld > 2000) {
        CWLevelTimerOld = millis();
        tft.fillRect(700, 442, 15, 15, RA8875_BLACK);  // Erase
      }
    }
    if (combinedCoeff > 50) {  // if  have a reasonable corr coeff, >50, then we have a keeper. // AFP 10-26-22
      audioTemp = 1;
    } else {
      audioTemp = 0;
    }
    //==============  acquire data on CW  ================
    DoCWDecoding(audioTemp);
  }
}

/*****
  Purpose: to provide spacing between letters

  Parameter list:
  void

  Return value:
  void

  CAUTION: Assumes that a global named ditLength holds the value for dit spacing
*****/
void LetterSpace() {
  MyDelay(3UL * ditLength);
}
/*****
  Purpose: to provide spacing between words

  Parameter list:
  void

  Return value:
  void

  CAUTION: Assumes that a global named ditLength holds the value for dit spacing
*****/
void WordSpace() {
  MyDelay(7UL * ditLength);
}


/*****
  Purpose: establish the dit length for code transmission. Crucial since
    all spacing is done using dit length

  Parameter list:
    int wpm

  Return value:
    void
*****/
void SetDitLength(int wpm) {
  ditLength = 1200 / wpm;
}

/*****
  Purpose: establish the dit length for code transmission. Crucial since
    all spacing is done using dit length

  Parameter list:
    int wpm

  Return value:
    void
*****/
void SetTransmitDitLength(int wpm) {
  transmitDitLength = 1200 / wpm;  // JJP 8/19/23

  // Total audio blocks that will be output = 1 (rise) + transmit(Dit|Dah)UnshapedBlocks + 1 (fall)
  // Blocks are assumed to be 10ms long, and the number of unshaped blocks is rounded to acheive
  // the best approximation of the actual desired dit and dah times.
  if (transmitDitLength < 20) {
    transmitDitUnshapedBlocks = 0;
    transmitDahUnshapedBlocks = 0;
  } else {
    transmitDitUnshapedBlocks = (unsigned long)rint(((double)transmitDitLength - 20.0) / 10.0);
    transmitDahUnshapedBlocks = (unsigned long)rint((((double)transmitDitLength * 3.0) - 20.0) / 10.0);
  }
}

/*****
  Purpose: Select straight key or keyer

  Parameter list:
    void

  Return value:
    void
*****/
void SetKeyType() {
  const char *keyChoice[] = { "Straight Key", "Keyer" };

  EEPROMData.keyType = SubmenuSelect(keyChoice, 2, 0);
  // Make sure the EEPROMData.paddleDit and EEPROMData.paddleDah variables are set correctly for straight key.
  // Paddle flip can reverse these, making the straight key inoperative.  KF5N August 9, 2023
  if (EEPROMData.keyType == 0) {
    EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
    EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;
  }
  EEPROMWrite();
}


/*****
  Purpose: Set up key at power-up.

  Parameter list:
    void

  Return value:
    void
*****/
FLASHMEM void SetKeyPowerUp() {
  if (EEPROMData.keyType == 0) {
    EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
    EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;
    return;
  }
  if (EEPROMData.paddleFlip) {  // Means right-paddle dit
    EEPROMData.paddleDit = KEYER_DAH_INPUT_RING;
    EEPROMData.paddleDah = KEYER_DIT_INPUT_TIP;
  } else {
    EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
    EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;
  }
}


/*****
  Purpose: Allow user to set the sidetone volume.  KF5N August 31, 2023

  Parameter list:
    void

  Return value;
    void
*****/
void SetSideToneVolume() {
  int val, sidetoneDisplay;
  bool keyDown;

  SetAudioOperatingState(CW_TRANSMIT_STRAIGHT_STATE);
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 60, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 48, MENUS_Y + 1);
  tft.print("Sidetone Volume:");
  tft.setCursor(SECONDARY_MENU_X + 220, MENUS_Y + 1);
  sidetoneDisplay = (int)(EEPROMData.sidetoneVolume);
  keyDown = false;
  tft.print(sidetoneDisplay);  // Display in range of 0 to 100.
  modeSelectInR.gain(0, 0);
  modeSelectInL.gain(0, 0);
  modeSelectInExR.gain(0, 0);
  modeSelectOutL.gain(0, 0);
  modeSelectOutR.gain(0, 0);
  modeSelectOutExL.gain(0, 0);
  modeSelectOutExR.gain(0, 0);
  digitalWrite(MUTE, LOW);      // unmutes audio
  modeSelectOutL.gain(1, 0.0);  // Sidetone  AFP 10-01-22
  modeSelectOutR.gain(1, 0.0);  // Sidetone  AFP 10-01-22

  while (true) {
    if (digitalRead(EEPROMData.paddleDit) == LOW || digitalRead(EEPROMData.paddleDah) == LOW) {
      if (keyDown) {
        CW_ExciterIQData(CW_SHAPING_NONE);
      } else {
        CW_ExciterIQData(CW_SHAPING_RISE);
        keyDown = true;
      }
    } else {
      if (keyDown) {
        CW_ExciterIQData(CW_SHAPING_FALL);
        keyDown = false;
      }
    }

    if (filterEncoderMove != 0) {
      //      EEPROMData.sidetoneVolume = EEPROMData.sidetoneVolume + (float)filterEncoderMove * 0.001;  // EEPROMData.sidetoneVolume range is 0.0 to 1.0 in 0.001 steps.  KF5N August 29, 2023
      sidetoneDisplay = sidetoneDisplay + filterEncoderMove;  // * 0.001;  // EEPROMData.sidetoneVolume range is 0.0 to 1.0 in 0.001 steps.  KF5N August 29, 2023
      if (sidetoneDisplay < 0)
        sidetoneDisplay = 0;
      else if (sidetoneDisplay > 100)  // 100% max
        sidetoneDisplay = 100;
      tft.fillRect(SECONDARY_MENU_X + 200, MENUS_Y, 70, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 220, MENUS_Y + 1);
      EEPROMData.sidetoneVolume = (float32_t)sidetoneDisplay;
      tft.setTextColor(RA8875_WHITE);
      tft.print(sidetoneDisplay);
      filterEncoderMove = 0;
    }
    modeSelectOutL.gain(0, volumeLog[(int)EEPROMData.sidetoneVolume]);  // Sidetone  AFP 10-01-22
                                                                        //    modeSelectOutR.gain(1, volumeLog[(int)EEPROMData.sidetoneVolume]);  // Right side not used.  KF5N September 1, 2023
    val = ReadSelectedPushButton();                                     // Read pin that controls all switches
    val = ProcessButtonPress(val);
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
                                      // EEPROMData.EEPROMData.sidetoneVolume = EEPROMData.sidetoneVolume;
      EEPROMWrite();
      break;
    }
  }
  EraseMenus();
  lastState = 1111;  // This is required due to the function deactivating the receiver.  This forces a pass through the receiver set-up code.  KF5N October 7, 2023
}


//==================================== Decoder =================
//DB2OO, 29-AUG-23: moved col declaration here
static int col = 0;  // Start at lower left

/*****
    DB2OO, 29-AUG-23: added
  Purpose: This function clears the morse code text buffer

  Parameter list:
    

  Return value
    void
*****/
void MorseCharacterClear(void) {
  col = 0;
  decodeBuffer[col] = '\0';  // Make it a string
}

/*****
  Purpose: This function displays the decoded Morse code below waterfall. Arranged as:

  Parameter list:
    char currentLetter

  Return value
    void
*****/
void MorseCharacterDisplay(char currentLetter) {
  if (col < MAX_DECODE_CHARS) {  // Start scrolling??
    decodeBuffer[col] = currentLetter;
    col++;
    decodeBuffer[col] = '\0';  // Make is a string
  } else {
    //DB2OO, 25-AUG-23: use memmove instead of memcpy(), to avoid the warning
    memmove(decodeBuffer, &decodeBuffer[1], MAX_DECODE_CHARS - 1);  // Slide array down 1 character.
    decodeBuffer[col - 1] = currentLetter;                          // Add to end
    decodeBuffer[col] = '\0';                                       // Make is a string
  }
  tft.fillRect(CW_TEXT_START_X, CW_TEXT_START_Y, CW_MESSAGE_WIDTH, CW_MESSAGE_HEIGHT * 2, RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(CW_TEXT_START_X, CW_TEXT_START_Y);
  tft.print(decodeBuffer);
}


/*****
  Purpose: This function uses the current WPM to set an estimate ditLength any time the tune
           endcoder is changed

  Parameter list:
    void

  Return value
    void
*****/
void ResetHistograms() {
  gapAtom = ditLength = transmitDitLength;
  gapChar = dahLength = transmitDitLength * 3;
  thresholdGeometricMean = (ditLength + dahLength) / 2;  // Use simple mean for starters so we don't have 0
  aveDitLength = ditLength;
  aveDahLength = dahLength;
  valRef1 = 0;
  valRef2 = 0;
  // Clear graph arrays
  memset(signalHistogram, 0, HISTOGRAM_ELEMENTS * sizeof(uint32_t));
  memset(gapHistogram, 0, HISTOGRAM_ELEMENTS * sizeof(uint32_t));
}


// This function was re-factored into a state machine by KF5N October 29, 2023.
/*****
  Purpose: Called when in CW mode and decoder flag is set. Function assumes:

      dit           = 1
      dah           = dit * 3
      inter-atom    = dit
      inter-letter  = dit * 3
      inter-word    = dit * 7

      You can distinguish between dah and inter-letter by presence/absence of signal. Same for inter-atom.

  Parameter list:
    float audioValue        the strength of audio signal

  Return value;
    void
*****/
// charProcessFlag means a character is being decoded.  blankFlag indicates a blank has already been printed.
bool charProcessFlag, blankFlag;
int currentTime, interElementGap, noSignalTimeStamp;
char *bigMorseCodeTree = (char *)"-EISH5--4--V---3--UF--------?-2--ARL---------.--.WP------J---1--TNDB6--.--X/-----KC------Y------MGZ7----,Q------O-8------9--0----";
FASTRUN void DoCWDecoding(int audioValue) {

  for (int i = 0; i < 2; i = i + 1) {
    switch (decodeStates) {
      // State 0.  Detects start of signal and starts timer.
      case state0:
        // Detect signal and redirect to appropriate state.
        if (audioValue == 1) {
          signalStart = millis();                                                                    // Time stamp beginning of signal.
          gapLength = signalStart - signalEnd;                                                       // Calculate the time gap between the start of this new signal and the end of the last one.
                                                                                                     //        Serial.printf("gapLength state0 = %d\n", gapLength);
          if (gapLength > LOWEST_ATOM_TIME && gapLength < (uint32_t)(thresholdGeometricMean * 3)) {  // range  LOWEST_ATOM_TIME = 20
            DoGapHistogram(gapLength);                                                               // Map the gap in the signal
          }
          //        Serial.printf("gapLength = %d gapChar = %d gapAtom = %d\n", gapLength, gapChar, gapAtom);
          decodeStates = state1;  // Go to "signalStart" state.
          break;                  // Go to state1;
        }
        // audioValue = 0, no signal:
        noSignalTimeStamp = millis();
        interElementGap = noSignalTimeStamp - signalEnd;
        //      if ((interElementGap > ditLength * 1.5) && charProcessFlag) {  // use thresholdGeometricMean??? was ditLength. End of character!  65 * 2
        if ((interElementGap > (gapAtom * 2)) && charProcessFlag) {  // use thresholdGeometricMean??? was ditLength. End of character!  65 * 2
          decodeStates = state3;                                     // Character ended, print it!
          break;
        }
        if (interElementGap > (gapAtom * 5) && not blankFlag && not charProcessFlag) {  // A big gap, print a blank, but don't repeat a blank.  85 * 3.5
          decodeStates = state4;
          break;
        }
        decodeStates = state0;  // Stay in state0; no signal.
        break;                  // End state0
      case state1:              // This state times a signal and measures its duration.  The next state determines if the signal is a dit or a dah.
        if (audioValue == 0) {
          currentTime = millis();
          signalElapsedTime = currentTime - signalStart;  // Calculate the duration of the signal.
          // Ignore short noisy signal bursts:
          if (signalElapsedTime < LOWEST_ATOM_TIME) {  // A hiccup or a real signal?  Make this a fraction of ditLength instead???
            decodeStates = state0;                     // False signal, start over.
            break;
          }
          //        Serial.printf("SET = %d TGM = %f dit = %d dah = %d gapChar = %d gapAtom =%d\n", signalElapsedTime, thresholdGeometricMean, ditLength, dahLength, gapChar, gapAtom);
          if (signalElapsedTime > LOWEST_ATOM_TIME && signalElapsedTime < HISTOGRAM_ELEMENTS) {  // Valid elapsed time?
            DoSignalHistogram(signalElapsedTime);                                                //Yep
          }
          signalEnd = currentTime;  // Time gap to next signal.
          decodeStates = state2;    // Proceed to state2.  A timed signal is available and must be processed.
          break;
        }
        decodeStates = state1;  // Signal still present, stay in state1.
        break;                  // End state1

      case state2:                                                                             // Determine if a timed signal was a dit or a dah and increment the decode tree.
        if (signalElapsedTime > (0.5 * ditLength) && signalElapsedTime < (1.5 * dahLength)) {  // All this does is provide a wide boundary for dit and dah lengths.
          currentDashJump = currentDashJump >> 1;                                              // Fast divide by 2
          if (signalElapsedTime < (int)thresholdGeometricMean) {                               // It was a dit
            charProcessFlag = true;
            currentDecoderIndex++;
          } else {  // It's a dah!
            charProcessFlag = true;
            currentDecoderIndex += currentDashJump;
          }
        }
        decodeStates = state0;  // Begin process again.
        break;                  // End state2
      case state3:
        MorseCharacterDisplay(bigMorseCodeTree[currentDecoderIndex]);  // This always prints.  How do blanks get printed.
        currentDecoderIndex = 0;                                       //Reset everything if char or word
        currentDashJump = DECODER_BUFFER_SIZE;
        charProcessFlag = false;  // Char printed and no longer in progress.
        decodeStates = state0;    // Start process for next incoming character.
        blankFlag = false;
        //      Serial.printf("IEG = %d\n", interElementGap);
        break;      // End state5
      case state4:  //  Blank printing state.
        MorseCharacterDisplay(' ');

        tft.setFontScale((enum RA8875tsize)0);  // Show estimated WPM
        tft.setTextColor(RA8875_GREEN);
        tft.fillRect(DECODER_X + 75, DECODER_Y - 5, tft.getFontWidth() * 3, tft.getFontHeight(), RA8875_BLACK);  // Erase old WPM.
        tft.setCursor(DECODER_X + 75, DECODER_Y - 5);
//        tft.print("(");
        tft.writeTo(L1);
        tft.print(1200L / (dahLength / 3));
        tft.writeTo(L1);
//        tft.print(" WPM)");
        tft.setTextColor(RA8875_WHITE);
        tft.setFontScale((enum RA8875tsize)3);
        //Serial.printf("gapLength = %d thresholdGeometricMean = %f interElementGap = %d\n", gapLength, thresholdGeometricMean, interElementGap);
        blankFlag = true;
        decodeStates = state0;  // Start process for next incoming character.
        break;
      default:
        break;
    }
  }
}


/*****
  Purpose: This function creates a distribution of the gaps between signals, expressed
           in milliseconds. The result is a tri-modal distribution around three timings:
            1. inter-atom time (one dit length)
            2. inter-character (three dit lengths)
            3. word end (seven dit lengths)

  Parameter list:
    long val        the duration of the signal gap (ms)

  Return value;
    void

*****/
FASTRUN void DoGapHistogram(long gapLen) {
  int32_t tempAtom, tempChar;
  int32_t atomIndex, charIndex, firstDit, temp;
  uint32_t offset;

  if (gapHistogram[gapLen] > 10) {  // Need over 1 so we don't have fractional value
    for (int k = 0; k < HISTOGRAM_ELEMENTS; k++) {
      gapHistogram[k] = (uint32_t)(.8 * (float)gapHistogram[k]);
    }
  }

  gapHistogram[gapLen]++;  // Add new signal to distribution

  atomIndex = charIndex = 0;
  if (gapLen <= thresholdGeometricMean) {                                                                                 // Find new dit length
    JackClusteredArrayMax(gapHistogram, (uint32_t)thresholdGeometricMean, &tempAtom, &atomIndex, &firstDit, (int32_t)1);  // Find max dit gap
    if (atomIndex) {                                                                                                      // if something found
      gapAtom = atomIndex;
    }
    for (int j = 0; j < HISTOGRAM_ELEMENTS; j++) {                        // count down
      if (gapHistogram[HISTOGRAM_ELEMENTS - j] > 0 && endGapFlag == 0) {  //Look for non-zero entries in the histogram
        if (HISTOGRAM_ELEMENTS - j < gapAtom * 2) {                       // limit search to probable gapAtom entries
          topGapIndex = HISTOGRAM_ELEMENTS - j;                           //Upper end of gapAtom range
          endGapFlag = 1;                                                 // set flag so we know tha this is the top of the gapAtom range
        }
      }
      if (topGapIndex > 2 * gapAtom) topGapIndex = topGapIndexOld;  // discard outliers
    }
    endGapFlag = 0;                //reset flag
    topGapIndexOld = topGapIndex;  //Keep good value for reference
  } else {                         // dah calculation
    if (gapLen <= thresholdGeometricMean * 2) {
      offset = (uint32_t)(thresholdGeometricMean * 2);  // Find number of elements to check
      JackClusteredArrayMax(&gapHistogram[(int32_t)thresholdGeometricMean + 1], offset, &tempChar, &charIndex, &temp, (int32_t)3);
      if (charIndex)  // if something found
        gapChar = charIndex;
    }
  }
  if (atomIndex) {
    gapAtom = atomIndex;
  }
  if (charIndex) {
    gapChar = charIndex;
  }
}

/*****
  Purpose: This function replaces the arm_max_float32() function that finds the maximum element in an array.
           The histograms are "fuzzy" in the sense that dits and dahs "cluster" around a maximum value rather
           than having a single max value. This algorithm looks at a given cell and the adds in the previous
           (index - 1) and next (index + 1) cells to get the total for that index.

  Parameter list:
    int32_t *array         the base address of the array to search
    int32_t elements       the number of elements of the array to examine
    int32_t *maxCount      the largest clustered value found
    int32_t *maxIndex      the index of the center of the cluster
    int32_t *firstNonZero  the first cell that has a non-zero value
    int32_t clusterSpread  tells how far previous and ahead elements are to be included in the measure.
                            Must be an odd integer > 1.

  Return value;
    void
*****/
void JackClusteredArrayMax(int32_t *array, int32_t elements, int32_t *maxCount, int32_t *maxIndex, int32_t *firstNonZero, int32_t spread) {
  int32_t i, j, clusteredIndex;
  int32_t clusteredMax, temp;

  *maxCount = '\0';  // Reset to empty
  *maxIndex = '\0';

  clusteredMax = 0;
  clusteredIndex = -1;  // Now we can check for an error

  for (i = spread; i < elements - spread; i++) {  // Start with 1 so we can look at the previous element's value
    temp = 0;
    for (j = i - spread; j <= i + spread; j++) {
      temp += array[j];
      ;  // Include adjacent elements
    }

    if (temp >= clusteredMax) {
      clusteredMax = temp;
      clusteredIndex = i;
    }
  }
  if (clusteredIndex > 0) {
    *maxCount = array[clusteredIndex];
    *maxIndex = clusteredIndex;
  }
}
/*****
  Purpose: This function creates a distribution of the dit and dahs lengths, expressed in
  milliseconds. The result is a bi-modal distribution around those two timings. The
  modal value is then used for the timing of the decoder. The range should be between 20
  (60wpm) and 240 (5wpm)

  Parameter list:
  long val        the strength of audio signal

  Return value;
  void

*****/
FASTRUN void DoSignalHistogram(long val) {
  float compareFactor = 2.0;
  int32_t firstNonEmpty;
  int32_t tempDit, tempDah;
  int32_t offset;

  if (valFlag == 0) {
    valRef1 = signalElapsedTime;
    signalStartOld = millis();
    valFlag = 1;
  }

  if (millis() - signalStartOld > LOWEST_ATOM_TIME && valFlag == 1) {
    gapRef1 = gapLength;
    valRef2 = signalElapsedTime;
    valFlag = 0;
  }

  if ((valRef2 >= valRef1 * compareFactor && gapRef1 <= valRef1 * compareFactor)
      || (valRef1 >= valRef2 * compareFactor && gapRef1 <= valRef2 * compareFactor)) {
    // See if consecutive signal lengths in approximate dit to dah ratio and which one is larger
    if (valRef2 >= valRef1) {
      aveDitLength = (long)(0.9 * aveDitLength + 0.1 * valRef1);  //Do some dit length averaging
      aveDahLength = (long)(0.9 * aveDahLength + 0.1 * valRef2);
    } else {
      aveDitLength = (long)(0.9 * aveDitLength + 0.1 * valRef2);  // Use larger one. Note reversal of calc order
      aveDahLength = (long)(0.9 * aveDahLength + 0.1 * valRef1);  // Do some dah length averaging
    }
  }
  thresholdGeometricMean = sqrt(aveDitLength * aveDahLength);    //calculate geometric mean
  thresholdArithmeticMean = (aveDitLength + aveDahLength) >> 1;  // Fast divide by 2 on integer data

  signalHistogram[val]++;  // Don't care which half it's in, just put it in

  offset = (uint32_t)thresholdGeometricMean - 1;  // Only do cast once
  // Dit calculation
  // 2nd parameter means we only look for dits below the geomean.

  for (int32_t j = (int32_t)thresholdGeometricMean; j; j--) {
    if (signalHistogram[j] != 0) {
      firstNonEmpty = j;
      break;
    }
  }

  JackClusteredArrayMax(signalHistogram, offset, &tempDit, (int32_t *)&ditLength, &firstNonEmpty, (int32_t)1);
  // dah calculation
  // Elements above the geomean. Note larger spread: higher variance
  JackClusteredArrayMax(&signalHistogram[offset], HISTOGRAM_ELEMENTS - offset, &tempDah, (int32_t *)&dahLength, &firstNonEmpty, (uint32_t)3);
  dahLength += (uint32_t)offset;

  if (tempDit > SCALE_CONSTANT && tempDah > SCALE_CONSTANT) {  //Adaptive dit signalHistogram[]
    for (int k = 0; k < HISTOGRAM_ELEMENTS; k++) {
      signalHistogram[k] = ADAPTIVE_SCALE_FACTOR * signalHistogram[k];
    }
  }
}

/*****
  Purpose: Calculate Goertzel Algorithn to enable decoding CW

  Parameter list:
    int numSamples,         // number of sample in data array
    int TARGET_FREQUENCY,   // frequency for which the magnitude of the transform is to be found
    int SAMPLING_RATE,      // Sampling rate in our case 24ksps
    float* data             // pointer to input data array

  Return value;
    float magnitude     //magnitude of the transform at the target frequency

*****/
float goertzel_mag(int numSamples, int TARGET_FREQUENCY, int SAMPLING_RATE, float *data) {
  int k, i;
  float floatnumSamples;
  float omega, sine, cosine, coeff, q0, q1, q2, magnitude, real, imag;

  float scalingFactor = numSamples / 2.0;

  floatnumSamples = (float)numSamples;
  k = (int)(0.5 + ((floatnumSamples * TARGET_FREQUENCY) / SAMPLING_RATE));
  omega = (2.0 * M_PI * k) / floatnumSamples;
  sine = sin(omega);
  cosine = cos(omega);
  coeff = 2.0 * cosine;
  q0 = 0;
  q1 = 0;
  q2 = 0;

  for (i = 0; i < numSamples; i++) {
    q0 = coeff * q1 - q2 + data[i];
    q2 = q1;
    q1 = q0;
  }
  real = (q1 - q2 * cosine) / scalingFactor;  // calculate the real and imaginary results scaling appropriately
  imag = (q2 * sine) / scalingFactor;

  magnitude = sqrtf(real * real + imag * imag);
  return magnitude;
}
