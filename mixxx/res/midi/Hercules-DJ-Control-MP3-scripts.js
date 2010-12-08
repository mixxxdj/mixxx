function HerculesMp3 () {}

//wheel sensitivity is changing between different versions of Mixxx.
//Change this accordingly.

jog_sens = 1;

//Warning! setting enable_beats to true, will enable Autobeat lights to flash near a beat. 
//Unfortunately I had to use a buffer to avoid sending signal to the controller too close each other.
//If you enable this feature, leds could not properly work.
//In the future, a function midi.sendShortMsgBuffered() will be implemented in Mixxx code and I will try to fix this.
//Until there, I recommend to not change this setting.







//If you do not know what you are doing, do not change any variable under this comment.

HerculesMp3.buttons123Modes = ["kill", "fx", "cue", "loop"];


HerculesMp3.controls = {
"inputs": {
    0x09: { "channel": 1, "name": "cue", "type": "button" },
    0x03: { "channel": 2, "name": "cue", "type": "button" },
    0x08: { "channel": 1, "name": "play", "type": "button" },
    0x02: { "channel": 2, "name": "play", "type": "button" },
    0x07: { "channel": 1, "name": "fx select", "type": "button", "mode": 0 },
    0x01: { "channel": 2, "name": "fx select", "type": "button", "mode": 0 },
    0x0F: { "channel": 1, "name": "fx 1", "type": "button", "used": false },
    0x10: { "channel": 2, "name": "fx 1", "type": "button", "used": false },
    0x0E: { "channel": 1, "name": "fx 2", "type": "button", "used": false },
    0x11: { "channel": 2, "name": "fx 2", "type": "button", "used": false },
    0x0D: { "channel": 1, "name": "fx 3", "type": "button", "used": false },
    0x12: { "channel": 2, "name": "fx 3", "type": "button", "used": false },
    0x1B: { "channel": 1, "name": "mouse", "type": "button" },
    0x1C: { "channel": 2, "name": "mouse", "type": "button" },
    0x16: { "channel": 1, "name": "master tempo", "type": "button" },
    0x1A: { "channel": 2, "name": "master tempo", "type": "button" },
    0x34: { "channel": 1, "name": "pitch", "type": "pot", "mode": 0 },
    0x35: { "channel": 2, "name": "pitch", "type": "pot", "mode": 0 },
    0x36: { "channel": 1, "name": "wheel", "type": "pot" },
    0x37: { "channel": 2, "name": "wheel", "type": "pot" },
    0x0B: { "channel": 1, "name": "prevtrack", "type": "button" },
    0x0C: { "channel": 1, "name": "nexttrack", "type": "button" },
    0x05: { "channel": 2, "name": "prevtrack", "type": "button" },
    0x06: { "channel": 2, "name": "nexttrack", "type": "button" }
},
"outputs": {
    0x0F: { "channel": 1, "name": "fx mode", "type": "led" , "status": 0x00 , "isblinking": false },
    0x10: { "channel": 2, "name": "fx mode", "type": "led" , "status": 0x00 , "isblinking": false },
    0x0E: { "channel": 1, "name": "cue mode", "type": "led" , "status": 0x00 },
    0x11: { "channel": 2, "name": "cue mode", "type": "led" , "status": 0x00 },
    0x0D: { "channel": 1, "name": "loop mode", "type": "led" , "status": 0x00 },
    0x12: { "channel": 2, "name": "loop mode", "type": "led" , "status": 0x00 },
    0x16: { "channel": 1, "name": "master tempo", "type": "led" , "status": 0x00 },
    0x1A: { "channel": 2, "name": "master tempo", "type": "led" , "status": 0x00 },
    0x0A: { "channel": 1, "name": "auto beat", "type": "led" , "status": 0x00 , "beatlen": 0 },
    0x04: { "channel": 2, "name": "auto beat", "type": "led" , "status": 0x00 , "beatlen": 0 },
    0x09: { "channel": 1, "name": "cue_set", "type": "led" , "status": 0x00 },
    0x03: { "channel": 2, "name": "cue_set", "type": "led" , "status": 0x00 },
    0x00: { "channel": 1, "name": "play blink", "type": "led" , "status": 0x00 },
    0x05: { "channel": 2, "name": "play blink", "type": "led" , "status": 0x00 },
    0x08: { "channel": 1, "name": "play", "type": "led" , "status": 0x00 },
    0x02: { "channel": 2, "name": "play", "type": "led" , "status": 0x00 },
    0x7E: { "channel": 1, "name": "pfl", "type": "led" , "status": 0x00 },
    0x7D: { "channel": 2, "name": "pfl", "type": "led" , "status": 0x00 }
}
};



