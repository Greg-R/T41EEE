
#include "SDT.h"

// DrawAudioSpectContainer()
// ShowName()
// ShowSpectrum()
// ShowBandwidth()
// ShowSpectrumdBScale()
// DrawSMeterContainer()
// ShowSpectrumdBScale()
// DrawSpectrumDisplayContainer()
// DrawFrequencyBarValue()
// ShowAutoStatus()
// BandInformation()
//

#define CLIP_AUDIO_PEAK 115  // The pixel value where audio peak overwrites S-meter
#define INCREMENT_X WATERFALL_RIGHT_X + 25
#define INCREMENT_Y WATERFALL_TOP_Y + 70
#define SMETER_X WATERFALL_RIGHT_X + 16
#define SMETER_Y YPIXELS * 0.22  // 480 * 0.22 = 106

const uint16_t gradient[] = {  // Color array for waterfall background
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
  0x10, 0x1F, 0x11F, 0x19F, 0x23F, 0x2BF, 0x33F, 0x3BF, 0x43F, 0x4BF,
  0x53F, 0x5BF, 0x63F, 0x6BF, 0x73F, 0x7FE, 0x7FA, 0x7F5, 0x7F0, 0x7EB,
  0x7E6, 0x7E2, 0x17E0, 0x3FE0, 0x67E0, 0x8FE0, 0xB7E0, 0xD7E0, 0xFFE0, 0xFFC0,
  0xFF80, 0xFF20, 0xFEE0, 0xFE80, 0xFE40, 0xFDE0, 0xFDA0, 0xFD40, 0xFD00, 0xFCA0,
  0xFC60, 0xFC00, 0xFBC0, 0xFB60, 0xFB20, 0xFAC0, 0xFA80, 0xFA20, 0xF9E0, 0xF980,
  0xF940, 0xF8E0, 0xF8A0, 0xF840, 0xF800, 0xF802, 0xF804, 0xF806, 0xF808, 0xF80A,
  0xF80C, 0xF80E, 0xF810, 0xF812, 0xF814, 0xF816, 0xF818, 0xF81A, 0xF81C, 0xF81E,
  0xF81E, 0xF81E, 0xF81E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF85E, 0xF85E, 0xF85E,
  0xF85E, 0xF87E, 0xF87E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF85E, 0xF85E, 0xF85E,
  0xF85E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E,
  0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF88F, 0xF88F, 0xF88F
};

int16_t spectrum_x = 10;
uint16_t waterfall[MAX_WATERFALL_WIDTH];
int maxYPlot;
int filterWidthX;              // The current filter X.
uint8_t twinpeaks_tested = 2;  // initial value --> 2 !!
uint8_t write_analog_gain = 0;
int16_t pos_x_time = 390;  // 14;
int16_t pos_y_time = 5;    //114;
float xExpand = 1.4;       //
int16_t spectrum_pos_centre_f = 64 * xExpand;
int pos_centre_f = 64;
int smeterLength;
float CPU_temperature = 0.0;
double elapsed_micros_mean;


