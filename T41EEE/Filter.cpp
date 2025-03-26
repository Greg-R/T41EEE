
#include "SDT.h"

#define IIR_ORDER 8
#define IIR_NUMSTAGES (IIR_ORDER / 2)
uint32_t m_NumTaps = (FFT_LENGTH / 2) + 1;
float32_t xmtEqScaleFactor = 150.0;

//================== Receive EQ Variables================= AFP 08-08-22
//Setup for EQ filters
float32_t DMAMEM EQ1_float_buffer_L[256];
float32_t DMAMEM EQ2_float_buffer_L[256];
float32_t DMAMEM EQ3_float_buffer_L[256];
float32_t DMAMEM EQ4_float_buffer_L[256];
float32_t DMAMEM EQ5_float_buffer_L[256];
float32_t DMAMEM EQ6_float_buffer_L[256];
float32_t DMAMEM EQ7_float_buffer_L[256];
float32_t DMAMEM EQ8_float_buffer_L[256];
float32_t DMAMEM EQ9_float_buffer_L[256];
float32_t DMAMEM EQ10_float_buffer_L[256];
float32_t DMAMEM EQ11_float_buffer_L[256];
float32_t DMAMEM EQ12_float_buffer_L[256];
float32_t DMAMEM EQ13_float_buffer_L[256];
float32_t DMAMEM EQ14_float_buffer_L[256];

float32_t rec_EQ_Band1_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t rec_EQ_Band2_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band3_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band4_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band5_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band6_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band7_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band8_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t rec_EQ_Band9_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band10_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band11_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band12_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band13_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t rec_EQ_Band14_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//EQ filter instances
arm_biquad_cascade_df2T_instance_f32 S1_Rec = { IIR_NUMSTAGES, rec_EQ_Band1_state, EQ_Band1Coeffs };
arm_biquad_cascade_df2T_instance_f32 S2_Rec = { IIR_NUMSTAGES, rec_EQ_Band2_state, EQ_Band2Coeffs };
arm_biquad_cascade_df2T_instance_f32 S3_Rec = { IIR_NUMSTAGES, rec_EQ_Band3_state, EQ_Band3Coeffs };
arm_biquad_cascade_df2T_instance_f32 S4_Rec = { IIR_NUMSTAGES, rec_EQ_Band4_state, EQ_Band4Coeffs };
arm_biquad_cascade_df2T_instance_f32 S5_Rec = { IIR_NUMSTAGES, rec_EQ_Band5_state, EQ_Band5Coeffs };
arm_biquad_cascade_df2T_instance_f32 S6_Rec = { IIR_NUMSTAGES, rec_EQ_Band6_state, EQ_Band6Coeffs };
arm_biquad_cascade_df2T_instance_f32 S7_Rec = { IIR_NUMSTAGES, rec_EQ_Band7_state, EQ_Band7Coeffs };
arm_biquad_cascade_df2T_instance_f32 S8_Rec = { IIR_NUMSTAGES, rec_EQ_Band8_state, EQ_Band8Coeffs };
arm_biquad_cascade_df2T_instance_f32 S9_Rec = { IIR_NUMSTAGES, rec_EQ_Band9_state, EQ_Band9Coeffs };
arm_biquad_cascade_df2T_instance_f32 S10_Rec = { IIR_NUMSTAGES, rec_EQ_Band10_state, EQ_Band10Coeffs };
arm_biquad_cascade_df2T_instance_f32 S11_Rec = { IIR_NUMSTAGES, rec_EQ_Band11_state, EQ_Band11Coeffs };
arm_biquad_cascade_df2T_instance_f32 S12_Rec = { IIR_NUMSTAGES, rec_EQ_Band12_state, EQ_Band12Coeffs };
arm_biquad_cascade_df2T_instance_f32 S13_Rec = { IIR_NUMSTAGES, rec_EQ_Band13_state, EQ_Band13Coeffs };
arm_biquad_cascade_df2T_instance_f32 S14_Rec = { IIR_NUMSTAGES, rec_EQ_Band14_state, EQ_Band14Coeffs };

