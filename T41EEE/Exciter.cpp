
#include "SDT.h"

/*****
  Purpose: The SSB exciter: Create I and Q signals from Mic input.
  Parameter list: none

  Return value; void
    Notes:
    There are several actions in this function:
    1.  Read in the data from the ADC into the Left Channel at 192KHz.
    2.  Format the left channel data and decimate (downsample and filter)the sampled data by x8.
          - the new effective sampling rate is now 24KHz.
    3.  Process the left channel data through the 7 EQ filters and combine to a single data stream.
    4.  Copy the left (I) channel to the Q channel
    5.  Process the I and Q channels through two Hilbert Transformers - I 45 deg phase shift and Q -45 deg phase shift.
          - Now I and Q are phase-shifted by 90 degrees relative to each other.
    6.  Interpolate 8x (upsample and filter) the data stream to 192KHz sample rate.
    7.  Output the data stream through the DACs at 192KHz and into the modulator.
*****/

void ExciterIQData()
{
  uint32_t N_BLOCKS_EX = N_B_EX;
  float32_t powerScale;

  /**********************************************************************************  AFP 12-31-20
        Get samples from queue buffers
        Teensy Audio Library stores ADC data in two buffers size=128, Q_in_L and Q_in_R as initiated from the audio lib.
        Then the buffers are  read into two arrays sp_L and sp_R in blocks of 128 up to N_BLOCKS.  The arrarys are
        of size BUFFER_SIZE*N_BLOCKS.  BUFFER_SIZE is 128.
        N_BLOCKS = FFT_L / 2 / BUFFER_SIZE * (uint32_t)DF; // should be 16 with DF == 8 and FFT_L = 512
        BUFFER_SIZE*N_BLOCKS = 2024 samples
     **********************************************************************************/
  // are there at least N_BLOCKS buffers in each channel available ?
  if ( (uint32_t) Q_in_L_Ex.available() > N_BLOCKS_EX) {

    // get audio samples from the audio  buffers and convert them to float
    // read in 32 blocks á 128 samples in I and Q
    for (unsigned i = 0; i < N_BLOCKS_EX; i++) {
//      sp_L2 = Q_in_L_Ex.readBuffer();
//      sp_R2 = Q_in_R_Ex.readBuffer();

      /**********************************************************************************  AFP 12-31-20
          Using arm_Math library, convert to float one buffer_size.
          Float_buffer samples are now standardized from > -1.0 to < 1.0
      **********************************************************************************/
      arm_q15_to_float (Q_in_L_Ex.readBuffer(), &float_buffer_L_EX[BUFFER_SIZE * i], BUFFER_SIZE); // convert int_buffer to float 32bit
//      arm_q15_to_float (Q_in_R_Ex.readBuffer(), &float_buffer_R_EX[BUFFER_SIZE * i], BUFFER_SIZE); // Right channel not used.  KF5N March 11, 2024
      Q_in_L_Ex.freeBuffer();
//      Q_in_R_Ex.freeBuffer(); // Right channel not used.  KF5N March 11, 2024
    }

    float exciteMaxL = 0;

    /**********************************************************************************  AFP 12-31-20
              Decimation is the process of downsampling the data stream and LP filtering
              Decimation is done in two stages to prevent reversal of the spectrum, which occurs with each even
              Decimation.  First select every 4th sample and then every 2nd sample, yielding 8x downsampling
              192KHz/8 = 24KHz, with 8xsmaller sample sizes
     **********************************************************************************/

    // 192KHz effective sample rate here
    // decimation-by-4 in-place!
    arm_fir_decimate_f32(&FIR_dec1_EX_I, float_buffer_L_EX, float_buffer_L_EX, BUFFER_SIZE * N_BLOCKS_EX );
//    arm_fir_decimate_f32(&FIR_dec1_EX_Q, float_buffer_R_EX, float_buffer_R_EX, BUFFER_SIZE * N_BLOCKS_EX ); // Right channel not used.  KF5N March 11, 2024
    // 48KHz effective sample rate here
    // decimation-by-2 in-place
    arm_fir_decimate_f32(&FIR_dec2_EX_I, float_buffer_L_EX, float_buffer_L_EX, 512);
//    arm_fir_decimate_f32(&FIR_dec2_EX_Q, float_buffer_R_EX, float_buffer_R_EX, 512); // Right channel not used.  KF5N March 11, 2024

    //============================  Transmit EQ  ========================  AFP 10-02-22
    if (EEPROMData.xmitEQFlag == ON ) {
      DoExciterEQ();  // The exciter equalizer works with left channel data only.
    }

    // Microphone audio has only 1 channel, so copy left to right.
    arm_copy_f32 (float_buffer_L_EX, float_buffer_R_EX, 256);

    // =========================    End CW Xmit
    //--------------  Hilbert Transformers

    /**********************************************************************************
             R and L channels are processed though the two Hilbert Transformers, L at 0 deg and R at 90 deg
             Tthe result are the quadrature data streans, I and Q necessary for Phasing calculations to
             create the SSB signals.
             Two Hilbert Transformers are used to preserve eliminate the relative time delays created during processing of the data
    **********************************************************************************/
    arm_fir_f32(&FIR_Hilbert_L, float_buffer_L_EX, float_buffer_L_EX, 256);
    arm_fir_f32(&FIR_Hilbert_R, float_buffer_R_EX, float_buffer_R_EX, 256);

    /**********************************************************************************
              Additional scaling, if nesessary to compensate for down-stream gain variations
     **********************************************************************************/

    if (bands[EEPROMData.currentBand].mode == DEMOD_LSB) { //AFP 12-27-21
      //arm_scale_f32 (float_buffer_L_EX, -EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);
      arm_scale_f32 (float_buffer_L_EX, + EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);     // Flip SSB sideband KF5N, minus sign was original
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBandA], 256);
    }
    else if (bands[EEPROMData.currentBand].mode == DEMOD_USB) { //AFP 12-27-21
      //arm_scale_f32 (float_buffer_L_EX, + EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);     // Flip SSB sideband KF5N, minus sign was original
      arm_scale_f32 (float_buffer_L_EX, - EEPROMData.IQXAmpCorrectionFactor[EEPROMData.currentBandA], float_buffer_L_EX, 256);    // Flip SSB sideband KF5N
      IQPhaseCorrection(float_buffer_L_EX, float_buffer_R_EX, EEPROMData.IQXPhaseCorrectionFactor[EEPROMData.currentBandA], 256);
    }
    arm_scale_f32 (float_buffer_R_EX, 1.00, float_buffer_R_EX, 256);

    exciteMaxL = 0;
    for (int k = 0; k < 256; k++) {
      if (float_buffer_L_EX[k] > exciteMaxL) {
        exciteMaxL = float_buffer_L_EX[k];
      }
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
    
    //  This is the correct place in the data stream to inject the scaling for power.
#ifdef QSE2
powerScale = 40.0 * EEPROMData.powerOutSSB[EEPROMData.currentBand];
#else
powerScale = 30.0 * EEPROMData.powerOutSSB[EEPROMData.currentBand];
#endif

    arm_scale_f32(float_buffer_L_EX, powerScale, float_buffer_L_EX, 2048); //Scale to compensate for losses in Interpolation
    arm_scale_f32(float_buffer_R_EX, powerScale, float_buffer_R_EX, 2048);

    /**********************************************************************************  AFP 12-31-20
      CONVERT TO INTEGER AND PLAY AUDIO
    **********************************************************************************/
    q15_t q15_buffer_LTemp[2048];  // KF5N
    q15_t q15_buffer_RTemp[2048];  // KF5N
 
      arm_float_to_q15 (float_buffer_L_EX, q15_buffer_LTemp, 2048);
      arm_float_to_q15 (float_buffer_R_EX, q15_buffer_RTemp, 2048);
      #ifdef QSE2
      arm_offset_q15(q15_buffer_LTemp, EEPROMData.iDCoffset[EEPROMData.currentBand] + EEPROMData.dacOffset, q15_buffer_LTemp, 2048);  // Carrier suppression offset.
      arm_offset_q15(q15_buffer_RTemp, EEPROMData.qDCoffset[EEPROMData.currentBand] + EEPROMData.dacOffset, q15_buffer_RTemp, 2048);
      #endif
      Q_out_L_Ex.play(q15_buffer_LTemp, 2048); // play it!  This is the I channel from the Audio Adapter line out to QSE I input.
      Q_out_R_Ex.play(q15_buffer_RTemp, 2048); // play it!  This is the Q channel from the Audio Adapter line out to QSE Q input.

    /*
    for (unsigned  i = 0; i < N_BLOCKS_EX; i++) {  //N_BLOCKS_EX=16  BUFFER_SIZE=128 16x128=2048
//      sp_L2 = Q_out_L_Ex.getBuffer();
//      sp_R2 = Q_out_R_Ex.getBuffer();
      arm_float_to_q15 (&float_buffer_L_EX[BUFFER_SIZE * i], Q_out_L_Ex.getBuffer(), BUFFER_SIZE);
      arm_float_to_q15 (&float_buffer_R_EX[BUFFER_SIZE * i], Q_out_R_Ex.getBuffer(), BUFFER_SIZE);
      arm_offset_q15(Q_out_L_Ex.getBuffer(), EEPROMData.iDCoffset[EEPROMData.currentBand] + 1900, Q_out_L_Ex.getBuffer(), 128);  // Carrier suppression offset.
      arm_offset_q15(Q_out_R_Ex.getBuffer(), EEPROMData.qDCoffset[EEPROMData.currentBand] + 1900, Q_out_R_Ex.getBuffer(), 128);
      Q_out_L_Ex.playBuffer(); // play it !
      Q_out_R_Ex.playBuffer(); // play it !
    }
    */

  }
}

/*****
  Purpose: Set the current band relay ON or OFF

  Parameter list:
    int state             OFF = 0, ON = 1

  Return value;
    void
*****/
void SetBandRelay(int state)
{
  // There are 4 physical relays.  Turn all of them off.
  for(int i = 0; i < 4; i = i + 1) {
  digitalWrite(bandswitchPins[i], LOW); // Set ALL band relays low.  KF5N July 21, 2023
  }
// Set current band relay "on".  Ignore 12M and 10M.  15M and 17M use the same relay.  KF5N September 27, 2023.
  if(EEPROMData.currentBand < 5) digitalWrite(bandswitchPins[EEPROMData.currentBand], state);  
}
