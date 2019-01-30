function HerculesMk4 () {}

//Hercules Mk4 scripts from Dj Kork.... enjoy them ;)



// Number of the standard RPM value. Lower values increase de sensitivity as the really records.
HerculesMk4.standardRpm = 33.33;

// The HerculesMk4.alpha value for the filter (start with 1/8 (0.125) and tune from there)
HerculesMk4.alpha = 1/8;

// The HerculesMk4.beta value for the filter (start with HerculesMk4.alpha/32 and tune from there)
HerculesMk4.beta = HerculesMk4.alpha/20;

HerculesMk4.scratchButton = 0;
HerculesMk4.scratchMode =[0,0];
HerculesMk4.antiguoMixCue=1;
HerculesMk4.sensitivityPitch=[5,5];
HerculesMk4.jogFastPosition=[0,0];




HerculesMk4.init = function (id) {
    // Switch off all LEDs
    for (var i=1; i<95; i++) midi.sendShortMsg(0x90, i, 0x00);
    // Switch-on the LEDs for improve the usability
    midi.sendShortMsg(0x90, 45, 0x7F);    // Scratch LED
    midi.sendShortMsg(0x90, 14, 0x7F);    // Cue deck A LED
    midi.sendShortMsg(0x90, 34, 0x7F);    // Cue deck B LED
};

HerculesMk4.shutdown = function (id) {
    // Switch off all LEDs
    for (var i=1; i<95; i++) midi.sendShortMsg(0x90, i, 0x00);
};



HerculesMk4.loadTrack = function (midino, control, value, status, group) {
    // Load the selected track in the corresponding deck only if the track is
    // paused

    if(value && engine.getValue(group, "play") != 1) {
        engine.setValue(group, "LoadSelectedTrack", 1);
        engine.setValue(group, "rate", 0);
        }
    else engine.setValue(group, "LoadSelectedTrack", 0);
};


HerculesMk4.scroll = function (midino, control, value, status, group) {
    // Button "Files": up 10 tracks
    // Button "Folder": down 10 tracks
    // This function scroll up or down 10 tracks on the playlist, like the mouse
    // scroll.

    if(control == 0x2C && value == 0x7F) {
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "1");
        engine.setValue("[Playlist]", "SelectPrevTrack", "0");
        }
    if (control == 0x2B && value == 0x7F) {
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "1");
        engine.setValue("[Playlist]", "SelectNextTrack", "0");
        }
};



HerculesMk4.keyButton = function (midino, control, value, status, group) {
    // Loop command for the first 4 Key, Hotcues command for the latest 4

    switch (control) {

    // Loop buttons
        case 0x01: case 0x15:      // K1, Loop in
            engine.setValue(group, "loop_in", value ? 1 : 0);
            break;
        case 0x02: case 0x16:    // K2, Loop out
            engine.setValue(group, "loop_out", value ? 1 : 0);
            break;
        case 0x03: case 0x17:    // K3, Reloop/Exit
            engine.setValue(group, "reloop_exit", value ? 1 : 0); break;
            break;
        case 0x04: case 0x18:    // K4, Reloop/Exit
            engine.setValue(group, "reloop_exit", value ? 1 : 0);
            break;

    // Hotcue buttons:
    // Simple press: go to the hotcue position
    // Shift (hold down "scratch"): clear the hotcue

        case 0x05: case 0x19 :    // K5
            if (HerculesMk4.scratchButton == 1) {
                //HerculesMk4.holdButton(group, value, "hotcue_1_set", "hotcue_1_clear");
                engine.setValue(group, "hotcue_1_clear", value ? 1 : 0);
            }
            else engine.setValue(group, "hotcue_1_activate", value ? 1 : 0);
            break;

        case 0x06: case 0x1A:    // K6
            if (HerculesMk4.scratchButton == 1) {
                //HerculesMk4.holdButton(group, value, "hotcue_2_set", "hotcue_2_clear");
                engine.setValue(group, "hotcue_2_clear", value ? 1 : 0);
            }
            else engine.setValue(group, "hotcue_2_activate", value ? 1 : 0);
            break;

        case 0x07: case 0x1B:    // K7
            if (HerculesMk4.scratchButton == 1) {
                //HerculesMk4.holdButton(group, value, "hotcue_3_set", "hotcue_3_clear");
                engine.setValue(group, "hotcue_3_clear", value ? 1 : 0);
            }
            else engine.setValue(group, "hotcue_3_activate", value ? 1 : 0);
            break;

        case 0x08: case 0x1C:    // K8
            if (HerculesMk4.scratchButton == 1) {
                //HerculesMk4.holdButton(group, value, "hotcue_4_set", "hotcue_4_clear");
                engine.setValue(group, "hotcue_4_clear", value ? 1 : 0);
            }
            else engine.setValue(group, "hotcue_4_activate", value ? 1 : 0);
            break;
        }
};

