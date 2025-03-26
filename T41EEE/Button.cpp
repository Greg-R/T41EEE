
#include "SDT.h"


/*
The button interrupt routine implements a first-order recursive filter, or "leaky integrator,"
as described at:

  https://www.edn.com/a-simple-software-lowpass-filter-suits-embedded-system-applications/

Filter bandwidth is dependent on the sample rate and the "k" parameter, as follows:

                                1 Hz
                          k   Bandwidth   Rise time (samples)
                          1   0.1197      3
                          2   0.0466      8
                          3   0.0217      16
                          4   0.0104      34
                          5   0.0051      69
                          6   0.0026      140
                          7   0.0012      280
                          8   0.0007      561

Thus, the default values below create a filter with 10000 * 0.0217 = 217 Hz bandwidth
*/

static unsigned long buttonFilterRegister;
const uint32_t BUTTON_FILTER_SHIFT = 3;  // Filter parameter k
static uint32_t buttonState, buttonADCPressed, buttonElapsed;
static volatile int buttonADCOut;
const uint32_t BUTTON_FILTER_SAMPLERATE = 10000;  // Hz
const uint32_t BUTTON_DEBOUNCE_DELAY = 5000;      // uSec
const uint32_t BUTTON_STATE_UP = 0;
const uint32_t BUTTON_STATE_DEBOUNCE = 1;
const uint32_t BUTTON_STATE_PRESSED = 2;
const float32_t BUTTON_USEC_PER_ISR = (1000000 / BUTTON_FILTER_SAMPLERATE);
const uint32_t BUTTON_OUTPUT_UP = 1023;  // Value to be output when in the UP state

/*****
  Purpose: ISR to read button ADC and detect button presses

  Parameter list:
    none
  Return value;
    void
*****/
void ButtonISR() {
  int filteredADCValue;

  buttonFilterRegister = buttonFilterRegister - (buttonFilterRegister >> BUTTON_FILTER_SHIFT) + analogRead(BUSY_ANALOG_PIN);
  filteredADCValue = (int)(buttonFilterRegister >> BUTTON_FILTER_SHIFT);

  switch (buttonState) {
    case BUTTON_STATE_UP:
      if (filteredADCValue <= CalData.buttonThresholdPressed) {
        buttonElapsed = 0;
        buttonState = BUTTON_STATE_DEBOUNCE;
      }

      break;
    case BUTTON_STATE_DEBOUNCE:
      if (buttonElapsed < BUTTON_DEBOUNCE_DELAY) {
        buttonElapsed += BUTTON_USEC_PER_ISR;
      } else {
        buttonADCOut = buttonADCPressed = filteredADCValue;
        buttonElapsed = 0;
        buttonState = BUTTON_STATE_PRESSED;
      }

      break;
    case BUTTON_STATE_PRESSED:
      if (filteredADCValue >= CalData.buttonThresholdReleased) {
        buttonState = BUTTON_STATE_UP;
      } else if (CalData.buttonRepeatDelay != 0) {  // buttonRepeatDelay of 0 disables repeat
        if (buttonElapsed < CalData.buttonRepeatDelay) {
          buttonElapsed += BUTTON_USEC_PER_ISR;
        } else {
          buttonADCOut = buttonADCPressed;
          buttonElapsed = 0;
        }
      }

      break;
  }
}


/*****
  Purpose: Starts button IntervalTimer and toggles subsequent button
           functions into interrupt mode.

  Parameter list:
    none
  Return value;
    void
*****/
void Button::EnableButtonInterrupts() {
  buttonADCOut = BUTTON_OUTPUT_UP;
  buttonFilterRegister = buttonADCOut << BUTTON_FILTER_SHIFT;
  buttonState = BUTTON_STATE_UP;
  buttonADCPressed = BUTTON_STATE_UP;
  buttonElapsed = 0;
  buttonInterrupts.begin(ButtonISR, 1000000 / BUTTON_FILTER_SAMPLERATE);
  buttonInterruptsEnabled = true;
}