/*****
  Purpose: Draw audio spectrum box.  AFP added 3-14-21

  Parameter list:

  Return value;
    void
*****/
void DrawAudioSpectContainer() {
  tft.writeTo(L1);
  tft.drawRect(BAND_INDICATOR_X - 9, SPECTRUM_BOTTOM - 118, 255, 118, RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  for (int k = 0; k < 6; k++) {
    tft.drawFastVLine(BAND_INDICATOR_X - 10 + k * 43.8, SPECTRUM_BOTTOM, 15, RA8875_GREEN);
    tft.setCursor(BAND_INDICATOR_X - 14 + k * 43.8, SPECTRUM_BOTTOM + 16);
    tft.print(k);
    tft.print("k");
  }
}


/*****
  Purpose: Show the program name and version number.

  Parameter list:
    void

  Return value;
    void
*****/
void ShowName() {
  tft.fillRect(RIGNAME_X_OFFSET, 0, XPIXELS - RIGNAME_X_OFFSET, tft.getFontHeight(), RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_YELLOW);
  tft.setCursor(RIGNAME_X_OFFSET - 20, 1);
  tft.print(RIGNAME);
  tft.setFontScale(0);
  tft.print(" ");                  // Added to correct for deleted leading space 4/16/2022 JACK
#ifdef FOURSQRP                    // Give visual indication that were using the 4SQRP code.  W8TEE October 11, 2023.
  tft.setTextColor(RA8875_GREEN);  // Make it green
#else
  tft.setTextColor(RA8875_RED);  // Make it red
#endif
  tft.print(ConfigData.versionSettings);
}


/*****
  Purpose: Show Spectrum display with auto RF gain for T41EEE.1.  Harry GM3RVL, January 16, 2024
            Note that this routine calls the Audio process Function during each display cycle,
            for each of the 512 display frequency bins.  This means that the audio is refreshed at the maximum rate
            and does not have to wait for the display to complete drawing the full spectrum.
            However, the display data are only updated ONCE during each full display cycle,
            ensuring consistent data for the erase/draw cycle at each frequency point.

  Parameter list:
    void

  Return value;
    void
*****/
void ShowSpectrum() {
#define LOWERPIXTARGET 13  //  HB start
#define UPPERPIXTARGET 15

  tft.writeTo(L1);                         // TEMPORARY for graphics debugging.
  int AudioH_max = 0, AudioH_max_box = 0;  // Used to center audio spectrum.
  char buff[10];
  int frequ_hist[32]{ 0 };   // All values are initialized to zero using this syntax.
  int audio_hist[256]{ 0 };  // All values are initialized to zero using this syntax.
  int j, k;
  int FH_max = 0, FH_max_box = 0;  //  HB finish
  int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2;
  int middleSlice = centerLine / 2;  // Approximate center element
  int x1 = 0;                        //AFP
  int h = SPECTRUM_HEIGHT + 3;
  int y_new_plot, y1_new_plot, y_old_plot, y_old2_plot;
  int test1;
  updateDisplayCounter = 0;

  tft.drawFastVLine(centerLine, SPECTRUM_TOP_Y, h, RA8875_GREEN);  // Draws centerline on spectrum display

  pixelnew[0] = 0;  // globals
  pixelnew[1] = 0;
  pixelCurrent[0] = 0;
  pixelCurrent[1] = 0;
  //                512
  for (x1 = 1; x1 < MAX_WATERFALL_WIDTH - 1; x1++)  //AFP, JJP changed init from 0 to 1 for x1: out of bounds addressing in line 112
  //Draws the main Spectrum, Waterfall and Audio displays
  {
    updateDisplayFlag = false;
    if ((ConfigData.spectrum_zoom == 0) and static_cast<uint32_t>(ADC_RX_I.available()) > N_BLOCKS and static_cast<uint32_t>(ADC_RX_Q.available()) > N_BLOCKS) {
      updateDisplayCounter = updateDisplayCounter + 1;
      if (updateDisplayCounter == 1) updateDisplayFlag = true;
    }
    if ((ConfigData.spectrum_zoom == 1) and static_cast<uint32_t>(ADC_RX_I.available()) > N_BLOCKS and static_cast<uint32_t>(ADC_RX_Q.available()) > N_BLOCKS) {
      updateDisplayCounter = updateDisplayCounter + 1;
      if (updateDisplayCounter == 1) updateDisplayFlag = true;
    }
    if ((ConfigData.spectrum_zoom == 2) and static_cast<uint32_t>(ADC_RX_I.available()) > N_BLOCKS and static_cast<uint32_t>(ADC_RX_Q.available()) > N_BLOCKS) {
      updateDisplayCounter = updateDisplayCounter + 1;
      if (updateDisplayCounter == 1) updateDisplayFlag = true;
    }
    if ((ConfigData.spectrum_zoom == 3) and static_cast<uint32_t>(ADC_RX_I.available()) > N_BLOCKS and static_cast<uint32_t>(ADC_RX_Q.available()) > N_BLOCKS) {
      updateDisplayCounter = updateDisplayCounter + 1;
      if (updateDisplayCounter == 3) updateDisplayFlag = true;
    }
    if ((ConfigData.spectrum_zoom == 4) and static_cast<uint32_t>(ADC_RX_I.available()) > N_BLOCKS and static_cast<uint32_t>(ADC_RX_Q.available()) > N_BLOCKS) {
      updateDisplayCounter = updateDisplayCounter + 1;
      if (updateDisplayCounter == 7) updateDisplayFlag = true;
    }

    // Don't call this function unless the filter bandwidth has been adjusted.  This requires 2 global variables.
    if (filter_pos != last_filter_pos) FilterSetSSB();

    process.ProcessIQData();  // Call the Audio process from within the display routine to eliminate conflicts with drawing the spectrum and waterfall displays

    EncoderCenterTune();  //Moved the tuning encoder to reduce lag times and interference during tuning.
    y_new = pixelnew[x1];
    y1_new = pixelnew[x1 - 1];
    y_old = pixelold[x1];  // pixelold spectrum is saved by the FFT function prior to a new FFT which generates the pixelnew spectrum.  KF5N
    y_old2 = pixelold[x1 - 1];

    y_new_plot = 247 - y_new;
    y1_new_plot = 247 - y1_new;
    y_old_plot = 247 - y_old;
    y_old2_plot = 247 - y_old2;

    // Collect a histogram of RF spectrum values.  This is used in AutoGain and AutoSpectrum.
    // 247 is the spectral display bottom.  120 is the spectral display top.
    if ((x1 > 51) && (x1 < 461))  //  HB start for auto RFgain collect frequency distribution. Limited to core of FFT and dividable by 4.
    {
      j = 247 - y_new_plot + 40;       // +40 to get 10 bins below zero - want to straddle zero to make the entire spectrum viewable.
      k = j >> 2;                      // Divide by 4
      if ((k > -1) && (k < 32)) {      // The index of the bin array.  This indicates a value within the defined spectral box.
        frequ_hist[k] += 1;            // Add (accumulate) to the bin.
        if (frequ_hist[k] > FH_max) {  // FH_max starts at 0.
          FH_max = frequ_hist[k];      // Reset FH_max to the current bin value.
          FH_max_box = k;              // Index of FH_max.
        }
      }
    }  //  HB finish

    // Collect a histogram of audio spectral values.  This is used to keep the audio spectrum in the viewable area.
    // 247??? is the spectral display bottom.  129 is the audio spectrum display top.
    if ((x1 < 256) and (audioYPixel[x1] > 0))  //  Collect audio frequency distribution to find noise floor.
    {
      k = audioYPixel[x1];               // +40 to get 10 bins below zero - want to straddle zero to make the entire spectrum viewable.
      audio_hist[k] += 1;                // Add (accumulate) to the bin.
                                         //       if(x1 == 50) Serial.printf("audio_hist[k] = %d AudioH_max = %d\n", audio_hist[k], AudioH_max);
      if (audio_hist[k] > AudioH_max) {  // FH_max starts at 0.
        AudioH_max = audio_hist[k];      // Reset FH_max to the current bin value.
        AudioH_max_box = k;              // Index of FH_max.  this corresponds to the noise floor.
                                         //        Serial.printf("k = %d AudioH_max_box = %d\n", k, AudioH_max_box);
      }
    }  //  HB finish

    // Prevent spectrum from going below the bottom of the spectrum area.  KF5N
    if (y_new_plot > 247) y_new_plot = 247;
    if (y1_new_plot > 247) y1_new_plot = 247;
    if (y_old_plot > 247) y_old_plot = 247;
    if (y_old2_plot > 247) y_old2_plot = 247;

    // Prevent spectrum from going above the top of the spectrum area.  KF5N
    if (y_new_plot < 120) y_new_plot = 120;
    if (y1_new_plot < 120) y1_new_plot = 120;
    if (y_old_plot < 120) y_old_plot = 120;
    if (y_old2_plot < 120) y_old2_plot = 120;

    // Erase the old spectrum, and draw the new spectrum.
    tft.drawLine(x1 + 1, y_old2_plot, x1 + 1, y_old_plot, RA8875_BLACK);   // Erase old...
    tft.drawLine(x1 + 1, y1_new_plot, x1 + 1, y_new_plot, RA8875_YELLOW);  // Draw new

    //  What is the actual spectrum at this time?  It's a combination of the old and new spectrums.
    //  In the case of a CW interrupt, the array pixelnew should be saved as the actual spectrum.
    pixelCurrent[x1] = pixelnew[x1];  //  This is the actual "old" spectrum!  This is required due to CW interrupts.  pixelCurrent gets copied to pixelold by the FFT function.  KF5N
    audioYPixelcurrent[x1] = audioYPixel[x1];

    // Draw audio spectrum.  The audio spectrum width is smaller than the RF spectrum width.
    if (x1 < 253) {                                                                      //AFP 09-01-22
      if (keyPressedOn == 1) {                                                           //AFP 09-01-22
        return;                                                                          //AFP 09-01-22
      } else {                                                                           //AFP 09-01-22
        if (audioYPixelold[x1] > CLIP_AUDIO_PEAK) audioYPixelold[x1] = CLIP_AUDIO_PEAK;  // audioSpectrumHeight = 118
        tft.drawFastVLine(532 + x1, 245 - audioYPixelold[x1] - 0, audioYPixelold[x1], RA8875_BLACK);
        if (audioYPixel[x1] != 0) {
          if (audioYPixel[x1] > CLIP_AUDIO_PEAK)  // audioSpectrumHeight = 118
            audioYPixel[x1] = CLIP_AUDIO_PEAK;
          if (x1 == middleSlice) {
            smeterLength = y_new;
          }
          // Draw a vertical line with the audio spectrum magnitude.  AUDIO_SPECTRUM_BOTTOM = 247
          tft.drawFastVLine(532 + x1, 245 - audioYPixel[x1] - 0, audioYPixel[x1], RA8875_MAGENTA);
        }
      }
    }

    test1 = -y_new_plot + 230;  // Nudged waterfall towards blue.  KF5N July 23, 2023
    if (test1 < 0) test1 = 0;
    if (test1 > 117) test1 = 117;
    waterfall[x1] = gradient[test1];  // Try to put pixel values in middle of gradient array.  KF5N
    tft.writeTo(L1);
  }  // End for(...) Draw MAX_WATERFALL_WIDTH spectral points

  // Use the Block Transfer Engine (BTE) to move waterfall down a line
  if (keyPressedOn == 1) {
    return;
  } else {
    tft.BTE_move(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE, MAX_WATERFALL_WIDTH, MAX_WATERFALL_ROWS - 2, WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, 1, 2);
    while (tft.readStatus())
      ;  // Make sure it is done.  Memory moves can take time.

    // Now bring waterfall back to the beginning of the 2nd row.
    tft.BTE_move(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, MAX_WATERFALL_WIDTH, MAX_WATERFALL_ROWS - 2, WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, 2);
    while (tft.readStatus())
      ;  // Make sure it's done.
  }
  // Then write new row data into the missing top row to get a scroll effect using display hardware, not the CPU.
  tft.writeRect(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE, MAX_WATERFALL_WIDTH, 1, waterfall);

  // Manage RF spectral display graphics.  Keep the spectrum within the viewable area.
  if (ConfigData.autoGain || ConfigData.autoSpectrum) {
    if (FH_max_box > UPPERPIXTARGET) {  // HB. Adjust rfGainAllBands 15 and 13 to alter to move target base up and down. UPPERPIXTARGET = 15
      if (ConfigData.autoGain) ConfigData.rfGainCurrent = ConfigData.rfGainCurrent - 1;
      if (ConfigData.autoSpectrum) {
        ConfigData.rfGainCurrent = ConfigData.rfGain[ConfigData.currentBand];
        fftOffset = fftOffset - 1;
      }
    }
    if (FH_max_box < LOWERPIXTARGET) {  // LOWERPIXTARGET = 13
      if (ConfigData.autoGain) {
        ConfigData.rfGainCurrent = ConfigData.rfGainCurrent + 1;
        if (ConfigData.rfGainCurrent > 25.0) ConfigData.rfGainCurrent = 25.0;  //  Do not allow RF gain greater than 25.
      }
      if (ConfigData.autoSpectrum) {
        ConfigData.rfGainCurrent = ConfigData.rfGain[ConfigData.currentBand];
        fftOffset = fftOffset + 1;
      }
    }
  }
  // Don't allow fftOffset to exceed 200:
  if (fftOffset > 200) fftOffset = 200;

  // Manage audio spectral display graphics.  Keep the spectrum within the viewable area.

  if (AudioH_max_box > 30) {  // HB. Adjust rfGainAllBands 15 and 13 to alter to move target base up and down. UPPERPIXTARGET = 15
    audioFFToffset = audioFFToffset - 1;
  }
  if (AudioH_max_box < 28) {  // LOWERPIXTARGET = 13
    audioFFToffset = audioFFToffset + 1;
  }

  // Report the current RF "gain" setting.
  tft.fillRect(SPECTRUM_LEFT_X + 125, SPECTRUM_TOP_Y + 2, 33, tft.getFontHeight(), RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SPECTRUM_LEFT_X + 64, SPECTRUM_TOP_Y + 2);
  tft.print("RF GAIN");
  tft.setCursor(SPECTRUM_LEFT_X + 129, SPECTRUM_TOP_Y + 2);
  if (ConfigData.autoGain) itoa(ConfigData.rfGainCurrent, buff, DEC);  // Make into a string
  else itoa(ConfigData.rfGain[ConfigData.currentBand], buff, DEC);

  tft.print(buff);  // HB End

}  // End ShowSpectrum()


/*****
  Purpose: Show filter bandwidth near center of spectrum.

  Parameter list:
    void

  Return value;
    void
        // AudioNoInterrupts();
        // M = demod_mode, FU & FL upper & lower frequency
        // this routine prints the frequency bars under the spectrum display
*****/
void ShowBandwidth() {
  char buff[10];
  int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2;
  int pos_left;

  if (ConfigData.spectrum_zoom != SPECTRUM_ZOOM_1)
    spectrum_pos_centre_f = 128 * xExpand - 1;  //AFP
  else
    spectrum_pos_centre_f = 64 * xExpand;  //AFP
  pos_left = centerLine + static_cast<int>(bands.bands[ConfigData.currentBand].FLoCut / 1000.0 * pixel_per_khz);
  if (pos_left < spectrum_x) {
    pos_left = spectrum_x;
  }

  // Need to add in code for zoom factor here AFP 10-20-22

  filterWidthX = pos_left + newCursorPosition - centerLine;
  tft.writeTo(L2);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_LIGHT_GREY);
  if (switchFilterSideband == false)
    tft.setTextColor(RA8875_WHITE);
  else if (switchFilterSideband == true)
    tft.setTextColor(RA8875_LIGHT_GREY);

  MyDrawFloat(static_cast<float>(bands.bands[ConfigData.currentBand].FLoCut / 1000.0f), 1, FILTER_PARAMETERS_X, FILTER_PARAMETERS_Y, buff);

  tft.print("kHz");
  if (switchFilterSideband == true)
    tft.setTextColor(RA8875_WHITE);
  else if (switchFilterSideband == false)
    tft.setTextColor(RA8875_LIGHT_GREY);
  MyDrawFloat(static_cast<float>(bands.bands[ConfigData.currentBand].FHiCut / 1000.0f), 1, FILTER_PARAMETERS_X + 80, FILTER_PARAMETERS_Y, buff);
  tft.print("kHz");

  tft.setTextColor(RA8875_WHITE);  // set text color to white for other print routines not to get confused ;-)
  tft.writeTo(L1);
}


