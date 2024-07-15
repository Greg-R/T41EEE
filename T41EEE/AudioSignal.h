// Teensy and Open Audio Signal Chains include file.

// Common to Transmitter and Receiver.  Use Open Audio version to simplify transmitter?
AudioInputI2SQuad i2s_quadIn;
AudioOutputI2SQuad i2s_quadOut;

// Transmitter
AudioControlSGTL5000_Extended sgtl5000_1;      // Controller for the Teensy Audio Board, transmitter only.
AudioConvert_I16toF32 int2Float1;              // Converts Int16 to Float.  See class in AudioStream_F32.h
//AudioEffectCompressor_F32 comp1;               // Compressor from OpenAudio library.  Used in microphone dataflow path.
AudioEffectCompressor2_F32  compressor1; // Audio Compressor
AudioEffectCompressor2_F32 *pc1 = &compressor1;
radioCESSB_Z_transmit_F32 cessb1;
AudioConvert_F32toI16 float2Int1, float2Int2;  // Converts Float to Int16.  See class in AudioStream_F32.h
AudioMixer4_F32 mixer1;                        // Used to switch in tone during calibration.
AudioSynthWaveformSine_F32  tone1kHz;          // Tone for SSB calibration.
AudioRecordQueue Q_in_L_Ex;                    // AudioRecordQueue for input Microphone channel.
AudioPlayQueue Q_out_L_Ex;                     // AudioPlayQueue for driving the I channel (CW/SSB) to the QSE.
AudioPlayQueue Q_out_R_Ex;                     // AudioPlayQueue for driving the Q channel (CW/SSB) to the QSE.
/* Original transmitter
AudioConnection patchCord1(i2s_quadIn, 0, int2Float1, 0);    // Microphone channel.
AudioConnection_F32 patchCord3(int2Float1, 0, comp1, 0);     // Microphone to compressor.
AudioConnection_F32 patchCord5(comp1, 0, float2Int1, 0);     // Compressor output.
AudioConnection patchCord7(float2Int1, 0, Q_in_L_Ex, 0);     // Microphone to AudioRecordQueue.
AudioConnection patchCord15(Q_out_L_Ex, 0, i2s_quadOut, 0);  // I channel to line out
AudioConnection patchCord16(Q_out_R_Ex, 0, i2s_quadOut, 1);  // Q channel to line out
*/
const float sample_rate_Hz = 48000.0f;
const int   audio_block_samples = 128;  // Always 128
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);
AudioRecordQueue Q_in_R_Ex;           // This 2nd channel is needed as we are bringing I and Q into the sketch instead of only microphone audio.

//  Begin transmit signal chain.
AudioConnection connect0(i2s_quadIn, 0, int2Float1, 0);    // Microphone audio channel.

// Need a mixer to switch in an audio tone during calibration.  Should be a nominal tone amplitude.
AudioConnection_F32 connect1(int2Float1, 0, mixer1, 0);  // Connect microphone mixer1 output 0
AudioConnection_F32 connect2(tone1kHz, 0, mixer1, 1);  // Connect tone for SSB calibration.

AudioConnection_F32 connect3(mixer1, 0, compressor1, 0);  // Mixer output to input of compressor.
AudioConnection_F32 connect4(compressor1, 0, cessb1, 0);

// Controlled envelope SSB from Open Audio library.
AudioConnection_F32 connect5(cessb1, 0, float2Int1, 0);
AudioConnection_F32 connect6(cessb1, 1, float2Int2, 0);

AudioConnection connect7(float2Int1, 0, Q_in_L_Ex, 0);
AudioConnection connect8(float2Int2, 0, Q_in_R_Ex, 0);

// Transmitter back-end.  This takes streaming data from the sketch and drives it into the I2S.
AudioConnection patchCord15(Q_out_L_Ex, 0, i2s_quadOut, 0);  // I channel to line out
AudioConnection patchCord16(Q_out_R_Ex, 0, i2s_quadOut, 1);  // Q channel to line out

// Receiver
//AudioMixer4 modeSelectInR;    // AFP 09-01-22
//AudioMixer4 modeSelectInL;    // AFP 09-01-22

//AudioMixer4 modeSelectOutL;    // AFP 09-01-22
//AudioMixer4 modeSelectOutR;    // AFP 09-01-22

AudioRecordQueue Q_in_L;
AudioRecordQueue Q_in_R;

AudioPlayQueue Q_out_L;
//AudioPlayQueue Q_out_R;  2nd audio channel not used.  KF5N March 11, 2024

