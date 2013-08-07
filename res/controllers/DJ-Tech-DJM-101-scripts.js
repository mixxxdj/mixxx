/****************************************************************/
/*      DJ-Tech DJM101 controller script                        */
/*      For Mixxx version 1.11                                  */
/*      Author: zestoi                                          */
/****************************************************************/

DJTechDJM101 = {};
DJTechDJM101.vumeter = {};
DJTechDJM101.vumeter[1] = 0;
DJTechDJM101.vumeter[2] = 0;

DJTechDJM101.init = function(id) {
	DJTechDJM101.vumeter_select(true);
	engine.connectControl("[Master]", "VuMeterL", "DJTechDJM101.VuMeterMasterL");
	engine.connectControl("[Master]", "VuMeterR", "DJTechDJM101.VuMeterMasterR");
	engine.connectControl("[Channel1]", "VuMeter", "DJTechDJM101.VuMeterDeck1");
	engine.connectControl("[Channel2]", "VuMeter", "DJTechDJM101.VuMeterDeck2");
	engine.connectControl("[Channel1]", "pfl", "DJTechDJM101.pfl");
	engine.connectControl("[Channel2]", "pfl", "DJTechDJM101.pfl");
}

DJTechDJM101.shutdown = function() {}

//
// change output to vumeter and reset
//

DJTechDJM101.vumeter_select = function(master)
{
	DJTechDJM101.vumeter_master_mode = master;
	DJTechDJM101.update_vumeter(1, 0);
	DJTechDJM101.update_vumeter(2, 0);
}

//
// select what data is sent to the vumeter
//

DJTechDJM101.vumeter_select_master = function(channel, control, value, status, group)
{
	if (value > 0) {
		DJTechDJM101.vumeter_select(true);
	}
}

DJTechDJM101.vumeter_select_pfl = function(channel, control, value, status, group)
{
	if (value > 0) {
		DJTechDJM101.vumeter_select(false);
	}
}

//
// update a vumeter channel
//

DJTechDJM101.update_vumeter = function(channel, value)
{
	var newval = parseInt(value * 0xf7);
	if (DJTechDJM101.vumeter[channel] != newval) {
		DJTechDJM101.vumeter[channel] = newval;

		//
		// a bit nasty - four different cc's depending on master/pfl mode and which channel
		//

		midi.sendShortMsg(0xb0, DJTechDJM101.vumeter_master_mode ? 0x4f + channel : 0x51 + channel, newval);
	}
}

//
// only feed the correct levels to each channel of the vumeter
//

DJTechDJM101.VuMeterMasterL = function(value)
{
	if (DJTechDJM101.vumeter_master_mode == false) return;
	DJTechDJM101.update_vumeter(1, value);
}

DJTechDJM101.VuMeterMasterR = function(value)
{
	if (DJTechDJM101.vumeter_master_mode == false) return;
	DJTechDJM101.update_vumeter(2, value);
}

DJTechDJM101.VuMeterDeck1 = function(value)
{
	if (DJTechDJM101.vumeter_master_mode == true) return;
	DJTechDJM101.update_vumeter(1, value);
}

DJTechDJM101.VuMeterDeck2 = function(value)
{
	if (DJTechDJM101.vumeter_master_mode == true) return;
	DJTechDJM101.update_vumeter(2, value);
}

//
// led feedback for headphone cue buttons
//

DJTechDJM101.pfl = function(value, group)
{
	// has to be a noteon msg in both case and seems more reliable issuing 0x7f for 'on' amd 0x0 for 'off'
	midi.sendShortMsg(0x90, group == "[Channel1]" ? 0x21 : 0x22, value > 0 ? 0x7f : 0);
}



