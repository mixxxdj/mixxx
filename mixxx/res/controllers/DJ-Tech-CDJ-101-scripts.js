/****************************************************************/
/*      DJ-Tech CDJ101 controller script                        */
/*      For Mixxx version 1.11                                  */
/*      Author: zestoi                                          */
/****************************************************************/
//
// pitch slider works as you'd expect
// holding down the push button works as a "shift" to activate secondary functions
// needs the cdj101 to be on midi channel 1 for deck1 and 2 for deck2
//
// track playing:
//
// * surface of jog wheel: scratch
// * edge of jog wheel: tempo bend
// * shift + edge of jog wheel: fine tempo bend
//
// track not playing:
//
// * click push button: load a track
// * surface of jog wheel: scratch thru track
// * shift + surface of jog wheel: scan through track quickly
// * edge of jog wheel: jog though track slowly
// * shift + cue: move beatgrid
//
// track playing or not:
//
// * rotate push button: navigate tracks
// * shift + rotate push button: switch between playlists
// * cue button: default cue behaviour
// * play: toggle play
// * shift + play: sync
// 

DJTechCDJ101 = {};
DJTechCDJ101.pushmaxtime = 200;
DJTechCDJ101.pushed = false;
DJTechCDJ101.pushedon = 0;
DJTechCDJ101.outer2inner = false;
DJTechCDJ101.scratch_timeout = 100;

DJTechCDJ101.init = function(id) {
	engine.connectControl("[Channel1]", "play", "DJTechCDJ101.play_feedback_deck1");
	engine.connectControl("[Channel2]", "play", "DJTechCDJ101.play_feedback_deck2");
	engine.connectControl("[Channel1]", "cue_default", "DJTechCDJ101.cue_feedback_deck1");
	engine.connectControl("[Channel2]", "cue_default", "DJTechCDJ101.cue_feedback_deck2");
}

DJTechCDJ101.shutdown = function() {}

//
// utility function
//

DJTechCDJ101.mtime = function() 
{
	var t = new Date();
	return t.getTime();
}

//
// cue+play led feedback
//

DJTechCDJ101.play_feedback_deck1 = function(value)
{
	midi.sendShortMsg(0x90, 0x2a, value > 0 ? 0x7f : 0);
}

DJTechCDJ101.play_feedback_deck2 = function(value)
{
	midi.sendShortMsg(0x91, 0x2a, value > 0 ? 0x7f : 0);
}

DJTechCDJ101.cue_feedback_deck1 = function(value)
{
	midi.sendShortMsg(0x90, 0x2b, value > 0 ? 0x7f : 0);
}

DJTechCDJ101.cue_feedback_deck2 = function(value)
{
	midi.sendShortMsg(0x91, 0x2b, value > 0 ? 0x7f : 0);
}

//
// translate 14bit pitchbend message into pitch control
//

DJTechCDJ101.pitch = function(channel, lsb, msb, status, group) 
{
	engine.setValue(group, "rate", script.pitch(lsb, msb, status)); // not working for some reason
	//engine.setValue(group, "rate", (8192 - (msb << 7 | lsb)) / 8192);
}

DJTechCDJ101.beatjump = function(group, jump)
{
	jump = jump * 120 / engine.getValue(group, "bpm") / engine.getValue(group, "track_samples") * engine.getValue(group, "track_samplerate");
	engine.setValue(group, "playposition", engine.getValue(group, "playposition") + jump);
}

//
// hold down encoder and turn to change playlists
// turn while not pushed in to scroll through tracks
// click to load track
//

DJTechCDJ101.browse = function(channel, control, value, status, group) 
{
	if (DJTechCDJ101.pushed) {
		engine.setValue("[Playlist]", value == 0x41 ? "SelectNextPlaylist" : "SelectPrevPlaylist", 1);
	}
	else {
		engine.setValue("[Playlist]", value == 0x41 ? "SelectNextTrack" : "SelectPrevTrack", 1);
	}
}

DJTechCDJ101.push = function(channel, control, value, status, group) 
{
	if (value > 0) {
		DJTechCDJ101.pushed = true;
		DJTechCDJ101.pushedon = DJTechCDJ101.mtime();
	}
	else {

		//
		// load selected track if released quickly enough
		//

		if (DJTechCDJ101.mtime() - DJTechCDJ101.pushedon < DJTechCDJ101.pushmaxtime) {
			engine.setValue(group, "LoadSelectedTrack", 1);
		}

		DJTechCDJ101.pushed = false;
		DJTechCDJ101.pushedon = 0;
	}
}

//
// when deck is not playing and the push button is held, pressing cue will move the beatgrid
//

DJTechCDJ101.cue = function(channel, control, value, status, group)
{
	if (DJTechCDJ101.pushed && !engine.getValue(group, "play")) {
		engine.setValue(group, "beats_translate_curpos", value > 0 ? 1 : 0);
	}
	else {
		engine.setValue(group, "cue_default", value > 0 ? 1 : 0);
	}
}

//
// when the push button is held, pressing play will sync 
//

DJTechCDJ101.play = function(channel, control, value, status, group)
{
	if (DJTechCDJ101.pushed) {
		engine.setValue(group, "beatsync", value > 0 ? 1 : 0);
	}
	else if (value > 0) {
		//
		// we want play to toggle
		//

		engine.setValue(group, "play", !engine.getValue(group, "play"));
	}
}

//
// when deck is playing either enable or disable scratch mode, no action otherwise
//

DJTechCDJ101.jogtouch = function(channel, control, value, status, group) 
{
	var deck = parseInt(group.substring(8,9));

	if (value > 0) {
		engine.scratchEnable(deck, 143, 45, 0.125, 0.125/32);
	}
	else {
		//
		// don't disable right away in case of spin backs if playing and also morph
		// outer jog movements into top jog movements during this time 
		//

		if (!engine.getValue(group, "play")) {
			DJTechCDJ101.finishScratch(deck, false);
		}
		else {
			DJTechCDJ101.outer2inner = true;
			DJTechCDJ101.scratch_timer = engine.beginTimer(DJTechCDJ101.scratch_timeout, 'DJTechCDJ101.finishScratch(' + deck + ', true)');
		}
	}
}

DJTechCDJ101.finishScratch = function(deck, stop_timer)
{
	if (stop_timer) {
		engine.stopTimer(DJTechCDJ101.scratch_timer);
	}
	engine.scratchDisable(deck);
	DJTechCDJ101.outer2inner = false;
}

//
// use outer part of jog to pitchbend when in play mode and fine track seek when not
//

DJTechCDJ101.jogouter = function(channel, control, value, status, group) 
{
	//
	// when touch surface has been release for a short time convert
	// outer jog movements into surface jog ones for spinbacks etc
	//

	if (DJTechCDJ101.outer2inner) {
		return DJTechCDJ101.jogtop(channel, control, value, status, group);
	}

	//
	// scale down based on whether we are playing and the shift is held down
	//

	value = (value - 0x40) / 2;
	if (DJTechCDJ101.pushed) value /= 2.5;
	if (!engine.getValue(group, "play")) value /= 2.5;
	engine.setValue(group, "jog", value);
}

//
// track playing: top of jog scratches
// track not playing: seek thru track (using same scratch ticks and gives more postive response) or seek faster through track when push button held down
//

DJTechCDJ101.jogtop = function(channel, control, value, status, group) 
{
	value -= 0x40;
	if (!engine.getValue(group, "play") && DJTechCDJ101.pushed) {
		DJTechCDJ101.beatjump(group, value);
	}
	else {
		engine.scratchTick(parseInt(group.substring(8,9)), value);
	}
}