//DB2OO, 30-AUG-23: this variable determines the pixels per S step. In the original code it was 12.2 pixels !?
#ifdef TCVSDR_SMETER
const float pixels_per_s = 12;
#else
const float pixels_per_s = 12.2;
#endif
/*****
  Purpose: DrawSMeterContainer()
  Parameter list:
    void
  Return value;
    void
*****/
void DrawSMeterContainer() {
  int i;
  // DB2OO, 30-AUG-23: the white line must only go till S9
  tft.drawFastHLine(SMETER_X, SMETER_Y - 1, 9 * pixels_per_s, RA8875_WHITE);
  tft.drawFastHLine(SMETER_X, SMETER_Y + SMETER_BAR_HEIGHT + 2, 9 * pixels_per_s, RA8875_WHITE);  // changed 6 to 20

  for (i = 0; i < 10; i++) {  // Draw tick marks for S-values
#ifdef TCVSDR_SMETER
    //DB2OO, 30-AUG-23: draw wider tick marks in the style of the Teensy Convolution SDR
    tft.drawRect(SMETER_X + i * pixels_per_s, SMETER_Y - 6 - (i % 2) * 2, 2, 6 + (i % 2) * 2, RA8875_WHITE);
#else
    tft.drawFastVLine(SMETER_X + i * 12.2, SMETER_Y - 6, 7, RA8875_WHITE);
#endif
  }

  // DB2OO, 30-AUG-23: the green line must start at S9
  tft.drawFastHLine(SMETER_X + 9 * pixels_per_s, SMETER_Y - 1, SMETER_BAR_LENGTH + 2 - 9 * pixels_per_s, RA8875_GREEN);
  tft.drawFastHLine(SMETER_X + 9 * pixels_per_s, SMETER_Y + SMETER_BAR_HEIGHT + 2, SMETER_BAR_LENGTH + 2 - 9 * pixels_per_s, RA8875_GREEN);

  for (i = 1; i <= 3; i++) {  // Draw tick marks for s9+ values in 10dB steps
#ifdef TCVSDR_SMETER
    //DB2OO, 30-AUG-23: draw wider tick marks in the style of the Teensy Convolution SDR
    tft.drawRect(SMETER_X + 9 * pixels_per_s + i * pixels_per_s * 10.0 / 6.0, SMETER_Y - 8 + (i % 2) * 2, 2, 8 - (i % 2) * 2, RA8875_GREEN);
#else
    tft.drawFastVLine(SMETER_X + 9 * pixels_per_s + i * pixels_per_s * 10.0 / 6.0, SMETER_Y - 6, 7, RA8875_GREEN);
#endif
  }

  tft.drawFastVLine(SMETER_X, SMETER_Y - 1, SMETER_BAR_HEIGHT + 3, RA8875_WHITE);
  tft.drawFastVLine(SMETER_X + SMETER_BAR_LENGTH + 2, SMETER_Y - 1, SMETER_BAR_HEIGHT + 3, RA8875_GREEN);

  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  //DB2OO, 30-AUG-23: moved single digits a bit to the right, to align
  tft.setCursor(SMETER_X - 8, SMETER_Y - 25);
  tft.print("S");
  tft.setCursor(SMETER_X + 8, SMETER_Y - 25);
  tft.print("1");
  tft.setCursor(SMETER_X + 32, SMETER_Y - 25);  // was 28, 48, 68, 88, 120 and -15 changed to -20
  tft.print("3");
  tft.setCursor(SMETER_X + 56, SMETER_Y - 25);
  tft.print("5");
  tft.setCursor(SMETER_X + 80, SMETER_Y - 25);
  tft.print("7");
  tft.setCursor(SMETER_X + 104, SMETER_Y - 25);
  tft.print("9");
  //DB2OO, 30-AUG-23 +20dB needs to get more left
  tft.setCursor(SMETER_X + 133, SMETER_Y - 25);
  tft.print("+20dB");
}


/*****
  Purpose: Print the vertical dB setting to the spectrum display.
  Parameter list:
    void
  Return value;
    void
*****/
void ShowSpectrumdBScale() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(SPECTRUM_LEFT_X + 1, SPECTRUM_TOP_Y + 2, 33, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(SPECTRUM_LEFT_X + 5, SPECTRUM_TOP_Y + 2);
  tft.setTextColor(RA8875_WHITE);
  tft.print(displayScale[ConfigData.currentScale].dbText);
}


/*****
  Purpose: This function draws spectrum display container
  Parameter list:
    void
  Return value;
    void
*****/
void DrawSpectrumDisplayContainer() {
  if (calOnFlag)
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, 362, RA8875_YELLOW);  // Spectrum box for calibration.
  else {
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, 362, RA8875_BLACK);               // Erase spectrum box for calibration.
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, SPECTRUM_HEIGHT, RA8875_YELLOW);  // Spectrum box.  SPECTRUM_HEIGHT = 150
  }
}


/*****
  Purpose: This function draws the frequency bar at the bottom of the spectrum scope, putting markers at every
            graticule and the full frequency

  Parameter list:
    void

  Return value;
    void
*****/
void DrawFrequencyBarValue() {
  char txt[16];
  int bignum;
  int centerIdx;
  int pos_help;
  float disp_freq;
  float freq_calc;
  float grat;
  int centerLine = MAX_WATERFALL_WIDTH / 2 + SPECTRUM_LEFT_X;
  // positions for graticules: first for ConfigData.spectrum_zoom < 3, then for ConfigData.spectrum_zoom > 2
  const static int idx2pos[2][9] = {
    { -43, 21, 50, 250, 140, 250, 232, 250, 315 },  //AFP 10-30-22
    { -43, 21, 50, 85, 200, 200, 232, 218, 315 }    //AFP 10-30-22
  };

  grat = static_cast<float>(SR[SampleRate].rate / 8000.0) / static_cast<float>(1 << ConfigData.spectrum_zoom);  // 1, 2, 4, 8, 16, 32, 64 . . . 4096

  tft.writeTo(L2);  // Not writing to correct layer?  KF5N.  July 31, 2023
  tft.setTextColor(RA8875_WHITE);
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(WATERFALL_LEFT_X, WATERFALL_TOP_Y, MAX_WATERFALL_WIDTH + 5, tft.getFontHeight(), RA8875_BLACK);  // 4-16-2022 JACK

  freq_calc = static_cast<float>(static_cast<uint32_t>(ConfigData.centerFreq));  // get current frequency in Hz

  if (ConfigData.spectrum_zoom == 0) {
    freq_calc += static_cast<float>(SR[SampleRate].rate) / 4.0;
  }

  if (ConfigData.spectrum_zoom < 5) {
    freq_calc = roundf(freq_calc / 100) / 10;  // round graticule frequency to the nearest 100Hz
  }

  if (ConfigData.spectrum_zoom != 0)
    centerIdx = 0;
  else
    centerIdx = -2;

  /**************************************************************************************************
    CENTER FREQUENCY PRINT
  **************************************************************************************************/
  ultoa((freq_calc + (centerIdx * grat)), txt, DEC);
  disp_freq = freq_calc + (centerIdx * grat);
  bignum = (int)disp_freq;
  itoa(bignum, txt, DEC);  // Make into a string
  //=================== AFP 10-21-22 =====
  tft.setTextColor(RA8875_GREEN);

  //  ========= AFP 1-21-22 ======
  if (ConfigData.spectrum_zoom == 0) {
    tft.setCursor(centerLine - 140, WATERFALL_TOP_Y);  //AFP 10-20-22
  } else {
    tft.setCursor(centerLine - 20, WATERFALL_TOP_Y);  //AFP 10-20-22
  }
  //  ========= AFP 1-21-22 ====
  tft.print(txt);
  tft.setTextColor(RA8875_WHITE);
  /**************************************************************************************************
     PRINT ALL OTHER FREQUENCIES (NON-CENTER)
   **************************************************************************************************/
  // snprint() extremely memory inefficient. replaced with simple str?? functions JJP
  for (int idx = -4; idx < 5; idx++) {
    pos_help = idx2pos[ConfigData.spectrum_zoom < 3 ? 0 : 1][idx + 4];
    if (idx != centerIdx) {
      ultoa((freq_calc + (idx * grat)), txt, DEC);
      //================== AFP 10-21-22 =============
      if (ConfigData.spectrum_zoom == 0) {
        tft.setCursor(WATERFALL_LEFT_X + pos_help * xExpand + 40, WATERFALL_TOP_Y);  // AFP 10-20-22
      } else {
        tft.setCursor(WATERFALL_LEFT_X + pos_help * xExpand + 40, WATERFALL_TOP_Y);  // AFP 10-20-22
      }
      // ============  AFP 10-21-22
      tft.print(txt);
      if (idx < 4) {
        tft.drawFastVLine((WATERFALL_LEFT_X + pos_help * xExpand + 60), WATERFALL_TOP_Y - 5, 7, RA8875_YELLOW);  // Tick marks depending on zoom
      } else {
        tft.drawFastVLine((WATERFALL_LEFT_X + (pos_help + 9) * xExpand + 60), WATERFALL_TOP_Y - 5, 7, RA8875_YELLOW);
      }
    }
    if (ConfigData.spectrum_zoom > 2 || freq_calc > 1000) {
      idx++;
    }
  }
  tft.writeTo(L1);  // Always leave on layer 1.  KF5N.  July 31, 2023
  tft.setFontScale((enum RA8875tsize)1);
  //  ShowBandwidth();
}


