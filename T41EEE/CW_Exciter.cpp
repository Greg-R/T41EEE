
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
//  uint32_t N_BLOCKS_EX = N_B_EX;
  float32_t powerScale;
//  q15_t q15_buffer_LTemp[2048];  //KF5N
//  q15_t q15_buffer_RTemp[256];  //KF5N
  q15_t q15_buffer_Sidetone[512];
//  float float_buffer_Sidetone[256];
  float float_buffer_cw[256];
  float float_buffer_temp_cw[512];

// Create CW waveform and play into the SSB transmit signal chain input.
//  arm_scale_f32(cosBuffer, 0.05, float_buffer_L_EX, 256);  // AFP 10-13-22 Use pre-calculated sin & cos instead of Hilbert
  arm_scale_f32(sinBuffer, 0.25, float_buffer_cw, 256);  // AFP 10-13-22

  if (shaping == CW_SHAPING_RISE) {
    arm_mult_f32(float_buffer_cw, cwRiseBuffer, float_buffer_cw, 256);
//    arm_mult_f32(float_buffer_R_EX, cwRiseBuffer, float_buffer_R_EX, 256);
  } else if (shaping == CW_SHAPING_FALL) {
    arm_mult_f32(float_buffer_cw, cwFallBuffer, float_buffer_cw, 256);
//    arm_mult_f32(float_buffer_R_EX, cwFallBuffer, float_buffer_R_EX, 256);
  } else if (shaping == CW_SHAPING_ZERO) {
    arm_scale_f32(float_buffer_cw, 0.0, float_buffer_cw, 256);
//    arm_scale_f32(float_buffer_R_EX, 0.0, float_buffer_R_EX, 256);
  }

      //  Interpolate by 2.  Output 512 samples.
    arm_fir_interpolate_f32(&FIR_int1_EX_I, float_buffer_cw, float_buffer_temp_cw, 256);

    // interpolation-by-4,  Output 2048 samples.
//    arm_fir_interpolate_f32(&FIR_int2_EX_I, float_buffer_LTemp, float_buffer_cw, 512);
//cwToneData.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
 cwToneData.setBehaviour(AudioPlayQueue_F32::NON_STALLING); 
    cwToneData.setMaxBuffers(8);
    cwToneData.play(float_buffer_temp_cw, 512);           // Push CW waveform into SSB transmitter input.

    arm_float_to_q15(float_buffer_temp_cw, q15_buffer_Sidetone, 512);

    Q_out_L.setBehaviour(AudioPlayQueue::NON_STALLING);
//    Q_out_L.setBehaviour(AudioPlayQueue::ORIGINAL);
//    Q_out_L.setMaxBuffers(64);
//    Q_out_L.play(q15_buffer_Sidetone, 128);                // CW sidetone.  Connected to receiver audio path during transmit.

//Serial.printf("CW Exciter %d\n", shaping);

  //  Need to have a sidetone which is independent of any scaling.  Re-use float_bufferL/RTemp and Q_out_L/Q_out_R buffers.
////  arm_scale_f32(float_buffer_L_EX, 20, float_buffer_Sidetone, 256);


  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are  read into two arrays in blocks of 128 up to N_BLOCKS.  The arrays are
        of size BUFFER_SIZE*N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_L / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_L = 512
        BUFFER_SIZE * N_BLOCKS = 2048 samples
     **********************************************************************************
  // are there at least N_BLOCKS buffers in each channel available ?
//  if ((uint32_t)Q_in_L_Ex.available() < 4 or (uint32_t)Q_in_R_Ex.available() < 4) {
//    Serial.printf("Q_in_L_Ex.available() = %d Q_in_R_Ex.available() = %d\n", Q_in_L_Ex.available(), Q_in_R_Ex.available());
//     return;
//  }
//    Serial.printf("Norm Op: Q_in_L_Ex.available() = %d Q_in_R_Ex.available() = %d\n", Q_in_L_Ex.available(), Q_in_R_Ex.available());
    // get audio samples from the audio  buffers and convert them to float
    // read in 32 blocks of 128 samples in I and Q
//    for (unsigned i = 0; i < 2; i++) {

      // *********************************************************************************  AFP 12-31-20
      //    Using arm_Math library, convert to float one buffer_size.
      //    Float_buffer samples are now standardized from > -1.0 to < 1.0
    //  **********************************************************************************
//      arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
//      arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // Right channel not used.  KF5N March 11, 2024
//      Q_in_L_Ex.freeBuffer();
//      Q_in_R_Ex.freeBuffer();  // Right channel not used.  KF5N March 11, 2024
//    }
*/
  float float_buffer_i[128] = {0.0};
  float float_buffer_q[128] = {0.0};

//arm_q15_to_float(Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // convert int_buffer to float 32bit
//arm_q15_to_float(Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE);  // Right channel not used.  KF5N March 11, 2024