// ===============================  AFP 10-02-22 ================

float32_t xmt_EQ_Band1_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //declare and zero biquad state variables
float32_t xmt_EQ_Band2_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band3_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band4_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band5_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band6_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band7_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band8_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band9_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band10_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band11_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band12_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band13_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };
float32_t xmt_EQ_Band14_state[IIR_NUMSTAGES * 2] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//EQ filter instances
arm_biquad_cascade_df2T_instance_f32 S1_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band1_state, EQ_Band1Coeffs };
arm_biquad_cascade_df2T_instance_f32 S2_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band2_state, EQ_Band2Coeffs };
arm_biquad_cascade_df2T_instance_f32 S3_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band3_state, EQ_Band3Coeffs };
arm_biquad_cascade_df2T_instance_f32 S4_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band4_state, EQ_Band4Coeffs };
arm_biquad_cascade_df2T_instance_f32 S5_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band5_state, EQ_Band5Coeffs };
arm_biquad_cascade_df2T_instance_f32 S6_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band6_state, EQ_Band6Coeffs };
arm_biquad_cascade_df2T_instance_f32 S7_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band7_state, EQ_Band7Coeffs };
arm_biquad_cascade_df2T_instance_f32 S8_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band8_state, EQ_Band8Coeffs };
arm_biquad_cascade_df2T_instance_f32 S9_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band9_state, EQ_Band9Coeffs };
arm_biquad_cascade_df2T_instance_f32 S10_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band10_state, EQ_Band10Coeffs };
arm_biquad_cascade_df2T_instance_f32 S11_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band11_state, EQ_Band11Coeffs };
arm_biquad_cascade_df2T_instance_f32 S12_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band12_state, EQ_Band12Coeffs };
arm_biquad_cascade_df2T_instance_f32 S13_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band13_state, EQ_Band13Coeffs };
arm_biquad_cascade_df2T_instance_f32 S14_Xmt = { IIR_NUMSTAGES, xmt_EQ_Band14_state, EQ_Band14Coeffs };
//===============================End xmit EQ filter setup  AFP 10-02-22 ================