/*****
  Purpose:  Indicate Auto-Gain or Auto-Spectrum is active.

  Parameter list:
    void

  Return value;
    void
*****/
void ShowAutoStatus() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(SPECTRUM_LEFT_X + 350, SPECTRUM_TOP_Y + 2, 130, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(SPECTRUM_LEFT_X + 350, SPECTRUM_TOP_Y + 2);
  tft.setTextColor(RA8875_WHITE);
  if (ConfigData.autoGain) {
    tft.print("Auto-Gain On");
  } else if (ConfigData.autoSpectrum) {
    tft.print("Auto-Spectrum On");
  } else
    tft.fillRect(SPECTRUM_LEFT_X + 350, SPECTRUM_TOP_Y + 2, 130, tft.getFontHeight(), RA8875_BLACK);
}


/*****
  Purpose: To display the current transmission frequency, band, mode, and sideband above the spectrum display

  Parameter list:
    void

  Return value;
    void

*****/
void BandInformation()  // SSB or CW
{
  std::string CWFilter[] = { "0.8kHz", "1.0kHz", "1.3kHz", "1.8kHz", "2.0kHz", " Off " };

  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(5, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_WHITE);

  tft.writeTo(L1);

  tft.print("Center Freq");                                    // This is static, never changes.
  tft.fillRect(100, FREQUENCY_Y + 31, 290, 15, RA8875_BLACK);  // Clear frequency, band, and mode.  This should be the only erase required.

  // Write the center frequency to the display.
  tft.setCursor(100, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_LIGHT_ORANGE);
  if (ConfigData.spectrum_zoom == SPECTRUM_ZOOM_1) {  // AFP 11-02-22
    tft.print(static_cast<int>(ConfigData.centerFreq) + 48000);
  } else {
    tft.print(static_cast<int>(ConfigData.centerFreq));
  }

  // Write the band to the display.
  tft.setTextColor(RA8875_LIGHT_ORANGE);
  tft.setCursor(OPERATION_STATS_X + 50, FREQUENCY_Y + 30);
  if (ConfigData.activeVFO == VFO_A) {
    tft.print(bands.bands[ConfigData.currentBandA].name);  // Write current band to the display.
  } else {
    tft.print(bands.bands[ConfigData.currentBandB].name);
  }

  // Write CW mode and filter bandwidth to display.
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(OPERATION_STATS_X + 90, FREQUENCY_Y + 30);  //AFP 10-18-22
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {
    tft.print("CW ");
    tft.setCursor(OPERATION_STATS_X + 111, FREQUENCY_Y + 30);  //AFP 10-18-22
    tft.print(CWFilter[ConfigData.CWFilterIndex].c_str());     //AFP 10-18-22

    //================  AFP 10-19-22 =========
  }

  // Write SSB mode to display
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
    if (ConfigData.cessb) tft.print("CESSB");  // Which mode
    if (not ConfigData.cessb) tft.print("SSB");
  }

  // Write FT8 mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {
    tft.print("FT8");
  }

  // Write AM mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::AM_MODE) {
    tft.print("AM");
  }

  // Write SAM mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SAM_MODE) {
    tft.print("SAM");
  }

  // Write sideband or AM demodulation type to display.
  tft.setCursor(OPERATION_STATS_X + 165, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_WHITE);

  switch (bands.bands[ConfigData.currentBand].sideband) {
    case Sideband::LOWER:
      tft.print("LSB");  // Which sideband //AFP 09-22-22

      break;

    case Sideband::UPPER:
      //      if (ConfigData.activeVFO == VFO_A) {
      tft.print("USB");  // Which sideband //AFP 09-22-22
      break;
    case Sideband::BOTH_AM:
      //      tft.setTextColor(RA8875_WHITE);
      tft.print("DSB");  //AFP 09-22-22
      break;
    case Sideband::BOTH_SAM:  //AFP 11-01-22
      tft.print("DSB");       //AFP 11-01-22
      break;
    default:
      break;
  }
}


/*****
  Purpose: Display current power setting

  Parameter list:
    void

  Return value;
    void
*****/
void ShowCurrentPowerSetting() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(OPERATION_STATS_X + 275, FREQUENCY_Y + 30, tft.getFontWidth() * 11, tft.getFontHeight(), RA8875_BLACK);  // Clear top-left menu area
  tft.setCursor(OPERATION_STATS_X + 275, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_RED);
  tft.print(ConfigData.transmitPowerLevel, 1);  // Power output is a float
  tft.print(" Watts");
  tft.setTextColor(RA8875_WHITE);
}


/*****
  Purpose: Format frequency for printing
  Parameter list:
    void
  Return value;
    void
    // show frequency
*****/
void FormatFrequency(uint32_t freq, char *freqBuffer) {
  char outBuffer[15];
  int i;
  int len;
  freq = static_cast<uint32_t>(freq);
  ltoa(freq, outBuffer, 10);
  len = strlen(outBuffer);

  switch (len) {
    case 6:  // below 530.999 KHz
      freqBuffer[0] = outBuffer[0];
      freqBuffer[1] = outBuffer[1];
      freqBuffer[2] = outBuffer[2];
      freqBuffer[3] = FREQ_SEP_CHARACTER;  // Add separation charcter
      for (i = 4; i < len; i++) {
        freqBuffer[i] = outBuffer[i - 1];  // Next 3 digit chars
      }
      freqBuffer[i] = '0';       // trailing 0
      freqBuffer[i + 1] = '\0';  // Make it a string
      break;

    case 7:  // 1.0 - 9.999 MHz
      freqBuffer[0] = outBuffer[0];
      freqBuffer[1] = FREQ_SEP_CHARACTER;  // Add separation charcter
      for (i = 2; i < 5; i++) {
        freqBuffer[i] = outBuffer[i - 1];  // Next 3 digit chars
      }
      freqBuffer[5] = FREQ_SEP_CHARACTER;  // Add separation charcter
      for (i = 6; i < 9; i++) {
        freqBuffer[i] = outBuffer[i - 2];  // Last 3 digit chars
      }
      freqBuffer[i] = '\0';  // Make it a string
      break;

    case 8:  // 10 MHz - 30MHz
      freqBuffer[0] = outBuffer[0];
      freqBuffer[1] = outBuffer[1];
      freqBuffer[2] = FREQ_SEP_CHARACTER;  // Add separation charcter
      for (i = 3; i < 6; i++) {
        freqBuffer[i] = outBuffer[i - 1];  // Next 3 digit chars
      }
      freqBuffer[6] = FREQ_SEP_CHARACTER;  // Add separation charcter
      for (i = 7; i < 10; i++) {
        freqBuffer[i] = outBuffer[i - 2];  // Last 3 digit chars
      }
      freqBuffer[i] = '\0';  // Make it a string
      break;
  }
}


