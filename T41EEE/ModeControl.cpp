// Mode control struct members.


#include "SDT.h"


void ModeControl::CWReceiveMode() {
  startRxFlag = true;
  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  encoderFilterFlag = true;
}


void ModeControl::CWTransmitMode() {
  display.EraseRFSpectrum();
  SetFreq();
}


void ModeControl::SSBReceiveMode() {
  startRxFlag = true;
  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  encoderFilterFlag = true;
}


void ModeControl::SSBTransmitMode() {
  display.EraseRFSpectrum();
  SetFreq();
}


void ModeControl::FT8ReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  encoderFilterFlag = true;
  display.ShowBandwidth();
}


void ModeControl::FT8TransmitMode() {
  display.EraseRFSpectrum();
  SetFreq();
}


void ModeControl::AMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  encoderFilterFlag = true;
  display.ShowBandwidth();
}


void ModeControl::SAMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates center frequency, band, mode, and sideband at top of spectrum.

  encoderFilterFlag = true;
  display.ShowBandwidth();
}
