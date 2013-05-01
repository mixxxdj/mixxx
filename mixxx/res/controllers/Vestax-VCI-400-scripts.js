/****************************************************************/
/*      Vestax VCI-400 MIDI controller script v1.00             */
/*          Copyright (C) 2011, Tobias Rafreider                */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.11                                  */
/****************************************************************/


/*
 * The VCI-400 class definition
 * All attributes of this class represent
 * buttons and controls affecting the MASTER output
 */
VestaxVCI400 = new function() {
   this.group = "[Master]";
   this.Controls = []; //The list of control objects, i.e., knobs
   this.Buttons = [];  //The list of buttons objects
}
VestaxVCI400.selectTrack = function(value){
    if(value == 127){
        engine.setValue("[Playlist]","SelectPrevTrack", true);
    }
    else{
        engine.setValue("[Playlist]","SelectNextTrack", true);
    }
}
/*
 * VU meters controls
 */
VestaxVCI400.onVuMeterLChanged = function(value){
    var normalizedVal = parseInt(value*127);
    midi.sendShortMsg("0xbe", 43, normalizedVal);
}
VestaxVCI400.onVuMeterRChanged = function(value){
    var normailizedVal = parseInt(value*127);
    midi.sendShortMsg("0xbe", 44, normailizedVal);

}

VestaxVCI400.ButtonLedState = {"on": 0x7F, "off": 0x00};
/*
 * Adds a button to the VCI-400 class object
 * The button must affect the MASTER.
 *
 * Parameters
 *  - buttonName: 'unique name of the button'
 *  - button 'the button object to be added
 *  - eventHandlder 'the class method which will be executed if button state has changed.
 */
VestaxVCI400.addButton = function(buttonName, button, eventHandler) {
   if(eventHandler) {
      var executionEnvironment = this;
      button.group = this.group;
      function handler(value) {
         button.state = value;
         executionEnvironment[eventHandler](value);
      }
      button.handler = handler;
   }
   this.Buttons[buttonName] = button; //Append the button the class attribute
};

/* ============BUTTON CLASS DEFINITION====================
 *
 * A button refers to push button control
 *
 * =======================================================
 */

VestaxVCI400.ButtonState = {"released":0x00, "pressed":0x7F};
VestaxVCI400.ScratchTimeOut = 20; //time in ms, see finishScratch()
/*
 * The button object
 */

VestaxVCI400.Button = function (statusByte, midiNo) {
   this.statusByte = statusByte;
   this.midiNo = midiNo;
   this.state = VestaxVCI400.ButtonState.released;
   this.illuminated = false;
   this.group; //this will be set when by the addButton method

};
/*
 * Button objects have a 'handleEvent' method
 * ,that is, the action to be performed if pressed.
 */
VestaxVCI400.Button.prototype.handleEvent = function(value) {
   this.handler(value);
};
//Calling this method will illuminate the button depening on if value is true or false
VestaxVCI400.Button.prototype.illuminate = function(value) {
   if(value ==true){
        midi.sendShortMsg(this.statusByte, this.midiNo, VestaxVCI400.ButtonLedState.on);
        this.illuminated = true;
    }
    else{
        midi.sendShortMsg(this.statusByte, this.midiNo, VestaxVCI400.ButtonLedState.off);
        this.illuminated = false;
    }
};

/*
 * =========DECK CLASS DEFINITION ===========================
 *
 * Although the VCI 400 has a 2 deck layout, you can
 * control up to 4 virtual decks.
 * Objects of this class do represent virtual decks.
 *
 *
 */
VestaxVCI400.Deck = function (deckNumber, group, active) {
   this.deckIdentifier = deckNumber;
   this.group = group;
   this.vinylMode = true;
   this.isScratching = false; //becomes true if wheel is touched for scratching
   this.isActive = active; //if this deck is currently controlled by the VCI-400
   this.Buttons = []; //the buttons
   this.deckNumber = group.substring(8,9);// [Channel1]
   this.scratchTimer = -1;
   this.lastScratchTick = -1;
}
/*
 * Each deck has a disjunct set of buttons
 * This method adds a assigns a button to a deck
 */
