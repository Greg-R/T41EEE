// Teensy and Open Audio Signal Chains include file.

// Common to Transmitter and Receiver.
const float sample_rate_Hz = 48000.0;  // The transmitter operates at 48ksps.
const int audio_block_samples = 128;   // Always 128
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);
AudioInputI2SQuad i2s_quadIn;  // 4 inputs available only in Teensy audio and not Open Audio library.
AudioOutputI2SQuad_F32 i2s_quadOut_f32(audio_settings);

// Transmitter
AudioControlSGTL5000 sgtl5000_1;                                                  // Controller for the Teensy Audio Adapter.
AudioConvert_I16toF32 int2Float1_tx;                                              // Converts Int16 to Float.
AudioEffectGain_F32 micGain(audio_settings), compGainCompensate(audio_settings);  // Microphone gain control.
AudioFilterEqualizer_F32 txEqualizer(audio_settings);
AudioEffectCompressor2_F32 compressor1;  // Open Audio Compressor
radioCESSB_Z_transmit_F32 cessb1;

AudioConvert_F32toI16 float2Int1_tx, float2Int2_tx;  // Converts Float to Int16.  See class in AudioStream_F32.h

////AudioSwitch4_OA_F32 switch1_tx, switch3_tx, switch4_tx;
AudioSwitch4_OA_F32 switch3_tx, switch4_tx;
AudioMixer4_F32 mixer1_tx, mixer2_tx, mixer3_tx;      // Used to switch in tone during calibration.
AudioSynthWaveformSine_F32 toneSSBCal1, toneSSBCal2;  // Tones for SSB calibration and IMD testing.

AudioRecordQueue_F32 Q_in_L_Ex;                       // I channel from SSB exciter.
AudioRecordQueue_F32 Q_in_R_Ex;                       // Q channel from SSB exciter.

AudioPlayQueue_F32 Q_out_L_Ex;                        // AudioPlayQueue for driving the I channel (CW/SSB) to the QSE.
AudioPlayQueue_F32 Q_out_R_Ex;                        // AudioPlayQueue for driving the Q channel (CW/SSB) to the QSE.
AudioPlayQueue_F32 cwToneData;                        // The tone from the CW Exciter.

//  Begin transmit signal chain.
AudioConnection connect0(i2s_quadIn, 0, int2Float1_tx, 0);  // Microphone audio channel.  Must use int2Float because Open Audio does not have quad input.

//AudioConnection_F32 connect1(int2Float1_tx, 0, switch1_tx, 0);  // Used switches here because it appeared necessary to stop flow of data.

// Need a mixer to switch between microphone audio and tones used for calibration, testing, and CW.
//AudioConnection_F32 connect3(switch1_tx, 0, mixer1_tx, 0);  // Connect microphone mixer1 output 0 via gain control.
AudioConnection_F32 connect3(int2Float1_tx, 0, mixer1_tx, 0);  // Connect microphone mixer1 output 0 via gain control.

AudioConnection_F32 connect4(toneSSBCal1, 0, mixer1_tx, 1);   // Connect tone for SSB calibration and IM3 testing.
AudioConnection_F32 connect22(toneSSBCal2, 0, mixer1_tx, 2);  // Connect tone for IM3 testing.
AudioConnection_F32 connect24(cwToneData, 0, mixer1_tx, 3);   // Connect CW transmit tone.

AudioConnection_F32 connect5(mixer1_tx, 0, micGain, 0);

// Half-octave Audio Equalizer
AudioConnection_F32 connect6(micGain, 0, switch3_tx, 0);
AudioConnection_F32 connect7(switch3_tx, 0, txEqualizer, 0);
AudioConnection_F32 connect8(txEqualizer, 0, mixer2_tx, 0);
// Bypass equalizer
AudioConnection_F32 connect9(switch3_tx, 1, mixer2_tx, 1);

AudioConnection_F32 connect10(mixer2_tx, 0, switch4_tx, 0);

AudioConnection_F32 connect11(switch4_tx, 0, compressor1, 0);
AudioConnection_F32 connect12(compressor1, 0, mixer3_tx, 0);

