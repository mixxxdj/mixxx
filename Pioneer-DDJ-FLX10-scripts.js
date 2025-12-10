// -------------------------------------------------------------------
// ------------------- DDJ-FLX10 script file v.0.1 -------------------
// -------------------------------------------------------------------

// *************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-FLX10.
// * Mostly adapted from the DDJ-1000 mapping script
// * Authors: Marc Zischka (Zim) based on Arnold Kalambani script fo DDJ-1000
// ****************************************************************************
//
//  Implemented (as per manufacturer's manual):
//      * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//      * Browsing and loading + Waveform zoom (shift)
//      * Jogwheels, Scratching, Bending, Loop adjust
//      * Cycle Temporange
//      * Beat Sync
//      * Hot Cue Mode
//      * Beat Loop Mode
//      * Beat Jump Mode
//      * Sampler Mode
//
//  Custom (Mixxx specific mappings):
//      * BeatFX: Assigned Effect Unit 1
//                v FX_SELECT focus EFFECT1.
//                < LEFT focus EFFECT2
//                > RIGHT focus EFFECT3
//                ON/OFF toggles focused effect slot
//                SHIFT + ON/OFF disables all three effect slots.
//                SHIFT + < loads previous effect
//                SHIFT + > loads next effect
//
//      * 32 beat jump forward & back (Shift + </> CUE/LOOP CALL arrows)
//      * Toggle quantize (Shift + channel cue)
//
//  Not implemented (after discussion and trial attempts):
//      * Loop Section:
//        * -4BEAT auto loop (hacky---prefer a clean way to set a 4 beat loop
//                            from a previous position on long press)
//
//        * CUE/LOOP CALL - memory & delete (complex and not useful. Hot cues are sufficient)
//
//      * Secondary pad modes (trial attempts complex and too experimental)
//        * Keyboard mode
//        * Pad FX1
//        * Pad FX2
//        * Keyshift mode

function PioneerDDJFLX10() {}

// (not used so far)const defaultOn = 0x7F;
const defaultOff = 0x00;

const HC_dimmedColor = 64;
const HC_Color = 29;
const Black = 0;
const tempoRangePadColor = 39;
const tempoRangePadColorSelected = 44;

const Tempo6 = 0.06;
const Tempo10 = 0.10;
const Tempo16 = 0.16;
const Tempo25 = 0.25;
const Tempo50 = 0.50;
const Tempo100 = 1;

// For Job Blinking 
var jogIsONCH1 = 0;
var jogIsONCH2 = 0;
var jogIsONCH3 = 0;
var jogIsONCH4 = 0;

// Pad port

const HcCH1Port = 0x97; // Hotcue port CH1
const HcCH2Port = 0x99; // Hotcue port CH2
const HcCH3Port = 0x9B; // Hotcue port CH3
const HcCH4Port = 0x9D; // Hotcue port CH4

// For position Bar
const positionBarSpeedFactor = 25; // if == 1, the position bar is very slow...

// For Jog Time selection
var timeTypeCH1 = 0;
var timeTypeCH2 = 0;
var timeTypeCH3 = 0;
var timeTypeCH4 = 0;

// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Init / Shutdown funtion -------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

PioneerDDJFLX10.init = function (id, debugging) {
		var i = 0;
		
		// Play & Cue
		for (i=0;i < 5;i++) {
			midi.sendShortMsg(0x90+i,0x0B,defaultOff); // Play
			midi.sendShortMsg(0x90+i,0x0C,defaultOff); // Cue
		}
		
		// Hotcue off
		for (i=0;i < 7;i++) {	
			midi.sendShortMsg(0x97,i,Black);
		}
		
		// Loop Button
		for (i=0;i < 5;i++) {	
			midi.sendShortMsg(0x90+i,0x10,0x7F);
			midi.sendShortMsg(0x90+i,0x11,0x7F); // Loop Out
			midi.sendShortMsg(0x90+i,0x14,0x00);
		}
			

		// Init for tempo Range
			engine.setValue("[Channel1]", "rateRange", Tempo6);
			engine.setValue("[Channel2]", "rateRange", Tempo6);
			engine.setValue("[Channel3]", "rateRange", Tempo6);
			engine.setValue("[Channel4]", "rateRange", Tempo6);
		
			
		// Callback connections
		// ------ VuMeters
		engine.connectControl("[Channel1]","VuMeter","PioneerDDJFLX10.LedVuMeterCH1");
		engine.connectControl("[Channel2]","VuMeter","PioneerDDJFLX10.LedVuMeterCH2");
		engine.connectControl("[Channel3]","VuMeter","PioneerDDJFLX10.LedVuMeterCH3");
		engine.connectControl("[Channel4]","VuMeter","PioneerDDJFLX10.LedVuMeterCH4");
		
		// ------ Loop
		// Enabled callbacks
		engine.connectControl("[Channel1]","loop_enabled","PioneerDDJFLX10.LoopSetFeedbackCH1");
		engine.connectControl("[Channel2]","loop_enabled","PioneerDDJFLX10.LoopSetFeedbackCH2");
		engine.connectControl("[Channel3]","loop_enabled","PioneerDDJFLX10.LoopSetFeedbackCH3");
		engine.connectControl("[Channel4]","loop_enabled","PioneerDDJFLX10.LoopSetFeedbackCH4");
		
		// ------ Track Time
		engine.connectControl("[Channel1]", "playposition","PioneerDDJFLX10.TrackTimeUpdateChannel1");
		engine.connectControl("[Channel2]", "playposition","PioneerDDJFLX10.TrackTimeUpdateChannel2");
		engine.connectControl("[Channel3]", "playposition","PioneerDDJFLX10.TrackTimeUpdateChannel3");
		engine.connectControl("[Channel4]", "playposition","PioneerDDJFLX10.TrackTimeUpdateChannel4");
		
		// ------ Actual BPM & Tempo range On Jog
		engine.connectControl("[Channel1]","bpm","PioneerDDJFLX10.BpmOnJogCH1"); // Update Bpm + Tempo Range
		engine.connectControl("[Channel2]","bpm","PioneerDDJFLX10.BpmOnJogCH2"); // Update Bpm + Tempo Range
		engine.connectControl("[Channel3]","bpm","PioneerDDJFLX10.BpmOnJogCH3"); // Update Bpm + Tempo Range
		engine.connectControl("[Channel4]","bpm","PioneerDDJFLX10.BpmOnJogCH4"); // Update Bpm + Tempo Range
		
		// Timer
		// ------ End of track blinking
		engine.beginTimer(250,"PioneerDDJFLX10.EndOfTrackBlink()");
		
		// Timer for Loop Roll flashing
		engine.beginTimer(250, "PioneerDDJFLX10.LoopRollBlink()");

	
    PioneerDDJFLX10.HotCueTriggerInit();
		
		// Jog Information on
		for (i=0;i < 4;i++) {
			midi.sendShortMsg(0x90+i,0x5D,0x00);
			midi.sendShortMsg(0x90+i,0x5B,0x01);
		}
		
	};
	