/*****
  Purpose: Determine which UI button was pressed

  Parameter list:
    int valPin            the ADC value from analogRead()

  Return value;
    int                   -1 if not valid push button, index of push button if valid
*****/
MenuSelect Button::ProcessButtonPress(int valPin) {
  int switchIndex;

  if (valPin == BOGUS_PIN_READ) {  // Not valid press
    return MenuSelect::BOGUS_PIN_READ;
  }

  for (switchIndex = 0; switchIndex < NUMBER_OF_SWITCHES; switchIndex++) {
    if (abs(valPin - CalData.switchValues[switchIndex]) < WIGGLE_ROOM)  // ...because ADC does return exact values every time
    {
      return static_cast<MenuSelect>(switchIndex);
    }
  }

  return MenuSelect::DEFAULT;
}


/*****
  Purpose: Check for UI button press. If pressed, return the ADC value

  Parameter list:
    none

  Return value;
    int                   -1 if not valid push button, ADC value if valid
*****/
int Button::ReadSelectedPushButton() {
  minPinRead = 0;
  int buttonReadOld = 1023;

  if (buttonInterruptsEnabled) {
    noInterrupts();
    buttonRead = buttonADCOut;

    /*
    Clear the button read.  If the button remains pressed, the ISR will reset the value nearly
    instantly.  Clearing the value here rather than in the ISR provides more consistent button
    press "feel" when calls to ReadSelectedPushButton have variable timing.
    */

    buttonADCOut = BUTTON_OUTPUT_UP;
    interrupts();
  } else {
    while (abs(minPinRead - buttonReadOld) > 3) {  // do averaging to smooth out the button response
      minPinRead = analogRead(BUSY_ANALOG_PIN);

      buttonRead = .1 * minPinRead + (1 - .1) * buttonReadOld;  // See expected values in next function.
      buttonReadOld = buttonRead;
    }
  }

  if (buttonRead > CalData.switchValues[0] + WIGGLE_ROOM) {  //AFP 10-29-22 per Jack Wilson
    return -1;
  }
  minPinRead = buttonRead;
  if (!buttonInterruptsEnabled) {
    delay(100L);
  }
  return minPinRead;
}


/*****
  Purpose: Function is designed to route program control to the proper execution point in response to
           a button press.

  Parameter list:
    MenuSelect val               The value from analogRead in loop()

  Return value;
    void
*****/
void Button::ExecuteButtonPress(MenuSelect val) {

  switch (val) {
    case MenuSelect::MENU_OPTION_SELECT:  // 0

      ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      functionPtr[mainMenuIndex]();  // These are processed in MenuProcessing.cpp
      EraseMenus();
      break;

    case MenuSelect::MAIN_MENU_UP:  // 1
      ButtonMenuIncrease();         // This makes sure the increment does go outta range
                                    //      if (menuStatus != NO_MENUS_ACTIVE) {  // Doing primary menu
      ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      //      }
      break;

    case MenuSelect::BAND_UP:  // 2 Now calls ProcessIQData and Encoders calls
      EraseMenus();
      ButtonBandIncrease();
      NCOFreq = 0L;
      break;

    case MenuSelect::ZOOM:  // 3
      ButtonZoom();
      break;

    case MenuSelect::MAIN_MENU_DN:  // 4
      ButtonMenuDecrease();
      ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      break;

    case MenuSelect::BAND_DN:  // 5
      EraseMenus();
      ButtonBandDecrease();
      NCOFreq = 0L;
      break;

    case MenuSelect::FILTER:  // 6
      EraseMenus();
      ButtonFilter();
      break;

    case MenuSelect::DEMODULATION:  // 7
      EraseMenus();
      ButtonSelectSideband();
      break;

    case MenuSelect::SET_MODE:  // 8
      ButtonMode();
      ShowSpectrumdBScale();
      break;

    case MenuSelect::NOISE_REDUCTION:  // 9
      ButtonNR();
      UpdateNoiseField();  // This is required because LMS NR must turn off AutoNotch.
      break;

    case MenuSelect::NOTCH_FILTER:  // 10
      ButtonNotchFilter();
      UpdateNotchField();
      break;

    case MenuSelect::MUTE_AUDIO:  // 11.  Was noise floor.  Greg KF5N February 12, 2025
                                  //      ButtonSetNoiseFloor();
      ButtonMuteAudio();
      break;

    case MenuSelect::FINE_TUNE_INCREMENT:  // 12
      ButtonFineFreqIncrement();
      break;

    case MenuSelect::DECODER_TOGGLE:  // 13
      ConfigData.decoderFlag = not ConfigData.decoderFlag;
      UpdateDecoderField();
      break;

    case MenuSelect::MAIN_TUNE_INCREMENT:  // 14
      ButtonCenterFreqIncrement();
      break;

    case MenuSelect::RESET_TUNING:  // 15   AFP 10-11-22
      ResetTuning();                // AFP 10-11-22
      break;                        // AFP 10-11-22

    case MenuSelect::UNUSED_1:  // 16
      if (calOnFlag == 0) {
        ButtonFrequencyEntry();
      }
      break;

    case MenuSelect::BEARING:  // 17  // AFP 10-11-22
      int doneViewing;
      float retVal;
      MenuSelect menu;

      tft.clearScreen(RA8875_BLACK);

      DrawKeyboard();
      CaptureKeystrokes();
      retVal = BearingHeading(keyboardBuffer);

      if (retVal != -1.0) {  // We have valid country
        bmpDraw((char *)myMapFiles[selectedMapIndex].mapNames, IMAGE_CORNER_X, IMAGE_CORNER_Y);
        doneViewing = false;
      } else {
        tft.setTextColor(RA8875_RED);
        tft.setCursor(380 - (17 * tft.getFontWidth(0)) / 2, 240);  // Center message
        tft.print("Country not found");
        tft.setTextColor(RA8875_WHITE);
      }
      while (true) {
        menu = readButton();                       // Poll UI push buttons
        if (menu != MenuSelect::BOGUS_PIN_READ) {  // If a button was pushed...
          switch (menu) {
            case MenuSelect::BEARING:  // Pressed puchbutton 18
              doneViewing = true;
              break;
            default:
              break;
          }
        }

        if (doneViewing == true) {
          break;
        }
      }
      RedrawDisplayScreen();
      break;

    case MenuSelect::BOGUS_PIN_READ:  // 18

      break;

    default:  // 19

      break;
  }
}


