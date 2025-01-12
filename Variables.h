/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define INIT_FLASH_MEMORY 0  // Warning, this will erase the entire flash contents. Usually a new Winbond chip is initialized automatically, use this only as last resort if nothing else goes.
#define INIT_ENTIRE_FLASH 0  // When selected with the above, the entire flash chip will be erased first. This takes around 20 seconds or less to perform before we can write data to the flash chip.
                             //If this is not set, the code will erase the flash chip by sector, which takes a bit more time to perform.
                             
#define DISABLE_FLASH 0      // When set it won't use the external flash ic
#define INTRO_ANIMATION 1    // When set an animation will be shown on power up
#define SHOW_FREE_RAM 0      // Will show how much RAM is left on the display after the animation
#define DEFAULT_NOTE 36      // Default note when adding steps to a new pattern, else the last used note number is carried on.
#define DEFAULT_MIDI_CH 10   // MIDI Default Channel for all 10 drum tracks - track 1 - 10
#define RECORD_FORCE_VEL 0   // When set velocity will always be the maximum for note track recording instead of quantizing the input velocity to the three usual accent values
#define EXTERNAL_CLOCK 0     // When set external midi beat clock is set by default
#define BUFFER_SIZE 128      // UART software buffer size, can go as low as 64
#define MIDI_OVER_USB 0      // When set will use 38400 bauds for the Serial interface - EOSequencer will not be received as midi device, needs special COM drivers to use it via USB with a DAW. \
                             // DIN midi is preferred for superior timing, EOSequencer was never planned as a USB MIDI device - you may experiment with this setting at your own risk.

#define DRUM_TRACKS 10       // This can't go above 16 - not recommended to change
#define NOTE_TRACKS 6        // This can't go above 8 - not recommended to change
#define PATTERNS 16          // This can't go above 64 - not recommended to change
#define PT_BANKS 16          // # of Pattern Banks - not recommended to change
#define STEPS 16             // This can't go above 16 - not recommended to change

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define TEMPORARY_MESSAGE_TIME 80            //Time for error messages and playback mode splashscreen, value is tied in with current BPM.
#define EXTERNAL_CLOCK_TIMER (F_CPU / 3960)  // Timer speed used to read the External MIDI Clock - MIDI is 31250 bauds = 3906 bytes per second
#define MIDI_INPUT_BUFFER (4 * 2)            // The size of the internal buffer used for recording, do not change

#define FLASH_CHIPSIZE MB64
#define FLASH_VERSION '2'

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