HerculesMk4.knobIncrement = function (group, action, minValue, maxValue, centralValue, step, sign) {
    // This function allows you to increment a non-linear value like the volume's knob
    // sign must be 1 for positive increment, -1 for negative increment
    var semiStep = step/2;
    var rangeWidthLeft = centralValue-minValue;
    var rangeWidthRight = maxValue-centralValue;
    var actual = engine.getValue(group, action);
    var increment;
    if (actual < 1) increment = ((rangeWidthLeft)/semiStep)*sign;
    if (actual > 1) increment = ((rangeWidthRight)/semiStep)*sign;
    if (actual == 1) {
        increment = (sign == 1) ? rangeWidthRight/semiStep : (rangeWidthLeft/semiStep)*sign;
    }

    var newValue;

    if (sign == 1 && actual < maxValue)     newValue = actual + increment;
    if (sign == -1 && actual > minValue)    newValue = actual + increment;
    return newValue;
};


HerculesMk4.deck=function(group){
 //channel 1 -->deck 0
 //channel 2 -->deck 1
 return (group=="[Channel1]") ? 0 : 1;
}

HerculesMk4.selectLed=function(group,led){
 //channel 1 -->led 0
 //channel 2 -->led + 20
 return (group=="[Channel1]") ? led : led+20;
}



HerculesMk4.pitch = function (midino, control, value, status, group) {
    // Simple: pitch slider


    if (HerculesMk4.scratchButton == 1) {
        var sign = (value == 0x01) ? 1 : -1;
        var newValue = HerculesMk4.knobIncrement(group, "pregain", 0, 4, 1, 60, sign);
        engine.setValue(group, "pregain", newValue);
    }
    else {

        var increment = 0.005 * HerculesMk4.sensitivityPitch[HerculesMk4.deck(group)];
        increment = (value == 0x01) ? increment : increment * -1;
        engine.setValue(group, "rate", engine.getValue(group, "rate") + increment);
    }
};


HerculesMk4.pfl = function (midino, control, value, status, group) {

    if(value){

        engine.setValue(group,"pfl",(engine.getValue(group,"pfl")) ? 0 :1 );

        var pfl1=engine.getValue("[Channel1]","pfl");
        var pfl2=engine.getValue("[Channel2]","pfl");


        var actualMixCue=engine.getValue("[Master]","headMix");

        if(pfl1==0 && pfl2==0){
            HerculesMk4.antiguoMixCue=actualMixCue;
            engine.setValue("[Master]","headMix",1);
        }else{
            if(actualMixCue==1){
                engine.setValue("[Master]","headMix",HerculesMk4.antiguoMixCue);
            }
        };
    };
};




HerculesMk4.pitchbend = function (midino, control, value, status, group) {
<<<<<<< HEAD
	// Pitch - : set pitch sensitivity
	// Pitch +:  set jog fast position
=======
    // Pitch - : set pitch sensivity
    // Pitch +:  set jog fast position
>>>>>>> upstream/2.2

    //ignore when releasing the button
    if(value==0x00) return;

    if (control == 0x0B || control == 0x1F) { // Pitchbend +
        var newValue = (HerculesMk4.jogFastPosition[HerculesMk4.deck(group)]==1)? 0 : 1;

        HerculesMk4.jogFastPosition[HerculesMk4.deck(group)]=newValue;

<<<<<<< HEAD
		if(newValue==1){
			midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,11), 0x7f);
		}else{
			midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,11), 0x00);
		}
	}
	else { // Pitchbend -
		HerculesMk4.sensitivityPitch[HerculesMk4.deck(group)]=HerculesMk4.toglePitchSensitivity(group,HerculesMk4.sensitivityPitch[HerculesMk4.deck(group)]);
=======
        if(newValue==1){
            midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,11), 0x7f);
        }else{
            midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,11), 0x00);
        }
    }
    else { // Pitchbend -
        HerculesMk4.sensivityPitch[HerculesMk4.deck(group)]=HerculesMk4.toglePitchSensivity(group,HerculesMk4.sensivityPitch[HerculesMk4.deck(group)]);
>>>>>>> upstream/2.2

    }

};