/*****
  Purpose: To process a center tuning increment button push

  Parameter list:
    void

  Return value:
    void
*****/
std::vector<uint32_t>::iterator result;                   // This is also used by fine tuning.  Greg KF5N June 29, 2024
std::vector<uint32_t> centerTuneArray CENTER_TUNE_ARRAY;  // k3pto
void Button::ButtonCenterFreqIncrement() {
  uint32_t index = 0;
  // Find the index of the current fine tune setting.
  result = std::find(centerTuneArray.begin(), centerTuneArray.end(), ConfigData.centerTuneStep);
  index = std::distance(centerTuneArray.begin(), result);
  index++;                                // Increment index.
  if (index == centerTuneArray.size()) {  // Wrap around.
    index = 0;
  }
  ConfigData.centerTuneStep = centerTuneArray[index];
  DisplayIncrementField();
}


/*****
  Purpose: To process a fine tuning increment button push

  Parameter list:
    void

  Return value;
    void
*****/
//std::vector<uint32_t>::iterator result;
std::vector<uint32_t> fineTuneArray FINE_TUNE_ARRAY;  // K3PTO
void Button::ButtonFineFreqIncrement() {
  uint32_t index = 0;
  // Find the index of the current fine tune setting.
  result = std::find(fineTuneArray.begin(), fineTuneArray.end(), ConfigData.fineTuneStep);
  index = std::distance(fineTuneArray.begin(), result);
  index++;                              // Increment index.
  if (index == fineTuneArray.size()) {  // Wrap around.
    index = 0;
  }
  ConfigData.fineTuneStep = fineTuneArray[index];
  DisplayIncrementField();
}


