function VinylModeSwitch () {}

VinylModeSwitch.cur_mode_ch1 = 0;
VinylModeSwitch.cur_mode_ch2 = 0;
VinylModeSwitch.inited_ch1 = false;
VinylModeSwitch.inited_ch2 = false;

VinylModeSwitch.init = function (id) {    // called when the MIDI device is opened & set up
    print ("VinylModeSwitch id: \""+id+"\" initialized.");
}

VinylModeSwitch.shutdown = function(id) {
}

VinylModeSwitch.VinylModeInc1 = function (group, control, value, status) { 
	if (value == 0)
		return;
		
	VinylModeSwitch.Check_Init_ch1();

	if (VinylModeSwitch.cur_mode_ch1 >= 2)
		VinylModeSwitch.cur_mode_ch1 = 0;
	else
		VinylModeSwitch.cur_mode_ch1++;
		
	engine.setValue("[Channel1]", "VinylMode", VinylModeSwitch.cur_mode_ch1);
}

VinylModeSwitch.Check_Init_ch1 = function () {
	if (!VinylModeSwitch.inited_ch1)
	{
		VinylModeSwitch.cur_mode_ch1 = engine.getValue("[Channel1]","VinylMode")
		VinylModeSwitch.inited_ch1 = true;
	}
}

VinylModeSwitch.VinylModeInc2 = function (group, control, value, status) { 
	if (value == 0)
		return;
		
	VinylModeSwitch.Check_Init_ch2()

	if (VinylModeSwitch.cur_mode_ch2 >= 2)
		VinylModeSwitch.cur_mode_ch2 = 0;
	else
		VinylModeSwitch.cur_mode_ch2++;
		
	engine.setValue("[Channel2]", "VinylMode", VinylModeSwitch.cur_mode_ch2);
}

VinylModeSwitch.Check_Init_ch2 = function () {
	if (!VinylModeSwitch.inited_ch2)
	{
		VinylModeSwitch.cur_mode_ch2 = engine.getValue("[Channel2]","VinylMode")
		VinylModeSwitch.inited_ch2 = true;
	}
}
