#pragma once

// Command audio Morse Response messages
//#define DEFAULT_SPEED_WPM 15           // 15 WPM default keyer speed
#define IAMBIC_B 0x10  // 0x00 for Iambic A, 0x10 for Iambic B (Binary 0001 0000)
#define IAMBIC_A 0     // Iambic B is default and assumed preferred.
#define EEADDRESS 3072  // Starting address for EEPROM
#define DIT_L 0x01     // Dit latch (Binary 0000 0001)
#define DAH_L 0x02     // Dah latch (Binary 0000 0010)
#define DIT_PROC 0x04  // Dit is being processed (Binary 0000 0100)


class CWProcessing {

public:

const int32_t HISTOGRAM_ELEMENTS = 750;
const int32_t MAX_DECODE_CHARS = 32;       // Max chars that can appear on decoder line.  Increased to 32.  KF5N October 29, 2023
const uint32_t DECODER_BUFFER_SIZE = 128;  // Max chars in binary search string with , . ?

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
bool keyPressedOn = false;
bool keyerFirstDit = false;
bool keyerFirstDah = false;

// This enum is used by an experimental Morse decoder.
enum states { state0,
              state1,
              state2,
              state3,
              state4,
              state5,
              state6 };
states decodeStates = state0;

float32_t transmitDitLength;
uint32_t transmitDitUnshapedBlocks;
uint32_t transmitDahUnshapedBlocks;
static constexpr float32_t cwBlockLength = (512.0 / 48000.0) * 1000.0;  // milliseconds of one chunk of CW waveform.
bool sendDit = false;
bool sendDah = false;

bool charProcessFlag, blankFlag;
int currentTime, interElementGap, noSignalTimeStamp;
char *bigMorseCodeTree = (char *)"-EISH5--4--V---3--UF--------?-2--ARL---------.--.WP------J---1--TNDB6--.--X/-----KC------Y------MGZ7----,Q------O-8------9--0----";

void JackClusteredArrayMax(int32_t *array, int32_t elements, int32_t *maxCount, int32_t *maxIndex, int32_t *firstNonZero, int32_t spread);
void SelectCWFilter();
void SelectCWOffset();
void DoCWReceiveProcessing();
void SetTransmitDitLength(int wpm);
void SetKeyType();
void SetKeyPowerUp();
void SetSideToneVolume(bool speaker);
void MorseCharacterClear(void);  // Remove this when Morse decoder is changed to BTE.
void MorseCharacterDisplay(char currentLetter);  // Also removed/modified after change to BTE.
void ResetHistograms();
void DoGapHistogram(uint32_t gapLen);
void DoCWDecoding(int audioValue);
void DoSignalHistogram(long val);
float goertzel_mag(int numSamples, int TARGET_FREQUENCY, int SAMPLING_RATE, float *data);

// Morse Iambic keyer.  The state machine is a derivative of the "Simple CW Keyer" by VE3WMB
// https://github.com/ve3wmb/SimpleCWKeyer

// Keyer State machine type - states for sending of Morse elements based on paddle input
enum class KSTYPE {
  IDLE,
  CHK_DIT,
  CHK_DAH,
  KEYED_PREP,
  KEYED,
  INTER_ELEMENT,
  PRE_IDLE,
};

uint8_t g_keyerControl;       // 0x1 Dit latch, 0x2 Dah latch, 0x04 Dit being processed, 0x10 for Iambic A or 1 for B
KSTYPE g_keyerState = KSTYPE::IDLE;   // Keyer state global variable

void init_iambic();
void iambicStateMachine(uint32_t& cwTimer);
void updatePaddleLatch();

};