//Init controller

HerculesMp3.init = function (id) { // called when the device is opened & set up

//  variables and arrays
    led_lock = false;
    enable_beats = false;
    ledmsg = 0x00;

    loopc = [];
    timerloopID = [];
    is_hold_loop=[];
    reloopon = [];
    is_hold_hotcue = [];
    timerhotcueID = [];
    loop_control=[];
    Deck = [];
    scratch = [];
    led_array = [];
    scratch_timer_on = [];
    scratch_timer = [];
    returnblink = [];
    blinktimer = [];

    bshot = [];

    VLset = [];
    
    for(decks=1;  decks<=2; decks++){
        Deck[decks] = "[Channel"+decks+"]";
        loopc[decks] = 0;
        VLset[decks] = false;
        timerloopID[decks] = 0;
        is_hold_loop[decks] = true;
        reloopon[decks] = false;
        scratch[decks] = false;
        VLset[decks] = false;
        scratch_timer_on[decks]=false;
    }





//  Messages to the console cannot be sent too close ( < 10ms)
//  So that in other scripts a pause function is used
//  afaik it is not allowed.
//  a timer and a buffer will rectify this.
//  Setting it to 20ms instead of 11ms as this first is the lowest value accepted for a timer.

    engine.beginTimer(20, "HerculesMp3.ledhelper()");


//  Be the leds!

    HerculesMp3.leds(true);
    engine.beginTimer(500, "HerculesMp3.leds(false)",true);

//  Connect leds here and not in xml. This is to avoid an annoying bug (maybe) in controller drivers.

    engine.connectControl("[Channel1]", "pfl", "HerculesMp3.pfl_left");
    engine.connectControl("[Channel2]", "pfl", "HerculesMp3.pfl_right");
    engine.connectControl("[Channel1]", "play", "HerculesMp3.play_left");
    engine.connectControl("[Channel2]", "play", "HerculesMp3.play_right");
    engine.connectControl("[Channel1]", "cue_default", "HerculesMp3.cue_left");
    engine.connectControl("[Channel2]", "cue_default", "HerculesMp3.cue_right");
    //engine.connectControl("[Channel1]", "keylock", "HerculesMp3.master_tempoA");
    //engine.connectControl("[Channel2]", "keylock", "HerculesMp3.master_tempoB");

    print ("HerculesMp3 id: \""+id+"\" initialized.");
};

//Shutdown the controller.

HerculesMp3.shutdown = function (id) {
    HerculesMp3.leds(false);
    print ("HerculesMp3 id: \""+id+"\" shut down.");
};


//Messages to the console cannot be sent too close ( < 10ms)

HerculesMp3.ledhelper = function () {

//  What does this function do? It is just an apostrophe between the words "I'll-do".

    if(typeof led_array == "undefined"){
        led_array = [];
    }
    if (led_array.length > 0){
        var nowled = led_array[0];
        var nowstatus = HerculesMp3.controls.outputs[nowled].status;
        led_array.shift();
        midi.sendShortMsg(0xB0,nowled,nowstatus);
        //clear buffer if there are too messages
    }
//  beat leds

    if(enable_beats){
        var beatA = HerculesMp3.controls.outputs[0x0A].beatlen;
        var beatB = HerculesMp3.controls.outputs[0x04].beatlen;
        if(beatA) HerculesMp3.bblinkA(beatA);
        if(beatB) HerculesMp3.bblinkB(beatB);
    }


};

