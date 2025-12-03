// Display class

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


/*****
  Purpose: Draw audio spectrum box.  AFP added 3-14-21

  Parameter list:

  Return value;
    void
*****/
void Display::DrawAudioSpectContainer() {
  tft.writeTo(L1);
  tft.drawRect(BAND_INDICATOR_X - 9, SPECTRUM_BOTTOM - 118, 255, 118, RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);
  for (int k = 0; k < 6; k++) {
    tft.drawFastVLine(BAND_INDICATOR_X - 9 + k * 43.8, SPECTRUM_BOTTOM, 15, RA8875_GREEN);
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
void Display::ShowName() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_YELLOW, RA8875_BLACK);
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
void Display::ShowSpectrum(bool drawSpectrum) {

  int AudioH_max = 0, AudioH_max_box = 0;  // Used to center audio spectrum.
  int audio_hist[256]{ 0 };                // All values are initialized to zero using this syntax.
  int k;
  int middleSlice = centerLine / 2;  // Approximate center element
  int x1 = 0;                        //AFP
                                     //  int h = SPECTRUM_HEIGHT + 7;
  int y1_new{ 0 }, y2_new{ 0 }, y1_old{ 0 }, y2_old{ 0 };
  int test1;
  updateDisplayCounter = 0;
  updateDisplayFlag = false;
  spectrumErased = false;
  bool plotInProgress = false;
  bool processed = false;
  bool blocksAvailable = false;

  for (x1 = 1; x1 < 511; x1++)  // Bins on the ends are junk, don't plot.
  //Draws the main Spectrum, Waterfall and Audio displays
  {
    // Is there enough data to calculate an FFT?
    blocksAvailable = (static_cast<uint32_t>(ADC_RX_I.available()) > 15) and (static_cast<uint32_t>(ADC_RX_Q.available()) > 15);

    // 1X zoom.
    if ((ConfigData.spectrum_zoom == 0) and (not plotInProgress) and blocksAvailable) {
      x1 = 1;  // This forces enough calls of ProcessIQData() to generate enough blocks before x1 > 511.
      updateDisplayFlag = true;
      plotInProgress = true;
    }
    // 2X zoom.
    if ((ConfigData.spectrum_zoom == 1) and (not plotInProgress) and blocksAvailable) {
      x1 = 1;
      updateDisplayFlag = true;
      plotInProgress = true;
    }
    // 4X zoom.
    if ((ConfigData.spectrum_zoom == 2) and (not plotInProgress) and blocksAvailable) {
      x1 = 1;
      updateDisplayFlag = true;
      plotInProgress = true;
    }
    // 8X zoom.
    if ((ConfigData.spectrum_zoom == 3) and (not plotInProgress) and blocksAvailable) {
      updateDisplayCounter = updateDisplayCounter + 1;
      x1 = 1;  // This forces enough calls of ProcessIQData() to generate enough blocks before x1 > 511.
      if (updateDisplayCounter == 3) {
        updateDisplayFlag = true;
        plotInProgress = true;
      }
    }
    // 16X zoom.
    if ((ConfigData.spectrum_zoom == 4) and (not plotInProgress) and blocksAvailable) {
      updateDisplayCounter = updateDisplayCounter + 1;
      x1 = 1;  // This forces enough calls of ProcessIQData() to generate enough blocks before x1 > 511.
      if (updateDisplayCounter == 7) {
        updateDisplayFlag = true;
        plotInProgress = true;
      }
    }

    if (startRxFlag) updateDisplayFlag = false;  // Don't process data the first time after coming out of transmit mode.
    startRxFlag = false;

    // This has to be repeatedly called for continuous audio.
    processed = process.ProcessIQData();

    // Don't call this function unless the filter bandwidth has been adjusted.
    if (encoderFilterFlag) {
      FilterSetSSB();
    }

    EncoderCenterTune();  //Moved the tuning encoder to reduce lag times and interference during tuning.

    if (not plotInProgress) {
      delayMicroseconds(100);  // WOW we have to delay to allow the data to catch up!
      continue;                // We are not plotting, so go to the next iteration.  Skip the following plotting stuff.
    }

    y1_new = pixelnew[x1];
    y2_new = pixelnew[x1 + 1];
    y1_old = pixelold[x1];  // pixelold spectrum is saved by the FFT function prior to a new FFT which generates the pixelnew spectrum.  KF5N
    y2_old = pixelold[x1 + 1];

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
    if (y1_new > 247) y1_new = 247;
    if (y2_new > 247) y2_new = 247;
    if (y1_old > 247) y1_old = 247;
    if (y2_old > 247) y2_old = 247;

    // Prevent spectrum from going above the top of the spectrum area.  KF5N
    if (y1_new < 120) y1_new = 120;
    if (y2_new < 120) y2_new = 120;
    if (y1_old < 120) y1_old = 120;
    if (y2_old < 120) y2_old = 120;

    // Erase the old RF spectrum, and draw the new RF spectrum.
    // Note the FFT bins are offset by x = 3 in the display.
    // Don't overwrite or erase the center line.
    if (drawSpectrum and x1 != (centerLine - 3)) {
      tft.drawLine(x1 + 3, y1_old, x1 + 3, y2_old, RA8875_BLACK);   // Erase old...
      tft.drawLine(x1 + 3, y1_new, x1 + 3, y2_new, RA8875_YELLOW);  // Draw new
    }

    // What is the actual spectrum at this time?  It's a combination of the old and new spectrums.
    // In the case of a CW interrupt, the array pixelnew should be saved as the actual spectrum.
    // This is the actual "old" spectrum!  This is required due to CW interrupts.  pixelCurrent
    // gets copied to pixelold by the FFT function.  Greg KF5N

    audioYPixelcurrent[x1] = audioYPixel[x1];

    if (keyPressedOn) {
      //  Quickly erase the spectrum on the display.
      for (int i = 0; i < 511; i = i + 1) {
        if (i == (centerLine - 3)) continue;  // Don't erase center line.
        // Erase old spectrum.
        if (i > x1) {
          y1_old = 247 - pixelold[i];
          y2_old = 247 - pixelold[i + 1];
          y1_old = (y1_old > 247) ? 247 : y1_old;
          y2_old = (y2_old > 247) ? 247 : y2_old;
          y1_old = (y1_old < 120) ? 120 : y1_old;
          y2_old = (y2_old < 120) ? 120 : y2_old;
          tft.drawLine(i + 3, y1_old, i + 3, y2_old, RA8875_BLACK);
        }
        // Erase the most recently plotted spectrum.
        if (i <= x1) {
          y1_old = 247 - pixelnew[i];
          y2_old = 247 - pixelnew[i + 1];
        }
        y1_old = (y1_old > 247) ? 247 : y1_old;
        y2_old = (y2_old > 247) ? 247 : y2_old;
        y1_old = (y1_old < 120) ? 120 : y1_old;
        y2_old = (y2_old < 120) ? 120 : y2_old;
        tft.drawLine(i + 3, y1_old, i + 3, y2_old, RA8875_BLACK);
      }
      spectrumErased = true;

      return;  // Bail out of receive and quickly go to transmit mode.
    }

    // Draw audio spectrum.  The audio spectrum width is smaller than the RF spectrum width.
    // The audio spectrum arrays are generated in ReceiveDSP.cpp by method ProcessIQData().
    if (x1 < 253) {                                                                               //AFP 09-01-22
      if (keyPressedOn == true) {                                                                 //AFP 09-01-22
                                                                                                  ////        return;
      } else {                                                                                    //AFP 09-01-22
        if (audioYPixelold[x1] > CLIP_AUDIO_PEAK) audioYPixelold[x1] = CLIP_AUDIO_PEAK;           // audioSpectrumHeight = 118
        tft.drawFastVLine(532 + x1, 245 - audioYPixelold[x1], audioYPixelold[x1], RA8875_BLACK);  // Erase
        if (audioYPixel[x1] != 0) {
          if (audioYPixel[x1] > CLIP_AUDIO_PEAK)  // audioSpectrumHeight = 118
            audioYPixel[x1] = CLIP_AUDIO_PEAK;
          if (x1 == middleSlice) {
            smeterLength = y_new;
          }
          // Draw a vertical line with the audio spectrum magnitude.  AUDIO_SPECTRUM_BOTTOM = 247
          tft.drawFastVLine(532 + x1, 245 - audioYPixel[x1], audioYPixel[x1], RA8875_MAGENTA);
        }
      }
    }

    test1 = -y1_new + 230;  // Nudged waterfall towards blue.  KF5N July 23, 2023
    if (test1 < 0) test1 = 0;
    if (test1 > 117) test1 = 117;
    waterfall[x1] = gradient[test1];  // Try to put pixel values in middle of gradient array.  KF5N
    tft.writeTo(L1);
  }  // End for(...) Draw MAX_WATERFALL_WIDTH spectral points

  // Use the Block Transfer Engine (BTE) to move waterfall down a line
  tft.BTE_move(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE, MAX_WATERFALL_WIDTH, MAX_WATERFALL_ROWS - 2, WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, 1, 2);
  while (tft.readStatus())
    ;  // Make sure it is done.  Memory moves can take time.

  // Now bring waterfall back to the beginning of the 2nd row.
  tft.BTE_move(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, MAX_WATERFALL_WIDTH, MAX_WATERFALL_ROWS - 2, WATERFALL_LEFT_X, FIRST_WATERFALL_LINE + 1, 2);
  while (tft.readStatus())
    ;  // Make sure it's done.

  // Then write new row data into the missing top row to get a scroll effect using display hardware, not the CPU.
  tft.writeRect(WATERFALL_LEFT_X, FIRST_WATERFALL_LINE, MAX_WATERFALL_WIDTH, 1, waterfall);

  // Manage audio spectral display graphics.  Keep the spectrum within the viewable area.
  if (AudioH_max_box > 30) {  // HB. Adjust rfGainAllBands 15 and 13 to alter to move target base up and down. UPPERPIXTARGET = 15
    audioFFToffset = audioFFToffset - 1;
  }
  if (AudioH_max_box < 28) {  // LOWERPIXTARGET = 13
    audioFFToffset = audioFFToffset + 1;
  }

}  // End ShowSpectrum()


/*****
  Purpose: Show filter bandwidth settings near the top center of the RF spectrum.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::ShowRFGain() {
  // Report the current RF "gain" setting.
  //  tft.fillRect(SPECTRUM_LEFT_X + 125, SPECTRUM_TOP_Y + 2, 33, tft.getFontHeight(), RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.setCursor(SPECTRUM_LEFT_X + 61, SPECTRUM_TOP_Y + 2);
  tft.print("RF GAIN ");
  //  tft.setCursor(SPECTRUM_LEFT_X + 129, SPECTRUM_TOP_Y + 2);
  tft.print(ConfigData.rfGain[ConfigData.currentBand]);
  tft.print("  ");
}


/*****
  Purpose: Show filter bandwidth settings near the top center of the RF spectrum.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::ShowBandwidth() {
  tft.writeTo(L2);  // Write to layer 2.
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_LIGHT_GREY, RA8875_BLACK);

  // Don't show HPF filter setting in AM modes, because it doesn't exist yet.
  if (bands.bands[ConfigData.currentBand].mode != RadioMode::SAM_MODE and bands.bands[ConfigData.currentBand].mode != RadioMode::AM_MODE) {
    tft.setCursor(158, 102);
    tft.print("HPF = ");
    tft.print(static_cast<float>(bands.bands[ConfigData.currentBand].FLoCut) / 1000.0f, 1);
    tft.print("kHz");
  }

  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SAM_MODE or bands.bands[ConfigData.currentBand].mode == RadioMode::AM_MODE) {
    tft.setCursor(265, 102);
    tft.print("LPF = ");
    tft.print(static_cast<float>(bands.bands[ConfigData.currentBand].FAMCut) / 1000.0f, 1);
    tft.print("kHz");
  } else {
    tft.setCursor(265, 102);
    tft.print("LPF = ");
    tft.print(static_cast<float>(bands.bands[ConfigData.currentBand].FHiCut) / 1000.0f, 1);
    tft.print("kHz");
  }
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);  // set text color to white for other print routines not to get confused ;-)
  tft.writeTo(L1);
}


//DB2OO, 30-AUG-23: this variable determines the pixels per S step. In the original code it was 12.2 pixels !?
#ifdef TCVSDR_SMETER
const float pixels_per_s = 12;
#else
const float pixels_per_s = 12.2;
#endif
/*****
  Purpose: Draw the S-meter container.
  Parameter list:
    void
  Return value;
    void
*****/
void Display::DrawSMeterContainer() {
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
void Display::ShowSpectrumdBScale() {
  tft.setFontScale((enum RA8875tsize)0);
  //  tft.fillRect(SPECTRUM_LEFT_X + 1, SPECTRUM_TOP_Y + 2, 33, tft.getFontHeight(), RA8875_BLACK);
  tft.setCursor(SPECTRUM_LEFT_X + 5, SPECTRUM_TOP_Y + 2);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.print("10 dB/");
}


/*****
  Purpose: This function draws spectrum display container.
  Parameter list:
    void
  Return value;
    void
*****/
void Display::DrawSpectrumDisplayContainer() {
  tft.writeTo(L1);  // Draw on L1 so it won't interfere with the blue tuning bar.  Don't let the spectrum overwrite the X position in ShowSpectrum().
  if (calOnFlag)
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, 362, RA8875_YELLOW);  // Spectrum box for calibration.
  else {
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, 362, RA8875_BLACK);               // Erase spectrum box for calibration.
    tft.drawRect(SPECTRUM_LEFT_X - 1, SPECTRUM_TOP_Y, MAX_WATERFALL_WIDTH + 2, SPECTRUM_HEIGHT, RA8875_YELLOW);  // Spectrum box.  SPECTRUM_HEIGHT = 150
    tft.drawFastVLine(centerLine, SPECTRUM_TOP_Y, h + 20, RA8875_GREEN);                                         // Draws centerline on spectrum display.
  }
}


