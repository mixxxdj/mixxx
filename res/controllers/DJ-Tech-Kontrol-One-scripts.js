function KONTROL1() {}
KONTROL1.debug=false;//debug levels - 1 to 3 or false
KONTROL1.disableFlash=false;//disables the bank indicator flashers so you can see debug messages and not get flooded out with timer msgs

//set defaults
KONTROL1.flashon=0;
KONTROL1.flashcount=0;
KONTROL1.LEDtimer=false;//stores reference to led timer - fires once per second
KONTROL1.flashTimer=false;//reference to flash timer - flashes leds quickly, fired by LEDtimer
KONTROL1.curChannel=0;//set current channel (deck selector - 0 to 3 = 1 to 4 on knob)
KONTROL1.loopmove=0.0125;//default loop move distance in seconds
KONTROL1.wheelloopmove=0.003125;//default loop move distance in seconds when using wheel to move
KONTROL1.scratching=[];
KONTROL1.modTimeout=200;//mod button timeout in ms - ie: how long do you have to hold a mod (like a knob button) to disable the bank switch or whatever it would normally do

KONTROL1.bankIndicatorLED={bank1:0x37, bank2:0x36, bank3:0x35, bank4:0x34};//midi address of bank indicator LEDs (knob leds)
KONTROL1.defaultDeck={0:"[Channel1]", 1:"[Channel2]", 2:"[Channel3]", 3:"[Channel4]"};//default deck for each channel
KONTROL1.altDeck={0:"[Channel2]", 1:"[Channel1]", 2:"[Channel4]", 3:"[Channel3]"};//alt deck for each channel
KONTROL1.syncDeck={0:"[Sampler1]", 1:"[Sampler2]", 2:"[Sampler3]", 3:"[Sampler4]"};//sync deck for each channel
KONTROL1.beatloopLengths=new Array(0,0.03125,0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32);//store loop lengths with presets in Mixxx

KONTROL1.beatlooprollactive={};
KONTROL1.beatlooprolllen={};

KONTROL1.overwriteHotcues=true;//when saving loop in points as hotcues, will the script overwrite an existing hotcue?

//scratch parameters
KONTROL1.wheelRes=150;
KONTROL1.scratchRev=160;
KONTROL1.scratchAlpha=1/16;
KONTROL1.scratchBeta=KONTROL1.scratchAlpha/32;
KONTROL1.scratchInterpolation=1;






//obj to store info specific to the state of each channel
KONTROL1.channels={};
KONTROL1.channels[0]={curShift:"a",curBank:"bank1",cc:0xb0,noteon:0x90};
KONTROL1.channels[1]={curShift:"a",curBank:"bank1",cc:0xb1,noteon:0x91};
KONTROL1.channels[2]={curShift:"a",curBank:"bank1",cc:0xb2,noteon:0x92};
KONTROL1.channels[3]={curShift:"a",curBank:"bank1",cc:0xb3,noteon:0x93};
//shift state is "a" off or "b" on
//banks are "bank1", "bank2", "bank3", or "bank4"
//plug additional state variables into this object - mutes on/off, kills on/off, etc.


//obj to store mod values - mods are pitch +-, Knob buttons, and shift when held down.
KONTROL1.mod={};
KONTROL1.mod["p1"]={state:"o",timer:false};
KONTROL1.mod["p2"]={state:"o",timer:false};
KONTROL1.mod["k1"]={state:"o",timer:false,bank:"bank1",dobankswitch:true};
KONTROL1.mod["k2"]={state:"o",timer:false,bank:"bank2",dobankswitch:true};
KONTROL1.mod["k3"]={state:"o",timer:false,bank:"bank3",dobankswitch:true};
KONTROL1.mod["k4"]={state:"o",timer:false,bank:"bank4",dobankswitch:true};
KONTROL1.mod["s"]={state:"o",timer:false,doshift:true};

//if (KONTROL1.debug){print("#####WORD#####")};



//############################################################################
//INIT AND SHUTDOWN
//############################################################################

KONTROL1.init = function init(id, debug) { // called when the device is opened & set up
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    KONTROL1.updateLEDs();
    if(KONTROL1.disableFlash!==true)KONTROL1.ledTimer = engine.beginTimer(1000, "KONTROL1.doLEDs()");//set timer for LED indicator flashes
    engine.connectControl("[Channel1]", "volume", "KONTROL1.testconnect");
    };

KONTROL1.shutdown = function shutdown() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    };



//############################################################################
//INPUT PROCESSING
//############################################################################

KONTROL1.button = function button(channel, control, value, status, group) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //input received - gatekeeper function connected to most buttons
    //check channel - see if the deck selector has been moved
    KONTROL1.checkChannel(channel);

    //get the commands for the current control on the current bank
    var thecontrol=KONTROL1.getControl(control);
    var command = (value==127)?thecontrol[0]:thecontrol[1];
    if (command!==false && command !='ReleaseEvalStr' && command!='ButtonFunctionEvalStr')eval(command);
    };

KONTROL1.knob = function knob(channel, control, value, status, group) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //input received - gatekeeper function connected to most knobs
    //check channel - see if the deck selector has been moved
    KONTROL1.checkChannel(channel);

    //get the commands for the current control on the current bank - press function only - release functions are invalid on knobs
    var thecontrol=KONTROL1.getControl(control);
    var command = thecontrol[0];
    if (command!==false && command!='ButtonFunctionEvalStr')eval(command);
    };


//MOD functions
KONTROL1.knobPress = function knobPress(knobnum){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //{state:"o",timer:false,dobankswitch:true};
    KONTROL1.modPress(knobnum);//turn mod on
    engine.stopTimer(KONTROL1.mod[knobnum]["timer"]);//kill any previous timer
    KONTROL1.mod[knobnum]["timer"]=engine.beginTimer(KONTROL1.modTimeout, "KONTROL1.disableBankSwitch('"+knobnum+"')", true);
    }

KONTROL1.knobRelease = function knobRelease(knobnum){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //switches banks when one of the four knob buttons is released
    //knobnum is k1, k2, k3, or k4
    //unless button has been held down for more than 1/2 second, in which case the knob is used as a modifier
    if (KONTROL1.mod[knobnum]["dobankswitch"]===true){
        engine.stopTimer(KONTROL1.mod[knobnum]["timer"]);
        KONTROL1.switchBank(KONTROL1.mod[knobnum]["bank"]);
        KONTROL1.mod[knobnum]["dobankswitch"]=true;
        KONTROL1.modRelease(knobnum);//turn mod off
        }else{
        //reset
        KONTROL1.mod[knobnum]["dobankswitch"]=true;
        KONTROL1.modRelease(knobnum);//turn mod off
        }

    //exit knob loop if no loop was active when knob pressed.
    if (KONTROL1.exitKnobLoop===true && knobnum=="k4"){
        var group=KONTROL1.getGroup("default");
        engine.setValue(group, "reloop_exit", 1);
        KONTROL1.exitKnobLoop=false;
        }
    };

KONTROL1.shiftPress = function shiftPress(){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    var knobnum="s";
    KONTROL1.modPress(knobnum);//turn mod on
    engine.stopTimer(KONTROL1.mod[knobnum]["timer"]);//kill any previous timer
    KONTROL1.mod[knobnum]["timer"]=engine.beginTimer(500, "KONTROL1.disableShiftSwitch('"+knobnum+"')", true);
    }

KONTROL1.shiftRelease = function shiftRelease(){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    var knobnum="s";
    //switches shift state
    //unless button has been held down for more than 1/2 second, in which case the knob is used as a modifier
    if (KONTROL1.mod[knobnum]["doshift"]===true){
        engine.stopTimer(KONTROL1.mod[knobnum]["timer"]);
        KONTROL1.switchShift(KONTROL1.mod[knobnum]["bank"]);
        KONTROL1.mod[knobnum]["doshift"]=true;
        KONTROL1.modRelease(knobnum);//turn mod off
        }else{
        engine.stopTimer(KONTROL1.mod[knobnum]["timer"]);
        //reset
        KONTROL1.mod[knobnum]["doshift"]=true;
        KONTROL1.modRelease(knobnum);//turn mod off
        }
    };

KONTROL1.modPress = function modPress(knobnum){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    KONTROL1.mod[knobnum]["state"]="I";//turn mod on
    if (KONTROL1.mod["p1"]["state"]=="I" && KONTROL1.mod["p2"]["state"]=="I"){//both pitch buttons on, force update of all LEDS
        KONTROL1.updateLEDs(false, true);
    } else {KONTROL1.updateLEDs();}
    }

KONTROL1.modRelease = function modRelease(knobnum){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    if (KONTROL1.mod["p1"]["state"]=="I"){
        //reset cue move stuff
        engine.stopTimer(KONTROL1.cueMoveIndicator);
        KONTROL1.cueMoveToNum=-1;
        KONTROL1.cuemoveLastIndicator=-1;
    }
    
    KONTROL1.mod[knobnum]["state"]="o";//turn mod off
    
    KONTROL1.updateLEDs();
    }



//############################################################################
//LED FUNCTIONS - indicator functions
//############################################################################

KONTROL1.doLEDs = function doLEDs() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    engine.stopTimer(KONTROL1.flashTimer);
    KONTROL1.flashTimer=engine.beginTimer(30, "KONTROL1.bankIndicators()");
    return;    
    };

KONTROL1.bankIndicators= function bankIndicators(){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //flash knob LEDs to indicate current bank
    var cc = KONTROL1.channels[KONTROL1.curChannel]['cc'];
    var address = KONTROL1.bankIndicatorLED[KONTROL1.channels[KONTROL1.curChannel]['curBank']];
    if (KONTROL1.flashon==0){
        if (KONTROL1.flashcount>=2){KONTROL1.stopFlashTimer();return;}
        midi.sendShortMsg(cc, address, 0);
        KONTROL1.flashon=1;
        KONTROL1.flashcount++;
        }else {
        if (KONTROL1.flashcount>=2){KONTROL1.stopFlashTimer();return;}
        midi.sendShortMsg(cc, address, 127);
        KONTROL1.flashon=0;
        KONTROL1.flashcount++;
        }
    }

