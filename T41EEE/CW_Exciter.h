// Class for the CW Exciter object.
#include "SDT.h"

class CW_Exciter {

public:

float sineBuffer[512] {0};  // Used to create CW tone waveform.
  float float_buffer_cw[512] {0};
  q15_t q15_buffer_Sidetone[512] {0};

void writeSineBuffer(int numCycles);

void CW_ExciterIQData(int shaping);
};