PioneerDDJFLX10.shutdown = function() {
		var i = 0;
		
		for (i=0;i < 5;i++) {
			midi.sendShortMsg(0x90+i,0x0B,defaultOff); // Play
			midi.sendShortMsg(0x90+i,0x0C,defaultOff); // Cue
		}
		
		for (i=0;i < 8;i++) {	
			midi.sendShortMsg(0x97,i,Black);
		}
		
		// Loop Button
		for (i=0;i < 5;i++) {	
			midi.sendShortMsg(0x90+i,0x10,0x00);
			midi.sendShortMsg(0x90+i,0x11,0x00); // Loop Out
			midi.sendShortMsg(0x90+i,0x14,0x00);
		}
		
			
		engine.stopTimer("PioneerDDJFLX10.EndOfTrackBlink()");
		
		// Jog Information off
		for (i=0;i < 4;i++) {
			midi.sendShortMsg(0x90+i,0x5B,0x00);
			midi.sendShortMsg(0x90+i,0x5D,0x7F);
		}
		
	};
	
// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Functions for init ------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
	
	// -------------- HotCue
	PioneerDDJFLX10.HotCueTriggerInit = function () {	// Callback setting for Hotcue illumation
		
		// Hotcue illumination triggers - CH1
		engine.connectControl("[Channel1]","hotcue_1_enabled","PioneerDDJFLX10.HotCue1TriggerCH1");// HotCues 1 CH1
		engine.connectControl("[Channel1]","hotcue_2_enabled","PioneerDDJFLX10.HotCue2TriggerCH1");// HotCues 2 CH1
		engine.connectControl("[Channel1]","hotcue_3_enabled","PioneerDDJFLX10.HotCue3TriggerCH1");// HotCues 3 CH1
		engine.connectControl("[Channel1]","hotcue_4_enabled","PioneerDDJFLX10.HotCue4TriggerCH1");// HotCues 4 CH1
		engine.connectControl("[Channel1]","hotcue_5_enabled","PioneerDDJFLX10.HotCue5TriggerCH1");// HotCues 5 CH1
		engine.connectControl("[Channel1]","hotcue_6_enabled","PioneerDDJFLX10.HotCue6TriggerCH1");// HotCues 6 CH1
		engine.connectControl("[Channel1]","hotcue_7_enabled","PioneerDDJFLX10.HotCue7TriggerCH1");// HotCues 7 CH1
		engine.connectControl("[Channel1]","hotcue_8_enabled","PioneerDDJFLX10.HotCue8TriggerCH1");// HotCues 8 CH1
		
		// Hotcue illumination triggers - CH2
		engine.connectControl("[Channel2]","hotcue_1_enabled","PioneerDDJFLX10.HotCue1TriggerCH2");// HotCues 1 CH2
		engine.connectControl("[Channel2]","hotcue_2_enabled","PioneerDDJFLX10.HotCue2TriggerCH2");// HotCues 2 CH2
		engine.connectControl("[Channel2]","hotcue_3_enabled","PioneerDDJFLX10.HotCue3TriggerCH2");// HotCues 3 CH2
		engine.connectControl("[Channel2]","hotcue_4_enabled","PioneerDDJFLX10.HotCue4TriggerCH2");// HotCues 4 CH2
		engine.connectControl("[Channel2]","hotcue_5_enabled","PioneerDDJFLX10.HotCue5TriggerCH2");// HotCues 5 CH2
		engine.connectControl("[Channel2]","hotcue_6_enabled","PioneerDDJFLX10.HotCue6TriggerCH2");// HotCues 6 CH2
		engine.connectControl("[Channel2]","hotcue_7_enabled","PioneerDDJFLX10.HotCue7TriggerCH2");// HotCues 7 CH2
		engine.connectControl("[Channel2]","hotcue_8_enabled","PioneerDDJFLX10.HotCue8TriggerCH2");// HotCues 8 CH2
		
		// Hotcue illumination triggers - CH3
		engine.connectControl("[Channel3]","hotcue_1_enabled","PioneerDDJFLX10.HotCue1TriggerCH3");// HotCues 1 CH3
		engine.connectControl("[Channel3]","hotcue_2_enabled","PioneerDDJFLX10.HotCue2TriggerCH3");// HotCues 2 CH3
		engine.connectControl("[Channel3]","hotcue_3_enabled","PioneerDDJFLX10.HotCue3TriggerCH3");// HotCues 3 CH3
		engine.connectControl("[Channel3]","hotcue_4_enabled","PioneerDDJFLX10.HotCue4TriggerCH3");// HotCues 4 CH3
		engine.connectControl("[Channel3]","hotcue_5_enabled","PioneerDDJFLX10.HotCue5TriggerCH3");// HotCues 5 CH3
		engine.connectControl("[Channel3]","hotcue_6_enabled","PioneerDDJFLX10.HotCue6TriggerCH3");// HotCues 6 CH3
		engine.connectControl("[Channel3]","hotcue_7_enabled","PioneerDDJFLX10.HotCue7TriggerCH3");// HotCues 7 CH3
		engine.connectControl("[Channel3]","hotcue_8_enabled","PioneerDDJFLX10.HotCue8TriggerCH3");// HotCues 8 CH3
		
		// Hotcue illumination triggers - CH4
		engine.connectControl("[Channel4]","hotcue_1_enabled","PioneerDDJFLX10.HotCue1TriggerCH4");// HotCues 1 CH4
		engine.connectControl("[Channel4]","hotcue_2_enabled","PioneerDDJFLX10.HotCue2TriggerCH4");// HotCues 2 CH4
		engine.connectControl("[Channel4]","hotcue_3_enabled","PioneerDDJFLX10.HotCue3TriggerCH4");// HotCues 3 CH4
		engine.connectControl("[Channel4]","hotcue_4_enabled","PioneerDDJFLX10.HotCue4TriggerCH4");// HotCues 4 CH4
		engine.connectControl("[Channel4]","hotcue_5_enabled","PioneerDDJFLX10.HotCue5TriggerCH4");// HotCues 5 CH4
		engine.connectControl("[Channel4]","hotcue_6_enabled","PioneerDDJFLX10.HotCue6TriggerCH4");// HotCues 6 CH4
		engine.connectControl("[Channel4]","hotcue_7_enabled","PioneerDDJFLX10.HotCue7TriggerCH4");// HotCues 7 CH4
		engine.connectControl("[Channel4]","hotcue_8_enabled","PioneerDDJFLX10.HotCue8TriggerCH4");// HotCues 8 CH4
	};