HerculesMp3.leds = function (onoff) {
    for (control in HerculesMp3.controls.outputs) {
        if (HerculesMp3.controls.outputs[control].type == 'led') {
            HerculesMp3.led(control,onoff);
        }
    }

};

HerculesMp3.ledblink = function (control,onoff){
    if(onoff) {
        actual_status = HerculesMp3.controls.outputs[control].status;
        blinktimer[control] = engine.beginTimer(300,"HerculesMp3.ledblinkhelper("+control+")");
        HerculesMp3.controls.outputs[control].isblinking = true;
    }
    else
    {
        if (HerculesMp3.controls.outputs[control].isblinking) {
            HerculesMp3.controls.outputs[control].isblinking = false;
            engine.stopTimer(blinktimer[control]);
            HerculesMp3.led(control,actual_status);
        }
    }
};

HerculesMp3.ledblinkhelper = function (control){
    var ledval = true;
    if (HerculesMp3.controls.outputs[control].status)
        {
        ledval = false;
        }
    HerculesMp3.led(control,ledval);
};

HerculesMp3.pfl_left = function(value) {
    HerculesMp3.led(0x7E, value);
};
HerculesMp3.pfl_right = function(value) {
    HerculesMp3.led(0x7D, value);
};
HerculesMp3.play_left = function(value) {
    HerculesMp3.led(0x08, value);
};
HerculesMp3.play_right = function(value) {
    HerculesMp3.led(0x02, value);
};
HerculesMp3.cue_left = function(value) {
    HerculesMp3.led(0x09, value);
};
HerculesMp3.cue_right = function (value) {
    HerculesMp3.led(0x03, value);

};
HerculesMp3.master_tempoA = function (value){
    HerculesMp3.led(0x16, value);
};

HerculesMp3.master_tempoB = function (value){
    HerculesMp3.led(0x1A, value);
};

HerculesMp3.led = function (control, isOn) {
    ledmsg = 0x00;
    if (isOn){
        ledmsg = 0x7F;
      //Clear buffer when there are too messages
        if(led_array.length > 10) {
            led_array = [];
        }
    }
    if(HerculesMp3.controls.outputs[control].status != ledmsg) {
        led_array.push(control);
        HerculesMp3.controls.outputs[control].status = ledmsg;
    }
};



//loop control


HerculesMp3.loophold = function (deck) {

    is_hold_loop[deck] = true;
    if(engine.getValue("[Channel"+deck+"]","loop_enabled"))
    {
        //problem with skins
        engine.setValue("[Channel"+deck+"]","reloop_exit",1);
        engine.setValue("[Channel"+deck+"]","reloop_exit",0);
    }
    else {
        engine.setValue("[Channel"+deck+"]","reloop_exit",0);
    }
    engine.setValue("[Channel"+deck+"]", "loop_in", 0);
    engine.setValue("[Channel"+deck+"]", "loop_start_position",-1);
    engine.setValue("[Channel"+deck+"]", "loop_out", 0);
    engine.setValue("[Channel"+deck+"]", "loop_end_position",-1);
    loopc[deck] = 0;
};




HerculesMp3.loop = function (deck, control, value) {
    if(value) {
        is_hold_loop[deck] = false;
        timerloopID[deck] = engine.beginTimer(700,"HerculesMp3.loophold("+deck+")",true);
    } 
    else {
        engine.stopTimer(timerloopID[deck]);
        if(!is_hold_loop[deck]) {
            loopc[deck]++;
            if(loopc[deck]==1)engine.setValue("[Channel"+deck+"]", "loop_in", 1);
            if(loopc[deck]==2)engine.setValue("[Channel"+deck+"]", "loop_out", 1);
            if(loopc[deck]>2) {
                engine.setValue("[Channel"+deck+"]", "reloop_exit", 1);
                loopc[deck]=2;
            }
        }
    }
};



//hotcue control

