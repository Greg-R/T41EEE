
#include "SDT.h"

int Zoom_FFT_M1;
int Zoom_FFT_M2;
int zoom_sample_ptr = 0;
float32_t LPF_spectrum = 0.82;

void ZoomFFTPrep()
{ // take value of spectrum_zoom and initialize FIR decimation filters for the right values

  /****************************************************************************************
     Zoom FFT: Initiate decimation FIR filters
  ****************************************************************************************/
  // two-stage decimation
  switch (EEPROMData.spectrum_zoom) 
  {
    case SPECTRUM_ZOOM_1:
      Zoom_FFT_M1 = 1; Zoom_FFT_M2 = 1; 
      break;
    case SPECTRUM_ZOOM_2:       
      Zoom_FFT_M1 = 2; Zoom_FFT_M2 = 1; 
      break;
    case SPECTRUM_ZOOM_4:       
      Zoom_FFT_M1 = 2; Zoom_FFT_M2 = 2; 
      break;
    case SPECTRUM_ZOOM_8:       
      Zoom_FFT_M1 = 4; Zoom_FFT_M2 = 2; 
      break;
    case SPECTRUM_ZOOM_16:       
      Zoom_FFT_M1 = 8; Zoom_FFT_M2 = 2; 
      break;
    default:
      Zoom_FFT_M1 = 1; Zoom_FFT_M2 = 1; 
      break;
  }
// init 1st stage 
  float32_t Fstop_Zoom = 0.5 * (float32_t) SR[SampleRate].rate / (1 << EEPROMData.spectrum_zoom); // Fstop should be the stop band at the final sample rate

#define Zoom_FFT_no_coeff1 12
#define Zoom_FFT_no_coeff2 8
// did not do proper calculation of the number of taps, just took a number for the start
// attenuation 70dB should be sufficient for the spectrum display
// 1st decimation stage
CalcFIRCoeffs (Fir_Zoom_FFT_Decimate1_coeffs, Zoom_FFT_no_coeff1, Fstop_Zoom, 60, 0, 0.0, (float32_t)SR[SampleRate].rate);

//[in,out]  S points to an instance of the floating-point FIR decimator structure
//[in]  numTaps number of coefficients in the filter
//[in]  M decimation factor
//[in]  pCoeffs points to the filter coefficients
//[in]  pState  points to the state buffer
//[in]  blockSize number of input samples to process per call 

  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I1, Zoom_FFT_no_coeff1, Zoom_FFT_M1, Fir_Zoom_FFT_Decimate1_coeffs, Fir_Zoom_FFT_Decimate_I1_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  // same coefficients, but specific state variables
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q1, Zoom_FFT_no_coeff1, Zoom_FFT_M1, Fir_Zoom_FFT_Decimate1_coeffs, Fir_Zoom_FFT_Decimate_Q1_state, BUFFER_SIZE * N_BLOCKS)) {
    Serial.println("Init of decimation failed");
    while(1);
  }

// 2nd decimation stage
  CalcFIRCoeffs (Fir_Zoom_FFT_Decimate2_coeffs, Zoom_FFT_no_coeff2, Fstop_Zoom, 60, 0, 0.0, (float32_t)SR[SampleRate].rate / Zoom_FFT_M1);

  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_I2, Zoom_FFT_no_coeff2, Zoom_FFT_M2, Fir_Zoom_FFT_Decimate2_coeffs, Fir_Zoom_FFT_Decimate_I2_state, BUFFER_SIZE * N_BLOCKS / Zoom_FFT_M1)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  // same coefficients, but specific state variables
  if (arm_fir_decimate_init_f32(&Fir_Zoom_FFT_Decimate_Q2, Zoom_FFT_no_coeff2, Zoom_FFT_M2, Fir_Zoom_FFT_Decimate2_coeffs, Fir_Zoom_FFT_Decimate_Q2_state, BUFFER_SIZE * N_BLOCKS / Zoom_FFT_M1)) {
    Serial.println("Init of decimation failed");
    while(1);
  }
  zoom_sample_ptr = 0;
}