/*****
  Purpose: void DoReceiveEQ  Parameter list:
    void
  Return value;
    void
*****/
void DoReceiveEQ() //AFP 08-09-22
{
//  for (int i = 0; i < 14; i++) {
//    recEQ_LevelScale[i] = (float)ConfigData.equalizerRec[i] / 100.0;
//  }
  arm_biquad_cascade_df2T_f32(&S1_Rec, float_buffer_L, EQ1_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S2_Rec, float_buffer_L, EQ2_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S3_Rec, float_buffer_L, EQ3_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S4_Rec, float_buffer_L, EQ4_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S5_Rec, float_buffer_L, EQ5_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S6_Rec, float_buffer_L, EQ6_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S7_Rec, float_buffer_L, EQ7_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S8_Rec, float_buffer_L, EQ8_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S9_Rec, float_buffer_L, EQ9_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S10_Rec, float_buffer_L, EQ10_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S11_Rec, float_buffer_L, EQ11_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S12_Rec, float_buffer_L, EQ12_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S13_Rec, float_buffer_L, EQ13_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S14_Rec, float_buffer_L, EQ14_float_buffer_L, 256);

  arm_scale_f32(EQ1_float_buffer_L, -(float)ConfigData.equalizerRec[0] / 100.0, EQ1_float_buffer_L, 256);
  arm_scale_f32(EQ2_float_buffer_L, (float)ConfigData.equalizerRec[1] / 100.0, EQ2_float_buffer_L, 256);
  arm_scale_f32(EQ3_float_buffer_L, -(float)ConfigData.equalizerRec[2] / 100.0, EQ3_float_buffer_L, 256);
  arm_scale_f32(EQ4_float_buffer_L, (float)ConfigData.equalizerRec[3] / 100.0, EQ4_float_buffer_L, 256);
  arm_scale_f32(EQ5_float_buffer_L, -(float)ConfigData.equalizerRec[4] / 100.0, EQ5_float_buffer_L, 256);
  arm_scale_f32(EQ6_float_buffer_L, (float)ConfigData.equalizerRec[5] / 100.0, EQ6_float_buffer_L, 256);
  arm_scale_f32(EQ7_float_buffer_L, -(float)ConfigData.equalizerRec[6] / 100.0, EQ7_float_buffer_L, 256);
  arm_scale_f32(EQ8_float_buffer_L, (float)ConfigData.equalizerRec[7] / 100.0, EQ8_float_buffer_L, 256);
  arm_scale_f32(EQ9_float_buffer_L, -(float)ConfigData.equalizerRec[8] / 100.0, EQ9_float_buffer_L, 256);
  arm_scale_f32(EQ10_float_buffer_L, (float)ConfigData.equalizerRec[9] / 100.0, EQ10_float_buffer_L, 256);
  arm_scale_f32(EQ11_float_buffer_L, -(float)ConfigData.equalizerRec[10] / 100.0, EQ11_float_buffer_L, 256);
  arm_scale_f32(EQ12_float_buffer_L, (float)ConfigData.equalizerRec[11] / 100.0, EQ12_float_buffer_L, 256);
  arm_scale_f32(EQ13_float_buffer_L, -(float)ConfigData.equalizerRec[12] / 100.0, EQ13_float_buffer_L, 256);
  arm_scale_f32(EQ14_float_buffer_L, (float)ConfigData.equalizerRec[13] / 100.0, EQ14_float_buffer_L, 256);

  arm_add_f32(EQ1_float_buffer_L , EQ2_float_buffer_L, float_buffer_L , 256 ) ;

  arm_add_f32(float_buffer_L , EQ3_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ4_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ5_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ6_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ7_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ8_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ9_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ10_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ11_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ12_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ13_float_buffer_L, float_buffer_L , 256 ) ;
  arm_add_f32(float_buffer_L , EQ14_float_buffer_L, float_buffer_L , 256 ) ;
}


