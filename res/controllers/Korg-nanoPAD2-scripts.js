function NANOPAD2RK() {}
NANOPAD2RK.debug=true;

//set defaults - don't change these
NANOPAD2RK.hotcueClear=false;//if true, hitting hotcue button clears it.
NANOPAD2RK.loopbuttonDown=false;//if true, special loop functions with knob modifiers are enabled.
NANOPAD2RK.loopmove=0.0125;//default loop move distance in seconds
NANOPAD2RK.doreloop=true;//don't change this
NANOPAD2RK.vol=new Array();//array of volumes to return to after mute - don't change
NANOPAD2RK.looptype="beatloop";//default looptype

//map hotcue pads
NANOPAD2RK.hotcues = new Array();

//initialize
//scene 1
//---------(group, cuenum)
NANOPAD2RK.hotcues[0x25] = new Array("[Channel1]", 1);
NANOPAD2RK.hotcues[0x27] = new Array("[Channel1]", 2);
NANOPAD2RK.hotcues[0x29] = new Array("[Channel1]", 3);
NANOPAD2RK.hotcues[0x2b] = new Array("[Channel1]", 4);
NANOPAD2RK.hotcues[0x2d] = new Array("[Channel1]", 5);
NANOPAD2RK.hotcues[0x2f] = new Array("[Channel1]", 6);
NANOPAD2RK.hotcues[0x31] = new Array("[Channel1]", 7);
NANOPAD2RK.hotcues[0x33] = new Array("[Channel1]", 8);

NANOPAD2RK.hotcues[0x24] = new Array("[Channel2]", 1);
NANOPAD2RK.hotcues[0x26] = new Array("[Channel2]", 2);
NANOPAD2RK.hotcues[0x28] = new Array("[Channel2]", 3);
NANOPAD2RK.hotcues[0x2a] = new Array("[Channel2]", 4);
NANOPAD2RK.hotcues[0x2c] = new Array("[Channel2]", 5);
NANOPAD2RK.hotcues[0x2e] = new Array("[Channel2]", 6);
NANOPAD2RK.hotcues[0x30] = new Array("[Channel2]", 7);

//scene 2
//---------(group, cuenum)
NANOPAD2RK.hotcues[0x35] = new Array("[Channel1]", 9);
NANOPAD2RK.hotcues[0x37] = new Array("[Channel1]", 10);
NANOPAD2RK.hotcues[0x39] = new Array("[Channel1]", 11);
NANOPAD2RK.hotcues[0x3b] = new Array("[Channel1]", 12);
NANOPAD2RK.hotcues[0x3d] = new Array("[Channel1]", 13);
NANOPAD2RK.hotcues[0x3f] = new Array("[Channel1]", 14);
NANOPAD2RK.hotcues[0x41] = new Array("[Channel1]", 15);
NANOPAD2RK.hotcues[0x43] = new Array("[Channel1]", 16);

NANOPAD2RK.hotcues[0x34] = new Array("[Channel2]", 9);
NANOPAD2RK.hotcues[0x36] = new Array("[Channel2]", 10);
NANOPAD2RK.hotcues[0x38] = new Array("[Channel2]", 11);
NANOPAD2RK.hotcues[0x3a] = new Array("[Channel2]", 12);
NANOPAD2RK.hotcues[0x3c] = new Array("[Channel2]", 13);
NANOPAD2RK.hotcues[0x3e] = new Array("[Channel2]", 14);
NANOPAD2RK.hotcues[0x40] = new Array("[Channel2]", 15);





//map loop pads
NANOPAD2RK.loops = new Array();
//scene 3
//---------(group, beatloop_len)
NANOPAD2RK.loops[0x47] = new Array("[Channel1]", 1);
NANOPAD2RK.loops[0x49] = new Array("[Channel1]", 2);
NANOPAD2RK.loops[0x4b] = new Array("[Channel1]", 4);

NANOPAD2RK.loops[0x46] = new Array("[Channel2]", 1);
NANOPAD2RK.loops[0x48] = new Array("[Channel2]", 2);
NANOPAD2RK.loops[0x4a] = new Array("[Channel2]", 4);





NANOPAD2RK.init = function (id, debug) { // called when the device is opened & set up
    if (NANOPAD2RK.debug){print("###init##############")};
};

NANOPAD2RK.shutdown = function () {
    
};
    
    
NANOPAD2RK.clear = function (){//enables hotcue clearing
    if (NANOPAD2RK.debug){print("###hotcueclear##############")};
    NANOPAD2RK.hotcueClear=true;
    };

NANOPAD2RK.noclear = function (){//disables hotcue clearing
    NANOPAD2RK.hotcueClear=false;
    };