KONTROL1.stopFlashTimer = function stopFlashTimer() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    engine.stopTimer(KONTROL1.flashTimer);
    KONTROL1.flashcount=0;
    var cc = KONTROL1.channels[KONTROL1.curChannel]['cc'];
    var address = KONTROL1.bankIndicatorLED[KONTROL1.channels[KONTROL1.curChannel]['curBank']];
    KONTROL1.updateLEDs(true);
    }

KONTROL1.updateLEDs = function updateLEDs(knobsonly, force) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //fires on mod press/release, shift switch, channel switch, bank switch
    //updates all LEDs
    var state;
    var samehook=false;
    var hookgroup;
    var midino;
    var control;
    var controllist=(knobsonly===true)?KONTROL1.knobLEDs:KONTROL1.allLEDs;
    var force=(force===true)?force:false;
    
    //clear hooks
    for (var key in controllist){
        midino=controllist[key];
        control = KONTROL1.getControl(midino);
        if ((control[2]==false || control[2]=='LEDstateType') && force !== true){continue;}//control has no LED, LED Type is not set, or LED doesn't change on this mod setting.  skip to next control
        
        //clear hooks
        if (KONTROL1.controls[midino]["hookSet"]===true){
            hookgroup=KONTROL1.getGroup(control[5]);//get mixxx control group
            //clear the hook
            engine.connectControl(KONTROL1.controls[midino]["hookGroup"], KONTROL1.controls[midino]["hookMixxxControl"], KONTROL1.controls[midino]["hookFunction"], true);
            KONTROL1.controls[midino]["hookSet"]=false;
            }//end if
        }

    for (var key in controllist){//set hooks
        midino=controllist[key];
        control = KONTROL1.getControl(midino);
        if ((control[2]==false || control[2]=='LEDstateType') && force !== true){continue;}//control has no LED, LED Type is not set, or LED doesn't change on this mod setting.  skip to next control
        

        if (control[2]=="on"){//LED is always on
            state=127;
            }
        else if (control[2]=="off"){//LED is always off
            state=0;
            }
        else if (control[2]=="hook"){//LED is hooked to a mixxx control
            print("#####-----SETTING HOOK##"+hookgroup+" - "+control[3]+" - "+control[4]);
            hookgroup=KONTROL1.getGroup(control[5]);//get mixxx control group
            engine.connectControl(hookgroup, control[3], control[4]);//set hook
            KONTROL1.controls[midino]["hookGroup"]=hookgroup;
            KONTROL1.controls[midino]["hookMixxxControl"]=control[3];
            KONTROL1.controls[midino]["hookFunction"]=control[4];
            KONTROL1.controls[midino]["hookSet"]=true;
            engine.trigger(hookgroup, control[3]);
            continue;
            }
        else if (control[2]=="eval"){//LED state determined by evaluating a javascript statement
            if (KONTROL1.debug>2){print("##updateLEDs Eval: "+control[3])};
            if (KONTROL1.debug>2){print("##updateLEDs midino: "+midino)};
            if (KONTROL1.debug>2){print("##evalstate: "+eval(control[3]))};
            state=eval(control[3]);
            }//end if
        //figure out whether we want a CC or noteon message - CC for knobs, noteon otherwise
        if (midino==0x37 || midino==0x36 || midino==0x35 || midino==0x34){
            ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
            if (KONTROL1.debug>2){print("ch: "+ch)};
            if (KONTROL1.debug>2){print("midino: "+midino)};
            }else{
            ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
            }
        midi.sendShortMsg(ch, midino, 0);
        midi.sendShortMsg(ch, midino, state);
        state=0;
        }//end for
    }

//connected control functions
KONTROL1.doLogKnobLEDs = function doLogKnobLEDs(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //show values of mixxx control on knob leds
    //logarithmic scale (0 min, 4 max) - used for pregain and lo mid hi filters, for example.
    //K = 4*(V)^2
    //V = (K/4)^0.5 -- K= value coming from prog (0-4, logarithmic scale), V=value adjusted to linear 0-1 
    if (KONTROL1.debug>2){print("unadjusted value: "+value)};
    value=(Math.pow((value/4),.5))*127;
    if (value<70){value+=10;}//add a bit to center 50% mark
        else if (value<110){value+=5;}
        else if (value<120){value+=2;}
    value=(value>127)?127:value;
    value=(value<0)?0:value;
    if (KONTROL1.debug>2){print("value: "+value)};
    var midino;
    var ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
    if (control=="pregain"){midino=0x37;}
    if (control=="filterHigh"){midino=0x36;}
    if (control=="filterMid"){midino=0x35;}
    if (control=="filterLow"){midino=0x34;}
    
    midi.sendShortMsg(ch, midino, value);
    }
 
KONTROL1.doHotcueLEDs = function doHotcueLEDs(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    value=value*127;
    if (KONTROL1.debug>2){print("value: "+value)};
    var hotcuenum=control.replace("hotcue_", "");
    var hotcuenum=hotcuenum.replace("_enabled", "");
    var ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    var midino=KONTROL1.hotcueLEDs[hotcuenum];
    
    midi.sendShortMsg(ch, midino, value);
    }

KONTROL1.doFilterKillLEDs = function doFilterKillLEDs(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    value=value*127;
    ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
    if (control=="filterHighKill"){midino=0x08}    
    if (control=="filterMidKill"){midino=0x07}    
    if (control=="filterLowKill"){midino=0x06}    
    midi.sendShortMsg(ch, midino, value);
    }

KONTROL1.syncLED = function syncLED(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
    value=(value>0)?1:0;
    ledstate=value*127;
    midi.sendShortMsg(ch, KONTROL1.getIt['sync'], ledstate);
    }

KONTROL1.cueLED = function cueLED(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    value=(value>0)?1:0;
    ledstate=value*127;
    midi.sendShortMsg(ch, KONTROL1.getIt['cue'], ledstate);
    }

KONTROL1.playLED = function playLED(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
    value=(value>0)?1:0;
    ledstate=value*127;
    midi.sendShortMsg(ch, KONTROL1.getIt['play'], ledstate);
    }

KONTROL1.loopLenLEDs = function loopLenLEDs(deck, num){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //light LEDS for loop length dial
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);

    if(typeof num === 'undefined'){
        num=false;
        for (i=1;i<11;i++){
            if (engine.getValue(group, "beatloop_"+KONTROL1.beatloopLengths[i]+"_enabled")==1){num=i;break;}
            }
        if (num==false){num=8;}//default to 2 beat loop
        }
    num=(num*12.8)-1;
    print("num="+num);
    ch=KONTROL1.channels[KONTROL1.curChannel]['cc'];
    midi.sendShortMsg(ch, 0x34, num);
    }


KONTROL1.lightButtonLEDs = function lightButtonLEDs(value,group,control){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //light button leds for simple binary values
    //set KONTROL1.buttonLEDs object to contain new addresses as required
    ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    value=value*127;
    value=(value>127)?127:value;
    value=(value<0)?0:value;
    if (typeof(KONTROL1.buttonLEDs[control])!= 'undefined'){
        var midino=KONTROL1.buttonLEDs[control];
        midi.sendShortMsg(ch, midino, value);
    }
};

KONTROL1.LoopBankLEDs = function LoopBankLEDs(value,group,control){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //light button leds for simple binary values
    //set KONTROL1.buttonLEDs object to contain new addresses as required
    ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    value=value*127;
    value=(value>127)?127:value;
    value=(value<0)?0:value;
    if (typeof(KONTROL1.loopbuttonLEDs[control])!= 'undefined'){
        var midino=KONTROL1.loopbuttonLEDs[control];
        midi.sendShortMsg(ch, midino, value);
    }
};

KONTROL1.resumeLED = function resumeLED() {//lights sync button if a resume position is set
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    var state;
    if (KONTROL1.channels[KONTROL1.curChannel]["resumepos"]>-1){
        state=127;
    }else{
        state=0;
    }//end if
    var ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    midi.sendShortMsg(ch, 0x1d, state);
    return state;
    };

KONTROL1.scratchParameterLEDs = function scratchParameterLEDs(midino,thearray,theindex) {//lights knobs for scratch parameter adjustment
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var state=Math.round((127/thearray.length)*theindex);
    state=(state>127)?127:state;
    state=(state<0)?0:state;

    var ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
    midi.sendShortMsg(ch, midino, state);
    return state;
    };


//############################################################################
//CHANNEL AND BANK SWITCHING
//############################################################################

KONTROL1.checkChannel = function checkChannel(channel){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //checks to see whether the channel has changed (deck selector knob sends no midi when changed,
    //so every time input is received, this function checks its midi channel to see if it's different from the last)
    if (KONTROL1.curChannel!=channel){
        //channel has changed.
        KONTROL1.curChannel=channel;
        //add in any functions to run here.  Eg: update LEDs, etc. [TODO]
        KONTROL1.updateLEDs();
        };
    };

KONTROL1.disableBankSwitch = function disableBankSwitch(knobnum) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //timed function - fires half a second after pressing knob.
    //Don't do the bank switch if you hold down the button (if knob button is used as modifier)
    KONTROL1.mod[knobnum]["dobankswitch"]=false;
    };

KONTROL1.switchBank = function switchBank(bank){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    KONTROL1.channels[KONTROL1.curChannel]["curBank"]=bank;
    //add in any functions to run here.  Eg: update LEDs, etc. [TODO]
    KONTROL1.updateLEDs();
    };

KONTROL1.disableShiftSwitch = function disableShiftSwitch(knobnum) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //timed function - fires half a second after pressing knob.
    //Don't do the bank switch if you hold down the button (if knob button is used as modifier)
    KONTROL1.mod[knobnum]["doshift"]=false;
    };