constexpr size_t SYSEX_CHUNK_DATA_SIZE = 140;  // Max data bytes per SysEx package
constexpr size_t SYSEX_HEADER_SIZE = 6;        // Header: F0 7D 45 4F 53 <TYPE>
constexpr size_t SYS_EX_BUFFER_SIZE = 160;     // Total buffer size (must fit in RAM)

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum {
  kButtonNone = 0,
  kButtonClicked,
  kButtonHold,
  kButtonRelease,
  kButtonReleaseNothingClicked,
  kRightSteps = 0,
  kRightTrackSelection,
  kRightPatternSelection,
  kRightMenuCopyPaste,
  kRightMenu,
  kMuteMenu,
  midiNoteOn = 0x90,
  midiNoteOff = 0x80,
  midiCC = 0xB0,
  midiProgramChange = 0xC0,
  midiChannels = 16,
  //_________________________________________________________________________________________________________________________________________________________________________________ONLY PATTERN SETUP MENU below
  menuFirst = 0,  //Start of menu

  menuPtPlays = 0,  //How many times to repeat current pattern before switching to "menuPtNext" - if there is no next pattern defined the setting is irrelevant. PAT REPEAT in setup menu - infinite to 255 times

  menuPtNext,  //Which pattern to play next if the repeats from previous setting have been played back. A simple song building tool. NEXT PAT in setup menu.
               //PtNext can be any pattern in any bank, but also it is possible to create a chain of patterns and jump from the last pattern to the first creating an infinite chain of patterns.

  menuPtPlaysChain,  //CHAIN REPEAT - this is the most complex setup parameter that allows to escape from the infinite chain of patterns.

  //Only a graph can describe this properly:
  //Lets program our song based on a pattern chain
  //PATT______PAT REPEAT________NEXT PAT_________CHAIN REPEAT
  //  01              04              02         x
  //  02              04              03         x
  //  03              04              04         x
  //  04              04              01         4

  // Actual playback from this chain setup:
  // 01 01 01 01 02 02 02 02 03 03 03 03 04 04 04 04
  // 01 01 01 01 02 02 02 02 03 03 03 03 04 04 04 04
  // 01 01 01 01 02 02 02 02 03 03 03 03 04 04 04 04
  // 01 01 01 01 02 02 02 02 03 03 03 03 04 04 04 04
  // 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05... 05 will repeat unless you stop playback or setup the next pattern chain for your song or performance

  //So CHAIN REPEAT actually counts the repeat cycles of the current set up pattern chain, decreasing by one with each loop.
  //when CHAIN REPEAT reaches zero it switches to the next pattern (05) in that case.

  //___________________________________________________________________________________________________________________________________________________________________________________________________________________________

  menuMIDIChannel,  //midi channel per sequencer track (1-16) select step 1-16 and select channel with -/+

  menuProgramChange,  //Program change per sequencer track (1-16), sent out at early playback start, select step 1-16 and select program change 1-127 with -/+.
                      //Mind the offset on some midi gear, on EOSequencer the first program will always be 1, on your equipment patch memory might start from 0.
                      //This is only hardcoded for now and can be still changed in the code.

  menuNote,  //note per drumtrack (1-10), select step 1-10 and select note with -/+

  menuAccent1,  //velocity discrete value 1
  menuAccent2,  //velocity discrete value 2
  menuAccent3,  //velocity discrete value 3

  menuVariationsABCD,  //Each pattern consists of 64 steps arranged in 4 sections A B C D. With this setup option you can change the amount of variation sections for current pattern to A, AB, ABC or full ABCD.

  menuTrackLen,  //Polymetrics TRACK length per sequencer track (1-16), Each track can have a different length and wraps independently.
                 //Pattern switching behaviour is based on the longest track, when the longest track was played once and a new pattern is changed,
                 //the switch will happen after the current polymetric pattern is played once. ABCD variations are disabled as soon as one track has less than 16 steps.
                 //Architecture wise it was not possible to realize polymetrics with tracks longer than 16.

  menuNoteLen,  //Midi note length per sequencer track (1-16), a full 16th note is 6 midi clock ticks (24PPQN), so a value below 6 can give very short sounds, depending on your sound module.
                //Notes can be up to 129 tics long, which equals more than one full bar

  menuShuffle,  //Global shuffle value (for all tracks) per pattern, based on 24PPQN timing. Shuffle 0 is no shuffle, shuffle 1 delays every even 16th and 32th note for one midiclock tick,
                //Shuffle 2 delays for two midiclock ticks, shuffle 3 is already very extreme setting and delays th 16th step for 3 tics
                //the 32nd step is not played anymore as it is outside the grid. Shuffle 4 and 5 is very extreme.
                //Shuffle 6 would not be possible as the delay is based on one 16th step (=6 tics in 14PPQN architecture) so 6 Tic shuffle would already delay into the next 16th note.
                //Shuffle does not mix well with Polymetrics, as we calculate the shuffle as a PPQ offset automatically, shuffling a Polymetric pattern can have unexpected behaviour but lead to musical results.
                //Explaino: We shuffle every second step, it is the classic Linn MPC shuffle but only with a 24PPQN timebase (Linn used 48PPQN).
                //So when we have polymetric patterns with an uneven amount of steps inside a track, these will loop at different points than a standard 16 step based pattern.
                //The shuffle logic will anyways shuffle every second step (based on step alignment of longest track length).

  menuTrackProbability,  //STEP SKIP per sequencer track (1-16), if 0 steps will be alway played, if 100 no steps will placed,
                         //if 50 there is a 50% chance that the current step will be played back. select step 1-16 and select STEP SKIP 0-100 in 10% increments with -/+



  //_______________________________________________________________________________________________________________________________________________________________________________________________________________global options below
  menuClockType,      //Run on Internal or external midiclock.
  menuSyncOut,        //Output of midiclock. Sequencer will ALWAYS send midi start/stop commands also on internal clock. Sequencer will only emit $F8 midi clock when playback has been started.
  menuClockDivider,   //As we found out that the EOSeuquencer can operate up to 1000BPM under certain conditions, we decided to offer divide down for the outgoing midiclock. Also as slave device, external midiclock can be divided down.
  menuPulseOut,       //Click sync TE/Volca/sync - Divider of the click (partially triplets and odd rythms are possible)
  menuPulseOutLen,    //Click sync TE/Volca/sync - Lenght of audio click - this really depends on your gear. If you hook up a led, this parameter will define how long the blinking impulse is.
                      //Sequencer is outputting this sync click also when slaved to midi clock, so in the most simple application you can use the bare brainboard without interface to convert midiclock to volca sync.
  menuMIDIinPattern,  //Pattern switching by midi note, input channel. First received note is bank, second note is pattern.
  menuProtect,        //Enable to protect flash from new writes or erasing - when you jam hard and dont want to destroy your songs.
  menuInit,           //Initialize BANK or WHOLE memory, reset setups and ERASE ALL DATA. We named this point ERASE in the actual setup menu as INIT just doesnt sound dangerous enough.

  lastMenu = menuInit,  //end of menu - wrap
  //_____________________________________________________________________________________________________________________________________________________________________________________________________________ONLY SETUP MENU above
  //
  kLeftMain = 0,
  kRandom,
  kErase,  //, kROL, kROR,
  kMemoryProtectMessage = 0,
  kPatternRepeatMessage,
  kRepeatModeNormal = 0,
  kRepeatModeChain,
  kRepeatModePattern

};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
SPIFlash flash;
byte segments[3][8];
byte leds[3];
byte buttons[3];  // raw button values
byte buttonEvent[3][8];
byte buttonEventWasHolding[3] = { 0, 0, 0 };
byte buttonIsHolding[8];
byte buttonDownTime[3][8];  // time of the button press
byte variation = 0;         // ABCD Variations
char forceVariation = -1;
byte curTrack = 0;
bool updateScreen = true;
byte curLeftScreen = 0;
byte curRightScreen = 0;
byte currentPattern = 0;
byte nextPattern = 0;
byte currentPatternBank = 0;
byte nextPatternBank = 0;
uint16_t patternBitsSelector;
bool mirror = false;
bool somethingClicked = false;
bool somethingHappened = false;
bool forceAccent = false;
bool seqPlaying = false;
byte seqPosition = 0;
byte PPQcounter = 0;
byte midiClockBeats = 0;
byte seqPositionPrev = 0;
byte PPQcounterPrev = 0;
byte variationPrev = 0;
byte midiClockBeatsPrev = 0;
bool somethingChangedPattern = false;  // if true saves the pattern (steps, double steps variations...)
bool somethingChangedGlobal = false;   // if true saves global parameters (clock int/ext, sync out, write protect, midi in to pattern call...)
bool streamNextPattern = false;
bool loadPatternNow = false;
byte ignoreButtons = false;
byte lastVelocity = B00000011;
byte lastVelocityMIDI = B00000011;
bool editingNote = false;
byte editStep = 0;
byte editVariation = 0;
byte prevPlayedNote[NOTE_TRACKS];  //
byte menuPosition = 0;
byte initMode = 0;
char flashHeader[8];
uint32_t prevMuteTrack = 0;
byte totalFlashErrors = 0;
byte prevPatternTotalVariations = 4;
byte midiInputStage = 0;
byte midiInputBuffer[2] = { 0, 0 };