VestaxVCI400.Deck.prototype.addButton = VestaxVCI400.addButton;


// ===========CREATING LEFT AND RIGHT DECKS OBJECTS
VestaxVCI400.Decks = {
    "A": new VestaxVCI400.Deck("A","[Channel1]", true), //Deck A and B will
    "B": new VestaxVCI400.Deck("B","[Channel2]", true),//be controlled by the VCI-400
    "C": new VestaxVCI400.Deck("C","[Channel3]", false),
    "D": new VestaxVCI400.Deck("D","[Channel4]", false)
};
VestaxVCI400.GroupToDeck = {
    "[Channel1]":"A",
    "[Channel2]":"B",
    "[Channel3]":"C",
    "[Channel4]":"D"
};
//returns the deck object given a group, e.g. "[Channel1]"
VestaxVCI400.GetDeck = function(group) {
   try {
      return VestaxVCI400.Decks[VestaxVCI400.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
}
// #######Adding deck methods like play(), stop, and so on.
/*
 * Starts or stops a track
 */
VestaxVCI400.Deck.prototype.onPlay = function(value) {
   if(value == VestaxVCI400.ButtonState.pressed){
        //If no track is loaded
    	 if (engine.getValue(this.group, "duration") == 0) {
    	        print("No track has been loaded. Vestax VCI400");
    			return; 
    	 }
    	 var currentlyPlaying = engine.getValue(this.group,"play");

    	 if(currentlyPlaying){
    	       //stop track
    	       engine.setValue(this.group,"play",0);    // Stop
			   this.Buttons.PLAY.illuminate(false);    // Turn off the Play LED
    	  }
    	  else{
    	       //start track
    	       engine.setValue(this.group,"play",1);    // Stop
			   this.Buttons.PLAY.illuminate(true);    // Turn on the Play LED
    	  }
    }
};
/*
 * CUE Button
 */
VestaxVCI400.Deck.prototype.onCue = function(value) {
   if(value == VestaxVCI400.ButtonState.pressed){
        //If no track is loaded
    	 if (engine.getValue(this.group, "duration") == 0) {
    	        print("No track has been loaded. Vestax VCI400");
    			return; 
    	 }
    	 engine.setValue(this.group,"cue_default",1);
         this.Buttons.PLAY.illuminate(false);   // Turn on the Play LED off
         this.Buttons.CUE.illuminate(true);  //CUE LED on
     }
     else{ //key up
        //return to cue point
        engine.setValue(this.group,"cue_default",0);
		//TURN CUE LED OFF
		this.Buttons.CUE.illuminate(false);

     }
};
// Vinyl button
VestaxVCI400.Deck.prototype.onVinyl = function(value) {
    this.vinylMode = (value == 127)? true: false;
};
// Turn scratch mode on or off depending on wheel touch
VestaxVCI400.Deck.prototype.onWheelTouch = function(value) {

   if(this.vinylMode == false)
       return;

   if(value == VestaxVCI400.ButtonState.pressed){
       engine.scratchEnable(this.deckNumber, 4096, 33.3333, 0.125, 0.125/32);
       this.isScratching = true;
    }
    else{
        /*
         * Note: When releasing the wheel you might have done a backspin scratch
         * Hence, we must not call 'scratchDisable()' here.
         * Let's set up a timer which check post delayed if the wheel has moved
         */
         if(this.scratchTimer == -1)
            this.scratchTimer = engine.beginTimer(VestaxVCI400.ScratchTimeOut, "VestaxVCI400.Decks."+this.deckIdentifier + ".finishScratch()");
    }
};

VestaxVCI400.Deck.prototype.finishScratch = function(){
    var currentTime = new Date().getTime();
    //print("finishScratch() called");
    if(currentTime - this.lastScratchTick >= VestaxVCI400.ScratchTimeOut)
    {
        engine.stopTimer(this.scratchTimer);
        this.scratchTimer = -1;
        engine.scratchDisable(this.deckNumber);
        this.isScratching = false;
        //print("Disable Scratch");
    }

};
//=========HOT CUE Buttons=========================
VestaxVCI400.Deck.prototype.onHotCue1Activate = function(value){
    //print("HOT CUE 1 ACTIVATED");
    this.onHotCUEButtonPressed(this.Buttons.HOT_CUE1, 1, value);
};
//HOT CUE Buttons
VestaxVCI400.Deck.prototype.onHotCue2Activate = function(value){
    this.onHotCUEButtonPressed(this.Buttons.HOT_CUE2, 2, value);
};
//HOT CUE Buttons
VestaxVCI400.Deck.prototype.onHotCue3Activate = function(value){
    this.onHotCUEButtonPressed(this.Buttons.HOT_CUE3, 3, value);
};
//HOT CUE Buttons
VestaxVCI400.Deck.prototype.onHotCue4Activate = function(value){
    this.onHotCUEButtonPressed(this.Buttons.HOT_CUE4, 4, value);
};
//Helper function to reduce code
VestaxVCI400.Deck.prototype.onHotCUEButtonPressed = function(hotCueButton, hotCueButtonNumber, value){
	var hotCueActivate = "hotcue_".concat(hotCueButtonNumber.toString()).concat("_activate");
	if(value == VestaxVCI400.ButtonState.pressed){
		engine.setValue(this.group, hotCueActivate, 1);
	}
	else{
		engine.setValue(this.group, hotCueActivate, 0);
	}

};

VestaxVCI400.Deck.prototype.onWheelMove = function(value) {
     var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW

     //wheel os touched and scratch mode is active
     if(this.isScratching){
        engine.scratchTick(this.deckNumber, jogValue);
        this.lastScratchTick = new Date().getTime();
     }
     else{
        //pitch bend via jog wheel, jogValue has been adjusted to be more soft
        engine.setValue(this.group,"jog",jogValue/50);
     }
};
VestaxVCI400.Deck.prototype.onPfl = function(value) {
    var isHeadPhoneActive = engine.getValue(this.group,"pfl");
    if(value == VestaxVCI400.ButtonState.pressed){
        if(!isHeadPhoneActive){
           engine.setValue(this.group, "pfl", 1);
           this.Buttons.PFL.illuminate(true);
        }
        else{
           engine.setValue(this.group, "pfl", 0);
           this.Buttons.PFL.illuminate(false);
        }
    }
};

VestaxVCI400.Deck.prototype.onLoopIn = function(value) {
	var isLoopActive = engine.getValue(this.group, "loop_enabled");

	if(value == VestaxVCI400.ButtonState.pressed){
			engine.setValue(this.group, "loop_in", 1);
	}
}

VestaxVCI400.Deck.prototype.onLoopOut = function(value) {
	var isLoopActive = engine.getValue(this.group, "loop_enabled");

	if(value == VestaxVCI400.ButtonState.pressed){
		if(!isLoopActive)
			engine.setValue(this.group, "loop_out", 1);
		else
			engine.setValue(this.group,"reloop_exit",1);  //Loop exit
	}
}

VestaxVCI400.Deck.prototype.onBeatLoop = function(value) {
	var isLoopActive = engine.getValue(this.group, "loop_enabled");

	if(value == VestaxVCI400.ButtonState.pressed){
		if(!isLoopActive)
			engine.setValue(this.group, "beatloop_4", 1);
		else
			engine.setValue(this.group, "beatloop_4", 0);  //Loop exit
	}
}

/*
 * The deck VU meters
 */
VestaxVCI400.Deck.prototype.onVuMeterChanged = function(value, group, key) {
     var normalizedVal = parseInt(value*127);
     var deckNumber = parseInt(group.substring(8,9));
     var midiNo = deckNumber + 1;
     var statusByte = "0xb".concat(midiNo.toString());
     midi.sendShortMsg(statusByte, 17, normalizedVal);
 };

/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */
VestaxVCI400.Deck.prototype.onPlayChanged = function(value, group, key) {
   try {
       var deck = VestaxVCI400.GetDeck(group);

       if(value == 0){
            deck.Buttons.PLAY.illuminate(false); //turn off LEDs
        }
        else{
            deck.Buttons.PLAY.illuminate(true); //turn LED on
        }
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
 };

 VestaxVCI400.Deck.prototype.onCUEChanged = function(value, group, key) {
   try {
       var deck = VestaxVCI400.GetDeck(group);

       if(value == 0){
            deck.Buttons.CUE.illuminate(false); //turn off LEDs
        }
        else{
            deck.Buttons.CUE.illuminate(true); //turn LED on
        }
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
 };


/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */
VestaxVCI400.Deck.prototype.onPFLChanged = function(value, group, key) {
   try {
       var deck = VestaxVCI400.GetDeck(group);

       if(value == 0){
            deck.Buttons.PFL.illuminate(false); //turn off LEDs
        }
        else{
            deck.Buttons.PFL.illuminate(true); //turn LED on
        }
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
 };

 /*
  * This is called when Mixxx notifies us that a button state has changed
  * via engine.connect.
  */
VestaxVCI400.Deck.prototype.onHotCue1Changed = function(value, group, key) {
	try {
        var deck = VestaxVCI400.GetDeck(group);
        deck.onHotCueChanged(deck.Buttons.HOT_CUE1, value);
	}
    catch(ex) {
    	VestaxVCI400.printError(ex);
    }
};
/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */
VestaxVCI400.Deck.prototype.onHotCue2Changed = function(value, group, key) {
	try {
        var deck = VestaxVCI400.GetDeck(group);
        deck.onHotCueChanged(deck.Buttons.HOT_CUE2, value);
	}
    catch(ex) {
    	VestaxVCI400.printError(ex);
    }
};
/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */
VestaxVCI400.Deck.prototype.onHotCue3Changed = function(value, group, key) {
	try {
        var deck = VestaxVCI400.GetDeck(group);
        deck.onHotCueChanged(deck.Buttons.HOT_CUE3, value);
	}
    catch(ex) {
    	VestaxVCI400.printError(ex);
    }
};
/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */
VestaxVCI400.Deck.prototype.onHotCue4Changed = function(value, group, key) {
	try {
        var deck = VestaxVCI400.GetDeck(group);
        deck.onHotCueChanged(deck.Buttons.HOT_CUE4, value);
	}
    catch(ex) {
    	VestaxVCI400.printError(ex);
    }
};
/*
 * This is called when Mixxx notifies us that a button state has changed
 * via engine.connect.
 */

VestaxVCI400.Deck.prototype.onHotCueChanged = function(button, value) {
    try {
        if(value == 0){
             button.illuminate(false); //turn off LEDs
         }
         else{
             button.illuminate(true); //turn LED on
         }
     }
     catch(ex) {
    	 VestaxVCI400.printError(ex);
    }
  };

  /*
   * This is called when Mixxx notifies us that a button state has changed
   * via engine.connect.
   */
VestaxVCI400.Deck.prototype.onLoopStatusChanged = function(value, group, key) {
		try {
	        var deck = VestaxVCI400.GetDeck(group);
	        //if loop has been enabled, illuminate LEDs
	        if(value == 1){
	        	deck.Buttons.LOOP_IN.illuminate(true);
	        	deck.Buttons.LOOP_OUT.illuminate(true);
	        }
	        else{
	        	deck.Buttons.LOOP_IN.illuminate(false);
	        	deck.Buttons.LOOP_OUT.illuminate(false);
	        }
		}
	    catch(ex) {
	    	VestaxVCI400.printError(ex);
	    }
	};
/*
 * Method to set the initial state of a deck
 */
VestaxVCI400.Deck.prototype.init = function() {
   // Make sure all buttons are not illuminated
   for(b in this.Buttons){
        this.Buttons[b].illuminate(false);
    }
    /*
     * Make sure vinyl LED is on as it is the default when switching on the VCI-400
     * Unfortunately, the VINYL buton has no LED receive slot, so we can't
     * enforce the button to be illuminated from a programming point of view.
     * This, however, is not a real limitation. The button is illuminated
     * after the device has been booted.
     */
     this.Buttons.VINYL.illuminate(true);

     //Connect controls
     engine.connectControl(this.group,"play", "VestaxVCI400.Decks."+this.deckIdentifier+".onPlayChanged");
     engine.connectControl(this.group,"cue_default", "VestaxVCI400.Decks."+this.deckIdentifier+".onPlayChanged");
     engine.connectControl(this.group,"pfl", "VestaxVCI400.Decks."+this.deckIdentifier+".onPFLChanged");
     engine.connectControl(this.group,"VuMeter", "VestaxVCI400.Decks."+this.deckIdentifier+".onVuMeterChanged");
     engine.connectControl(this.group,"hotcue_1_enabled", "VestaxVCI400.Decks."+this.deckIdentifier+".onHotCue1Changed");
     engine.connectControl(this.group,"hotcue_2_enabled", "VestaxVCI400.Decks."+this.deckIdentifier+".onHotCue2Changed");
     engine.connectControl(this.group,"hotcue_3_enabled", "VestaxVCI400.Decks."+this.deckIdentifier+".onHotCue3Changed");
     engine.connectControl(this.group,"hotcue_4_enabled", "VestaxVCI400.Decks."+this.deckIdentifier+".onHotCue4Changed");
     engine.connectControl(this.group,"loop_enabled", "VestaxVCI400.Decks."+this.deckIdentifier+".onLoopStatusChanged");
};
//Creating buttons referring to left deck
VestaxVCI400.Decks.A.addButton("PLAY", new VestaxVCI400.Button(0x92,0x1e), "onPlay");
VestaxVCI400.Decks.B.addButton("PLAY", new VestaxVCI400.Button(0x93,0x1e), "onPlay");
//VestaxVCI400.Decks.C.addButton("PLAY", new VestaxVCI400.Button(0x94,0x1e), "onPlay");
//VestaxVCI400.Decks.D.addButton("PLAY", new VestaxVCI400.Button(0x95,0x1e), "onPlay");

VestaxVCI400.Decks.A.addButton("CUE", new VestaxVCI400.Button(0x92,0x1d), "onCue");
VestaxVCI400.Decks.B.addButton("CUE", new VestaxVCI400.Button(0x93,0x1d), "onCue");
//VestaxVCI400.Decks.C.addButton("CUE", new VestaxVCI400.Button(0x94,0x1d), "onCue");
//VestaxVCI400.Decks.D.addButton("CUE", new VestaxVCI400.Button(0x95,0x1d), "onCue");

VestaxVCI400.Decks.A.addButton("VINYL", new VestaxVCI400.Button(0x92,0x06), "onVinyl");
VestaxVCI400.Decks.B.addButton("VINYL", new VestaxVCI400.Button(0x93,0x06), "onVinyl");
//VestaxVCI400.Decks.C.addButton("VINYL", new VestaxVCI400.Button(0x94,0x06), "onVinyl");
//VestaxVCI400.Decks.D.addButton("VINYL", new VestaxVCI400.Button(0x95,0x06), "onVinyl");

VestaxVCI400.Decks.A.addButton("WHEEL", new VestaxVCI400.Button(0x92,0x27), "onWheelTouch");
VestaxVCI400.Decks.B.addButton("WHEEL", new VestaxVCI400.Button(0x93,0x27), "onWheelTouch");
//VestaxVCI400.Decks.C.addButton("WHEEL", new VestaxVCI400.Button(0x94,0x27), "onWheelTouch");
//VestaxVCI400.Decks.D.addButton("WHEEL", new VestaxVCI400.Button(0x95,0x27), "onWheelTouch");

VestaxVCI400.Decks.A.addButton("PFL", new VestaxVCI400.Button(0x92,0x5), "onPfl");
VestaxVCI400.Decks.B.addButton("PFL", new VestaxVCI400.Button(0x93,0x5), "onPfl");
//VestaxVCI400.Decks.C.addButton("PFL", new VestaxVCI400.Button(0x94,0x5), "onPfl");
//VestaxVCI400.Decks.D.addButton("PFL", new VestaxVCI400.Button(0x95,0x5), "onPfl");

VestaxVCI400.Decks.A.addButton("HOT_CUE1", new VestaxVCI400.Button(0x92,0x0b), "onHotCue1Activate");
VestaxVCI400.Decks.A.addButton("HOT_CUE2", new VestaxVCI400.Button(0x92,0x0c), "onHotCue2Activate");
VestaxVCI400.Decks.A.addButton("HOT_CUE3", new VestaxVCI400.Button(0x92,0x0d), "onHotCue3Activate");
VestaxVCI400.Decks.A.addButton("HOT_CUE4", new VestaxVCI400.Button(0x92,0x0e), "onHotCue4Activate");

VestaxVCI400.Decks.B.addButton("HOT_CUE1", new VestaxVCI400.Button(0x93,0x0b), "onHotCue1Activate");
VestaxVCI400.Decks.B.addButton("HOT_CUE2", new VestaxVCI400.Button(0x93,0x0c), "onHotCue2Activate");
VestaxVCI400.Decks.B.addButton("HOT_CUE3", new VestaxVCI400.Button(0x93,0x0d), "onHotCue3Activate");
VestaxVCI400.Decks.B.addButton("HOT_CUE4", new VestaxVCI400.Button(0x93,0x0e), "onHotCue4Activate");

VestaxVCI400.Decks.A.addButton("LOOP_IN", new VestaxVCI400.Button(0x92,0x9), "onLoopIn");
VestaxVCI400.Decks.B.addButton("LOOP_IN", new VestaxVCI400.Button(0x93,0x9), "onLoopIn");

VestaxVCI400.Decks.A.addButton("LOOP_OUT", new VestaxVCI400.Button(0x92,0xA), "onLoopOut");
VestaxVCI400.Decks.B.addButton("LOOP_OUT", new VestaxVCI400.Button(0x93,0xA), "onLoopOut");

//Beat loop push button
VestaxVCI400.Decks.A.addButton("BEAT_LOOP", new VestaxVCI400.Button(0x92,0x14), "onBeatLoop");
VestaxVCI400.Decks.B.addButton("BEAT_LOOP", new VestaxVCI400.Button(0x93,0x14), "onBeatLoop");


/* ================MAPPING FUNCTIONS ==========================
 *
 * Having defined the some objects and class definitions
 * we can now easily build the mapping
 */


/*
 * Called when the MIDI device is opened for set up
 */
VestaxVCI400.init = function (id) {
   engine.setValue("[Master]", "num_decks", 4);
   //Initialize controls and their default values here
   VestaxVCI400.Decks.A.init();
   VestaxVCI400.Decks.B.init();

   //Connect vu meters
   engine.connectControl("[Master]","VuMeterL", "VestaxVCI400.onVuMeterLChanged");
   engine.connectControl("[Master]","VuMeterR", "VestaxVCI400.onVuMeterRChanged");

   //Reset VU meters
   midi.sendShortMsg("0xbe", 43, 0);
   midi.sendShortMsg("0xbe", 44, 0);
};

/*
 * Called when the MIDI device is closed
 */
VestaxVCI400.shutdown = function () {
	//Initialize controls and their default values here
	   VestaxVCI400.Decks.A.init();
	   VestaxVCI400.Decks.B.init();

	 //Reset VU meters
	 midi.sendShortMsg("0xbe", 43, 0);
	 midi.sendShortMsg("0xbe", 44, 0);

};
/*
 * Mapping play for left deck
 */
VestaxVCI400.play = function (channel, control, value, status, group) {
    var deck = VestaxVCI400.GetDeck(group);

    try {
        deck.Buttons.PLAY.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * Mapping CUE button
 */
VestaxVCI400.cue = function (channel, control, value, status, group) {
    try {
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.CUE.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * Mapping vinyl button
 */
VestaxVCI400.vinyl = function (channel, control, value, status, group) {
    try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.VINYL.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * Mapping deck selection switch
 */
VestaxVCI400.deckSwitch = function (channel, control, value, status, group) {
    var deck = VestaxVCI400.GetDeck(group);
    deck.isActive = (value == 127)? true: false;
    print("Deck "+deck.deckIdentifier+ " is controlled by VCI-400: "+deck.isActive);
};
/*
 * Mapping wheel touch
 */
VestaxVCI400.wheelTouch = function (channel, control, value, status, group) {
    try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.WHEEL.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * Mapping wheel motion
 */
VestaxVCI400.wheelMove = function (channel, control, value, status, group) {
    try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.onWheelMove(value);

    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * Mapping PFL buttons, i.e., headphone buttons
 */
VestaxVCI400.pfl = function (channel, control, value, status, group) {
    try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.PFL.handleEvent(value);

    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
VestaxVCI400.trackSelectionWheel = function (channel, control, value, status, group) {
    try{
        var deck = VestaxVCI400.GetDeck(group);
        VestaxVCI400.selectTrack(value);

    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};
/*
 * HOT CUE ACTIVATE Buttons
 */
VestaxVCI400.hotCue1Activate = function (channel, control, value, status, group) {
    try{
       var deck = VestaxVCI400.GetDeck(group);
       deck.Buttons.HOT_CUE1.handleEvent(value);
   }
   catch(ex) {
       print("Error in hotCue1Activate(): 4-Deck control is not supported by Mixxx");
       VestaxVCI400.printError(ex);
  }
};

VestaxVCI400.hotCue2Activate = function (channel, control, value, status, group) {
     try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.HOT_CUE2.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};

VestaxVCI400.hotCue3Activate = function (channel, control, value, status, group) {
     try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.HOT_CUE3.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};

VestaxVCI400.hotCue4Activate = function (channel, control, value, status, group) {
     try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.HOT_CUE4.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
};

VestaxVCI400.beatLoop = function (channel, control, value, status, group) {
	try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.BEAT_LOOP.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
}

VestaxVCI400.loopIn = function (channel, control, value, status, group) {
	try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.LOOP_IN.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
}

VestaxVCI400.loopOut = function (channel, control, value, status, group) {
	try{
        var deck = VestaxVCI400.GetDeck(group);
        deck.Buttons.LOOP_OUT.handleEvent(value);
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
}

VestaxVCI400.beatLengthWheel = function (channel, control, value, status, group) {
	try{
        var deck = VestaxVCI400.GetDeck(group);
        var isLoopActive = engine.getValue(deck.group, "loop_enabled");
        var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW

        if(isLoopActive){
        	if(jogValue > 0)
        		engine.setValue(deck.group, "loop_halve", 1)
        	else
        		engine.setValue(deck.group, "loop_double", 1)
        }
    }
    catch(ex) {
    	VestaxVCI400.printError(ex);
   }
}

VestaxVCI400.printError = function(exception){
	print("Mixxx has detected a mapping error. You may have pressed a button relating to 4-deck control which is currently not supported!");
	print("Error Msg: "+exception.toString());
};
