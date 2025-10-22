#pragma once


class Display {

public:

uint32_t TEMP_X_OFFSET {15};
uint32_t TEMP_Y_OFFSET {465};  // 480 * 0.97 = 465

const uint32_t SPECTRUM_RES {512};

int16_t spectrum_x = 10;
uint16_t waterfall[MAX_WATERFALL_WIDTH];
int maxYPlot;
int filterWidthX;              // The current filter X.
//uint8_t twinpeaks_tested = 2;  // initial value --> 2 !!
uint8_t write_analog_gain = 0;
int16_t pos_x_time = 390;  // 14;
int16_t pos_y_time = 5;    //114;
float xExpand = 1.4;       //
int16_t spectrum_pos_centre_f = 64 * xExpand;
int pos_centre_f = 64;
int smeterLength;
float CPU_temperature = 0.0;
double elapsed_micros_mean;
int centerLine = (MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2 + 2;  // The FFT is plotted starting at x = 3.
bool spectrumErased{ false };

const uint16_t gradient[117]  {  // Color array for waterfall background
  0x0,    0x1,    0x2,    0x3,    0x4,    0x5,    0x6,    0x7,    0x8,    0x9,
  0x10,   0x1F,   0x11F,  0x19F,  0x23F,  0x2BF,  0x33F,  0x3BF,  0x43F,  0x4BF,
  0x53F,  0x5BF,  0x63F,  0x6BF,  0x73F,  0x7FE,  0x7FA,  0x7F5,  0x7F0,  0x7EB,
  0x7E6,  0x7E2,  0x17E0, 0x3FE0, 0x67E0, 0x8FE0, 0xB7E0, 0xD7E0, 0xFFE0, 0xFFC0,
  0xFF80, 0xFF20, 0xFEE0, 0xFE80, 0xFE40, 0xFDE0, 0xFDA0, 0xFD40, 0xFD00, 0xFCA0,
  0xFC60, 0xFC00, 0xFBC0, 0xFB60, 0xFB20, 0xFAC0, 0xFA80, 0xFA20, 0xF9E0, 0xF980,
  0xF940, 0xF8E0, 0xF8A0, 0xF840, 0xF800, 0xF802, 0xF804, 0xF806, 0xF808, 0xF80A,
  0xF80C, 0xF80E, 0xF810, 0xF812, 0xF814, 0xF816, 0xF818, 0xF81A, 0xF81C, 0xF81E,
  0xF81E, 0xF81E, 0xF81E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF85E, 0xF85E, 0xF85E,
  0xF85E, 0xF87E, 0xF87E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF85E, 0xF85E, 0xF85E,
  0xF85E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF87E,
  0xF87E, 0xF87E, 0xF87E, 0xF87E, 0xF88F, 0xF88F, 0xF88F
};


void DrawAudioSpectContainer();  // Draw audio spectrum box.
void ShowName();  // Show the program name and version number.
void ShowSpectrum(bool drawSpectrum);  // Draws the RF and audio spectrums.
void ShowBandwidth();  // Show filter bandwidth near center of spectrum.
void DrawSMeterContainer();  // Draw the S-meter container.
void ShowSpectrumdBScale();  // Print the vertical dB setting to the spectrum display.
void DrawSpectrumDisplayContainer();  // This function draws RF spectrum display container.
void DrawFrequencyBarValue();  // This function draws the frequency bar at the bottom of the spectrum scope, putting markers at every
                               //  graticule and the full frequency.
void ShowAutoStatus();  // Indicate Auto-Gain or Auto-Spectrum is active.
void BandInformation();  // To display the current transmission frequency, band, mode, and sideband above the spectrum display
void ShowCurrentPowerSetting();  // Display current power setting.
void FormatFrequency(uint32_t freq, char *freqBuffer);  // Format frequency for printing.
void SwitchVFO();  // Switch active VFO in the display.
void ShowFrequency();  // Show Main frequency display at top.  This shows VFO A and VFO B.
void DisplaydbM();  // Display signal level in dBm.
void ShowTempAndLoad();  // Display the current temperature and load figures for Teensy 4.1.
void MyDrawFloat(float val, int decimals, int x, int y, char *buff);  // Format a floating point number.
void UpdateInfoWindow();  // Shows the startup settings for the information displayed int the lower-right box.
void UpdateAudioField();  // Updates the displayed states of the speaker and headphone.
void UpdateVolumeField();  // Updates the Volume setting on the display.
void UpdateAGCField();  // Updates the AGC setting on the display.
void DisplayIncrementField();  // Updates the frequency increment setting on the display.
void UpdateNotchField();  // Updates the notch value on the display.
void UpdateZoomField();  // Updates the zoom setting on the display.
void UpdateCompressionField();  // Updates the compression setting in the info window.
void UpdateAudioGraphics();  // Updates Morse decoder related graphics including CW, highpass, and lowpass filter bandwidths.
void UpdateEqualizerField(bool rxEqState, bool txEqState);  // Updates the displayed Rx and Tx equalizer states.
void UpdateWPMField();  // Updates the displayed Keyer and WPM settings.
void UpdateNoiseField();  // Updates the noise field on the display.
void DrawInfoWindowFrame();  // This function draws the Info Window frame.
void RedrawDisplayScreen();  // This function redraws the entire display screen.
void DrawBandWidthIndicatorBar();  // Draw Tuned Bandwidth on Spectrum Plot.
void EraseSpectrumDisplayContainer();  // This function removes the spectrum display container.
void EraseSpectrumWindow();  // This function erases the contents of the spectrum display.
void EraseMenus();  // Erases both primary and secondary menus from display.
void ErasePrimaryMenu();  // Erase primary menu from display.
void EraseSecondaryMenu();  // Erase secondary menu from display.
void ShowTransmitReceiveStatus();  // Shows transmit (red) and receive (green) mode.

private:

const int32_t CLIP_AUDIO_PEAK {115};  // The pixel value where audio peak overwrites S-meter
const uint32_t INCREMENT_X {WATERFALL_RIGHT_X + 25};
const uint32_t INCREMENT_Y {WATERFALL_TOP_Y + 70};
const uint32_t SMETER_X {WATERFALL_RIGHT_X + 16};
const float32_t SMETER_Y {YPIXELS * 0.22};  // 480 * 0.22 = 106

float32_t pixel_per_khz{ 0 };
int pos_left{ 0 };
int filterWidth{ 0 };

};