//byte midiOutputBuffer[3][(DRUM_TRACKS + NOTE_TRACKS) * 5];  // = 3*(10+6)*3 = 144 bytes - 48 3byte midi events
byte midiOutputBuffer[3][48];  //

volatile byte midiOutputBufferPosition = 0;
volatile byte calculateSequencer = 0;
uint16_t patternPagePos = 0;
uint16_t pagePos = 0;
char editingNoteTranspose = -127;
bool noteTransposeWasChanged = false;
bool noteTransposeEditAllSteps = false;
byte leftLEDsVarSeq = 0;
byte leftLEDsVarSeqBuffer = 0;
uint16_t chaseLEDs[2] = { 0, 0 };
uint16_t chaseLEDsBuffer[2] = { 0, 0 };
byte stepVelocity = 0;
byte noteStepGlideDoubleOff = 0;
bool ignoreNextButton = false;
byte currentPlaysPattern = 0;
byte currentPlaysChain = 0;
byte repeatMode = 0;
byte showTemporaryMessage = 0;
byte temporaryMessageCounter = 0;
byte xm = 0;
byte inputByte = 0;
byte noteLenCounters[DRUM_TRACKS + NOTE_TRACKS];
byte noteLenCountersLED[DRUM_TRACKS + NOTE_TRACKS];
byte tickOutCounter = 0;
byte tickOutCounterLen = 0;
bool tickOutPinState = false;
bool recordEnabled = false;
byte recordBuffer[3][MIDI_INPUT_BUFFER];
byte recordBufferPosition = 0;
bool isSelectingBank = false;
byte preventABCD = 0;  //flag for preventing ABCD mode change when button [0][7] is pushed
uint16_t realBPM = 140;
uint16_t freeRunningCounter = 0;  // counter for animating segments independent from playback speed
uint8_t quantizedValue = 0;       // Global quantized value (0-15) for char animation counter
byte trackPosition[DRUM_TRACKS + NOTE_TRACKS] = {};
const byte trackSequence[16] = { 9, 8, 7, 6, 15, 5, 14, 4, 13, 3, 12, 2, 11, 1, 10, 0 };  // playback priority of tracks, reverse order! 0 is drum track 1, 10 is synth track 1
volatile byte longestTRACK = 0;                                                           // amount of steps in longest track, polymetrics
volatile byte longestTrackIndex = 0;                                                      //for polymetrics, to determine when to switch to next pattern, index is pointing to patternData.trackLength
bool doAlign = false;                                                                     //alignment of polymetric tracks
uint32_t muteTrack = 0x00;                                                                //all channels unmuted;
volatile byte shuffleCounter = 0;                                                         //each even 16th and 32nd step is generated by shufflecounter and value in setup
volatile bool alignedToPlay16 = false;                                                    //shuffled or straight playback flag for 16th step
volatile bool alignedToPlay32 = false;                                                    //same for 32nd step
volatile bool isEvenStep = false;                                                         //flag to determine even steps when to start shuffle counter
volatile byte ledPosition = 0;                                                            //position memory to show shuffle correctly on playback leds
volatile byte midiClockCounter = 0;                                                       // Counter for midiclock divide down division
volatile bool midiToBank = true;                                                                   // Flag to interpret first midi note on external pattern switching control as bank number (second as patttern)
volatile byte sysExBuffer[SYS_EX_BUFFER_SIZE];                                            // Buffer for SysEx data
volatile size_t sysExBufferIndex = 0;                                                     // Index for the buffer
volatile byte receivingExternalSysEx = false;                                             //flags when pattern dump request or pattern dump is received
volatile bool sysExCompleteFlag = false;                                                  //flags when sysex transmission has ended
volatile bool sysExOverflowFlag = false;                                                  //flags when data is sent to fast, ideally use a delay of 500mS between buffers, use elektron C6 or midiox to request dumps
volatile bool sysExChunkCompleteFlag = false;                                             //sysex is chunked into aprrox 128 byte packages, packages stars with $F0 and ends with $F7, each package must be sent with a buffer delay to give the arduino time to reconstruct the bytes and do a final flash memory write


