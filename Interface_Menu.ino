/*

   Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com

*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showMenuCopyPaste() {
  // 12341234 12341234
  // PAT COPY PST INIT
  //
  segments[1][0] = S_P;
  segments[1][1] = S_A;
  segments[1][2] = S_T;
  //
  segments[1][4] = S_C;
  segments[1][5] = S_O;
  segments[1][6] = S_P;
  segments[1][7] = S_Y;
  //
  segments[2][0] = S_P;
  segments[2][1] = S_S;
  segments[2][2] = S_T;
  segments[2][3] = S_E;
  //
  segments[2][4] = B00000000;
  segments[2][5] = S_C;
  segments[2][6] = S_L;
  segments[2][7] = S_R;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void processMenuCopyPaste(byte button) {
  if (button >= 4 && button <= 7)  // Copy
  {
    pagePos = 16 + (PT_BANKS * ((64 * 16) + 16));
    eraseSector(pagePos);
    if (!flash.writeAnything(pagePos, (uint8_t)0, patternData)) showErrorMsg(flash.error());
    pagePos++;
    if (!flash.writeAnything(pagePos, (uint8_t)0, stepsData)) showErrorMsg(flash.error());

  } else if (button >= 8 && button <= 11)  // Paste
  {
    pagePos = 16 + (PT_BANKS * ((64 * 16) + 16));
    if (!flash.readAnything(pagePos, (uint8_t)0, patternData)) showErrorMsg(flash.error());
    pagePos++;
    if (!flash.readAnything(pagePos, (uint8_t)0, stepsData)) showErrorMsg(flash.error());
    somethingChangedPattern = true;
  } else if (button >= 13 && button <= 15)  // Init
  {
    for (byte xs = 0; xs < STEPS; xs++) { stepsData[xs].init(); }
    somethingChangedPattern = true;
  }
  //
  curRightScreen = kRightSteps;
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showMenu() {
  switch (menuPosition) {

    case menuTrackLen:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = S_b;  //
      segments[0][1] |= B10000000;
      segments[1][0] = S_T;
      segments[1][1] = S_R;
      segments[1][2] = S_A;
      segments[1][3] = S_K;
      //
      segments[1][4] = S_L;
      segments[1][5] = S_E;
      segments[1][6] = S_N;
      //
      printNumber(2, 5, patternData.trackLen[curTrack]);
      break;
      //

    case menuTrackProbability:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = S_E;  //
      segments[0][1] |= B10000000;
      segments[1][0] = S_T;
      segments[1][1] = S_R;
      segments[1][2] = S_A;
      segments[1][3] = S_K;
      //
      segments[1][4] = S_S;
      segments[1][5] = S_T;
      segments[1][6] = S_E;
      segments[1][7] = S_P;
      segments[2][0] = S_S;
      segments[2][1] = S_K;
      segments[2][2] = S_I;
      segments[2][3] = S_P;
      //
      printNumber(2, 5, patternData.trackProbability[curTrack]);
      break;
      //

    case menuShuffle:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = S_d;  //
      segments[0][1] |= B10000000;
      segments[1][0] = S_S;
      segments[1][1] = S_H;
      segments[1][2] = S_U;
      segments[1][3] = S_F;
      segments[1][4] = S_F;
      segments[1][5] = S_L;
      segments[1][6] = S_E;

      //
      printNumber(2, 5, patternData.shuffleDelay);
      break;
      //

    case menuProgramChange:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B11101101;  // menuProgramChange
      segments[1][0] = S_P;
      segments[1][1] = S_R;
      segments[1][2] = S_O;
      segments[1][3] = S_G;
      segments[1][4] = S_C;
      segments[1][5] = S_H;
      segments[1][6] = S_N;
      segments[1][7] = S_G;

      if (patternData.programChange[curTrack] > 0) printNumber(2, 5, patternData.programChange[curTrack]);
      else showOnOrOff(false);  //trying to fix program number to 1 minimum - redux edit 2
      break;

    case menuMIDIChannel:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B11100110;  // menuMIDIChannel
      segments[1][0] = S_N;
      segments[1][1] = S_I;
      segments[1][2] = S_d;
      segments[1][3] = S_I;
      //
      segments[1][5] = S_C;
      segments[1][6] = S_H;
      segments[1][7] = S_N;
      //
      printNumber(2, 5, patternData.trackMidiCH[curTrack] + 1);
      break;
      //
    case menuPtPlays:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B10000110;  // menuPtPlays
      segments[1][0] = S_P;
      segments[1][1] = S_A;
      segments[1][2] = S_T;
      //
      segments[1][4] = S_R;
      segments[1][5] = S_E;
      segments[1][6] = S_P;
      segments[1][7] = S_E;
      segments[2][0] = S_A;
      segments[2][1] = S_T;

      //
      if (patternData.playsPattern == 0) {
        segments[2][5] = S_I;
        segments[2][6] = S_N;
        segments[2][7] = S_F;
      } else printNumber(2, 5, patternData.playsPattern);
      break;
    //
    case menuPtPlaysChain:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B11001111;  // menuPtPlaysChain
      segments[1][0] = S_C;
      segments[1][1] = S_H;
      segments[1][2] = S_A;
      segments[1][3] = S_I;
      segments[1][4] = S_N;
      //
      segments[1][6] = S_R;
      segments[1][7] = S_E;
      segments[2][0] = S_P;
      segments[2][1] = S_E;
      segments[2][2] = S_A;
      segments[2][3] = S_T;

      //
      if (patternData.playsChain == 0) {
        segments[2][5] = S_I;
        segments[2][6] = S_N;
        segments[2][7] = S_F;
      } else if (patternData.playsChain == 1) showOnOrOff(false);
      else printNumber(2, 5, patternData.playsChain - 1);
      break;
    //
    case menuPtNext:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B11011011;  // menuPtNext
      segments[1][0] = S_N;
      segments[1][1] = S_E;
      segments[1][2] = S_X;
      segments[1][3] = S_T;
      //

      segments[1][5] = S_P;
      segments[1][6] = S_A;
      segments[1][7] = S_T;
      //
      if (patternData.nextPattern == 0) {

        segments[2][0] = S_P;
        segments[2][1] = S_L;
        segments[2][2] = S_U;
        segments[2][3] = S_S;

        segments[2][5] = S_O;
        segments[2][6] = S_N;
        segments[2][7] = S_E;
      } else {
        printNumber(2, 5, ((patternData.nextPattern - 1) % 16) + 1);
        segments[2][5] = getBankLetter(((patternData.nextPattern - 1) / 16) + 1);
        segments[2][5] |= B10000000;
      }
      //
      break;
    //
    case menuNote:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = B11111101;  // menuNote
      segments[1][0] = S_d;
      segments[1][1] = S_r;
      segments[1][2] = S_u;
      segments[1][3] = S_n;

      segments[1][4] = S_N;
      segments[1][5] = S_O;
      segments[1][6] = S_T;
      segments[1][7] = S_E;
      //
      if (curTrack < DRUM_TRACKS) {
        printNumber(2, 0, patternData.trackNote[curTrack]);
        printMIDInote(patternData.trackNote[curTrack], 2, 4, 7);
      } else printDashDash(2, 6);
      break;
    //
    case menuNoteLen:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = S_C;  // menuNoteLen
      segments[0][1] |= B10000000;
      segments[1][0] = S_N;
      segments[1][1] = S_O;
      segments[1][2] = S_T;
      segments[1][3] = S_E;
      //
      segments[1][5] = S_L;
      segments[1][6] = S_E;
      segments[1][7] = S_N;


      //
      printNumber(2, 5, patternData.drumNoteLen[curTrack]);
      break;
    //
    case menuPulseOut:
      segments[0][0] = S_G;
      segments[0][1] = B11100110;  // menuPulseOut
      segments[1][0] = S_P;
      segments[1][1] = S_U;
      segments[1][2] = S_L;
      segments[1][3] = S_S;
      segments[1][4] = S_E;
      segments[1][5] = S_O;
      segments[1][6] = S_U;
      segments[1][7] = S_T;
      //
      if (globalData.tickOut > 0) printNumber(2, 5, globalData.tickOut);
      else showOnOrOff(false);
      break;
    //
    case menuPulseOutLen:
      segments[0][0] = S_G;
      segments[0][1] = B11101101;  // menuPulseOutLen
      segments[1][0] = S_P;
      segments[1][1] = S_U;
      segments[1][2] = S_L;
      segments[1][3] = S_S;
      segments[1][4] = S_E;
      segments[1][5] = S_L;
      segments[1][6] = S_E;
      segments[1][7] = S_N;


      //
      printNumber(2, 5, globalData.tickOutLen);
      break;
    //
    case menuProtect:
      segments[0][0] = S_G;
      segments[0][1] = B10000111;  // menuProtect
      showMemoryProtected();
      break;
    //
    case menuAccent1:

    case menuAccent2:

    case menuAccent3:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = (char)pgm_read_word(&numbers[menuPosition - menuAccent1 + 7]);
      segments[0][1] |= B10000000;
      segments[1][0] = S_U;
      segments[1][1] = S_E;
      segments[1][2] = S_L;
      segments[1][3] = S_O;
      segments[1][4] = S_C;
      segments[1][5] = S_I;
      segments[1][6] = S_T;
      segments[1][7] = S_Y;
      //
      segments[2][0] = S_U;
      segments[2][1] = (char)pgm_read_word(&numbers[menuPosition - menuAccent1 + 1]);
      //
      printNumber(2, 4, patternData.accentValues[menuPosition - menuAccent1]);
      break;

    case menuVariationsABCD:
      segments[0][0] = S_P;
      segments[0][0] |= B10000000;
      segments[0][1] = S_A;  // menuVariationsABCD
      segments[0][1] |= B10000000;
      segments[1][0] = S_P;
      segments[1][1] = S_A;
      segments[1][2] = S_T;
      //
      segments[1][4] = S_U;
      segments[1][5] = S_A;
      segments[1][6] = S_R;
      segments[1][7] = S_I;
      //
      segments[2][4] = S_A;
      if (patternData.totalVariations >= 2) segments[2][5] = S_b;
      if (patternData.totalVariations >= 3) segments[2][6] = S_C;
      if (patternData.totalVariations >= 4) segments[2][7] = S_d;
      break;
    //
    case menuSyncOut:
      segments[0][0] = S_G;
      segments[0][1] = B11011011;  // menuSyncOut
      segments[1][0] = S_C;
      segments[1][1] = S_L;
      segments[1][2] = S_O;
      segments[1][3] = S_C;
      segments[1][4] = S_K;
      //
      segments[1][5] = S_O;
      segments[1][6] = S_U;
      segments[1][7] = S_T;
      //
      showOnOrOff(globalData.seqSyncOut);
      break;

    case menuClockDivider:
      segments[0][0] = S_G;
      segments[0][1] = B11001111;  //
      segments[1][0] = S_C;
      segments[1][1] = S_L;
      segments[1][2] = S_O;
      segments[1][3] = S_C;
      segments[1][4] = S_K;

      segments[1][5] = S_d;
      segments[1][6] = S_I;
      segments[1][7] = S_U;
      segments[2][0] = S_I;
      segments[2][1] = S_d;
      segments[2][2] = S_E;
      segments[2][3] = S_R;

      //
      printNumber(2, 5, globalData.midiClockDivide);
      break;
      //

    //
    case menuMIDIinPattern:
      segments[0][0] = S_G;
      segments[0][1] = B11111101;  // menuMIDIinPattern
      segments[1][0] = S_N;
      segments[1][1] = S_I;
      segments[1][2] = S_d;
      segments[1][3] = S_I;
      //
      segments[1][5] = S_P;
      segments[1][6] = S_A;
      segments[1][7] = S_T;
      //
      segments[2][0] = S_C;
      segments[2][1] = S_H;
      segments[2][2] = S_N;
      segments[2][3] = S_G;

      //
      printNumber(2, 5, globalData.midiInputToPatternChannel + 1);
      break;
    //
    case menuClockType:
      segments[0][0] = S_G;
      segments[0][1] = B10000110;  // menuClockType
      segments[1][0] = S_C;
      segments[1][1] = S_L;
      segments[1][2] = S_O;
      segments[1][3] = S_C;
      segments[1][4] = S_K;
      //
      if (globalData.midiClockInternal) {
        segments[2][4] = S_I;
        segments[2][5] = S_N;
        segments[2][6] = S_T;
      } else {
        segments[2][4] = S_E;
        segments[2][5] = S_X;
        segments[2][6] = S_T;
      }
      break;
    //
    case menuInit:
      segments[0][0] = S_G;
      segments[0][1] = B11111111;  // menuInit
      segments[1][0] = S_d;
      segments[1][1] = B01011100;
      segments[1][2] = B00000000;
      segments[1][3] = S_E;
      segments[1][4] = S_R;
      segments[1][5] = S_A;
      segments[1][6] = S_S;
      segments[1][7] = S_E;
      //
      if (initMode == 0) {
        segments[2][4] = S_N;
        segments[2][5] = S_O;
        segments[2][6] = S_N;
        segments[2][7] = S_E;
      } else if (initMode == 1) {
        segments[2][4] = S_b;
        segments[2][5] = S_A;
        segments[2][6] = S_N;
        segments[2][7] = S_K;
      } else {
        segments[2][5] = S_A;
        segments[2][6] = S_L;
        segments[2][7] = S_L;
      }
      break;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void processMenuOK() {
  switch (menuPosition) {
    case menuInit:
      if (seqPlaying) printStopSequencer();
      else {
        if (initMode == 1) {
          stopSequencer();
          reset();
          int porc = 0;
          initPatternBank(currentPatternBank, true, porc, true);
          loadPatternBank(currentPatternBank);
          if (curRightScreen == kRightMenu) curRightScreen = kRightSteps;
        } else if (initMode == 2) {
          stopSequencer();
          reset();
          currentPatternBank = 0;
          flashInit(true);
          if (curRightScreen == kRightMenu) curRightScreen = kRightSteps;
        } else if (curRightScreen == kRightMenu) curRightScreen = kRightSteps;
      }
      break;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void processMenu(char value) {
  switch (menuPosition) {


    case menuProgramChange:
      if (value > 0 && patternData.programChange[curTrack] < 127) patternData.programChange[curTrack]++;
      else if (value < 0 && patternData.programChange[curTrack] > 0) patternData.programChange[curTrack]--;
      sendMIDIProgramChange(curTrack);

      somethingChangedPattern = true;
      break;

    case menuMIDIChannel:
      if (value > 0 && patternData.trackMidiCH[curTrack] < 15) patternData.trackMidiCH[curTrack]++;
      else if (value < 0 && patternData.trackMidiCH[curTrack] > 0) patternData.trackMidiCH[curTrack]--;
      somethingChangedPattern = true;
      break;
    //
    //
    case menuTrackLen:
      if (value > 0 && patternData.trackLen[curTrack] < 16) patternData.trackLen[curTrack]++;
      else if (value < 0 && patternData.trackLen[curTrack] > 1) patternData.trackLen[curTrack]--;
      somethingChangedPattern = true;
      break;
      //

    case menuTrackProbability:  // Step playback probability
      if (value > 0 && patternData.trackProbability[curTrack] < 100) {
        patternData.trackProbability[curTrack] += 10;
      } else if (value < 0 && patternData.trackProbability[curTrack] > 0) {
        patternData.trackProbability[curTrack] -= 10;
      }
      somethingChangedPattern = true;
      break;

    //
    case menuShuffle:
      if (value > 0 && patternData.shuffleDelay < 5) patternData.shuffleDelay++;
      else if (value < 0 && patternData.shuffleDelay > 0) patternData.shuffleDelay--;

      somethingChangedPattern = true;
      break;

    case menuClockDivider:
      if (value > 0 && globalData.midiClockDivide < 16) globalData.midiClockDivide++;
      else if (value < 0 && globalData.midiClockDivide > 1) globalData.midiClockDivide--;

      somethingChangedGlobal = true;
      break;



    //
    case menuNote:
      if (curTrack < DRUM_TRACKS) {
        if (value > 0 && patternData.trackNote[curTrack] < 127) patternData.trackNote[curTrack]++;
        else if (value < 0 && patternData.trackNote[curTrack] > 0) patternData.trackNote[curTrack]--;
        somethingChangedPattern = true;
      }
      break;
    //
    case menuPulseOut:
      if (value > 0 && globalData.tickOut < 17) globalData.tickOut++;
      else if (value < 0 && globalData.tickOut > 0) globalData.tickOut--;
      if (globalData.tickOut > 0) {
        pulseOut(false);
        tickOutPinState = false;
        tickOutCounterLen = 0;
        tickOutCounter = 0;
      } else {
        pulseOut(false);
        tickOutPinState = false;
      }
      somethingChangedGlobal = true;
      break;
    //
    case menuPulseOutLen:
      if (value > 0 && globalData.tickOutLen < 16) globalData.tickOutLen++;
      else if (value < 0 && globalData.tickOutLen > 1) globalData.tickOutLen--;
      somethingChangedGlobal = true;
      break;
    //
    case menuNoteLen:
      if (value > 0 && patternData.drumNoteLen[curTrack] < 129) patternData.drumNoteLen[curTrack]++;
      else if (value < 0 && patternData.drumNoteLen[curTrack] > 1) patternData.drumNoteLen[curTrack]--;
      somethingChangedPattern = true;
      break;
    //
    case menuAccent1:
    case menuAccent2:
    case menuAccent3:
      if (value > 0 && patternData.accentValues[menuPosition - menuAccent1] < 127) patternData.accentValues[menuPosition - menuAccent1]++;
      else if (value < 1 && patternData.accentValues[menuPosition - menuAccent1] > 0) patternData.accentValues[menuPosition - menuAccent1]--;
      somethingChangedPattern = true;
      break;

    case menuProtect:
      globalData.writeProtectFlash = !globalData.writeProtectFlash;
      saveGlobalData(true);
      break;

    case menuPtNext:
      if (value > 0 && patternData.nextPattern < 256) patternData.nextPattern++;
      else if (value < 1 && patternData.nextPattern > 0) patternData.nextPattern--;
      somethingChangedPattern = true;
      break;
    //
    case menuPtPlays:
      if (value > 0 && patternData.playsPattern < 255) patternData.playsPattern++;
      else if (value < 1 && patternData.playsPattern > 0) patternData.playsPattern--;
      somethingChangedPattern = true;
      break;
    //
    case menuPtPlaysChain:
      if (value > 0 && patternData.playsChain < 255) patternData.playsChain++;
      else if (value < 1 && patternData.playsChain > 0) patternData.playsChain--;
      somethingChangedPattern = true;
      break;
    //
    case menuMIDIinPattern:
      if (value > 0 && globalData.midiInputToPatternChannel < 15) globalData.midiInputToPatternChannel++;
      else if (value < 1 && globalData.midiInputToPatternChannel > 0) globalData.midiInputToPatternChannel--;
      somethingChangedGlobal = true;
      break;
    //
    case menuClockType:
      globalData.midiClockInternal = !globalData.midiClockInternal;
      setupTimerForExternalMIDISync(!globalData.midiClockInternal);
      somethingChangedGlobal = true;
      break;

    case menuVariationsABCD:
      if (value > 0 && patternData.totalVariations < 4) patternData.totalVariations++;
      else if (value < 1 && patternData.totalVariations > 1) patternData.totalVariations--;
      somethingChangedPattern = true;
      break;
    //
    case menuSyncOut:
      globalData.seqSyncOut = !globalData.seqSyncOut;
      somethingChangedGlobal = true;
      break;
    //
    case menuInit:
      if (value > 0 && initMode < 2) initMode++;
      else if (value < 1 && initMode > 0) initMode--;
      break;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//END OF LINE