NANOPAD2RK.hotcueButton = function (channel, control, value, status, group) {

    if (NANOPAD2RK.debug){print("###hotcueButton##############")};
    if (NANOPAD2RK.debug){print("status:"+status)};
    if (NANOPAD2RK.debug){print("channel:"+channel)};
    if (NANOPAD2RK.debug){print("control:"+control)};
    if (NANOPAD2RK.debug){print("hotcuebank:"+NANOPAD2RK.hotcueBank)};
    if (NANOPAD2RK.debug){print("phrase:#"+status.toString(16).toLowerCase()+control.toString(16).toLowerCase()+"#")};
    
    //activate or clear depending on whether clear button is pressed
    var thecue = NANOPAD2RK.hotcues[control];
    if (NANOPAD2RK.hotcueClear){
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_clear", 1);
        if (NANOPAD2RK.debug){print("cleared")};
        } else {
        engine.setValue(thecue[0], "hotcue_"+thecue[1]+"_activate", 1);
        if (NANOPAD2RK.debug){print("###"+control+"--activated")};
        };

    };

NANOPAD2RK.loopButton = function (channel, control, value, status, group) {
    if (NANOPAD2RK.debug){print(NANOPAD2RK.looplen+"len");}
    if (NANOPAD2RK.debug){print(NANOPAD2RK.looptype+"type");}
    //activate beatloop
    var theloop = NANOPAD2RK.loops[control];
    NANOPAD2RK.loopbuttonDown=true;
    print("engine.setValue("+group+", "+NANOPAD2RK.looptype+"_"+theloop[1]+"_activate, 1)");
    engine.setValue(group, NANOPAD2RK.looptype+"_"+theloop[1]+"_activate", 1);
    };

NANOPAD2RK.loopButtonRelease = function (channel, control, value, status, group) {//button was released
    NANOPAD2RK.loopbuttonDown=false;
    };

NANOPAD2RK.reloopButton = function (channel, control, value, status, group) {
    if (NANOPAD2RK.debug){print(NANOPAD2RK.looplen+"len");}
    if (NANOPAD2RK.debug){print(NANOPAD2RK.looptype+"type");}
    
    NANOPAD2RK.loopbuttonDown=true;
    NANOPAD2RK.doreloop=true;
    NANOPAD2RK.reloopTimer = engine.beginTimer(500, "NANOPAD2RK.disablereloop()", true);
    };

NANOPAD2RK.reloopButtonRelease = function (channel, control, value, status, group) {//button was released
    if (NANOPAD2RK.debug){print("reloop release");}
    NANOPAD2RK.loopbuttonDown=false;
    if (NANOPAD2RK.doreloop===true) {engine.setValue(group, "reloop_exit", 1);};
    NANOPAD2RK.doreloop=true;
    };

NANOPAD2RK.disablereloop = function () {
    //timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
    NANOPAD2RK.doreloop=false;
    };


NANOPAD2RK.loopplus = function (channel, control, value, status, group) {
    //grows loop or moves loop forward
    if (NANOPAD2RK.loopbuttonDown !== true){engine.setValue(group, "loop_double", 1); return false;}//shrink loop if no loop button down 
    else if (engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        var interval =    NANOPAD2RK.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start+interval);
        engine.setValue(group, "loop_end_position", end+interval);
        return true;
        };
    };
NANOPAD2RK.loopminus = function (channel, control, value, status, group) {
    //shrinks loop or moves loop back
    if (NANOPAD2RK.loopbuttonDown !== true){engine.setValue(group, "loop_halve", 1); return false;}//shrink loop if no loop button down 
    else if (engine.getValue(group, "loop_start_position")>=0 && engine.getValue(group, "loop_end_position")>=0 ){
        //move loop
        var interval =    NANOPAD2RK.loopmove*engine.getValue(group, "track_samples")/engine.getValue(group, "duration");
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        engine.setValue(group, "loop_start_position", start-interval);
        engine.setValue(group, "loop_end_position", end-interval);
        return true;
        };
    };


NANOPAD2RK.muteOn = function (channel, control, value, status, group) {
    //toggles mute, then returns to previous volume
    storedvol=NANOPAD2RK.vol[group];
    curvol=engine.getValue(group, "volume");
    engine.softTakeover(group,"volume",false);
    engine.setValue(group, "volume", 0);
    NANOPAD2RK.vol[group]=curvol;
    engine.softTakeover(group,"volume",true);
    
    if (NANOPAD2RK.debug){print("MUTE");}
    };

NANOPAD2RK.muteOff = function (channel, control, value, status, group) {
    //toggles mute, then returns to previous volume
    storedvol=NANOPAD2RK.vol[group];
    curvol=engine.getValue(group, "volume");
    engine.softTakeover(group,"volume",false);
    engine.setValue(group, "volume", storedvol);
    engine.softTakeover(group,"volume",true);
    
    if (NANOPAD2RK.debug){print("MUTE");}
    };



