// ButtonMenuIncrease
// ButtonMenuDecrease
// ButtonBandIncrease
// ButtonBandDecrease
// BandSet
// ButtonZoom
// ButtonFilter
// ButtonDemodMode
// ButtonMode

// ResetZoom
// ButtonFrequencyEntry

#include "SDT.h"

#define TOP_MENU_COUNT 11  // Menus to process AFP 09-27-22, JJP 7-8-23

bool save_last_frequency = false;  // Make this the default behavior.  Greg KF5N October 16, 2024.
int directFreqFlag = 0;
auto subMenuMaxOptions = 0;  // Holds the number of submenu options.
auto TxRxFreqOld = 0;


/*****
  Purpose: To process a menu increase button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonMenuIncrease() {
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
void ButtonMenuDecrease() {
  mainMenuIndex--;
  if (mainMenuIndex < 0) {               // At last menu option, so...
    mainMenuIndex = TOP_MENU_COUNT - 1;  // ...wrap around to first menu option
  }
}


/*****
  Purpose: To process a band increase button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonBandIncrease() {
  int tempIndex;
  tempIndex = EEPROMData.currentBandA;
  if (EEPROMData.currentBand == NUMBER_OF_BANDS) {  // Incremented too far?
    EEPROMData.currentBand = 0;                     // Yep. Roll to list front.
  }
  NCOFreq = 0L;
  switch (EEPROMData.activeVFO) {
    case VFO_A:
      tempIndex = EEPROMData.currentBandA;
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandA++;
      if (EEPROMData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
        EEPROMData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandA;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqA = EEPROMData.lastFrequencies[EEPROMData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      tempIndex = EEPROMData.currentBandB;
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandB++;
      if (EEPROMData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        EEPROMData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandB;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqB = EEPROMData.lastFrequencies[EEPROMData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  SetBand();
  SetFreq();
  ShowFrequency();
  ShowSpectrumdBScale();
  AudioInterrupts();
  eeprom.EEPROMWrite();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  UpdateDecoderField();
  FilterSetSSB();
}


/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonBandDecrease() {
  int tempIndex = EEPROMData.currentBand;

  EEPROMData.currentBand--;  // decrement band index

  if (EEPROMData.currentBand < 0) {                // decremented too far?
    EEPROMData.currentBand = NUMBER_OF_BANDS - 1;  // Yep. Roll to list end.
  }

  switch (EEPROMData.activeVFO) {
    case VFO_A:
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;

          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandA--;
      if (EEPROMData.currentBandA == NUMBER_OF_BANDS) {  // decremented too far?
        EEPROMData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      if (EEPROMData.currentBandA < 0) {                // Incremented too far?
        EEPROMData.currentBandA = NUMBER_OF_BANDS - 1;  // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandA;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqA = EEPROMData.lastFrequencies[EEPROMData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;

          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandB--;
      if (EEPROMData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        EEPROMData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      if (EEPROMData.currentBandB < 0) {                // Incremented too far?
        EEPROMData.currentBandB = NUMBER_OF_BANDS - 1;  // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandB;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqB = EEPROMData.lastFrequencies[EEPROMData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  SetBand();
  SetFreq();
  ShowFrequency();
  ShowSpectrumdBScale();
  AudioInterrupts();
  eeprom.EEPROMWrite();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  UpdateDecoderField();
  FilterSetSSB();
}


/*****
  Purpose: Set the radio to a band using the band parameter.

  Parameter list:
    int band

  Return value:
    void
*****/
void BandSet(int band) {
  int tempIndex;
  tempIndex = EEPROMData.currentBandA;
  if (EEPROMData.currentBand == NUMBER_OF_BANDS) {  // Incremented too far?
    EEPROMData.currentBand = 0;                     // Yep. Roll to list front.
  }
  NCOFreq = 0L;
  switch (EEPROMData.activeVFO) {
    case VFO_A:
      tempIndex = EEPROMData.currentBandA;
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_A] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandA = band;
      if (EEPROMData.currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
        EEPROMData.currentBandA = 0;                     // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandA;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqA = EEPROMData.lastFrequencies[EEPROMData.currentBandA][VFO_A] + NCOFreq;
      break;

    case VFO_B:
      tempIndex = EEPROMData.currentBandB;
      if (save_last_frequency == 1) {
        EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
      } else {
        if (save_last_frequency == 0) {
          if (directFreqFlag == 1) {
            EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreqOld;
          } else {
            if (directFreqFlag == 0) {
              EEPROMData.lastFrequencies[tempIndex][VFO_B] = TxRxFreq;
            }
          }
          TxRxFreqOld = TxRxFreq;
        }
      }
      EEPROMData.currentBandB = band;
      if (EEPROMData.currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
        EEPROMData.currentBandB = 0;                     // Yep. Roll to list front.
      }
      EEPROMData.currentBand = EEPROMData.currentBandB;
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqB = EEPROMData.lastFrequencies[EEPROMData.currentBandB][VFO_B] + NCOFreq;
      break;

    case VFO_SPLIT:
      DoSplitVFO();
      break;
  }
  directFreqFlag = 0;
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  SetBand();
  SetFreq();
  ShowFrequency();
  ShowSpectrumdBScale();
  AudioInterrupts();
  eeprom.EEPROMWrite();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  UpdateDecoderField();
  FilterSetSSB();
}


