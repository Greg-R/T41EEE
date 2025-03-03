// Teensy and Open Audio Signal Chains include file.

//#include "USB_Audio_F32.h"  // Required to use F32 USB in/out.

// Common to Transmitter and Receiver.
const float sample_rate_Hz = 48000.0f;
const int   audio_block_samples = 128;  // Always 128
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);
AudioInputI2SQuad i2s_quadIn;     // 4 inputs/outputs available only in Teensy audio not Open Audio library.
AudioOutputI2SQuad i2s_quadOut;

// Transmitter
AudioControlSGTL5000 sgtl5000_1;      // Controller for the Teensy Audio Adapter.
AudioConvert_I16toF32 int2Float1;              // Converts Int16 to Float.  See class in AudioStream_F32.h
//AudioEffectGain_F32 micGain(audio_settings);                   // Microphone gain control.
AudioEffectGain_F32 micGain;                   // Microphone gain control.
AudioEffectCompressor2_F32  compressor1; // Open Audio Compressor
//AudioEffectCompressor2_F32 *pc1 = &compressor1;
radioCESSB_Z_transmit_F32 cessb1;
AudioConvert_F32toI16 float2Int1, float2Int2, float2Int3, float2Int4;  // Converts Float to Int16.  See class in AudioStream_F32.h
// AudioConvert_I16toF32 int2float1, int2float2;
AudioSwitch4_OA_F32 switch1, switch2, switch3;
AudioMixer4_F32 mixer1, mixer2;          // Used to switch in tone during calibration.
AudioSynthWaveformSine_F32  toneSSBCal;          // Tone for SSB calibration.
AudioRecordQueue Q_in_L_Ex;                    // AudioRecordQueue for input Microphone channel.
AudioRecordQueue Q_in_R_Ex;           // This 2nd channel is needed as we are bringing I and Q into the sketch instead of only microphone audio.
AudioPlayQueue Q_out_L_Ex;                     // AudioPlayQueue for driving the I channel (CW/SSB) to the QSE.
AudioPlayQueue Q_out_R_Ex;                     // AudioPlayQueue for driving the Q channel (CW/SSB) to the QSE.
//AudioInputUSB_F32 usbIn_F32(audio_settings);
//AudioInputUSB usbIn;
//radioCWModulator_F32     CWMod1(audio_settings);

//  Begin transmit signal chain.
AudioConnection connect0(i2s_quadIn, 0, int2Float1, 0);    // Microphone audio channel.  Must use int2Float because Open Audio does not have quad input.

AudioConnection_F32 connect1(int2Float1, 0, switch1, 0);   // Used switches here because it appeared necessary to stop flow of data.
AudioConnection_F32 connect2(toneSSBCal,  0, switch2, 0);  // Tone used during SSB calibration.

//AudioConnection connect3(usbIn, 0, int2Float2, 0);
//AudioConnection_F32 connect4(int2Float2, 0, switch4, 0);          // Connect USB for FT8 from WSJTX on PC.

//AudioConnection_F32 connect4(usbIn_F32, 0, switch4, 0);

// Need a mixer to switch in an audio tone during calibration.  Should be a nominal tone amplitude.
AudioConnection_F32 connect3(switch1, 0, mixer1, 0);  // Connect microphone mixer1 output 0 via gain control.
AudioConnection_F32 connect4(switch2, 0, mixer1, 1);  // Connect tone for SSB calibration.
//AudioConnection_F32 connect7(switch4, 0, mixer1, 2);  // USB audio from WSJTX on PC.

AudioConnection_F32 connect5(mixer1, 0, micGain, 0);
AudioConnection_F32 connect6(micGain, 0, switch3, 0);

AudioConnection_F32 connect7(switch3, 0, compressor1, 0);
AudioConnection_F32 connect8(compressor1, 0, mixer2, 0);

AudioConnection_F32 connect9(switch3, 1, mixer2, 1);  // Compressor bypass path.

