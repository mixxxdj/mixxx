function MIXER1() {}

MIXER1.init = function init(id, debug) { // called when the device is opened & set up
    //connect VUmeters
    engine.connectControl("[Master]", "VuMeterL", "MIXER1.volumeLEDs");
    engine.connectControl("[Master]", "VuMeterR", "MIXER1.volumeLEDs");
    engine.connectControl("[Channel1]", "VuMeter", "MIXER1.volumeLEDs");
    engine.connectControl("[Channel2]", "VuMeter", "MIXER1.volumeLEDs");
    engine.connectControl("[Channel3]", "VuMeter", "MIXER1.volumeLEDs");
    engine.connectControl("[Channel4]", "VuMeter", "MIXER1.volumeLEDs");
    };

MIXER1.shutdown = function shutdown() {
    //disconnect VUmeters
    engine.connectControl("[Master]", "VuMeterL", "MIXER1.volumeLEDs", true);
    engine.connectControl("[Master]", "VuMeterR", "MIXER1.volumeLEDs", true);
    engine.connectControl("[Channel1]", "VuMeter", "MIXER1.volumeLEDs", true);
    engine.connectControl("[Channel2]", "VuMeter", "MIXER1.volumeLEDs", true);
    engine.connectControl("[Channel3]", "VuMeter", "MIXER1.volumeLEDs", true);
    engine.connectControl("[Channel4]", "VuMeter", "MIXER1.volumeLEDs", true);
    };

MIXER1.volumeLEDs = function volumeLEDs(value, group, control){
        
    value=(value*127);
    if (group=="[Master]" && control=="VuMeterL"){ch=0xB5;midino=0x1f;midi.sendShortMsg(ch, midino, value);ch=0xB7;midino=0x1f;midi.sendShortMsg(ch, midino, value);}
    if (group=="[Master]" && control=="VuMeterR"){ch=0xB5;midino=0x20;midi.sendShortMsg(ch, midino, value);ch=0xB7;midino=0x20;midi.sendShortMsg(ch, midino, value);}
    if (group=="[Channel1]" && control=="VuMeter"){ch=0xB5;midino=0x22;midi.sendShortMsg(ch, midino, value);}
    if (group=="[Channel2]" && control=="VuMeter"){ch=0xB6;midino=0x23;midi.sendShortMsg(ch, midino, value);}
    if (group=="[Channel3]" && control=="VuMeter"){ch=0xB7;midino=0x22;midi.sendShortMsg(ch, midino, value);}
    if (group=="[Channel4]" && control=="VuMeter"){ch=0xB8;midino=0x23;midi.sendShortMsg(ch, midino, value);}
    };

MIXER1.clearVolumeLEDs = function clearVolumeLEDs(){
    //send zeros to all volumeLED channels
    
    value=0;
    ch=0xB5;midino=0x1f;midi.sendShortMsg(ch, midino, value);
    ch=0xB7;midino=0x1f;midi.sendShortMsg(ch, midino, value);
    ch=0xB5;midino=0x20;midi.sendShortMsg(ch, midino, value);
    ch=0xB7;midino=0x20;midi.sendShortMsg(ch, midino, value);
    ch=0xB5;midino=0x22;midi.sendShortMsg(ch, midino, value);
    ch=0xB6;midino=0x23;midi.sendShortMsg(ch, midino, value);
    ch=0xB7;midino=0x22;midi.sendShortMsg(ch, midino, value);
    ch=0xB8;midino=0x23;midi.sendShortMsg(ch, midino, value);
    };

MIXER1.mute = function mute(channel, control, value){
    //mute master
    if (value==127){
        //button was pressed
        if (engine.getValue("[Master]", "volume")>0){MIXER1.mutestoredvol=engine.getValue("[Master]", "volume");}
        engine.setValue("[Master]", "volume", 0);
        midi.sendShortMsg(0x97, 0x05, 127);//light Inverse LED
        midi.sendShortMsg(0x95, 0x05, 127);//light Inverse LED
        }else{
        //button was released
        engine.setValue("[Master]", "volume", MIXER1.mutestoredvol);        
        midi.sendShortMsg(0x97, 0x05, 0);//turn off Inverse LED
        midi.sendShortMsg(0x95, 0x05, 0);//turn off Inverse LED
        }
    };

MIXER1.xfaderCurve = function xfaderCurve(channel, control, value){
    //set xfader curve
    script.crossfaderCurve(value, 0, 127);
    print("########XFADE###########"+value);
    };