//Serial.printf("BEFORE Q_in_L_Ex.available = %d\n", Q_in_L_Ex.available());

if(Q_in_L_Ex.available() > 0) {
 arm_q15_to_float(Q_in_L_Ex.readBuffer(), float_buffer_i, 128);  // convert int_buffer to float 32bit
 arm_q15_to_float(Q_in_R_Ex.readBuffer(), float_buffer_q, 128);
}
else return;
//arm_q15_to_float(Q_in_R_Ex.readBuffer(), float_buffer_q, 128);  // Right channel not used.  KF5N March 11, 2024

//Serial.printf("AFTER Q_in_L_Ex.available = %d\n", Q_in_L_Ex.available());

Q_in_L_Ex.freeBuffer();
Q_in_R_Ex.freeBuffer();


    // Set the sideband.
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
    if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

    // Apply amplitude and phase corrections.  FT8 uses CW corrections and is always USB.

    //  THIS NEEDS TO BE DONE EARLIER!  Moved to AudioSignal.h.

//    if(bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
//    cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorLSB[ConfigData.currentBand], CalData.IQSSBPhaseCorrectionFactorLSB[ConfigData.currentBand], 0.0);
//    } else if(bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
//    cessb1.setIQCorrections(true, CalData.IQSSBAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQSSBPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
//    }

    //  This is the correct place in the data flow to inject the scaling for power.
#ifdef QSE2
    powerScale = 2.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#else
    powerScale = 1.4 * ConfigData.powerOutCW[ConfigData.currentBand];
#endif

    arm_scale_f32(float_buffer_i, powerScale, float_buffer_i, 128);
    arm_scale_f32(float_buffer_q, powerScale, float_buffer_q, 128);

    // **********************************************************************************  AFP 12-31-20
    //  CONVERT TO INTEGER AND PLAY AUDIO
    // **********************************************************************************
    q15_t q15_buffer_LTemp[128];  // KF5N
    q15_t q15_buffer_RTemp[128];  // KF5N


    arm_float_to_q15(float_buffer_i, q15_buffer_LTemp, 128);
    arm_float_to_q15(float_buffer_q, q15_buffer_RTemp, 128);
#ifdef QSE2
//    if(bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
    arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_LTemp, 128);  // Carrier suppression offset.
    arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetSSB[ConfigData.currentBand] + CalData.dacOffsetSSB, q15_buffer_RTemp, 128);
//    } else if (bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {
//    arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, 2048);  // Carrier suppression offset.
//    arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, 2048);      
//    }
#endif
  Q_out_L_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
  Q_out_R_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);
//  Q_out_L_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
//  Q_out_R_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
    Q_out_L_Ex.play(q15_buffer_LTemp, 128);  // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
    Q_out_R_Ex.play(q15_buffer_RTemp, 128);  // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.

    Q_out_L.play(q15_buffer_Sidetone, 128); 

/*  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
  powerScale = 40.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#else
  powerScale = 30.0 * ConfigData.powerOutCW[ConfigData.currentBand];
#endif

  //  192KHz effective sample rate here
  arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 256);  //Scale to compensate for losses in Interpolation
//  arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

  // **********************************************************************************  AFP 12-31-20
  //    CONVERT TO INTEGER AND PLAY AUDIO, I and Q of CW transmitter and also sidetone.
  //  **********************************************************************************

  // Transmitter I and Q.  Cast the arrays from float to q15_t.  q15_t is equivalent to int16_t.
  arm_float_to_q15(float_buffer_Sidetone, q15_buffer_Sidetone, 256);  // source, destination, number of samples
  arm_float_to_q15(float_buffer_L_EX, q15_buffer_LTemp, 256);         // source, destination, number of samples
  arm_float_to_q15(float_buffer_R_EX, q15_buffer_RTemp, 256);

// Inject the DC offset from carrier calibration.  There is an ARM function for this.
#ifdef QSE2
  arm_offset_q15(q15_buffer_LTemp, CalData.iDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_LTemp, 256);
  arm_offset_q15(q15_buffer_RTemp, CalData.qDCoffsetCW[ConfigData.currentBand] + CalData.dacOffsetCW, q15_buffer_RTemp, 256);
#endif

//  Q_out_L_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);  // This mode is required when setting sidetone volume.
//  Q_out_R_Ex.setBehaviour(AudioPlayQueue::NON_STALLING);  // The process will stall due to disconnection of I and Q patchcords.
  Q_out_L_Ex.play(q15_buffer_LTemp, 256);                // Transmitter
  Q_out_R_Ex.play(q15_buffer_RTemp, 256);                // Transmitter

//  Q_out_L.play(q15_buffer_Sidetone, 256);                // CW sidetone.  Connected to receiver audio path during transmit.
  */
}
