
#include "SDT.h"

float32_t DMAMEM float_buffer_L_3[2048];
float32_t DMAMEM float_buffer_R_3[2048];

float32_t NCO_INC;
float64_t OSC_COS;
float64_t OSC_SIN;
float64_t Osc_Vect_Q = 1.0;
float64_t Osc_Vect_I = 0.0;
float64_t Osc_Gain = 0.0;
float64_t Osc_Q = 0.0;
float64_t Osc_I = 0.0;
int encoderStepOld;
float32_t hh1 = 0.0;
float32_t hh2 = 0.0;

/*****
  Purpose: void FreqShift1()
          AFP 12-31-20
        Frequency translation by Fs/4 without multiplication from Lyons (2011): chapter 13.1.2 page 646
        together with the savings of not having to shift/rotate the FFT_buffer, this saves
        about 1% of processor use

        This is for +Fs/4 [moves receive frequency to the left in the spectrum display]
           float_buffer_L contains I = real values
           float_buffer_R contains Q = imaginary values
           xnew(0) =  xreal(0) + jximag(0)
               leave first value (DC component) as it is!
           xnew(1) =  - ximag(1) + jxreal(1)
  Parameter list:
    void

  Return value:
    void
*****/
void FreqShift1()
{
  for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS; i += 4) {
    hh1 = - float_buffer_R[i + 1];  // xnew(1) =  - ximag(1) + jxreal(1)
    hh2 =   float_buffer_L[i + 1];
    float_buffer_L[i + 1] = hh1;
    float_buffer_R[i + 1] = hh2;
    hh1 = - float_buffer_L[i + 2];
    hh2 = - float_buffer_R[i + 2];
    float_buffer_L[i + 2] = hh1;
    float_buffer_R[i + 2] = hh2;
    hh1 =   float_buffer_R[i + 3];
    hh2 = - float_buffer_L[i + 3];
    float_buffer_L[i + 3] = hh1;
    float_buffer_R[i + 3] = hh2;
  }
  for (unsigned i = 0; i < BUFFER_SIZE * N_BLOCKS; i ++) {
    float_buffer_L_3[i] = float_buffer_L[i];
    float_buffer_R_3[i] = float_buffer_R[i];
  }
  // this is for -Fs/4 [moves receive frequency to the right in the spectrumdisplay]
}


/*****
  Purpose: Shift Receive frequency by an arbitrary amount

  Parameter list:
    void

  Return value;
    void
    Notes:  Routine includes checks to ensure the frequency selection stays within the bounds of the
    displayed spectrum
    Also included a variable frequency step, depending on how fast the encoder id turned.  Step varies from 50Hz/step to 10KHz/step

    freq_conv2()

    FREQUENCY CONVERSION USING A SOFTWARE QUADRATURE OSCILLATOR (NCO)

    THIS VERSION calculates the COS AND SIN WAVE on the fly AND IS SLOW

    MAJOR ADVANTAGE: frequency conversion can be done for any frequency !

    large parts of the code taken from the mcHF code by Clint, KA7OEI, thank you!
      see here for more info on quadrature oscillators:
    Wheatley, M. (2011): CuteSDR Technical Manual Ver. 1.01. - http://sourceforge.net/projects/cutesdr/
    Lyons, R.G. (2011): Understanding Digital Processing. – Pearson, 3rd edition.
    Requires 4 complex multiplies and two adds per data point within the time domain buffer.  Applied after the data
    stream is sent to the Zoom FFT , but befor decimation.
*****/
void FreqShift2()
{
  uint i;
  //long currentFreqAOld;  Not used.  KF5N July 22, 2023
  int sideToneShift = 0;
  int cwFreqOffset;

  if (fineTuneEncoderMove != 0L) {
   // SetFreq();           //AFP 10-04-22
   // ShowFrequency();
   // DrawBandWidthIndicatorBar();
    // ); //AFP 10-04-22
    // EncoderFineTune();      //AFP 10-04-22

    if (NCOFreq > 40000L) {
      NCOFreq = 40000L;
    }
    // ConfigData.centerFreq += ConfigData.freqIncrement;
    currentFreq = ConfigData.centerFreq + NCOFreq;
    //SetFreq(); //AFP 10-04-22
    //ShowFrequency();
  }

  encoderStepOld = fineTuneEncoderMove;
  //currentFreqAOld = TxRxFreq;
  TxRxFreq = ConfigData.centerFreq + NCOFreq;
  //if (abs(currentFreqAOld - TxRxFreq) < 9 * ConfigData.stepFineTune && currentFreqAOld != TxRxFreq) {  // AFP 10-30-22
  //  ShowFrequency();
  //  DrawBandWidthIndicatorBar();
  //}
  if (bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE ) {
    sideToneShift = 0;
  } 
  
    if (bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE ) {
      cwFreqOffset = (ConfigData.CWOffset + 6) * 24000 / 256;
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) sideToneShift = -cwFreqOffset;
        if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) sideToneShift =  cwFreqOffset;
        }

  NCO_INC = 2.0 * PI * (NCOFreq + sideToneShift) / SR[SampleRate].rate; // 192000 SPS is the actual sample rate used in the Receive ADC

  OSC_COS = cos (NCO_INC);
  OSC_SIN = sin (NCO_INC);

  for (i = 0; i < BUFFER_SIZE * N_BLOCKS; i++) {
    // generate local oscillator on-the-fly:  This takes a lot of processor time!
    Osc_Q = (Osc_Vect_Q * OSC_COS) - (Osc_Vect_I * OSC_SIN);  // Q channel of oscillator
    Osc_I = (Osc_Vect_I * OSC_COS) + (Osc_Vect_Q * OSC_SIN);  // I channel of oscillator
    Osc_Gain = 1.95 - ((Osc_Vect_Q * Osc_Vect_Q) + (Osc_Vect_I * Osc_Vect_I));  // Amplitude control of oscillator

    // rotate vectors while maintaining constant oscillator amplitude
    Osc_Vect_Q = Osc_Gain * Osc_Q;
    Osc_Vect_I = Osc_Gain * Osc_I;
    //
    // do actual frequency conversion
    float freqAdjFactor = 1.1;
    float_buffer_L[i] = (float_buffer_L_3[i] * freqAdjFactor * Osc_Q) + (float_buffer_R_3[i] * freqAdjFactor * Osc_I); // multiply I/Q data by sine/cosine data to do translation
    float_buffer_R[i] = (float_buffer_R_3[i] * freqAdjFactor * Osc_Q) - (float_buffer_L_3[i] * freqAdjFactor * Osc_I);
  }
}
