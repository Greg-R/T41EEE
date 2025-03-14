// Teensy and Open Audio Signal Chains include file.


// Common to Transmitter and Receiver.
const float sample_rate_Hz = 48000.0f;
const int audio_block_samples = 128;  // Always 128
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);
AudioInputI2SQuad i2s_quadIn;  // 4 inputs/outputs available only in Teensy audio and not Open Audio library.
AudioOutputI2SQuad i2s_quadOut;

// Transmitter
AudioControlSGTL5000 sgtl5000_1;  // Controller for the Teensy Audio Adapter.
AudioConvert_I16toF32 int2Float1;             // Converts Int16 to Float.  See class in AudioStream_F32.h
AudioEffectGain_F32 micGain(audio_settings);  // Microphone gain control.
AudioFilterEqualizer_F32 txEqualizer(audio_settings);
AudioEffectCompressor2_F32 compressor1;       // Open Audio Compressor
radioCESSB_Z_transmit_F32 cessb1;
AudioConvert_F32toI16 float2Int1, float2Int2, float2Int3, float2Int4;  // Converts Float to Int16.  See class in AudioStream_F32.h
AudioSwitch4_OA_F32 switch1, switch2, switch3;
AudioMixer4_F32 mixer1, mixer2;         // Used to switch in tone during calibration.
AudioSynthWaveformSine_F32 toneSSBCal;  // Tone for SSB calibration.
AudioRecordQueue Q_in_L_Ex;             // AudioRecordQueue for input Microphone channel.
AudioRecordQueue Q_in_R_Ex;             // This 2nd channel is needed as we are bringing I and Q into the sketch instead of only microphone audio.
AudioPlayQueue Q_out_L_Ex;              // AudioPlayQueue for driving the I channel (CW/SSB) to the QSE.
AudioPlayQueue Q_out_R_Ex;              // AudioPlayQueue for driving the Q channel (CW/SSB) to the QSE.

//  Begin transmit signal chain.
AudioConnection connect0(i2s_quadIn, 0, int2Float1, 0);  // Microphone audio channel.  Must use int2Float because Open Audio does not have quad input.

AudioConnection_F32 connect1(int2Float1, 0, switch1, 0);  // Used switches here because it appeared necessary to stop flow of data.
AudioConnection_F32 connect2(toneSSBCal, 0, switch2, 0);  // Tone used during SSB calibration.

// Need a mixer to switch in an audio tone during calibration.  Should be a nominal tone amplitude.
AudioConnection_F32 connect3(switch1, 0, mixer1, 0);  // Connect microphone mixer1 output 0 via gain control.
AudioConnection_F32 connect4(switch2, 0, mixer1, 1);  // Connect tone for SSB calibration.

AudioConnection_F32 connect5(mixer1, 0, micGain, 0);
//AudioConnection_F32 connect6(micGain, 0, switch3, 0);

// Audio Equalizer
AudioConnection_F32 patchCord30(micGain, 0, txEqualizer, 0);
AudioConnection_F32 patchCord31(txEqualizer, 0, switch3, 0);

AudioConnection_F32 connect7(switch3, 0, compressor1, 0);
AudioConnection_F32 connect8(compressor1, 0, mixer2, 0);

AudioConnection_F32 connect9(switch3, 1, mixer2, 1);  // Compressor bypass path.

AudioConnection_F32 connect10(mixer2, 0, cessb1, 0);

// Controlled envelope SSB from Open Audio library.
AudioConnection_F32 connect11(cessb1, 0, float2Int1, 0);
AudioConnection_F32 connect12(cessb1, 1, float2Int2, 0);

AudioConnection connect13(float2Int1, 0, Q_in_L_Ex, 0);  // Stream I and Q into the sketch.
AudioConnection connect14(float2Int2, 0, Q_in_R_Ex, 0);

// Transmitter back-end.  This takes streaming data from the sketch and drives it into the I2S.
AudioConnection connect15(Q_out_L_Ex, 0, i2s_quadOut, 0);  // I channel to line out
AudioConnection connect16(Q_out_R_Ex, 0, i2s_quadOut, 1);  // Q channel to line out

// Receiver data flow
AudioEffectCompressor2_F32 compressor2_1;  // Used for audio AGC.
AudioEffectCompressor2_F32 *pc1 = &compressor2_1;
//AudioSettings_F32 audio_settings2(192000.0, 128);

AudioEffectGain_F32 speakerVolume, speakerScale, headphoneVolume, headphoneScale, compGain;
AudioAmplifier testGain;
AudioMixer4_F32 mixer4;
AudioSwitch4_OA_F32 switch4;
AudioConvert_F32toI16 float2Int5, float2Int6;