HerculesMp3.fxbutton = function (fxnumber) {
    switch(fxnumber)
    {
        case "fx 1":
            return "hotcue_1_";
            break;

        case "fx 2" :
            return "hotcue_2_";
            break;

        case "fx 3" :
            return "hotcue_3_";
            break;	
    }

};

HerculesMp3.hotcue = function (deck, control, value) {
    hotcue_string = HerculesMp3.fxbutton(HerculesMp3.controls.inputs[control].name);
    if(value){
        is_hold_hotcue[control] = false;
        timerhotcueID[control] = engine.beginTimer(500,"HerculesMp3.hchold("+deck+","+control+","+"\""+hotcue_string+"\")",true);
    }
    else {
        engine.stopTimer(timerhotcueID[control]);
        if(!is_hold_hotcue[control]){
            if(!engine.getValue("[Channel"+deck+"]",hotcue_string+"enabled")) {engine.setValue("[Channel"+deck+"]",hotcue_string+"activate",1);}
            else{engine.setValue("[Channel"+deck+"]",hotcue_string+"goto",1);}
        }
    }
};


HerculesMp3.hchold = function (deck,control,hotcuestr){
    is_hold_hotcue[control]=true;
    engine.setValue("[Channel"+deck+"]",hotcuestr+"clear",1);
};


//Cycle between kill,cue,loop,fx

HerculesMp3.mode = function (group, control, value, status) {


    var n = HerculesMp3.controls.inputs[control].channel;

    var group = Deck[n];

//  took from other Hercules scripts

    if (value) { // Only do stuff when pushed, not released.
        currentMode = HerculesMp3.controls.inputs[control].mode;
        nextMode = currentMode < HerculesMp3.buttons123Modes.length-1 ? currentMode+1 : 0;
        HerculesMp3.controls.inputs[control].mode = nextMode;
        sNextMode = HerculesMp3.buttons123Modes[nextMode];
        sCurrentMode = HerculesMp3.buttons123Modes[currentMode];
        for ( fxcontrol in HerculesMp3.controls.outputs){
            if(HerculesMp3.controls.outputs[fxcontrol].channel == n && 
                    HerculesMp3.controls.outputs[fxcontrol].name == sCurrentMode+" mode")
            {
                currentLed = fxcontrol;
            }
            if(HerculesMp3.controls.outputs[fxcontrol].channel == n && 
                    HerculesMp3.controls.outputs[fxcontrol].name == sNextMode+" mode")
            {
                nextLed = fxcontrol;
            }
        }
        switch (sNextMode) {
            case "kill":
                HerculesMp3.led(currentLed, false);
                break;
            case "fx":
                returnblink[n] = false;
                if(HerculesMp3.controls.outputs[nextLed].isblinking){
                    HerculesMp3.ledblink(nextLed, false);
                    returnblink[n] = true;
                }
                HerculesMp3.led(nextLed, true); 
                //print("HerculesMp3.buttons123mode: Switching to " + sNextMode + " mode");
                break;
            case "cue":
                if(returnblink[n]){
                    HerculesMp3.ledblink(currentLed, true);
                }
                HerculesMp3.led(currentLed, false); 
                HerculesMp3.led(nextLed, true); 
                //print("HerculesMp3.buttons123mode: Switching to " + sNextMode + " mode");
                break; 
            case "loop":
                HerculesMp3.led(currentLed, false); 
                HerculesMp3.led(nextLed, true); 
                //print("HerculesMp3.buttons123mode: " + sNextMode + " mode");
                break;
        }


    }
};

//fx buttons


