// CW Exciter class

#include "SDT.h"

/*****
  Purpose: Load buffers used to modulate the transmitter during calibration.
          The epilogue must restore the buffers for normal operation!

   Parameter List:
      void

   Return value:
      void
 *****/
void CW_Exciter::writeSineBuffer(int numCycles) {
  float32_t theta{ 0.0 };  //, increment{ 0.0 };
  float32_t freqSideTone{ 0.0 };
  const float32_t amplitude{ 0.12 };  // Sets the peak amplitude of the sinusoid.
  freqSideTone = static_cast<float>(numCycles) * 48000.0 / 512.0;
  for (int kf = 0, increment = 0; kf < 512; increment += 1, kf++) {  // Calc: numCycles = 8, 750 hz sine wave.
    theta = static_cast<float32_t>(increment) * 2.0 * PI * freqSideTone / 48000.0;
    sineBuffer[kf] = amplitude * sin(theta);  // Create the CW tone waveform.
  }
  //  Serial.printf("numCycles = %d\n", numCycles);
}


/*****
  Purpose: Create I and Q signals for CW transmission.

  Parameter list:
    int shaping   one of CW_SHAPING_RISE, CW_SHAPING_FALL, or CW_SHAPING_NONE

  Return value;
    void
*****/
void CW_Exciter::CW_ExciterIQData(int shaping)  //AFP 08-20-22
{
  float32_t powerScale = 0;

  if (shaping == CW_SHAPING_RISE) {
    arm_mult_f32(sineBuffer, cwRiseBuffer, float_buffer_cw, 512);
  } else if (shaping == CW_SHAPING_FALL) {
    arm_mult_f32(sineBuffer, cwFallBuffer, float_buffer_cw, 512);
  } else if (shaping == CW_SHAPING_ZERO) {
    arm_scale_f32(sineBuffer, 0.0, float_buffer_cw, 512);
  } else {
    arm_scale_f32(sineBuffer, 1.0, float_buffer_cw, 512);
  }

  cwToneData.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
  cwToneData.setMaxBuffers(4);
  cwToneData.play(float_buffer_cw, 512);  // Push CW waveform into SSB transmitter input.

  // Make Q15 data for CW sidetone.
  arm_float_to_q15(float_buffer_cw, q15_buffer_Sidetone, 512);

//  Q_out_L.setBehaviour(AudioPlayQueue::NON_STALLING);  Now set in AudioSignal.h.

  float32_t float_buffer_i[512] = { 0.0 };
  float32_t float_buffer_q[512] = { 0.0 };
  float32_t* iBuffer = nullptr;  // I and Q pointers needed for one-time read of record queues.
  float32_t* qBuffer = nullptr;

// Read incoming I and Q audio blocks from the SSB exciter.
  if (Q_in_L_Ex.available() > 3 and Q_in_R_Ex.available() > 3) {
    for (int i = 0; i < 4; i = i + 1) {
      iBuffer = Q_in_L_Ex.readBuffer();
      qBuffer = Q_in_R_Ex.readBuffer();
      std::copy(iBuffer, iBuffer + 128, &float_buffer_i[128 * i]);
      std::copy(qBuffer, qBuffer + 128, &float_buffer_q[128 * i]);
      Q_in_L_Ex.freeBuffer();
      Q_in_R_Ex.freeBuffer();
    }
  } else return;
  

/*
    if (Q_in_L_Ex.available() > 3 and Q_in_R_Ex.available() > 3) {
    for (int i = 0; i < 4; i = i + 1) {
      arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_i[128 * i], 128);  // convert int_buffer to float 32bit
      arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_q[128 * i], 128);
      Q_in_L_Ex.freeBuffer();
      Q_in_R_Ex.freeBuffer();
    }
  } else return;
  */

  // Set the sideband.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

    //  This is the correct place in the data flow to inject the scaling for power.
#ifdef QSE2
  powerScale = 2.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#else
  powerScale = 1.4 * ConfigData.powerOutCW[ConfigData.currentBand];
#endif

  arm_scale_f32(float_buffer_i, powerScale, float_buffer_i, 512);
  arm_scale_f32(float_buffer_q, powerScale, float_buffer_q, 512);

#ifdef QSE2
  arm_offset_f32(float_buffer_i, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_i, 512);  // Carrier suppression offset.
  arm_offset_f32(float_buffer_q, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, float_buffer_q, 512);
#endif

  Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
  Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);

  // Play audio.
  Q_out_L_Ex.play(float_buffer_i, 512);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
  Q_out_R_Ex.play(float_buffer_q, 512);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.

  Q_out_L.play(q15_buffer_Sidetone, 512);
}