HerculesMk4.toglePitchSensitivity=function (group,sensitivity) {

<<<<<<< HEAD
	sensitivity=sensitivity+2;

	if(sensitivity>5){
		sensitivity=1;
	}



	if(sensitivity==1){
		//pitch very fine
		midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,10), 0x00);	// minus led off
		midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,58), 0x7F);	// Blink minus led

	} else if (sensitivity==3){
		//pitch fine
		midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,58), 0x00);	// Blink minus led off
		midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,10), 0x7F);	// minus led
=======
    sensivity=sensivity+2;

    if(sensivity>5){
        sensivity=1;
    }



    if(sensivity==1){
        //pitch very fine
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,10), 0x00);    // minus led off
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,58), 0x7F);    // Blink minus led

    } else if (sensivity==3){
        //pitch fine
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,58), 0x00);    // Blink minus led off
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,10), 0x7F);    // minus led
>>>>>>> upstream/2.2

    } else {
        //pitch coarse
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,58), 0x00);    // Blink minus led off
        midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,10), 0x00);    // minus led off
    }

<<<<<<< HEAD
	return sensitivity;
=======
    return sensivity;
>>>>>>> upstream/2.2

}

HerculesMk4.cue = function (midino, control, value, status, group) {
    // Don't set Cue accidentally at the end of the song

    if(engine.getValue(group, "playposition") <= 0.97) {
        engine.setValue(group, "cue_default", value ? 1 : 0);
    }
    else     engine.setValue(group, "cue_preview", value ? 1 : 0);
};

HerculesMk4.scratch = function (midino, control, value, status, group) {
    // The "scratch" button is used like a shift button only for enable the scratch mode on the
    // deck A and/or B with the "sync" buttons
    if (value) {
        HerculesMk4.scratchButton = 1;
        //Switch-on the LEDs of Sync buttons while the "scratch" button is hold down
        midi.sendShortMsg(0x90, 18, 0x7F);
        midi.sendShortMsg(0x90, 38, 0x7F);
        }
    else {
        HerculesMk4.scratchButton = 0;
        midi.sendShortMsg(0x90, 18, 0x00);
        midi.sendShortMsg(0x90, 38, 0x00);
        }
};

HerculesMk4.sync = function (midino, control, value, status, group) {

    if (HerculesMk4.scratchButton && value) { // If the "scratch" button is hold down

        if(HerculesMk4.scratchMode[HerculesMk4.deck(group)]==0) {
            engine.scratchEnable(HerculesMk4.deck(group)+1, 128, HerculesMk4.standardRpm, HerculesMk4.alpha, HerculesMk4.beta); // Enable the scratch mode on Deck A
            HerculesMk4.scratchMode[HerculesMk4.deck(group)] = 1;
            midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,66), 0x7F); // Blinks the sync led
        }
        else {
            engine.scratchDisable(HerculesMk4.deck(group)+1);
            HerculesMk4.scratchMode[HerculesMk4.deck(group)] = 0;
            midi.sendShortMsg(0x90, HerculesMk4.selectLed(group,66), 0x00); // Switch-off the sync led
        }

    }
    else engine.setValue(group, "beatsync", value ? 1 : 0);
};

HerculesMk4.jogWheel = function (midino, control, value, status, group) {
    // This function is called everytime the jog is moved

    var direction=(value == 0x01)? 1: -1;

    engine.scratchTick(HerculesMk4.deck(group)+1, direction);

    var turboSeek=engine.getValue(group,"play")==0 && HerculesMk4.jogFastPosition[HerculesMk4.deck(group)]==1 && HerculesMk4.scratchMode[HerculesMk4.deck(group)]==0;

    if(turboSeek) {
        var new_position = engine.getValue(group,"playposition")+ 0.008*direction;
        if(new_position<0) new_position=0;
        if(new_position>1) new_position=1;

        engine.setValue(group,"playposition",new_position);
    }


    if( !turboSeek){
        engine.setValue(group, "jog", direction);
    }
};
