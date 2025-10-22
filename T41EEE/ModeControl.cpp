
#include "SDT.h"


void ModeControl::CWReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.
  FilterBandwidth();  // Required to set up filter properly for this mode.
  display.DrawBandWidthIndicatorBar();
}


void ModeControl::CWTransmitMode() {

  SetFreq();


}


void ModeControl::SSBReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.
  FilterBandwidth();  // Required to set up filter properly for this mode.
    display.DrawBandWidthIndicatorBar();
}


void ModeControl::SSBTransmitMode() {

  SetFreq();


}


void ModeControl::FT8ReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.
  FilterBandwidth();  // Required to set up filter properly for this mode.
    display.DrawBandWidthIndicatorBar();
}


void ModeControl::FT8TransmitMode() {

  SetFreq();


}


void ModeControl::AMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.
  FilterBandwidth();  // Required to set up filter properly for this mode.
  display.DrawBandWidthIndicatorBar();
}


void ModeControl::SAMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.
  FilterBandwidth();  // Required to set up filter properly for this mode.
  display.DrawBandWidthIndicatorBar();
}



