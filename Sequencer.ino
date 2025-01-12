/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define PPQ_TICK_DOUBLE_NOTE 3
#define PPQ_TICK_END 6
#define PPQ 24

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) {
  if (globalData.midiClockInternal) {

    if (globalData.seqSyncOut && (seqPlaying)) {
      // Increment the counter every PPQ tick
      midiClockCounter++;

      // Check if it's time to send a MIDI clock pulse
      if (midiClockCounter >= globalData.midiClockDivide) {
        Serial.write(0xF8);    // Send MIDI Clock
        midiClockCounter = 0;  // Reset counter
      }
    }

    outputMIDIBuffer();    //first: doticksequencer and send buffer, then
    calculateSequencer++;  //second: set PPQ calc - do not swap in try to optimize, it makes everything jitter!
    pulseOut(tickOutPinState);
    ledsBufferFlip();
  } else {
    stopTimer(true);
    handleMIDIInput();
    startTimer(true);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void outputMIDIBuffer() {
  while (calculateSequencer > 0) { doTickSequencer(); }
  while (midiOutputBufferPosition > 0) {
    Serial.write(midiOutputBuffer[0][midiOutputBufferPosition - 1]);
    Serial.write(midiOutputBuffer[1][midiOutputBufferPosition - 1]);
    if (midiOutputBuffer[2][midiOutputBufferPosition - 1] != 0xFF) Serial.write(midiOutputBuffer[2][midiOutputBufferPosition - 1]);
    midiOutputBufferPosition--;
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
// I´m looking for a heartbeat...

void doTickSequencer() {
  while (calculateSequencer > 0) {  //flag for executing the tick, 24 pulse per quarter note based
    if (seqPlaying) {
      // Track Mute Logic
      if (prevMuteTrack != muteTrack) {
        for (byte x = 0; x < (NOTE_TRACKS + DRUM_TRACKS); x++) {  //V1777 NOTE_TRACKS + DRUM_TRACKS + 1    +1 removed
          if (bitRead(prevMuteTrack, x) != bitRead(muteTrack, x) && bitRead(muteTrack, x) == 1) {
            if (x >= DRUM_TRACKS) {
              stopDrumTrackPrevNote(x - DRUM_TRACKS, false);  // Adjust global index to local index (0–5 for note tracks)
            } else {
              stopDrumTrackPrevNote(x, true);  // No adjustment needed for drum tracks
            }
          }
        }
        prevMuteTrack = muteTrack;
      }


      // Determine if the current step is even or uneven
      isEvenStep = (trackPosition[longestTrackIndex] + 1) % 2 == 0;

      if (PPQcounter == 0) {
        if (!isEvenStep) {                                 //count ticks from uneven steps, 1 3 5 etc.
          shuffleCounter = patternData.shuffleDelay + 11;  //LDA shuffle delay, full step offset (6 tics), 32nd offset (3 tics), +1 tic since we decrease NOW in the same tick - to align PPQ 0 with shuffleCounter 9, +1 offset to have zero as a neutral value
          alignedToPlay16 = false;                         // Reset alignments
          alignedToPlay32 = false;
        }
      }

      if (shuffleCounter > 0) {
        shuffleCounter--;  // Decrement every PPQ tick
      }

      if (shuffleCounter == 4) { alignedToPlay16 = true; }
      if (shuffleCounter == 1) { alignedToPlay32 = true; }

      if (((PPQcounter == 0 || PPQcounter == PPQ_TICK_DOUBLE_NOTE) && (!isEvenStep)) || (alignedToPlay16) || (alignedToPlay32)) {



        for (byte i = 0; i < 16; i++) {
          byte x = trackSequence[i];  // Get the current track from the sequence


          // Drum Track Handling
          if (x < DRUM_TRACKS) {  // Drum tracks are indexed as 0–9
            bool trackIsMuted = bitRead(muteTrack, x) == 1;
            byte xvel = bitRead(stepsData[trackPosition[x]].steps[x], 1 + (variation * 2)) << 1 | bitRead(stepsData[trackPosition[x]].steps[x], 0 + (variation * 2));
            bool isDouble = bitRead(stepsData[trackPosition[x]].stepsDouble[variation], x);
            //  if ((xvel > 0 && PPQcounter == 0) || (PPQcounter == PPQ_TICK_DOUBLE_NOTE && isDouble))  //old logic

            if (xvel > 0) {
              // now check the time conditions
              if (((PPQcounter == 0) && (!isEvenStep)) || ((PPQcounter == PPQ_TICK_DOUBLE_NOTE) && isDouble && (!isEvenStep)) || alignedToPlay16 || (alignedToPlay32 && isDouble)) {
                if (!trackIsMuted) {
                  // byte theVelocity = patternData.accentValues[xvel - 1];
                  // trackNoteOn(x, patternData.trackNote[x], theVelocity);

                  //Playback Probability - step skip
                  // Generate a random number between 0 and 99
                  byte randomChance = random(100);  // Ensures values 0-99

                  // Check if the step should be skipped
                  if (randomChance >= patternData.trackProbability[x]) {
                    // Step is played
                    byte theVelocity = patternData.accentValues[xvel - 1];
                    trackNoteOn(x, patternData.trackNote[x], theVelocity);
                  }

                } else {
                  noteLenCountersLED[x] = patternData.drumNoteLen[x] + 1;
                }
              }
            }
          }

          // Note Track Handling
          else {

            // Note tracks are indexed as 10–15
            bool NtrackIsMuted = bitRead(muteTrack, x) == 1;
            byte Nxvel = bitRead(stepsData[trackPosition[x]].noteStepsExtras[x - DRUM_TRACKS][0], 1 + (variation * 2)) << 1 | bitRead(stepsData[trackPosition[x]].noteStepsExtras[x - DRUM_TRACKS][0], 0 + (variation * 2));
            byte extra = bitRead(stepsData[trackPosition[x]].noteStepsExtras[x - DRUM_TRACKS][1], 1 + (variation * 2)) << 1 | bitRead(stepsData[trackPosition[x]].noteStepsExtras[x - DRUM_TRACKS][1], 0 + (variation * 2));
            bool isSlide = (extra == 1);
            bool isNoteOff = (extra == 3);
            byte xnote = stepsData[trackPosition[x]].noteSteps[x - DRUM_TRACKS][variation];
            if ((xnote > 0 && PPQcounter == 0) || (xnote > 0 && PPQcounter == 0 && alignedToPlay16)) {
              if (isNoteOff) {
                //sendMidiEvent(midiNoteOff, xnote, 0, patternData.trackMidiCH[x]);
                noteLenCounters[x] = 1;
                //prevPlayedNote[x - DRUM_TRACKS] = (xnote == prevPlayedNote[x - DRUM_TRACKS]) ? 0 : prevPlayedNote[x - DRUM_TRACKS];
              }
            }
            // if (xnote > 0 && Nxvel > 0 && PPQcounter == 0) { //old logic
            if ((xnote > 0 && Nxvel > 0 && PPQcounter == 0) || (xnote > 0 && Nxvel > 0 && alignedToPlay16)) {
              if (!NtrackIsMuted) {
                // Generate a random number between 0 and 99
                byte randomChance = random(100);  // Ensures values 0-99

                // Check if the step should be skipped
                if (randomChance >= patternData.trackProbability[x]) {

                  byte theVelocity = patternData.accentValues[Nxvel - 1];
                  noteTrackNoteOn(x, xnote, theVelocity, isSlide);
                }

              } else {
                noteLenCountersLED[x] = patternData.drumNoteLen[x] + 1;
              }
            }

          }  //else note tracks


        }  // 16 x playback loop
        //alignedToPlay16 = false;
        alignedToPlay32 = false;
      }  //the initial IF



      // Recording Check
      if (recordEnabled && PPQcounter == 0) {
        for (xm = 0; xm < recordBufferPosition; xm++) {
          recordInputCheck(recordBuffer[0][xm], recordBuffer[1][xm], recordBuffer[2][xm], curTrack);
        }
        recordBufferPosition = 0;
      }

      // Tick Output Check
      if (globalData.tickOut > 0 && (PPQcounter == 0 || PPQcounter == PPQ_TICK_DOUBLE_NOTE)) {
        tickOutCounter = (tickOutCounter + 1) % globalData.tickOut;
        if (tickOutCounter == globalData.tickOut - 1) tickOutCounterLen = globalData.tickOutLen;
        tickOutPinState = (tickOutCounterLen > 0);
        if (tickOutPinState) tickOutCounterLen--;
      }

      // NOTE LENGTH COUNTER

      for (byte i = 0; i < 16; i++) {
        byte x = trackSequence[i];  // Process tracks in the correct priority order
        if (noteLenCounters[x] > 0) {
          if (--noteLenCounters[x] == 0) {
            // Only send Note Off for the corresponding note
            if (x < DRUM_TRACKS) {
              sendMidiEvent(midiNoteOff, patternData.trackNote[x], 0, patternData.trackMidiCH[x]);
            } else if (prevPlayedNote[x - DRUM_TRACKS] > 0) {
              sendMidiEvent(midiNoteOff, prevPlayedNote[x - DRUM_TRACKS], 0, patternData.trackMidiCH[x]);
              prevPlayedNote[x - DRUM_TRACKS] = 0;  // Reset the previous note
            }
          }
        }
        if (noteLenCountersLED[x] > 0) {
          if (--noteLenCountersLED[x] == 0) {
          }
        }
      }


      //

      // only three must pass:
      //     void doTickSequencer() {
      //    while (calculateSequencer > 0) {
      //   if (seqPlaying) {

      //---==================-------==================-------==================-------==================-------==================----
      //---==================-------==================-------==================-------==================-------==================----
      //---==================-------==================-------==================-------==================-------==================----
      //                   _________-----_____
      //        _____------           __      ----_
      // ___----             ___------              \
      //    ----________        ----                 \
      //                -----__    |             _____)
      //                     __-                /     \
      //         _______-----    ___--          \    /)\
      //   ------_______      ---____            \__/  /
      //                -----__    \ --    _          /\
      //                       --__--__     \_____/   \_/\
      //                               ----|   /          |
      //                                   |  |___________|
      //                                   |  | ((_(_)| )_)
      //                                   |  \_((_(_)|/(_)
      //                                   \             (
      //                                    \_____________)
      //---==================-------==================-------==================-------==================-------==================----
      //---==================-------==================-------==================-------==================-------==================----
      //---==================-------==================-------==================-------==================-------==================----
    }

    // Pattern Stream Handling
    if (PPQcounter == PPQ_TICK_DOUBLE_NOTE) {
      longestTRACK = 0;
      longestTrackIndex = 0;

      for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {


        if (patternData.trackLen[x] > longestTRACK) {
          // Update longest track
          longestTRACK = patternData.trackLen[x];
          longestTrackIndex = x;
        }
      }

      if (seqPlaying) {

        // trigger pattern switching based longest track's position
        if ((trackPosition[longestTrackIndex] + 1) >= patternData.trackLen[longestTrackIndex] && (variation + 1) >= prevPatternTotalVariations) {
          //last 32nd step before wrapping, +3 tics from here and we are at position 0
          //once per polyMetric or once per 16/32/48/64 steps A/B/C/D wrap

          if (streamNextPattern) {
            //next pattern is chained, in 3 tics we do the switch

            streamNextPattern = false;
            loadPatternNow = true;
            currentPlaysPattern = currentPlaysChain = 0;
            for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {

              if (patternData.trackLen[x] != 16) {  //set all steps to their last position if we were in polyMetric mode
                trackPosition[x] = 15;

              } else doAlign = true;  //set flag to ensure first step alignment
            }
          }


          // Handle Playlist/Chaining
          else {
            if (patternData.playsPattern != 0 && (repeatMode == kRepeatModeNormal || repeatMode == kRepeatModeChain)) {
              currentPlaysPattern++;
              if ((currentPlaysPattern - 1) == (patternData.playsPattern - 1)) {
                currentPlaysPattern = 0;
                //
                if (patternData.playsChain != 1) {
                  currentPlaysChain++;
                  if (patternData.playsChain == 0 || repeatMode == kRepeatModeChain) currentPlaysChain = 0;
                  if (patternData.playsChain != 0 && currentPlaysChain >= (patternData.playsChain - 1)) {
                    currentPlaysChain = 0;
                    nextPattern = currentPattern + 1;
                    loadPatternNow = true;
                    for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {

                      if (patternData.trackLen[x] != 16) {  //set all steps to their last position if we were in polyMetric mode
                        trackPosition[x] = 15;

                      } else doAlign = true;  //set flag to ensure first step alignment
                    }
                  } else {
                    if (patternData.nextPattern == 0) nextPattern = currentPattern + 1;
                    else {
                      nextPattern = ((patternData.nextPattern - 1) % 16);
                      nextPatternBank = (patternData.nextPattern - 1) / 16;
                    }
                    loadPatternNow = true;
                    for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {

                      if (patternData.trackLen[x] != 16) {  //set all steps to their last position if we were in polyMetric mode
                        trackPosition[x] = 15;

                      } else doAlign = true;  //set flag to ensure first step alignment
                    }
                  }
                } else {
                  if (patternData.nextPattern == 0) nextPattern = currentPattern + 1;
                  else {
                    nextPattern = ((patternData.nextPattern - 1) % 16);
                    nextPatternBank = (patternData.nextPattern - 1) / 16;
                  }
                  loadPatternNow = true;
                  for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {

                    if (patternData.trackLen[x] != 16) {  //set all steps to their last position if we were in polyMetric mode
                      trackPosition[x] = 15;

                    } else doAlign = true;  //set flag to ensure first step alignment
                  }
                }
              }
            }
          }
        }
      }
    }


    // only two must pass:
    //     void doTickSequencer() {
    // while (calculateSequencer > 0) {


    //---==================-------==================-------==================-------==================----
    //---==================-------==================-------==================-------==================----
    //---==================-------==================-------==================-------==================----

    //                            _
    //                          _| |
    //                        _| | |
    //                       | | | |
    //                       | | | | __
    //                       | | | |/  \
    //                       |       /\ \
    //                       |      /  \/
    //                       |      \  /\
    //                       |       \/ /
    //                       \        /
    //                         |     /
    //                         |    |

    //---==================-------==================-------==================-------==================----
    //---==================-------==================-------==================-------==================----
    //---==================-------==================-------==================-------==================----

    PPQcounter++;
    if (PPQcounter >= PPQ_TICK_END) {
      PPQcounter = 0;



      for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {
        // Align tracks when coming from a full 16 step pattern
        if (doAlign == true) {
          trackPosition[x] = (patternData.trackLen[x] - 1);
        }
        // Increment and wrap position based on track length
        trackPosition[x] = (trackPosition[x] + 1) % patternData.trackLen[x];


        // Polymetric mode - Disable variations if any track is shorter than 16 steps
        if (patternData.trackLen[x] < 16) {
          prevPatternTotalVariations = patternData.totalVariations = 1;

          if (variation != 0) {
            variation = 0;
            forceVariation = -1;
            mirror = false;
          }
        }
      }
      doAlign = false;
      seqPosition = trackPosition[longestTrackIndex];


      if (trackPosition[longestTrackIndex] == 0 && seqPlaying == true) {
        //we are at position 0 now
        //wrapping of longest track if polyMetric, else wrapping of 16step pattern


        //
        if (patternData.totalVariations != 1) {
          //this part only for patterns with more than 16 steps
          //WRAP ABCD

          variation = (variation + 1) % prevPatternTotalVariations;


          // if (variation == 0) {
          //   Serial.write(0xFE);
          //   // All variations completed, trigger pattern stream
          //   checkPatternStream();

          //   // Display error if pattern loading is delayed
          //   if (loadPatternNow) showErrorMsg(99);
          // }
        }



        // Always update variation tracking
        prevPatternTotalVariations = patternData.totalVariations;
      }
    }


    // Reset LED buffers
    chaseLEDsBuffer[0] = 0;
    chaseLEDsBuffer[1] = 0;
    // Track LED update only for the currently selected track

    isEvenStep = (trackPosition[longestTrackIndex] + 1) % 2 == 0;
    if ((PPQcounter == 0 && !isEvenStep) || alignedToPlay16) {

      ledPosition = trackPosition[curTrack];  // Current step for the selected track
    }
    if (ledPosition < patternData.trackLen[curTrack]) {
      if (ledPosition < 8) {
        bitSet(chaseLEDsBuffer[0], ledPosition);
      } else {
        bitSet(chaseLEDsBuffer[1], ledPosition - 8);
      }
    }
    alignedToPlay16 = false;

    // Update LEDs on screen
    ledsVarSeqUpdate();
    updateScreen = true;

    calculateSequencer--;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void recordInputCheck(byte data1, byte data2, byte channel, byte track) {

  byte xvariation = variation;
  byte theStep = trackPosition[track];
  //
  if (theStep > 0) theStep--;
  else {
    theStep = patternData.trackLen[curTrack] - 1;  // was STEPS
    if (xvariation > 0) xvariation--;
    else xvariation = 3;
  }
  //
  byte xVar = xvariation;
  if (forceVariation >= 0) xVar = forceVariation;
  if (mirror) xVar = 0;
  //
  if (track < DRUM_TRACKS)  //LOGIC FOR RECORDING DRUM TRACKS

  {
    if (data2 > 0)  // Check if there is a note on event with velocity
    {
      bitClear(stepsData[theStep].steps[track], (xVar * 2));
      bitClear(stepsData[theStep].steps[track], (xVar * 2) + 1);
      //
      // Quantize velocity for drum tracks  - REDUX 4
      lastVelocityMIDI = 1;                                                 // Default
      if (data2 >= patternData.accentValues[2]) lastVelocityMIDI = 3;       // Strong accent
      else if (data2 >= patternData.accentValues[1]) lastVelocityMIDI = 2;  // Mild accent


      // Apply quantized velocity to the drum step
      somethingChangedPattern = true;
      stepsData[theStep].steps[track] |= lastVelocityMIDI << (xVar * 2);
      //stepsData[theStep].steps[track] |= 1 << (xVar * 2);  //REDUX 4  old version

      // Apply the mirror logic across all variations
      if (mirror) {
        // Clear the step for all variations (set it to 0x00)
        stepsData[theStep].steps[track] = 0x00;

        // Apply mirrored velocity (stepVelocity can be 1, 2, or 3)
        stepsData[theStep].steps[track] |= lastVelocityMIDI;       // Set first position
        stepsData[theStep].steps[track] |= lastVelocityMIDI << 2;  // Set second position
        stepsData[theStep].steps[track] |= lastVelocityMIDI << 4;  // Set third position
        stepsData[theStep].steps[track] |= lastVelocityMIDI << 6;  // Set fourth position
      }
    }
  } else  //LOGIC FOR RECORDING NOTE TRACKS
          // {
          //  if (patternData.trackMidiCH[track] == channel)  //REDUX 4 allow rec from any channel
  {
    if (data2 > 0) {
      // clearStepsExtrasBits(theStep, xVar, track-DRUM_TRACKS);
      patternData.lastNote[track - DRUM_TRACKS] = data1;
      //
      lastVelocityMIDI = 1;
      if (data2 >= patternData.accentValues[2]) lastVelocityMIDI = 3;
      else if (data2 >= patternData.accentValues[1]) lastVelocityMIDI = 2;
#if RECORD_FORCE_VEL
      lastVelocityMIDI = 3;
#endif
      //
      stepsData[theStep].noteSteps[track - DRUM_TRACKS][xVar] = patternData.lastNote[track - DRUM_TRACKS];
      // Clear the specific bits before setting the velocity for the given variation
      stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0] &= ~(0b11 << (xVar * 2));           // Clear two bits at the target position
      stepsData[theStep].noteStepsExtras[track - DRUM_TRACKS][0] |= lastVelocityMIDI << (xVar * 2);  // Set new velocity bits

      //
      checkIfMirrorAndCopy(theStep, track - DRUM_TRACKS);
      somethingChangedPattern = true;
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ledsVarSeqUpdate() {
  leftLEDsVarSeqBuffer = 0;
  bitSet(leftLEDsVarSeqBuffer, (seqPosition / 4));
  byte xVar = variation;
  if (forceVariation >= 0) xVar = forceVariation;
  if (mirror) leftLEDsVarSeqBuffer |= B11110000;
  else {
    if (forceVariation >= 0) {
      leftLEDsVarSeqBuffer |= B11110000;
      bitClear(leftLEDsVarSeqBuffer, xVar + 4);
    } else bitSet(leftLEDsVarSeqBuffer, xVar + 4);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ledsBufferFlip() {
  leftLEDsVarSeq = leftLEDsVarSeqBuffer;
  chaseLEDs[0] = chaseLEDsBuffer[0];
  chaseLEDs[1] = chaseLEDsBuffer[1];
  //showBPMdot = showBPMdotBuffer;
  updateScreen = true;
}


//               *
//              / \
  //             /___\
  //            ( o o )            * *
//            )  L  (           /   * *
//    ________()(-)()________  /     * * *
//  E\| _____ )()()() ______ |/B     * * *
//    |/      ()()()(       \|      * * * *
//            | )() |
//            /     \
  //           / *  *  \
  //          /   *  *  \
  //         / *_  *  _  \



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

inline void trackNoteOn(byte xtrack, byte xnote, byte xvelocity)  //drum track
{
  sendMidiEvent(midiNoteOn, xnote, xvelocity, patternData.trackMidiCH[xtrack]);
  if (noteLenCounters[xtrack] > 0) {
    sendMidiEvent(midiNoteOff, xnote, 0, patternData.trackMidiCH[xtrack]);
    noteLenCounters[xtrack] = 0;
  }
  //
  noteLenCounters[xtrack] = patternData.drumNoteLen[xtrack] + 1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void noteTrackNoteOn(byte xtrack, byte xnote, byte xvelocity, bool slide)  //note track
{

  // Handle previous note logic
  if ((prevPlayedNote[xtrack - DRUM_TRACKS] > 0) || prevPlayedNote[xtrack - DRUM_TRACKS] == xnote) {
    if (noteLenCounters[xtrack] > 130) {
      // Glide Mode: Play new note while holding previous note
      sendMidiEvent(midiNoteOff, prevPlayedNote[xtrack - DRUM_TRACKS], 0, patternData.trackMidiCH[xtrack]);  // Add Old Note Off
    }
  }
  sendMidiEvent(midiNoteOn, xnote, xvelocity, patternData.trackMidiCH[xtrack]);  // Add New Note On
  if ((prevPlayedNote[xtrack - DRUM_TRACKS] > 0) || prevPlayedNote[xtrack - DRUM_TRACKS] == xnote) {
    if (noteLenCounters[xtrack] < 130) {

      sendMidiEvent(midiNoteOff, prevPlayedNote[xtrack - DRUM_TRACKS], 0, patternData.trackMidiCH[xtrack]);  // Add Old Note Off
    }
  }

  // Update previous note to the current note
  prevPlayedNote[xtrack - DRUM_TRACKS] = xnote;
  // Set note length based on slide mode
  if (slide) {
    noteLenCounters[xtrack] = 255;  // Max length for glide
  } else {
    noteLenCounters[xtrack] = patternData.drumNoteLen[xtrack] + 1;  // Standard length
  }
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetSequencer() {
  pulseOut(false);
  tickOutCounter = 0;
  tickOutCounterLen = 0;
  tickOutPinState = false;
  //
  seqPositionPrev = seqPosition;
  PPQcounterPrev = PPQcounter;
  variationPrev = variation;
  midiClockBeatsPrev = midiClockBeats;
  currentPlaysPattern = currentPlaysChain = 0;
  recordBufferPosition = 0;
  //
  memset(midiOutputBuffer, 0, sizeof(midiOutputBuffer));
  midiOutputBufferPosition = 0;
  memset(noteLenCounters, 0, sizeof(noteLenCounters));

  seqPosition = PPQcounter = variation = midiClockBeats = 0;

  for (byte x = 0; x < DRUM_TRACKS; x++) {
    stopDrumTrackPrevNote(x, true);
    trackPosition[x] = 0;
  }

  for (byte x = 0; x < NOTE_TRACKS; x++) {
    stopDrumTrackPrevNote(x, false);
    trackPosition[DRUM_TRACKS + x] = 0;
  }

  if (globalData.writeProtectFlash) ShowTemporaryMessage(kMemoryProtectMessage);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void stopDrumTrackPrevNote(byte track, bool isDrumTrack) {
  if (isDrumTrack) {
    sendMidiEvent(midiNoteOff, patternData.trackNote[track], 0, patternData.trackMidiCH[track]);
  } else {
    if (prevPlayedNote[track] > 0) {
      sendMidiEvent(midiNoteOff, prevPlayedNote[track], 0, patternData.trackMidiCH[DRUM_TRACKS + track]);
      prevPlayedNote[track] = 0;
    }
  }
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startSequencer() {
  bool alreadyPlaying = seqPlaying;
  stopTimer(false);
  seqPlaying = true;

  resetSequencer();
  resetProgramChangeAndCC();

  for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {
    trackPosition[x] = 0;
  }
  prevPatternTotalVariations = patternData.totalVariations;
  if (globalData.seqSyncOut && globalData.midiClockInternal) Serial.write(0xFA);  // MIDI Start
  if (alreadyPlaying) {
    memset(midiOutputBuffer, 0, sizeof(midiOutputBuffer));
    midiOutputBufferPosition = 0;
    calculateSequencer = 1;
    doTickSequencer();
  }
  //
  outputMIDIBuffer();
  calculateSequencer = 1;
  //
  ledsVarSeqUpdate();
  startTimer(false);
  leftLEDsVarSeq = leftLEDsVarSeqBuffer;
  chaseLEDs[0] = chaseLEDs[1] = 0;
  chaseLEDs[0] = B10000000;
  updateScreen = true;
  //
  tickOutPinState = true;
  pulseOut(true);
  tickOutCounter = 0;
  tickOutCounterLen = globalData.tickOutLen;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void startTimer(bool force) {
  if (globalData.midiClockInternal || force) {
    TCCR1A = TCCR1B = 0;
    if (globalData.midiClockInternal) bitWrite(TCCR1B, CS11, 1);
    else bitWrite(TCCR1B, CS10, 1);
    bitWrite(TCCR1B, WGM12, 1);
    updateSequencerSpeed(false);
    bitWrite(TIMSK1, OCIE1A, 1);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void stopTimer(bool force) {
  if (globalData.midiClockInternal || force) {
    bitWrite(TIMSK1, OCIE1A, 0);
    TCCR1A = TCCR1B = OCR1A = 0;
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateSequencerSpeed(bool force) {
  // Calculates the Frequency for the Timer, used by the PPQ clock (Pulses Per Quarter Note) //
  // This uses the 16-bit Timer1, unused by the Arduino, unless you use the analogWrite or Tone functions //

  if (globalData.midiClockInternal || force) OCR1A = (F_CPU / 8) / (((float)realBPM * (float)PPQ) / 60.0) - 1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void setupTimerForExternalMIDISync(bool active) {

  if (active) {
    stopTimer(true);
    OCR1A = EXTERNAL_CLOCK_TIMER;
    startTimer(true);
  } else {
    stopTimer(true);
    updateSequencerSpeed(true);
    startTimer(true);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void stopSequencer(void)  // a lot of serial flush and delay commands are necessary to prevent crash - condition: when a lot of steps is used (lots of midi data), high BPM
{
  // Stop Timer //


  seqPlaying = false;
  stopTimer(false);
  Serial.flush();
  delay(40);
  checkIfDataNeedsSaving();
  delay(40);

  if (globalData.seqSyncOut && globalData.midiClockInternal) Serial.write(0xFC);  // MIDI Stop
  Serial.flush();
  //  resetSequencer(); //below lines excerpt from reset sequencer routine, original resetsequencer routine messes with the buffers and likely causes conflicts/crashes

  for (byte x = 0; x < DRUM_TRACKS; x++) {
    stopDrumTrackPrevNote(x, true);
  }
  //
  for (byte x = 0; x < NOTE_TRACKS; x++) {
    stopDrumTrackPrevNote(x, false);
  }

  //
  Serial.flush();
  delay(20);
  resetProgramChangeAndCC();

  outputMIDIBuffer();
  Serial.flush();
  MIDIallNotesOff();  //sends B0 7B 00 - BF 7B 00
  Serial.flush();
  if (streamNextPattern) {
    streamNextPattern = false;
    loadPatternNow = true;
  }

  startTimer(false);
  updateScreen = true;
  delay(20);
  calculateSequencer = 1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MIDIallNotesOff() {
  for (byte x = 0; x < midiChannels; x++) {
    sendMidiEvent(0xB0, 0x7B, 0x00, x);
    Serial.flush();
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Send MIDI event

inline void sendMidiEvent(byte type, byte byte1, byte byte2, byte channel) {
  midiOutputBuffer[0][midiOutputBufferPosition] = type | (channel & 0x0F);  // Mask channel to 4 bits
  midiOutputBuffer[1][midiOutputBufferPosition] = byte1;
  midiOutputBuffer[2][midiOutputBufferPosition] = byte2;
  midiOutputBufferPosition++;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startMIDIinterface() {
#if DEBUG_SERIAL
  Serial.begin(9600);
  Serial.println("Startup");
#else
#if MIDI_OVER_USB
  Serial.begin(38400);
#else
  Serial.begin(31250);  // 31250 MIDI Interface //
#endif
  Serial.flush();
  resetSequencer();
  MIDIallNotesOff();
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void sendMIDIProgramChange(byte track) {
  if (patternData.programChange[track] > 0) sendMidiEvent(midiProgramChange, patternData.programChange[track] - 1, 0xFF, patternData.trackMidiCH[track]);
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void resetProgramChangeAndCC() {

  for (byte x = 0; x < (DRUM_TRACKS + NOTE_TRACKS); x++) {
    sendMIDIProgramChange(x);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline void pulseOut(bool enable) {
  if (enable) PORTC = 0xFF;
  else PORTC = 0x00;
}

//END OF LINE