AudioRecordQueue ADC_RX_I;  // I channel from ADC PCM1808.
AudioRecordQueue ADC_RX_Q;  // Q channel from ADC PCM1808.

AudioPlayQueue Q_out_L;  // Receiver audio out and CW sidetone.

AudioConvert_I16toF32 int2Float2;
AudioConnection patchCord1(i2s_quadIn, 2, ADC_RX_I, 0);  // Receiver I and Q channel data stream.
AudioConnection patchCord2(i2s_quadIn, 3, ADC_RX_Q, 0);  // This data stream goes to sketch code for processing.

AudioConnection patchCord3(Q_out_L, 0, int2Float2, 0);      // 192ksps Audio data stream from sketch code.  Receiver audio or CW sidetone.



AudioConnection_F32 patchCord4(int2Float2, 0, switch4, 0);  // Used to bypass compressor2_1.

AudioConnection_F32 patchCord5(switch4, 0, compressor2_1, 0);  // Compressor used as audio AGC.
AudioConnection_F32 patchCord6(compressor2_1, 0, mixer4, 0);

AudioConnection_F32 patchCord7(switch4, 1, compGain, 0);  // Bypass compressor2_1.  Equalize compressor gain.
AudioConnection_F32 patchCord8(compGain, 0, mixer4, 1);

// Speaker path
AudioConnection_F32 patchCord9(mixer4, 0, speakerScale, 0);  // speakerScale is used to adjust for different audio amplifier gains.
AudioConnection_F32 patchCord10(speakerScale, 0, speakerVolume, 0);
AudioConnection_F32 patchCord11(speakerVolume, 0, float2Int3, 0);
AudioConnection patchCord12(float2Int3, 0, i2s_quadOut, 2);  //  Speaker audio to PCM5102 via Teensy pin 32.

// Headphone path
AudioConnection_F32 patchCord13(mixer4, 0, headphoneScale, 0);
AudioConnection_F32 patchCord14(headphoneScale, 0, headphoneVolume, 0);
AudioConnection_F32 patchCord15(headphoneVolume, 0, float2Int4, 0);

AudioConnection patchCord25(float2Int4, 0, i2s_quadOut, 0);  // Headphone
AudioConnection patchCord26(float2Int4, 0, i2s_quadOut, 1);  // Headphone

// Some sort of octave band equalizer as a one alternative, 10 bands
//float32_t fBand1[] = {    40.0,  80.0,   160.0,   320.0,  640.0,  1280.0,   2560.0,   5120.0, 10240.0, 22058.5};
//float32_t fBand1[] = {    187.5,  375.0,   750.0,   1500.0,  3000.0,  6000.0,   12000.0,   24000.0, 48000, 96000.0};
float32_t fBand1[] = {    50.0,  70.711,   100.0,   141.421,  200.0,  282.843,   400.0,   565.685, 800.0,   1131.371, 1600.0, 2262.742, 3200.0, 4525.483, 6400.0, 24000.0};
//float32_t dbBand1[] = {10.0,  2.0, -2.0,  -5.0, -2.0,  -4.0,   -10.0,   6.0,    10.0,    -100};
//float32_t dbBand1[] = {-100.0,  -100.0, -100.0,  0.0, -100.0,  -100.0,   -100.0,   -100.0, -100.0,   -100};
float32_t dbBand1[] = {-100.0,  -100.0,    -100.0,   -100.0,  -100.0,  -100.0,   0.0,    0.0,    0.0,      0.0,      0.0,  -100.0,  -100.0, -100.0,  -100.0, -100.0};
float32_t equalizeCoeffs[249];

// End dataflow code

void controlAudioOut(AudioState audioState, bool mute);

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