// Compressor bypass path.
AudioConnection_F32 connect13(switch4_tx, 1, compGainCompensate, 0);  // Compressor bypass path.
AudioConnection_F32 connect23(compGainCompensate, 0, mixer3_tx, 1);

AudioConnection_F32 connect14(mixer3_tx, 0, cessb1, 0);

// Controlled envelope SSB from Open Audio library.

//AudioConnection_F32 connect15(cessb1, 0, float2Int1_tx, 0);
//AudioConnection_F32 connect16(cessb1, 1, float2Int2_tx, 0);

//AudioConnection connect17(float2Int1_tx, 0, Q_in_L_Ex, 0);  // Stream I and Q into the sketch.
//AudioConnection connect18(float2Int2_tx, 0, Q_in_R_Ex, 0);  // Tones for SSB calibration and IMD testing.

AudioConnection_F32 connect15(cessb1, 0, Q_in_L_Ex, 0);
AudioConnection_F32 connect16(cessb1, 1, Q_in_R_Ex, 0);

// End transmitter signal chain.
// Transmitter back-end is merged with the receiver headphone outputs below.

// Begin receiver signal chain.
AudioEffectCompressor2_F32 compressor2_1;  // Used for audio AGC.
AudioEffectCompressor2_F32 *pc1 = &compressor2_1;

AudioEffectGain_F32 speakerVolume, speakerScale, headphoneVolume, headphoneScale, compGain, buffer;
AudioAmplifier testGain;
AudioMixer4_F32 mixer4;
AudioSwitch4_OA_F32 switch1_rx, switch2_rx;
AudioConvert_F32toI16 float2Int3, float2Int4, float2Int5, float2Int6;

AudioRecordQueue ADC_RX_I;  // Receiver I channel from ADC PCM1808, 16 bit.
AudioRecordQueue ADC_RX_Q;  // Receiver Q channel from ADC PCM1808, 16 bit.

AudioPlayQueue Q_out_L;  // Receiver audio out and CW sidetone.

AudioConvert_I16toF32 int2Float2;
AudioConnection patchCord1(i2s_quadIn, 3, ADC_RX_I, 0);  // Receiver I and Q channel data stream.
AudioConnection patchCord2(i2s_quadIn, 2, ADC_RX_Q, 0);  // This data stream goes to sketch code for processing.

AudioConnection patchCord3(Q_out_L, 0, int2Float2, 0);  // 192ksps Audio data stream from sketch code.  Receiver audio or CW sidetone.

// F32 from here forward.
AudioConnection_F32 patchCord4(int2Float2, 0, switch1_rx, 0);  // Used to bypass compressor2_1.

AudioConnection_F32 patchCord5(switch1_rx, 0, compressor2_1, 0);  // Compressor used as audio AGC.
AudioConnection_F32 patchCord6(compressor2_1, 0, mixer4, 0);

AudioConnection_F32 patchCord7(switch1_rx, 1, compGain, 0);  // Bypass compressor2_1.  Equalize compressor gain.
AudioConnection_F32 patchCord8(compGain, 0, mixer4, 1);

// Switch in front of headphone and speaker audio paths so that inactive path can be switched off.
AudioConnection_F32 patchCord9(mixer4, 0, switch2_rx, 0);

// Speaker path
AudioConnection_F32 patchCord10(switch2_rx, 0, speakerScale, 0);  // speakerScale is used to adjust for different audio amplifier gains.
AudioConnection_F32 patchCord11(speakerScale, 0, speakerVolume, 0);

AudioConnection_F32 patchCord12(speakerVolume, 0, i2s_quadOut_f32, 2);  //  Speaker audio to PCM5102 via Teensy pin 32.

// Headphone path
AudioConnection_F32 patchCord13(switch2_rx, 1, headphoneScale, 0);  // headphoneScale is user centering of headphone volume.
AudioConnection_F32 patchCord14(headphoneScale, 0, headphoneVolume, 0);

// Now connect both headphone and speaker paths to the same output (2) of the switch.
AudioConnection_F32 patchCord27(switch2_rx, 2, speakerScale, 0);    // Headphone right
AudioConnection_F32 patchCord28(switch2_rx, 2, headphoneScale, 0);  // Headphone right