/*****
  Purpose: This function draws the frequency bar at the bottom of the spectrum scope, putting markers at every
            graticule and the full frequency.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::DrawFrequencyBarValue() {
  int centerIdx{ 0 };
  float disp_freq{ 0.0 };
  float freq_calc{ 0.0 };
  float grat(0.0);
  const uint32_t graticulePos[7]{ 2, 87, 174, 258, 343, 427, 515 };
  const uint32_t cursorPos[7]{ 0, 76, 153, 233, 314, 391, 463 };

  // Used to calculate graticule frequency in most cases.  Zoom = 1 is the exception.
  grat = static_cast<float>(SR[SampleRate].rate) / 6000.0 / static_cast<float>(1 << ConfigData.spectrum_zoom);

  tft.writeTo(L1);  // Not writing to correct layer?  KF5N.  July 31, 2023
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.setFontScale((enum RA8875tsize)0);

  freq_calc = static_cast<float>(ConfigData.centerFreq) / 1000.0;  // Current frequency in kHz.

  if (ConfigData.spectrum_zoom != 0) centerIdx = 0;
  else centerIdx = -2;

  //    CENTER FREQUENCY PRINT
  disp_freq = freq_calc + (centerIdx * grat);
  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);

  if (ConfigData.spectrum_zoom == 0) {   // Zoom == 1
    tft.setCursor(75, WATERFALL_TOP_Y);  // Erase other zoom's frequencies.
    tft.print("                 ");
    // Also erase graticules not used with 1X zoom.
    tft.drawFastVLine(graticulePos[1], WATERFALL_TOP_Y - 5, 7, RA8875_BLACK);
    tft.drawFastVLine(graticulePos[2], WATERFALL_TOP_Y - 5, 7, RA8875_BLACK);
    tft.setCursor(centerLine - 148, WATERFALL_TOP_Y);
    tft.print("       ");
    tft.setCursor(centerLine - 148, WATERFALL_TOP_Y);
  } else {
    tft.setCursor(centerLine - 148, WATERFALL_TOP_Y);  // Erase Zoom == 1 center frequency first!
    tft.print("       ");
    tft.drawFastVLine(130, WATERFALL_TOP_Y - 5, 7, RA8875_BLACK);  // Erase center frequency graticule for 1X zoom.
    tft.setCursor(centerLine - 25, WATERFALL_TOP_Y);               // centerLine = 258
    tft.print("       ");
    tft.setCursor(centerLine - 25, WATERFALL_TOP_Y);
  }
  tft.print(freq_calc, 1);

  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);

  // Print the frequencies.
  // Zoom == 1.  This is a special case, and the x-axis is weird.
  if (ConfigData.spectrum_zoom == 0) {
    for (int idx = -3; idx < 4; idx++) {
      // Don't print the center frequency, it was printed above.  Also skip the 2nd and 3rd.
      if (idx == -3 or idx >= 0) {
        if (idx == -3) disp_freq = freq_calc - 48.0;  // Farthest left.
        if (idx >= 0) disp_freq = freq_calc + (idx * grat) + 48.0;
        tft.setCursor(cursorPos[idx + 3], WATERFALL_TOP_Y);
        tft.print(disp_freq, 1);
        tft.print(" ");
        tft.drawFastVLine(graticulePos[idx + 3], WATERFALL_TOP_Y - 5, 7, RA8875_YELLOW);
      }
    }
    tft.drawFastVLine(130, WATERFALL_TOP_Y - 5, 7, RA8875_YELLOW);  // Center frequency graticule for 1X zoom.
  }

  // Zooms not equal to 1:
  if (ConfigData.spectrum_zoom != 0) {
    for (int idx = -3; idx < 4; idx++) {
      // Don't print the center frequency, it was printed above.
      if (idx != centerIdx) {
        disp_freq = freq_calc + (idx * grat);
        if (ConfigData.spectrum_zoom != 0) tft.setCursor(cursorPos[idx + 3], WATERFALL_TOP_Y);
        tft.print(disp_freq, 1);
        tft.print(" ");
        tft.drawFastVLine(graticulePos[idx + 3], WATERFALL_TOP_Y - 5, 7, RA8875_YELLOW);
      }
    }
  }

  tft.writeTo(L1);  // Always leave on layer 1.  KF5N.  July 31, 2023
  tft.setFontScale((enum RA8875tsize)1);
}


/*****
  Purpose:  Indicate Auto-Gain or Auto-Spectrum is active.

  Parameter list:
    void

  Return value;
    void
*****
void Display::ShowAutoStatus() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(SPECTRUM_LEFT_X + 350, SPECTRUM_TOP_Y + 2);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  if (ConfigData.autoGain) {
    tft.print("Auto-Gain On    ");
  } else if (ConfigData.autoSpectrum) {
    tft.print("Auto-Spectrum On");
  } else
    tft.print("                ");
}
*/

