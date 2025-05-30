
#include "SDT.h"


/*****
  Purpose: to send a Morse code dit

  Parameter list:
    void

  Return value:
    void
*****/
void KeyTipOn() {
  if (digitalRead(KEYER_DIT_INPUT_TIP) == LOW and bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {
    keyPressedOn = 1;
  }
}

/*****
  Purpose: CW Key interrupt //AFP 09-01-22

  Parameter list:

  Return value;
  voidKeyRingOn(

*****/
void KeyRingOn()  //AFP 09-25-22
{
  if (ConfigData.keyType == 1) {
    if (digitalRead(KEYER_DAH_INPUT_RING) == LOW and bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE) {
      keyPressedOn = 1;
    }
  }
}


/*****
  Purpose: Create I and Q signals for CW transmission.

  Parameter list:
    int shaping   one of CW_SHAPING_RISE, CW_SHAPING_FALL, or CW_SHAPING_NONE

  Return value;
    void
*****/
void CW_ExciterIQData(int shaping)  //AFP 08-20-22
{
  // uint32_t N_BLOCKS_EX = N_B_EX;
  float32_t powerScale;
  q15_t q15_buffer_LTemp[2048];  //KF5N
  q15_t q15_buffer_RTemp[2048];  //KF5N
  q15_t q15_buffer_Sidetone[2048];
  float float_buffer_Sidetone[2048];

  arm_scale_f32(cosBuffer, 0.20, float_buffer_L_EX, 256);  // AFP 10-13-22 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32(sinBuffer, 0.20, float_buffer_R_EX, 256);  // AFP 10-13-22

  //  Apply amplitude and phase corrections.

  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
    arm_scale_f32(float_buffer_L_EX, -CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand], float_buffer_L_EX, 256);       //Adjust level of L buffer KF5N flipped sign, original was +.
    IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand], 256);  // Adjust phase
  } else {
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
      arm_scale_f32(float_buffer_L_EX, +CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], float_buffer_L_EX, 256);       // KF5N flipped sign, original was minus.
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 256);  // Adjust phase
    }
  }


  if (shaping == CW_SHAPING_RISE) {
    arm_mult_f32(float_buffer_L_EX, cwRiseBuffer, float_buffer_L_EX, 256);
    arm_mult_f32(float_buffer_R_EX, cwRiseBuffer, float_buffer_R_EX, 256);
  } else if (shaping == CW_SHAPING_FALL) {
    arm_mult_f32(float_buffer_L_EX, cwFallBuffer, float_buffer_L_EX, 256);
    arm_mult_f32(float_buffer_R_EX, cwFallBuffer, float_buffer_R_EX, 256);
  } else if (shaping == CW_SHAPING_ZERO) {
    arm_scale_f32(float_buffer_L_EX, 0.0, float_buffer_L_EX, 256);
    arm_scale_f32(float_buffer_R_EX, 0.0, float_buffer_R_EX, 256);
  }

  /**********************************************************************************
              Interpolate (upsample the data streams by 8X to create the 192KHx sample rate for output
              Requires a LPF FIR 48 tap 10KHz and 8KHz
     **********************************************************************************/
  //24KHz effective sample rate here
  arm_fir_interpolate_f32(&FIR_int1_EX_I, float_buffer_L_EX, float_buffer_LTemp, 256);
  arm_fir_interpolate_f32(&FIR_int1_EX_Q, float_buffer_R_EX, float_buffer_RTemp, 256);

  // interpolation-by-4,  48KHz effective sample rate here
  arm_fir_interpolate_f32(&FIR_int2_EX_I, float_buffer_LTemp, float_buffer_L_EX, 512);
  arm_fir_interpolate_f32(&FIR_int2_EX_Q, float_buffer_RTemp, float_buffer_R_EX, 512);

  //  Need to have a sidetone which is independent of any scaling.  Re-use float_bufferL/RTemp and Q_out_L/Q_out_R buffers.
  arm_scale_f32(float_buffer_L_EX, 20, float_buffer_Sidetone, 2048);

//  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
  powerScale = 40.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#else
  powerScale = 30.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#endif

  //  192KHz effective sample rate here
  arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048);  //Scale to compensate for losses in Interpolation
  arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

  /**********************************************************************************  AFP 12-31-20
      CONVERT TO INTEGER AND PLAY AUDIO, I and Q of CW transmitter and also sidetone.
    **********************************************************************************/

  // Transmitter I and Q.  Cast the arrays from float to q15_t.  q15_t is equivalent to int16_t.
  arm_float_to_q15(float_buffer_Sidetone, q15_buffer_Sidetone, 2048);  // source, destination, number of samples
  arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 2048);         // source, destination, number of samples
  arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 2048);

// Inject the DC offset from carrier calibration.  There is an ARM function for this.
#ifdef QSE2
  arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, 2048);
  arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, 2048);
#endif

  Q_out_L_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);  // This mode is required when setting sidetone volume.
  Q_out_R_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);  // The process will stall due to disconnection of I and Q patchcords.
  Q_out_L_Ex.play(q15_buffer_LTemp, 2048);                // Transmitter
  Q_out_R_Ex.play(q15_buffer_RTemp, 2048);                // Transmitter
  Q_out_L.play(q15_buffer_Sidetone, 2048);                // CW sidetone.  Connected to receiver audio path during transmit.
}