//_____BELOW: STEP MEMORY ARRAYS, PATTERN configData ARRAY, GLOBAL configData ARRAY______________________________________________________________________________________________________________________________________________________


#pragma pack(push, 1)  //pragma pack is needed to ensure all variables take exact placement in the byte space without unnecessary padding, 
                       //so eventual array based syx dumps can be read nicer and also be dumped back and be saved to flash memory.

// Structuring of STEP data in arrays:
//
//Drum Tracks:
//Compressed format: Each step uses 2 bits for velocity 00 01 10 11
//For drum tracks only velocity is saved, step note is defined by patternData.trackNote[0-9]
//A single byte can hold the velocity for 4 steps in a single variation in AABBCCDD format.
//Double step byte, one bit per step - only a one bit flag per step, packed as AABBCCDD.

//Note Tracks:
//Full format:
//1 byte per step for the note value 0-127.
//2 extra byte per step for velocity and additional flags (noteStepsExtras).

////    Bits in noteStepsExtras

//noteStepsExtras[x][0] and noteStepsExtras[x][1] are used to store additional step data.
//Each variation occupies 2 bits, packed into nibbles (4 bits) per byte.

// noteStepsExtras[x][0] - Velocity (Lower Nybble)
// ------------------------------------------------
// - Stores velocity information for each variation:
//   - 00: Velocity 0 (step off)
//   - 01: Velocity 1 (low)
//   - 10: Velocity 2 (medium)
//   - 11: Velocity 3 (high)
// - Each variation's 2 bits are mapped sequentially:
//   - Variation A: Bits 0–1
//   - Variation B: Bits 2–3
//   - Variation C: Bits 4–5
//   - Variation D: Bits 6–7