/*****
  Purpose: Show Main frequency display at top.  This shows currentFreqA and currentFreqB.

  Parameter list:
    void

  Return value;
    void
    // show frequency
*****/
void ShowFrequency() {
  char freqBuffer[15] = "              ";  // Initialize to blanks.;
  if (ConfigData.activeVFO == VFO_A) {  // Needed for edge checking
    ConfigData.currentBand = ConfigData.currentBandA;
  } else {
    ConfigData.currentBand = ConfigData.currentBandB;
  }

  if (ConfigData.activeVFO == VFO_A) {
    FormatFrequency(TxRxFreq, freqBuffer);
    tft.setFontScale(3, 2);  // JJP 7/15/23
    if (TxRxFreq < bands.bands[ConfigData.currentBandA].fBandLow or TxRxFreq > bands.bands[ConfigData.currentBandA].fBandHigh) {
      tft.setTextColor(RA8875_RED);  // Out of band
    } else {
      tft.setTextColor(RA8875_GREEN);  // In band
    }
    tft.fillRect(0, FREQUENCY_Y - 8, tft.getFontWidth() * 10, 34, RA8875_BLACK);  // This erases VFOA.
    tft.setCursor(0, FREQUENCY_Y - 17);                                                             // To adjust for Greg's font change jjp 7/14/23
    tft.print(freqBuffer);                                                                          // Show VFO_A
    tft.setFontScale(1, 2);                                                                         // JJP 7/15/23
    tft.setTextColor(RA8875_LIGHT_GREY);
    tft.setCursor(FREQUENCY_X_SPLIT + 60, FREQUENCY_Y - 15);
    FormatFrequency(ConfigData.currentFreqB, freqBuffer);
    tft.print(freqBuffer);
  } else {  // Show VFO_B
    FormatFrequency(TxRxFreq, freqBuffer);
    tft.setFontScale(3, 2);                                                                                              // JJP 7/15/23
                                                                                                                         //  tft.fillRect(FREQUENCY_X_SPLIT - 60, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);  //JJP 7/15/23
    tft.fillRect(FREQUENCY_X_SPLIT - 60, FREQUENCY_Y - 14, tft.getFontWidth() * 10, tft.getFontHeight(), RA8875_BLACK);  //JJP 7/15/23
    tft.setCursor(FREQUENCY_X_SPLIT - 60, FREQUENCY_Y - 12);
    if (TxRxFreq < bands.bands[ConfigData.currentBandB].fBandLow || TxRxFreq > bands.bands[ConfigData.currentBandB].fBandHigh) {
      tft.setTextColor(RA8875_RED);
    } else {
      tft.setTextColor(RA8875_GREEN);
    }
    tft.print(freqBuffer);   // Show VFO_A
    tft.setFontScale(1, 2);  // JJP 7/15/23
    FormatFrequency(TxRxFreq, freqBuffer);
    tft.fillRect(0, FREQUENCY_Y - 14, tft.getFontWidth() * 10, tft.getFontHeight(), RA8875_BLACK);  // JJP 7/15/23
    tft.setTextColor(RA8875_LIGHT_GREY);
    tft.setCursor(20, FREQUENCY_Y - 17);
    FormatFrequency(ConfigData.currentFreqA, freqBuffer);
    tft.print(freqBuffer);  // Show VFO_A
  }                         // end Show VFO_B
  tft.setFontDefault();
}


/*****
  Purpose: Display dBm
  Parameter list:
    void
  Return value;
    void
*****/
void DisplaydbM() {
  char buff[10];
  const char *unit_label;
  int16_t smeterPad;
  float32_t rfGain;
#ifdef TCVSDR_SMETER
  const float32_t slope = 10.0;
  const float32_t cons = -92;
#else
  float32_t audioLogAveSq;
#endif

  //DB2OO, 30-AUG-23: the S-Meter bar and the dBm value were inconsistent, as they were using different base values.
  // Moreover the bar could go over the limits of the S-meter box, as the map() function, does not constrain the values
  // with TCVSDR_SMETER defined the S-Meter bar will be consistent with the dBm value and the S-Meter bar will always be restricted to the box
  tft.fillRect(SMETER_X + 1, SMETER_Y + 1, SMETER_BAR_LENGTH, SMETER_BAR_HEIGHT, RA8875_BLACK);  //AFP 09-18-22  Erase old bar
#ifdef TCVSDR_SMETER
  //DB2OO, 9-OCT_23: dbm_calibration set to -22 in SDT.ino; gainCorrection is a value between -2 and +6 to compensate the frequency dependant pre-Amp gain
  // attenuator is 0 and could be set in a future HW revision; RFgain is initialized to 1 in the bands.bands[] init in SDT.ino; cons=-92; slope=10
  if (ConfigData.autoGain) rfGain = ConfigData.rfGainCurrent;
  else rfGain = ConfigData.rfGain[ConfigData.currentBand];
  dbm = CalData.dBm_calibration + bands.bands[ConfigData.currentBand].gainCorrection + static_cast<float32_t>(attenuator) + slope * log10f_fast(audioMaxSquaredAve) + cons - static_cast<float32_t>(bands.bands[ConfigData.currentBand].RFgain) * 1.5 - rfGain;  //DB2OO, 08-OCT-23; added ConfigData.rfGainAllBands
#else
  //DB2OO, 9-OCT-23: audioMaxSquaredAve is proportional to the input power. With ConfigData.rfGainAllBands=0 it is approx. 40 for -73dBm @ 14074kHz with the V010 boards and the pre-Amp fed by 12V
  // for audioMaxSquaredAve=40 audioLogAveSq will be 26
  audioLogAveSq = 10 * log10f_fast(audioMaxSquaredAve) + 10;  //AFP 09-18-22
  //DB2OO, 9-OCT-23: calculate dBm value from audioLogAveSq and ignore band gain differences and a potential attenuator like in original code
  dbm = audioLogAveSq - 100;

//DB2OO, 9-OCT-23: this is the orginal code, that will map a 30dB difference (35-5) to 60 (635-575) pixels, i.e. 5 S steps to 5*12 pixels
// SMETER_X is 528 --> X=635 would be 107 pixels / 12pixels per S step --> approx. S9
//  smeterPad = map(audioLogAveSq, 5, 35, 575, 635);                                                //AFP 09-18-22
//  tft.fillRect(SMETER_X + 1, SMETER_Y + 1, smeterPad - SMETER_X, SMETER_BAR_HEIGHT, RA8875_RED);  //AFP 09-18-22
#endif
  // determine length of S-meter bar, limit it to the box and draw it
  smeterPad = map(dbm, -73.0 - 9 * 6.0 /*S1*/, -73.0 /*S9*/, 0, 9 * pixels_per_s);
  //DB2OO; make sure, that it does not extend beyond the field
  smeterPad = max(0, smeterPad);
  smeterPad = min(SMETER_BAR_LENGTH, smeterPad);
  tft.fillRect(SMETER_X + 1, SMETER_Y + 2, smeterPad, SMETER_BAR_HEIGHT - 2, RA8875_RED);  //DB2OO: bar 2*1 pixel smaller than the field

  tft.setTextColor(RA8875_WHITE);

  //DB2OO, 17-AUG-23: create PWM analog output signal on the "HW_SMETER" output. This is scaled for a 250uA  S-meter full scale,
  // connected to HW_SMTER output via a 8.2kOhm resistor and a 4.7kOhm resistor and 10uF capacitor parallel to the S-Meter
#ifdef HW_SMETER
  {
    int hw_s;
    hw_s = map((int)dbm - 3, -73 - (8 * 6), -73 + 60, 0, 228);
    hw_s = max(0, min(hw_s, 255));
    analogWrite(HW_SMETER, hw_s);
  }
#endif

  // DB2OO, 13-OCT-23: if DEBUG_SMETER defined debug messages on S-Meter variables will be put out
  //#define DEBUG_SMETER

#ifdef DEBUG_SMETER
  //added, to debug S-Meter display problems
  Serial.printf("DisplaydbM(): dbm=%.1f, dbm_calibration=%.1f, bands.bands[ConfigData.currentBand].gainCorrection=%.1f, attenuator=%d, bands.bands[ConfigData.currentBand].RFgain=%d, ConfigData.rfGainAllBands=%d\n",
                dbm, dbm_calibration, bands.bands[ConfigData.currentBand].gainCorrection, attenuator, bands.bands[ConfigData.currentBand].RFgain, ConfigData.rfGainAllBands);
  Serial.printf("\taudioMaxSquaredAve=%.4f, audioLogAveSq=%.1f\n", audioMaxSquaredAve, audioLogAveSq);
#endif

  unit_label = "dBm";
  tft.setFontScale((enum RA8875tsize)0);

  tft.fillRect(SMETER_X + 185, SMETER_Y, 80, tft.getFontHeight(), RA8875_BLACK);  // The dB figure at end of S
  //DB2OO, 29-AUG-23: consider no decimals in the S-meter dBm value as it is very busy with decimals
  MyDrawFloat(dbm, /*0*/ 1, SMETER_X + 184, SMETER_Y, buff);
  tft.setTextColor(RA8875_GREEN);
  tft.print(unit_label);
}