HerculesMp3.fx = function (group, control, value, status) {

    var n = HerculesMp3.controls.inputs[control].channel;

    var fxdeck = n==1 ? 0x07 : 0x01;

    var currentMode = HerculesMp3.controls.inputs[fxdeck].mode;

    switch (HerculesMp3.buttons123Modes[currentMode]){
        case "kill" :
            if (value){
                HerculesMp3.controls.inputs[control].used = true;
            }
            else { HerculesMp3.controls.inputs[control].used = false; 
            }
            break;

        case "loop" :

            HerculesMp3.loop(n, control, value);

            break;

        case "cue" :

            HerculesMp3.hotcue(n, control, value);

            break;

        case "fx" :

            if (HerculesMp3.controls.inputs[control].name == "fx 1"){
                if(value)
                    {
                    scratch[n]=!scratch[n];
                    HerculesMp3.controls.outputs[nextLed].isblinking = scratch[n];
                    returnblink[n] = scratch[n];
                    }
                
            }
            if (HerculesMp3.controls.inputs[control].name == "fx 2"){
                if(value){bpm.tapButton(n);}
            }
            if (HerculesMp3.controls.inputs[control].name == "fx 3"){
                if(value){
                enable_beats = !enable_beats;
                if(enable_beats){
                    engine.connectControl("[Channel1]", "playposition", "HerculesMp3.playpositionA");
                    engine.connectControl("[Channel1]","file_bpm","HerculesMp3.bpconnectA");
                    engine.connectControl("[Channel2]", "playposition", "HerculesMp3.playpositionB");
                    engine.connectControl("[Channel2]","file_bpm","HerculesMp3.bpconnectB");
                }
                else
                    {
                    engine.connectControl("[Channel1]", "playposition", "HerculesMp3.playpositionA",true);
                    engine.connectControl("[Channel1]","file_bpm","HerculesMp3.bpconnectA",true);
                    engine.connectControl("[Channel2]", "playposition", "HerculesMp3.playpositionB",true);
                    engine.connectControl("[Channel2]","file_bpm","HerculesMp3.bpconnectB",true);
                    }
                }
            }

            break;

    }

};



HerculesMp3.pitchpot = function (group, control, value, status) {

    var n = HerculesMp3.controls.inputs[control].channel;
    var fxdeck = n ==1 ? 0x07 : 0x01;

    var currentMode = HerculesMp3.controls.inputs[fxdeck].mode;

    var name = "none";

    if (currentMode == 0){
        for ( fxcontrol in HerculesMp3.controls.inputs){
            if(HerculesMp3.controls.inputs[fxcontrol].channel == n && HerculesMp3.controls.inputs[fxcontrol].used)
            {
                name = HerculesMp3.controls.inputs[fxcontrol].name;
            }
        }
    }

    if (name == "fx 1"){ 
        chan = Deck[n];
        action = "pregain";
        min = -4;
        max = 4;
    }
    else if (name == "fx 2"){
        chan = "[Master]";
        action="headVolume";
        min=0;
        max=5;
    }
    else if (name == "fx 3"){
        chan = "[Master]";
        action="headMix";
        min=-1;
        max=1;
    }
    else {
        chan = Deck[n];
        action = "rate";
        min = -1;
        max = 1;
    }


    if(value){
        var increment = (max-min)/50;
        increment = (value <= 0x3F) ? increment : increment * -1;
        var newValue = engine.getValue(chan,action) + increment;
        newValue = newValue > max ? max : newValue < min ? min : newValue;
        engine.setValue(chan,action,newValue);
    }
};


HerculesMp3.jog_wheel = function (group, control, value, status) {
    var n = HerculesMp3.controls.inputs[control].channel;
    if(scratch[n]){
        if(value){
            if (scratch_timer_on[n])
            {
                engine.stopTimer(scratch_timer[n]);
                scratch_timer_on[n] = false;
            }
            if (!engine.getValue(Deck[n],"scratch2_enable")){
                engine.scratchEnable(n, 128, 33+1/3, 1.0/8*(0.500), (1.0/8)*(0.500)/32);
            }
            else {
            var newValue=(value-0x40);
            engine.scratchTick(n,-newValue);
            }
        }

        if(engine.getValue(Deck[n],"scratch2_enable"))
        {
            //when not moved for 200 msecs, probably we are not touching the wheel anymore
            scratch_timer[n] = engine.beginTimer(200,"HerculesMp3.jog_wheelhelper("+n+")",true);
            scratch_timer_on[n] = true;
        }
    }
    else {
        if(value){
            if (!engine.getValue(Deck[n],"play")) {
                var jogValue = value >=0x40 ? value - 0x80 : value;
                jogValue = jogValue*jog_sens;
                engine.setValue(Deck[n],"jog", jogValue);
                
            }
        }
    }
};


