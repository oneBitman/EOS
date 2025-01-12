/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void createScreen() {
  // Sequence Position ---------------------------------------------------------------------------------------------------------------- //
  leds[0] = leds[1] = leds[2] = 0;
  //if (seqPlaying) leds[0] = leftLEDsVarSeq; else bitSet(leds[0], 0); //BLINKENLIGHTS left REDUX
  //
  if (!seqPlaying || seqPlaying)  //ABCD leds always
  {
    byte xVar = variation;
    if (forceVariation >= 0) xVar = forceVariation;
    if (mirror) leds[0] |= B11110000;
    else {
      if (forceVariation >= 0) {
        leds[0] |= B11110000;
        bitClear(leds[0], xVar + 4);
      } else bitSet(leds[0], xVar + 4);
    }
  }

  if (curRightScreen == kRightPatternSelection) {
    // Clear all LEDs first and display current bank/pattern on 16 step leds
    leds[1] = 0;
    leds[2] = 0;

    if (isSelectingBank) {
      if (nextPatternBank < 8) {
        bitSet(leds[1], nextPatternBank);  // Set LED on the first byte (1-8 patterns)
      } else {
        bitSet(leds[2], nextPatternBank - 8);  // Set LED on the second byte (9-16 patterns)
      }
    } else {
      //
      if (nextPattern < 8) {
        bitSet(leds[1], nextPattern);  // Set LED on the first byte (1-8 patterns)
      } else {
        bitSet(leds[2], nextPattern - 8);  // Set LED on the second byte (9-16 patterns)
      }
    }
  }


  //where to show (not show) track selection led outside of normal sequencing
  if ((curRightScreen != kMuteMenu && curRightScreen != kRightPatternSelection && curRightScreen == kRightMenu && menuPosition != menuVariationsABCD
       && menuPosition != menuPtPlays && menuPosition != menuPtPlaysChain && menuPosition != menuPtNext && menuPosition != menuClockType
       && menuPosition != menuInit && menuPosition != menuPulseOut && menuPosition != menuPulseOutLen && menuPosition != menuProtect
       && menuPosition != menuMIDIinPattern && menuPosition != menuSyncOut && menuPosition != menuShuffle && menuPosition != menuClockDivider)
      || curRightScreen == kRightTrackSelection || curRightScreen == kRightSteps)

  {

    if (curTrack < 8) bitSet(leds[1], curTrack);
    else bitSet(leds[2], curTrack - 8);
  }
  // else leds[1] = leds[2] = 0;
  //
  if (seqPlaying && (curRightScreen != kRightTrackSelection && curRightScreen != kRightMenu)) {
    leds[1] = leds[1] ^ chaseLEDs[0];
    leds[2] = leds[2] ^ chaseLEDs[1];
  }

  //
  // Left Screen ---------------------------------------------------------------------------------------------------------------------- //

  if (curLeftScreen == kLeftMain && forceAccent == false && (curRightScreen != kRightMenu && curRightScreen != kMuteMenu)) {
    resetSegments(0, 2);
    if (editingNoteTranspose != -127 && !editingNote && curRightScreen == kRightSteps && curTrack >= DRUM_TRACKS) {
      if (noteTransposeEditAllSteps) {
        recordEnabled = false;  //auto turn off record mode to show A for all transpose
        segments[0][0] = S_A;   //letter for transpose all
        printNumber(0, 1, editingNoteTranspose);
      } else {
        recordEnabled = false;       //auto turn off record mode to show N for note transpose
        segments[0][0] = B01010100;  //small n for note transpose
        byte holdingNote = 0;
        for (byte x = 0; x < 8; x++) {
          for (byte i = 0; i < 2; i++) {
            if (bitRead(buttonEventWasHolding[i + 1], x)) {
              if (holdingNote == 0) holdingNote = stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][editVariation];
              else {
                holdingNote = 0;
                break;
              }
            }
          }
        }
        //
        if (holdingNote == 0) printNumber(0, 1, editingNoteTranspose);
        else {
          segments[0][2] = 0;
          printNumber(0, 1, holdingNote);
        }
      }
      byte pattern[] = { B01010100, B01010100, S_A, S_A };
      segments[0][7] = pattern[quantizedValue % 4];

    }

    else

    {
      if (globalData.midiClockInternal) {
        segments[0][0] = B00000000;  //definition of first char in normal mode: empty
        printNumber(0, 1, realBPM);
      } else {
        segments[0][0] = B00000000;
        segments[0][1] = S_E;
        segments[0][2] = S_X;
        segments[0][3] = S_T;
      }

      // Determine the segment value for the first position
      if (seqPosition % 4 == 0) {
        if (repeatMode == kRepeatModeNormal) {
          segments[0][0] = B01011100;
        } else if (repeatMode == kRepeatModeChain) {
          segments[0][0] = S_C;
        } else if (repeatMode == kRepeatModePattern) {
          segments[0][0] = S_P;
        }
      }

      // Handle record-enabled states for second, third, and fourth positions
      if (recordEnabled) {
        if (seqPosition % 4 != 0) {
          segments[0][0] = S_R;
        }
      } else {
        if (seqPosition % 4 != 0) {
          segments[0][0] = B00000000;
        }
      }

      if (nextPattern != currentPattern) printNumber(0, 5, nextPattern + 1);
      else printNumber(0, 5, currentPattern + 1);

      if (streamNextPattern || loadPatternNow) {

        // Map `seqPosition` to corresponding segments for the loading animation
        const byte segmentMapping[8] = {
          B00001000,  // First position
          B00010000,  // Second position
          B01000000,  // Third position
          B00000010,  // Fourth position
          B00000001,  // Fifth position
          B00100000,  // Sixth position
          B01000000,  // Seventh position
          B00000100   // Eighth position
        };

        // Assign the segment value based on `seqPosition`
        segments[0][4] = segmentMapping[seqPosition % 8];

      } else {

        byte bufferFill = 64 - Serial.availableForWrite();  // Bytes in use
        byte visualPattern;

        // Determine the visual pattern based on load

        if (bufferFill <= 6) {        // Low load
          visualPattern = B00000000;  // Minimal visual indicator
        }

        else if (bufferFill <= 12) {    // Low load
          visualPattern = B00001000;    // Minimal visual indicator
        } else if (bufferFill <= 45) {  // Mid load
          visualPattern = B01001000;    // Mid-level load pattern
        } else {                        // High load / Emergency
          visualPattern = B01001001;    // Full visual indicator
        }

        segments[0][4] = visualPattern;  // Assign to your display
      }

      byte theBank = nextPatternBank + 1;  // Use nextPatternBank directly, assuming switch is happening
                                           //     if (currentPatternBank != nextPatternBank) theBank = nextPatternBank;
      segments[0][5] = getBankLetter(theBank);
      segments[0][5] |= B10000000;
      //if (showBPMdot) segments[0][5] |= B10000000;  //bpm indicator dot can be changed here
      //if (showBPMdot) segments[0][4] |= B01011100;  //bpm indicator dot can be changed here  FUCKING TOO IMPRECISE, WE USE SEQUENCER STEPS
    }
  }

  // new left (MAIN) screen manipulations based on various parameters

  //__________________________________________________________________________________________________________________________________

  if (curRightScreen == kMuteMenu) {
    segments[0][0] = B00000000;
    segments[0][1] = B00000000;
    segments[0][2] = S_N;
    segments[0][3] = S_U;
    //T letter provided by MuteSolo code, dont ask me why
    segments[0][5] = S_E;
    segments[0][6] = B00000000;

    const byte exitKeyMapping[4] = {
      B01000000,  // First position
      B01001001,  // Second position
      B01111001,  // Third position
      B01001001   // Fourth position
    };

    // Assign the segment value based on `quantizedValue`
    segments[0][7] = exitKeyMapping[quantizedValue % 4];
  }

  //__________________________________________________________________________________________________________________________________

  if (curRightScreen == kRightMenu) {
    segments[0][0] = B00000000;
    segments[0][1] = B00000000;


    segments[0][2] = B00011100;  //DOWN
    segments[0][3] = B00100011;  //UP
    segments[0][4] = B01011000;  //LEFT
    segments[0][5] = B01001100;  //RIGHT

    segments[0][6] = B00000000;

    // Define mapping array for exit key animation
    const byte exitKeyMapping[4] = {
      B01000000,  // First position
      B01001001,  // Second position
      B01111001,  // Third position
      B01001001   // Fourth position
    };

    // Assign the segment value based on `quantizedValue`
    segments[0][7] = exitKeyMapping[quantizedValue % 4];
  }

  //resetSegments( 1, 2);

  //__________________________________________________________________________________________________________________________________

  if (forceAccent == true && curRightScreen != kRightMenu && curLeftScreen == kLeftMain && (segments[0][0] != S_A && segments[0][0] != B01010100)) {  //segments: prevent splash from showing in transpose mode
    segments[0][2] = B00001100;
    segments[0][3] = B00011000;
    segments[0][4] = S_R;
    segments[0][5] = S_E;
    segments[0][6] = B00000000;

    // Define mapping array for ABCD animation
    const byte ABCDMapping[4] = {
      S_A,  // First position
      S_b,  // Second position
      S_C,  // Third position
      S_d   // Fourth position
    };

    // Assign the segment value based on `quantizedValue`
    segments[0][7] = ABCDMapping[quantizedValue % 4];

    if (repeatMode == kRepeatModeNormal) {
      segments[0][0] = B01011100;
      segments[0][1] = B00000000;
    }
    if (repeatMode == kRepeatModeChain) {
      segments[0][0] = S_C;
      segments[0][1] = B00000000;
    }
    if (repeatMode == kRepeatModePattern) {
      segments[0][0] = S_P;
      segments[0][1] = B00000000;
      ;
    }
  }

  //__________________________________________________________________________________________________________________________________

  else if (curLeftScreen == kRandom) {
    segments[0][1] = S_R;
    segments[0][2] = S_A;
    segments[0][3] = S_N;
    segments[0][4] = S_d;
    segments[0][5] = S_O;
    segments[0][6] = S_N;
    segments[0][0] = B00000000;
    segments[0][7] = B00000000;
  }
  //__________________________________________________________________________________________________________________________________
  else if (curLeftScreen == kErase) {
    segments[0][0] = S_E;
    segments[0][1] = S_R;
    segments[0][2] = S_A;
    segments[0][3] = S_S;
    segments[0][4] = S_E;
    segments[0][5] = S_T;
    segments[0][6] = S_R;
    segments[0][7] = S_K;
  }

  //__________________________________________________________________________________________________________________________________

  if (forceAccent == true && curRightScreen != kRightMenu && segments[0][0] == B01010100 && segments[0][2] != S_T) {
    segments[0][0] = S_A;
    segments[0][1] = S_L;
    segments[0][2] = S_L;
    segments[0][3] = B00000000;
    segments[0][4] = S_T;
    segments[0][5] = S_R;
    segments[0][6] = S_S;
    segments[0][7] = S_P;
  }

  else if (forceAccent == true && curRightScreen != kRightMenu && segments[0][0] == S_A && segments[0][1] != S_L) {
    segments[0][0] = B01010100;
    segments[0][1] = S_O;
    segments[0][2] = S_T;
    segments[0][3] = S_E;
    segments[0][4] = S_E;
    segments[0][5] = S_d;
    segments[0][6] = S_I;
    segments[0][7] = S_T;
  }
  //__________________________________________________________________________________________________________________________________

  //
  resetSegments(1, 2);
  //
  if (temporaryMessageCounter > 0) {
    if (showTemporaryMessage == kMemoryProtectMessage) showMemoryProtected();
    else if (showTemporaryMessage == kPatternRepeatMessage) {
      if (repeatMode == kRepeatModeNormal) {
        segments[1][1] = S_N;  // normal playback or normal chain playback
        segments[1][2] = S_O;
        segments[1][3] = S_R;
        segments[1][4] = S_N;
        segments[1][5] = S_A;
        segments[1][6] = S_L;
        //
        segments[2][1] = S_P;
        segments[2][2] = S_L;
        segments[2][3] = S_A;
        segments[2][4] = S_Y;
      } else if (repeatMode == kRepeatModeChain) {
        segments[1][2] = S_C;  // repeat whole chain regardless of chain repeat parameter
        segments[1][3] = S_H;
        segments[1][4] = S_A;
        segments[1][5] = S_I;
        segments[1][6] = S_N;

        segments[2][1] = S_R;
        segments[2][2] = S_E;
        segments[2][3] = S_P;
        segments[2][4] = S_E;
        segments[2][5] = S_A;
        segments[2][6] = S_T;
        //

      } else {
        segments[1][1] = S_O;  // 1 pattern repeat
        segments[1][2] = S_N;
        segments[1][3] = S_E;

        segments[1][5] = S_P;
        segments[1][6] = S_A;
        segments[1][7] = S_T;

        segments[2][1] = S_R;
        segments[2][2] = S_E;
        segments[2][3] = S_P;
        segments[2][4] = S_E;
        segments[2][5] = S_A;
        segments[2][6] = S_T;
      }
    }
    //
    temporaryMessageCounter++;
    if (temporaryMessageCounter >= TEMPORARY_MESSAGE_TIME) temporaryMessageCounter = 0;
    return;
  }

  //
  // MUTE Screen --------------------------------------------------------------------------------------------------------------------- //
  if (curRightScreen == kMuteMenu) {
    createScreenMute();
    return;
  }
  //
  //
  // Right Screen --------------------------------------------------------------------------------------------------------------------- //
  if (curRightScreen == kRightMenuCopyPaste) {
    showMenuCopyPaste();
  } else if (curRightScreen == kRightMenu)  //setup
  {
    showMenu();
  }

  if (curRightScreen == kRightPatternSelection && isSelectingBank) {
    // Final "BANK SELECT" message
    segments[1][2] = S_b;
    segments[1][3] = S_A;
    segments[1][4] = S_N;
    segments[1][5] = S_K;

    segments[2][1] = S_S;
    segments[2][2] = S_E;
    segments[2][3] = S_L;
    segments[2][4] = S_E;
    segments[2][5] = S_C;
    segments[2][6] = S_T;
  }

  else if (curRightScreen == kRightPatternSelection) {
    // Final "PATTERN SELECT" message
    segments[1][1] = S_P;
    segments[1][2] = S_A;
    segments[1][3] = S_T;
    segments[1][4] = S_T;
    segments[1][5] = S_E;
    segments[1][6] = S_R;
    segments[1][7] = S_N;

    segments[2][1] = S_S;
    segments[2][2] = S_E;
    segments[2][3] = S_L;
    segments[2][4] = S_E;
    segments[2][5] = S_C;
    segments[2][6] = S_T;
  }


  else if (curRightScreen == kRightTrackSelection) {
    // Final "TRACK SELECT" message
    segments[1][2] = S_T;
    segments[1][3] = S_R;
    segments[1][4] = S_A;
    segments[1][5] = S_C;
    segments[1][6] = S_K;

    segments[2][1] = S_S;
    segments[2][2] = S_E;
    segments[2][3] = S_L;
    segments[2][4] = S_E;
    segments[2][5] = S_C;
    segments[2][6] = S_T;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  else if (curRightScreen == kRightSteps || curRightScreen == kRightTrackSelection || curRightScreen == kRightPatternSelection) {



#if TRACK_DEBUG_MIDI
    if (curRightScreen == kRightTrackSelection) {
      printNumber2(1, 3, lastMIDIinput[3]);
      printNumber2(1, 7, lastMIDIinput[2]);
      printNumber2(2, 3, lastMIDIinput[1]);
      printNumber2(2, 7, lastMIDIinput[0]);
      return;
    }
#endif

    //
    byte xVar = variation;
    if (forceVariation >= 0) xVar = forceVariation;
    //

    if (curTrack < DRUM_TRACKS) {
      for (byte xs = 0; xs < 2; xs++) {
        for (byte x = 0; x < 8; x++) {
          getStepVelocity(x + (xs * 8), curTrack, xVar, false);
          //
          segments[xs + 1][x] = (char)pgm_read_word(&stepChars[stepVelocity]);
          //
          if (bitRead(stepsData[x + (xs * 8)].stepsDouble[xVar], curTrack)) segments[xs + 1][x] |= B10000000;  //double step char
        }
      }
    } else  // NOTE TRACKS ---===----==---==-- //

      for (byte xs = 0; xs < 2; xs++) {
        for (byte x = 0; x < 8; x++) {
          xm = bitRead(stepsData[x + (xs * 8)].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (xVar * 2)) << 1;
          xm |= bitRead(stepsData[x + (xs * 8)].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (xVar * 2));
          getStepVelocity(x + (xs * 8), curTrack, xVar, false);
          //
          if (xm != 3 && stepsData[x + (xs * 8)].noteSteps[curTrack - DRUM_TRACKS][xVar] > 0 && stepVelocity != 0)
            segments[xs + 1][x] = (char)pgm_read_word(&stepChars[stepVelocity]);
          //
          if (xm == 1) segments[xs + 1][x] |= B00011000;  //set segment for slide/legato
          //else if (xm == 2) segments[xs + 1][x] |= B10000000;  //double step note track - not used
          else if (xm == 3) segments[xs + 1][x] |= B01001001;  //note off
        }
      }
  }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void checkInterface()  //BUTTONS / TACTILES
{
  if (curRightScreen == kMuteMenu)  // MUTE Editing // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  {
    checkInterfaceMute();
    return;
  }
  //
  if (!somethingHappened) return;
  somethingHappened = false;
  //
  //
  // Start Sequencer //
  if (buttonEvent[0][0] >= kButtonClicked) {
    if (buttonEvent[0][0] == kButtonClicked) {
      // Handle short tap
      if (seqPlaying) {
        if (forceAccent) {
          repeatMode++;
          if (repeatMode > kRepeatModePattern) repeatMode = kRepeatModeNormal;
          ShowTemporaryMessage(kPatternRepeatMessage);
          preventABCD = 1;
        } else {
          recordEnabled = !recordEnabled;
        }
      } else if (!seqPlaying && globalData.midiClockInternal) {
        startSequencer();
      }
    } else if (buttonEvent[0][0] == kButtonHold) {
      // Handle long tap
      if (seqPlaying) {
        if (forceAccent) {
          repeatMode++;
          if (repeatMode > kRepeatModePattern) repeatMode = kRepeatModeNormal;
          ShowTemporaryMessage(kPatternRepeatMessage);
          preventABCD = 1;
        } else {
          recordEnabled = !recordEnabled;
        }
      } else if (!seqPlaying && globalData.midiClockInternal) {
        startSequencer();
      }
    }
    buttonEvent[0][0] = 0;  // Reset the button state
  }


  //
  // Stop Sequencer //
  if (buttonEvent[0][1] >= kButtonClicked && forceAccent == false && curRightScreen != kRightMenu) {

    if (globalData.midiClockInternal) {
      recordEnabled = false;
      stopSequencer();
      //segments[0][4] = B00000000;
    }

    buttonEvent[0][1] = 0;
    editingNoteTranspose = -127;
  }
  //
  // (-) BPM or Menu  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  if (buttonEvent[0][2] >= kButtonClicked) {

    if (buttonEvent[0][2] >= kButtonClicked && forceAccent) {
      rotateSteps(true);  // Rotate to the left
      preventABCD = 1;    // Prevent ABCD change
    }
    if (buttonEvent[0][2] == kButtonClicked && forceAccent == false) {
      if (editingNoteTranspose != -127 && curRightScreen == kRightSteps && curTrack >= DRUM_TRACKS) {
        bool foundOne = false;
        for (byte x = 0; x < 8; x++) {
          for (byte i = 0; i < 2; i++) {
            if ((noteTransposeEditAllSteps || bitRead(buttonEventWasHolding[i + 1], x))) {
              for (byte xp = 0; xp < 4; xp++) {
                if (!mirror) xp = editVariation;
                if (stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp] > 0) {
                  stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp]--;
                  patternData.lastNote[curTrack - DRUM_TRACKS] = stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp];
                  //
                  if (!foundOne && editingNoteTranspose > -125) editingNoteTranspose--;
                  foundOne = true;
                }
                //
                if (!mirror) break;
              }
              //
              noteTransposeWasChanged = true;
            }
          }
        }
      } else if (curRightScreen == kRightMenu) {
        processMenu(-1);
      } else {
        if (realBPM > 40) {
          realBPM--;
          //segments[0][4] = B00000000;
          // somethingChangedConfig = true;
        }
        updateSequencerSpeed(false);
      }
    }

    buttonEvent[0][2] = 0;
  }
  // (+) BPM or Menu  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //

  if (buttonEvent[0][3] >= kButtonClicked) {
    if (buttonEvent[0][3] >= kButtonClicked && forceAccent) {
      rotateSteps(false);  // Rotate to the left
      preventABCD = 1;     // Prevent ABCD change
    }


    if (buttonEvent[0][3] == kButtonClicked && forceAccent == false) {

      if (editingNoteTranspose != -127 && curRightScreen == kRightSteps && curTrack >= DRUM_TRACKS) {
        bool foundOne = false;
        for (byte x = 0; x < 8; x++) {
          for (byte i = 0; i < 2; i++) {
            if ((noteTransposeEditAllSteps || bitRead(buttonEventWasHolding[i + 1], x))) {
              for (byte xp = 0; xp < 4; xp++) {
                if (!mirror) xp = editVariation;
                if (stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp] < 127) {
                  stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp]++;
                  patternData.lastNote[curTrack - DRUM_TRACKS] = stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][xp];
                  //
                  if (!foundOne && editingNoteTranspose < 127) editingNoteTranspose++;
                  foundOne = true;
                }
                //
                if (!mirror) break;
              }
              //
              noteTransposeWasChanged = true;
            }
          }
        }
      } else if (curRightScreen == kRightMenu) {
        processMenu(1);
      } else {
        if (realBPM < 1000) {
          realBPM++;

          // somethingChangedConfig = true;
        }
        updateSequencerSpeed(false);
      }
    }


    buttonEvent[0][3] = 0;
  }
  //

  updateScreen = true;
  if (curRightScreen == kRightSteps)  // Regular DRUM Steps Editing // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  {
    byte xVar = variation;
    if (forceVariation >= 0) xVar = forceVariation;
    if (mirror) xVar = 0;
    //
    if (curTrack < DRUM_TRACKS) {
      for (byte x = 0; x < 8; x++) {
        for (byte i = 0; i < 2; i++) {
          if (buttonEvent[i + 1][x] > kButtonNone) {
            getStepVelocity(x + (i * 8), curTrack, xVar, (buttonEvent[i + 1][x] == kButtonClicked));
            //
            if (buttonEvent[i + 1][x] == kButtonClicked) {
              somethingChangedPattern = true;
              //
              if (forceAccent) {
                if (stepVelocity == 0) {
                  stepVelocity = 1;
                  preventABCD = 1;
                } else if (stepVelocity == 1) {
                  stepVelocity = 2;
                  preventABCD = 1;
                } else if (stepVelocity == 2) {
                  stepVelocity = 3;
                  preventABCD = 1;
                } else if (stepVelocity == 3) {
                  stepVelocity = 1;
                  preventABCD = 1;
                }
              } else {
                if (stepVelocity == 0) stepVelocity = 3;
                else stepVelocity = 0;  //which velocity for drumstep without force accent button
              }
              stepsData[x + (i * 8)].steps[curTrack] |= stepVelocity << (xVar * 2);
              //
              if (mirror) {
                stepsData[x + (i * 8)].steps[curTrack] = 0x00;
                stepsData[x + (i * 8)].steps[curTrack] |= stepVelocity;
                stepsData[x + (i * 8)].steps[curTrack] |= stepVelocity << 2;
                stepsData[x + (i * 8)].steps[curTrack] |= stepVelocity << 4;
                stepsData[x + (i * 8)].steps[curTrack] |= stepVelocity << 6;
              }
            } else if (buttonEvent[i + 1][x] == kButtonHold) {
              somethingChangedPattern = true;
              //
              if (bitRead(stepsData[x + (i * 8)].stepsDouble[xVar], curTrack)) {
                bitClear(stepsData[x + (i * 8)].stepsDouble[xVar], curTrack);
                if (mirror) {
                  bitClear(stepsData[x + (i * 8)].stepsDouble[1], curTrack);
                  bitClear(stepsData[x + (i * 8)].stepsDouble[2], curTrack);
                  bitClear(stepsData[x + (i * 8)].stepsDouble[3], curTrack);
                }
              } else {
                bitSet(stepsData[x + (i * 8)].stepsDouble[xVar], curTrack);
                if (mirror) {
                  bitSet(stepsData[x + (i * 8)].stepsDouble[1], curTrack);
                  bitSet(stepsData[x + (i * 8)].stepsDouble[2], curTrack);
                  bitSet(stepsData[x + (i * 8)].stepsDouble[3], curTrack);
                }
              }
            }
            //
            buttonEvent[i + 1][x] = 0;
            updateScreen = true;
          }
        }
      }
    } else  // NOTE TRACKS  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
    {
      for (byte x = 0; x < 8; x++) {
        for (byte i = 0; i < 2; i++) {
          if (buttonEvent[i + 1][x] > kButtonNone) {
            //Note NEW // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //           {
            somethingChangedPattern = true;
            //
            editStep = x + (i * 8);
            editVariation = xVar;
            getStepVelocity(editStep, curTrack, editVariation, false);





            if ((buttonEvent[i + 1][x] == kButtonClicked || buttonEvent[i + 1][x] == kButtonHold) && (editingNoteTranspose == -127)) {
              // Allow longer hold on note steps
              if (stepsData[editStep].noteSteps[curTrack - DRUM_TRACKS][editVariation] == 0 && patternData.lastNote[curTrack - DRUM_TRACKS] == 0) {
                clearStepsExtrasBits(editStep, editVariation, curTrack - DRUM_TRACKS);
                editingNote = true;
                patternData.lastNote[curTrack - DRUM_TRACKS] = 60;
                stepsData[editStep].noteSteps[curTrack - DRUM_TRACKS][editVariation] = patternData.lastNote[curTrack - DRUM_TRACKS];
                stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0] |= lastVelocity << (editVariation * 2);
              } else if (stepsData[editStep].noteSteps[curTrack - DRUM_TRACKS][editVariation] == 0 && patternData.lastNote[curTrack - DRUM_TRACKS] > 0) {
                clearStepsExtrasBits(editStep, editVariation, curTrack - DRUM_TRACKS);
                stepsData[editStep].noteSteps[curTrack - DRUM_TRACKS][editVariation] = patternData.lastNote[curTrack - DRUM_TRACKS];
                stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0] |= lastVelocity << (editVariation * 2);
              } else if (forceAccent) {
                if (buttonEvent[i + 1][x] == kButtonHold) {
                  // Handle slide toggle on long hold with forceAccent
                  byte slideState = (bitRead(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2)) | (bitRead(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2)) << 1));

                  if (slideState == 1) {  // If currently slide, switch to note off
                                          // bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2));
                    //bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2));
                    bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 1 + (editVariation * 2));
                    bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 0 + (editVariation * 2));
                    bitSet(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2));
                    bitSet(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2));


                    preventABCD = 1;
                  } else {  // Otherwise, set it as slide
                    bitSet(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2));
                    bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2));  // Ensure it's only slide
                    preventABCD = 1;
                  }
                } else {  // Cycle velocities only for short or mid hold with forceAccent
                  getStepVelocity(editStep, curTrack, editVariation, true);
                  getNoteStepGlideDoubleOff(editStep, curTrack - DRUM_TRACKS, editVariation, true);

                  if (stepVelocity == 0) {
                    stepVelocity = 1;
                    preventABCD = 1;
                  } else if (stepVelocity == 1) {
                    stepVelocity = 2;
                    preventABCD = 1;
                  } else if (stepVelocity == 2) {
                    stepVelocity = 3;
                    preventABCD = 1;
                  } else if (stepVelocity == 3) {
                    stepVelocity = 1;
                    preventABCD = 1;
                  }

                  lastVelocity = stepVelocity;
                  stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0] |= stepVelocity << (editVariation * 2);
                  stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1] |= noteStepGlideDoubleOff << (editVariation * 2);
                }

                checkIfMirrorAndCopy(editStep, curTrack - DRUM_TRACKS);
              } else if (stepsData[editStep].noteSteps[curTrack - DRUM_TRACKS][editVariation] > 0) {
                getNoteStepGlideDoubleOff(editStep, curTrack - DRUM_TRACKS, editVariation, false);
                if (noteStepGlideDoubleOff == 3) {
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2));
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2));
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 1 + (editVariation * 2));
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 0 + (editVariation * 2));
                }

                if (stepVelocity == 0) {  // set step MAXIMUM VELOCITY
                  bitSet(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 1 + (editVariation * 2));
                  bitSet(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 0 + (editVariation * 2));
                } else {  // clear step
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 1 + (editVariation * 2));
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][0], 0 + (editVariation * 2));
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 0 + (editVariation * 2));  // clear slide/note off
                  bitClear(stepsData[editStep].noteStepsExtras[curTrack - DRUM_TRACKS][1], 1 + (editVariation * 2));  // clear slide/note off
                }
              }
            }
            //
            checkIfMirrorAndCopy(editStep, curTrack - DRUM_TRACKS);
          }
          // Reset transpose on button release
          if (buttonEvent[i + 1][x] == kButtonRelease) {
            editingNoteTranspose = -127;
          }
          // Reset button events
          buttonEvent[i + 1][x] = 0;
          updateScreen = true;
        }
      }
    }
  }



  //
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
  //
  if (curRightScreen == kRightTrackSelection || curRightScreen == kRightPatternSelection || curRightScreen == kRightMenuCopyPaste || curRightScreen == kRightMenu) {
    char leButton = -1;
    for (byte x = 0; x < 8; x++) {
      for (byte i = 0; i < 2; i++) {
        if (buttonEvent[i + 1][x] >= kButtonClicked || buttonEvent[i + 1][x] == kButtonHold || buttonEvent[i + 1][x] >= kButtonHold) {
          buttonEvent[i + 1][x] = 0;
          updateScreen = true;
          leButton = x + (i * 8);
        }
      }
    }
    //
    if (leButton >= 0) {
      if (curRightScreen == kRightMenu && menuPosition == menuInit) {
        processMenuOK();
      } else if (curRightScreen == kRightTrackSelection || curRightScreen == kRightMenu) {
        editingNote = false;
        curTrack = leButton;
      }

      else if (curRightScreen == kRightMenuCopyPaste) {
        processMenuCopyPaste(leButton);
      } else if (curRightScreen == kRightPatternSelection) {


        if (isSelectingBank) {

          // Bank selection logic
          bitSet(patternBitsSelector, leButton);  // Set the bit for bank selection
          nextPatternBank = leButton;
          nextPattern = 0;  // Reset pattern to 0 when switching banks
        } else {

          // Pattern selection logic
          bitSet(patternBitsSelector, leButton);  // Set the bit for pattern selection
          nextPattern = leButton;
        }

        // Load pattern only if there's a change
        if (currentPattern != nextPattern || nextPatternBank != currentPatternBank) {
          loadPattern(nextPattern);  // Load the selected pattern from the correct bank
        }
      }
    }
  }

  //
  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------


  // Combo 1: Bank + Pattern = Setup Menu
  if ((isSelectingBank && buttonEvent[0][5] == kButtonHold) || (!isSelectingBank && curRightScreen == kRightPatternSelection && buttonEvent[0][4] == kButtonHold) || (buttonEvent[0][4] == kButtonHold && buttonEvent[0][5] == kButtonHold)) {

    curRightScreen = kRightMenu;  // Activate Setup Menu
    initMode = 0;                 // menuPosition = 0; - lets always jump back to the last used menu option
    updateScreen = true;
    buttonEvent[0][4] = buttonEvent[0][5] = 0;  // Reset button events
    isSelectingBank = false;                    // Exit bank selection mode after combo

  }
  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Combo 2: Pattern + Track = Copy Menu
  else if (((!isSelectingBank && curRightScreen == kRightPatternSelection && buttonEvent[0][6] == kButtonHold) || (curRightScreen == kRightTrackSelection && buttonEvent[0][5] == kButtonHold) || (buttonEvent[0][5] == kButtonHold && buttonEvent[0][6] == kButtonHold)) && curRightScreen != kRightMenu)

  {
    curRightScreen = kRightMenuCopyPaste;  // Copy Menu
    updateScreen = true;
    buttonEvent[0][5] = buttonEvent[0][6] = 0;  // Reset button events
  }
  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Combo 3: Track + Shift = Mute Menu
  else if ((curRightScreen == kRightTrackSelection && buttonEvent[0][7] == kButtonHold) || (forceAccent && buttonEvent[0][6] == kButtonHold) || (buttonEvent[0][6] == kButtonHold && buttonEvent[0][7] == kButtonHold)) {

    curRightScreen = kMuteMenu;  // Mute Screen
    updateScreen = true;
    forceAccent = false;
    isSelectingBank = false;                    // Reset bank selection flag if active
    preventABCD = 0;                            // use variable to debounce
    buttonEvent[0][6] = buttonEvent[0][7] = 0;  // Reset button events
  }


  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Logic for reading single buttons 4 (Bank), 5 (Pattern), 6 (Track), 7 (Shift/ABCD/Velocity/Transpose)

  // Button 4: Bank Selection
  if (curRightScreen != kMuteMenu) {  // Ensure action only if not in mute screen
    if (curRightScreen == kRightMenu) {
      // In setup menu, Button 4 decreases setup option
      if (buttonEvent[0][4] == kButtonHold) {
        if (menuPosition > menuFirst) menuPosition--;
        else menuPosition = lastMenu;
        initMode = 0;
        buttonEvent[0][4] = 0;
        updateScreen = true;
      }
    } else {
      // Normal bank selection logic
      if (buttonEvent[0][4] == kButtonHold && forceAccent == true) {
        randomTrack();
        curLeftScreen = kRandom;
        updateScreen = true;
      }

      else if (buttonEvent[0][4] == kButtonHold) {
        isSelectingBank = true;
        curRightScreen = kRightPatternSelection;
        buttonEvent[0][4] = 0;
        updateScreen = true;
      } else if (buttonEvent[0][4] > kButtonHold || buttonEvent[0][4] == kButtonClicked) {
        isSelectingBank = false;
        curRightScreen = kRightSteps;
        curLeftScreen = kLeftMain;
        buttonEvent[0][4] = 0;
        preventABCD = 1;
        updateScreen = true;
      }
    }
  }
  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Button 5: Pattern Selection
  if (curRightScreen != kMuteMenu) {  // Ensure action only if not in mute screen
    if (curRightScreen == kRightMenu) {
      // In setup menu, Button 5 increases setup option
      if (buttonEvent[0][5] == kButtonHold) {
        menuPosition++;
        initMode = 0;
        if (menuPosition > lastMenu) menuPosition = menuFirst;
        buttonEvent[0][5] = 0;
        updateScreen = true;
      }
    } else {
      // Normal pattern selection logic
      if (buttonEvent[0][5] == kButtonHold && forceAccent == true) {
        eraseTrack();
        curLeftScreen = kErase;
        updateScreen = true;
      }

      else if (buttonEvent[0][5] == kButtonHold) {
        isSelectingBank = false;
        curRightScreen = kRightPatternSelection;
        buttonEvent[0][5] = 0;
        updateScreen = true;
      } else if (buttonEvent[0][5] > kButtonHold || buttonEvent[0][5] == kButtonClicked) {
        curRightScreen = kRightSteps;
        curLeftScreen = kLeftMain;
        buttonEvent[0][5] = 0;
        preventABCD = 1;
        updateScreen = true;
      }
    }
  }
  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Button 6: Track Selection
  if (curRightScreen != kMuteMenu && curRightScreen != kRightMenu) {  // Ensure action only if not in mute or setup menu
    if (buttonEvent[0][6] == kButtonHold) {
      curRightScreen = kRightTrackSelection;
      buttonEvent[0][6] = 0;
      updateScreen = true;
    } else if (buttonEvent[0][6] > kButtonHold || buttonEvent[0][6] == kButtonClicked) {
      curRightScreen = kRightSteps;
      curLeftScreen = kLeftMain;
      buttonEvent[0][6] = 0;
      updateScreen = true;
    }
  }

  //-===========-------===========-------===========-------===========-------===========-------===========-------===========-------===========-------

  // Button 7: Shift/ABCD/Exit

  if (buttonEvent[0][7] == kButtonHold)  // Button 7 logic (Shift/ABCD/Exit)

  {
    buttonEvent[0][7] = 0;

    if (preventABCD == 1 && curRightScreen == kRightSteps) preventABCD = 0;

    forceAccent = true;
    isSelectingBank = false;
    updateScreen = true;
  } else if (buttonEvent[0][7] > kButtonHold || buttonEvent[0][7] == kButtonClicked)  // Button release
  {
    forceAccent = false;
    isSelectingBank = false;
    buttonEvent[0][7] = 0;
    curLeftScreen = kLeftMain;
    updateScreen = true;

    if (curRightScreen == kRightMenu)  //replacement for checkMenuClose()
    {
      curRightScreen = kRightSteps;
      preventABCD = 1;
    }
    if (editingNoteTranspose != -127) {
      noteTransposeEditAllSteps = !noteTransposeEditAllSteps;
    }

    else if (preventABCD == 0)  // ABCD variation flag check
    {
      if (buttonEvent[0][6] != kButtonHold)  // Skip ABCD logic if both buttons were pressed simultaneously (i.e., mute screen was triggered)
      {
        if (patternData.totalVariations == 1) {
          mirror = false;  // Only one variation (A), so disable mirror mode
          forceVariation = 0;
        }
        // Cycle through edit modes: !mirror, mirror, only A, only B, only C, only D
        else if (forceVariation == -1) {
          if (mirror) {
            mirror = false;  // Exit mirror mode and start cycling ABCD
            forceVariation = 0;
          } else {
            mirror = true;
          }
        } else if (patternData.totalVariations > 1) {
          // Cycle through forceVariation
          forceVariation++;  // A=0, B=1, C=2, D=3
          if (forceVariation == patternData.totalVariations) {
            forceVariation = -1;  //ABCD cycling finished, toggle back to mirror mode
          }
        }
      }
    }
  }
}





// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

inline void checkIfMirrorAndCopy(byte thestep, byte track) {
  if (mirror) {
    stepsData[thestep].noteSteps[track][1] = stepsData[thestep].noteSteps[track][0];
    stepsData[thestep].noteSteps[track][2] = stepsData[thestep].noteSteps[track][0];
    stepsData[thestep].noteSteps[track][3] = stepsData[thestep].noteSteps[track][0];
    //
    stepsData[thestep].noteStepsExtras[track][0] &= B00000011;
    stepsData[thestep].noteStepsExtras[track][0] |= stepsData[thestep].noteStepsExtras[track][0] << 2;
    stepsData[thestep].noteStepsExtras[track][0] |= stepsData[thestep].noteStepsExtras[track][0] << 4;
    stepsData[thestep].noteStepsExtras[track][0] |= stepsData[thestep].noteStepsExtras[track][0] << 6;
    //
    stepsData[thestep].noteStepsExtras[track][1] &= B00000011;
    stepsData[thestep].noteStepsExtras[track][1] |= stepsData[thestep].noteStepsExtras[track][1] << 2;
    stepsData[thestep].noteStepsExtras[track][1] |= stepsData[thestep].noteStepsExtras[track][1] << 4;
    stepsData[thestep].noteStepsExtras[track][1] |= stepsData[thestep].noteStepsExtras[track][1] << 6;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void clearStepsExtrasBits(byte thestep, byte xVar, byte track) {
  bitClear(stepsData[thestep].noteStepsExtras[track][0], (editVariation * 2));
  bitClear(stepsData[thestep].noteStepsExtras[track][0], (editVariation * 2) + 1);
  bitClear(stepsData[thestep].noteStepsExtras[track][1], (editVariation * 2));
  bitClear(stepsData[thestep].noteStepsExtras[track][1], (editVariation * 2) + 1);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetSegments(byte xs, byte xe) {
  for (byte xx = xs; xx <= xe; xx++) { memset(segments[xx], 0, sizeof(segments[0])); }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void printStopSequencer() {
  leds[0] = leds[1] = leds[2] = 0;
  resetSegments(1, 2);
  segments[1][4] = S_S;
  segments[1][5] = S_T;
  segments[1][6] = S_O;
  segments[1][7] = S_P;
  //
  segments[2][0] = S_S;
  segments[2][1] = S_E;
  segments[2][2] = S_O;
  segments[2][3] = S_U;
  segments[2][4] = S_E;
  segments[2][5] = S_N;
  segments[2][6] = S_C;
  segments[2][7] = S_R;
  //
  sendScreen();
  waitMs(2000);
  updateScreen = true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void getStepVelocity(byte theStep, byte track, byte variation, bool cleanBits) {
  if (track < DRUM_TRACKS) {
    stepVelocity = bitRead(stepsData[theStep].steps[track], 1 + (variation * 2)) << 1;
    stepVelocity |= bitRead(stepsData[theStep].steps[track], 0 + (variation * 2));
  } else {
    stepVelocity = bitRead(stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0], 1 + (variation * 2)) << 1;
    stepVelocity |= bitRead(stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0], 0 + (variation * 2));
  }
  //
  if (cleanBits) {
    if (track < DRUM_TRACKS) {
      bitClear(stepsData[theStep].steps[track], 1 + (variation * 2));
      bitClear(stepsData[theStep].steps[track], 0 + (variation * 2));
    } else {
      bitClear(stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0], (variation * 2));
      bitClear(stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0], (variation * 2) + 1);
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void getNoteStepGlideDoubleOff(byte theStep, byte track, byte variation, bool cleanBits) {
  noteStepGlideDoubleOff = bitRead(stepsData[theStep].noteStepsExtras[track][1], 1 + (variation * 2)) << 1;
  noteStepGlideDoubleOff |= bitRead(stepsData[theStep].noteStepsExtras[track][1], 0 + (variation * 2));
  //
  if (cleanBits) {
    bitClear(stepsData[theStep].noteStepsExtras[track][1], (variation * 2));
    bitClear(stepsData[theStep].noteStepsExtras[track][1], (variation * 2) + 1);
  }
}
// ---------------------------------------------------------------------------------------------------------------------------------------

void rotateSteps(bool rotateLeft) {
  byte noteTrackIndex = curTrack - DRUM_TRACKS;  // Note track index
  byte buffer, bufferExtras, bufferDouble;       // Buffers for rotating steps

  // Determine loop range based on mode
  byte startVar, endVar;
  startVar = 0;
  endVar = patternData.totalVariations - 1;

  for (byte var = startVar; var <= endVar; var++) {
    if (curTrack < DRUM_TRACKS) {
      // Drum track rotation
      buffer = rotateLeft ? stepsData[0].steps[curTrack] & (0b11 << (var * 2)) : stepsData[STEPS - 1].steps[curTrack] & (0b11 << (var * 2));
      bufferDouble = rotateLeft ? (stepsData[0].stepsDouble[var] & (1 << curTrack)) : (stepsData[STEPS - 1].stepsDouble[var] & (1 << curTrack));

      for (byte step = 0; step < STEPS - 1; step++) {
        if (rotateLeft) {
          stepsData[step].steps[curTrack] =
            (stepsData[step + 1].steps[curTrack] & (0b11 << (var * 2))) | (stepsData[step].steps[curTrack] & ~(0b11 << (var * 2)));
          stepsData[step].stepsDouble[var] =
            (stepsData[step + 1].stepsDouble[var] & (1 << curTrack)) | (stepsData[step].stepsDouble[var] & ~(1 << curTrack));
        } else {
          stepsData[STEPS - 1 - step].steps[curTrack] =
            (stepsData[STEPS - 2 - step].steps[curTrack] & (0b11 << (var * 2))) | (stepsData[STEPS - 1 - step].steps[curTrack] & ~(0b11 << (var * 2)));
          stepsData[STEPS - 1 - step].stepsDouble[var] =
            (stepsData[STEPS - 2 - step].stepsDouble[var] & (1 << curTrack)) | (stepsData[STEPS - 1 - step].stepsDouble[var] & ~(1 << curTrack));
        }
      }
      if (rotateLeft) {
        stepsData[STEPS - 1].steps[curTrack] =
          (buffer & (0b11 << (var * 2))) | (stepsData[STEPS - 1].steps[curTrack] & ~(0b11 << (var * 2)));
        stepsData[STEPS - 1].stepsDouble[var] =
          (bufferDouble & (1 << curTrack)) | (stepsData[STEPS - 1].stepsDouble[var] & ~(1 << curTrack));
      } else {
        stepsData[0].steps[curTrack] =
          (buffer & (0b11 << (var * 2))) | (stepsData[0].steps[curTrack] & ~(0b11 << (var * 2)));
        stepsData[0].stepsDouble[var] =
          (bufferDouble & (1 << curTrack)) | (stepsData[0].stepsDouble[var] & ~(1 << curTrack));
      }
    } else {
      // Note track rotation
      buffer = rotateLeft ? stepsData[0].noteSteps[noteTrackIndex][var] : stepsData[STEPS - 1].noteSteps[noteTrackIndex][var];
      bufferExtras = rotateLeft ? stepsData[0].noteStepsExtras[noteTrackIndex][var] : stepsData[STEPS - 1].noteStepsExtras[noteTrackIndex][var];

      for (byte step = 0; step < STEPS - 1; step++) {
        if (rotateLeft) {
          stepsData[step].noteSteps[noteTrackIndex][var] =
            stepsData[step + 1].noteSteps[noteTrackIndex][var];
          stepsData[step].noteStepsExtras[noteTrackIndex][var] =
            stepsData[step + 1].noteStepsExtras[noteTrackIndex][var];
        } else {
          stepsData[STEPS - 1 - step].noteSteps[noteTrackIndex][var] =
            stepsData[STEPS - 2 - step].noteSteps[noteTrackIndex][var];
          stepsData[STEPS - 1 - step].noteStepsExtras[noteTrackIndex][var] =
            stepsData[STEPS - 2 - step].noteStepsExtras[noteTrackIndex][var];
        }
      }

      // Handle wraparound
      if (rotateLeft) {
        stepsData[STEPS - 1].noteSteps[noteTrackIndex][var] = buffer;
        stepsData[STEPS - 1].noteStepsExtras[noteTrackIndex][var] = bufferExtras;
      } else {
        stepsData[0].noteSteps[noteTrackIndex][var] = buffer;
        stepsData[0].noteStepsExtras[noteTrackIndex][var] = bufferExtras;
      }
    }
  }

  // Mark pattern as changed
  somethingChangedPattern = true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//RND - new function, deemed to be OK 23 nov 2024
void randomTrack() {
  //byte track = curTrack; // Use curTrack as the active track
  byte noteTrackIndex = curTrack - DRUM_TRACKS;  // Adjust for note track memory
  byte lastNote = (curTrack >= DRUM_TRACKS) ? patternData.lastNote[noteTrackIndex] : 0;
  byte targetVariation = (forceVariation >= 0 && forceVariation <= 3) ? forceVariation : 0;  // Adjusted range

  for (byte step = 0; step < STEPS; step++) {
    if (random(0, 2) == 0) continue;  // 50% chance to skip this step

    if (curTrack < DRUM_TRACKS) {
      // Drum Tracks Randomization
      for (byte var = 0; var < 4; var++) {
        // Mirror Mode: Generate only Variation A, others will be mirrored
        if (mirror && var != 0) continue;

        // Single Variation Mode: Generate only the selected variation
        if (!mirror && forceVariation >= 0 && var != targetVariation) continue;

        // Serial Mode: Generate for all variations when forceVariation == -1
        if ((stepsData[step].steps[curTrack] & (0b11 << (var * 2))) == 0) {
          byte randomVelocity = random(1, 4);                              // Random value for velocity (1-3)
          stepsData[step].steps[curTrack] |= randomVelocity << (var * 2);  // Set velocity

          // 21% chance to add a 32nd-note step
          bool doubleStep = (random(0, 21) == 0);
          if (doubleStep) {
            stepsData[step].stepsDouble[var] |= (1 << curTrack);  // Set 32nd note flag
          }
        }
      }
    } else {
      // Note Tracks Randomization
      for (byte var = 0; var < 4; var++) {
        // Mirror Mode: Generate only Variation A, others will be mirrored
        if (mirror && var != 0) continue;

        // Single Variation Mode: Generate only the selected variation
        if (!mirror && forceVariation >= 0 && var != targetVariation) continue;

        // Serial Mode: Generate for all variations when forceVariation == -1
        if ((stepsData[step].noteStepsExtras[noteTrackIndex][0] & (0b11 << (var * 2))) == 0) {
          char randomNote = lastNote + random(-12, 13);  // Random pitch within +/-12 semitones
          byte randomVelocity = random(1, 4);            // Random value for velocity (1-3)
          stepsData[step].noteSteps[noteTrackIndex][var] = randomNote;
          stepsData[step].noteStepsExtras[noteTrackIndex][0] &= ~(0b11 << (var * 2));         // Clear velocity bits
          stepsData[step].noteStepsExtras[noteTrackIndex][0] |= randomVelocity << (var * 2);  // Set velocity
        }
      }
    }
  }

  // Mirror Logic
  if (mirror) {
    for (byte step = 0; step < STEPS; step++) {
      if (curTrack < DRUM_TRACKS) {
        // Mirror velocity and 32nd-note flags for drum tracks
        byte baseVelocity = stepsData[step].steps[curTrack] & 0b11;  // Base velocity from A
        for (byte var = 1; var < 4; var++) {
          stepsData[step].steps[curTrack] &= ~(0b11 << (var * 2));       // Clear existing velocity
          stepsData[step].steps[curTrack] |= baseVelocity << (var * 2);  // Mirror velocity
          if (stepsData[step].stepsDouble[0] & (1 << curTrack)) {
            stepsData[step].stepsDouble[var] |= (1 << curTrack);  // Mirror 32nd note flag
          } else {
            stepsData[step].stepsDouble[var] &= ~(1 << curTrack);  // Clear 32nd note flag
          }
        }
      } else {
        // Mirror Note Tracks
        stepsData[step].noteSteps[noteTrackIndex][1] = stepsData[step].noteSteps[noteTrackIndex][0];
        stepsData[step].noteSteps[noteTrackIndex][2] = stepsData[step].noteSteps[noteTrackIndex][0];
        stepsData[step].noteSteps[noteTrackIndex][3] = stepsData[step].noteSteps[noteTrackIndex][0];

        // Copy velocity (Extras)
        stepsData[step].noteStepsExtras[noteTrackIndex][0] &= 0b11;
        stepsData[step].noteStepsExtras[noteTrackIndex][0] |= stepsData[step].noteStepsExtras[noteTrackIndex][0] << 2;
        stepsData[step].noteStepsExtras[noteTrackIndex][0] |= stepsData[step].noteStepsExtras[noteTrackIndex][0] << 4;
        stepsData[step].noteStepsExtras[noteTrackIndex][0] |= stepsData[step].noteStepsExtras[noteTrackIndex][0] << 6;
      }
    }
  }

  // Mark pattern as changed
  somethingChangedPattern = true;
}


// -------------------------------------------------------------------------------------------------------------------

void eraseTrack() {
  //byte track = curTrack; // Use curTrack as the active track

  for (byte step = 0; step < STEPS; step++) {
    if (curTrack < DRUM_TRACKS) {
      // Erase drum track: Clear all velocity and double-step (32nd-note) flags
      stepsData[step].steps[curTrack] = 0x00;  // Clear all velocity bits
      for (byte var = 0; var < 4; var++) {
        stepsData[step].stepsDouble[var] &= ~(1 << curTrack);  // Clear 32nd note flag
      }
    } else {
      // Erase note track: Clear all notes, velocity, and extra flags
      byte noteTrackIndex = curTrack - DRUM_TRACKS;
      for (byte var = 0; var < 4; var++) {
        stepsData[step].noteSteps[noteTrackIndex][var] = 0;  // Clear note values
      }
      stepsData[step].noteStepsExtras[noteTrackIndex][0] = 0x00;  // Clear velocity bits
      stepsData[step].noteStepsExtras[noteTrackIndex][1] = 0x00;  // Clear extra flags (e.g., glide/double)
    }
  }

  // Mark pattern as changed
  somethingChangedPattern = true;
}


// -----------------------------------------------------------------------------------------------------------------------------
char stepBeenHold() {
  char holdingNote = -2;
  for (byte x = 0; x < 8; x++) {
    for (byte i = 0; i < 2; i++) {
      if (bitRead(buttonEventWasHolding[i + 1], x)) {
        if (holdingNote == -2) holdingNote = stepsData[x + (i * 8)].noteSteps[curTrack - DRUM_TRACKS][editVariation];
        else return -1;
      }
    }
  }
  return holdingNote;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ShowTemporaryMessage(byte message) {
  showTemporaryMessage = message;
  temporaryMessageCounter = 1;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showOnOrOff(bool showOn) {
  if (showOn) {
    segments[2][6] = S_O;
    segments[2][7] = S_N;
  } else {
    segments[2][5] = S_O;
    segments[2][6] = S_F;
    segments[2][7] = S_F;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showMemoryProtected() {
  segments[1][0] = S_P;
  segments[1][1] = S_R;
  segments[1][2] = S_O;
  segments[1][3] = S_T;
  segments[1][4] = S_E;
  segments[1][5] = S_C;
  segments[1][6] = S_T;
  //
  showOnOrOff(globalData.writeProtectFlash);
}

//==================================================================================================================================


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
char getBankLetter(byte value) {
  if (value <= 9) return (char)pgm_read_word(&numbers[value]);
  else if (value == 10) return S_A;
  else if (value == 11) return S_b;
  else if (value == 12) return S_C;
  else if (value == 13) return S_d;
  else if (value == 14) return S_E;
  else if (value == 15) return S_F;
  else return S_G;
}

//END OF LINE
