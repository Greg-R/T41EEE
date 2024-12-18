
#include "SDT.h"

int buttonRead = 0;
int minPinRead = 1024;
int secondaryMenuChoiceMade;
void ButtonFineFreqIncrement();
void ButtonCenterFreqIncrement();

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

const auto BUTTON_FILTER_SAMPLERATE = 10000;  // Hz
const auto BUTTON_FILTER_SHIFT = 3;           // Filter parameter k
const auto BUTTON_DEBOUNCE_DELAY = 5000;      // uSec

const auto BUTTON_STATE_UP = 0;
const auto BUTTON_STATE_DEBOUNCE = 1;
const auto BUTTON_STATE_PRESSED = 2;

const auto BUTTON_USEC_PER_ISR = (1000000 / BUTTON_FILTER_SAMPLERATE);

const auto BUTTON_OUTPUT_UP = 1023;  // Value to be output when in the UP state

IntervalTimer buttonInterrupts;
bool buttonInterruptsEnabled = false;
static unsigned long buttonFilterRegister;
static int buttonState, buttonADCPressed, buttonElapsed;
static volatile int buttonADCOut;


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
      if (filteredADCValue <= EEPROMData.buttonThresholdPressed) {
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
      if (filteredADCValue >= EEPROMData.buttonThresholdReleased) {
        buttonState = BUTTON_STATE_UP;
      } else if (EEPROMData.buttonRepeatDelay != 0) {  // buttonRepeatDelay of 0 disables repeat
        if (buttonElapsed < EEPROMData.buttonRepeatDelay) {
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
void EnableButtonInterrupts() {
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
MenuSelect ProcessButtonPress(int valPin) {
  int switchIndex;

  if (valPin == BOGUS_PIN_READ) {  // Not valid press
    return MenuSelect::BOGUS_PIN_READ;
  }

  for (switchIndex = 0; switchIndex < NUMBER_OF_SWITCHES; switchIndex++) {
    if (abs(valPin - EEPROMData.switchValues[switchIndex]) < WIGGLE_ROOM)  // ...because ADC does return exact values every time
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
int ReadSelectedPushButton() {
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

  if (buttonRead > EEPROMData.switchValues[0] + WIGGLE_ROOM) {  //AFP 10-29-22 per Jack Wilson
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
    int vsl               the value from analogRead in loop()

  Return value;
    void
*****/
void ExecuteButtonPress(MenuSelect val) {

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
      if (EEPROMData.currentBand < 5) digitalWrite(bandswitchPins[EEPROMData.currentBand], LOW);  // Added if so unused GPOs will not be touched.  KF5N October 16, 2023.
      ButtonBandIncrease();
      if (EEPROMData.currentBand < 5) digitalWrite(bandswitchPins[EEPROMData.currentBand], HIGH);
      EEPROMData.rfGainCurrent = 10;
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
      SetFreq();
      ShowSpectrum();
      UpdateDecoderField();
      ShowAutoStatus();
      break;

    case MenuSelect::ZOOM:  // 3
      EraseMenus();
      ButtonZoom();
      break;

    case MenuSelect::MAIN_MENU_DN:  // 4
      ButtonMenuDecrease();
      ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      break;

    case MenuSelect::BAND_DN:  // 5
      EraseMenus();
      ShowSpectrum();  //Now calls ProcessIQData and Encoders calls
      if (EEPROMData.currentBand < 5) digitalWrite(bandswitchPins[EEPROMData.currentBand], LOW);
      ButtonBandDecrease();
      if (EEPROMData.currentBand < 5) digitalWrite(bandswitchPins[EEPROMData.currentBand], HIGH);
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  //AFP 10-20-22
      UpdateDecoderField();
      ShowAutoStatus();
      break;

    case MenuSelect::FILTER:  // 6
      EraseMenus();
      ButtonFilter();
      break;

    case MenuSelect::DEMODULATION:  // 7
      EraseMenus();
      ButtonDemodMode();
      break;

    case MenuSelect::SET_MODE:  // 8
      ButtonMode();
      ShowSpectrumdBScale();
      break;

    case MenuSelect::NOISE_REDUCTION:  // 9
      ButtonNR();
      UpdateNotchField();  // This is required because LMS NR must turn off AutoNotch.
      break;

    case MenuSelect::NOTCH_FILTER:  // 10
      ButtonNotchFilter();
      UpdateNotchField();
      break;

    case MenuSelect::NOISE_FLOOR:  // 11
      ButtonSetNoiseFloor();
      break;

    case MenuSelect::FINE_TUNE_INCREMENT:  // 12
      ButtonFineFreqIncrement();
      break;

    case MenuSelect::DECODER_TOGGLE:  // 13
      EEPROMData.decoderFlag = !EEPROMData.decoderFlag;
      if ((EEPROMData.xmtMode == RadioMode::CW_MODE) && (EEPROMData.decoderFlag == 1)) {
        radioMode = RadioMode::CW_MODE;
      }
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
                                                   //        delay(100L);
        if (menu != MenuSelect::BOGUS_PIN_READ) {  // If a button was pushed...
                                                   //          menu = ProcessButtonPress(valPin);  // Winner, winner...chicken dinner!
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
      ShowFrequency();
      DrawFrequencyBarValue();

      break;

    case MenuSelect::BOGUS_PIN_READ:   // 18

    break;

    default:                           // 19

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
void ButtonCenterFreqIncrement() {
  uint32_t index = 0;
  // Find the index of the current fine tune setting.
  result = std::find(centerTuneArray.begin(), centerTuneArray.end(), EEPROMData.centerTuneStep);
  index = std::distance(centerTuneArray.begin(), result);
  index++;                                // Increment index.
  if (index == centerTuneArray.size()) {  // Wrap around.
    index = 0;
  }
  EEPROMData.centerTuneStep = centerTuneArray[index];
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
void ButtonFineFreqIncrement() {
  uint32_t index = 0;
  // Find the index of the current fine tune setting.
  result = std::find(fineTuneArray.begin(), fineTuneArray.end(), EEPROMData.fineTuneStep);
  index = std::distance(fineTuneArray.begin(), result);
  index++;                              // Increment index.
  if (index == fineTuneArray.size()) {  // Wrap around.
    index = 0;
  }
  EEPROMData.fineTuneStep = fineTuneArray[index];
  DisplayIncrementField();
}