/*****
  Purpose: void DoExciterEQ

  Parameter list:
    void
  Return value;
    void
*****/
void DoExciterEQ() //AFP 10-02-22
{
  arm_biquad_cascade_df2T_f32(&S1_Xmt,  float_buffer_L_EX, EQ1_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S2_Xmt,  float_buffer_L_EX, EQ2_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S3_Xmt,  float_buffer_L_EX, EQ3_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S4_Xmt,  float_buffer_L_EX, EQ4_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S5_Xmt,  float_buffer_L_EX, EQ5_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S6_Xmt,  float_buffer_L_EX, EQ6_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S7_Xmt,  float_buffer_L_EX, EQ7_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S8_Xmt,  float_buffer_L_EX, EQ8_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S9_Xmt,  float_buffer_L_EX, EQ9_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S10_Xmt, float_buffer_L_EX, EQ10_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S11_Xmt, float_buffer_L_EX, EQ11_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S12_Xmt, float_buffer_L_EX, EQ12_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S13_Xmt, float_buffer_L_EX, EQ13_float_buffer_L, 256);
  arm_biquad_cascade_df2T_f32(&S14_Xmt, float_buffer_L_EX, EQ14_float_buffer_L, 256);

  arm_scale_f32(EQ1_float_buffer_L,  -(float)ConfigData.equalizerXmt[0] / xmtEqScaleFactor,  EQ1_float_buffer_L, 256);
  arm_scale_f32(EQ2_float_buffer_L,   (float)ConfigData.equalizerXmt[1] / xmtEqScaleFactor,  EQ2_float_buffer_L, 256);
  arm_scale_f32(EQ3_float_buffer_L,  -(float)ConfigData.equalizerXmt[2] / xmtEqScaleFactor,  EQ3_float_buffer_L, 256);
  arm_scale_f32(EQ4_float_buffer_L,   (float)ConfigData.equalizerXmt[3] / xmtEqScaleFactor,  EQ4_float_buffer_L, 256);
  arm_scale_f32(EQ5_float_buffer_L,  -(float)ConfigData.equalizerXmt[4] / xmtEqScaleFactor,  EQ5_float_buffer_L, 256);
  arm_scale_f32(EQ6_float_buffer_L,   (float)ConfigData.equalizerXmt[5] / xmtEqScaleFactor,  EQ6_float_buffer_L, 256);
  arm_scale_f32(EQ7_float_buffer_L,  -(float)ConfigData.equalizerXmt[6] / xmtEqScaleFactor,  EQ7_float_buffer_L, 256);
  arm_scale_f32(EQ8_float_buffer_L,   (float)ConfigData.equalizerXmt[7] / xmtEqScaleFactor,  EQ8_float_buffer_L, 256);
  arm_scale_f32(EQ9_float_buffer_L,  -(float)ConfigData.equalizerXmt[8] / xmtEqScaleFactor,  EQ9_float_buffer_L, 256);
  arm_scale_f32(EQ10_float_buffer_L,  (float)ConfigData.equalizerXmt[9] / xmtEqScaleFactor,  EQ10_float_buffer_L, 256);
  arm_scale_f32(EQ11_float_buffer_L, -(float)ConfigData.equalizerXmt[10] / xmtEqScaleFactor, EQ11_float_buffer_L, 256);
  arm_scale_f32(EQ12_float_buffer_L,  (float)ConfigData.equalizerXmt[11] / xmtEqScaleFactor, EQ12_float_buffer_L, 256);
  arm_scale_f32(EQ13_float_buffer_L, -(float)ConfigData.equalizerXmt[12] / xmtEqScaleFactor, EQ13_float_buffer_L, 256);
  arm_scale_f32(EQ14_float_buffer_L,  (float)ConfigData.equalizerXmt[13] / xmtEqScaleFactor, EQ14_float_buffer_L, 256);

  arm_add_f32(EQ1_float_buffer_L , EQ2_float_buffer_L, float_buffer_L_EX , 256 ) ;

  arm_add_f32(float_buffer_L_EX , EQ3_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ4_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ5_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ6_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ7_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ8_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ9_float_buffer_L,  float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ10_float_buffer_L, float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ11_float_buffer_L, float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ12_float_buffer_L, float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ13_float_buffer_L, float_buffer_L_EX , 256 ) ;
  arm_add_f32(float_buffer_L_EX , EQ14_float_buffer_L, float_buffer_L_EX , 256 ) ;
}


/*****
  Purpose: Adjust the audio filter band limits based on user input from encoder.
    void
  Return value;
    void
*****/
void FilterBandwidth()
{
////  AudioNoInterrupts();

//Serial.printf("FilterBandwidth()\n");
//Serial.printf("bands.bands[ConfigData.currentBand].FHiCut = %d bands.bands[ConfigData.currentBand].FLoCut = %d\n", bands.bands[ConfigData.currentBand].FHiCut, bands.bands[ConfigData.currentBand].FLoCut);
//Serial.printf("bands.bands[ConfigData.currentBand].FAMCut = %d\n", bands.bands[ConfigData.currentBand].FAMCut);
//Serial.printf("bands.bands[ConfigData.currentBand].sideband = %d\n", bands.bands[ConfigData.currentBand].sideband);


// The filter must be set up differently for AM and SAM modes.
if(bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE or bands.bands[ConfigData.currentBand].mode == RadioMode::CW_MODE or bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) {

switch(bands.bands[ConfigData.currentBand].sideband) {

case Sideband::LOWER:
  CalcCplxFIRCoeffs(FIR_Coef_I, FIR_Coef_Q, m_NumTaps, static_cast<float>(-bands.bands[ConfigData.currentBand].FHiCut), static_cast<float>(-bands.bands[ConfigData.currentBand].FLoCut), static_cast<float>(SR[SampleRate].rate / DF));
break;

case Sideband::UPPER:
CalcCplxFIRCoeffs(FIR_Coef_I, FIR_Coef_Q, m_NumTaps, static_cast<float>(bands.bands[ConfigData.currentBand].FLoCut), static_cast<float>(bands.bands[ConfigData.currentBand].FHiCut), static_cast<float>(SR[SampleRate].rate / DF));
break;

default:
break;
}
}

if(bands.bands[ConfigData.currentBand].mode == RadioMode::AM_MODE or bands.bands[ConfigData.currentBand].mode == RadioMode::SAM_MODE) {
  CalcCplxFIRCoeffs(FIR_Coef_I, FIR_Coef_Q, m_NumTaps, static_cast<float>(-bands.bands[ConfigData.currentBand].FAMCut), static_cast<float>(bands.bands[ConfigData.currentBand].FAMCut), static_cast<float>(SR[SampleRate].rate / DF));
}
InitFilterMask();

  for (int i = 0; i < 5; i++) {
    biquad_lowpass1_coeffs[i] = coefficient_set[i];
  }

  // And adjust decimation and interpolation filters
  SetDecIntFilters();
} // end filter_bandwidth