/*****
  Purpose: To display the current transmission frequency, band, mode, and sideband above the spectrum display.

  Parameter list:
    void

  Return value;
    void

*****/
void Display::BandInformation() {
  std::string CWFilter[] = { "0.8kHz", "1.0kHz", "1.3kHz", "1.8kHz", "2.0kHz", " Off  " };

  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
  tft.setCursor(5, FREQUENCY_Y + 30);
  // Need to erase all old information.
  tft.print("                                                  ");
  tft.setCursor(5, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.print("Center Freq ");  // This is static, never changes.
                              //  tft.fillRect(100, FREQUENCY_Y + 31, 290, 15, RA8875_BLACK);  // Clear frequency, band, and mode.  This should be the only erase required.

  // Write the center frequency to the display.
  //  tft.setCursor(100, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_LIGHT_ORANGE, RA8875_BLACK);
  if (ConfigData.spectrum_zoom == SPECTRUM_ZOOM_1) {  // AFP 11-02-22
    tft.print(static_cast<int>(ConfigData.centerFreq) + 48000);
  } else {
    tft.print(static_cast<int>(ConfigData.centerFreq));
  }

  // Write the band to the display.
  tft.print("  ");
  tft.setTextColor(RA8875_LIGHT_ORANGE, RA8875_BLACK);
  tft.setCursor(OPERATION_STATS_X + 50, FREQUENCY_Y + 30);
  if (ConfigData.activeVFO == VFO_A) {
    tft.print(bands.bands[ConfigData.currentBandA].name);  // Write current band to the display.
  } else {
    tft.print(bands.bands[ConfigData.currentBandB].name);
  }

  // Write the word MODE in white.
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.print("  MODE ");

  // Write CW mode and filter bandwidth to display.
  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
  //  tft.setCursor(OPERATION_STATS_X + 90, FREQUENCY_Y + 30);  //AFP 10-18-22
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {
    tft.print("CW  ");
  }

  // Write SSB mode to display
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
    if (ConfigData.cessb) tft.print("CESSB ");  // Which mode
    if (not ConfigData.cessb) tft.print("SSB ");
  }

  // Write FT8 mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {
    if (not ft8EnableFlag) tft.print("FT8 ");
    if (ft8EnableFlag) {
      tft.setTextColor(RA8875_RED, RA8875_BLACK);
      tft.print("FT8  ");  // Indicate that FT8 transmit is enabled.
      tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
    }
  }

  // Write AM mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::AM_MODE) {
    tft.print("AM  ");
  }

  // Write SAM mode to display.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SAM_MODE) {
    tft.print("SAM ");
  }

  // Write sideband or AM demodulation type to display.
  //  tft.setCursor(OPERATION_STATS_X + 165, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);

  switch (bands.bands[ConfigData.currentBand].sideband) {
    case Sideband::LOWER:
      tft.print("LSB");
      break;
    case Sideband::UPPER:
      tft.print("USB");
      break;
    case Sideband::BOTH_AM:
      tft.print("DSB");
      break;
    case Sideband::BOTH_SAM:
      tft.print("DSB");
      break;
    default:
      break;
  }

  // Printing the CW filter status needs to be done last.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {
    // Now print CW filter status and setting.
    tft.setCursor(375, 102);
    tft.print("CW FILTER ");
    tft.print(CWFilter[ConfigData.CWFilterIndex].c_str());
  } else {
    tft.setCursor(375, 102);
    tft.print("                 ");  // Erase if not in CW mode.
  }
}


