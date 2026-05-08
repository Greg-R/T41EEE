#pragma once

// Command audio Morse Response messages
#define CMD_MODE_ENTRY_MESSAGE " K"   // Entered command mode
#define CMD_MODE_EXIT_MESSAGE " * K"  // "AR K" - Exit Command Mode
#define CMD_OK_MESSAGE " R"           // Command input via paddles is recognized
#define CMD_NOT_OK_MESSAGE " ?"       // Command input via the paddles is not supported
#define CMD_ERROR_MESSAGE " #"        // ....... Error in command parameter
#define PWR_ON_MESSAGE "OK"           // Keyer Power on Message
#define DEFAULT_SPEED_WPM 15           // 15 WPM default keyer speed
#define IAMBIC_B 0x10  // 0x00 for Iambic A, 0x10 for Iambic B (Binary 0001 0000)
#define EEADDRESS 3072  // Starting address for EEPROM

#define DIT_L 0x01     // Dit latch (Binary 0000 0001)
#define DAH_L 0x02     // Dah latch (Binary 0000 0010)
#define DIT_PROC 0x04  // Dit is being processed (Binary 0000 0100)
#define IAMBIC_B 0x10  // 0x00 for Iambic A, 0x10 for Iambic B (Binary 0001 0000)
#define IAMBIC_A 0


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

// Iambic keyer.

//  Morse Iambic Keyer State Machine type
// Keyer State machine type - states for sending of Morse elements based on paddle input
enum KSTYPE {
  IDLE,
  CHK_DIT,
  CHK_DAH,
  KEYED_PREP,
  KEYED,
  INTER_ELEMENT,
  PRE_IDLE,
};


// Keyer Command State type - states for processing of Commands entered via the paddles after entering Command Mode
enum KEYER_CMD_STATE_TYPE {
  CMD_IDLE,
  CMD_ENTER,
  CMD_TUNE,
  CMD_SPEED_WAIT_D1,
  CMD_SPEED_WAIT_D2,

};

// Data type for storing Keyer settings in EEPROM
struct KeyerConfigType {
  uint32_t ms_per_dit;         // Speed
  uint8_t dit_paddle_pin;      // Dit paddle pin number
  uint8_t dah_paddle_pin;      // Dah Paddle pin number
  uint8_t iambic_keying_mode;  // 0 - Iambic-A, 1 Iambic-B
  uint8_t sidetone_is_muted;   //
  uint32_t num_writes;         // Number of writes to EEPROM, incremented on each 'W' command
  uint32_t data_checksum;      // Checksum containing a 32-bit sum of all of the other fields
};

uint32_t g_ditTime;           // Number of milliseconds per dit
uint8_t g_keyerControl;       // 0x1 Dit latch, 0x2 Dah latch, 0x04 Dit being processed, 0x10 for Iambic A or 1 for B
KSTYPE g_keyerState = IDLE;   // Keyer state global variable
KeyerConfigType g_ks_eeprom;  // Persistant Keyer config info - written to EEPROM on 'W' command.
uint8_t g_new_keyer_wpm;
bool g_sidetone_muted;  // If 'true' the Piezo speaker output is muted except for when in command mode


// We declare these as variables so that DIT and DAH paddles can be swapped for both left and right hand users. Convention is DIT on thumb.
uint8_t g_dit_paddle;  // Current PIN number for Dit paddle
uint8_t g_dah_paddle;  // Current PIN number for Dah paddle

// We use this variable to encode Morse elements (DIT = 0, DAH =1) to capture the current character sent via the paddles which is needed for user commands.
// The encoding practice is to left pad the character with 1's. A zero start bit is to the left of the first encoded element.
// We start with B11111110 and shift left with a 0 or 1 from the least significant bit, according the last Morse element received (DIT = 0, DAH =1).
uint8_t g_current_morse_character = B11111110;

// Keyer Command Mode Variables
uint32_t g_cmd_mode_input_timeout = 0;                  // Maximum wait time after hitting the command button with no paddle input before Command Mode auto-exit
KEYER_CMD_STATE_TYPE g_keyer_command_state = CMD_IDLE;  // This is the state variable for the Command Mode state machine

uint32_t ktimer;

// Keyer response messages sent as Morse Audio feedback via the Piezo Speaker (message contents defined in KeyerCmd.h)
// Note that these are also globals but given that they are constants there is no chance that they can be accidentally overwritten.
const char pwr_on_msg[3] = PWR_ON_MESSAGE;
const char cmd_mode_entry_msg[3] = CMD_MODE_ENTRY_MESSAGE;
const char cmd_mode_exit_msg[5] = CMD_MODE_EXIT_MESSAGE;
const char cmd_recognized_msg[3] = CMD_OK_MESSAGE;
const char cmd_not_recognized_msg[3] = CMD_NOT_OK_MESSAGE;
const char cmd_error_msg[3] = CMD_ERROR_MESSAGE;

void loadWPM(int wpm);
void init_iambic();
uint32_t calculate_ee_checksum();
bool validate_ee_checksum();
void iambicStateMachine(uint32_t& cwTimer);
void updatePaddleLatch();

};