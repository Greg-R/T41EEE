
#include "SDT.h"


void ModeControl::CWReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.

  display.UpdateAudioGraphics();
  FilterSetSSB();  // Required to set up filter properly for this mode.
}


void ModeControl::CWTransmitMode() {

  SetFreq();


}


void ModeControl::SSBReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.

    display.UpdateAudioGraphics();
      FilterSetSSB();  // Required to set up filter properly for this mode.
}


void ModeControl::SSBTransmitMode() {

  SetFreq();


}


void ModeControl::FT8ReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.

    display.UpdateAudioGraphics();
      FilterSetSSB();  // Required to set up filter properly for this mode.
}


void ModeControl::FT8TransmitMode() {

  SetFreq();


}


void ModeControl::AMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.

  display.UpdateAudioGraphics();
    FilterSetSSB();  // Required to set up filter properly for this mode.
}


void ModeControl::SAMReceiveMode() {

  SetFreq();
  display.BandInformation();  // Updates display when mode changes.

  display.UpdateAudioGraphics();
    FilterSetSSB();  // Required to set up filter properly for this mode.
}



