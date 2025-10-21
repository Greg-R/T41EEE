// Class for the CW Exciter object.
// Greg Raven KF5N October 20, 2025.

#pragma once

#include "SDT.h"

class CW_Exciter {

public:

  void writeSineBuffer(int numCycles);
  void CW_ExciterIQData(int shaping);

private:

  float sineBuffer[512]{ 0 };  // Used to create CW tone waveform.
  float float_buffer_cw[512]{ 0 };
  q15_t q15_buffer_Sidetone[512]{ 0 };

};