/*****
  Purpose: To process a menu increase button push

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonMenuIncrease() {
  mainMenuIndex++;
  if (mainMenuIndex == TOP_MENU_COUNT) {  // At last menu option, so...
    mainMenuIndex = 0;                    // ...wrap around to first menu option
  }
}


/*****
  Purpose: To process a menu decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonMenuDecrease() {
  mainMenuIndex--;
  if (mainMenuIndex < 0) {               // At last menu option, so...
    mainMenuIndex = TOP_MENU_COUNT - 1;  // ...wrap around to first menu option
  }
}


/*****
  Purpose: To process a band increase button push.

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonBandIncrease() {
  int tempIndex;
  tempIndex = ConfigData.currentBandA;
  if (ConfigData.currentBand == NUMBER_OF_BANDS) {  // Incremented too far?
    ConfigData.currentBand = 0;                     // Yep. Roll to list front.
  }
  NCOFreq = 0;
  switch (ConfigData.activeVFO) {
    case VFO_A:
      tempIndex = ConfigData.currentBandA;
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandA++;
      if (ConfigData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
        ConfigData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandA;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqA = ConfigData.lastFrequencies[ConfigData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      tempIndex = ConfigData.currentBandB;
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandB++;
      if (ConfigData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        ConfigData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandB;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqB = ConfigData.lastFrequencies[ConfigData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
  ExecuteModeChange();
}


/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonBandDecrease() {
  int tempIndex = ConfigData.currentBand;

  ConfigData.currentBand--;  // decrement band index

  if (ConfigData.currentBand < 0) {                // decremented too far?
    ConfigData.currentBand = NUMBER_OF_BANDS - 1;  // Yep. Roll to list end.
  }

  switch (ConfigData.activeVFO) {
    case VFO_A:
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;

          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandA--;
      if (ConfigData.currentBandA == NUMBER_OF_BANDS) {  // decremented too far?
        ConfigData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      if (ConfigData.currentBandA < 0) {                // Incremented too far?
        ConfigData.currentBandA = NUMBER_OF_BANDS - 1;  // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandA;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqA = ConfigData.lastFrequencies[ConfigData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;

          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandB--;
      if (ConfigData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        ConfigData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      if (ConfigData.currentBandB < 0) {                // Incremented too far?
        ConfigData.currentBandB = NUMBER_OF_BANDS - 1;  // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandB;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqB = ConfigData.lastFrequencies[ConfigData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
  ExecuteModeChange();
}


/*****
  Purpose: Set the radio to a band using the band parameter.  This function is probably obsolete thanks to the new modal states.

  Parameter list:
    int band

  Return value:
    void
*****/
void Button::BandSet(int band) {
  int tempIndex;
  tempIndex = ConfigData.currentBandA;
  if (ConfigData.currentBand == NUMBER_OF_BANDS) {  // Incremented too far?
    ConfigData.currentBand = 0;                     // Yep. Roll to list front.
  }
  NCOFreq = 0L;
  switch (ConfigData.activeVFO) {
    case VFO_A:
      tempIndex = ConfigData.currentBandA;
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandA = band;
      if (ConfigData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
        ConfigData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandA;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqA = ConfigData.lastFrequencies[ConfigData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      tempIndex = ConfigData.currentBandB;
      if (save_last_frequency == 1) {
        ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              ConfigData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      ConfigData.currentBandB = band;
      if (ConfigData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        ConfigData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      ConfigData.currentBand = ConfigData.currentBandB;
      ConfigData.centerFreq = TxRxFreq = ConfigData.currentFreqB = ConfigData.lastFrequencies[ConfigData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
}


/***** AFP 09-27-22
  Purpose: Chnage the horizontal scale of the frequency display

  Parameter list:
    void

  Return value:
    int             index of the option selected
*****/
void Button::ButtonZoom() {
  zoomIndex++;

  if (zoomIndex == MAX_ZOOM_ENTRIES) {
    zoomIndex = 0;
  }
  if (zoomIndex <= 0)
    ConfigData.spectrum_zoom = 0;
  else
    ConfigData.spectrum_zoom = zoomIndex;
  ZoomFFTPrep();
  UpdateZoomField();
  tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Always exit function in L1.  KF5N August 15, 2023
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  ShowFrequency();
  ResetTuning();  // AFP 10-11-22
  FilterSetSSB();
}


/*****
  Purpose: To process a filter button push.  This switches the Filter Encoder from the lower limit to the upper limit
           of the audio filter.

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonFilter() {
  switchFilterSideband = not switchFilterSideband;
  FilterSetSSB();  // Call this so the delimiter is set to the correct color.
}


/*****
  Purpose: Change LSB, USB, but only in CW or SSB modes.

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonSelectSideband() {

  switch (bands.bands[ConfigData.currentBand].sideband) {
    case Sideband::LOWER:  // Switch to USB.
      bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;
      ConfigData.lastSideband[ConfigData.currentBand] = Sideband::UPPER;

      break;
    case Sideband::UPPER:  // Switch to LSB.
      // Leave FT8 in USB.
      if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {
        bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;
        return;
      }
      bands.bands[ConfigData.currentBand].sideband = Sideband::LOWER;
      ConfigData.lastSideband[ConfigData.currentBand] = Sideband::LOWER;

      break;
    case Sideband::BOTH_AM:
    case Sideband::BOTH_SAM:
      //  If already in AM or SAM mode, don't need to alter sidebands.  Demod selection already setup sidebands.
      return;
      break;
    default:
      break;
  }

  ExecuteModeChange();
}


/*****
  Purpose: Set radio mode for SSB, CW, FT8 or AM receive modes.

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonMode()  //  Greg KF5N March 11, 2025
{
  // Toggle modes:
  switch (bands.bands[ConfigData.currentBand].mode) {

    case RadioMode::CW_MODE:  // Toggle the current mode
      bands.bands[ConfigData.currentBand].mode = RadioMode::SSB_MODE;
      radioState = RadioState::SSB_RECEIVE_STATE;
      bands.bands[ConfigData.currentBand].mode = RadioMode::SSB_MODE;
      bands.bands[ConfigData.currentBand].sideband = ConfigData.lastSideband[ConfigData.currentBand];
      break;
    case RadioMode::SSB_MODE:
      bands.bands[ConfigData.currentBand].mode = RadioMode::FT8_MODE;
      radioState = RadioState::FT8_RECEIVE_STATE;
      bands.bands[ConfigData.currentBand].mode = RadioMode::FT8_MODE;
      bands.bands[ConfigData.currentBand].sideband = Sideband::UPPER;  // FT8 always USB.
      break;
    case RadioMode::FT8_MODE:  // Toggle the current mode
      bands.bands[ConfigData.currentBand].mode = RadioMode::AM_MODE;
      radioState = RadioState::AM_RECEIVE_STATE;
      bands.bands[ConfigData.currentBand].mode = RadioMode::AM_MODE;
      bands.bands[ConfigData.currentBand].sideband = Sideband::BOTH_AM;
      break;
    case RadioMode::AM_MODE:  // Toggle the current mode
      bands.bands[ConfigData.currentBand].mode = RadioMode::SAM_MODE;
      radioState = RadioState::SAM_RECEIVE_STATE;
      bands.bands[ConfigData.currentBand].mode = RadioMode::SAM_MODE;
      bands.bands[ConfigData.currentBand].sideband = Sideband::BOTH_SAM;
      break;
    case RadioMode::SAM_MODE:  // Toggle the current mode
      bands.bands[ConfigData.currentBand].mode = RadioMode::CW_MODE;
      radioState = RadioState::CW_RECEIVE_STATE;
      bands.bands[ConfigData.currentBand].mode = RadioMode::CW_MODE;
      bands.bands[ConfigData.currentBand].sideband = ConfigData.lastSideband[ConfigData.currentBand];
      break;
    default:
      break;
  }
}


/*****
  Purpose: To process select noise reduction

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonNR()  //AFP 09-19-22 update
{
  ConfigData.nrOptionSelect++;
  if (ConfigData.nrOptionSelect > 3) {
    ConfigData.nrOptionSelect = 0;
  }
  if (ConfigData.nrOptionSelect == 3) ANR_notch = false;  // Turn off AutoNotch if LMS NR is selected.
  UpdateNoiseField();
}


/*****
  Purpose: To set the notch filter

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonNotchFilter() {
  ANR_notch = not ANR_notch;
  //  If the notch is activated and LMS NR is also active, turn off NR and update display.
  if (ANR_notch && ConfigData.nrOptionSelect == 3) {
    ConfigData.nrOptionSelect = 0;  // Turn off noise reduction.  Other NR selections will be valid.
    UpdateNoiseField();
  }
}


/*****
  Purpose: Mute speaker, headphone, or both with button pushes.  Greg KF5N February 12, 2025.

  Parameter list:
    void

  Return value:
    void
*****/
void Button::ButtonMuteAudio() {
  switch (ConfigData.audioOut) {
    case AudioState::SPEAKER:         // Speaker is on, mute and unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);  //  Speaker mute
      sgtl5000_1.unmuteHeadphone();   // Make the headphone output active.
      ConfigData.audioOut = AudioState::HEADPHONE;
      break;
    case AudioState::HEADPHONE:       // Mute both speaker and headphones.
      digitalWrite(MUTE, MUTEAUDIO);  // Speaker unmute
      sgtl5000_1.muteHeadphone();     // Mute headphonts
      ConfigData.audioOut = AudioState::MUTE_BOTH;
      break;
    case AudioState::MUTE_BOTH:         //  Unmute both.
      digitalWrite(MUTE, UNMUTEAUDIO);  //  Mute Speaker
      sgtl5000_1.unmuteHeadphone();     // Make the headphone output active.
      ConfigData.audioOut = AudioState::BOTH;
      break;
    case AudioState::BOTH:              //  Headphones mute, speaker on
      digitalWrite(MUTE, UNMUTEAUDIO);  //  Unmute Speaker
      sgtl5000_1.muteHeadphone();       //  Mute headphone
      ConfigData.audioOut = AudioState::SPEAKER;
      break;
    default:
      return;
      break;
  }
  UpdateAudioField();
}


/*****
  Purpose: Draw in a red line at the new floor position

  Parameter list:
    int floor     the pixel position for the new floor

  Return value;
    int           the current noise floor value
*****/
int Button::DrawNewFloor(int floor) {
  static int oldY = SPECTRUM_BOTTOM;

  if (floor < 0) {
    floor = 0;
    oldY = SPECTRUM_BOTTOM - floor;
    return floor;
  }

  tft.drawFastHLine(SPECTRUM_LEFT_X + 30, oldY - floor, 100, RA8875_BLACK);
  tft.drawFastHLine(SPECTRUM_LEFT_X + 30, oldY - floor - 1, 100, RA8875_BLACK);
  tft.drawFastHLine(SPECTRUM_LEFT_X + 30, oldY - floor, 100, RA8875_RED);
  tft.drawFastHLine(SPECTRUM_LEFT_X + 30, oldY - floor - 1, 100, RA8875_RED);
  oldY = SPECTRUM_BOTTOM - floor;
  return floor;
}


/*****
  Purpose: Reset Zoom to zoomIndex

  Parameter list:
    void

  Return value;
    int           the current noise floor value
*****/
void Button::ResetZoom(int zoomIndex1) {
  if (zoomIndex1 == MAX_ZOOM_ENTRIES) {
    zoomIndex1 = 0;
  }
  if (zoomIndex1 <= 0)
    ConfigData.spectrum_zoom = 0;
  else
    ConfigData.spectrum_zoom = zoomIndex1;

  ZoomFFTPrep();
  UpdateZoomField();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  ShowFrequency();
}


/*****
  Purpose: Direct Frequency Entry

  Parameter list:
    void

  Return value;
    void
    Base Code courtesy of Harry  GM3RVL
*****/
void Button::ButtonFrequencyEntry() {
  bool valid_frequency = false;
  TxRxFreqOld = TxRxFreq;

#define show_FEHelp
  bool doneFE = false;                         // set to true when a valid frequency is entered
  long enteredF = 0L;                          // desired frequency
  char strF[6] = { ' ', ' ', ' ', ' ', ' ' };  // container for frequency string during entry
  String stringF;
  MenuSelect menu = MenuSelect::DEFAULT;
  int key;
  int numdigits = 0;  // number of digits entered
  int pushButtonSwitchIndex;
  // Arrays for allocating values associated with keys and switches - choose whether USB keypad or analogue switch matrix
  // USB keypad and analogue switch matrix
  const char *DE_Band[] = { "80m", "40m", "20m", "17m", "15m", "12m", "10m" };
  const char *DE_Flimit[] = { "4.5", "9", "16", "26", "26", "30", "30" };
  int numKeys[] = { 0x0D, 0x7F, 0x58,  // values to be allocated to each key push
                    0x37, 0x38, 0x39,
                    0x34, 0x35, 0x36,
                    0x31, 0x32, 0x33,
                    0x30, 0x7F, 0x7F,
                    0x7F, 0x7F, 0x99 };
  EraseMenus();
#ifdef show_FEHelp
  int keyCol[] = { YELLOW, RED, RED,
                   RA8875_BLUE, RA8875_GREEN, RA8875_GREEN,
                   RA8875_BLUE, RA8875_BLUE, RA8875_BLUE,
                   RED, RED, RED,
                   RED, RA8875_BLACK, RA8875_BLACK,
                   YELLOW, YELLOW, RA8875_BLACK };
  int textCol[] = { RA8875_BLACK, RA8875_WHITE, RA8875_WHITE,
                    RA8875_WHITE, RA8875_BLACK, RA8875_BLACK,
                    RA8875_WHITE, RA8875_WHITE, RA8875_WHITE,
                    RA8875_WHITE, RA8875_WHITE, RA8875_WHITE,
                    RA8875_WHITE, RA8875_WHITE, RA8875_WHITE,
                    RA8875_BLACK, RA8875_BLACK, RA8875_WHITE };
  const char *key_labels[] = { "<", "", "X",
                               "7", "8", "9",
                               "4", "5", "6",
                               "1", "2", "3",
                               "0", "D", "",
                               "", "", "S" };

#define KEYPAD_LEFT 350
#define KEYPAD_TOP SPECTRUM_TOP_Y + 35
#define KEYPAD_WIDTH 150
#define KEYPAD_HEIGHT 300
#define BUTTONS_LEFT KEYPAD_LEFT + 30
#define BUTTONS_TOP KEYPAD_TOP + 30
#define BUTTONS_SPACE 45
#define BUTTONS_RADIUS 15
#define TEXT_OFFSET -8

  tft.writeTo(L1);
  tft.fillRect(WATERFALL_LEFT_X, SPECTRUM_TOP_Y + 1, MAX_WATERFALL_WIDTH, WATERFALL_BOTTOM - SPECTRUM_TOP_Y, RA8875_BLACK);  // Make space for FEInfo
  tft.fillRect(MAX_WATERFALL_WIDTH, WATERFALL_TOP_Y - 10, 15, 30, RA8875_BLACK);
  tft.writeTo(L2);

  tft.fillRect(WATERFALL_LEFT_X, SPECTRUM_TOP_Y + 1, MAX_WATERFALL_WIDTH, WATERFALL_BOTTOM - SPECTRUM_TOP_Y, RA8875_BLACK);

  tft.setCursor(centerLine - 140, WATERFALL_TOP_Y);
  tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, 360, RA8875_YELLOW);  // Spectrum box

  // Draw keypad box
  tft.fillRect(KEYPAD_LEFT, KEYPAD_TOP, KEYPAD_WIDTH, KEYPAD_HEIGHT, DARKGREY);
  // put some circles
  tft.setFontScale((enum RA8875tsize)1);
  for (unsigned i = 0; i < 6; i++) {
    for (unsigned j = 0; j < 3; j++) {
      tft.fillCircle(BUTTONS_LEFT + j * BUTTONS_SPACE, BUTTONS_TOP + i * BUTTONS_SPACE, BUTTONS_RADIUS, keyCol[j + 3 * i]);
      tft.setCursor(BUTTONS_LEFT + j * BUTTONS_SPACE + TEXT_OFFSET, BUTTONS_TOP + i * BUTTONS_SPACE - 18);
      tft.setTextColor(textCol[j + 3 * i]);
      tft.print(key_labels[j + 3 * i]);
    }
  }
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 50);
  tft.setTextColor(RA8875_WHITE);
  tft.print("Direct Frequency Entry");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 100);
  tft.print("<   Apply entered frequency");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 130);
  tft.print("X   Exit without changing frequency");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 160);
  tft.print("D   Delete last digit");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 190);
  tft.print("S   Save Direct to Last Freq. ");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 240);
  tft.print("Direct Entry was called from ");
  tft.print(DE_Band[ConfigData.currentBand]);
  tft.print(" band");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 270);
  tft.print("Frequency response limited above ");
  tft.print(DE_Flimit[ConfigData.currentBand]);
  tft.print("MHz");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 300);
  tft.print("For widest direct entry frequency range");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 330);
  tft.print("call from 12m or 10m band");