/*****
  Purpose: Display current RF power setting.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::ShowCurrentPowerSetting() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setCursor(OPERATION_STATS_X + 275, FREQUENCY_Y + 30);
  tft.setTextColor(RA8875_RED, RA8875_BLACK);
  tft.print(ConfigData.transmitPowerLevel, 1);  // Power output is a float
  tft.print(" Watts  ");
  tft.setTextColor(RA8875_WHITE);
}


/*****
  Purpose: Format frequency for printing.
  Parameter list:
    void
  Return value;
    void
    // show frequency
*****/
void Display::FormatFrequency(uint32_t freq, char *freqBuffer) {
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
  Purpose: Used to swap the currently active VFO in the display.

  Parameter list:
    void

  Return value
    none
*****/
void Display::SwitchVFO() {
  char freqBuffer[15] = "              ";                                       // Initialize to blanks.
  tft.fillRect(0, FREQUENCY_Y - 8, tft.getFontWidth() * 33, 34, RA8875_BLACK);  // Erase both VFO frequencies.
                                                                                // Write VFO A as active VFO.
  if (ConfigData.activeVFO == VFO_A) {
    if (ConfigData.currentFreqA < bands.bands[ConfigData.currentBandA].fBandLow or ConfigData.currentFreqA > bands.bands[ConfigData.currentBandA].fBandHigh) {
      tft.setTextColor(RA8875_RED);  // Out of band
    } else {
      tft.setTextColor(RA8875_GREEN);  // In band
    }
    FormatFrequency(ConfigData.currentFreqA, freqBuffer);
    tft.setFontScale(3, 2);              // Larger font for active VFO.
    tft.setCursor(0, FREQUENCY_Y - 18);  // To adjust for Greg's font change jjp 7/14/23
    tft.print(freqBuffer);               // Print VFOA frequency.
                                         // Now reduce the font of VFOB and print it.
    tft.setFontScale(1, 2);              // Smaller font for inactive VFO.
    tft.setTextColor(RA8875_LIGHT_GREY);
    tft.setCursor(FREQUENCY_X_SPLIT + 60, FREQUENCY_Y - 18);
    FormatFrequency(ConfigData.currentFreqB, freqBuffer);
    tft.print(freqBuffer);  // Print VFOB frequency.
  }
  // Write VFO B as active VFO.
  if (ConfigData.activeVFO == VFO_B) {
    if (ConfigData.currentFreqB < bands.bands[ConfigData.currentBandB].fBandLow or ConfigData.currentFreqB > bands.bands[ConfigData.currentBandB].fBandHigh) {
      tft.setTextColor(RA8875_RED);  // Out of band
    } else {
      tft.setTextColor(RA8875_GREEN);  // In band
    }
    FormatFrequency(ConfigData.currentFreqB, freqBuffer);
    tft.setFontScale(3, 2);                                   // Larger font for active VFO.
    tft.setCursor(FREQUENCY_X_SPLIT - 80, FREQUENCY_Y - 18);  // To adjust for Greg's font change jjp 7/14/23
    tft.print(freqBuffer);                                    // Print VFOB frequency.
                                                              // Now reduce the font of VFOB and print it.
    tft.setFontScale(1, 2);                                   // Smaller font for inactive VFO.
    tft.setTextColor(RA8875_LIGHT_GREY);
    tft.setCursor(0, FREQUENCY_Y - 18);
    FormatFrequency(ConfigData.currentFreqA, freqBuffer);
    tft.print(freqBuffer);  // Print VFOB frequency.
  }
}


/*****
  Purpose: Show Main frequency display at top.  This shows currentFreqA and currentFreqB.
           This function updates the active VFO only!  It is minimalist for smooth tuning.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::ShowFrequency() {
  char freqBuffer[15] = "              ";  // Initialize to blanks.
  if (ConfigData.activeVFO == VFO_A) {     // Needed for edge checking
    ConfigData.currentBand = ConfigData.currentBandA;
  } else {
    ConfigData.currentBand = ConfigData.currentBandB;
  }
  // Update VFO A.
  if (ConfigData.activeVFO == VFO_A) {
    FormatFrequency(TxRxFreq, freqBuffer);
    tft.setFontScale(3, 2);  // JJP 7/15/23
    // Change frequency readout to red if out of band to alert user.
    if (TxRxFreq < bands.bands[ConfigData.currentBandA].fBandLow or TxRxFreq > bands.bands[ConfigData.currentBandA].fBandHigh) {
      tft.setTextColor(RA8875_RED);  // Out of band
    } else {
      tft.setTextColor(RA8875_GREEN);  // In band
    }
    tft.fillRect(0, FREQUENCY_Y - 8, tft.getFontWidth() * 10, 34, RA8875_BLACK);  // Erase VFOA frequency.
    tft.setCursor(0, FREQUENCY_Y - 18);                                           // To adjust for Greg's font change jjp 7/14/23
    tft.print(freqBuffer);                                                        // Show VFO_A
  } else {                                                                        // Update VFO_B.
    FormatFrequency(TxRxFreq, freqBuffer);
    tft.setFontScale(3, 2);
    tft.fillRect(FREQUENCY_X_SPLIT - 76, FREQUENCY_Y - 8, tft.getFontWidth() * 10, 34, RA8875_BLACK);  // Erase VFOB frequency.
    tft.setCursor(FREQUENCY_X_SPLIT - 80, FREQUENCY_Y - 18);
    if (TxRxFreq < bands.bands[ConfigData.currentBandB].fBandLow or TxRxFreq > bands.bands[ConfigData.currentBandB].fBandHigh) {
      tft.setTextColor(RA8875_RED);
    } else {
      tft.setTextColor(RA8875_GREEN);
    }
    tft.print(freqBuffer);
  }
  tft.setFontDefault();
}


/*****
  Purpose: Display signal level in dBm.
  Parameter list:
    void
  Return value;
    void
*****/
void Display::DisplaydbM() {
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
  Purpose: Display the current temperature and load figures for Teensy 4.1.

  Parameter list:
    int notchF        the notch to use
    int MODE          the current MODE

  Return value;
    void
*****/
void Display::ShowTempAndLoad() {
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
  Purpose: Format a floating point number.

  Parameter list:
    float val         the value to format
    int decimals      the number of decimal places
    int x             the x coordinate for display
    int y                 y          "

  Return value;
    void
*****/
void Display::MyDrawFloat(float val, int decimals, int x, int y, char *buff) {
  dtostrf(val, FLOAT_PRECISION, decimals, buff);  // Use 8 as that is the max prevision on a float
  tft.fillRect(x + 15, y, 12 * sizeof(buff), 15, RA8875_BLACK);
  tft.setCursor(x, y);
  tft.print(buff);
}


/*****
  Purpose: Erase and re-draw lower-right box for the information window.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::DrawInfoWindow() {
  tft.fillRect(INFORMATION_WINDOW_X - 8, INFORMATION_WINDOW_Y, 250, 170, RA8875_BLACK);    // Clear fields
  tft.drawRect(BAND_INDICATOR_X - 10, BAND_INDICATOR_Y - 2, 260, 200, RA8875_LIGHT_GREY);  // Redraw Info Window
}


/*****
  Purpose: Updates the displayed states of the speaker and headphone.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateAudioOutputField() {
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
}


/*****
  Purpose: Updates the Volume setting on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateVolumeField() {
  tft.writeTo(L1);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setCursor(BAND_INDICATOR_X + 10, BAND_INDICATOR_Y);  // Volume
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  tft.print("Vol:   ");
  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X - 17, BAND_INDICATOR_Y);
  // When both speaker and headphone are active, volume control is speaker only.  This is intended for monitoring FT8.
  if (ConfigData.audioOut == AudioState::SPEAKER or ConfigData.audioOut == AudioState::BOTH) tft.print(ConfigData.speakerVolume);
  if (ConfigData.audioOut == AudioState::HEADPHONE) tft.print(ConfigData.headphoneVolume);
}


/*****
  Purpose: Updates the AGC setting on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateAGCField() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_YELLOW, RA8875_BLACK);
  tft.setCursor(BAND_INDICATOR_X + 133, BAND_INDICATOR_Y);
  if (ConfigData.AGCMode) {  // The option for AGC
    tft.print("AGC ON ");
  } else {
    tft.setTextColor(DARKGREY, RA8875_BLACK);
    tft.print("AGC OFF");
  }
}


/*****
  Purpose: Updates the frequency increment setting on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::DisplayIncrementField() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);  // Frequency increment
  tft.setCursor(INCREMENT_X + 100, INCREMENT_Y - 1);
  tft.print("Coarse Inc: ");
  tft.setCursor(INCREMENT_X - 3, INCREMENT_Y - 1);
  tft.print("Fine Inc: ");
  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X - 23, INCREMENT_Y - 1);
  tft.print("   ");
  tft.setCursor(FIELD_OFFSET_X - 23, INCREMENT_Y - 1);
  tft.print(ConfigData.fineTuneStep);
  tft.setCursor(FIELD_OFFSET_X + 95, INCREMENT_Y - 1);
  tft.print("       ");
  tft.setCursor(FIELD_OFFSET_X + 95, INCREMENT_Y - 1);
  tft.print(ConfigData.centerTuneStep);
}


/*****
  Purpose: Updates the notch value on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateNotchField() {
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
  Purpose: Updates the zoom setting on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateZoomField() {
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
void Display::UpdateCompressionField()  // JJP 8/26/2023
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
  Purpose: Updates Morse decoder related graphics including CW, highpass, and lowpass filter bandwidths.
           This method calls DrawBandWidthIndicatorBar().

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateAudioGraphics() {
  float CWFilterPosition = 0.0;

  // Update text and graphics.  Use a method of clearing everything, and then add only what is needed.
  tft.writeTo(L2);    // This has to be written to L2 or it will be erased by the audio spectrum eraser.
  tft.clearMemory();  // Deletes audio filter delimiter bars.

  //  Calculate position and draw filter delimiters in audio spectrum box.
  int filterLoPositionMarker{ 0 };
  int filterHiPositionMarker{ 0 };
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    filterLoPositionMarker = map(bands.bands[ConfigData.currentBand].FLoCut, 0, 6000, 0, 256);
    filterHiPositionMarker = map(bands.bands[ConfigData.currentBand].FHiCut, 0, 6000, 0, 256);
  } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    filterHiPositionMarker = map(bands.bands[ConfigData.currentBand].FAMCut, 0, 6000, 0, 256);
  }

  // Draw Filter indicator lines on audio plot to Layer 2.
  // The encoder should adjust the low side of the filter if the CW filter is on.  This creates a tunable bandpass.  Sort of.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE and ConfigData.CWFilterIndex != 5) switchFilterSideband = true;

  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    if (not switchFilterSideband) {
      tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 2, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 117, RA8875_LIGHT_GREY);
      tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 2, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 117, RA8875_RED);
    } else {
      tft.drawLine(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 2, BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), SPECTRUM_BOTTOM - 117, RA8875_RED);
      tft.drawLine(BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 2, BAND_INDICATOR_X - 7 + abs(filterHiPositionMarker), SPECTRUM_BOTTOM - 117, RA8875_LIGHT_GREY);
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

  // Draw CW filter box if required.  Don't do this if the CW filter is off.  This has to be written to L2 or it will be erased by the audio spectrum eraser.
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE and ConfigData.CWFilterIndex != 5) {
    tft.fillRect(BAND_INDICATOR_X - 6 + abs(filterLoPositionMarker), AUDIO_SPECTRUM_TOP, CWFilterPosition - abs(filterLoPositionMarker), 120, MAROON);
    tft.drawFastVLine(BAND_INDICATOR_X - 8 + CWFilterPosition, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_LIGHT_GREY);
  }

  // Draw decoder delimiters if in CW mode.  The delimiters are also used for frequency spotting (for same TX frequency as received signal).
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {  // and ConfigData.decoderFlag) {  CW delimiters always shown in CW mode.  Greg KF5N October 2025
    // Draw delimiter bars for CW offset frequency.  This depends on the user selected offset.
    if (ConfigData.CWOffset == 0) {                                                              // 562.5
      tft.drawFastVLine(BAND_INDICATOR_X + 12, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator 15 10
      tft.drawFastVLine(BAND_INDICATOR_X + 18, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator 21 16
    }
    if (ConfigData.CWOffset == 1) {                                                              // 656.5
      tft.drawFastVLine(BAND_INDICATOR_X + 16, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator 19 14
      tft.drawFastVLine(BAND_INDICATOR_X + 22, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator 25 20
    }
    if (ConfigData.CWOffset == 2) {
      tft.drawFastVLine(BAND_INDICATOR_X + 20, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator 23 18
      tft.drawFastVLine(BAND_INDICATOR_X + 26, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator 29 24
    }
    if (ConfigData.CWOffset == 3) {
      tft.drawFastVLine(BAND_INDICATOR_X + 24, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW lower freq indicator 27 22
      tft.drawFastVLine(BAND_INDICATOR_X + 30, AUDIO_SPECTRUM_BOTTOM - 118, 118, RA8875_GREEN);  //CW upper freq indicator 33 28
    }
  }

  DrawBandWidthIndicatorBar();  // This method has to be run BEFORE UpdateAudioGraphics().  It can cause erasure.
  ShowBandwidth();
  tft.writeTo(L1);
}


/*****
  Purpose: Updates the displayed Rx and Tx equalizer states.

  Parameter list:
    bool rxEqState, txEqState

  Return value;
    void
*****/
void Display::UpdateEqualizerField(bool rxEqState, bool txEqState) {
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
  Purpose: Updates the displayed Keyer and WPM settings.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateKeyType() {
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
  Purpose: Update CW decoder status in the information window.
  Parameter list:
    void
  Return value;
    void
*****/
void Display::UpdateDecoderStatus() {
  tft.setFontScale((enum RA8875tsize)0);
  tft.setTextColor(RA8875_WHITE);  // Display zoom factor
  tft.setCursor(DECODER_X + 5, DECODER_Y - 5);
  tft.print("Decoder:");

  tft.setTextColor(RA8875_GREEN, RA8875_BLACK);
  tft.setCursor(FIELD_OFFSET_X, DECODER_Y - 5);
  if (ConfigData.decoderFlag) {
    tft.print("    WPM");  // Morse decoder prints the WPM value.
  } else {
    tft.print("Off       ");
  }
}


/*****
  Purpose: Updates the noise field on the display.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::UpdateNoiseField() {
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
  Purpose: This function redraws the entire display screen.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::RedrawAll() {
  tft.fillWindow();  // Clear the display.
  DrawAudioSpectContainer();
  DrawSpectrumDisplayContainer();
  DrawSMeterContainer();
  DrawInfoWindow();
  ShowName();
  SwitchVFO();
  ShowSpectrumdBScale();
  DrawFrequencyBarValue();
  //  ShowAutoStatus();
  UpdateAGCField();
  DisplayIncrementField();
  UpdateNotchField();
  UpdateNoiseField();
  UpdateZoomField();
  UpdateCompressionField();
  UpdateKeyType();
  UpdateDecoderStatus();
  UpdateEqualizerField(ConfigData.receiveEQFlag, ConfigData.xmitEQFlag);
  ShowCurrentPowerSetting();
  UpdateAudioOutputField();
  BandInformation();
  UpdateVolumeField();
  ShowRFGain();
  ShowFrequency();
  SetBandRelay();                   // Set LPF relays for current band.
  lastState = RadioState::NOSTATE;  // Force an update.
}


/*****
  Purpose: Draw Tuned Bandwidth graphic (blue bar) on Spectrum Plot. // Calculations simplified KF5N April 22, 2024
           This method is called at the conclusion of UpdateAudioGraphics().

  Parameter list:

  Return value;
    void
*****/
void Display::DrawBandWidthIndicatorBar()  // AFP 10-30-22
{
  int32_t Zoom1Offset = 0.0;
  int32_t cwOffsetPixels = 0;
  int32_t cwOffsets[4]{ 563, 657, 750, 844 };  // Rounded to nearest Hz.
  float hz_per_pixel = 0.0;
  int32_t hpf_offset{ 0 };

  int32_t cwFilterBW[5]{ 800, 1000, 1300, 1800, 2000 };  // CW filter bandwidths.

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

  // Calculate the offset due to the high-pass filter.
  hpf_offset = static_cast<int32_t>(static_cast<float>(bands.bands[ConfigData.currentBand].FLoCut) / hz_per_pixel);

  // newCursorPosition must take into account the high-pass filter setting.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) newCursorPosition = static_cast<int>(NCOFreq / hz_per_pixel) + Zoom1Offset + hpf_offset;  // More accurate tuning bar position.  KF5N May 17, 2024
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) newCursorPosition = static_cast<int>(NCOFreq / hz_per_pixel) + Zoom1Offset;

  tft.writeTo(L2);  // Write graphics to Layer 2.

  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER or bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER)
    filterWidth = static_cast<int32_t>((bands.bands[ConfigData.currentBand].FHiCut - bands.bands[ConfigData.currentBand].FLoCut) / hz_per_pixel);  // AFP 10-30-22
  else if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM)
    filterWidth = static_cast<int32_t>((static_cast<float32_t>(bands.bands[ConfigData.currentBand].FAMCut) * 2.0) / hz_per_pixel);  // AFP 10-30-22

  // Draw the bar, except for CW.
  if (radioState != RadioState::CW_RECEIVE_STATE) {
    switch (bands.bands[ConfigData.currentBand].sideband) {
      case Sideband::LOWER:
        tft.fillRect(centerLine - oldFilterWidth + oldCursorPosition, SPECTRUM_TOP_Y + 20, oldFilterWidth + 1, SPECTRUM_HEIGHT - 20, RA8875_BLACK);
        tft.fillRect(centerLine - filterWidth + newCursorPosition - hpf_offset, SPECTRUM_TOP_Y + 20, filterWidth + 1, SPECTRUM_HEIGHT - 20, FILTER_WIN);
        break;

      case Sideband::UPPER:
        tft.fillRect(centerLine + oldCursorPosition, SPECTRUM_TOP_Y + 20, oldFilterWidth + 1, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  //AFP 03-27-22 Layers
        tft.fillRect(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);           //AFP 03-27-22 Layers
        break;

      case Sideband::BOTH_AM:
      case Sideband::BOTH_SAM:
        tft.fillRect(centerLine - oldFilterWidth / 2 + oldCursorPosition, SPECTRUM_TOP_Y + 20, oldFilterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  //AFP 10-30-22
        tft.fillRect(centerLine - filterWidth / 2 + newCursorPosition, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);          //AFP 10-30-22
                                                                                                                                                     //      tft.drawFastVLine(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, h - 10, RA8875_CYAN);                 //AFP 10-30-22*/
        break;

      default:
        break;
    }
  }

  // Draw the offset bar for CW.
  if (radioState == RadioState::CW_RECEIVE_STATE) {

    // If a CW filter is on, filterWidth needs to take this into account.
    if (ConfigData.CWFilterIndex != 5) {
      filterWidth = static_cast<int>((cwFilterBW[ConfigData.CWFilterIndex] - bands.bands[ConfigData.currentBand].FLoCut) / hz_per_pixel);
    }

    // Calculate the number of bins to offset based on CW offset, which can be 562.5, 656.5, 750.0 or 843.75 Hz.
    cwOffsetPixels = cwOffsets[ConfigData.CWOffset] / hz_per_pixel;
    switch (bands.bands[ConfigData.currentBand].sideband) {
      case Sideband::LOWER:
        tft.fillRect(centerLine + oldCursorPosition + cwOffsetPixels - (oldFilterWidth + old_hpf_offset), SPECTRUM_TOP_Y + 20, oldFilterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);
        tft.fillRect(centerLine + newCursorPosition + cwOffsetPixels - (filterWidth + hpf_offset), SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);
        break;

      case Sideband::UPPER:
        tft.fillRect(centerLine + oldCursorPosition - cwOffsetPixels, SPECTRUM_TOP_Y + 20, oldFilterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);
        tft.fillRect(centerLine + newCursorPosition - cwOffsetPixels, SPECTRUM_TOP_Y + 20, filterWidth, SPECTRUM_HEIGHT - 20, FILTER_WIN);
        break;

      default:
        break;
    }
  }

  // Draw tuning alignment line in blue bandwidth bar.  Need to ignore the high-pass filter offset.
  // The old bar needs to be erased first!
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    tft.drawFastVLine(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, h - 6, RA8875_CYAN);
  }
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
    tft.drawFastVLine(centerLine + newCursorPosition - hpf_offset, SPECTRUM_TOP_Y + 20, h - 6, RA8875_CYAN);
  }
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_AM or bands.bands[ConfigData.currentBand].sideband == Sideband::BOTH_SAM) {
    tft.drawFastVLine(centerLine + newCursorPosition, SPECTRUM_TOP_Y + 20, h - 6, RA8875_CYAN);
  }

  oldFilterWidth = filterWidth;
  oldCursorPosition = newCursorPosition;
  old_hpf_offset = hpf_offset;
  tft.writeTo(L1);  //AFP 03-27-22 Layers
}


/*****
  Purpose: This function removes the spectrum display container.

  Parameter list:
    void

  Return value;
    void
*****/
void Display::EraseSpectrumDisplayContainer() {
  tft.fillRect(SPECTRUM_LEFT_X - 2, SPECTRUM_TOP_Y - 1, MAX_WATERFALL_WIDTH + 6, SPECTRUM_HEIGHT + 8, RA8875_BLACK);  // Spectrum box
}


/*****
  Purpose: Erases both primary and secondary menus from display.

  Parameter list:

  Return value;
    void
*****/
void Display::EraseMenus() {
  tft.fillRect(PRIMARY_MENU_X, MENUS_Y, BOTH_MENU_WIDTHS, CHAR_HEIGHT + 1, RA8875_BLACK);  // Erase menu choices
}


/*****
  Purpose: Show a colored transmit status rectangle in the display.
  
  Parameter list: voi

  Return value;
    void
*****/
void Display::ShowTransmitReceiveStatus() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setCursor(X_R_STATUS_X + 4, X_R_STATUS_Y - 5);
  tft.setTextColor(RA8875_BLACK);
  if (radioState == RadioState::SSB_TRANSMIT_STATE or radioState == RadioState::FT8_TRANSMIT_STATE or radioState == RadioState::CW_TRANSMIT_STRAIGHT_STATE
      or radioState == RadioState::CW_TRANSMIT_KEYER_STATE or radioState == RadioState::CW_CALIBRATE_STATE
      or radioState == RadioState::SSB_CALIBRATE_STATE or radioState == RadioState::RECEIVE_CALIBRATE_STATE or radioState == RadioState::SSB_IM3TEST_STATE) {
    tft.fillRect(X_R_STATUS_X, X_R_STATUS_Y, 55, 25, RA8875_RED);
    if (digitalRead(RXTX)) tft.print("XMT");  // Make sure the hardware is actually in transmit mode!
  } else {
    tft.fillRect(X_R_STATUS_X, X_R_STATUS_Y, 55, 25, RA8875_GREEN);
    if (not digitalRead(RXTX)) tft.print("REC");  // Don't claim to be in receive mode when the TX relay is not in RX position!
  }
}