/*****
  Purpose: Display the current temperature and load figures for T4.1

  Parameter list:
    int notchF        the notch to use
    int MODE          the current MODE

  Return value;
    void
*****/
void ShowTempAndLoad() {
  char buff[10];
  int valueColor = RA8875_GREEN;
  double block_time;
  double processor_load;
  elapsed_micros_mean = elapsed_micros_sum / elapsed_micros_idx_t;

  block_time = 128.0 / (double)SR[SampleRate].rate;  // one audio block is 128 samples and uses this in seconds
  block_time = block_time * N_BLOCKS;

  block_time *= 1000000.0;                                  // now in µseconds
  processor_load = elapsed_micros_mean / block_time * 100;  // take audio processing time divide by block_time, convert to %

  if (processor_load >= 100.0) {
    processor_load = 100.0;
    valueColor = RA8875_RED;
  }

  tft.setFontScale((enum RA8875tsize)0);

  CPU_temperature = TGetTemp();

  tft.fillRect(TEMP_X_OFFSET, TEMP_Y_OFFSET, MAX_WATERFALL_WIDTH, tft.getFontHeight(), RA8875_BLACK);  // Erase current data
  tft.setCursor(TEMP_X_OFFSET, TEMP_Y_OFFSET);
  tft.setTextColor(RA8875_WHITE);
  tft.print("Temp:");
  tft.setCursor(TEMP_X_OFFSET + 120, TEMP_Y_OFFSET);
  tft.print("Load:");

  tft.setTextColor(valueColor);
  MyDrawFloat(CPU_temperature, 1, TEMP_X_OFFSET + tft.getFontWidth() * 3, TEMP_Y_OFFSET, buff);

  tft.drawCircle(TEMP_X_OFFSET + 80, TEMP_Y_OFFSET + 5, 3, RA8875_GREEN);
  MyDrawFloat(processor_load, 1, TEMP_X_OFFSET + 150, TEMP_Y_OFFSET, buff);
  tft.print("%");
  elapsed_micros_idx_t = 0;
  elapsed_micros_sum = 0;
  elapsed_micros_mean = 0;
  tft.setTextColor(RA8875_WHITE);
}

/*****
  Purpose: format a floating point number

  Parameter list:
    float val         the value to format
    int decimals      the number of decimal places
    int x             the x coordinate for display
    int y                 y          "

  Return value;
    void
*****/
void MyDrawFloat(float val, int decimals, int x, int y, char *buff) {
  dtostrf(val, FLOAT_PRECISION, decimals, buff);  // Use 8 as that is the max prevision on a float

  tft.fillRect(x + 15, y, 12 * sizeof(buff), 15, RA8875_BLACK);
  tft.setCursor(x, y);

  tft.print(buff);
}


/*****
  Purpose: Shows the startup settings for the information displayed int the lower-right box.

  Parameter list:
    void

  Return value;
    void
*****/
FLASHMEM void UpdateInfoWindow() {
  tft.fillRect(INFORMATION_WINDOW_X - 8, INFORMATION_WINDOW_Y, 250, 170, RA8875_BLACK);    // Clear fields
  tft.drawRect(BAND_INDICATOR_X - 10, BAND_INDICATOR_Y - 2, 260, 200, RA8875_LIGHT_GREY);  // Redraw Info Window
}


/*****
  Purpose: Updates the states of the speaker and headphone.

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateAudioField() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(670, 365);
  tft.fillRect(670, 365, 115, 30, RA8875_BLACK);  // Erase previous states.
  switch (ConfigData.audioOut) {
    case AudioState::SPEAKER:
      tft.setTextColor(RA8875_WHITE);
      tft.print("Speaker On");
      tft.setCursor(670, 380);
      tft.setTextColor(RA8875_RED);
      tft.print("Headphone Mute");
      break;
    case AudioState::MUTE_BOTH:
      tft.setTextColor(RA8875_RED);
      tft.print("Speaker Mute");
      tft.setCursor(670, 380);
      tft.setTextColor(RA8875_RED);
      tft.print("Headphone Mute");
      break;
    case AudioState::HEADPHONE:
      tft.setTextColor(RA8875_RED);
      tft.print("Speaker Mute");
      tft.setCursor(670, 380);
      tft.setTextColor(RA8875_WHITE);
      tft.print("Headphone On");
      break;
    case AudioState::BOTH:
      tft.setTextColor(RA8875_WHITE);
      tft.print("Speaker On");
      tft.setCursor(670, 380);
      tft.print("Headphone On");
      break;
    default:
      break;
  }

  //  tft.setTextColor(RA8875_GREEN);
  //  tft.fillRect(BAND_INDICATOR_X + 90, BAND_INDICATOR_Y, tft.getFontWidth() * 3 + 2, tft.getFontHeight(), RA8875_BLACK);
  //  tft.setCursor(FIELD_OFFSET_X, BAND_INDICATOR_Y);
  //  tft.print(ConfigData.audioVolume);
}


/*****
  Purpose: Updates the Volume setting on the display

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateVolumeField() {
  tft.setFontScale((enum RA8875tsize)1);

  tft.setCursor(BAND_INDICATOR_X + 10, BAND_INDICATOR_Y);  // Volume
  tft.setTextColor(RA8875_WHITE);
  tft.print("Vol:");
  tft.setTextColor(RA8875_GREEN);
  tft.fillRect(BAND_INDICATOR_X + 80, BAND_INDICATOR_Y, tft.getFontWidth() * 3 + 2, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X - 10, BAND_INDICATOR_Y);
  tft.print(ConfigData.audioVolume);
}


void UpdateAGCField() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(AGC_X_OFFSET - 10, AGC_Y_OFFSET, tft.getFontWidth() * 7 + 2, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(BAND_INDICATOR_X + 133, BAND_INDICATOR_Y);
  if (ConfigData.AGCMode) {  // The option for AGC
    tft.setTextColor(RA8875_YELLOW);
    tft.print("AGC ON");
  } else {
    tft.setTextColor(DARKGREY);
    tft.print("AGC OFF");
  }
}


/*****
  Purpose: Updates the AGC on the display.  Long option added. G0ORX September 6, 2023

  Parameter list:
    void

  Return value;
    void
*****
void UpdateAGCField() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.fillRect(AGC_X_OFFSET - 10, AGC_Y_OFFSET, tft.getFontWidth() * 7, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(BAND_INDICATOR_X + 133, BAND_INDICATOR_Y);
  switch (ConfigData.AGCMode) {  // The option for AGC
    case 0:                      // Off
                                 //    tft.setCursor(BAND_INDICATOR_X + 140, BAND_INDICATOR_Y);
      tft.setTextColor(DARKGREY);
      tft.print("AGC OFF");
      //      tft.setFontScale((enum RA8875tsize)0);
      //      tft.setCursor(BAND_INDICATOR_X + 200, BAND_INDICATOR_Y + 15);
      //      tft.print(" off");
      //      tft.setFontScale((enum RA8875tsize)1);
      break;

    case 1:  // Long
      tft.setTextColor(RA8875_YELLOW);
      tft.print("AGC L");
      break;

    case 2:  // Slow
      tft.setTextColor(RA8875_WHITE);
      tft.print("AGC S");
      break;

    case 3:  // Medium
      tft.setTextColor(ORANGE);
      tft.print("AGC M");
      break;

    case 4:  // Fast
      tft.setTextColor(RA8875_GREEN);
      tft.print("AGC F");
      break;

    default:
      break;
  }
}
*/

/*****
  Purpose: AGC active indicator.

  Parameter list:
    void

  Return value;
    void
*****
void DisplayAGC() {
  if (ConfigData.AGCMode != 0) {  // Don't update AGC indicator if AGC is off.
    if (agc_action) tft.fillRect(765, AGC_Y_OFFSET + 10, 15, 15, RA8875_GREEN);
    else tft.fillRect(765, AGC_Y_OFFSET + 10, 15, 15, RA8875_BLACK);
  }
  //  Serial.printf("agc_action = %d\n", agc_action);
}
*/

/*****
  Purpose: Updates the increment setting on the display

  Parameter list:
    void

  Return value;
    void
*****/
void DisplayIncrementField() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);  // Frequency increment
  tft.setCursor(INCREMENT_X + 100, INCREMENT_Y - 1);
  tft.print("Coarse Inc: ");
  tft.setCursor(INCREMENT_X - 3, INCREMENT_Y - 1);
  tft.print("Fine Inc: ");

  tft.fillRect(INCREMENT_X + 67, INCREMENT_Y, tft.getFontWidth() * 4, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X - 23, INCREMENT_Y - 1);
  tft.setTextColor(RA8875_GREEN);
  tft.print(ConfigData.fineTuneStep);

  tft.fillRect(INCREMENT_X + 188, INCREMENT_Y, tft.getFontWidth() * 7 + 2, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X + 95, INCREMENT_Y - 1);
  tft.setTextColor(RA8875_GREEN);
  tft.print(ConfigData.centerTuneStep);
}


