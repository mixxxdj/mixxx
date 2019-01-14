DJTechiMixReload = new function(){
    this.buttons = { 
	"deck1play" :0x11,
	"deck1cue" : 0x0b,
	"deck1sync": 0x05,
	"deck1hotcue1": 0x08,
	"deck1hotcue2": 0x0e,
	"deck1hotcue3": 0x14,
	"deck1set":0x02,
	"deck1loop":0x01,
	"deck1loopsize-": 0x07,
	"deck1loopsize+": 0x0d,
	"deck1relooper":0x13,
	"deck1fx1": 0x04,
	"deck1fx2": 0x0a,
	"deck1keylock": 0x03,
	"deck1downbeat": 0x17,
	"deck2play" :0x2f,
	"deck2cue" : 0x29,
	"deck2sync": 0x23,
	"deck2hotcue1": 0x20,
	"deck2hotcue2": 0x26,
	"deck2hotcue3": 0x2c,
	"deck2set":0x1a,
	"deck2loop":0x19,
	"deck2loopsize-": 0x1f,
	"deck2loopsize+": 0x25,
	"deck2relooper":0x2b,
	"deck2fx1": 0x10,
	"deck2fx2": 0x16,
	"deck2keylock": 0x2d,
	"deck2downbeat": 0x1d,
	"mixerch1pfl" :0x15,
	"mixerch1highkill" : 0x1c,
	"mixerch1midkill": 0x18,
	"mixerch1lowkill": 0x12,
	"mixerch2pfl" :0x1b,
	"mixerch2highkill" : 0x22,
	"mixerch2midkill": 0x1e,
	"mixerch2lowkill": 0x24
    };
    this.scratching = {"deck1":false, "deck2":false};
    this.alpha = 0.125;
    this.beta = this.alpha/32;
};


DJTechiMixReload.ledfunctions = function (ledname)
{
    return function(value){
	var eval = ledname;
	if(value != 0)
	    midi.sendShortMsg(0x90,DJTechiMixReload.buttons[ledname],0x7f);
	else
	    midi.sendShortMsg(0x90,DJTechiMixReload.buttons[ledname],0x00);
    };
};
DJTechiMixReload.allleadson = function (){
    for(var i in DJTechiMixReload.buttons){
	DJTechiMixReload.ledfunctions(i)(1,false);
    }
};
DJTechiMixReload.allleadsoff = function (){
    for(var i in DJTechiMixReload.buttons){
	DJTechiMixReload.ledfunctions(i)(0,false);
    }
};
DJTechiMixReload.init = function(ID) 
{
    for(var key1 in DJTechiMixReload.buttons){
	print("Button " + key1 + " -> MIDI " + DJTechiMixReload.buttons[key1]);
	for(var key2 in DJTechiMixReload.buttons){
	    if (key1 != key2){
		if (DJTechiMixReload.buttons[key1] == DJTechiMixReload.buttons[key2]){
		    print("Warning: " + key1 + " and " + key2 + "have same midi note!");
		}
	    }
	}
    }

    
    DJTechiMixReload.setbutton = new Object();
    DJTechiMixReload.setbutton["[Channel1]"] = false;
    DJTechiMixReload.setbutton["[Channel2]"] = false;

    DJTechiMixReload.allleadson();
    engine.beginTimer(4000,"DJTechiMixReload.allleadsoff()",true);
    for(var deck = 1; deck <= 2; deck++){
	engine.connectControl("[Channel" + deck + "]","play","DJTechiMixReload.deck" + deck + "play");	
	engine.connectControl("[Channel" + deck + "]","cue_default","DJTechiMixReload.deck" + deck + "cue");	
	engine.connectControl("[Channel" + deck + "]","loop_enabled","DJTechiMixReload.deck" + deck + "loop");	
	for(var hotcue = 1; hotcue <= 3; hotcue++){
	    engine.connectControl("[Channel" + deck + "]","hotcue_"+ hotcue + "_enabled","DJTechiMixReload.deck" + deck + "hotcue" + hotcue);	
	}
	engine.connectControl("[Channel" + deck + "]","flanger","DJTechiMixReload.deck" + deck + "fx1");	
	engine.connectControl("[Channel" + deck + "]","filterHighKill","DJTechiMixReload.mixerch" + deck + "highkill");	
	engine.connectControl("[Channel" + deck + "]","filterMidKill","DJTechiMixReload.mixerch" + deck + "midkill");	
	engine.connectControl("[Channel" + deck + "]","filterLowKill","DJTechiMixReload.mixerch" + deck + "lowkill");	
	engine.connectControl("[Channel" + deck + "]","pfl","DJTechiMixReload.mixerch" + deck + "pfl");	
    }
};
DJTechiMixReload.shutdown = function(ID) 
{
    DJTechiMixReload.allleadsoff();
};