const int fftWidth = 512;
void ZoomFFTExe (uint32_t blockSize)
{
  // totally rebuilt 27.8.2020 DD4WH
  // however, I did not manage to implement a correct routine for magnifications > 2048x
  // maybe the next days
  float32_t x_buffer[blockSize]; // can be 2048 (FFT length == 512), or 4096 [FFT length == 1024] or even 8192 [FFT length == 2048]
  float32_t y_buffer[blockSize];
  static float32_t FFT_ring_buffer_x[fftWidth*2];
  static float32_t FFT_ring_buffer_y[fftWidth*2];
  static int32_t flag_2nd_decimation = 0;
  //static uint32_t high_Zoom_buffer_ptr = 0;
  uint8_t high_Zoom = 0;
  //uint32_t high_Zoom_2nd_dec_rounds = (1 << (EEPROMData.spectrum_zoom - 11));
  //Serial.print("2nd dec rounds"); Serial.println(high_Zoom_2nd_dec_rounds);
  int sample_no = 256;
  // sample_no is 256, in high magnify modes it is smaller!
  // but it must never be > 256

  sample_no = BUFFER_SIZE * N_BLOCKS / (1 << EEPROMData.spectrum_zoom);

  if (sample_no > fftWidth)
  {
    sample_no = fftWidth;
  }
 
      // decimation stage 1
          arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_I1, float_buffer_L, x_buffer, blockSize);
          arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_Q1, float_buffer_R, y_buffer, blockSize);
          if(high_Zoom ==1) flag_2nd_decimation++;
      // decimation stage 2
            arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_I2, x_buffer, x_buffer, blockSize / Zoom_FFT_M1);
            arm_fir_decimate_f32(&Fir_Zoom_FFT_Decimate_Q2, y_buffer, y_buffer, blockSize / Zoom_FFT_M1);
  
    //this puts the sample_no samples into the ringbuffer -->
    // the right order has to be thought about!
    // we take all the samples from zoom_sample_ptr to 256 and
    // then all samples from 0 to zoom_sampl_ptr - 1

    // fill into ringbuffer
    for (int i = 0; i < sample_no; i++)
    { // interleave real and imaginary input values [real, imag, real, imag . . .]
      FFT_ring_buffer_x[zoom_sample_ptr] = x_buffer[i];
      FFT_ring_buffer_y[zoom_sample_ptr] = y_buffer[i];
      zoom_sample_ptr++;
      if (zoom_sample_ptr >= fftWidth) zoom_sample_ptr = 0;
    }
 
  // when do we want to display a new spectrum?
  // if we wait for zoom_sample_ptr to be 255,
  // it can last more than a few seconds in 4096x Zoom
  // so we calculate an FFT and display the spectrum every time this function is called?

//    zoom_display = 1;

    // copy from ringbuffer to FFT_buffer
    // in the right order and
    // apply FFT window here
    // Nuttall window
    // zoom_sample_ptr points to the oldest sample now

    float32_t multiplier = (float32_t)EEPROMData.spectrum_zoom * (float32_t)EEPROMData.spectrum_zoom;
    
    for (int idx = 0; idx < fftWidth; idx++)
    {
   //   buffer_spec_FFT[idx * 2 + 0] =  multiplier * FFT_ring_buffer_x[zoom_sample_ptr] * nuttallWindow256[idx];
   //   buffer_spec_FFT[idx * 2 + 1] =  multiplier * FFT_ring_buffer_y[zoom_sample_ptr] * nuttallWindow256[idx];
      buffer_spec_FFT[idx * 2 + 0] =  multiplier * FFT_ring_buffer_x[zoom_sample_ptr] * (0.5 - 0.5 * cos(6.28 * idx / SPECTRUM_RES)); //Hanning Window AFP 03-12-21
      buffer_spec_FFT[idx * 2 + 1] =  multiplier * FFT_ring_buffer_y[zoom_sample_ptr] * (0.5 - 0.5 * cos(6.28 * idx / SPECTRUM_RES));
      zoom_sample_ptr++;
      if (zoom_sample_ptr >= fftWidth) zoom_sample_ptr = 0;
    }

    //***************
    // adjust lowpass filter coefficient, so that
    // "spectrum display smoothness" is the same across the different sample rates
    // and the same across different magnify modes . . .