#endif

  tft.writeTo(L2);

  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(10, 0);
  tft.print("Enter Frequency");

  tft.fillRect(SECONDARY_MENU_X + 20, MENUS_Y, EACH_MENU_WIDTH + 10, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_BLACK);  // JJP 7/17/23
  tft.setCursor(SECONDARY_MENU_X + 21, MENUS_Y + 1);
  tft.print("kHz or MHz:");
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(WATERFALL_LEFT_X + 50, SPECTRUM_TOP_Y + 260);
  tft.print("Save Direct to Last Freq.= ");
  tft.setCursor(WATERFALL_LEFT_X + 270, SPECTRUM_TOP_Y + 190);
  if (save_last_frequency == 0) {
    tft.setTextColor(RA8875_MAGENTA);
    tft.print("Off");
  } else {
    if (save_last_frequency == 1) {
      tft.setTextColor(RA8875_GREEN);
      tft.print("On");
    }
  }

  while (doneFE == false) {
    menu = readButton();
    pushButtonSwitchIndex = static_cast<int>(menu);
    if (pushButtonSwitchIndex != 18) {
      key = numKeys[pushButtonSwitchIndex];
      switch (key) {
        case 0x7F:  // erase last digit =127
          if (numdigits != 0) {
            numdigits--;
            strF[numdigits] = ' ';
          }
          break;
        case 0x58:  // Exit without updating frequency =88
          doneFE = true;
          valid_frequency = false;
          break;
        case 0x0D:  // Apply the entered frequency (if valid) =13
          stringF = String(strF);
          enteredF = stringF.toInt();
          if ((numdigits == 1) || (numdigits == 2)) {
            enteredF = enteredF * 1000000;
            valid_frequency = true;
          }
          if ((numdigits == 4) || (numdigits == 5)) {
            enteredF = enteredF * 1000;
            valid_frequency = true;
          }
          if ((enteredF > 30000000) || (enteredF < 1250000)) {
            stringF = "     ";  // 5 spaces
            stringF.toCharArray(strF, stringF.length());
            numdigits = 0;
            valid_frequency = false;
          } else {
            doneFE = true;
          }
          break;
        case 0x99:  // Toggle "Save Direct to Last Freq".
          save_last_frequency = not save_last_frequency;
          tft.setFontScale((enum RA8875tsize)0);
          tft.fillRect(WATERFALL_LEFT_X + 269, SPECTRUM_TOP_Y + 190, 50, CHAR_HEIGHT, RA8875_BLACK);
          tft.setCursor(WATERFALL_LEFT_X + 260, SPECTRUM_TOP_Y + 190);
          if (save_last_frequency == false) {
            tft.setTextColor(RA8875_MAGENTA);
            tft.print("Off");
            break;
          } else {
            tft.setTextColor(RA8875_GREEN);
            tft.print("On");
          }
          break;
        default:
          if ((numdigits == 5) || ((key == 0x30) & (numdigits == 0))) {
          } else {
            strF[numdigits] = char(key);
            numdigits++;
          }
          break;
      }
      tft.setTextColor(RA8875_WHITE);
      tft.setFontScale((enum RA8875tsize)1);
      tft.fillRect(SECONDARY_MENU_X + 195, MENUS_Y + 1, 85, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
      tft.print(strF);
      delay(250);  // only for analogue switch matrix
    }
  }
  if (key != 0x58) {
    TxRxFreq = enteredF;
  }
  NCOFreq = 0L;
  directFreqFlag = 1;
  ConfigData.centerFreq = TxRxFreq;
  centerTuneFlag = 1;  // Put back in so tuning bar is refreshed.  KF5N July 31, 2023
  SetFreq();           // Used here instead of centerTuneFlag.  KF5N July 22, 2023
  if (save_last_frequency == true and valid_frequency == true) {
    ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO] = enteredF;
  } else {
    if (save_last_frequency == 0) {
      ConfigData.lastFrequencies[ConfigData.currentBand][ConfigData.activeVFO] = TxRxFreqOld;
    }
  }
  tft.fillRect(0, 0, 799, 479, RA8875_BLACK);  // Clear layer 2  JJP 7/23/23
  tft.writeTo(L1);
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  DrawFrequencyBarValue();
  //  SetBand();
  SetFreq();
  ShowFrequency();
  ShowSpectrumdBScale();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  RedrawDisplayScreen();  // KD0RC
  FilterSetSSB();
}


/*****
  Purpose: Execute mode change (including sideband change).
           Consolidation of all functions required to update DSP and graphics
           done in correct order.

  Parameter list:
    void

  Return value;
    void
*****/
void Button::ExecuteModeChange() {
  tft.writeTo(L2);  // Destroy the bandwidth indicator bar.  KF5N July 30, 2023
  tft.clearMemory();
  tft.writeTo(L1);
  FilterSetSSB();
  FilterBandwidth();
  ShowBandwidth();
  ShowFrequency();
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();  // Restore the bandwidth indicator bar.  KF5N July 30, 2023
  UpdateDecoderField();         // KF5N December 28 2023.
  BandInformation();
  fftOffset = 140;
  Serial.printf("Execute Mode Change\n");
  SetBandRelay();  // Set relays in LPF for current band.
  SetFreq();       // Must update frequency, for example moving from SSB to CW, the RX LO is shifted.  KF5N
  powerUp = true;
}