// ---------------------------------------------------------------------------------------------------------------
// ------------------------- All propose functions ---------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

	PioneerDDJFLX10.ColorTrigger = function (value, midiPort, midiNumber, OnColor, OffColor) {	// For callback functions dimmed LED
		if (value == 1) { // if touch pressed
			midi.sendShortMsg(midiPort,midiNumber,OnColor);
			midi.sendShortMsg(midiPort+1,midiNumber,OnColor); // For Shift illumination
		} else { // if touch released
			midi.sendShortMsg(midiPort,midiNumber,OffColor);
			midi.sendShortMsg(midiPort+1,midiNumber,OffColor); // For Shift illumination
		}
	};
	
	PioneerDDJFLX10.sensitivityMinimizer = function (value, factor) { // factor must be lower than value !!!
		return (value/factor);
	};

	PioneerDDJFLX10.sensitivityMaximizer = function (value, factor) { // factor value not to high !!!
		return (value*factor);
	};
	
	// Dimmed function
	PioneerDDJFLX10.DimButton = function (value, midiPort, midiNumber, dimColor, normalColor) {	
		if (value == 0x7F) { // if touch pressed
			midi.sendShortMsg(midiPort,midiNumber,dimColor);
		} else { // if touch released
			midi.sendShortMsg(midiPort,midiNumber,normalColor);
		}
	};


