/* ╔══:::Made Lovingly By Circuitfry:::═════════════════════════════════╗
 * ║     Hercules DJConsole RMX 2 Mapping Scripts                       ║
 * ╚════════════════════════════════════════════════════════════════════╝
 */

function DJCRMX2() {}
DJCRMX2.decks = [];

/* [ Function init ]
 * Initiates some global variables and assigns an ID. Required.
 */
DJCRMX2.init = function (id) {
    DJCRMX2.id = id;
    DJCRMX2.decks[1] = new DJCRMX2.Deck(1);
    DJCRMX2.decks[2] = new DJCRMX2.Deck(2);
    engine.setValue("[Microphone]", "enabled", 0);
    engine.setValue("[Microphone]", "talkover", 0);
}

////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

DJCRMX2.Deck = function(number) {
    this.number = number;
    this.group = "[Channel" + this.number + "]";
    this.scratchTimer = 0;
};

DJCRMX2.Deck.prototype.wheelPress = function (value) {
    if (this.scratchTimer != 0) {
        // The wheel was touched again, reset the timer.
        engine.stopTimer(this.scratchTimer);
        this.scratchTimer = 0;
    }
    if (value == 0x7F) {
        // And the jog wheel is pressed down:

         /*  engine.scratchEnable(int,int,float,float,float,bool);
         * [ int deck ]              Which track/platter is playing?
         * [ int intervalsPerRev ]   # of MIDI signals sent in 1 spin.
         * [ float rpm ]             Imaginary vinyl rotation speed.
         * [ float alpha ]           Just a fine-tuning variable.
         * [ float beta ]            Just a fine-tuning variable.
         * [ bool ramp ]             As far as I know, nothing...
         */

         var alpha = 1.0 / 8;
         var beta = alpha / 32;
         engine.scratchEnable(this.number, 256, 33 + 1/3, alpha, beta);
    } else {
        // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
        // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        var inertiaTime = Math.pow(1.8, scratchRate) * 50;
        if (inertiaTime < 100) {
            // Just do it now.
            this.finishWheelPress();
        } else {         
            this.scratchTimer = engine.beginTimer(
                    100, "DJCRMX2.decks[" + this.number + "].finishWheelPress()", true);
        }
    }
}

DJCRMX2.Deck.prototype.finishWheelPress = function() {
    this.scratchTimer = 0;
    var play = engine.getValue(this.group, "play");
    if (play != 0) {
        // If we are playing, just hand off to the engine.
        engine.scratchDisable(this.number, true);
    } else {
        // If things are paused, there will be a non-smooth handoff between scratching and jogging.
        // Instead, keep scratch on until the platter is not moving.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable scratch and hand off to jogging.
            engine.scratchDisable(this.number, false);
        } else {
            // Check again soon.
            this.scratchTimer = engine.beginTimer(
                    100, "DJCRMX2.decks[" + this.number + "].finishWheelPress()", true);
        }
    }
};


/* [ Function wheelTurn ]
 * Pays attention to the current deck, checks scratching, affects the
 * song accordingly.
 */
DJCRMX2.Deck.prototype.wheelTurn = function (value) {
    var newValue = 0;
    // Spinning backwards = 127 or less (less meaning faster)
    // Spinning forwards  = 1 or more (more meaning faster)
    if (value - 64 > 0) {
        newValue = value - 128;
    } else {
        newValue = value;
    }
    
    if (engine.isScratching(this.number)) {
        engine.scratchTick(this.number, newValue);
    } else {
        engine.setValue(this.group, "jog", newValue);
    }   
}

/*  [ Function wheelPress ]
 * Detects whether a jog wheel is pressed or not and sets a specific
 * variable on and off accordingly.
 */
DJCRMX2.wheelPress = function (channel, control, value, status, group) {
    var deck = 0;     
    if (group == "[Channel1]") {
        deck = 1;
    } else  if (group == "[Channel2]") {
        deck = 2;
    } else {
        return; 
    }
    DJCRMX2.decks[deck].wheelPress(value);
}

 
/* [ Function wheelTurn ]
 * Pays attention to the current deck, checks scratching, affects the
 * song accordingly.
 */
DJCRMX2.wheelTurn = function (channel, control, value, status, group) {
    var deck = 0;     
    if (group == "[Channel1]") {
        deck = 1;
    } else  if (group == "[Channel2]") {
        deck = 2;
    } else {
        return; 
    }
    DJCRMX2.decks[deck].wheelTurn(value);
}

DJCRMX2.micSwitch = function (channel, control, value, status)
{
    if (status == 0x90 && control == 0x48 && value == 0x7F) {
        engine.setValue("[Microphone]","enabled",1);
        engine.setValue("[Microphone]","talkover",1);
    } else if (status == 0x90 && control == 0x48 && value == 0x00) {
        engine.setValue("[Microphone]","enabled",0);
        engine.setValue("[Microphone]","talkover",0);
    }
}

/* [ Function shutdown ] - Version 0.1.3
 * Sets variables down for shutoff.
 */
DJCRMX2.shutdown = function (id) {
    engine.setValue("[Microphone]", "enabled", 0);
    engine.setValue("[Microphone]", "talkover", 0);
}