/*****
  Purpose: Updates the notch value on the display

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateNotchField() {
  tft.setFontScale((enum RA8875tsize)0);

  if (NR_first_time == 0) {  // Notch setting
    tft.setTextColor(RA8875_LIGHT_GREY);
  } else {
    tft.setTextColor(RA8875_WHITE);
  }
  tft.fillRect(NOTCH_X + 60, NOTCH_Y - 3, 150, tft.getFontHeight() + 5, RA8875_BLACK);
  tft.setCursor(NOTCH_X - 27, NOTCH_Y - 2);
  tft.print("AutoNotch:");
  tft.setCursor(FIELD_OFFSET_X, NOTCH_Y - 2);
  tft.setTextColor(RA8875_GREEN);
  if (ANR_notch == false) {
    tft.print("Off");
  } else {
    tft.print("On");
  }
}

/*****
  Purpose: Updates the zoom setting on the display

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateZoomField() {
  tft.setFontScale((enum RA8875tsize)0);

  tft.fillRect(ZOOM_X, ZOOM_Y, 80, tft.getFontHeight(), RA8875_BLACK);
  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(ZOOM_X + 5, ZOOM_Y - 4);
  tft.print("Zoom:");
  tft.setCursor(FIELD_OFFSET_X, ZOOM_Y - 4);
  tft.setTextColor(RA8875_GREEN);
  tft.print(zoomOptions[zoomIndex]);
}


/*****
  Purpose: Updates the compression setting in the info window.

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateCompressionField()  // JJP 8/26/2023
{
  tft.fillRect(COMPRESSION_X, COMPRESSION_Y - 2, 175, 15, RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(COMPRESSION_X + 5, COMPRESSION_Y - 5);
  tft.print("Compress:");
  tft.setCursor(FIELD_OFFSET_X, COMPRESSION_Y - 5);
  tft.setTextColor(RA8875_GREEN);
  if (ConfigData.compressorFlag == true) {  // JJP 8/26/2023
    tft.print("On  ");
    tft.print(ConfigData.micCompRatio, 0);  // This is the compression ratio of the Open Audio compressor.  Greg KF5N July 22, 2024
  } else {
    tft.setCursor(FIELD_OFFSET_X, COMPRESSION_Y - 5);
    tft.print("Off");
  }
}


/*****
  Purpose: Updates whether the decoder is on or off and decoder related graphics including CW, highpass, and lowpass filter bandwidths.

  Parameter list:
    void

  Return value;
    void
*****/
FLASHMEM void UpdateAudioGraphics() {
  float CWFilterPosition = 0.0;
  // Update text
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(DECODER_X + 5, DECODER_Y - 5);
  tft.print("Decoder:");
  tft.setTextColor(RA8875_GREEN);
  //  tft.fillRect(DECODER_X + 90, DECODER_Y, tft.getFontWidth() * 20, tft.getFontHeight() + 2, RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X, DECODER_Y - 5);
  tft.fillRect(FIELD_OFFSET_X, DECODER_Y - 5, 140, 17, RA8875_BLACK);  // Erase

  // Update text and graphics.  Use a method of clearing everything, and then add only what is needed.
  tft.writeTo(L2);  // This has to be written to L2 or it will be erased by the audio spectrum eraser.
  tft.clearMemory();

  // Update CW decoder status in information window.
  if (ConfigData.decoderFlag) {
    tft.setCursor(FIELD_OFFSET_X, DECODER_Y - 5);
    tft.print("    WPM");
  } else {
    tft.fillRect(FIELD_OFFSET_X, DECODER_Y - 5, 140, 17, RA8875_BLACK);  // Erase
    tft.print("Off");
  }

  //  The following code was moved from ShowSpectrum() in Display.cpp.
  int filterLoPositionMarker{ 0 };
  int filterHiPositionMarker{ 0 };
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    filterLoPositionMarker = map(bands.bands[ConfigData.currentBand].FLoCut, 0, 6000, 0, 256);
    filterHiPositionMarker = map(bands.bands[ConfigData.currentBand].FHiCut, 0, 6000, 0, 256);
  } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    filterHiPositionMarker = map(bands.bands[ConfigData.currentBand].FAMCut, 0, 6000, 0, 256);
  }

  //Draw Filter indicator lines on audio plot to Layer 2.
  tft.writeTo(L2);
  // The encoder should adjust the low side of the filter if the CW filter is on.  This creates a tunable bandpass.  Sort of.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE and ConfigData.CWFilterIndex != 5) switchFilterSideband = true;

  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    if (not switchFilterSideband) {
      tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_LIGHT_GREY);
      tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_RED);
    } else {
      tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_RED);
      tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_LIGHT_GREY);
    }
  }

  // In AM modes draw high delimiter only and always make it red (active);
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 3, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 112, RA8875_RED);
  }

  // Select the x-axis position for the CW filter box and delimiter line (on the right side of the box).
  switch (ConfigData.CWFilterIndex) {
    case 0:
      CWFilterPosition = 35.7;  // 0.84 * 42.5;
      break;
    case 1:
      CWFilterPosition = 42.5;
      break;
    case 2:
      CWFilterPosition = 55.25;  // 1.3 * 42.5;
      break;
    case 3:
      CWFilterPosition = 76.5;  // 1.8 * 42.5;
      break;
    case 4:
      CWFilterPosition = 85.0;  // 2.0 * 42.5;
      break;
    case 5:
      CWFilterPosition = 0.0;
      break;
  }

  // Draw CW filter box if required.  Don't do this if the CW filter is off.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE and ConfigData.CWFilterIndex != 5) {
    tft.writeTo(L2);  // This has to be written to L2 or it will be erased by the audio spectrum eraser.
    tft.fillRect(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), AUDIO_SPECTRUM_TOP, CWFilterPosition - abs(filterLoPositionMarker), 120, MAROON);
    tft.drawFastVLine(BAND_INDICATOR_X - 8 + CWFilterPosition, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_LIGHT_GREY);
  }

  // Draw decoder delimiters if the decoder is on.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE and ConfigData.decoderFlag) {
    // Draw delimiter bars for CW offset frequency.  This depends on the user selected offset.
    if (ConfigData.CWOffset == 0) {
      tft.drawFastVLine(BAND_INDICATOR_X + 15, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator
      tft.drawFastVLine(BAND_INDICATOR_X + 21, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator
    }
    if (ConfigData.CWOffset == 1) {
      tft.drawFastVLine(BAND_INDICATOR_X + 18, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator
      tft.drawFastVLine(BAND_INDICATOR_X + 24, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator
    }
    if (ConfigData.CWOffset == 2) {
      tft.drawFastVLine(BAND_INDICATOR_X + 23, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator
      tft.drawFastVLine(BAND_INDICATOR_X + 29, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator
    }
    if (ConfigData.CWOffset == 3) {
      tft.drawFastVLine(BAND_INDICATOR_X + 26, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator
      tft.drawFastVLine(BAND_INDICATOR_X + 32, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator
    }
  }

  // Update stuff including graphics that got erase by this function.
  BandInformation();
  ShowBandwidth();
  DrawFrequencyBarValue();  // This calls ShowBandwidth().  YES, this function is useful here.
  DrawBandWidthIndicatorBar();
  tft.writeTo(L1);
}


/*****
  Purpose: Updates the Rx and Tx equalizer states
           shown on the display.

  Parameter list:
    bool rxEqState, txEqState

  Return value;
    void
*****/
FLASHMEM void UpdateEqualizerField(bool rxEqState, bool txEqState) {
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(FIELD_OFFSET_X, DECODER_Y + 15, tft.getFontWidth() * 15, tft.getFontHeight() + 2, RA8875_BLACK);
  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(547, DECODER_Y + 15);
  tft.print("Equalizer:");
  tft.setCursor(FIELD_OFFSET_X, DECODER_Y + 15);
  if (rxEqState) {
    tft.setTextColor(RA8875_GREEN);
    tft.print("Rx");
    tft.setTextColor(RA8875_WHITE);
    tft.setCursor(FIELD_OFFSET_X + 25, DECODER_Y + 15);
    tft.print("On");
  } else {
    tft.setTextColor(RA8875_RED);
    tft.print("Rx");
    tft.setCursor(FIELD_OFFSET_X + 25, DECODER_Y + 15);
    tft.setTextColor(RA8875_WHITE);
    tft.print("Off");
  }

  tft.setCursor(FIELD_OFFSET_X + 55, DECODER_Y + 15);
  if (txEqState) {
    tft.setTextColor(RA8875_GREEN);
    tft.print("Tx");
    tft.setTextColor(RA8875_WHITE);
    tft.setCursor(FIELD_OFFSET_X + 80, DECODER_Y + 15);
    tft.print("On");
  } else {
    tft.setTextColor(RA8875_RED);
    tft.print("Tx");
    tft.setTextColor(RA8875_WHITE);
    tft.setCursor(FIELD_OFFSET_X + 80, DECODER_Y + 15);
    tft.print("Off");
  }
}


/*****
  Purpose: Updates the Keyer and WPM setting on the display

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateWPMField() {
  tft.setFontScale((enum RA8875tsize)0);

  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(WPM_X + 5, WPM_Y - 5);
  tft.print("Keyer:");
  tft.setTextColor(RA8875_GREEN);
  tft.fillRect(WPM_X + 59, WPM_Y - 4, tft.getFontWidth() * 15, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X, WPM_Y - 5);
  if (ConfigData.keyType == 1) {  // 1 is keyer.
    // KD0RC start
    tft.print("Paddles ");
    if (ConfigData.paddleFlip == 0) {
      tft.print("R");
    } else {
      tft.print("L");
    }
    tft.print(" ");
    // KD0RC end
    tft.print(ConfigData.currentWPM);
  } else {
    tft.print("Straight Key");
  }
}


/*****
  Purpose: Updates the noise field on the display

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateNoiseField() {
  const char *filter[] = { "Off", "Kim", "Spec", "LMS" };  //AFP 09-19-22
  tft.setFontScale((enum RA8875tsize)0);
  tft.fillRect(FIELD_OFFSET_X, NOISE_REDUCE_Y, 35, tft.getFontHeight(), RA8875_BLACK);
  tft.setTextColor(RA8875_WHITE);  // Noise reduction
  tft.setCursor(NOISE_REDUCE_X + 5, NOISE_REDUCE_Y - 3);
  tft.print("Noise:");
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(FIELD_OFFSET_X, NOISE_REDUCE_Y - 3);
  tft.print(filter[ConfigData.nrOptionSelect]);
}


/*****
  Purpose: This function draws the Info Window frame

  Parameter list:
    void

  Return value;
    void
*****/
void DrawInfoWindowFrame() {
  tft.drawRect(BAND_INDICATOR_X - 10, BAND_INDICATOR_Y - 2, 260, 200, RA8875_LIGHT_GREY);
  tft.fillRect(TEMP_X_OFFSET, TEMP_Y_OFFSET + 80, 80, tft.getFontHeight() + 10, RA8875_BLACK);  // Clear volume field
}


/*****
  Purpose: This function redraws the entire display screen.

  Parameter list:
    void

  Return value;
    void
*****/
void RedrawDisplayScreen() {
  tft.fillWindow();  // Clear the display.
  DrawAudioSpectContainer();
  DrawSpectrumDisplayContainer();
  ShowSpectrumdBScale();
  DrawSMeterContainer();
  ShowName();
  UpdateInfoWindow();
  //  Initial values to display.
  ShowFrequency();
  ShowBandwidth();
  BandInformation();
  DrawBandWidthIndicatorBar();
  ShowAutoStatus();
  UpdateAGCField();
  UpdateVolumeField();
  DisplayIncrementField();
  UpdateNotchField();
  UpdateNoiseField();
  UpdateZoomField();
  UpdateCompressionField();
  UpdateWPMField();
  UpdateAudioGraphics();
  UpdateEqualizerField(ConfigData.receiveEQFlag, ConfigData.xmitEQFlag);
  UpdateAudioField();
  ShowCurrentPowerSetting();
  SetBandRelay();                   // Set LPF relays for current band.
  lastState = RadioState::NOSTATE;  // Force an update.
}


/*****
  Purpose: Draw Tuned Bandwidth on Spectrum Plot. // Calculations simplified KF5N April 22, 2024

  Parameter list:

  Return value;
    void
*****/
void DrawBandWidthIndicatorBar()  // AFP 10-30-22
{
  int Zoom1Offset = 0.0;
  float hz_per_pixel = 0.0;

  switch (zoomIndex) {
    case 0:  // 1X
      hz_per_pixel = 375.0;
      Zoom1Offset = -128;  // The "center" is halfway to the left of the spectrum window center.  // KF5N April 23, 2024
      break;

    case 1:  // 2X
      hz_per_pixel = 187.5;
      Zoom1Offset = 0;
      break;

    case 2:  // 4X
      hz_per_pixel = 93.75;
      Zoom1Offset = 0;
      break;

    case 3:  //  8X
      hz_per_pixel = 46.875;
      Zoom1Offset = 0;
      break;

    case 4:  // 16X
      hz_per_pixel = 23.4375;
      Zoom1Offset = 0;
      break;
  }
  newCursorPosition = static_cast<int>(NCOFreq / hz_per_pixel) + Zoom1Offset;  // More accurate tuning bar position.  KF5N May 17, 2024
  tft.writeTo(L2);                                                             // Write graphics to Layer 2.
  //  tft.clearMemory();              // This destroys the CW filter graphics, removed.  KF5N July 30, 2023
  //  tft.clearScreen(RA8875_BLACK);  // This causes an audio hole in fine tuning.  KF5N 7-16-23

  pixel_per_khz = ((1 << ConfigData.spectrum_zoom) * SPECTRUM_RES * 1000.0 / SR[SampleRate].rate);
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
    filterWidth = static_cast<int>(((bands.bands[ConfigData.currentBand].FHiCut - bands.bands[ConfigData.currentBand].FLoCut) / 1000.0) * pixel_per_khz * 1.06);  // AFP 10-30-22
  else if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM)
    filterWidth = static_cast<int>(((bands.bands[ConfigData.currentBand].FAMCut * 2.0) / 1000.0) * pixel_per_khz * 1.06);  // AFP 10-30-22

  switch (bands.bands[ConfigData.currentBand].sideband) {
    case Sideband::LOWER:
      tft.fillRect(centerLine - filterWidth + oldCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth * 1.0, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  // Was 0.96.  KF5N July 31, 2023
      tft.fillRect(centerLine - filterWidth + newCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);
      break;

    case Sideband::UPPER:
      tft.fillRect(centerLine + oldCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  //AFP 03-27-22 Layers
      tft.fillRect(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);    //AFP 03-27-22 Layers                                                                                                                           //      tft.drawFastVLine(centerLine + oldCursorPosition, SPECTRUM_TOP_Y + 20, h - 10, RA8875_BLACK); // Yep. Erase old, draw new...//AFP 03-27-22 Layers                                                                                                                          //      tft.drawFastVLine(centerLine + newCursorPosition , SPECTRUM_TOP_Y + 20, h - 10, RA8875_CYAN); //AFP 03-27-22 Layers
      break;

    case Sideband::BOTH_AM:
    case Sideband::BOTH_SAM:
      tft.fillRect(centerLine - filterWidth / 2 + oldCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);              //AFP 10-30-22
      tft.fillRect(centerLine - filterWidth / 2 * 0.93 + newCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth * 0.95, SPECTRUM_HEIGHT - 20, FILTER_WIN);  //AFP 10-30-22
                                                                                                                                                         //      tft.drawFastVLine(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, h - 10, RA8875_CYAN);                 //AFP 10-30-22*/
      break;

    default:
      break;
  }

  tft.drawFastVLine(centerLine + oldCursorPosition, SPECTRUM_TOP_Y + 20, h - 10, RA8875_BLACK);  // refactored from above JJP 7/12/23
  tft.drawFastVLine(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, h - 10, RA8875_CYAN);
  oldCursorPosition = newCursorPosition;
  tft.writeTo(L1);  //AFP 03-27-22 Layers
}

/*****
  Purpose: This function removes the spectrum display container

  Parameter list:
    void

  Return value;
    void
*****/
void EraseSpectrumDisplayContainer() {
  tft.fillRect(SPECTRUM_LEFT_X - 2, SPECTRUM_TOP_Y - 1, MAX_WATERFALL_WIDTH + 6, SPECTRUM_HEIGHT + 8, RA8875_BLACK);  // Spectrum box
}


/*****
  Purpose: This function erases the contents of the spectrum display

  Parameter list:
    void

  Return value;
    void
*****/
void EraseSpectrumWindow() {
  tft.fillRect(SPECTRUM_LEFT_X, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH, SPECTRUM_HEIGHT, RA8875_BLACK);  // Spectrum box
}


/*****
  Purpose: To erase both primary and secondary menus from display

  Parameter list:

  Return value;
    void
*****/
void EraseMenus() {
  tft.fillRect(PRIMARY_MENU_X, MENUS_Y, BOTH_MENU_WIDTHS, CHAR_HEIGHT + 1, RA8875_BLACK);  // Erase menu choices
}


/*****
  Purpose: To erase primary menu from display

  Parameter list:

  Return value;
    void
*****/
void ErasePrimaryMenu() {
  tft.fillRect(PRIMARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT + 1, RA8875_BLACK);  // Erase menu choices
  //  menuStatus = NO_MENUS_ACTIVE;                                                           // Change menu state
}


/*****
  Purpose: To erase secondary menu from display

  Parameter list:

  Return value;
    void
*****/
void EraseSecondaryMenu() {
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT + 1, RA8875_BLACK);  // Erase menu choices
  //  menuStatus = NO_MENUS_ACTIVE;                                                             // Change menu state
}


/*****
  Purpose: Shows transmit (red) and receive (green) mode

  Parameter list:

  Return value;
    void
*****/
void ShowTransmitReceiveStatus() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_BLACK);
  if (radioState == RadioState::SSB_TRANSMIT_STATE or radioState == RadioState::FT8_TRANSMIT_STATE or radioState == RadioState::CW_TRANSMIT_STRAIGHT_STATE
      or radioState == RadioState::CW_TRANSMIT_KEYER_STATE or radioState == RadioState::CW_CALIBRATE_STATE or radioState == RadioState::SSB_CALIBRATE_STATE) {
    tft.fillRect(X_R_STATUS_X, X_R_STATUS_Y, 55, 25, RA8875_RED);
    tft.setCursor(X_R_STATUS_X + 4, X_R_STATUS_Y - 5);
    tft.print("XMT");
  } else {
    tft.fillRect(X_R_STATUS_X, X_R_STATUS_Y, 55, 25, RA8875_GREEN);
    tft.setCursor(X_R_STATUS_X + 4, X_R_STATUS_Y - 5);
    tft.print("REC");
  }
}