/*****
  Purpose: InitFilterMask()

  Parameter list:
    void

  Return value;
    void
*****/
void InitFilterMask()
{
  /****************************************************************************************
     Calculate the FFT of the FIR filter coefficients once to produce the FIR filter mask
  ****************************************************************************************/
  // the FIR has exactly m_NumTaps and a maximum of (FFT_length / 2) + 1 taps = coefficients, so we have to add (FFT_length / 2) -1 zeros before the FFT
  // in order to produce a FFT_length point input buffer for the FFT
  // copy coefficients into real values of first part of buffer, rest is zero

  for (unsigned i = 0; i < m_NumTaps; i++) {
    // try out a window function to eliminate ringing of the filter at the stop frequency
    //             sd.FFT_Samples[i] = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
    FIR_filter_mask[i * 2] = FIR_Coef_I [i];
    FIR_filter_mask[i * 2 + 1] = FIR_Coef_Q [i];
  }

  for (unsigned i = FFT_length + 1; i < FFT_length * 2; i++) {
    FIR_filter_mask[i] = 0.0;
  }

  // FFT of FIR_filter_mask
  // perform FFT (in-place), needs only to be done once (or every time the filter coeffs change)
  arm_cfft_f32(maskS, FIR_filter_mask, 0, 1);

} // end init_filter_mask


/*****
  Purpose: void SetDecIntFilters()
  Parameter list:
    void
  Return value;
    void
*****/
void SetDecIntFilters()
{
  /****************************************************************************************
     Recalculate decimation and interpolation FIR filters
  ****************************************************************************************/
  int LP_F_help;
  int filter_BW_highest = bands.bands[ConfigData.currentBand].FHiCut;

  if (filter_BW_highest < - bands.bands[ConfigData.currentBand].FLoCut) {
    filter_BW_highest = - bands.bands[ConfigData.currentBand].FLoCut;
  }
 LP_F_help = filter_BW_highest;

if (LP_F_help > 10000) {
    LP_F_help = 10000;
  }

//  LP_F_help = 10000;  // 

  CalcFIRCoeffs(FIR_dec1_coeffs, n_dec1_taps, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SampleRate].rate));
  CalcFIRCoeffs(FIR_dec2_coeffs, n_dec2_taps, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SampleRate].rate / DF1));

  CalcFIRCoeffs(FIR_int1_coeffs, 48, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)(SR[SampleRate].rate / DF1));
  CalcFIRCoeffs(FIR_int2_coeffs, 32, (float32_t)(LP_F_help), n_att, 0, 0.0, (float32_t)SR[SampleRate].rate);
  bin_BW = 1.0 / (DF * FFT_length) * (float32_t)SR[SampleRate].rate;
}