KONTROL1.switchShift = function switchShift(bank){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    KONTROL1.channels[KONTROL1.curChannel]["curShift"]=(KONTROL1.channels[KONTROL1.curChannel]["curShift"]=="a")?"b":"a";
    //add in any functions to run here.  Eg: update LEDs, etc. [TODO]
    KONTROL1.updateLEDs();
    };



//############################################################################
//MUSIC FUNCTIONALITY - stuff that actually does stuff
//############################################################################

KONTROL1.toggleBinaryControl = function toggleBinaryControl(control, newstate){//toggles a binary control.  If newstate is provided, it will toggle to that state, otherwise it toggles on/off
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group = KONTROL1.getGroup(deck);
    
    if (newstate!==1 && newstate!==0){var newstate = (engine.getValue(group, control)==0)?1:0;}
    
    engine.setValue(group, control, newstate);
    }


KONTROL1.logKnobAdjust = function logKnobAdjust(group, control, minRange, maxRange, value){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //use a knob to adjust a mixxx control
    //on a logarithmic scale (0 min, 4 max)
    //K = 4*(V)^2
    //V = (K/4)^0.5 -- K= value coming from prog (0-4, logarithmic scale), V=value adjusted to linear 0-1 
    var curValue=engine.getValue(KONTROL1.getGroup(group), control);
    var adjustedValue=Math.pow((curValue/4),.5);//adjust range so it's on a scale of 0 to 1 - logarithmic

    var inc=(value==127)?.02:-.02;//increment - how far to move on each knob click

    var newValue=4*Math.pow((adjustedValue+inc),2);

    if (KONTROL1.debug>2){print("group: "+KONTROL1.getGroup(group))};
    if (KONTROL1.debug>2){print("control: "+control)};
    if (KONTROL1.debug>2){print("curValue: "+curValue)};
    if (KONTROL1.debug>2){print("adjustedValue: "+adjustedValue)};
    if (KONTROL1.debug>2){print("newValue: "+newValue)};
    if (KONTROL1.debug>2){print("inc: "+inc)};
    if (newValue>4)newValue=4;
    if (newValue<0)newValue=0;
    engine.setValue(KONTROL1.getGroup(group), control, newValue);
    }

KONTROL1.wheelTouch = function wheelTouch(channel, control, value, status, group) {//activate scratching
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    if (KONTROL1.debug>2){print("channel: "+channel)};

    if (value == 0x7F) {//wheel is being touched
        engine.scratchEnable(channel+1, KONTROL1.wheelResArray[KONTROL1.wheelResIndex]*KONTROL1.scratchInterpolation, KONTROL1.scratchRevArray[KONTROL1.scratchRevIndex], KONTROL1.scratchAlphaArray[KONTROL1.scratchAlphaIndex], KONTROL1.scratchAlphaArray[KONTROL1.scratchAlphaIndex]/KONTROL1.scratchBetaArray[KONTROL1.scratchBetaIndex]);
        // Keep track of whether we're scratching on this virtual deck - for v1.10.x or below
        KONTROL1.scratching[group] = true;
    } else {//wheel is released
        engine.scratchDisable(channel+1);
        KONTROL1.scratching[group] = false;  // Only for v1.10.x and below
        }
    };

KONTROL1.wheelTurn = function wheelTurn(channel, control, value, status, group) {//do scratching
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    if (KONTROL1.debug>2){print("channel: "+channel)};

    if (value>1){var newValue=1}else{var newValue=-1};
    if (!KONTROL1.scratching[group]){//do jog or other functions (move loops, hotcues, etc.) if not scratching
        
        //check if a loop needs to be moved
        if (KONTROL1.loopbuttonDown === true || KONTROL1.loopinbuttonDown === true || KONTROL1.loopoutbuttonDown === true){
            if (KONTROL1.wheelMoveLoop(value)===true){return true;}//function returns true if loop moved
        }
        
        engine.setValue(group, "jog", newValue);    
        return true;
        };
    for (i=0; i<KONTROL1.scratchInterpolation; i++){
        engine.scratchTick(channel+1,newValue);
    }
    };

KONTROL1.hotcueButton = function hotcueButton(hotcuenum, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group = KONTROL1.getGroup(deck);
    //activate 
    if (KONTROL1.mod["p1"]["state"]=="I"){
        engine.setValue(group, "hotcue_"+hotcuenum+"_clear", 1);
        if (KONTROL1.debug>2){print("cleared")};
        } else {
        engine.setValue(group, "hotcue_"+hotcuenum+"_activate", 1);
        engine.setValue(group, "hotcue_"+hotcuenum+"_activate", 0);
        if (KONTROL1.debug>2){print("activated")};
        };

    };

KONTROL1.cueClear = function cueClear(cue, control, deck){//clear hotcue - OR move hotcue to new button
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group = KONTROL1.getGroup(deck);

    
    if(engine.getValue(group, "hotcue_"+cue+"_enabled")!=true){//hotcue not set - prepare to move next hotcue pressed to this button
        KONTROL1.cueMoveToNum=cue;
        engine.stopTimer(KONTROL1.cueMoveIndicator);
        KONTROL1.cueMoveIndicator=engine.beginTimer(100, "KONTROL1.cueMoveIndicatorLEDs("+control+")");//start timer for LED indicator flasher showing the button we're moving to
        return true;
    }
    
    if(KONTROL1.cueMoveToNum>0){//hotcue set, but we're moving it, not clearing it.
        engine.setValue(group, "hotcue_"+KONTROL1.cueMoveToNum+"_set", 1);
        engine.setValue(group, "hotcue_"+KONTROL1.cueMoveToNum+"_position", engine.getValue(group, "hotcue_"+cue+"_position"));    
        engine.setValue(group, "hotcue_"+cue+"_clear", 1);
        engine.setValue(group, "hotcue_"+cue+"_clear", 0);
    
        engine.stopTimer(KONTROL1.cueMoveIndicator);
        KONTROL1.cueMoveToNum=-1;//reset
        KONTROL1.cuemoveLastIndicator=-1;
        return true;
    }
    
    engine.setValue(group, "hotcue_"+cue+"_clear", 1);
    engine.setValue(group, "hotcue_"+cue+"_clear", 0);
}

KONTROL1.cueMoveIndicatorLEDs = function cueMoveIndicatorLEDs(control){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    ch=KONTROL1.channels[KONTROL1.curChannel]['noteon'];
     
    KONTROL1.cuemovecontrol=control;
    if (KONTROL1.cuemoveLastIndicator!=-1){midi.sendShortMsg(ch, KONTROL1.cuemoveLastIndicator, 0);}//turn off last indicator in case timer was interrupted (keeps last led from being left on if you switch "move to" buttons.
    KONTROL1.cuemoveLastIndicator=control;
    KONTROL1.cuemoveflashon=(KONTROL1.cuemoveflashon!=0 && KONTROL1.cuemoveflashon!=1)?0:KONTROL1.cuemoveflashon;

    if (KONTROL1.cuemoveflashon==0){
        midi.sendShortMsg(ch, control, 0);
        KONTROL1.cuemoveflashon=1;
        }else {
        midi.sendShortMsg(ch, control, 127);
        KONTROL1.cuemoveflashon=0;
        }

    return true;
};

KONTROL1.cueLoop = function cueLoop(cue, len){//jump to hotcue and start loop
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group = KONTROL1.getGroup(deck);

    if (engine.getValue(group, "hotcue_"+cue+"_enabled")!==1){//set hotcue and start loop
        engine.setValue(group, "hotcue_"+cue+"_activate", 1);
        engine.setValue(group, "hotcue_"+cue+"_activate", 0);
        engine.setValue(group, "beatloop_"+len+"_activate", 1);
        }else{//jump to existing cue and loop
        
        //calculate start and end points
        var startpos=engine.getValue(group, "hotcue_"+cue+"_position");
        var loopseconds = len*(1/(engine.getValue(group, "bpm")/60));
        var loopsamples = loopseconds*engine.getValue(group, "track_samplerate")*2;//*2 to compensate for stereo samples
        var endpos=startpos+loopsamples;

        //disable loop if currently enabled
        if (engine.getValue(group, "loop_enabled")==true){engine.setValue(group, "reloop_exit", 1);}
        
        //set start and endpoints
        engine.setValue(group, "loop_start_position", startpos);
        engine.setValue(group, "loop_end_position", endpos);
        //enable loop    
        engine.setValue(group, "reloop_exit", 1);
    }
}

KONTROL1.kill = function kill(freq, value, deck){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //kills low mid high, or mutes all by recording current track volume level, switching to 0, then restoring previous volume level
    //if P1 mod is on when button pressed, it toggles on and off.  Otherwise it's momentary
    //args: freq = "all", "low", "mid", or "high"
    //value: 127=button pressed ; 0=button released
    var group=KONTROL1.getGroup(deck);

    if (value==127){//button was pressed
        if (KONTROL1.channels[KONTROL1.curChannel]['kill'+freq]===true){
            //kill is already on...  change setting so it clears on release
            KONTROL1.channels[KONTROL1.curChannel]['kill'+freq]=false;
            return true;
            } else
        if (KONTROL1.mod["p1"]["state"]=="I"){//mod is on, set toggle
            KONTROL1.channels[KONTROL1.curChannel]['kill'+freq]=true;
            }
        if (freq=="low"){engine.setValue(group, "filterLowKill", 1);}else
        if (freq=="mid"){engine.setValue(group, "filterMidKill", 1);}else
        if (freq=="high"){engine.setValue(group, "filterHighKill", 1);}else
        if (freq=="all"){
            //mute
            curvol=engine.getValue(group, "volume");
            if (curvol==0){return;}//do nothing if volume is already zeroyyz
            engine.setValue(group, "volume", 0);
            KONTROL1.channels[KONTROL1.curChannel]['mutestoredvol']=curvol;
            KONTROL1.channels[KONTROL1.curChannel]["isMuted"]=true;
            (KONTROL1.channels[KONTROL1.curChannel]["isMuted"]==true)?127:0;
            midi.sendShortMsg(KONTROL1.channels[KONTROL1.curChannel]['noteon'], 0x09, 127);//light led
            }
        }else{//button was released
        if (KONTROL1.channels[KONTROL1.curChannel]['kill'+freq]===true){
            //kill is toggled...  ignore button release
            return true;
            }        
        if (freq=="low"){engine.setValue(group, "filterLowKill", 0);}else
        if (freq=="mid"){engine.setValue(group, "filterMidKill", 0);}else
        if (freq=="high"){engine.setValue(group, "filterHighKill", 0);}else
        if (freq=="all"){
            //unmute
            engine.setValue(group, "volume", KONTROL1.channels[KONTROL1.curChannel]['mutestoredvol']);
            KONTROL1.channels[KONTROL1.curChannel]["isMuted"]=false;
            midi.sendShortMsg(KONTROL1.channels[KONTROL1.curChannel]['noteon'], 0x09, 0);//light led
            }
        }
    }