AudioConnection patchCord9(i2s_quadIn, 2, Q_in_L, 0);  //Input Rec
AudioConnection patchCord10(i2s_quadIn, 3, Q_in_R, 0);

//AudioConnection patchCord13(modeSelectInR, 0, Q_in_R, 0);  // Rec in Queue
//AudioConnection patchCord14(modeSelectInL, 0, Q_in_L, 0);

//AudioConnection patchCord17(Q_out_L, 0, i2s_quadOut, 2);  // Rec out Queue
//AudioConnection patchCord18(Q_out_R, 0, i2s_quadOut, 3);  2nd audio channel not used.  KF5N March 11, 2024

//AudioConnection patchCord21(modeSelectOutL, 0, i2s_quadOut, 2);  //Rec out
//AudioConnection patchCord22(modeSelectOutR, 0, i2s_quadOut, 3);
AudioAmplifier volumeAdjust;
AudioConnection patchCord17(Q_out_L, 0, volumeAdjust, 0);
AudioConnection patchCord18(volumeAdjust, 0, i2s_quadOut, 2);

AudioControlSGTL5000 sgtl5000_2;  // This is not a 2nd Audio Adapter.  It is I2S to the PCM1808 (ADC I and Q receiver in) and PCM5102 (DAC audio out).
// End dataflow code


/*****
  Purpose: Manage AudioRecordQueue objects and patchCord connections based on
           the radio's operating mode in a way that minimizes unnecessary
           AudioMemory usage.

  Parameter list:
    int operatingState    radioState/lastState constant indicating desired state

  Return value:
    void

*****/
void SetAudioOperatingState(int operatingState) {
#ifdef DEBUG
  Serial.printf("lastState=%d radioState=%d memory_used=%d memory_used_max=%d f32_memory_used=%d f32_memory_used_max=%d\n",
                lastState,
                radioState,
                (int)AudioStream::memory_used,
                (int)AudioStream::memory_used_max,
                (int)AudioStream_F32::f32_memory_used,
                (int)AudioStream_F32::f32_memory_used_max);
  AudioStream::memory_used_max = 0;
  AudioStream_F32::f32_memory_used_max = 0;
#endif
  switch (operatingState) {
    case SSB_RECEIVE_STATE:
    case AM_RECEIVE_STATE:
    case CW_RECEIVE_STATE:
      // Disconnect and deactivate microphone audio.
      //      patchCord1.disconnect();
      Q_in_L_Ex.end();  // Microphone audio path.
      Q_in_L_Ex.clear();
      // Deactivate TX audio output path.
      patchCord15.disconnect();  // Disconnect transmitter I and Q channel outputs.
      patchCord16.disconnect();
      // QSD connected and enabled
      Q_in_L.begin();                                        // Receiver I channel
      Q_in_R.begin();                                        // Receiver Q channel
      patchCord9.connect();                                  // Receiver I channel
      patchCord10.connect();                                 // Receiver Q channel
      patchCord17.connect();                                 // Receiver audio channel
      volumeAdjust.gain(volumeLog[EEPROMData.audioVolume]);  // Set volume because sidetone may have changed it.
      break;
    case SSB_TRANSMIT_STATE:
      // QSD disabled and disconnected
      patchCord9.disconnect();   // Receiver I channel
      patchCord10.disconnect();  // Receiver Q channel
      patchCord17.disconnect();  // CW sidetone
      Q_in_L.end();
      Q_in_L.clear();
      Q_in_R.end();
      Q_in_R.clear();

      // Microphone input enabled and connected
      //      patchCord1.connect();  // Microphone audio
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      patchCord15.connect();  // Transmitter I channel
      patchCord16.connect();  // Transmitter Q channel

      break;
    case CW_TRANSMIT_STRAIGHT_STATE:
    case CW_TRANSMIT_KEYER_STATE:
      // QSD disabled and disconnected
      patchCord9.disconnect();
      patchCord10.disconnect();
      Q_in_L.end();
      Q_in_L.clear();
      Q_in_R.end();
      Q_in_R.clear();

      // Microphone input disabled and disconnected
      //      patchCord1.disconnect();
      Q_in_L_Ex.end();  // Clear microphone queue.
      Q_in_L_Ex.clear();

      patchCord15.connect();  // Connect I and Q transmitter output channels.
      patchCord16.connect();
      patchCord17.connect();                                    // Sidetone goes into receiver audio path.
      volumeAdjust.gain(volumeLog[EEPROMData.sidetoneVolume]);  // Adjust sidetone volume.

      break;
  }
}