// The headphone and transmitter back-end share inputs 0 and 1 of the quad I2s output object.
// Because 2 outputs can't be connected to the same input, two more mixers are required.

AudioMixer4_F32 mixer_rxtx_I, mixer_rxtx_Q;

// Transmitter back-end to Audio Adapter.
AudioConnection_F32 patchCord29(Q_out_L_Ex, 0, mixer_rxtx_I, 0);
AudioConnection_F32 patchCord30(Q_out_R_Ex, 0, mixer_rxtx_Q, 0);

// Headphone to Audio Adapter.
AudioConnection_F32 patchCord31(headphoneVolume, 0, mixer_rxtx_I, 1);
AudioConnection_F32 patchCord32(headphoneVolume, 0, mixer_rxtx_Q, 1);

// I and Q channels from outputs of mixers to the quad I2S output object's inputs.
AudioConnection_F32 patchCord33(mixer_rxtx_I, 0, i2s_quadOut_f32, 0);
AudioConnection_F32 patchCord34(mixer_rxtx_Q, 0, i2s_quadOut_f32, 1);

// Half-octave transmit band equalizer, 16 bands, but only the lower 14 are used.
float32_t fBand1[] = { 50.0, 70.711, 100.0, 141.421, 200.0, 282.843, 400.0, 565.685, 800.0, 1131.371, 1600.0, 2262.742, 3200.0, 4525.483, 6400.0, 24000.0 };
// These are default values useful for SSB voice.  Bypass equalizer during FT8.
float32_t dbBand1[] = { -100.0, -100.0, -100.0, -100.0, -100.0, -100.0, 0.0, 0.0, 0.0, 0.0, 0.0, -100.0, -100.0, -100.0, -100.0, -100.0 };
float32_t equalizeCoeffs[249];

// End receiver and transmitter signal chain code.

// This function implemented below.
void controlAudioOut(AudioState audioState, bool mute);  // Declaration, implemenation below.

// Configure basic compressor macro.  This is used in the audio path as a form of AGC.
void initializeAudioPaths() {
  // pc1 is global pointer to compressor2_1 object for receiver AGC
  int16_t delaySize = 256;  // Any power of 2, i.e., 256, 128, 64, etc.
  compressor2_1.setDelayBufferSize(delaySize);
  compressor2_1.setAttackReleaseSec(0.005f, 2.0f);
  // Limiter at -1 dB out for highest 60 dB, limiter macro <<<<  MAKE VARIABLE <<<<<<<<<<<<<<<<
  // basicCompressorBegin(pobject, linearInDB, compressionRatio);
  // The compression curve for basicCompressorBegin() has a 3 segments. It is linear up
  // to an input linearInDB and then decreases gain according to compressionRatioDB up to
  // an input -10 dB where it is almost limited, with an increase of output level of 1 dB
  // for a 10 dB increase in input level. The output level at full input is 1 dB below
  // full output.
  basicCompressorBegin(pc1, ConfigData.AGCThreshold, 10.0f);
}


