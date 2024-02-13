#ifndef BEENHERE
#include "SDT.h"
#endif

//=====================  file all new  AFP 09-01-22


/*****
  Purpose: to send a Morse code dit

  Parameter list:
    void

  Return value:
    void
*****/
void KeyTipOn()
{
  if (digitalRead(KEYER_DIT_INPUT_TIP) == LOW && EEPROMData.xmtMode == CW_MODE ) {
    keyPressedOn = 1;
  }
}

/*****
  Purpose: CW Key interrupt //AFP 09-01-22

  Parameter list:

  Return value;
  voidKeyRingOn(

*****/
void KeyRingOn() //AFP 09-25-22
{
  if (EEPROMData.keyType == 1) {
    if (digitalRead(KEYER_DAH_INPUT_RING) == LOW && EEPROMData.xmtMode == CW_MODE ) {
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
void CW_ExciterIQData(int shaping) //AFP 08-20-22
{
  uint32_t N_BLOCKS_EX = N_B_EX;
 
  arm_scale_f32 (cosBuffer, 0.3, float_buffer_L_EX, 256);  // AFP 10-13-22 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32 (sinBuffer, 0.3, float_buffer_R_EX, 256);  // AFP 10-13-22
  /**********************************************************************************
            Additional scaling, if nesessary to compensate for down-stream gain variations
   **********************************************************************************/

  //============================== AFP 10-21-22  Begin new

  if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) {
    //arm_scale_f32 (float_buffer_L_EX, EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);  //Adjust level of L buffer
    arm_scale_f32(float_buffer_L_EX, -EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L_EX, 256);       //Adjust level of L buffer KF5N flipped sign, original was +
    IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand], 256);  // Adjust phase
  } else {
    if (bands[EEPROMData.currentBand].mode == DEMOD_USB) {
      //arm_scale_f32 (float_buffer_L_EX, -EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);
      arm_scale_f32 (float_buffer_L_EX, + EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBand], float_buffer_L_EX, 256);   // KF5N flipped sign, original was minus
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBand], 256);
    }
  }

  if (shaping == CW_SHAPING_RISE) {
    arm_mult_f32(float_buffer_L_EX, cwRiseBuffer, float_buffer_L_EX, 256);
    arm_mult_f32(float_buffer_R_EX, cwRiseBuffer, float_buffer_R_EX, 256);
  } else if (shaping == CW_SHAPING_FALL) {
    arm_mult_f32(float_buffer_L_EX, cwFallBuffer, float_buffer_L_EX, 256);
    arm_mult_f32(float_buffer_R_EX, cwFallBuffer, float_buffer_R_EX, 256);
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

    //  192KHz effective sample rate here
    arm_scale_f32(float_buffer_L_EX, 25, float_buffer_L_EX, 2048); //Scale to compensate for losses in Interpolation
    arm_scale_f32(float_buffer_R_EX, 25, float_buffer_R_EX, 2048);

    /**********************************************************************************  AFP 12-31-20
      CONVERT TO INTEGER AND PLAY AUDIO
    **********************************************************************************/

    for (unsigned  i = 0; i < N_BLOCKS_EX; i++) {  //N_BLOCKS_EX=16  BUFFER_SIZE=128 16x128=2048
      sp_L2 = Q_out_L_Ex.getBuffer();
      sp_R2 = Q_out_R_Ex.getBuffer();
      arm_float_to_q15 (&float_buffer_L_EX[BUFFER_SIZE * i], sp_L2, BUFFER_SIZE);
      arm_float_to_q15 (&float_buffer_R_EX[BUFFER_SIZE * i], sp_R2, BUFFER_SIZE);
      Q_out_L_Ex.playBuffer(); // play it !
      Q_out_R_Ex.playBuffer(); // play it !
    }
}