HerculesMp3.jog_wheelhelper = function(n) {
    engine.scratchDisable(n);
    scratch_timer_on[n] = false;
}

HerculesMp3.joystick = function (group, control, value, status) {
    if(!value) 
        engine.setValue("[Playlist]","SelectPrevTrack", 1);
    if(value == 0x7F) 
        engine.setValue("[Playlist]","SelectNextTrack",1);
};



HerculesMp3.loadSelectedTrack = function (group, control, value, status) {
    if (value) { // Only do stuff when pushed, not released.
        var deck = HerculesMp3.controls.inputs[control].channel;
        engine.setValue(Deck[deck] , "LoadSelectedTrack", 1);
    }
};


//This is called when bpm is calculated.
//Reusing one function for both decks caused a strange behavior and I am too lazy to fix it.

HerculesMp3.bpconnectA = function (valueA) {
       if(valueA){
        HerculesMp3.controls.outputs[0x0A].beatlen = 6000/valueA;
    }
    else	{
        HerculesMp3.controls.outputs[0x0A].beatlen=0;
    }
};

HerculesMp3.bpconnectB = function (valueB) {
    if(valueB){
        HerculesMp3.controls.outputs[0x04].beatlen = 6000/valueB;
    }
    else	{
        HerculesMp3.controls.outputs[0x04].beatlen=0;
    }
};

//These are for beat blink.

HerculesMp3.bblinkA = function (csecA) {
    var actposA = engine.getValue(Deck[1], "duration")*engine.getValue(Deck[1], "playposition")*100;
    if(engine.getValue(Deck[1],"play")){

        if(actposA) {
            if( (actposA-csecA/6)<bshot[1]*csecA && actposA>((bshot[1]-1)*csecA) ) {
                if(!HerculesMp3.controls.outputs[0x0A].status)
                    {HerculesMp3.led(0x0A, true);
                    }
            } 
            if((actposA+csecA/3)>(bshot[1]-1)*csecA && (actposA+csecA/6)<bshot[1]*csecA) {
                if(HerculesMp3.controls.outputs[0x0A].status) HerculesMp3.led(0x0A, false);
            }

        }
    }
    else
    {
        if(HerculesMp3.controls.outputs[0x0A].status)
        {
            HerculesMp3.led(0x0A, false);
        }
    }
};

HerculesMp3.bblinkB = function (csecB) {
    var actposB = engine.getValue(Deck[2], "duration")*engine.getValue(Deck[2], "playposition")*100;
    if(engine.getValue(Deck[2],"play")){

        if(actposB) {
            if( (actposB-csecB/6)<bshot[2]*csecB && actposB>((bshot[2]-1)*csecB) ) {
                if(!HerculesMp3.controls.outputs[0x04].status)
                {HerculesMp3.led(0x04, true);
                }
            }
            else {
                if((actposB+csecB/3)>(bshot[2]-1)*csecB && (actposB+csecB/6)<bshot[2]*csecB) {
                    if(HerculesMp3.controls.outputs[0x04].status) HerculesMp3.led(0x04, false);
                }
            }

        }
    }
    else
    {
        if(HerculesMp3.controls.outputs[0x04].status)
        {
            HerculesMp3.led(0x04, false);
        }
    }
};

//Find out where the next beat is.

HerculesMp3.playpositionA = function (value) {
    actposA = engine.getValue(Deck[1], "duration")*value*100;
    csecA =  HerculesMp3.controls.outputs[0x0A].beatlen;
    if(csecA) bshot[1] = Math.ceil(actposA/csecA);
};

HerculesMp3.playpositionB = function (value) {
    actposB = engine.getValue(Deck[2], "duration")*value*100;
    csecB =  HerculesMp3.controls.outputs[0x04].beatlen;
    if(csecB) bshot[2] = Math.ceil(actposB/csecB);
};