/*****
  Purpose: Manage AudioRecordQueue objects and patchCord connections based on
           the radio's operating mode in a way that minimizes unnecessary
           AudioMemory usage.

  Parameter list:
    int operatingState    radioState/lastState constant indicating desired state

  Return value:
    void

*****/
void SetAudioOperatingState(RadioState operatingState) {
  Serial.printf("AudioOperatingState\n");
  AudioNoInterrupts();
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
    case RadioState::SSB_RECEIVE_STATE:
    case RadioState::FT8_RECEIVE_STATE:
    case RadioState::AM_RECEIVE_STATE:
    case RadioState::SAM_RECEIVE_STATE:
    case RadioState::CW_RECEIVE_STATE:
      SampleRate = SAMPLE_RATE_192K;
      InitializeDataArrays();    // I2S sample rate set in this function.
      sgtl5000_1.muteLineout();  // Shut off I and Q baseband to QSE.
      // Stop and clear the data buffers.
      ADC_RX_I.end();    // Receiver I channel
      ADC_RX_Q.end();    // Receiver Q channel
      ADC_RX_I.clear();
      ADC_RX_Q.clear();

      Q_in_L_Ex.end();   // Transmit I channel path.
      Q_in_R_Ex.end();   // Transmit Q channel path.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.clear();

      // Deactivate microphone and tones.
      mixer1_tx.gain(0, 0.0);
      mixer1_tx.gain(1, 0.0);
      mixer1_tx.gain(2, 0);
      mixer1_tx.gain(3, 0);       // CW tone path.
      mixer_rxtx_I.gain(0, 0.0);  // Disconnect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 0.0);
      mixer_rxtx_I.gain(1, 1.0);  // Connect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 1.0);
      toneSSBCal1.end();
      toneSSBCal2.end();

      switch4_tx.setChannel(1);  // Bypass compressor.
      mixer3_tx.gain(0, 0.0);
      mixer3_tx.gain(1, 0.0);

      // Connect audio paths
      patchCord1.connect();
      patchCord2.connect();
      patchCord3.connect();
      connect0.disconnect();  // Disconnect microphone input data stream.
      Q_out_L.setBehaviour(AudioPlayQueue::NON_STALLING);

      // Configure audio compressor (AGC)
      if (ConfigData.AGCMode == true) {  // Activate compressor2_1 path.
        switch1_rx.setChannel(0);
        mixer4.gain(0, 1.0);
        mixer4.gain(1, 0.0);
        initializeAudioPaths();
      } else {
        switch1_rx.setChannel(1);  // Bypass compressor2_1.
        mixer4.gain(0, 0.0);
        mixer4.gain(1, 1.0);
        compGain.setGain_dB(30.0);  // Set the gain equal to the compressor2_1 nominal gain (below threshold).
      }

      // QSD connected and enabled
      ADC_RX_I.begin();  // Receiver I channel
      ADC_RX_Q.begin();  // Receiver Q channel

      speakerVolume.setGain(volumeLog[ConfigData.speakerVolume]);      // Set volume because sidetone may have changed it.
      headphoneVolume.setGain(volumeLog[ConfigData.headphoneVolume]);  // Set volume because sidetone may have changed it.
      sgtl5000_1.volume(0.8);                                          // Restore headphone volume after CW sidetone reduction.

      controlAudioOut(ConfigData.audioOut, false);  // Configure audio out; don't mute all.

      break;
    case RadioState::SSB_TRANSMIT_STATE:
    case RadioState::FT8_TRANSMIT_STATE:

// Don't use compressor or CESSB in FT8 mode.
if(bands.bands[ConfigData.currentBand].mode == RadioMode::FT8_MODE) { 
  ConfigData.compressorFlag = false;
      ConfigData.cessb = false;
      cessb1.setProcessing(ConfigData.cessb);
}
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();  // I2S sample rate set in this function.
      // QSD disabled and disconnected
      controlAudioOut(ConfigData.audioOut, true);  // Mute all receiver audio.
      sgtl5000_1.unmuteLineout();
      patchCord1.disconnect();  // Receiver I channel
      patchCord2.disconnect();  // Receiver Q channel
      patchCord3.disconnect();  // Receiver audio
      connect0.connect();       // Connect microphone input data stream.

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      mixer1_tx.gain(0, 1);       // microphone audio on.
      mixer1_tx.gain(1, 0);       // testTone off.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0.0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0.0);

      if (ConfigData.compressorFlag) {
        switch4_tx.setChannel(0);
        mixer3_tx.gain(0, 1.0);
        mixer3_tx.gain(1, 0.0);
      } else {
        switch4_tx.setChannel(1);  // Bypass compressor.
        mixer3_tx.gain(0, 0.0);
        mixer3_tx.gain(1, 1.0);
        compGainCompensate.setGain_dB(10.0);  // Use compressor's below threshold gain.
      }

      if (ConfigData.xmitEQFlag and bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
        switch3_tx.setChannel(0);
        mixer2_tx.gain(0, 1.0);
        mixer2_tx.gain(1, 0.0);
      } else {
        switch3_tx.setChannel(1);  // Bypass equalizer.  Must bypass for FT8.
        mixer2_tx.gain(0, 0.0);
        mixer2_tx.gain(1, 1.0);
      }
      ////      Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      ////      Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      // Update equalizer.  Update first 14 only.  Last two are constant.
      for (int i = 0; i < 14; i = i + 1) dbBand1[i] = static_cast<float32_t>(ConfigData.equalizerXmt[i]);

      txEqualizer.equalizerNew(16, &fBand1[0], &dbBand1[0], 249, &equalizeCoeffs[0], 65.0f);
      updateMic();

      break;

    case RadioState::SSB_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();                        // I2S sample rate set in this function.
      controlAudioOut(AudioState::MUTE_BOTH, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord1.connect();  // Receiver I channel
      patchCord2.connect();  // Receiver Q channel
      patchCord3.disconnect();  // Receiver audio

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      cessb1.setSampleRate_Hz(48000);  ////

      // Test tone enabled and connected
      toneSSBCal1.setSampleRate_Hz(48000);
      //      toneSSBCal1.pureSpectrum(true);
      toneSSBCal1.amplitude(0.12);  // Set to same amplitude as CW tone.
      toneSSBCal1.frequency(750.0);
      toneSSBCal1.begin();
      toneSSBCal2.end();
      mixer1_tx.gain(0, 0);       // microphone audio off.
      mixer1_tx.gain(1, 1);       // testTone on.
      mixer1_tx.gain(2, 0);       // testTone 2 off.

      connect0.disconnect();      // Disconnect microphone input data stream.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0);

      if (ConfigData.compressorFlag) {
        switch4_tx.setChannel(0);
        mixer3_tx.gain(0, 1.0);
        mixer3_tx.gain(1, 0.0);
      } else {
        switch4_tx.setChannel(1);  // Bypass compressor.
        mixer3_tx.gain(0, 0.0);
        mixer3_tx.gain(1, 1.0);
        compGainCompensate.setGain_dB(10.0);  // Use compressor's below threshold gain.
      }

      if (ConfigData.xmitEQFlag and bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
        switch3_tx.setChannel(0);
        mixer2_tx.gain(0, 1.0);
        mixer2_tx.gain(1, 0.0);
      } else {
        switch3_tx.setChannel(1);  // Bypass equalizer.  Must bypass for FT8.
        mixer2_tx.gain(0, 0.0);
        mixer2_tx.gain(1, 1.0);
      }

      Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio
      ADC_RX_I.begin();   // Calibration is full duplex!  Activate receiver data.  No demodulation during calibrate, spectrum only.
      ADC_RX_Q.begin();

      // Update equalizer.  Update first 14 only.  Last two are constant.
      for (int i = 0; i < 14; i = i + 1) dbBand1[i] = ConfigData.equalizerXmt[i];

      txEqualizer.equalizerNew(16, &fBand1[0], &dbBand1[0], 249, &equalizeCoeffs[0], 65.0f);
      updateMic();

      break;

    case RadioState::SSB_IM3TEST_STATE:
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();  // I2S sample rate set in this function.
      // QSD disabled and disconnected
      controlAudioOut(AudioState::MUTE_BOTH, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord1.disconnect();  // Receiver I channel
      patchCord2.disconnect();  // Receiver Q channel
      patchCord3.disconnect();  // Receiver audio
      connect0.disconnect();       // Disconnect microphone input data stream.

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Test tone enabled and connected
      toneSSBCal1.setSampleRate_Hz(48000);
      toneSSBCal1.amplitude(0.1);
      toneSSBCal1.frequency(700.0);
      toneSSBCal1.begin();
      toneSSBCal2.setSampleRate_Hz(48000);
      toneSSBCal2.amplitude(0.1);
      toneSSBCal2.frequency(1900.0);
      toneSSBCal2.begin();
      mixer1_tx.gain(0, 0);       // microphone audio off.
      mixer1_tx.gain(1, 1);       // testTone 1 on.
      mixer1_tx.gain(2, 1);       // testTone 2 on.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0.0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0.0);

      if (ConfigData.compressorFlag) {
        switch4_tx.setChannel(0);
        mixer3_tx.gain(0, 1.0);
        mixer3_tx.gain(1, 0.0);
      } else {
        switch4_tx.setChannel(1);  // Bypass compressor.
        mixer3_tx.gain(0, 0.0);
        mixer3_tx.gain(1, 1.0);
        compGainCompensate.setGain_dB(10.0);  // Use compressor's below threshold gain.
      }

      if (ConfigData.xmitEQFlag and bands.bands[ConfigData.currentBand].mode == RadioMode::SSB_MODE) {
        switch3_tx.setChannel(0);
        mixer2_tx.gain(0, 1.0);
        mixer2_tx.gain(1, 0.0);
      } else {
        switch3_tx.setChannel(1);  // Bypass equalizer.  Must bypass for FT8.
        mixer2_tx.gain(0, 0.0);
        mixer2_tx.gain(1, 1.0);
      }

      ////      Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      ////      Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      // Update equalizer.  Update first 14 only.  Last two are constant.
      for (int i = 0; i < 14; i = i + 1) dbBand1[i] = ConfigData.equalizerXmt[i];

      txEqualizer.equalizerNew(16, &fBand1[0], &dbBand1[0], 249, &equalizeCoeffs[0], 65.0f);
      updateMic();

      break;

    case RadioState::CW_TRANSMIT_STRAIGHT_STATE:
    case RadioState::CW_TRANSMIT_KEYER_STATE:

      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();  // I2S sample rate set in this function.

      controlAudioOut(ConfigData.audioOut, true);  // Mute all audio.  Unmuted at end of configuration.
      sgtl5000_1.unmuteLineout();
      // QSD disabled and disconnected
      patchCord1.disconnect();  // Receiver I channel
      patchCord2.disconnect();  // Receiver Q channel
      patchCord3.connect();     // Sidetone
      connect0.disconnect();      // Disconnect microphone input data stream.
      // Speaker and headphones should be unmuted according to current audio out state for sidetone.
      controlAudioOut(ConfigData.audioOut, false);

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      toneSSBCal1.end();
      toneSSBCal2.end();
      mixer1_tx.gain(0, 0);       // microphone audio off.
      mixer1_tx.gain(1, 0);       // testTone 1 off.
      mixer1_tx.gain(2, 0);       // testTone 2 off.
      mixer1_tx.gain(3, 1);       // CW tone path.  Mic gain is also in this path.  Set micGain to 0dB.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0.0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0.0);

      // Bypass equalizer and compressor in CW mode!
      switch3_tx.setChannel(1);  // Bypass equalizer.  Must bypass for FT8.
      mixer2_tx.gain(0, 0.0);
      mixer2_tx.gain(1, 1.0);

      switch4_tx.setChannel(1);  // Bypass compressor.
      mixer3_tx.gain(0, 0.0);
      mixer3_tx.gain(1, 1.0);
      compGainCompensate.setGain_dB(10.0);  // Use compressor's below threshold gain.

        // Set the sideband.
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) cessb1.setSideband(false);
  if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) cessb1.setSideband(true);

      // Set CW calibration factors.
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand], 0.0);
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
      }

      Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      // Speaker and headphones should be unmuted according to current audio out state for sidetone.
      controlAudioOut(ConfigData.audioOut, false);
      sgtl5000_1.unmuteLineout();

      sgtl5000_1.volume(static_cast<float32_t>(ConfigData.sidetoneHeadphone) / 100.0);  // This is for scaling the sidetone in the headphone output.  This is                                               // Headphone sidetone is same as transmit I and Q!
      speakerVolume.setGain(volumeLog[ConfigData.sidetoneSpeaker]);                     // Adjust sidetone volume.  Headphone sidetone done in hardware.

      break;

    case RadioState::CW_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();                        // I2S sample rate set in this function.
      controlAudioOut(AudioState::MUTE_BOTH, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      connect0.disconnect();      // Disconnect microphone input data stream.

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Test tone enabled and connected
      toneSSBCal1.setSampleRate_Hz(48000);
      toneSSBCal1.amplitude(0.12);
      toneSSBCal1.frequency(750.0);
      toneSSBCal1.begin();
      toneSSBCal2.end();
      mixer1_tx.gain(0, 0);       // microphone audio off.
      mixer1_tx.gain(1, 1);       // testTone on.
      mixer1_tx.gain(2, 0);       // testTone 2 off.
                                  ////      switch1_tx.setChannel(1);   // Disconnect microphone path.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0.0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0.0);

      // Bypass equalizer and compressor in CW mode!
      switch3_tx.setChannel(1);  // Bypass equalizer.
      mixer2_tx.gain(0, 0.0);
      mixer2_tx.gain(1, 1.0);

      switch4_tx.setChannel(1);  // Bypass compressor.
      mixer3_tx.gain(0, 0.0);
      mixer3_tx.gain(1, 1.0);
      compGainCompensate.setGain_dB(10.0);  // Use compressor's below threshold gain when bypassed.

      // Set CW calibration factors.
      if (bands.bands[ConfigData.currentBand].sideband == Sideband::LOWER) {
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorLSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorLSB[ConfigData.currentBand], 0.0);
      } else if (bands.bands[ConfigData.currentBand].sideband == Sideband::UPPER) {
        cessb1.setIQCorrections(true, CalData.IQCWAmpCorrectionFactorUSB[ConfigData.currentBand], CalData.IQCWPhaseCorrectionFactorUSB[ConfigData.currentBand], 0.0);
      }

      patchCord1.connect();  // Connect I and Q receiver data converters.
      patchCord2.connect();
      ADC_RX_I.begin();  // Calibration is full duplex!
      ADC_RX_Q.begin();

      Q_out_L_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      Q_out_R_Ex.setBehaviour(AudioPlayQueue_F32::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel to sketch
      Q_in_R_Ex.begin();  // Q channel to sketch

      break;

    case RadioState::RECEIVE_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_192K;
      SetI2SFreq(SR[SampleRate].rate);
      // QSD receiver enabled.  Calibrate is full duplex.
      controlAudioOut(AudioState::MUTE_BOTH, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();

      // CW does not use the transmitter front-end.
      Q_in_L_Ex.end();  // Transmit I channel path.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Transmit Q channel path.
      Q_in_R_Ex.clear();

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      toneSSBCal1.end();
      toneSSBCal2.end();
      mixer1_tx.gain(0, 0);       // microphone audio off.
      mixer1_tx.gain(1, 0);       // testTone off.
      mixer_rxtx_I.gain(0, 1.0);  // Connect transmitter back-end to Audio Adapter.
      mixer_rxtx_Q.gain(0, 1.0);
      mixer_rxtx_I.gain(1, 0.0);  // Disconnect headphone path to Audio Adapter.
      mixer_rxtx_Q.gain(1, 0.0);
      patchCord1.connect();
      patchCord2.connect();
      ADC_RX_I.begin();  // Calibration is full duplex!
      ADC_RX_Q.begin();

      break;

    case RadioState::NOSTATE:
      break;

    default:
      break;
  }
  AudioInterrupts();
}