//  rxEqualizer.equalizerNew(10, &fBand1[0], &dbBand1[0], 30, &equalizeCoeffs[0], 60.0);
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
      InitializeDataArrays();  // I2S sample rate set in this function.
      sgtl5000_1.muteLineout();
      // Deactivate microphone and 1 kHz test tone.
      mixer1.gain(0, 0.0);
      mixer1.gain(1, 0.0);
      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(1);  //  Disconnect 1 kHz test tone path.
      // Stop and clear the data buffers.
      ADC_RX_I.end();    // Receiver I channel
      ADC_RX_Q.end();    // Receiver Q channel
      ADC_RX_I.clear();  // Receiver I channel
      ADC_RX_Q.clear();  // Receiver Q channel
      Q_in_L_Ex.end();   // Transmit I channel path.
      Q_in_R_Ex.end();   // Transmit Q channel path.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.clear();
      // Deactivate TX audio output path.
      connect15.disconnect();
      connect16.disconnect();
      // Connect audio paths
      patchCord1.connect();
      patchCord2.connect();
      patchCord3.connect();
      patchCord25.connect();
      patchCord26.connect();
      // Configure audio compressor (AGC)
      if (ConfigData.AGCMode == true) {  // Activate compressor2_1 path.
        switch4.setChannel(0);
        mixer4.gain(0, 1.0);
        mixer4.gain(1, 0.0);
        initializeAudioPaths();
      } else {
        switch4.setChannel(1);  // Bypass compressor2_1.
        mixer4.gain(0, 0.0);
        mixer4.gain(1, 1.0);
        compGain.setGain_dB(30.0);  // Set the gain equal to the compressor2_1 nominal gain (below threshold).
      }

      // QSD connected and enabled
      ADC_RX_I.begin();  // Receiver I channel
      ADC_RX_Q.begin();  // Receiver Q channel

      speakerVolume.setGain(volumeLog[ConfigData.audioVolume]);    // Set volume because sidetone may have changed it.
      headphoneVolume.setGain(volumeLog[ConfigData.audioVolume]);  // Set volume because sidetone may have changed it.
      sgtl5000_1.volume(0.8);  // Restore headphone volume after CW sidetone reduction.

      controlAudioOut(ConfigData.audioOut, false);  // Configure audio out; don't mute all.




      break;
    case RadioState::SSB_TRANSMIT_STATE:
    case RadioState::FT8_TRANSMIT_STATE:

      SampleRate = SAMPLE_RATE_48K;