// ---------------------------------------------------------------------------------------------------------------
// ------------------------- HotCue Illumination -----------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

	// ---- CH 1
	PioneerDDJFLX10.HotCue1TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x00, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue2TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x01, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue3TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x02, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue4TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x03, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue5TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x04, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue6TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x05, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue7TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x06, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue8TriggerCH1 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH1Port, 0x07, HC_Color, Black);
	};
	
	// ---- CH 2
	PioneerDDJFLX10.HotCue1TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x00, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue2TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x01, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue3TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x02, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue4TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x03, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue5TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x04, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue6TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x05, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue7TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x06, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue8TriggerCH2 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH2Port, 0x07, HC_Color, Black);
	};
	
	// ---- CH 3
	PioneerDDJFLX10.HotCue1TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x00, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue2TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x01, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue3TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x02, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue4TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x03, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue5TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x04, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue6TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x05, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue7TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x06, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue8TriggerCH3 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH3Port, 0x07, HC_Color, Black);
	};
	
	// ---- CH 4
	PioneerDDJFLX10.HotCue1TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x00, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue2TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x01, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue3TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x02, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue4TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x03, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue5TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x04, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue6TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x05, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue7TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x06, HC_Color, Black);
	};
	
	PioneerDDJFLX10.HotCue8TriggerCH4 = function (value, group, control) {
		PioneerDDJFLX10.ColorTrigger (value, HcCH4Port, 0x07, HC_Color, Black);
	};
	
	
	
	// PioneerDDJFLX10.HotCueTouch -> Pads have to be dimmed ?
	PioneerDDJFLX10.HotCueTouch = function (channel, control, value, status, group) {
		
		switch (control) { // DimOnly if Hotcue is set
			case 0 :	if (engine.getValue(group, "hotcue_1_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 1 :	if (engine.getValue(group, "hotcue_2_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 2 :	if (engine.getValue(group, "hotcue_3_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 3 :	if (engine.getValue(group, "hotcue_4_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 4 :	if (engine.getValue(group, "hotcue_5_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 5 :	if (engine.getValue(group, "hotcue_6_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 6 :	if (engine.getValue(group, "hotcue_7_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
			case 7 :	if (engine.getValue(group, "hotcue_8_enabled")) {
							PioneerDDJFLX10.DimButton(value, status, control, HC_dimmedColor, HC_Color);
						}
						break;
		}
	};
	
	

// ---------------------------------------------------------------------------------------------------------------
// ------------------------- PreFader Level ----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

	PioneerDDJFLX10.LedVuMeterCH1 = function (value, group, control) {
		midi.sendShortMsg(0xB0,0x02,value*127);
	};
	
	PioneerDDJFLX10.LedVuMeterCH2 = function (value, group, control) {
		midi.sendShortMsg(0xB1,0x02,value*127);
	};
	
	PioneerDDJFLX10.LedVuMeterCH3 = function (value, group, control) {
		midi.sendShortMsg(0xB2,0x02,value*127);
	};
	
	PioneerDDJFLX10.LedVuMeterCH4 = function (value, group, control) {
		midi.sendShortMsg(0xB3,0x02,value*127);
	};



// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Scratch, Pitch bend -----------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

	// For the button that enables/disables scratching
    PioneerDDJFLX10.wheelTouch = function (channel, control, value, status, group) {  
        var deckNumber = script.deckFromGroup(group);
      if (value == 0x7F) { 
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(deckNumber, 32767, 33+1/3, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(deckNumber);
        }
    };
     
    // The wheel that actually controls the scratching
    PioneerDDJFLX10.wheelTurn = function (channel, control, value, status, group) { 
        var newValue = value - 64;
        var deckNumber = script.deckFromGroup(group);
		
        if (engine.isScratching(deckNumber)) {
            engine.scratchTick(deckNumber, PioneerDDJFLX10.sensitivityMaximizer(newValue,1.5)); // Scratch!
        } else {
			engine.setValue(group, 'jog', PioneerDDJFLX10.sensitivityMinimizer(newValue,16)); // Pitch bend
        }
    };
	
	
	
// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Jog illumination and Information ----------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
	
	// ----- End of Track Warning
	
	PioneerDDJFLX10.EndOfTrackBlink  = function () { // call function for blinking, to spare "timer"
		PioneerDDJFLX10.EndOfTrackBlinkCH1();
		PioneerDDJFLX10.EndOfTrackBlinkCH2();
		PioneerDDJFLX10.EndOfTrackBlinkCH3();
		PioneerDDJFLX10.EndOfTrackBlinkCH4();
	};
	
	PioneerDDJFLX10.EndOfTrackBlinkCH1 = function () {
		if (engine.getValue("[Channel1]","end_of_track")) {
			if (jogIsONCH1) {
				midi.sendShortMsg(0x90,0x5B,0x01);
				jogIsONCH1 = 0;
			} else {
				midi.sendShortMsg(0x90,0x5B,0x00);
				jogIsONCH1 = 1;
			}
		} else {
			midi.sendShortMsg(0x90,0x5B,0x01);
		}
	};
	
	PioneerDDJFLX10.EndOfTrackBlinkCH2 = function () {
		if (engine.getValue("[Channel2]","end_of_track")) {
			if (jogIsONCH2) {
				midi.sendShortMsg(0x91,0x5B,0x01);
				jogIsONCH2 = 0;
			} else {
				midi.sendShortMsg(0x91,0x5B,0x00);
				jogIsONCH2 = 1;
			}
		} else {
			midi.sendShortMsg(0x91,0x5B,0x01);
		}
	};
	
	PioneerDDJFLX10.EndOfTrackBlinkCH3 = function () {
		if (engine.getValue("[Channel3]","end_of_track")) {
			if (jogIsONCH3) {
				midi.sendShortMsg(0x92,0x5B,0x01);
				jogIsONCH3 = 0;
			} else {
				midi.sendShortMsg(0x92,0x5B,0x00);
				jogIsONCH3 = 1;
			}
		} else {
			midi.sendShortMsg(0x92,0x5B,0x01);
		}
	};
	
	PioneerDDJFLX10.EndOfTrackBlinkCH4 = function () {
		if (engine.getValue("[Channel4]","end_of_track")) {
			if (jogIsONCH4) {
				midi.sendShortMsg(0x93,0x5B,0x01);
				jogIsONCH4 = 0;
			} else {
				midi.sendShortMsg(0x93,0x5B,0x00);
				jogIsONCH4 = 1;
			}
		} else {
			midi.sendShortMsg(0x93,0x5B,0x01);
		}
	};
	
	// Active Deck Detection  
	PioneerDDJFLX10.activeDeck = 1;

	PioneerDDJFLX10.setActiveDeck = function(deck) {
   	 PioneerDDJFLX10.activeDeck = deck;
   	 print("Deck actif : " + deck);
	};

	// ----- Time
	
	PioneerDDJFLX10.TrackTimeUpdateChannel1 = function (value, group, control){    // For channel 1
		PioneerDDJFLX10.TrackTimeDisplay(1, value, group, timeTypeCH1);
	};
	
	PioneerDDJFLX10.TrackTimeUpdateChannel2 = function (value, group, control){    // For channel 2
		PioneerDDJFLX10.TrackTimeDisplay(2, value, group, timeTypeCH2);
	};
	
	PioneerDDJFLX10.TrackTimeUpdateChannel3 = function (value, group, control){    // For channel 3
		PioneerDDJFLX10.TrackTimeDisplay(3, value, group, timeTypeCH3);
	};
	
	PioneerDDJFLX10.TrackTimeUpdateChannel4 = function (value, group, control){    // For channel 4
		PioneerDDJFLX10.TrackTimeDisplay(4, value, group, timeTypeCH4);	
	};
	
	PioneerDDJFLX10.TrackTimeDisplay = function (channel, value, group, timeType) {	
		var midiPort = 0x90 + (channel-1);
		var trackDuration = engine.getValue(group, "duration"); // get total time
		var timeLeft = trackDuration * (1.0 - value); // compute time left
		var actualTime = trackDuration * value;
		var minutesLeft = timeLeft/60; 
		var secondesLeft = timeLeft%60; // modulo to get seconde...
		var actualMinutes = actualTime/60; 
		var actualSecondes = actualTime%60; // modulo to get seconde...
		
		if (timeType) { // True -> remaining time ; False -> elapsed time
			midi.sendShortMsg(midiPort,0x44,0x7F); // Show "minus"
			midi.sendShortMsg(midiPort,0x42,minutesLeft); // send minutes
			midi.sendShortMsg(midiPort,0x43,secondesLeft); // send secondes
		} else {
			midi.sendShortMsg(midiPort,0x44,0x00); // Hide "minus"
			midi.sendShortMsg(midiPort,0x42,actualMinutes); // send minutes
			midi.sendShortMsg(midiPort,0x43,actualSecondes); // send secondes
		}
		
		PioneerDDJFLX10.CurrentPositionBarUpdate(channel,actualSecondes); // Update Position Bar for channel 4
	};
	
	PioneerDDJFLX10.CurrentPositionBarUpdate = function (channelNumber,value){ // David Goodenough
		var currentBarPosition = value*6*positionBarSpeedFactor;
		var channelNbrCode = 176+(channelNumber-1);
		var valueOnA360Circle = currentBarPosition%360;
		var MSB = Math.floor(valueOnA360Circle/128);
		var LSB = valueOnA360Circle - (128*MSB);
		
		midi.sendShortMsg(channelNbrCode,0x14,MSB);
		midi.sendShortMsg(channelNbrCode,0x34,LSB);
		
	};
	
	PioneerDDJFLX10.TimeTypeChangeGlobal = function (channel, control, value, status, group) { 
		if (value == 0x7F) {
			switch (status) {
				case 0x90 : PioneerDDJFLX10.TimeTypeChange(1); break;
				case 0x91 : PioneerDDJFLX10.TimeTypeChange(2); break;
				case 0x92 : PioneerDDJFLX10.TimeTypeChange(3); break;
				case 0x93 : PioneerDDJFLX10.TimeTypeChange(4); break;
			}
		}
	};
	
	
	PioneerDDJFLX10.TimeTypeChange = function (channel) {	
		if (channel == 1) {
			switch (timeTypeCH1) {
				case 0 : timeTypeCH1 = 1; break;
				case 1 : timeTypeCH1 = 0; break;
				default : timeTypeCH1 = 0; break;
			}
		}
		
		if (channel == 2) {
			switch (timeTypeCH2) {
				case 0 : timeTypeCH2 = 1; break;
				case 1 : timeTypeCH2 = 0; break;
				default : timeTypeCH2 = 0; break;
			}
		}
		
		if (channel == 3) {
			switch (timeTypeCH3) {
				case 0 : timeTypeCH3 = 1; break;
				case 1 : timeTypeCH3 = 0; break;
				default : timeTypeCH3 = 0; break;
			}
		}
		
		if (channel == 4) {
			switch (timeTypeCH4) {
				case 0 : timeTypeCH4 = 1; break;
				case 1 : timeTypeCH4 = 0; break;
				default : timeTypeCH4 = 0; break;
			}
		}
		
	};
	
	// ----- Rate range & Track Tempo display
	
	PioneerDDJFLX10.BpmOnJogCH1 = function (value, group, control) {  // For Track BPM and Pitch range
		PioneerDDJFLX10.DisplayBpmAndPitchRange(1, value, group); // Display Bpm 
	};
	
	PioneerDDJFLX10.BpmOnJogCH2 = function (value, group, control) {  // For Track BPM and Pitch range
		PioneerDDJFLX10.DisplayBpmAndPitchRange(2, value, group); // Display Bpm 
	};
	
	PioneerDDJFLX10.BpmOnJogCH3 = function (value, group, control) {  // For Track BPM and Pitch range
		PioneerDDJFLX10.DisplayBpmAndPitchRange(3, value, group); // Display Bpm 
	};
	
	PioneerDDJFLX10.BpmOnJogCH4 = function (value, group, control) {  // For Track BPM and Pitch range
		PioneerDDJFLX10.DisplayBpmAndPitchRange(4, value, group); // Display Bpm 
	};
	
	PioneerDDJFLX10.DisplayBpmAndPitchRange = function (channel, value, group) {
		
		var midiPort = 0xB0 + (channel-1);
		var absoluteBPM = Math.floor(value*10);
		var actualTempo = engine.getValue(group,"rateRange");
		var actualRate = engine.getValue(group, "rate");
		var absolutePitchRange = Math.floor(((actualTempo * 10) * actualRate) * 100);
		var adaptPitchRange = 0;
		var MSB = 0;
		var LSB = 0;
		
		// Bpm value update
		
		if (value == 0) {
			MSB = 0;
			LSB = 0;
		} else {
			MSB = Math.floor(absoluteBPM / 128);
			LSB = absoluteBPM % 128;
		}
		
		midi.sendShortMsg(midiPort,0x15,MSB);
		midi.sendShortMsg(midiPort,0x35,LSB);
		
		// Pitch Range update

		if (absolutePitchRange < 0) {
			adaptPitchRange = 1000 - absolutePitchRange;
			MSB = Math.floor(adaptPitchRange / 128);
			LSB = adaptPitchRange % 128;
		} else {
			adaptPitchRange = 1000 + absolutePitchRange;
			MSB = Math.floor(adaptPitchRange / 128);
			LSB = adaptPitchRange % 128;
		}
		
		midi.sendShortMsg(midiPort,0x16,MSB);
		midi.sendShortMsg(midiPort,0x36,LSB);
	};
	
	
		
// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Loop State --------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
	
	PioneerDDJFLX10.DimLoopButton = function (channel, control, value, status, group) {
		PioneerDDJFLX10.DimButton(value, status, control, 0, 0x7F);
	};
	
	PioneerDDJFLX10.LoopSetFeedbackCH1 = function (value, group, control) {
		midi.sendShortMsg(0x90,0x14,0x7F);
	};
	
	PioneerDDJFLX10.LoopSetFeedbackCH2 = function (value, group, control) {
		midi.sendShortMsg(0x91,0x14,0x7F);
	};
	
	PioneerDDJFLX10.LoopSetFeedbackCH3 = function (value, group, control) {
		midi.sendShortMsg(0x92,0x14,0x7F);
	};
	
	PioneerDDJFLX10.LoopSetFeedbackCH4 = function (value, group, control) {
		midi.sendShortMsg(0x93,0x14,0x7F);
	};
	
// -----------------------------
// SHIFT state management
// -----------------------------
PioneerDDJFLX10.shiftActive = false;

// Called from XML (script-binding) on shift press
PioneerDDJFLX10.shiftPressed = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJFLX10.shiftActive = true;
        // print("Shift pressed");
    }
};

// Called from XML (script-binding) on shift release
PioneerDDJFLX10.shiftReleased = function(channel, control, value, status, group) {
    if (value === 0x00 || value === 0) {
        PioneerDDJFLX10.shiftActive = false;
        // print("Shift released");
    }
};
// DEBUG LOOP
PioneerDDJFLX10._debugLoopButtons = function(channel, control, value, status, group) {
    print("DEBUG LOOP BUTTON: status=" + status.toString(16)
        + " control=" + control.toString(16)
        + " value=" + value
        + " group=" + group);
};

// -----------------------------
// SHIFT + LOOP actions 
// -----------------------------
PioneerDDJFLX10.LoopHalveShift = function(channel, control, value, status, group) {
    if (value !== 0x7F) return; // only on press
    try {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_halve", 1);
            print("LoopHalveShift: triggered on " + group);
        } else {
            // No-op: normal Loop In is handled by the XML mapping to loop_in
            // print("LoopHalveShift: shift not active, ignoring");
        }
    } catch (e) {
        print("LoopHalveShift ERROR: " + e);
    }
};

PioneerDDJFLX10.LoopDoubleShift = function(channel, control, value, status, group) {
    if (value !== 0x7F) return; // only on press
    try {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_double", 1);
            print("LoopDoubleShift: triggered on " + group);
        } else {
            // No-op: normal Loop Out is handled by the XML mapping to loop_out
            // print("LoopDoubleShift: shift not active, ignoring");
        }
    } catch (e) {
        print("LoopDoubleShift ERROR: " + e);
    }
};

// ----------------------------------------------------------------------
// LOOP ROLL – LED BLINK ON Loop IN + OUT
// ----------------------------------------------------------------------

PioneerDDJFLX10._rollBlinkState = 0;

PioneerDDJFLX10.LoopRollBlink = function () {
    PioneerDDJFLX10._rollBlinkState ^= 1; // Toggle 0 ↔ 1 each timer cycle

    // Check per deck
    PioneerDDJFLX10._LoopRollBlinkDeck(1);
    PioneerDDJFLX10._LoopRollBlinkDeck(2);
    PioneerDDJFLX10._LoopRollBlinkDeck(3);
    PioneerDDJFLX10._LoopRollBlinkDeck(4);
};

// Helper: blink Loop In + Loop Out for a deck if roll active
PioneerDDJFLX10._LoopRollBlinkDeck = function (deck) {

    var group = "[Channel" + deck + "]";
    var rollSize = engine.getValue(group, "beatlooproll_size");

    // MIDI status per deck for buttons (0x90 + deck-1)
    var status = 0x90 + (deck - 1);

    // Loop IN = 0x10, Loop OUT = 0x11
    var loopIn = 0x10;
    var loopOut = 0x11;

    if (rollSize > 0) {  
        // Loop Roll active → blink both buttons
        var led = PioneerDDJFLX10._rollBlinkState ? 0x7F : 0x00;
        midi.sendShortMsg(status, loopIn, led);
        midi.sendShortMsg(status, loopOut, led);
    } else {
        // No roll → restore normal LED state handled by Mixxx loop_enabled callback
        // DO NOT overwrite if loop enabled
        var isLoop = engine.getValue(group, "loop_enabled");

        midi.sendShortMsg(status, loopIn, isLoop ? 0x7F : 0x00);
        midi.sendShortMsg(status, loopOut, isLoop ? 0x7F : 0x00);
    }
};

// ---------------------------------------------------------------------------------------------------------------
// -------------------------FX buttons for Mixxx effects -------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
		
	PioneerDDJFLX10.fxButton = function(channel, control, value, status, group) {
   	 if (value) {
      	  engine.setValue("[EffectRack1_EffectUnit1]", "enabled", !engine.getValue("[EffectRack1_EffectUnit1]", "enabled"));
 	   }
	};

// ---------------------------------------------------------------------------------------------------------------
// ------------------------- Rate Range Selector ----------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

	PioneerDDJFLX10.RangeSelector = function (channel, control, value, status, group) {
		if (value == 0x7F) {
			switch(engine.getValue(group,"rateRange")) {
				case Tempo6: engine.setValue(group,"rateRange",Tempo10); break;
				case Tempo10: engine.setValue(group,"rateRange",Tempo16); break;
				case Tempo16: engine.setValue(group,"rateRange",Tempo25); break;
				case Tempo25: engine.setValue(group,"rateRange",Tempo50); break;
				case Tempo50: engine.setValue(group,"rateRange",Tempo100); break;
				case Tempo100: engine.setValue(group,"rateRange",Tempo6); break;
			}
		}
	};

// ---------------------------------------------------------------------------------------
// ------------ Rate MSB/LSB handler (14-bit) with Pioneer inverted mode --------------
//----------------------------------------------------------------------------------------

PioneerDDJFLX10._rateMSB = {1:0,2:0,3:0,4:0};

PioneerDDJFLX10._rate14bitToNormalized = function(value14) {
    return (value14 / 16383.0) * 2.0 - 1.0;
};

PioneerDDJFLX10._writeRateToEngine = function(deck, value14) {
    var norm = PioneerDDJFLX10._rate14bitToNormalized(value14);
    norm = -norm; // inversion Pioneer (fader vers le bas = tempo plus rapide)
    engine.setValue("[Channel" + deck + "]", "rate", norm);
};

PioneerDDJFLX10.rate_msb = function(channel, control, value, status, group) {
    var deck = parseInt(group.replace(/[^0-9]/g,'')) || 1;
    PioneerDDJFLX10._rateMSB[deck] = value & 0x7F;
};

PioneerDDJFLX10.rate_lsb = function(channel, control, value, status, group) {
    var deck = parseInt(group.replace(/[^0-9]/g,'')) || 1;
    var msb = PioneerDDJFLX10._rateMSB[deck] || 0;
    var lsb = value & 0x7F;
    var combined = (msb << 7) | lsb;
    PioneerDDJFLX10._writeRateToEngine(deck, combined);
};

// ---------------------------------------------------------------------------------------
// ------------------------- Tempo Reset (XML: rate_reset) --------------------------------
// ---------------------------------------------------------------------------------------
PioneerDDJFLX10.rate_reset = function(channel, control, value, status, group) {
    if (value !== 0x7F) return; // sécurité : n’agir que sur pression

    try {
        // Identifier le deck
        var deck = script.deckFromGroup(group);

        // Remet le tempo à 0 exactement
        engine.setValue("[Channel" + deck + "]", "rate", 0.0);

        // Allume la LED si disponible
        // (certains mappings utilisent status=0x9X et control=0x41)
        midi.sendShortMsg(status, control, 0x7F);

        // Debug optionnel
        // print("Tempo Reset sur deck " + deck);

    } catch (err) {
        print("Erreur dans rate_reset : " + err);
    }
};
//----------------------------------------------------------------------------------------
// ==========  Active Deck with number ==========
//----------------------------------------------------------------------------------------
PioneerDDJFLX10.setActiveDeck = function(deck) {
    try {
        // Conversion et sécurisation du numéro de deck
        var deckNum = parseInt(deck);
        if (isNaN(deckNum) || deckNum < 1 || deckNum > 4) {
            deckNum = 1; // fallback
        }

        PioneerDDJFLX10.activeDeck = deckNum;

        // Lecture du titre du morceau
        var title = engine.getValue("[Channel" + deckNum + "]", "track_title");
        if (!title || title === "") {
            title = "Aucun morceau chargé";
        }

        // Chaîne complète affichée dans la barre de titre
        var windowTitle = "Deck " + deckNum + " actif  -  " + title;

        // Forçage de la mise à jour du titre de la fenêtre Mixxx
        engine.setValue("[Master]", "title", windowTitle);

        // Indication visuelle courte (flash vumètre)
        engine.setValue("[Channel" + deckNum + "]", "peakIndicator", true);
        engine.beginTimer(200, function() {
            engine.setValue("[Channel" + deckNum + "]", "peakIndicator", false);
        }, true);

    } catch (err) {
        print("Erreur setActiveDeck : " + err);
    }
};

// -------------------------------------------
// Beatjump 4 / 16 with Shift
// -------------------------------------------
// Beatjump handler: uses beatjump_size + beatjump_forward/backward (works in Mixxx 2.3.x)
PioneerDDJFLX10.beatjumpHandler = function(channel, control, value, status, group) {
    if (value !== 0x7F) return; // only on press

    // detect deck from status if group not present
    var deck = 1;
    if (typeof status !== "undefined") {
        deck = (status & 0x0F) + 1;
    } else if (typeof group === "string") {
        var m = group.match(/\d+/);
        if (m) deck = parseInt(m[0], 10);
    }
    var grp = "[Channel" + deck + "]";

    // Determine jump size: 4 or 16 when shift active
    var jumpSize = PioneerDDJFLX10.shiftActive ? 16 : 4;

    // Determine direction from control (midino)
    // For deck 1 you said 0x70 = backward, 0x71 = forward
    // If your hardware uses same midino for all decks, control value is same across decks.
    var backward = (control === 0x70);

    try {
        // Set the jump size first
        engine.setValue(grp, "beatjump_size", jumpSize);

        // Then trigger forward/backward push
        if (backward) {
            engine.setValue(grp, "beatjump_backward", 1);
        } else {
            engine.setValue(grp, "beatjump_forward", 1);
        }
    } catch (e) {
        print("beatjumpHandler ERROR: " + e);
    }
};

// ------------------------------------------------------
// SLIP REVERSE (momentary) + REVERSE (toggle) with SHIFT
// ------------------------------------------------------
// Reverse / Slip-Reverse handler (press + release)
// Single physical button: press = slip-reverse while held, shift+press = toggle reverse
PioneerDDJFLX10.reverseHandler = function(channel, control, value, status, group) {
// Determine deck & group even if XML doesn't pass group
    var deck = 1;
    if (typeof status !== "undefined") {
        deck = (status & 0x0F) + 1;
    } else if (typeof group === "string") {
        var m = group.match(/\d+/);
        if (m) deck = parseInt(m[0], 10);
    }
    var grp = "[Channel" + deck + "]";

    // Pressed
    if (value === 0x7F) {
        // If SHIFT not active -> slip reverse (momentary)
        if (!PioneerDDJFLX10.shiftActive) {
            try {
                // Enable slip first
                engine.setValue(grp, "slip_enabled", 1);
                // Then start reverse playback
                engine.setValue(grp, "reverse", 1);
            } catch (e) {
                print("reverseHandler press ERROR: " + e);
            }
        } else {
            // SHIFT active -> toggle reverse (persistent), no slip
            try {
                var cur = engine.getValue(grp, "reverse");
                if (cur) {
                    engine.setValue(grp, "reverse", 0);
                    engine.setValue(grp, "slip_enabled", 0);
                } else {
                    engine.setValue(grp, "slip_enabled", 0);
                    engine.setValue(grp, "reverse", 1);
                }
            } catch (e) {
                print("reverseHandler SHIFT toggle ERROR: " + e);
            }
        }
        return;
    }

    // Released (value == 0 or other)
    if (value === 0x00 || value === 0) {
        // On release we must stop slip-reverse if we were in momentary mode
        // If SHIFT was pressed at press time, behavior already toggled on press; release shouldn't change that.
        try {
            // Ensure reverse is off and slip disabled for momentary case.
            // We check if shift is inactive now (common case); even if shift changed meanwhile, this is safe.
            if (!PioneerDDJFLX10.shiftActive) {
                engine.setValue(grp, "reverse", 0);
                engine.setValue(grp, "slip_enabled", 0);
            }
            // If SHIFT toggled reverse, nothing to do on release.
        } catch (e) {
            print("reverseHandler release ERROR: " + e);
        }
    }
};