//    float32_t LPFcoeff = LPF_spectrum * (AUDIO_SAMPLE_RATE_EXACT / SR[SampleRate].rate);
    float32_t LPFcoeff = 0.6;
    if (LPFcoeff > 1.0) LPFcoeff = 1.0;
    if (LPFcoeff < 0.001) LPFcoeff = 0.001;
    float32_t onem_LPFcoeff = 1.0 - LPFcoeff;

    // The rest of the function is activated when the buffers are full and ready.
    // Save old pixels for lowpass filter.
    if(updateDisplayFlag == 1) {
    for (int i = 0; i < fftWidth; i++)
    {
      pixelold[i] = pixelCurrent[i];
    }
    // perform complex FFT
    // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
    arm_cfft_f32(spec_FFT, buffer_spec_FFT, 0, 1);
    // calculate mag = I*I + Q*Q,
    // and simultaneously put them into the right order
          for (int i = 0; i < fftWidth/2; i++)
          {
            FFT_spec[i + fftWidth/2] = (buffer_spec_FFT[i * 2] * buffer_spec_FFT[i * 2] + buffer_spec_FFT[i * 2 + 1] * buffer_spec_FFT[i * 2 + 1]);
            FFT_spec[i] = (buffer_spec_FFT[(i + fftWidth/2) * 2] * buffer_spec_FFT[(i + fftWidth/2)  * 2] + buffer_spec_FFT[(i + fftWidth/2)  * 2 + 1] * buffer_spec_FFT[(i + fftWidth/2)  * 2 + 1]);
          }
    // apply low pass filter and scale the magnitude values and convert to int for spectrum display
    // apply spectrum AGC
    //
    for (int16_t x = 0; x < fftWidth; x++)
    {
      FFT_spec[x] = LPFcoeff * FFT_spec[x] + onem_LPFcoeff * FFT_spec_old[x];
      FFT_spec_old[x] = FFT_spec[x];
    }

    for (int16_t x = 0; x < fftWidth; x++)
    {
      pixelnew[x] = displayScale[EEPROMData.currentScale].baseOffset + bands[EEPROMData.currentBand].pixel_offset + (int16_t)(displayScale[EEPROMData.currentScale].dBScale * log10f_fast(FFT_spec[x]));
      if (pixelnew[x] > 220)   pixelnew[x] = 220;
    }
}
}


/*****
  Purpose: CalcZoom1Magn()
  Parameter list:
    void
  Return value;
    void
    Used when Spectrum Zoom =1
*****/
void CalcZoom1Magn()
{
 if (updateDisplayFlag == 1) {
  float32_t spec_help = 0.0;
  float32_t LPFcoeff = 0.7;  // Is this a global or not?
  if (LPFcoeff > 1.0) {
    LPFcoeff = 1.0;
  }
  for (int i = 0; i < fftWidth; i++) {
    pixelold[i] = pixelCurrent[i];
  }

  for (int i = 0; i < fftWidth; i++) { // interleave real and imaginary input values [real, imag, real, imag . . .]
    buffer_spec_FFT[i * 2] =      float_buffer_L[i] * (0.5 - 0.5 * cos(6.28 * i / fftWidth)); //Hanning
    buffer_spec_FFT[i * 2 + 1] =  float_buffer_R[i] * (0.5 - 0.5 * cos(6.28 * i / fftWidth));
  }
  // perform complex FFT
  // calculation is performed in-place the FFT_buffer [re, im, re, im, re, im . . .]
  arm_cfft_f32(spec_FFT, buffer_spec_FFT, 0, 1);

  // calculate magnitudes and put into FFT_spec
  // we do not need to calculate magnitudes with square roots, it would seem to be sufficient to
  // calculate mag = I*I + Q*Q, because we are doing a log10-transformation later anyway
  // and simultaneously put them into the right order
  // 38.50%, saves 0.05% of processor power and 1kbyte RAM ;-)

  for (int i = 0; i < SPECTRUM_RES/2; i++) {
      FFT_spec[i + SPECTRUM_RES/2] = (buffer_spec_FFT[i * 2] * buffer_spec_FFT[i * 2] + buffer_spec_FFT[i * 2 + 1] * buffer_spec_FFT[i * 2 + 1]);
      FFT_spec[i]                  = (buffer_spec_FFT[(i + SPECTRUM_RES/2) * 2] * buffer_spec_FFT[(i + SPECTRUM_RES/2)  * 2] + buffer_spec_FFT[(i + SPECTRUM_RES/2)  * 2 + 1] * buffer_spec_FFT[(i + SPECTRUM_RES/2)  * 2 + 1]);
    }
  // apply low pass filter and scale the magnitude values and convert to int for spectrum display

  for (int16_t x = 0; x < SPECTRUM_RES; x++) {
    spec_help = EEPROMData.LPFcoeff * FFT_spec[x] + (1.0 - EEPROMData.LPFcoeff) * FFT_spec_old[x];
    FFT_spec_old[x] = spec_help;

#ifdef USE_LOG10FAST
    pixelnew[x] = displayScale[EEPROMData.currentScale].baseOffset + bands[EEPROMData.currentBand].pixel_offset + (int16_t) (displayScale[EEPROMData.currentScale].dBScale * log10f_fast(FFT_spec[x]));
#else
    pixelnew[x] = displayScale[EEPROMData.currentScale].baseOffset + bands[EEPROMData.currentBand].pixel_offset + (int16_t) (displayScale[EEPROMData.currentScale].dBScale * log10f(spec_help));
#endif
  }
 }
} // end calc_256_magn