AudioConnection_F32 connect10(mixer2, 0, cessb1, 0);

// Controlled envelope SSB from Open Audio library.
AudioConnection_F32 connect11(cessb1, 0, float2Int1, 0);
AudioConnection_F32 connect12(cessb1, 1, float2Int2, 0);

AudioConnection connect13(float2Int1, 0, Q_in_L_Ex, 0);
AudioConnection connect14(float2Int2, 0, Q_in_R_Ex, 0);

// Transmitter back-end.  This takes streaming data from the sketch and drives it into the I2S.
AudioConnection patchCord15(Q_out_L_Ex, 0, i2s_quadOut, 0);  // I channel to line out
AudioConnection patchCord16(Q_out_R_Ex, 0, i2s_quadOut, 1);  // Q channel to line out

// Receiver

AudioRecordQueue ADC_RX_I;  // I channel from ADC PCM1808.
AudioRecordQueue ADC_RX_Q;  // Q channel from ADC PCM1808.

AudioPlayQueue Q_out_L;  // Receiver audio out and CW sidetone.
//AudioPlayQueue Q_out_R;  2nd audio channel not used.  KF5N March 11, 2024

//AudioOutputUSB usbOut;
//AudioOutputUSB_F32 usbOut_F32(audio_settings);

AudioConnection patchCord9(i2s_quadIn, 2, ADC_RX_I, 0);  // Receiver I and Q channel data stream.
AudioConnection patchCord10(i2s_quadIn, 3, ADC_RX_Q, 0);

AudioAmplifier volumeAdjust, speakerScale, headphoneScale;
//AudioConnection patchCord17(Q_out_L, 0, volumeAdjust, 0);

AudioConnection patchCord17(Q_out_L, 0, speakerScale, 0);

//AudioConnection patchCord18(volumeAdjust, 0, speakerScale, 0);   // To audio ADC PCM5102 via Teensy pin 32.  Speaker.

AudioConnection patchCord18(speakerScale, 0, volumeAdjust, 0);   // To audio ADC PCM5102 via Teensy pin 32.  Speaker.

AudioConnection patchCord19(volumeAdjust, 0, headphoneScale, 0);   // To Audio Adapter via via Teensy pin 7.  Headphones.
//AudioConnection patchCord20(volumeAdjust, 0, i2s_quadOut, 1);   // To Audio Adapter via via Teensy pin 7.  Headphones (other side).

//AudioConnection patchCord20(speakerScale, 0, i2s_quadOut, 2);   // To audio ADC PCM5102 via Teensy pin 32.  Speaker.

AudioConnection patchCord20(volumeAdjust, 0, i2s_quadOut, 2);   // To audio ADC PCM5102 via Teensy pin 32.  Speaker.

AudioConnection patchCord21(headphoneScale, 0, i2s_quadOut, 0);   // To Audio Adapter via via Teensy pin 7.  Headphones.
AudioConnection patchCord22(headphoneScale, 0, i2s_quadOut, 1);   // To Audio Adapter via via Teensy pin 7.  Headphones (other side).

//AudioConnection patchCord20(volumeAdjust, 0, usbOut, 0);   // To Audio Adapter via via Teensy pin 7.
//AudioConnection patchCord21(volumeAdjust, 0, usbOut, 1);   // To Audio Adapter via via Teensy pin 7.

//AudioConnection patchCord23(volumeAdjust, 0, float2Int3, 0);   // To Audio Adapter via via Teensy pin 7.
//AudioConnection patchCord24(volumeAdjust, 0, float2Int4, 0);   // To Audio Adapter via via Teensy pin 7.

//AudioConnection patchCord25(float2Int3, 0, usbOut_F32, 0);   // To Audio Adapter via via Teensy pin 7.
//AudioConnection patchCord26(float2Int4, 0, usbOut_F32, 1);   // To Audio Adapter via via Teensy pin 7.