//        SetI2SFreq(SR[SampleRate].rate);
      InitializeDataArrays();  // I2S sample rate set in this function.
      // QSD disabled and disconnected
      controlAudioOut(ConfigData.audioOut, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord1.disconnect();  // Receiver I channel
      patchCord2.disconnect();  // Receiver Q channel
      patchCord3.disconnect();  // Receiver audio

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Test tone enabled and connected
      //      toneSSBCal.setSampleRate_Hz(48000);
      //      toneSSBCal.amplitude(0.3);
      //      toneSSBCal.frequency(750.0);
      //      toneSSBCal.begin();
      mixer1.gain(0, 1);      // microphone audio off.
      mixer1.gain(1, 0);      // testTone on.
      switch1.setChannel(0);  // Disconnect microphone path.
      switch2.setChannel(1);  // Connect 1 kHz test tone path.

      if (ConfigData.compressorFlag) {
        switch3.setChannel(0);
        mixer2.gain(0, 1.0);
        mixer2.gain(1, 0.0);
      } else {
        switch3.setChannel(1);  // Bypass compressor.
        mixer2.gain(0, 0.0);
        mixer2.gain(1, 1.0);
      }

      Q_out_L_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      Q_out_R_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
      Q_in_L_Ex.begin();         // I channel Microphone audio
      Q_in_R_Ex.begin();         // Q channel Microphone audio
      patchCord25.disconnect();  // Disconnect headphone.
      patchCord26.disconnect();

      // Update equalizer.  Update first 14 only.  Last two are constant.
      for(int i = 0; i < 14; i = i + 1) dbBand1[i] = ConfigData.equalizerXmt[i];
      
      txEqualizer.equalizerNew(16, &fBand1[0], &dbBand1[0], 249, &equalizeCoeffs[0], 65.0f);
      updateMic();
      connect15.connect();  // Transmitter I channel
      connect16.connect();  // Transmitter Q channel

      break;

    case RadioState::SSB_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();  // I2S sample rate set in this function.
      // QSD disabled and disconnected
      controlAudioOut(ConfigData.audioOut, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord1.connect();  // Receiver I channel
      patchCord2.connect();  // Receiver Q channel

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Test tone enabled and connected
      toneSSBCal.setSampleRate_Hz(48000);
      toneSSBCal.amplitude(0.3);
      toneSSBCal.frequency(750.0);
      toneSSBCal.begin();
      mixer1.gain(0, 0);      // microphone audio off.
      mixer1.gain(1, 1);      // testTone on.
      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(0);  // Connect 1 kHz test tone path.

      if (ConfigData.compressorFlag) {
        switch3.setChannel(0);
        mixer2.gain(0, 1.0);
        mixer2.gain(1, 0.0);
      } else {
        switch3.setChannel(1);  // Bypass compressor.
        mixer2.gain(0, 0.0);
        mixer2.gain(1, 1.0);
      }

      Q_out_L_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);  // Need this as CW will put into wrong mode.  Greg KF5N August 4, 2024.
      Q_out_R_Ex.setBehaviour(AudioPlayQueue::ORIGINAL);
      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio
      ADC_RX_I.begin();   // Calibration is full duplex!  Activate receiver data.  No demodulation during calibrate, spectrum only.
      ADC_RX_Q.begin();
      patchCord25.disconnect();
      patchCord26.disconnect();
      connect15.connect();  // Transmitter I channel
      connect16.connect();  // Transmitter Q channel

      break;

    case RadioState::CW_TRANSMIT_STRAIGHT_STATE:
    case RadioState::CW_TRANSMIT_KEYER_STATE:
      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Baseband data cleared and ended.
      Q_in_L_Ex.end();  // Clear I channel.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Clear Q channel.
      Q_in_R_Ex.clear();
      // QSD disabled and disconnected
      patchCord1.disconnect();
      patchCord2.disconnect();
      patchCord3.connect();  // Sidetone
      // Speaker and headphones should be unmuted according to current audio out state for sidetone.
      controlAudioOut(ConfigData.audioOut, false);  // FT8 audio should be headphone only.
      sgtl5000_1.unmuteLineout();

      sgtl5000_1.volume(0.3);  // This is for scaling the sidetone in the headphone output.

      patchCord25.disconnect();  // Disconnect headphone, which is shared with I and Q transmit.
      patchCord26.disconnect();
      connect15.connect();  // Connect I and Q transmitter output channels.
      connect16.connect();

      speakerVolume.setGain(volumeLog[ConfigData.sidetoneVolume]);    // Adjust sidetone volume.
      headphoneVolume.setGain(volumeLog[ConfigData.sidetoneVolume]);  // Adjust sidetone volume.

      break;

    case RadioState::CW_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_192K;
      SetI2SFreq(SR[SampleRate].rate);
      // QSD receiver enabled.  Calibrate is full duplex.
      controlAudioOut(ConfigData.audioOut, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();

      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(1);  //  Disconnect 1 kHz test tone path.

      // CW does not use the transmitter front-end.
      Q_in_L_Ex.end();  // Transmit I channel path.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Transmit Q channel path.
      Q_in_R_Ex.clear();

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      toneSSBCal.end();
      mixer1.gain(0, 0);  // microphone audio off.
      mixer1.gain(1, 0);  // testTone off.
      patchCord1.connect();
      patchCord2.connect();
      ADC_RX_I.begin();  // Calibration is full duplex!
      ADC_RX_Q.begin();
      patchCord25.disconnect();  // Disconnect headphone, which is shared with I and Q transmit.
      patchCord26.disconnect();
      //  Transmitter back-end needs to be active during CW calibration.
      connect15.connect();  // Transmitter I channel
      connect16.connect();  // Transmitter Q channel


      //      patchCord3.connect();  // Sidetone
      break;

    case RadioState::SET_CW_SIDETONE:
      SampleRate = SAMPLE_RATE_192K;
      SetI2SFreq(SR[SampleRate].rate);
      //// Speaker and headphones should be unmuted according to current audio out state!!!!!!!!
      controlAudioOut(ConfigData.audioOut, false);  // Unmute according to audio state for sidetone.
                                                    ////      patchCord9.disconnect();
                                                    ////      patchCord10.disconnect();
      // Baseband CESSB data cleared and ended.

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      Q_in_L_Ex.end();  // Clear I channel.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Clear Q channel.
      Q_in_R_Ex.clear();

      //      patchCord17.connect();                                    // Sidetone goes into receiver audio path.

      break;

    case RadioState::NOSTATE:
      break;

    default:
      break;
  }
}


/*****
  Purpose: Manage headphone and speaker mute/unmute.

  Parameter list:
    void AudioState   Desired audio state.
    bool mute         Mute all audio for SSB and FT8 transmit.
                      CW requires audio due to sidetone.
  Return value:
    void

*****/
void controlAudioOut(AudioState audioState, bool mute) {
  if (mute) {
    sgtl5000_1.muteHeadphone();     // Unmute headphones.
    digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
    return;
  }
  switch (audioState) {
    case AudioState::HEADPHONE:
      sgtl5000_1.unmuteHeadphone();   // Unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
      break;

    case AudioState::SPEAKER:
      sgtl5000_1.muteHeadphone();       // Unmute headphones.
      digitalWrite(MUTE, UNMUTEAUDIO);  // Mute speaker audio amplifier.
      break;

    case AudioState::BOTH:
      sgtl5000_1.unmuteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, UNMUTEAUDIO);  // Mute speaker audio amplifier.
      break;

    case AudioState::MUTE_BOTH:
      sgtl5000_1.muteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);  // Mute speaker audio amplifier.
      break;

    default:
      break;
  }
}