// noteStepsExtras[x][1] - Attributes (Upper Byte)
// ------------------------------------------------
// - Stores step attributes for each variation:
//   - 00: No attribute
//   - 01: Slide (legato enabled)
//   - 10: Double Note (triggers double note event) //not used
//   - 11: Note Off (explicit note-off for this step)
// - Each variation's 2 bits are mapped sequentially:
//   - Variation A: Bits 0–1
//   - Variation B: Bits 2–3
//   - Variation C: Bits 4–5
//   - Variation D: Bits 6–7


// Compact Encoding:
// -----------------
// Each note step uses 2 bytes (16 bits) to store velocity and attributes for all 4 variations.

struct WSTEPS  // Actual STEP data
{
  byte steps[DRUM_TRACKS];  // steps are stored as 8 bits variations in AABBCCDD format. 2 bits per step per variation. 0~3
  uint16_t stepsDouble[4];  // drum tracks, 4 bits each for the ABCD variations
  byte noteSteps[NOTE_TRACKS][4];
  byte noteStepsExtras[NOTE_TRACKS][2];  // 2 bytes packed as ABCD: [0] = Velocity (2 bits) ABCD, [1] = Glide/Double ABCD (2 bits)
  //
  void init() {
    memset(steps, 0, sizeof(steps));
    memset(stepsDouble, 0, sizeof(stepsDouble));
    memset(noteSteps, 0, sizeof(noteSteps));
    memset(noteStepsExtras, 0, sizeof(noteStepsExtras));
  }
};



//__________BELOW: PATTERN configData Array memory
//__________mind config data per pattern can be carried over to a new pattern by using pattern copy
//__________ERASE by setup will always apply the init values defined in this array
//__________This means ERASE per setup option will INITIALIZE either a whole BANK or whole MEMORY to the below init values
//__________That way it is possibly to have a complete different config for each pattern
//__________But also with great power comes great responsibility: If you start to write patterns and later realize that you changed the settings a lot with each pattern, you can only manually apply all the settings you changed (e.g. have different drum notes suddenly but expect the previous patterns also to have the new sounds).
//__________this is why PAT COPY PASTE CLR is an essential function for editing, as it copies ALL pattern data, all steps and also the config
//__________CLR (CLEAR) in the PAT COPY PASTE CLR menu will only erase the steps velocities, the pattern setup data stays unchanged.

struct WPATTERN  // Config per PATTERN
{
  byte totalVariations;
  byte lastNote[NOTE_TRACKS];
  uint16_t nextPattern;
  byte playsPattern, playsChain;
  byte programChange[DRUM_TRACKS + NOTE_TRACKS];
  volatile byte shuffleDelay;
  volatile byte trackLen[DRUM_TRACKS + NOTE_TRACKS];
  volatile byte trackProbability[DRUM_TRACKS + NOTE_TRACKS];
  byte drumNoteLen[DRUM_TRACKS + NOTE_TRACKS];
  byte trackNote[DRUM_TRACKS];
  byte trackMidiCH[DRUM_TRACKS + NOTE_TRACKS];  // 0~15
  byte accentValues[3];