// It is unclear that this is actually required.
AudioControlSGTL5000 sgtl5000_2;  // This is not a 2nd Audio Adapter.  It is I2S to the PCM1808 (ADC I and Q receiver in) and PCM5102 (DAC audio out).
// End dataflow code

void controlAudioOut(AudioState audioState, bool mute);


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
      SetI2SFreq(SR[SampleRate].rate);
      sgtl5000_1.muteLineout();

      // Deactivate microphone and 1 kHz test tone.
      mixer1.gain(0, 0.0);
      mixer1.gain(1, 0.0);
      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(1);  //  Disconnect 1 kHz test tone path.
      // Stop and clear the data buffers.     
      ADC_RX_I.end();                                        // Receiver I channel
      ADC_RX_Q.end();                                        // Receiver Q channel
      ADC_RX_I.clear();                                      // Receiver I channel
      ADC_RX_Q.clear();                                      // Receiver Q channel      
      Q_in_L_Ex.end();  // Transmit I channel path.
      Q_in_R_Ex.end();  // Transmit Q channel path.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.clear();
      // Deactivate TX audio output path.
      patchCord15.disconnect();  // Disconnect transmitter I and Q channel outputs.
      patchCord16.disconnect();
//      digitalWrite(MUTE, UNMUTEAUDIO);  // Unmute speaker audio amplifier.
//      sgtl5000_1.unmuteHeadphone();
      // QSD connected and enabled
      ADC_RX_I.begin();                                        // Receiver I channel
      ADC_RX_Q.begin();                                        // Receiver Q channel
      patchCord9.connect();                                  // Receiver I channel
      patchCord10.connect();                                 // Receiver Q channel
      patchCord17.connect();                                 // Receiver audio channel
      patchCord18.connect();
      patchCord19.connect();
      patchCord20.connect();  // Experimental USB output. 
      patchCord21.connect();  // Experimental USB output.
      patchCord22.connect();  // Experimental USB output.
//      patchCord23.connect();
//      patchCord24.connect();
//      patchCord25.connect();
//      patchCord26.connect();
      volumeAdjust.gain(volumeLog[EEPROMData.audioVolume]);  // Set volume because sidetone may have changed it.
//      speakerScale.gain(volumeLog[SPEAKERSCALE]);
      speakerScale.gain(SPEAKERSCALE);      
//      headphoneScale.gain(volumeLog[HEADPHONESCALE]);
      headphoneScale.gain(HEADPHONESCALE);     
      sgtl5000_1.volume(0.8);
      controlAudioOut(EEPROMData.audioOut, false);  // Configure audio out; don't mute all.
//      sgtl5000_1.unmuteHeadphone();
      break;
    case RadioState::SSB_TRANSMIT_STATE:
        // QSD disabled and disconnected
      controlAudioOut(EEPROMData.audioOut, true);  // Mute all.
      sgtl5000_1.unmuteLineout();
      patchCord9.disconnect();   // Receiver I channel
      patchCord10.disconnect();  // Receiver Q channel
      patchCord17.disconnect();  // CW sidetone
      patchCord18.disconnect();
      patchCord19.disconnect();
      patchCord20.disconnect();
      patchCord21.disconnect();
      patchCord22.disconnect();
      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();
      SampleRate = SAMPLE_RATE_48K;
      SetI2SFreq(SR[SampleRate].rate);
      toneSSBCal.end();
//      updateMic();
      mixer1.gain(0, 1.0);    // Connect microphone audio to transmit chain.
      mixer1.gain(1, 0.0);    // Disconnect 1 kHz test tone.
