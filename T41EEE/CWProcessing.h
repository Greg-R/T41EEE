// CWProcessing class.
// CW mode related functions.

#pragma once

class CWProcessing {

public:

  static constexpr int32_t HISTOGRAM_ELEMENTS = 750;
  static constexpr int32_t MAX_DECODE_CHARS = 32;       // Max chars that can appear on decoder line.  Increased to 32.  KF5N October 29, 2023
  static constexpr uint32_t DECODER_BUFFER_SIZE = 128;  // Max chars in binary search string with , . ?

  byte currentDashJump = DECODER_BUFFER_SIZE;
  byte currentDecoderIndex = 0;

  float32_t aveCorrResultR;  // Used in running averages; must not be inside function.
  float32_t aveCorrResultL;
  float32_t *float_Corr_BufferR = new float32_t[511];
  float32_t *float_Corr_BufferL = new float32_t[511];
  float CWLevelTimer;
  float CWLevelTimerOld;
  float goertzelMagnitude;
  char decodeBuffer[33] = { 0 };  // The buffer for holding the decoded characters.  Increased to 33.  KF5N October 29, 2023
  int endGapFlag = 0;
  int topGapIndex;
  int topGapIndexOld;
  int32_t *gapHistogram = new int32_t[HISTOGRAM_ELEMENTS];
  int32_t *signalHistogram = new int32_t[HISTOGRAM_ELEMENTS];
  long valRef1;
  long valRef2;
  long gapRef1;
  int valFlag = 0;
  long signalStartOld = 0;
  long aveDitLength = 80;
  long aveDahLength = 200;
  float thresholdGeometricMean = 140.0;  // This changes as decoder runs
  float thresholdArithmeticMean;
  int dahLength;
  int gapAtom;  // Space between atoms
  int gapChar;  // Space between characters
  long signalElapsedTime;
  long signalStart;
  long signalEnd;  // Start-end of dit or dah
  uint32_t gapLength;
  float32_t freq[4] = { 562.5, 656.5, 750.0, 843.75 };
  bool drewGreenLastLoop{ false };  // Variables used to control drawing of CW decode
  bool drewBlackLastLoop{ false };  // indicator square.
  std::vector<std::string> CWOffsets = { "562.5", "656.5", "750", "843.8" };
  std::vector<std::string> keyChoice = { "Straight Key", "Keyer" };
  std::vector<std::string> CWFilter = { "0.8kHz", "1.0kHz", "1.3kHz", "1.8kHz", "2.0kHz", " Off " };
  int col = 0;  // Start at lower left
  // charProcessFlag means a character is being decoded.  blankFlag indicates a blank has already been printed.
  bool charProcessFlag, blankFlag;
  int currentTime, interElementGap, noSignalTimeStamp;
  char *bigMorseCodeTree = (char *)"-EISH5--4--V---3--UF--------?-2--ARL---------.--.WP------J---1--TNDB6--.--X/-----KC------Y------MGZ7----,Q------O-8------9--0----";

  // This enum is used by an experimental Morse decoder.
  enum states { state0,
                state1,
                state2,
                state3,
                state4,
                state5,
                state6 };
  states decodeStates = state0;

  void JackClusteredArrayMax(int32_t *array, int32_t elements, int32_t *maxCount, int32_t *maxIndex, int32_t *firstNonZero, int32_t spread);
  void SelectCWFilter();
  void SelectCWOffset();
  void SetKeyerWPM();
  void DoCWReceiveProcessing();
  void SetTransmitDitLength(int wpm);
  void SetKeyType();
  void SetKeyPowerUp();
  void SetSideToneVolume(bool speaker);
  void MorseCharacterClear(void);
  void MorseCharacterDisplay(char currentLetter);
  void ResetHistograms();
  void DoGapHistogram(uint32_t gapLen);
  void DoCWDecoding(int audioValue);
  void DoSignalHistogram(long val);
  float goertzel_mag(int numSamples, int TARGET_FREQUENCY, int SAMPLING_RATE, float *data);
};