  void init() {

    memset(lastNote, DEFAULT_NOTE, sizeof(lastNote));
    memset((void *)trackLen, 16, sizeof(trackLen));
    memset((void *)trackProbability, 0, sizeof(trackProbability));
    memset(drumNoteLen, 6, sizeof(drumNoteLen));
    memset(trackMidiCH, (DEFAULT_MIDI_CH - 1), sizeof(trackMidiCH));
    for (xm = 0; xm < NOTE_TRACKS; xm++) {
      trackMidiCH[DRUM_TRACKS + xm] = xm;
    }

    totalVariations = 4;
    nextPattern = 0;
    playsPattern = 0;
    playsChain = 1;
    shuffleDelay = 0;

    accentValues[0] = 63;
    accentValues[1] = 93;
    accentValues[2] = 127;

    trackNote[0] = 36;  //default drum notes can be changed here
    trackNote[1] = 38;
    trackNote[2] = 39;
    trackNote[3] = 40;
    trackNote[4] = 41;
    trackNote[5] = 42;
    trackNote[6] = 44;
    trackNote[7] = 46;
    trackNote[8] = 48;
    trackNote[9] = 50;
  }
};
//



struct GCONFIG  // GLOBAL config
{
  bool seqSyncOut;
  byte midiInputToPatternChannel;
  bool midiClockInternal;
  bool writeProtectFlash;
  byte tickOut;
  byte tickOutLen;
  volatile byte midiClockDivide;
  //
  void init() {
    //
    tickOut = 2;
    tickOutLen = 2;
    seqSyncOut = true;
    midiInputToPatternChannel = 15;
    writeProtectFlash = false;
    midiClockDivide = 1;
//
#if EXTERNAL_CLOCK
    midiClockInternal = false;
#else
    midiClockInternal = true;
#endif
  }
};

#pragma pack(pop)


//

WSTEPS stepsData[STEPS];  //actual step data, velocity, 32nd note drumstep, note track pitch/velocity/glide/noteoff...
WPATTERN patternData;  //config data, unique saved for each pattern
GCONFIG globalData;    //global data, saved in a permanent resident spot in memory, valid for any pattern

//_____________________________________________________________________________________________________________________________________________________________________________________________________________________________________


// ---7 SEGMENT DISPLAY DEFINITIONS----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 *    A     --
 *   F B   |  |
 *    G     --
 *   E C   |  |
 *    D     --
 *       X      .
 *       
 *       
 *  XGFEDCBA
 *  00110111
 *  *  B00000001
 *  *  B00000010
 *  *  B01000000
 *  *  B00010000
 *  *  B00001000
 *  *  B00000100
 *  *  B01000000
 *  *  B00100000
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  7 segment font can be customized here

#define S_MUTE B00001000 //CHAR SIGN FOR MUTED
#define S_UNMUTE B00110111  //CHAR SIGN FOR UNMUTED

//Below: Alphabet and custom number definition
#define S_1 B00000110
#define S_1_ B00110000
#define S_W1 B01100100
#define S_W2 B01010010
#define S_u B00011100
#define S_n B01010100
#define S_d B01011110
#define S_V B00111100
#define S_X B01000100
#define S_Z B01101100
#define S_R B01010111
#define S_DASH B01000000
#define S_S B01101101
#define S_Y B01101110
#define S_L B00111000
#define S_O B00111111
#define S_U B00111110
#define S_G B00111101
#define S_F B01110001
#define S_o B01011100
#define S_C B00111001
#define S_N B00110111
#define S_E B01111001
#define S_t B01111000
#define S_n B01010100
#define S_K B01111010
#define S_r B01010000
#define S_T B00110001
#define S_d B01011110
#define S_c B01011000
#define S_A B01110111
#define S_b B01111100
#define S_P B01110011
#define S_I B00110000
#define S_H B01110110
#define S__ B00001000
#define S_DOT B10000000

//Below: Char representation for empty step, step velocities - 32nd note, legato and note off display chars definitions are embedded in code as where needed
const byte stepChars[4] PROGMEM = { B00000000, B01000000, B01100010, B01100011 };

//Below: Standardiyed number font
const byte numbers[10] PROGMEM = { B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01101111 };
//END OF LINE