//      mixer1.gain(2, 0.0);    // Connect USB audio from WSJTX on PC.
      switch1.setChannel(0);  // Connect microphone path.
      switch2.setChannel(1);  //  Disconnect 1 kHz test tone path.
     // switch4.setChannel(1);  //  Disconnect USB audio from WSJTX on PC.

      if(EEPROMData.compressorFlag) {
      switch3.setChannel(0);  // Enable compressor path!
      mixer2.gain(0, 1.0);
      mixer2.gain(1, 0.0);
      cessb1.getLevels(0);
      updateMic();
      } else {
      switch3.setChannel(1);  // Disable compressor path!
      mixer2.gain(0, 0.0);
      mixer2.gain(1, 1.0);
      }

      patchCord15.connect();  // Transmitter I channel
      patchCord16.connect();  // Transmitter Q channel

      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      break;
  
    case RadioState::FT8_TRANSMIT_STATE:
      // QSD disabled and disconnected
      controlAudioOut(EEPROMData.audioOut, false);  // FT8 audio should be headphone only.
      sgtl5000_1.unmuteLineout();
      patchCord9.disconnect();   // Receiver I channel
      patchCord10.disconnect();  // Receiver Q channel
      patchCord17.disconnect();  // CW sidetone
      patchCord18.disconnect();
      patchCord19.disconnect();
      patchCord20.disconnect();
      patchCord21.disconnect();
      patchCord22.disconnect();

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      SampleRate = SAMPLE_RATE_48K;
      SetI2SFreq(SR[SampleRate].rate);
      toneSSBCal.end();
//      updateMic();    Not needed here?
      mixer1.gain(0, 0.0);    // Disonnect microphone audio to transmit chain.
      mixer1.gain(1, 0.0);    // Disconnect 1 kHz test tone.
      mixer1.gain(2, 1.0);    // Connect USB audio from WSJTX on PC.
      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(1);  //  Disconnect 1 kHz test tone path.
//      switch4.setChannel(0);  //  Connect USB audio from WSJTX on PC.

      switch3.setChannel(1);  // Don't use compressor with FT8!
      mixer2.gain(0, 0.0);
      mixer2.gain(1, 1.0);