/*****
  Purpose: Manage headphone and speaker mute/unmute.
           This function is extern as it is used by
           the audio output button from the Button class.

  Parameter list:
    void AudioState   Desired audio state.
    bool mute         Mute all audio for SSB and FT8 transmit.
                      CW requires audio due to sidetone.
  Return value:
    void

*****/
void controlAudioOut(AudioState audioState, bool mute) {

  switch (audioState) {
    case AudioState::HEADPHONE:
      digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
      switch2_rx.setChannel(1);       // Switch to headphone path.
      sgtl5000_1.unmuteHeadphone();   // Unmute headphones.
      display.UpdateVolumeField();
      break;

    case AudioState::SPEAKER:
      sgtl5000_1.muteHeadphone();       // Mute headphones.
      switch2_rx.setChannel(0);         // Switch to speaker path.
      digitalWrite(MUTE, UNMUTEAUDIO);  // Unmute speaker audio amplifier.
      display.UpdateVolumeField();
      break;

    case AudioState::BOTH:
      switch2_rx.setChannel(2);         // Drive both audio paths.
      sgtl5000_1.unmuteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, UNMUTEAUDIO);  // Unmute speaker audio amplifier.
      display.UpdateVolumeField();      // Volume control for speaker only if BOTH.
      break;

    case AudioState::MUTE_BOTH:
      switch2_rx.setChannel(3);       // Switch to unused path.
      sgtl5000_1.muteHeadphone();     // Mute headphones.
      digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
      break;

    default:
      break;
  }
  // Physically mute audio, but leave audio paths set above intact.
  if (mute) {
    sgtl5000_1.muteHeadphone();     // Mute headphones.
    digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
    return;
  }
}
