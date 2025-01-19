/*
 * Beat707NXT V2.0
 * Created by William Kalfelz @ Beat707 (c) 2011-2018, last change Nov 20, 2018 - http://www.Wusik.com
 * NXT Core Beta testing and creative development by Gert Borovcak (c) 2018
 *
 * Beat707 is a William Kalfelz / Wusik.com product. Here's how it all started:
 * https://www.matrixsynth.com/2011/05/beat707-arduino-drum-sequencer-review.html
 *
 * //
 *
 * Fork: EOSequencer - Experimental Open Sequencer V3.2 RC
 * Last edit by Gert Borovcak, Jan 09, 2025 - No rights reserved
 *
 * Please consider supporting William - the MIDI Wizard - if you enjoy our sequencer:
 * https://www.patreon.com/williamkwusik
 *
 * No full manual is available yet, but the system should be fairly intuitive.
 * Please look up Variables.h for an in-depth description of setup parameters.
 *

  Programming info:
  Some people reported issues when compiling on a Mac ArduinoIDE environment.
  We will add detailed description of required compiler flags soon.
  
  Using only standard flags can produce a working build but it can have a lot of timing issues and dropouts...
  
  So for people that only want to program the MCU and build it, the binaries have been uploaded.
  
  Easiest to use usbasp (clone) programmer and AVRdude and optional AVRdudess for a nice interface.
  Flags need to be set aswell before programming, to use the pcb mount 8Mhz resonator for more precision.
  The internal RC oscillator is not very good. 
  L 0xFF
  H 0xDA
  E 0xFD
  
  avrdude.exe -c usbasp(-clone) -p m328p -U lfuse:w:0xFF:m -U hfuse:w:0xDA:m -U efuse:w:0xFD:m
  For newer arduino pro mini clones, m328p must be changed to m328pb.
  The pb version is fully backwards compatible with the older m328p.
__________________________________________________________________________________________________

************************************************************************
* ONLY FOR ARDUINO PRO MINI 3.3V 8Mhz Atmega 328P(B)                   *
* AND 3x TM1638 PERIPHERAL MODULES                                     *
************************************************************************

************************************************************************
* FOOTPRINT: 16 patterns with 16 midi tracks, arranged in 16 banks     *
************************************************************************

Principal Track Layout:
-----------------------

* 16 MIDI TRACKS:  
  - **DRUM TRACKS**: 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10  
  - **NOTE TRACKS (Monophonic)**: 1 - 2 - 3 - 4 - 5 - 6 (actual tracks 10-16)

************************************************************************

Quick Writeup About the Interface:
----------------------------------

* 1x CONTROL BOARD  
* 2x 8-STEP BOARDS  

Control Board Overview:
-----------------------

* **LEDs**: Variations (A B C D)  
  - 4 Variations × 16 Steps = 64 Total Steps  

* **DISPLAY**:  
  PLAY/REC | BPM | BPM | BPM | CPU LOAD | BANK X | PATT. XX  
  - 16 Patterns in 16 Banks  
  - Bank Navigation: 1 . 2 . 3 . <> . E . F . G  

* **BUTTONS**:  
  PLAY/REC | STOP | (-) | (+) | BANK | PATTERN | TRACK | ACTION  

Basic Button Combinations:
--------------------------

- **BANK + PATTERN**: Pattern & Global Setup Menu (Latch menu with exit key)  
- **PATTERN + TRACK**: Copy, Paste, or Clear Pattern Steps (Hold to use)  
- **TRACK + ACTION**: Mute Screen (Latch menu with exit key)  
- **ACTION + PLAY/REC**: Change Chain Playback Mode  
  - Repeat Current Pattern (Infinite)  
  - Repeat Current Chain (Infinite)  
  - Normal Playback  

Action Button - Multiple Functions:
-----------------------------------

* **Native Mode - pushing only the ACTION Button - ABCD Editing:**  
  - As represented by LED patterns on the CONTROL Board:  
    - 1111 = Edit All Variations Identically  
    - 0111 = Edit Only A, 1011 = Edit Only B, etc.  
    - Modes cycle with each button press, indicated by LEDs (A B C D).  
    - The 6 variation modes and their LED patterns:  
      - Chase mode: A=1000, B=0100, C=0010, D=0001  
      - All variations: ABCD=1111  
      - Only A/B/C/D: A=0111, B=1011, C=1101, D=1110  

* **Drum & Note Track Velocity Editing:**  
  - Hold ACTION Button + Push Step: Set Velocity (shown as 1 > 2 > 3 on segment display).  
  - Design allows pushing several steps simultaneously to change velocity.  

* **Note Track Transpose Editing:**  
  - Hold Note Step + ACTION Button: Switch between "All Transpose" and "Note Edit".  
  - Use (-) and (+) to adjust while still holding the step.  
  - Design allows holding several steps simultaneously for "Note Edit" changes.  
  - Behavior depends on whether ACTION Button or STEP Button is pressed first.  

Advanced Editing with Action Button:
------------------------------------

* Keep ACTION Button held + Long Press a Note Step:  
  - Add Legato/Slide (first press of step).  
  - Add Note Off Step (second press of step).  

* Hold ACTION Button on Any Track:  
  - Small menu appears on main control screen segment display:  
    - **o/C/P**: Switch playback mode (Normal/Chain Repeat/Pattern Repeat).  
      - First letter indicator changes to reflect mode.  
    - **[ / ]**: Rotate/Nudge Steps left/right.  
    - **R**: Randomize Steps (works with ABCD variation modes).  
    - **E**: Erase Current Track Steps.  

************************************************************************

MIDI Realtime Recording:
-------------------------

* Record live to steps from MIDI IN during playback:  
  - Push PLAY button a second time while playing to enable recording.  
  - A blinking "R" indicates recording mode.  

* **ATTENTION:**  
  - Recording may conflict with pattern switching via MIDI.  
  - The MIDI channel selected for pattern switching cannot be used for live input recording.  

* **NOTE TRACKS:**  
  - Any Note On input is re-channelized to the current track's MIDI channel.  
  - Velocity is quantized into three levels (defined in setup).  
  - Note Offs are NOT recorded automatically but can be added manually.  
  - Default note length is determined by the track's setup parameter.  

* **DRUM TRACKS:**  
  - Input is quantized to the current drum note for the selected track.  
  - Velocity is quantized into three levels (defined in setup).  
  - Recording is limited to one drum track at a time (track-by-track).  

*/