//================ AFP 09-27-22
/*****
  Purpose: Chnage the horizontal scale of the frequency display

  Parameter list:
    void

  Return value:
    int             index of the option selected
*****/
void ButtonZoom() {
  zoomIndex++;

  if (zoomIndex == MAX_ZOOM_ENTRIES) {
    zoomIndex = 0;
  }
  if (zoomIndex <= 0)
    EEPROMData.spectrum_zoom = 0;
  else
    EEPROMData.spectrum_zoom = zoomIndex;
  ZoomFFTPrep();
  UpdateZoomField();
  tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);  // Always exit function in L1.  KF5N August 15, 2023
  DrawBandWidthIndicatorBar();
  ShowSpectrumdBScale();
  DrawFrequencyBarValue();
  ShowFrequency();
  ShowBandwidth();
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
void ButtonFilter() {
  switchFilterSideband = not switchFilterSideband;
  FilterSetSSB();  // Call this so the delimiter is set to the correct color.
  ControlFilterF();
  FilterBandwidth();
  ShowFrequency();
}


/*****
  Purpose: Process demodulation mode

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonDemodMode() {
  bands[EEPROMData.currentBand].mode++;
  if (bands[EEPROMData.currentBand].mode > DEMOD_MAX) {
    bands[EEPROMData.currentBand].mode = DEMOD_MIN;  // cycle thru demod modes
  }
  BandInformation();
  SetupMode(bands[EEPROMData.currentBand].mode);
  ShowFrequency();
  ControlFilterF();
  tft.writeTo(L2);  // Destroy the bandwidth indicator bar.  KF5N July 30, 2023
  tft.clearMemory();
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();  // Restory the bandwidth indicator bar.  KF5N July 30, 2023
  FilterBandwidth();
  DrawSMeterContainer();
  AudioInterrupts();
  SetFreq();                                                                                              // Must update frequency, for example moving from SSB to CW, the RX LO is shifted.  KF5N
  if ((EEPROMData.xmtMode == RadioMode::CW_MODE) && (EEPROMData.decoderFlag == 1)) {
  radioMode = RadioMode::CW_MODE;
  UpdateDecoderField();  // KF5N December 28 2023.
}
  FilterSetSSB();
}


/*****
  Purpose: Set radio mode for SSB or CW.

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonMode()  //====== Changed AFP 10-05-22  =================
{
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) {  // Toggle the current mode
    EEPROMData.xmtMode = RadioMode::SSB_MODE;
    radioMode = RadioMode::SSB_MODE;
  } else {
    EEPROMData.xmtMode = RadioMode::CW_MODE;
    radioMode = RadioMode::CW_MODE;
  }
  SetFreq();  // Required due to RX LO shift from CW to SSB modes.  KF5N
  DrawSpectrumDisplayContainer();
  DrawFrequencyBarValue();
  DrawInfoWindowFrame();
  DisplayIncrementField();
  AGCPrep();
  UpdateAGCField();
  EncoderVolume();
  UpdateInfoWindow();
  ControlFilterF();
  BandInformation();
  FilterBandwidth();
  DrawSMeterContainer();
  DrawAudioSpectContainer();
  SpectralNoiseReductionInit();
  UpdateNoiseField();
  ShowSpectrumdBScale();
  ShowTransmitReceiveStatus();
  ShowFrequency();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  if (EEPROMData.xmtMode == RadioMode::SSB_MODE) {
    tft.writeTo(L2);
    tft.clearMemory();
  } else BandInformation();
  DrawBandWidthIndicatorBar();
  FilterSetSSB();
}


/*****
  Purpose: To process select noise reduction

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonNR()  //AFP 09-19-22 update
{
  EEPROMData.nrOptionSelect++;
  if (EEPROMData.nrOptionSelect > 3) {
    EEPROMData.nrOptionSelect = 0;
  }
  if (EEPROMData.nrOptionSelect == 3) ANR_notch = false;  // Turn off AutoNotch if LMS NR is selected.
  UpdateNoiseField();
}


/*****
  Purpose: To set the notch filter

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonNotchFilter() {
  ANR_notch =  not ANR_notch;
  //  If the notch is activated and LMS NR is also active, turn off NR and update display.
  if (ANR_notch && EEPROMData.nrOptionSelect == 3) {
    EEPROMData.nrOptionSelect = 0;  // Turn off noise reduction.  Other NR selections will be valid.
    UpdateNoiseField();
  }
}


/*****
  Purpose: Allows quick setting of noise floor in spectrum display

  Parameter list:
    void

  Return value;
    void
*****/
void ButtonSetNoiseFloor() {
  int floor = EEPROMData.currentNoiseFloor[EEPROMData.currentBand];  // KF5N
//  int val;
  MenuSelect menu;

  tft.setFontScale((enum RA8875tsize)1);
  ErasePrimaryMenu();
  tft.fillRect(SECONDARY_MENU_X - 100, MENUS_Y, EACH_MENU_WIDTH + 120, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_BLACK);  // JJP 7/17/23
  tft.setCursor(SECONDARY_MENU_X - 98, MENUS_Y + 1);
  tft.print("Pixels above axis:");
  tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
  tft.print(EEPROMData.currentNoiseFloor[EEPROMData.currentBand]);
  delay(150L);

  while (true) {
    if (filterEncoderMove != 0) {
      floor += filterEncoderMove;  // It moves the display
      EraseSpectrumWindow();
      floor = DrawNewFloor(floor);
      tft.fillRect(SECONDARY_MENU_X + 190, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
      tft.print(floor);
      filterEncoderMove = 0;
    }

//    val = ReadSelectedPushButton();  // Get ADC value
//    delay(100L);
    menu = readButton();
    if (menu == MenuSelect::MENU_OPTION_SELECT)  // If they made a choice...
    {
      EEPROMData.currentNoiseFloor[EEPROMData.currentBand] = floor;
      eeprom.EEPROMWrite();
      break;
    }
  }
  EraseMenus();
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  tft.setTextColor(RA8875_WHITE);
  DrawSpectrumDisplayContainer();
  ShowSpectrumdBScale();
  ShowSpectrum();
  tft.writeTo(L2);
  DrawFrequencyBarValue();
  tft.writeTo(L1);
}


/*****
  Purpose: Draw in a red line at the new floor position

  Parameter list:
    int floor     the pixel position for the new floor

  Return value;
    int           the current noise floor value
*****/
int DrawNewFloor(int floor) {
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
void ResetZoom(int zoomIndex1) {
  if (zoomIndex1 == MAX_ZOOM_ENTRIES) {
    zoomIndex1 = 0;
  }
  if (zoomIndex1 <= 0)
    EEPROMData.spectrum_zoom = 0;
  else
    EEPROMData.spectrum_zoom = zoomIndex1;

  ZoomFFTPrep();
  UpdateZoomField();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  ShowFrequency();
  ShowBandwidth();
  RedrawDisplayScreen();
}


/*****
  Purpose: Direct Frequency Entry

  Parameter list:
    void

  Return value;
    void
    Base Code courtesy of Harry  GM3RVL
*****/
void ButtonFrequencyEntry() {
  bool valid_frequency = false;
  TxRxFreqOld = TxRxFreq;

#define show_FEHelp
  bool doneFE = false;                         // set to true when a valid frequency is entered
  long enteredF = 0L;                          // desired frequency
  char strF[6] = { ' ', ' ', ' ', ' ', ' ' };  // container for frequency string during entry
  String stringF;
//  int valPin;
  MenuSelect menu = MenuSelect::DEFAULT;
  int key;
  int numdigits = 0;  // number of digits entered
  int pushButtonSwitchIndex;
//  EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO] = TxRxFreq;
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
  tft.print(DE_Band[EEPROMData.currentBand]);
  tft.print(" band");
  tft.setCursor(WATERFALL_LEFT_X + 20, SPECTRUM_TOP_Y + 270);
  tft.print("Frequency response limited above ");
  tft.print(DE_Flimit[EEPROMData.currentBand]);
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
  EEPROMData.centerFreq = TxRxFreq;
  centerTuneFlag = 1;  // Put back in so tuning bar is refreshed.  KF5N July 31, 2023
  SetFreq();           // Used here instead of centerTuneFlag.  KF5N July 22, 2023
  if (save_last_frequency == true and valid_frequency == true) {
    EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO] = enteredF;
  } else {
    if (save_last_frequency == 0) {
      EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO] = TxRxFreqOld;
    }
  }
  tft.fillRect(0, 0, 799, 479, RA8875_BLACK);  // Clear layer 2  JJP 7/23/23
  tft.writeTo(L1);
  EraseSpectrumDisplayContainer();
  DrawSpectrumDisplayContainer();
  DrawFrequencyBarValue();
  SetBand();
  SetFreq();
  ShowFrequency();
  ShowSpectrumdBScale();
  AudioInterrupts();
  eeprom.EEPROMWrite();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  if (EEPROMData.xmtMode == RadioMode::CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  RedrawDisplayScreen();  // KD0RC
  FilterSetSSB();
}