KONTROL1.resetKnob = function resetKnob(control, deck){//resets gain, low mid high to 50%
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    engine.setValue(group, control, 1);
    }

KONTROL1.play = function play(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    var state=(engine.getValue(group, "play")==1)?0:1;
    if (value>0){//button was pressed
        if (KONTROL1.debug){print("play newstate: "+state)};
        if (KONTROL1.debug){print("play group: "+group)};
        engine.setValue(group, "play", state);
        } else {//button was released
        };
    
}

KONTROL1.cue = function play(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    if (value>0){//button was pressed
        engine.setValue(group, "cue_default", 1);
        } else {//button was released
        };
    
}

//LOOPS
KONTROL1.beatLoop = function beatLoop(looplen, value, deck) {//activate beatloop
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    //activate beatloop
    if (value>0){//button was pressed
        KONTROL1.loopbuttonDown=true;
        if (engine.getValue(group, "beatloop_"+looplen+"_enabled")!=1){//if it's not already enabled (this is to prevent turning a beatlooproll into a beatloop of the same length
            engine.setValue(group, "beatloop_"+looplen+"_activate", 1);
            }
        } else {//button was released
        KONTROL1.loopbuttonDown=false;
        };

    };

KONTROL1.beatLoopRoll = function beatLoopRoll(looplen, value, deck) {//activate beatloop
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    //activate beatlooproll
    if (value>0){//button was pressed
        KONTROL1.loopbuttonDown=true;
        engine.setValue(group, "beatlooproll_"+looplen+"_activate", 1);
        KONTROL1.beatlooprollactive[group]=true;
        KONTROL1.beatlooprolllen[group]=looplen;
        } else {//button was released
        KONTROL1.loopbuttonDown=false;
        };

    };

KONTROL1.beatjump = function (jumplen) {//jumps back jumplen beats
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    var curpos = engine.getValue(group, "playposition")*engine.getValue(group, "track_samples");
    var numbeats = jumplen;
    var backseconds = numbeats*(1/(engine.getValue(group, "bpm")/60));
    var backsamples = backseconds*engine.getValue(group, "track_samplerate")*2;//*2 to compensate for stereo samples
    var newpos = curpos-(backsamples);
    
    if (KONTROL1.debug){print("backseconds: "+backseconds);}
    if (KONTROL1.debug){print("backsamples: "+backsamples);}
    if (KONTROL1.debug){print("curpos: "+curpos);}
    if (KONTROL1.debug){print("newpos: "+newpos);}
    if (KONTROL1.debug){print("numbeats: "+numbeats);}
    
    engine.setValue(group, "playposition", newpos/engine.getValue(group, "track_samples"));
    };

KONTROL1.saveLoop = function saveLoop(cue){//save the current loop inpoint as a hotcue
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);

    if (engine.getValue(group, "hotcue_"+cue+"_enabled")==1 && KONTROL1.overwriteHotcues===false){//hotcue is already set, return false
        return false;
        }else if (engine.getValue(group, "loop_enabled")!=true){//no loop currently active, return false
        return false;
        }else{//save the loop as a hotcue
        engine.setValue(group, "hotcue_"+cue+"_set", 1);
        engine.setValue(group, "hotcue_"+cue+"_position", engine.getValue(group, "loop_start_position"));
        return true;
    }
}

KONTROL1.reloopButton = function reloopButton(value, deck) {//reloop/exit button
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (value>0){//button was pressed
        engine.stopTimer(KONTROL1.reloopTimer);
        KONTROL1.loopbuttonDown=true;
        KONTROL1.doreloop=true;
        KONTROL1.reloopTimer = engine.beginTimer(500, "KONTROL1.disablereloop()", true);
        } else {//button was released
        KONTROL1.loopbuttonDown=false;
        if (KONTROL1.doreloop===true) {
            if (KONTROL1.beatlooprollactive[group]===true && KONTROL1.beatlooprolllen[group]>0){
                engine.setValue(group, "beatlooproll_"+KONTROL1.beatlooprolllen[group]+"_activate", 0);
                KONTROL1.beatlooprollactive[group]=false;
                KONTROL1.beatlooprolllen[group]=-1;
                }else{
                engine.setValue(group, "reloop_exit", 1);
                }
            
            };
        KONTROL1.doreloop=true;
        engine.stopTimer(KONTROL1.reloopTimer);
        };

    };

KONTROL1.disablereloop = function disablereloop() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
    KONTROL1.doreloop=false;
    };

KONTROL1.looplenDial = function looplenDial(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //activates variable length loop depending on dial position

    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    //is a loop active?  If not, exit loop when knob button is released
    if (engine.getValue(group, "loop_enabled")!=1){
        KONTROL1.exitKnobLoop=true;
        }
    
    //determine active loop length, default to 2 beats
    var arraypointer=false;
    var inc=(value==127)?1:-1;//increment - how far to move on each knob click
    for (i=1;i<12;i++){
        if (engine.getValue(group, "beatloop_"+KONTROL1.beatloopLengths[i]+"_enabled")==1){arraypointer=i;break;}
        }
    if (arraypointer==false){arraypointer=8;inc=0;}//default to 2 beat loop, don't increment
    arraypointer+=inc;
    arraypointer=(arraypointer<1)?1:arraypointer;
    arraypointer=(arraypointer>11)?11:arraypointer;

    var newLoopLen=KONTROL1.beatloopLengths[arraypointer];
    engine.setValue(group, "beatloop_"+newLoopLen+"_activate", 1);
    KONTROL1.loopLenLEDs("default",arraypointer);
    return true;
    };

KONTROL1.loopIn = function loopIn(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //Set or move loop in point
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (value>0){//button was pressed
        KONTROL1.loopinbuttonDown=true;
        KONTROL1.doloopin=true;
        KONTROL1.loopinbuttonTimer = engine.beginTimer(500, "KONTROL1.disableloopin()", true);
        } else {//button was released
        KONTROL1.loopinbuttonDown=false;
        if (KONTROL1.doloopin===true) {engine.setValue(group, "loop_in", 1);engine.setValue(group, "loop_in", 0);};
        KONTROL1.doloopin=true;
        engine.stopTimer(KONTROL1.loopinbuttonTimer);
        };
}

KONTROL1.loopOut = function loopOut(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //set or move loop out point
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (value>0){//button was pressed
        KONTROL1.loopoutbuttonDown=true;
        KONTROL1.doloopout=true;
        KONTROL1.loopoutbuttonTimer = engine.beginTimer(500, "KONTROL1.disableloopout()", true);
        } else {//button was released
        KONTROL1.loopoutbuttonDown=false;
        if (KONTROL1.doloopout===true) {engine.setValue(group, "loop_out", 1);};
        KONTROL1.doloopout=true;
        engine.stopTimer(KONTROL1.loopoutbuttonTimer);
        };
}

KONTROL1.disableloopin = function disableloopin() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
    KONTROL1.doloopin=false;
    };

KONTROL1.disableloopout = function disableloopout() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
    KONTROL1.doloopout=false;
    };

