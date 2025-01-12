/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void handleMIDIInput()
{
  if (Serial.available() > 0)
  {
    if (receivingExternalSysEx)
    {
      inputByte = Serial.read();
      if (inputByte == 0xF7) receivingExternalSysEx = false;
    }
    else if (midiInputStage == 0)
    {
      inputByte = Serial.read();
      //
      #if TRACK_DEBUG_MIDI
        lastMIDIinput[3] = lastMIDIinput[2];
        lastMIDIinput[2] = lastMIDIinput[1];
        lastMIDIinput[1] = lastMIDIinput[0];
        lastMIDIinput[0] = inputByte;
      #endif     
      //
      if (inputByte == 0xF8) // Midi Clock Sync Tick 24PPQ =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        if (!globalData.midiClockInternal) 
        {
          if (globalData.seqSyncOut) Serial.write(0xF8);

          outputMIDIBuffer();  //REDUX 4
          calculateSequencer++;
          pulseOut(tickOutPinState);
          ledsBufferFlip(); 
        }
      }
      else if (inputByte == 0xFA) // MIDI Start =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
       
          if (globalData.seqSyncOut) Serial.write(0xFA);
          startSequencer();
        
      }
      else if (inputByte == 0xFC) // MIDI Stop =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
       
          if (globalData.seqSyncOut) Serial.write(0xFC);
          stopSequencer();
        
      }
      else if (inputByte == 0xFB)   // MIDI Clock Continue
      {

        
          if (globalData.seqSyncOut) Serial.write(0xFA);
          #if EXTERNAL_CONTINUE
            startSequencer();
          #endif
               
      }
      else if (inputByte == 0xF0) // System Exclusive Data - SysEx =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        receivingExternalSysEx = true;
      }
      else if (inputByte == 0xFE   // Active Sensing
              || inputByte == 0xF6   // Tune Request
              || inputByte == 0xFF)  // System Reset
      {
        if (globalData.seqSyncOut && inputByte == 0xFB) Serial.write(0xFB);
      }
      else
      {
        midiInputStage = 1;
        midiInputBuffer[0] = inputByte;
      }
    }
    else if (midiInputStage == 1)
    {
      midiInputBuffer[1] = Serial.read();
      //
      switch(midiInputBuffer[0] & 0xF0)
      {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xF2:
          midiInputStage = 2;
          break;
        //
        default:
          midiInputStage = 0;
          //
          #if TRACK_DEBUG_MIDI
            lastMIDIinput[3] = 0;
            lastMIDIinput[2] = 0;
            lastMIDIinput[1] = midiInputBuffer[0];
            lastMIDIinput[0] = midiInputBuffer[1];
          #endif              
          //
          break;
      }        
    }
    else if (midiInputStage == 2)
    {
      byte lastData = Serial.read();
      midiInputStage = 0;
      byte channel = midiInputBuffer[0] & 0xF;
      //
      #if TRACK_DEBUG_MIDI
        lastMIDIinput[3] = 0;
        lastMIDIinput[2] = midiInputBuffer[0];
        lastMIDIinput[1] = midiInputBuffer[1];
        lastMIDIinput[0] = lastData;
      #endif              
      //      
      switch(midiInputBuffer[0] & 0xF0)
      {
 case 0x80: // Note Off -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        {
          // if (recordEnabled && seqPlaying) //addRecordNotes(midiInputBuffer[1], 0, channel); // we do not want to record note offs
          //
          #if RECORD_ENABLED_ECHO
//            Serial.write(midiInputBuffer[0]);   //old midi thru(echo) code
//            Serial.write(midiInputBuffer[1]);
//            Serial.write(lastData);
    if (curTrack < DRUM_TRACKS) 
    {
      // Drum Tracks - Modify Channel and Note - quantizing ANY midi input channel & note to current track setup
      byte newChannel = patternData.trackMidiCH[curTrack] & 0x0F; // Mask lower 4 bits for the channel
      byte newNote = patternData.trackNote[curTrack] & 0x7F;      // Mask the note to keep it within valid range
      
      // Replace the channel in midiInputBuffer[0] and the note in midiInputBuffer[1]
      Serial.write(0x80 | newChannel);  // Note Off + new channel
      Serial.write(newNote);            // New note
      Serial.write(lastData);           // Velocity (lastData stays the same)
    } 
    else 
    {
      // Note Tracks - Modify only the Channel - quantizing ANY midi input channel to current track setup
      byte newChannel = patternData.trackMidiCH[curTrack] & 0x0F;
      
      Serial.write(0x80 | newChannel);  // Note Off + new channel
      Serial.write(midiInputBuffer[1]); // Original note
      Serial.write(lastData);           // Velocity
    }
          #endif
          //
          break;
        }
        //
 case 0x90: // Note On =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        {

  #if MIDI_IN_TO_PATTERN  

         
            if (!recordEnabled && channel == globalData.midiInputToPatternChannel) {
                int noteOffset = (midiInputBuffer[1] - 36) % 16;  // Offset from note 36, limited to 16 keys
                if (noteOffset < 0) noteOffset += 16;              // Ensure positive wrap-around if below range
                byte noteValue = noteOffset;                   // Map to 1-16
            
                // Toggle between bank and pattern selection
                if (midiToBank) {
                    nextPatternBank = noteValue;
                } else {
                    nextPattern = noteValue;
                }
                
                midiToBank = !midiToBank;  // Toggle for next press
            
                // Only call loadPattern if there's a change
                if (nextPatternBank != currentPatternBank || nextPattern != currentPattern) {
                    loadPattern(nextPattern);  // Update display immediately
                }
            }
  #endif


          if (recordEnabled && seqPlaying && lastData > 0)  //do not record note on zero velocity as note off steps
            {
               addRecordNotes(midiInputBuffer[1], lastData, channel);
              }
          //

          
          #if RECORD_ENABLED_ECHO
//            Serial.write(midiInputBuffer[0]); //old midi thru(echo) code
//            Serial.write(midiInputBuffer[1]);
//            Serial.write(lastData);
    if (curTrack < DRUM_TRACKS) 
    {
      // Drum Tracks - Modify Channel and Note  - quantizing ANY midi input channel & note to current track setup
      byte newChannel = patternData.trackMidiCH[curTrack] & 0x0F; // Mask lower 4 bits for the channel
      byte newNote = patternData.trackNote[curTrack] & 0x7F;      // Mask the note to keep it within valid range

      Serial.write(0x90 | newChannel);  // Note On + new channel
      Serial.write(newNote);            // New note
      Serial.write(lastData);           // Velocity (lastData stays the same)
    }
    else 
    {
      // Note Tracks - Modify only the Channel  - quantizing ANY midi input channel to current track setup
      byte newChannel = patternData.trackMidiCH[curTrack] & 0x0F;

      Serial.write(0x90 | newChannel);  // Note On + new channel
      Serial.write(midiInputBuffer[1]); // Original note
      Serial.write(lastData);           // Velocity
    }
  #endif

          
          break; 
        }     


         
      }
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void addRecordNotes(byte data1, byte data2, byte channel)
{
  if (recordBufferPosition < MIDI_INPUT_BUFFER)
  {
    recordBuffer[0][recordBufferPosition] = data1;
    recordBuffer[1][recordBufferPosition] = data2;
    recordBuffer[2][recordBufferPosition] = channel;
    recordBufferPosition++;
  }
}

//END OF LINE