//      cessb1.getLevels(0);  // Initialize the CESSB information struct.
      patchCord15.connect();  // Transmitter I channel
      patchCord16.connect();  // Transmitter Q channel

      Q_in_L_Ex.begin();  // I channel Microphone audio
      Q_in_R_Ex.begin();  // Q channel Microphone audio

      break;

    case RadioState::SSB_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_48K;
      InitializeDataArrays();  // I2S sample rate set in this function.
      // QSD disabled and disconnected
      controlAudioOut(EEPROMData.audioOut, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord9.connect();   // Receiver I channel
      patchCord10.connect();  // Receiver Q channel
      patchCord17.disconnect();  // Receiver audio and CW sidetone
      patchCord18.disconnect();
      patchCord19.disconnect();
      patchCord20.disconnect();
      patchCord21.disconnect();
      patchCord22.disconnect();

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Test tone enabled and connected
      toneSSBCal.setSampleRate_Hz(48000);
      toneSSBCal.amplitude(0.3);
      toneSSBCal.frequency(750.0);
      toneSSBCal.begin();
      mixer1.gain(0, 0);  // microphone audio off.
      mixer1.gain(1, 1);  // testTone on.
      switch1.setChannel(1);  // Disconnect microphone path.
      switch2.setChannel(0);  // Connect 1 kHz test tone path.

      if(EEPROMData.compressorFlag) {
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
      ADC_RX_I.begin();     // Calibration is full duplex!  Activate receiver data.  No demodulation during calibrate, spectrum only.
      ADC_RX_Q.begin();
      patchCord15.connect();  // Transmitter I channel
      patchCord16.connect();  // Transmitter Q channel

      break;

    case RadioState::CW_TRANSMIT_STRAIGHT_STATE:
    case RadioState::CW_TRANSMIT_KEYER_STATE:
      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      // Baseband SSB data cleared and ended.
      Q_in_L_Ex.end();  // Clear I channel.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Clear Q channel.
      Q_in_R_Ex.clear();
      // QSD disabled and disconnected
      // Speaker and headphones should be unmuted according to current audio out state for sidetone.
      controlAudioOut(EEPROMData.audioOut, false);  // FT8 audio should be headphone only.
      sgtl5000_1.unmuteLineout();
 //     sgtl5000_1.volume(volumeLog[EEPROMData.sidetoneVolume]);
      sgtl5000_1.volume(0.3);  // This is for sidetone in the headphone output.  Hardcoding for now.
      patchCord9.disconnect();
      patchCord10.disconnect();
      patchCord17.disconnect();
//      speakerScale.gain(0.0);
 //     headphoneScale.gain(0.0);
//      patchCord18.disconnect();
//      patchCord19.disconnect();
//      patchCord20.disconnect();
      patchCord21.disconnect();
      patchCord22.disconnect();     

      patchCord15.connect();  // Connect I and Q transmitter output channels.
      patchCord16.connect();
      patchCord17.connect();                                    // Sidetone goes into receiver audio path.
      patchCord21.connect();
      patchCord22.connect(); 
      volumeAdjust.gain(volumeLog[EEPROMData.sidetoneVolume]);  // Adjust sidetone volume.

      break;

    case RadioState::CW_CALIBRATE_STATE:
      SampleRate = SAMPLE_RATE_192K;
      SetI2SFreq(SR[SampleRate].rate);
      // QSD receiver enabled.  Calibrate is full duplex.
      controlAudioOut(EEPROMData.audioOut, true);  // Mute all audio.
      sgtl5000_1.unmuteLineout();
      patchCord9.connect();   // Receiver I channel
      patchCord10.connect();  // Receiver Q channel
      patchCord17.disconnect();  // CW sidetone
//      patchCord18.disconnect();
//      patchCord19.disconnect();
//      patchCord20.disconnect();
//      patchCord21.disconnect();
//      patchCord22.disconnect();
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

      ADC_RX_I.begin();     // Calibration is full duplex!
      ADC_RX_Q.begin();
      //  Transmitter back-end needs to be active during CW calibration.
      patchCord15.connect();  // Transmitter I channel
      patchCord16.connect();  // Transmitter Q channel

      break;

      case RadioState::SET_CW_SIDETONE:
      SampleRate = SAMPLE_RATE_192K;
      SetI2SFreq(SR[SampleRate].rate);
      //// Speaker and headphones should be unmuted according to current audio out state!!!!!!!!
      controlAudioOut(EEPROMData.audioOut, false);  // Unmute according to audio state for sidetone.
      patchCord9.disconnect();
      patchCord10.disconnect();
      // Baseband CESSB data cleared and ended.

      ADC_RX_I.end();
      ADC_RX_I.clear();
      ADC_RX_Q.end();
      ADC_RX_Q.clear();

      Q_in_L_Ex.end();  // Clear I channel.
      Q_in_L_Ex.clear();
      Q_in_R_Ex.end();  // Clear Q channel.
      Q_in_R_Ex.clear();

      patchCord17.connect();                                    // Sidetone goes into receiver audio path.
      volumeAdjust.gain(volumeLog[EEPROMData.sidetoneVolume]);  // Adjust sidetone volume.
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
  if(mute) {
      sgtl5000_1.muteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);   // Mute speaker audio amplifier.
      return;
  }
  switch(audioState) {
     case AudioState::HEADPHONE:
      sgtl5000_1.unmuteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);   // Mute speaker audio amplifier.
      break;

     case AudioState::SPEAKER:
      sgtl5000_1.muteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, UNMUTEAUDIO);   // Mute speaker audio amplifier.
      break;

     case AudioState::BOTH:
      sgtl5000_1.unmuteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, UNMUTEAUDIO);   // Mute speaker audio amplifier.
      break;

     case AudioState::MUTE_BOTH:
      sgtl5000_1.muteHeadphone();     // Unmute headphones.
      digitalWrite(MUTE, MUTEAUDIO);   // Mute speaker audio amplifier.
      break;

      default:
      break;
}
}
