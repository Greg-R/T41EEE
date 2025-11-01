
#include "SDT.h"


void ModeControl::CWReceiveMode() {
  Serial.printf("CWReceiveMode()\n");
  startRxFlag = true;
  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  display.UpdateAudioGraphics();
  FilterBandwidth();  // Required to set up filter properly for this mode.
  //  display.ShowBandwidth();  Run by UpdateAudioGraphics().
}


void ModeControl::CWTransmitMode() {
  Serial.printf("CWTransmitMode()\n");
  Serial.printf("radioState = %d\n", radioState);
  SetFreq();
}


void ModeControl::SSBReceiveMode() {
  Serial.printf("SSBReceiveMode()\n");
  startRxFlag = true;
  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  display.UpdateAudioGraphics();
  FilterBandwidth();  // Required to set up filter properly for this mode.
//  display.ShowBandwidth();
}


void ModeControl::SSBTransmitMode() {

  SetFreq();
}


void ModeControl::FT8ReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  display.UpdateAudioGraphics();
  FilterBandwidth();  // Required to set up filter properly for this mode.
  display.ShowBandwidth();
}


void ModeControl::FT8TransmitMode() {

  SetFreq();
}


void ModeControl::AMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  display.UpdateAudioGraphics();
  FilterBandwidth();  // Required to set up filter properly for this mode.
  display.ShowBandwidth();
}


void ModeControl::SAMReceiveMode() {

  SetFreq();
  //  bands.bands[ConfigData.currentBand].sideband = Sideband::BOTH_SAM;
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  display.UpdateAudioGraphics();
  //    FilterSetSSB();  // Required to set up filter properly for this mode.
  FilterBandwidth();
  display.ShowBandwidth();
}