DJTechiMixReload.setfunction = function(channel, control, value, status,group)
{
    if(value == 0x7f){
	DJTechiMixReload.setbutton[group] = true;
	print("set button pressed");
    }
    else
	DJTechiMixReload.setbutton[group] = false;

};
DJTechiMixReload.hotcuefunction = function(channel, control, value, status,group)
{
    if(value == 0x00){
	return;
    }
    print("one hotcue button pressed");
    var hotcue = null;
    switch(control){
    case DJTechiMixReload.buttons["deck1hotcue1"]:
    case DJTechiMixReload.buttons["deck2hotcue1"]:
	hotcue = "hotcue_1";
	break;
    case DJTechiMixReload.buttons["deck1hotcue2"]:
    case DJTechiMixReload.buttons["deck2hotcue2"]:
	hotcue = "hotcue_2";
	break;
    case DJTechiMixReload.buttons["deck1hotcue3"]:
    case DJTechiMixReload.buttons["deck2hotcue3"]:
	hotcue = "hotcue_3";
	break;
    default:
	print("Something wrong: value = " + value);
    }

    if(DJTechiMixReload.setbutton[group]){
	engine.setValue(group,hotcue+"_clear",1);
	midi.sendShortMsg(0x90,control,0x7f);
    }
    else{
	engine.setValue(group,hotcue+"_activate",1);
	midi.sendShortMsg(0x90,control,0x00);
    }
};

DJTechiMixReload.wheeltouch = function(channel, control, value, status,group)
{
    print("value: " + value + " control: " + control);
    print(group + group[8]);
    if(value == 0x00)
	engine.scratchDisable(group[8]);
    else
	engine.scratchEnable(group[8],148,33+1/3.0,DJTechiMixReload.alpha,DJTechiMixReload.beta);
};

DJTechiMixReload.scratch = function(channel, control, value, status,group)
{
    print("value: " + value + " control: " + control);
    switch(control){
    case 0x60:
    case 0x62:
	engine.scratchTick(group[8],1);
	break;
    case 0x61:
    case 0x63:
	engine.scratchTick(group[8],-1);
	break;
    }
    
};

DJTechiMixReload.deck1play = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1play"],value?0x7f:0x00);
};
DJTechiMixReload.deck1cue = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1cue"],value?0x7f:0x00);
};
DJTechiMixReload.deck1sync = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1sync"],value?0x7f:0x00);
};
DJTechiMixReload.deck1hotcue1 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1hotcue1"],value?0x7f:0x00);
};
DJTechiMixReload.deck1hotcue2 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1hotcue2"],value?0x7f:0x00);
};
DJTechiMixReload.deck1hotcue3 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1hotcue3"],value?0x7f:0x00);
};
DJTechiMixReload.deck1loop = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1loop"],value?0x7f:0x00);
};
DJTechiMixReload.deck1fx1 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1fx1"],value?0x7f:0x00);
};
DJTechiMixReload.deck1downbeat = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck1downbeat"],value?0x7f:0x00);
};
DJTechiMixReload.deck2play = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2play"],value?0x7f:0x00);
};
DJTechiMixReload.deck2cue = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2cue"],value?0x7f:0x00);
};
DJTechiMixReload.deck2sync = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2sync"],value?0x7f:0x00);
};
DJTechiMixReload.deck2hotcue1 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2hotcue1"],value?0x7f:0x00);
};
DJTechiMixReload.deck2hotcue2 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2hotcue2"],value?0x7f:0x00);
};
DJTechiMixReload.deck2hotcue3 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2hotcue3"],value?0x7f:0x00);
};
DJTechiMixReload.deck2loop = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2loop"],value?0x7f:0x00);
};
DJTechiMixReload.deck2fx1 = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2fx1"],value?0x7f:0x00);
};
DJTechiMixReload.deck2downbeat = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["deck2downbeat"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch1pfl = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixer1ch1pfl"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch1highkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch1highkill"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch1midkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch1midkill"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch1lowkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch1lowkill"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch2pfl = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch2pfl"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch2highkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch2highkill"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch2midkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch2midkill"],value?0x7f:0x00);
};
DJTechiMixReload.mixerch2lowkill = function(value) 
{
    midi.sendShortMsg(0x90,DJTechiMixReload.buttons["mixerch2lowkill"],value?0x7f:0x00);
};