KONTROL1.loopMinus = function loopMinus(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //shrinks loop or moves loop back
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (KONTROL1.loopbuttonDown !== true && KONTROL1.loopoutbuttonDown !== true && KONTROL1.loopinbuttonDown !== true){engine.setValue(group, "loop_halve", 1); return false;}//shrink loop if no loop button down 
    else if (KONTROL1.loopbuttonDown === true && engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        if (KONTROL1.debug>2){print("##doing move both back#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start-interval);
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        }
    else if (KONTROL1.loopinbuttonDown === true && engine.getValue(group, "loop_start_position")>=0){
        //move loop in point    
        if (KONTROL1.debug>2){print("##doing move inpoint back#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        engine.setValue(group, "loop_start_position", start-interval);
        return true;
        }
    else if (KONTROL1.loopoutbuttonDown === true && engine.getValue(group, "loop_end_position")>=0){
        //move loop out point    
        if (KONTROL1.debug>2){print("##doing move outpoint back#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        }//end if
}

KONTROL1.wheelMoveLoop = function wheelMoveLoop (val){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    var val=(val>64)?-1:1;
    var interval =    val*KONTROL1.wheelloopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
    var start = engine.getValue(group, "loop_start_position");
    var end = engine.getValue(group, "loop_end_position");

    //shrinks loop or moves loop back
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (KONTROL1.loopbuttonDown !== true && KONTROL1.loopoutbuttonDown !== true && KONTROL1.loopinbuttonDown !== true){engine.setValue(group, "loop_halve", 1); return false;}//shrink loop if no loop button down 
    else if (KONTROL1.loopbuttonDown === true && start>=0 && end>=0 ){
        //move loop
        if (KONTROL1.debug>2){print("##doing move both back#####")};
        engine.setValue(group, "loop_start_position", start-interval);
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        }
    else if (KONTROL1.loopinbuttonDown === true && start>=0){
        //move loop in point    
        if (KONTROL1.debug>2){print("##doing move inpoint back#####")};
        engine.setValue(group, "loop_start_position", start-interval);
        return true;
        }
    else if (KONTROL1.loopoutbuttonDown === true && end>=0){
        //move loop out point    
        if (KONTROL1.debug>2){print("##doing move outpoint back#####")};
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        }//end if
    
    return false;
    }

KONTROL1.moveLoop = function moveLoop(moveInPoint, moveOutPoint, increment) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //
    }

KONTROL1.loopPlus = function loopPlus(value, deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //grows loop or moves loop forward
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);
    
    if (KONTROL1.loopbuttonDown !== true && KONTROL1.loopoutbuttonDown !== true && KONTROL1.loopinbuttonDown !== true){engine.setValue(group, "loop_double", 1); return false;}//shrink loop if no loop button down 
    else if (KONTROL1.loopbuttonDown === true && engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        if (KONTROL1.debug>2){print("##doing move both forward#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start+interval);
        engine.setValue(group, "loop_end_position", end+interval);
        return true;
        }
    else if (KONTROL1.loopinbuttonDown === true && engine.getValue(group, "loop_start_position")>=0){
        //move loop in point    
        if (KONTROL1.debug>2){print("##doing move inpoint forward#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        engine.setValue(group, "loop_start_position", start+interval);
        return true;        }
    else if (KONTROL1.loopoutbuttonDown === true && engine.getValue(group, "loop_end_position")>=0){
        //move loop out point    
        if (KONTROL1.debug>2){print("##doing move outpoint forward#####")};
        var interval =    KONTROL1.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_end_position", end+interval);
        return true;
        }//end if
}

//sync and resume

KONTROL1.resumeSet = function resumeSet(deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //records the time the set button is pressed, and the current play position
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);

    if (engine.getValue(group, "playposition")>=0 && engine.getValue(group, "playposition")<=1){
        KONTROL1.channels[KONTROL1.curChannel]["resumepos"] = engine.getValue(group, "playposition");
        KONTROL1.channels[KONTROL1.curChannel]["resumetime"] = Date.now();//get time in milliseconds
        };
    KONTROL1.resumeLED();
    };

KONTROL1.resumeClear = function resumeClear(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    KONTROL1.channels[KONTROL1.curChannel]["resumepos"]=-1;
    KONTROL1.resumeLED();
    };

KONTROL1.resumeUnset = function resumeUnset(value,group,control) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //when a new track is loaded, turn off resume settings
    var ch;
    for (var k in KONTROL1.defaultDeck){
        if (KONTROL1.defaultDeck[k]==group){ch=k;}
        }
        
    if (KONTROL1.debug){print("##ch: "+ch)};
    KONTROL1.channels[ch]["resumepos"]=-1;
    KONTROL1.resumeLED();
    };


//tie in hooks
engine.connectControl("[Channel1]","LoadSelectedTrack","KONTROL1.resumeUnset");
engine.connectControl("[Channel2]","LoadSelectedTrack","KONTROL1.resumeUnset");
engine.connectControl("[Channel3]","LoadSelectedTrack","KONTROL1.resumeUnset");
engine.connectControl("[Channel4]","LoadSelectedTrack","KONTROL1.resumeUnset");

KONTROL1.resume = function resume(deck) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    //records the time the set button is pressed, and the current play position
    deck=(typeof deck !== 'undefined')?deck:"default";
    var group=KONTROL1.getGroup(deck);

    if (KONTROL1.channels[KONTROL1.curChannel]["resumepos"]>-1){
        var oldpos = KONTROL1.channels[KONTROL1.curChannel]["resumepos"]*engine.getValue(group, "track_samples");
        var seconds = (Date.now()- KONTROL1.channels[KONTROL1.curChannel]["resumetime"])/1000;//get elapsed time in seconds
        var samples = seconds*engine.getValue(group, "track_samplerate")*2;//how many samples to jump forward - multiplied by 2 for stereo samples
        var newpos = oldpos + samples;//new position in samples
        engine.setValue(group, "playposition", newpos/engine.getValue(group, "track_samples"));
        }else{KONTROL1.resumeSet();}
    KONTROL1.resumeLED();
    };

//scratch parameters
KONTROL1.wheelResArray=new Array(50,75,100,125,150,175,200,225,250,275,300,325,350);
KONTROL1.scratchRevArray=new Array(16,20,24,29,33,37,40,45,50,55,60,70,78);
KONTROL1.scratchAlphaArray=new Array(1/128,1/64,1/32,1/16,1/8,1/4,1/2,1);
KONTROL1.scratchBetaArray=new Array(128,64,32,16,8,4,2,1);

KONTROL1.wheelResIndex=4;
KONTROL1.scratchRevIndex=7;
KONTROL1.scratchAlphaIndex=3;
KONTROL1.scratchBetaIndex=2;

KONTROL1.wheelResDefaultIndex=4;
KONTROL1.scratchRevDefaultIndex=7;
KONTROL1.scratchAlphaDefaultIndex=3;
KONTROL1.scratchBetaDefaultIndex=2;







KONTROL1.setScratchRev = function setScratchRev(value) {//set scratch parameter - record RPM
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    var inc=(value==127)?1:-1;//increment - how far to move on each knob click
    KONTROL1.scratchRevIndex+=inc;
    KONTROL1.scratchRevIndex=(KONTROL1.scratchRevIndex<0)?0:KONTROL1.scratchRevIndex;
    KONTROL1.scratchRevIndex=(KONTROL1.scratchRevIndex>KONTROL1.scratchRevArray.length-1)?KONTROL1.scratchRevArray.length-1:KONTROL1.scratchRevIndex;

    KONTROL1.scratchParameterLEDs(0x37,KONTROL1.scratchRevArray,KONTROL1.scratchRevIndex);
    return true;
    }

KONTROL1.setScratchWheelRes = function setScratchWheelRes(value) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    var inc=(value==127)?1:-1;//increment - how far to move on each knob click
    KONTROL1.wheelResIndex+=inc;
    KONTROL1.wheelResIndex=(KONTROL1.wheelResIndex<0)?0:KONTROL1.wheelResIndex;
    KONTROL1.wheelResIndex=(KONTROL1.wheelResIndex>KONTROL1.wheelResArray.length-1)?KONTROL1.wheelResArray.length-1:KONTROL1.wheelResIndex;

    KONTROL1.scratchParameterLEDs(0x36,KONTROL1.wheelResArray,KONTROL1.wheelResIndex);
    return true;
    }

KONTROL1.setScratchAlpha = function setScratchAlpha(value) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    var inc=(value==127)?1:-1;//increment - how far to move on each knob click
    KONTROL1.scratchAlphaIndex+=inc;
    KONTROL1.scratchAlphaIndex=(KONTROL1.scratchAlphaIndex<0)?0:KONTROL1.scratchAlphaIndex;
    KONTROL1.scratchAlphaIndex=(KONTROL1.scratchAlphaIndex>KONTROL1.scratchAlphaArray.length-1)?KONTROL1.scratchAlphaArray.length-1:KONTROL1.scratchAlphaIndex;

    KONTROL1.scratchParameterLEDs(0x35,KONTROL1.scratchAlphaArray,KONTROL1.scratchAlphaIndex);
    return true;
}

KONTROL1.setScratchBeta = function setScratchBeta(value) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};
    
    var inc=(value==127)?1:-1;//increment - how far to move on each knob click
    KONTROL1.scratchBetaIndex+=inc;
    KONTROL1.scratchBetaIndex=(KONTROL1.scratchBetaIndex<0)?0:KONTROL1.scratchBetaIndex;
    KONTROL1.scratchBetaIndex=(KONTROL1.scratchBetaIndex>KONTROL1.scratchBetaArray.length-1)?KONTROL1.scratchBetaArray.length-1:KONTROL1.scratchBetaIndex;

    KONTROL1.scratchParameterLEDs(0x34,KONTROL1.scratchBetaArray,KONTROL1.scratchBetaIndex);
    return true;
}

//############################################################################
//BASIC FUNCTIONS - stuff that makes the other stuff work
//############################################################################

KONTROL1.getControl=function getControl(control){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    //returns the array of control commands and LED state functions for the control pressed for the current bank/mods
    var bank=KONTROL1.getBankCode();//check for setting on current bank with mods first
    var nomodbank=KONTROL1.getNoModBankCode();//then current bank without mods
    var noshiftbank=KONTROL1.getNoShiftBankCode();//then for setting without shift button.  If nothing found, then use default bank.
    var noshiftnomodbank=KONTROL1.getNoShiftNoModBankCode();//then for setting without shift button.  If nothing found, then use default bank.
    var defaultbankwmod="bank1a-"+KONTROL1.getModCode();
    var defaultbank="bank1a-ooooooo";
    if(KONTROL1.controls[control][bank] !== undefined ){return KONTROL1.controls[control][bank];}
    if(KONTROL1.controls[control][nomodbank] !== undefined ){return KONTROL1.controls[control][nomodbank];}
    if(KONTROL1.controls[control][noshiftbank] !== undefined ){return KONTROL1.controls[control][noshiftbank];}
    if(KONTROL1.controls[control][noshiftnomodbank] !== undefined ){return KONTROL1.controls[control][noshiftnomodbank];}
    if(KONTROL1.controls[control][defaultbankwmod] !== undefined ){return KONTROL1.controls[control][defaultbankwmod];}
    return KONTROL1.controls[control][defaultbank];
    }

KONTROL1.getModCode = function getModCode() {//returns the mod code to use as part of the key to access the controls object
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr=KONTROL1.mod["p1"]["state"]+KONTROL1.mod["p2"]["state"]+KONTROL1.mod["k1"]["state"]+KONTROL1.mod["k2"]["state"]+KONTROL1.mod["k3"]["state"]+KONTROL1.mod["k4"]["state"]+KONTROL1.mod["s"]["state"];
    return outstr;
    };

KONTROL1.getBankCode = function getBankCode() {//returns the bank code (with mods) to use as a key to access the controls object
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr=KONTROL1.channels[KONTROL1.curChannel]["curBank"]+KONTROL1.channels[KONTROL1.curChannel]["curShift"]+"-"+KONTROL1.getModCode();
    return outstr;
    };

KONTROL1.getNoModBankCode = function getNoModBankCode() {//return current bank with no mods
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr=KONTROL1.channels[KONTROL1.curChannel]["curBank"]+KONTROL1.channels[KONTROL1.curChannel]["curShift"]+"-ooooooo";
    return outstr;
    };

KONTROL1.getNoShiftBankCode = function getNoShiftBankCode() {//return current bank with mods but no shift
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr=KONTROL1.channels[KONTROL1.curChannel]["curBank"]+"a-"+KONTROL1.getModCode();
    return outstr;
    };

KONTROL1.getNoShiftNoModBankCode = function getNoShiftNoModBankCode() {//return current bank with no shifts or mods
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr=KONTROL1.channels[KONTROL1.curChannel]["curBank"]+"a-ooooooo";
    return outstr;
    };

KONTROL1.getGroup = function getGroup(groupstr) {//return Mixx group
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var outstr;
    if (groupstr=="default"){outstr=KONTROL1.defaultDeck[KONTROL1.curChannel];}
    else if (groupstr=="alt"){outstr=KONTROL1.altDeck[KONTROL1.curChannel];}
    else if (groupstr=="sync"){outstr=KONTROL1.syncDeck[KONTROL1.curChannel];}
    else {outstr=groupstr;}
    if (KONTROL1.debug>2){print("##getGroup groupstr: "+groupstr)};
    if (KONTROL1.debug>2){print("##getGroup outstr: "+outstr)};
    return outstr;
    };

KONTROL1.getFunctionName = function getFunctionName(){
    var re = /function (.*?)\(/
    var s = KONTROL1.getFunctionName.caller.toString();
    var m = re.exec( s )
    return m[1];
    }


//############################################################################
//TEST FUNCTIONS
//############################################################################
KONTROL1.testval=60;




KONTROL1.testconnect=function testconnect(value,group,control){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    print ("CONNECT CONTROL TEST");
    print ("value: "+value);
    print ("group: "+group);
    print ("control: "+control);
    //var args = Array.prototype.slice.call(arguments)
    //for (var key in args){
    //    print (arguments[key]);
    //    }
    }

KONTROL1.testvalknob=function testvalknob(channel, control, value, status, group){
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    var inc=(value==127)?3:-3;
    KONTROL1.testval+=inc;
    if (KONTROL1.testval>127)KONTROL1.testval=127;
    if (KONTROL1.testval<0)KONTROL1.testval=0;
    midi.sendShortMsg(0xB0, 0x37, KONTROL1.testval);
    }


KONTROL1.testflash2 = function testflash2() {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    if (KONTROL1.flashon==0){
        midi.sendShortMsg(0xB0, 0x37, 0);
        KONTROL1.flashon=1;
        KONTROL1.flashcount++;
        if (KONTROL1.flashcount>3){KONTROL1.stopFlashTimer();}
        }else {
        midi.sendShortMsg(0xB0, 0x37, 127);
        KONTROL1.flashon=0;
        KONTROL1.flashcount++;
        if (KONTROL1.flashcount>3){KONTROL1.stopFlashTimer();}
        }
    
    };


KONTROL1.test = function test(channel, control, value, status, group) {
    if (KONTROL1.debug>2){print("##function: "+KONTROL1.getFunctionName())};

    print("channel: "+channel);
    print("control: "+control);
    print("control.toString: "+control.toString(16));
    print("value: "+value);
    print("status: "+status);
    print("group: "+group);
    print("currentbank: "+KONTROL1.channels[0]["curBank"]);
    print("currentShift: "+KONTROL1.channels[0]["curShift"]);
    print("currentBank code: "+KONTROL1.getBankCode());
    };


    //KONTROL1.ledTimer = engine.beginTimer(250, "KONTROL1.testflash()");
    //midi.sendShortMsg(status, ctrl, state);
    //engine.stopTimer(KONTROL1.ledTimer);




    
//############################################################################
//INITIALIZE CONTROLS 
//############################################################################

//initialize control object for buttons, knobs, etc.
KONTROL1.controls={};//array of controls, with commands and LED hooks

//reference array - array of control items by name
KONTROL1.getIt={"play":0x1b, "cue":0x1c, "sync":0x1d, "wheeltouch":0x1f, "wheelmove":0x20,"pslider":0x21,"pminus":0x19,"pplus":0x1e,"shift":0x1a,"k1": 0x37,"k2": 0x36,"k3": 0x35,"k4": 0x34,"kb1": 0x04,"kb2": 0x03,"kb3": 0x02,"kb4": 0x01,"b1":0x09,"b2":0x08,"b3":0x07,"b4": 0x06,"b5": 0x0e,"b6": 0x0d,"b7": 0x0c,"b8": 0x0b,"b9": 0x13,"b10": 0x12,"b11": 0x11,"b12": 0x10,"b13": 0x18,"b14": 0x17,"b15": 0x16,"b16": 0x15};//reference array - everything else by name

//initialize object
//row 1
KONTROL1.controls[0x09]={};KONTROL1.controls[0x08]={};KONTROL1.controls[0x07]={};KONTROL1.controls[0x06]={};
//row 2
KONTROL1.controls[0x0e]={};KONTROL1.controls[0x0d]={};KONTROL1.controls[0x0c]={};KONTROL1.controls[0x0b]={};
//row 3
KONTROL1.controls[0x13]={};KONTROL1.controls[0x12]={};KONTROL1.controls[0x11]={};KONTROL1.controls[0x10]={};
//row 4
KONTROL1.controls[0x18]={};KONTROL1.controls[0x17]={};KONTROL1.controls[0x16]={};KONTROL1.controls[0x15]={};
//sync, cue, play
KONTROL1.controls[0x1d]={};KONTROL1.controls[0x1c]={};KONTROL1.controls[0x1b]={};
//wheel touch, wheel move
KONTROL1.controls[0x1f]={};KONTROL1.controls[0x20]={};
//pitch slider
KONTROL1.controls[0x21]={};
//mod1 (pitch -), mod2 (pitch +), shift
KONTROL1.controls[0x19]={};KONTROL1.controls[0x1e]={};KONTROL1.controls[0x1a]={};
//knob buttons
KONTROL1.controls[0x04]={};KONTROL1.controls[0x03]={};KONTROL1.controls[0x02]={};KONTROL1.controls[0x01]={};
//knobs
KONTROL1.controls[0x37]={};KONTROL1.controls[0x36]={};KONTROL1.controls[0x35]={};KONTROL1.controls[0x34]={};


//KONTROL1.controls[midino]["bank-ooooooo"]=new Array('PressEvalStr', 'ReleaseEvalStr', 'LEDstateType', 'LEDStr1', 'LEDstr2', 'LEDstr3');
//LEDstateType = false if control has no LED, "hook" if using connectControl, "eval" if using an eval statement to get LED state
//LEDstateType = "off" if LED is always off, "on" if LED is always on
//for HOOKS: LEDstr1 = mixxx control to connect to, LEDstr2 = script function
//for HOOKS: LEDstr3 = mixxx control group - literals accepted, or "default", "sync" or "alt" = default, sync or alt deck for that channel
//for EVAL: LEDstr1 = javascript to evaluate to get LED state, LEDstr2 and 3 are not used
//mod = o=off I=on - order: mod1 mod2 knob1 knob2 knob3 knob4 shift

//BANK 1a - STANDARD CONTROLS (buttons default to this if nothing is set for other banks/mods
//row 1
KONTROL1.controls[0x09]["bank1a-ooooooo"]=new Array('KONTROL1.kill("all", value, "default");', 'KONTROL1.kill("all", value, "default");', 'eval', '(KONTROL1.channels[KONTROL1.curChannel]["isMuted"]==true)?127:0;', 'LEDstr2', 'LEDstr3');
KONTROL1.controls[0x08]["bank1a-ooooooo"]=new Array('KONTROL1.kill("high", value, "default");', 'KONTROL1.kill("high", value, "default");', 'hook', 'filterHighKill', 'KONTROL1.doFilterKillLEDs', 'default');
KONTROL1.controls[0x07]["bank1a-ooooooo"]=new Array('KONTROL1.kill("mid", value, "default");', 'KONTROL1.kill("mid", value, "default");', 'hook', 'filterMidKill', 'KONTROL1.doFilterKillLEDs', 'default');
KONTROL1.controls[0x06]["bank1a-ooooooo"]=new Array('KONTROL1.kill("low", value, "default");', 'KONTROL1.kill("low", value, "default");', 'hook', 'filterLowKill', 'KONTROL1.doFilterKillLEDs', 'default');

//row 2 - hotcues
for (i=5;i<9;i++){
    var num=i;
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-ooooooo"]=new Array('KONTROL1.hotcueButton('+num+')', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-Ioooooo"]=new Array('KONTROL1.cueClear('+num+', '+KONTROL1.getIt["b"+i]+')', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');

    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-ooIoooo"]=new Array('KONTROL1.cueLoop('+num+', .25)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-oooIooo"]=new Array('KONTROL1.cueLoop('+num+', .5)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-ooooIoo"]=new Array('KONTROL1.cueLoop('+num+', 1)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-oooooIo"]=new Array('KONTROL1.cueLoop('+num+', 2)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
}

//row 3
KONTROL1.controls[0x13]["bank1a-ooooooo"]=new Array('KONTROL1.beatLoop(1, value, "default");', 'KONTROL1.beatLoop(1, value, "default");', 'hook', "beatloop_1_enabled", 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x12]["bank1a-ooooooo"]=new Array('KONTROL1.beatLoop(2, value, "default");', 'KONTROL1.beatLoop(2, value, "default");', 'hook', "beatloop_2_enabled", 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x11]["bank1a-ooooooo"]=new Array('KONTROL1.beatLoop(4, value, "default");', 'KONTROL1.beatLoop(4, value, "default");', 'hook', "beatloop_4_enabled", 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x10]["bank1a-ooooooo"]=new Array('KONTROL1.reloopButton(value, "default");', 'KONTROL1.reloopButton(value, "default");', 'hook', "loop_enabled", 'KONTROL1.lightButtonLEDs', 'default');

//row 4
KONTROL1.controls[0x18]["bank1a-ooooooo"]=new Array('KONTROL1.loopIn(value, "default");', 'KONTROL1.loopIn(value, "default");', 'hook', 'loop_start_position', 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x17]["bank1a-ooooooo"]=new Array('KONTROL1.loopOut(value, "default");', 'KONTROL1.loopOut(value, "default");', 'hook', 'loop_end_position', 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x16]["bank1a-ooooooo"]=new Array('KONTROL1.loopMinus(value, "default");', false, 'off', 'LEDStr1', 'LEDstr2', 'LEDstr3');
KONTROL1.controls[0x15]["bank1a-ooooooo"]=new Array('KONTROL1.loopPlus(value, "default");', false, 'off', 'LEDStr1', 'LEDstr2', 'LEDstr3');


//sync, cue, play - no mod - slip mode toggle, cue default, play
//SLIP MODE CAN't BE CLEARED YET - Using homebrew slip mode - KONTROL1.controls[0x1d]["bank1a-ooooooo"]=new Array('KONTROL1.toggleBinaryControl("slip_enabled");', false, 'hook', 'slip_enabled', 'KONTROL1.syncLED', 'default');
KONTROL1.controls[0x1d]["bank1a-ooooooo"]=new Array('KONTROL1.resume("default");', false, 'eval', 'KONTROL1.resumeLED()');
KONTROL1.controls[0x1c]["bank1a-ooooooo"]=new Array('KONTROL1.toggleBinaryControl("cue_default", 1);', 'KONTROL1.toggleBinaryControl("cue_default", 0);', "off");
KONTROL1.controls[0x1b]["bank1a-ooooooo"]=new Array('KONTROL1.toggleBinaryControl("play");', false, 'hook', 'play', 'KONTROL1.playLED', 'default');

//sync, cue, play - MOD1 - slip mode reset, quantize, keylock
//SLIP MODE CAN't BE CLEARED YET - Using homebrew slip mode - KONTROL1.controls[0x1d]["bank1a-Ioooooo"]=new Array('KONTROL1.toggleBinaryControl("slip_enabled", 1);', false, 'hook', 'slip_enabled', 'KONTROL1.syncLED', 'default');
KONTROL1.controls[0x1d]["bank1a-Ioooooo"]=new Array('KONTROL1.resumeClear("default");', 'ReleaseEvalStr', 'eval', 'KONTROL1.resumeLED()');//mod P1
KONTROL1.controls[0x1c]["bank1a-Ioooooo"]=new Array('KONTROL1.toggleBinaryControl("quantize");', false, 'hook', 'quantize', 'KONTROL1.cueLED', 'default');
KONTROL1.controls[0x1b]["bank1a-Ioooooo"]=new Array('KONTROL1.toggleBinaryControl("keylock");', false, 'hook', 'play', 'KONTROL1.playLED', 'default');

//sync, cue, play - MOD2 - beatsync, beatgrid align, bpm tap
KONTROL1.controls[0x1d]["bank1a-oIooooo"]=new Array('KONTROL1.toggleBinaryControl("beatsync", 1);', 'KONTROL1.toggleBinaryControl("beatsync", 0);', 'off');
KONTROL1.controls[0x1c]["bank1a-oIooooo"]=new Array('KONTROL1.toggleBinaryControl("beats_translate_curpos", 1);', 'KONTROL1.toggleBinaryControl("beats_translate_curpos", 0);', 'off');
KONTROL1.controls[0x1b]["bank1a-oIooooo"]=new Array('KONTROL1.toggleBinaryControl("bpm_tap", 1);', 'KONTROL1.toggleBinaryControl("bpm_tap", 0);', 'off');

//sync, cue, play - knob1 - flanger, PFL, go to start of track
KONTROL1.controls[0x1d]["bank1a-ooIoooo"]=new Array('KONTROL1.toggleBinaryControl("flanger");', false, 'hook', 'flanger', 'KONTROL1.syncLED', 'default');
KONTROL1.controls[0x1c]["bank1a-ooIoooo"]=new Array('KONTROL1.toggleBinaryControl("pfl");', false, 'hook', 'pfl', 'KONTROL1.cueLED', 'default');
KONTROL1.controls[0x1b]["bank1a-ooIoooo"]=new Array('KONTROL1.toggleBinaryControl("start",1);', 'KONTROL1.toggleBinaryControl("start",0);', 'off');

//sync, cue, play - knob2 - spinbacks

//sync, cue, play - knob3 - rewind, ff, reverse play
KONTROL1.controls[0x1d]["bank1a-ooooIoo"]=new Array('KONTROL1.toggleBinaryControl("back", 1);', 'KONTROL1.toggleBinaryControl("back", 0);', 'hook', 'back', 'KONTROL1.syncLED', 'default');
KONTROL1.controls[0x1c]["bank1a-ooooIoo"]=new Array('KONTROL1.toggleBinaryControl("fwd", 1);', 'KONTROL1.toggleBinaryControl("fwd", 0);', 'hook', 'fwd', 'KONTROL1.cueLED', 'default');
KONTROL1.controls[0x1b]["bank1a-ooooIoo"]=new Array('KONTROL1.toggleBinaryControl("reverse", 1);', 'KONTROL1.toggleBinaryControl("reverse", 0);', 'hook', 'reverse', 'KONTROL1.playLED', 'default');

//sync, cue, play - knob4 - loop halve, loop double, reloop_exit
KONTROL1.controls[0x1d]["bank1a-oooooIo"]=new Array('KONTROL1.toggleBinaryControl("loop_halve", 1);', 'KONTROL1.toggleBinaryControl("loop_halve", 0);', 'off');
KONTROL1.controls[0x1c]["bank1a-oooooIo"]=new Array('KONTROL1.toggleBinaryControl("loop_double", 1);', 'KONTROL1.toggleBinaryControl("loop_double", 0);', 'off');
KONTROL1.controls[0x1b]["bank1a-oooooIo"]=new Array('KONTROL1.toggleBinaryControl("reloop_exit", 1);', 'KONTROL1.toggleBinaryControl("reloop_exit", 0);', 'off');



//shift
KONTROL1.controls[0x1a]["bank1a-ooooooo"]=new Array('KONTROL1.shiftPress()', 'KONTROL1.shiftRelease()', "eval", '(KONTROL1.channels[KONTROL1.curChannel]["curShift"]=="a")?0:127;', 'LEDstr2', 'LEDstr3');

//mod1 (pitch -)
KONTROL1.controls[0x19]["bank1a-ooooooo"]=new Array('KONTROL1.modPress("p1");', 'KONTROL1.modRelease("p1");', false);
//mod2 (pitch +)
KONTROL1.controls[0x1e]["bank1a-ooooooo"]=new Array('KONTROL1.modPress("p2");', 'KONTROL1.modRelease("p2");', false);

//MOD1 and MOD2 - w. knob1 - pitch temp
KONTROL1.controls[0x19]["bank1a-ooIoooo"]=new Array('KONTROL1.toggleBinaryControl("rate_temp_down", 1);', 'KONTROL1.toggleBinaryControl("rate_temp_down", 0);', false);
KONTROL1.controls[0x1e]["bank1a-ooIoooo"]=new Array('KONTROL1.toggleBinaryControl("rate_temp_up", 1);', 'KONTROL1.toggleBinaryControl("rate_temp_up", 0);', false);

//MOD1 and MOD2 - w. knob2 - pitch perm
KONTROL1.controls[0x19]["bank1a-oooIooo"]=new Array('KONTROL1.toggleBinaryControl("rate_perm_down", 1);', 'KONTROL1.toggleBinaryControl("rate_perm_down", 0);', false);
KONTROL1.controls[0x1e]["bank1a-oooIooo"]=new Array('KONTROL1.toggleBinaryControl("rate_perm_up", 1);', 'KONTROL1.toggleBinaryControl("rate_perm_up", 0);', false);

//MOD1 and MOD2 - w. knob3 - pitch temp small
KONTROL1.controls[0x19]["bank1a-ooooIoo"]=new Array('KONTROL1.toggleBinaryControl("rate_temp_down_small", 1);', 'KONTROL1.toggleBinaryControl("rate_temp_down_small", 0);', false);
KONTROL1.controls[0x1e]["bank1a-ooooIoo"]=new Array('KONTROL1.toggleBinaryControl("rate_temp_up_small", 1);', 'KONTROL1.toggleBinaryControl("rate_temp_up_small", 0);', false);

//MOD1 and MOD2 - w. knob4 - pitch perm small
KONTROL1.controls[0x19]["bank1a-oooooIo"]=new Array('KONTROL1.toggleBinaryControl("rate_perm_down_small", 1);', 'KONTROL1.toggleBinaryControl("rate_perm_down_small", 0);', false);
KONTROL1.controls[0x1e]["bank1a-oooooIo"]=new Array('KONTROL1.toggleBinaryControl("rate_perm_up_small", 1);', 'KONTROL1.toggleBinaryControl("rate_perm_up_small", 0);', false);

//knob buttons
KONTROL1.controls[0x04]["bank1a-ooooooo"]=new Array('KONTROL1.knobPress("k1");', 'KONTROL1.knobRelease("k1");', false);
KONTROL1.controls[0x03]["bank1a-ooooooo"]=new Array('KONTROL1.knobPress("k2");', 'KONTROL1.knobRelease("k2");', false);
KONTROL1.controls[0x02]["bank1a-ooooooo"]=new Array('KONTROL1.knobPress("k3");', 'KONTROL1.knobRelease("k3");', false);
KONTROL1.controls[0x01]["bank1a-ooooooo"]=new Array('KONTROL1.knobPress("k4");', 'KONTROL1.knobRelease("k4");', false);
//knob buttons
KONTROL1.controls[0x04]["bank1a-Ioooooo"]=new Array('KONTROL1.resetKnob("pregain");', false, false);
KONTROL1.controls[0x03]["bank1a-Ioooooo"]=new Array('KONTROL1.resetKnob("filterHigh");', false, false);
KONTROL1.controls[0x02]["bank1a-Ioooooo"]=new Array('KONTROL1.resetKnob("filterMid");', false, false);
KONTROL1.controls[0x01]["bank1a-Ioooooo"]=new Array('KONTROL1.resetKnob("filterLow");', false, false);

//knobs
KONTROL1.controls[0x37]["bank1a-ooooooo"]=new Array('KONTROL1.logKnobAdjust("default", "pregain", 0, 4, value)', false, "hook", 'pregain', 'KONTROL1.doLogKnobLEDs', 'default');
KONTROL1.controls[0x36]["bank1a-ooooooo"]=new Array('KONTROL1.logKnobAdjust("default", "filterHigh", 0, 4, value)', false, "hook", 'filterHigh', 'KONTROL1.doLogKnobLEDs', 'default');
KONTROL1.controls[0x35]["bank1a-ooooooo"]=new Array('KONTROL1.logKnobAdjust("default", "filterMid", 0, 4, value)', false, "hook", 'filterMid', 'KONTROL1.doLogKnobLEDs', 'default');
KONTROL1.controls[0x34]["bank1a-ooooooo"]=new Array('KONTROL1.logKnobAdjust("default", "filterLow", 0, 4, value)', false, "hook", 'filterLow', 'KONTROL1.doLogKnobLEDs', 'default');

//knobs w p1 p2 mods - scratch parameters
KONTROL1.controls[0x37]["bank1a-IIooooo"]=new Array('KONTROL1.setScratchRev(value)', false, "eval", 'KONTROL1.scratchParameterLEDs(0x37,KONTROL1.scratchRevArray,KONTROL1.scratchRevIndex);');
KONTROL1.controls[0x36]["bank1a-IIooooo"]=new Array('KONTROL1.setScratchWheelRes(value)', false, "eval", 'KONTROL1.scratchParameterLEDs(0x36,KONTROL1.wheelResArray,KONTROL1.wheelResIndex);');
KONTROL1.controls[0x35]["bank1a-IIooooo"]=new Array('KONTROL1.setScratchAlpha(value)', false, "eval", 'KONTROL1.scratchParameterLEDs(0x35,KONTROL1.scratchAlphaArray,KONTROL1.scratchAlphaIndex);');
KONTROL1.controls[0x34]["bank1a-IIooooo"]=new Array('KONTROL1.setScratchBeta(value)', false, "eval", 'KONTROL1.scratchParameterLEDs(0x34,KONTROL1.scratchBetaArray,KONTROL1.scratchBetaIndex);');


//loop length adjust
KONTROL1.controls[0x34]["bank1a-oooooIo"]=new Array('KONTROL1.looplenDial(value, "default");', false, "eval", 'KONTROL1.loopLenLEDs("default");');

//hotcue loops, shown when knob 4 is pressed
for (i=1;i<17;i++){
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1a-oooooIo"]=new Array('KONTROL1.cueLoop('+i+', 2);', false, 'hook', 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank1b-oooooIo"]=new Array('KONTROL1.cueLoop('+(i+16)+', 2);', false, 'hook', 'hotcue_'+(i+16)+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
}


//wheel touch
KONTROL1.controls[0x1f]["bank1a-ooooooo"]=new Array('KONTROL1.wheelTouch(channel, control, value, status, group)', 'KONTROL1.wheelTouch(channel, control, value, status, group)', false);
//wheel move
KONTROL1.controls[0x20]["bank1a-ooooooo"]=new Array('KONTROL1.wheelTurn(channel, control, value, status, group)', false, false);

//pitch slider
KONTROL1.controls[0x21]["bank1a-ooooooo"]=new Array('engine.setValue(KONTROL1.getGroup("default"), "rate", (value-64)/64);', false, false);




//BANK 2a - HOTCUES - pitch minus button clears them
for (i=1;i<17;i++){
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-ooooooo"]=new Array('KONTROL1.hotcueButton('+i+')', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-Ioooooo"]=new Array('KONTROL1.cueClear('+i+', '+KONTROL1.getIt["b"+i]+')', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');

    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-ooIoooo"]=new Array('KONTROL1.cueLoop('+i+', .25)', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-oooIooo"]=new Array('KONTROL1.cueLoop('+i+', .5)', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-ooooIoo"]=new Array('KONTROL1.cueLoop('+i+', 1)', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2a-oooooIo"]=new Array('KONTROL1.cueLoop('+i+', 2)', false, "hook", 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
}

//BANK 2b - HOTCUES - pitch minus button clears them.  knob buttons activate hotcue loops
for (i=1;i<17;i++){
    var num=i+16;
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-ooooooo"]=new Array('KONTROL1.hotcueButton('+num+')', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-Ioooooo"]=new Array('KONTROL1.cueClear('+num+', '+KONTROL1.getIt["b"+i]+')', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');

    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-ooIoooo"]=new Array('KONTROL1.cueLoop('+num+', .25)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-oooIooo"]=new Array('KONTROL1.cueLoop('+num+', .5)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-ooooIoo"]=new Array('KONTROL1.cueLoop('+num+', 1)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank2b-oooooIo"]=new Array('KONTROL1.cueLoop('+num+', 2)', false, "hook", 'hotcue_'+num+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
}

//BANK 3a - Loops, beatjumps, beatlooprolls
for (i=1;i<12;i++){
    len=KONTROL1.beatloopLengths[i];
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-ooooooo"]=new Array('KONTROL1.beatLoop('+len+', value, "default");', 'KONTROL1.beatLoop('+len+', value, "default");', 'hook', "beatloop_"+len+"_enabled", 'KONTROL1.LoopBankLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-oIooooo"]=new Array('KONTROL1.beatLoopRoll('+len+', value, "default");', 'KONTROL1.beatLoopRoll('+len+', value, "default");', 'hook', "beatloop_"+len+"_enabled", 'KONTROL1.LoopBankLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-Ioooooo"]=new Array('KONTROL1.beatjump('+len+');', false, 'hook', false);
}

//reloop
KONTROL1.controls[0x10]["bank3a-ooooooo"]=new Array('KONTROL1.reloopButton(value, "default");', 'KONTROL1.reloopButton(value, "default");', 'hook', "loop_enabled", 'KONTROL1.LoopBankLEDs', 'default');

//row 4
KONTROL1.controls[0x18]["bank3a-ooooooo"]=new Array('KONTROL1.loopIn(value, "default");', 'KONTROL1.loopIn(value, "default");', 'hook', 'loop_start_position', 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x17]["bank3a-ooooooo"]=new Array('KONTROL1.loopOut(value, "default");', 'KONTROL1.loopOut(value, "default");', 'hook', 'loop_end_position', 'KONTROL1.lightButtonLEDs', 'default');
KONTROL1.controls[0x16]["bank3a-ooooooo"]=new Array('KONTROL1.loopMinus(value, "default");', false, 'LEDstateType', 'LEDStr1', 'LEDstr2', 'LEDstr3');
KONTROL1.controls[0x15]["bank3a-ooooooo"]=new Array('KONTROL1.loopPlus(value, "default");', false, 'LEDstateType', 'LEDStr1', 'LEDstr2', 'LEDstr3');

//hotcue loops, save loop in point
for (i=1;i<17;i++){
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-ooIoooo"]=new Array('KONTROL1.saveLoop('+i+');', false, 'hook', 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-oooIooo"]=new Array('KONTROL1.saveLoop('+(i+16)+');', false, 'hook', 'hotcue_'+(i+16)+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-ooooIoo"]=new Array('KONTROL1.cueLoop('+i+', 1)', false, 'hook', 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3a-oooooIo"]=new Array('KONTROL1.cueLoop('+i+', 2);', false, 'hook', 'hotcue_'+i+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3b-ooooIoo"]=new Array('KONTROL1.cueLoop('+(i+16)+', 1)', false, 'hook', 'hotcue_'+(i+16)+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
    KONTROL1.controls[KONTROL1.getIt["b"+i]]["bank3b-oooooIo"]=new Array('KONTROL1.cueLoop('+(i+16)+', 2);', false, 'hook', 'hotcue_'+(i+16)+'_enabled', 'KONTROL1.doHotcueLEDs', 'default');
}




//########################################
// LED LOOKUP ARRAYS
//########################################

//simple list of all the available leds
KONTROL1.allLEDs=new Array(0x09,0x08,0x07,0x06,0x0e,0x0d,0x0c,0x0b,0x13,0x12,0x11,0x10,0x18,0x17,0x16,0x15,0x1d,0x1c,0x1b,0x1a,0x37,0x36,0x35,0x34);
KONTROL1.knobLEDs=new Array(0x37, 0x36, 0x35, 0x34);//lists only the knob leds

//hotcue array, allows finding LED address by hotcue number 1-32
KONTROL1.hotcueLEDs=new Array(0,0x09,0x08,0x07,0x06,0x0e,0x0d,0x0c,0x0b,0x13,0x12,0x11,0x10,0x18,0x17,0x16,0x15, 0x09,0x08,0x07,0x06,0x0e,0x0d,0x0c,0x0b,0x13,0x12,0x11,0x10,0x18,0x17,0x16,0x15);

//button LED array - allows finding which button to light up from the control name (for use with hooks) (lightButtonLeds uses this)
KONTROL1.buttonLEDs={loop_enabled:0x10,beatloop_1_enabled:0x13,beatloop_2_enabled:0x12,beatloop_4_enabled:0x11,loop_start_position:0x18,loop_end_position:0x17,play:0x1b};

//array for loop bank
KONTROL1.loopbuttonLEDs={loop_enabled:0x10,loop_start_position:0x18,loop_end_position:0x17,play:0x1b};
for (i=1;i<12;i++){
    len=KONTROL1.beatloopLengths[i];
    KONTROL1.loopbuttonLEDs["beatloop_"+len+"_enabled"]=KONTROL1.getIt["b"+i];
}