/*****
  Purpose: DisplayClock()
  Parameter list:
    void
  Return value;
    void
*****/
void Display::DisplayClock() {
  char timeBuffer[15];
  char temp[5];

  temp[0] = '\0';
  timeBuffer[0] = '\0';
  strcpy(timeBuffer, MY_TIMEZONE);  // e.g., EST
#ifdef TIME_24H
  //DB2OO, 29-AUG-23: use 24h format
  itoa(hour(), temp, DEC);
#else
  itoa(hourFormat12(), temp, DEC);
#endif
  if (strlen(temp) < 2) {
    strcat(timeBuffer, "0");
  }
  strcat(timeBuffer, temp);
  strcat(timeBuffer, ":");

  itoa(minute(), temp, DEC);
  if (strlen(temp) < 2) {
    strcat(timeBuffer, "0");
  }
  strcat(timeBuffer, temp);
  strcat(timeBuffer, ":");

  itoa(second(), temp, DEC);
  if (strlen(temp) < 2) {
    strcat(timeBuffer, "0");
  }
  strcat(timeBuffer, temp);

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(TIME_X, TIME_Y, XPIXELS - TIME_X - 1, CHAR_HEIGHT, RA8875_BLACK);
  tft.setCursor(TIME_X, TIME_Y);
  tft.setTextColor(RA8875_WHITE);
  tft.print(timeBuffer);
}  // end function Displayclock
