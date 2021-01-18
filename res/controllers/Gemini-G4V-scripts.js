<<<<<<< HEAD
////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////
/**
 * Gemini G4V controller script
 *
 * For Mixxx 2.0.0+
 * Written by Javier Vilarroig 2018
 *
 **/
 
 // Utility functions
 
 // Prints the contents of an object for debugging
 function printObj(object) {
 	for (key in object) {
 		print(key+":"+object[key]);
	 }
 }

/*
 * Mapping architecture
 *
 * Class controller - The full controller
 *     Contains:
 *         - decks
 *         - common controls (not in a deck)
 *         - common leds (not in a deck)
 *         - common levels (not in a deck)
 * Class deck - One deck, including the mixer
 *     Contains:
 *         - deck controls
 *         - deck leds
 *         - deck levels
 * Class Connection - One connection between a control and a call back
 * Class control - One control
 * Class led - One led
 * Class level - One level
 */

print("Loading G4V script");

// Connection Class
// Manages the events connections from Mixxx engine
//
// Properties:
// controlGroup:        Mixxx group name
// controlName:            Mixxx control name
// callbackFunction:    Name of the function to be called when the control is triggered
//
// Methods:
// activate():        Activates the connection between Mixxx and the callback function.
//                    Connection is not built until the connection is activated.
// deactivate():    Deactivates the connection between Mixxx and the callback function.
// refresh():        Refreshes the status of the controller.
var Connection = function(group, control, callback) {
    this.groupName = group;
    this.controlName = control;
    this.callbackFunction = callback;
    this.conn = 0;

    if (g4v.debug) {
        print("Connection::constructor: control:" + group + "/" + control + " callback:" + callback);
    }

    this.activate = function() {
        if (g4v.debug) {
            print("Connection::activate: control:" + this.groupName + "/" + this.controlName);
        }

        this.conn = engine.makeConnection(this.groupName, this.controlName, this.callbackFunction);
        this.conn.trigger();
    };

    this.deactivate = function() {
        if (g4v.debug) {
            print("Connection::deactivate: control:" + this.groupName + "/" + this.controlName);
        }

        if (this.conn !== 0) {
            this.conn.disconnect();
        }
    };

    this.refresh = function() {
        if (g4v.debug) {
            print("Connection::refresh: control:" + this.groupName + "/" + this.controlName);
        }

        if (this.conn !== 0) {
            this.conn.trigger();
        }
    };
};

// Control class for control objects
// Represents Mixxx controls
// This controls are to be attached to Decks or Controllers to allow simple
// management of Mixxx controls.
//
// Properties:
// key:     Control name in Mixxx
// group:     Group associated to the control in Mixxx
//
// Methods:
// set(value):    Assign the value to the control
// get():        Return the value of the control

var Control = function(group, key) {
    this.key = key;
    this.group = group;

    if (g4v.debug) {
        print("Control::Constructor - key:" + key + " group:" + group);
    }

    this.set = function(value) {
        if (g4v.debug) {
            print("Control::set - group:" + group + " key:" + this.key + " value:" + value);
        }

        engine.setValue(this.group, this.key, value);
    };

    this.get = function() {
        if (g4v.debug) {
            print("Control::get - key:" + this.key);
        }

        return engine.getValue(this.group, this.key);
    };

    this.id = function() {
        if (g4v.debug) {
            print("Control::id - key:" + this.key);
        }

        return (this.group + "/" + this.key);
    };
};

// Class for managing leds
//
// Properties:
// name:            Name of the control. Used to find the light in the for the callback
// midiS:            MIDI Status value to interact with the light
// midiD:            MIDI Data to send to the deck
// objStr:            Object name. To be used in the call backs
// lit:                Status of the light
// freq:            Flashing delay, in ms
// flashTimerOn:    Timer to manage the flash on events
// flashTimerOff:    Timer to manage the flash off events
// counter:            Counter for limited flashing. null if no blinking
//
// Methods:
// set(value):		Sets light status.
//						value: 1-on, 2-off
// flashOn(cycles): Starts blinking.
//						cycles: Number of cycles before stop flashing. Forever if 0
// flashOff():		Stops flashing. Allows to define if the light must be left on or off
// (P)flashOnce:	Manages one flash cycle.
var Led = function(nameP, midiS, midiD, object) {
    this.name = nameP;
    this.midiStatus = midiS;
    this.midiData = midiD;
    this.lit = false;
    this.freq = 600;
    this.flashTimer = 0;
    this.counter = null;
    this.debug = g4v.debug;


    if (this.debug) {
        print("Led::Constructor - name:"+nameP+" MIDI(status:0x"+midiS.toString(16)+" data:0x"+midiD.toString(16)+") object:"+object);
    }

    this.set = function(value) {
        if (this.debug) {
            print("Led::set - name:" + this.name + ((value == 1) ? " ON " : " OFF "));
        }
        midi.sendShortMsg(this.midiStatus, this.midiData, ((value == 1) ? 0x7F : 0x00));
        this.lit = value;
    };

    this.flashOnce = function() {
        if (this.debug) {
            print("Led::flashOnce");
        }

        this.set(!this.lit);
        
        if (this.counter !== null) {
        	if(--this.counter == 0) {
        		this.flashOff();
    		}
    	}	
    };

    this.flashOff = function() {
        if (this.debug) {
            print("Led::Flash Off - name:" + this.name);
        }

        // Destroys the timer
        if (this.flashTimer !== 0) {
            engine.stopTimer(this.flashTimer);
            this.flashTimer = 0;
        }
        this.counter = null;
    };

    this.flash = function(flashNo) {
        if (this.debug) {
            print("Led::Flash On - name:" + this.name + " flashNo:" + flashNo);
        }

        // if defined, sets the number of cycles
        if (flashNo !== undefined) {
            this.counter = flashNo;
        }

        // Stop flashing in case the led was already flashing
        this.flashOff();

		// First blink
		this.set(!this.lite);
		
        // Set the cycle
        this.flashTimer = engine.beginTimer(this.freq, this.flashOnce.bind(this), false);
    };
};

// Deck class
// The deck class holds properties for checking attributes about the deck (e.g.
// whether track loaded, holds the functions for scratching/pitch bending on the
// jog wheel, and has a 'control' array, which holds all the controls (buttons,etc)
// associated with the deck.
//
// ATTENTION - This class must be customized for the controller
//        - List of controls must be customized
//        - List of connections (including the name of the parent controller variable)
//
// Properties:
// deckNum: Deck Number
// group: The channel associated to that deck
// controls: Array of control objects representing the different controls in the deck

// Methods:
// addConnection: Adds a connection to the deck
// addControl: Adds a control to the deck
// isLoaded: Return true if the deck has a track loaded
// refresh: Refreshes the connections to ensure proper synchronization with the controller
// TODO - To finalize documenting the methods

var Deck = function(deckN) {
    // Deck number
    this.deckNum = deckN;
    // Deck group to wich the deck is associated
    this.group = "[Channel" + this.deckNum + "]";
    // Array of controls that are modified by the controller in Mixxx
    this.controls = [];
    // Array of connections between Mixxx and the controller
    this.connections = [];
    // Array of Pad connections between Mixxx and the controller
    this.padConnections = [];
    // Array of leds on the deck
    this.leds = [];
    // Is this deck active?
    this.active = 0;
    // Is this deck scratching?
    this.scratching = false;
    // Last scratch even
    this.lastScratch = null;
    // Scratch Control Timer
    this.scratchTimer = 0;
    // Pad mode 1 = Hot Cue 2 = Auto Loop 3 = Sample 4 = Loop Roll 5 = Manual Loop 6 = Beat Jump
    this.padMode = 1;
    // Loop Move knob position
    this.loopMovePos = 0x41;
    // Is this deck in slip mode?
    this.slip = 0;

    print("Creating Deck " + deckN);

    ////////////////////////
    // Internal functions //
    ////////////////////////

    // Returns true if there is a track loaded on the deck
    this.isLoaded = function() {
        return (engine.getValue(this.group, "track_loaded"));
    };

    // Facility function for adding led objects
    this.addLed = function(ID, key) {
        if (g4v.debug) {
            print("Deck.addled - ID:" + ID + " key:" + key.toString(16));
        }

        // Insert the led in leds array
        this.leds[ID]= new Led(ID, ((this.deckNum === 1 || this.deckNum === 3) ? 0x90 : 0x91), key, "g4v.decks["+this.deckNum+"].leds.");
    };

    // Facility function for adding control objects to a deck
    this.addDeckControl = function(ID, key, group) {
        if (g4v.debug) {
            print("Deck.addDeckControl - ID:" + ID + " key:" + key + " group:" + group);
        }

        // Insert the control in all the decks controls array
        this.controls[ID] = new Control(((group === undefined) ? this.group : group), key);
    };

    // Facility function for adding common connections objects
    this.addConnection = function(control, callback, name, group) {
        if (g4v.debug) {
            print("Deck.addConnection - key:" + control + " callback:" + callback);
        }

        // Insert the control in the deck controls array
        this.connections[((name === undefined) ? control : name)] = new Connection(((group === undefined) ? this.group : group), control, callback);
    };

    // Facility function for adding pad button connections objects
    this.addPadConnection = function(control, callback, name, group) {
        if (g4v.debug) {
            print("Deck.addPadConnection - key:" + control + " callback:" + callback + " name:" + name + " group:" + group);
        }

        // Insert the control in the deck controls array
        this.padConnections[((name === undefined) ? control : name)] = new Connection(((group === undefined) ? this.group : group), control, callback);
    };

    // Refreshes all the deck connections to ensure synchronization with the controlles
    this.refresh = function() {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").refresh");
        }

        var names = Object.keys(this.connections);

        // Refreshes all the connected call backs
        for (var i = 0; i < names.length; i++) {
            this.connections[names[i]].refresh();
        }

        // Shows status only for decks 3 and 4
        this.leds.deck_select.set((this.deckNum === 3 || this.deckNum === 4) && this.active);

        // Shows Slip status
        this.leds.slip.set(this.slip);
    };

    // Activates or deactivates the deck
    // Parameters:
    //    flag:    1 - Activates the deck, 0 - deactivates the deck
    this.activate = function(flag) {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").activate - flag:" + flag);
        }

        var names = Object.keys(this.connections);

        // Manages connected callbacks
        for (var i = 0; i < names.length; i++) {
            var connection = this.connections[names[i]];
            if (flag) {
                connection.activate();
            } else
                connection.deactivate();
        }

        // Sets status
        this.active = flag;

        // Sets pad status
        this.setPadGroup(this.padMode);

        // Refreshes the deck
        this.refresh();
    };

    // Deactivates scratching if jog wheel is stopped
    this.stopScratch = function() {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").stopScratch");
        }

        print("Stop? "+(Date.now() - this.lastScratch));
        if ((Date.now() - this.lastScratch) < 20) {
            return;
        }

        engine.scratchDisable(this.deckNum, 1);
        engine.stopTimer(this.scratchTimer);
        this.scratching = false;
        this.scratchTimer = 0;

        if (this.slip === 1) {
            this.controls.slip.set(0);
        }
    };

    // Set pad group
    // Parameters:
    //    set - Group to set
    this.setPadGroup = function(set) {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").setPadGroup("+set+")");
        }

        if (this.parMode === set) {
            return;
        }

        // Sets pad mode led Leds
        this.leds.hot_cue.set(set === 1 ? 1 : 0);
        this.leds.auto_loop.set(set === 2 ? 1 : 0);
        this.leds.sample.set(set === 3 ? 1 : 0);
        this.leds.loop_roll.set(set === 4 ? 1 : 0);
        if (set === 5) {
            this.leds.loop_roll.flash();
        } else {
            this.leds.loop_roll.flashOff();
        }
        if (set === 6) {
            this.leds.sample.flash();
        } else {
            this.leds.sample.flashOff();
        }

        // Deactivates old connections
        var sampleGroup;
        if (this.padMode === 1) {
            this.padConnections.hotcue_1_enabled.deactivate();
            this.padConnections.hotcue_2_enabled.deactivate();
            this.padConnections.hotcue_3_enabled.deactivate();
            this.padConnections.hotcue_4_enabled.deactivate();
            this.padConnections.hotcue_5_enabled.deactivate();
            this.padConnections.hotcue_6_enabled.deactivate();
            this.padConnections.hotcue_7_enabled.deactivate();
            this.padConnections.hotcue_8_enabled.deactivate();
        }
        if (this.padMode === 2) {
            this.padConnections["beatloop_0.125_enabled"].deactivate();
            this.padConnections["beatloop_0.25_enabled"].deactivate();
            this.padConnections["beatloop_0.5_enabled"].deactivate();
            this.padConnections.beatloop_1_enabled.deactivate();
            this.padConnections.beatloop_2_enabled.deactivate();
            this.padConnections.beatloop_4_enabled.deactivate();
            this.padConnections.beatloop_8_enabled.deactivate();
            this.padConnections.beatloop_16_enabled.deactivate();
        }
        if (this.padMode === 3) {
            // Hides Sampler
            this.controls.show_samplers.set(0);
            sampleGroup = 1+(this.deckNum-1)*16;
            print("Group:"+sampleGroup);
            this.padConnections["s"+(sampleGroup+0)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+1)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+2)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+3)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+4)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+5)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+6)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+7)+"_play_indicator"].deactivate();
        }
        if (this.padMode === 4) {
            this.padConnections["beatlooproll_0.125_activate"].deactivate();
            this.padConnections["beatlooproll_0.25_activate"].deactivate();
            this.padConnections["beatlooproll_0.5_activate"].deactivate();
            this.padConnections.beatlooproll_1_activate.deactivate();
            this.padConnections.beatlooproll_2_activate.deactivate();
            this.padConnections.beatlooproll_4_activate.deactivate();
            this.padConnections.beatlooproll_8_activate.deactivate();
            this.padConnections.beatlooproll_16_activate.deactivate();
        }
        if (this.padMode === 5) {
            this.padConnections.loop_in.deactivate();
            this.padConnections.loop_out.deactivate();
            this.padConnections.loop_enabled.deactivate();
        }

        // Activates pad connections
        if (set === 1) {    // Hot Cue Mode
            this.padConnections.hotcue_1_enabled.activate();
            this.padConnections.hotcue_2_enabled.activate();
            this.padConnections.hotcue_3_enabled.activate();
            this.padConnections.hotcue_4_enabled.activate();
            this.padConnections.hotcue_5_enabled.activate();
            this.padConnections.hotcue_6_enabled.activate();
            this.padConnections.hotcue_7_enabled.activate();
            this.padConnections.hotcue_8_enabled.activate();
        }
        if (set === 2) {    // Auto Loop mode
            this.padConnections["beatloop_0.125_enabled"].activate();
            this.padConnections["beatloop_0.25_enabled"].activate();
            this.padConnections["beatloop_0.5_enabled"].activate();
            this.padConnections.beatloop_1_enabled.activate();
            this.padConnections.beatloop_2_enabled.activate();
            this.padConnections.beatloop_4_enabled.activate();
            this.padConnections.beatloop_8_enabled.activate();
            this.padConnections.beatloop_16_enabled.activate();
        }
        if (set === 3) {    // Samples mode
            // Shows Sampler
            this.controls.show_samplers.set(1);
            sampleGroup = 1+(this.deckNum-1)*16;
            print("Group:"+sampleGroup);
            this.padConnections["s"+(sampleGroup+0)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+1)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+2)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+3)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+4)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+5)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+6)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+7)+"_play_indicator"].activate();
        }
        if (set === 4) { // Loop Roll mode
            this.padConnections["beatlooproll_0.125_activate"].activate();
            this.padConnections["beatlooproll_0.25_activate"].activate();
            this.padConnections["beatlooproll_0.5_activate"].activate();
            this.padConnections.beatlooproll_1_activate.activate();
            this.padConnections.beatlooproll_2_activate.activate();
            this.padConnections.beatlooproll_4_activate.activate();
            this.padConnections.beatlooproll_8_activate.activate();
            this.padConnections.beatlooproll_16_activate.activate();
        }
        if (set === 5) { // Manual loop mode
            this.padConnections.loop_in.activate();
            this.padConnections.loop_out.activate();
            this.padConnections.loop_enabled.activate();
        }
        if (set === 6) {    // Beat Jump mode
            // Leds off - no connections
            this.leds.pad1.set(0);
            this.leds.pad2.set(0);
            this.leds.pad3.set(0);
            this.leds.pad4.set(0);
            this.leds.pad5.set(0);
            this.leds.pad6.set(0);
            this.leds.pad7.set(0);
            this.leds.pad8.set(0);
        }

        // Sets pad mode
        this.padMode = set;
    };

    // Shutdown the deck
    this.shutdown = function() {
        var myObj = this;
        Object.keys(this.leds).forEach(function(element, key) { myObj.leds[element].set(0); });
    };

    //////////////////////////////////////////////////////////////////////////////
    // Call back functions for connecting Mixxx controls to controller controls //
    //////////////////////////////////////////////////////////////////////////////
    this.cbPlay = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.play.set(value === 0 ? 0 : 1);
    };

    this.cbCue = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.cue.set(value === 0 ? 0 : 1);
    };

    this.cbSync = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.sync.set(value === 0 ? 0 : 1);
    };

    this.cbKeylock = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.keylock.set(value === 0 ? 0 : 1);
    };

    this.cbPad1 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad1.set(value === 0 ? 0 : 1);
    };

    this.cbPad2 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad2.set(value === 0 ? 0 : 1);
    };

    this.cbPad3 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad3.set(value === 0 ? 0 : 1);
    };

    this.cbPad4 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad4.set(value === 0 ? 0 : 1);
    };

    this.cbPad5 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad5.set(value === 0 ? 0 : 1);
    };

    this.cbPad6 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad6.set(value === 0 ? 0 : 1);
    };

    this.cbPad7 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad7.set(value === 0 ? 0 : 1);
    };

    this.cbPad8 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) === "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9, 10) === "]" ? 9 : 10))/16)].leds.pad8.set(value === 0 ? 0 : 1);
    };

    this.cbFx = function(value, group, control) {
        g4v.decks[group.substring(23, 24)].leds.fx.set(value === 0 ? 0 : 1);
    };

    /////////////////////////////////////////////////////
    // Callback for reacting to deck controller events //
    /////////////////////////////////////////////////////

    // Play buttons 0x90/0x01 - 0x91/0x01
    // Simple: Starts playing the deck
    this.evBtnPlay = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnPlay - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Toggles control
        this.controls.play.set(this.controls.play.get() === 1 ? 0 : 1);
    };

    // Cue buttons 0x90/0x02 - 0x91/0x02
    // Simple: CUE press
    this.evBtnCue = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnCue - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Toggles control
        this.controls.cue.set(value === 0x7f ? 1 : 0);
    };

    // Cue buttons 0x90/0x03 - 0x91/0x03
    // Simple: Play track from CUE point
    this.evBtnCup = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnCup - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Toggles control
        this.controls.cup.set(value === 0x7f ? 1 : 0);
    };

    // Sync buttons 0x90/0x04 - 0x91/0x04
    // Simple: Sync button press
    this.evBtnSync = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSync - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // On release, check if less than a second happened since push deactivates sync
        if (value === 0x00) {
            print("Time:"+(Date.now() - this.syncPressTime));
            if ((Date.now() - this.syncPressTime) < 1000) {
                this.controls.sync.set(0);
            }
            this.syncPressTime = 0;
            return;
        }

        // Activates sync
        this.controls.sync.set(1);

        // Saves time
        this.syncPressTime = Date.now();
    };

    // Key buttons 0x90/0x05 - 0x91/0x05
    // Simple: Toggle Key lock
    this.evBtnKey = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnKey - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Toggles control
        this.controls.keylock.set(this.controls.keylock.get() === 0 ? 1 : 0);
    };

    // Fx buttons 0x90/0x1a - 0x91/0x1a
    // Fx buttons 0x90/0x1a - 0x91/0x1a
    this.evBtnFx = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnFx - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Toggles control
        script.toggleControl("[EffectRack1_EffectUnit"+this.deckNum+"]", "enabled");
    };

    // Slip buttons 0x90/0x19 - 0x91/0x19
    // Simple: Toggle Slip
    this.evBtnSlip = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSlip - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Toggles control
        this.slip = (this.slip === 0 ? 1 : 0);

        // Set led
        this.leds.slip.set(this.slip);
    };

    // Jog Wheel buttons 0x90/0x25 - 0x91/0x25
    // Press: Activate scratching
    // Release: If jog wheel is stopped, cancels scratching
    this.evBtnJog = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnJog - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch (value) {
        case 0x7f:
            var intervalsPerRev = 250;
            var rpm = 30+1/3;
            var alpha = (1.0/4);
            var beta = (alpha / 32);
            engine.scratchEnable(this.deckNum, intervalsPerRev, rpm, alpha, beta);
            this.scratching = true;
            if (this.slip === 1) {
                this.controls.slip.set(1);
            }
            break;
        case 0x00:
            if (this.scratchTimer === 0) {
                this.scratchTimer = engine.beginTimer(20, "g4v.decks["+this.deckNum+"].stopScratch()");
            }
            break;
        }
    };

    // Loop Move knob 0xb0/0x02 - 0xb1/0x02
    // Move Loop position
    this.evKnoLoopMove = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoLoopMove - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        this.controls.loop_move.set(value-this.loopMovePos);
        this.loopMovePos = value;
    };

    // Sample Volume knob 0xb0/0x03 - 0xb1/0x03
    // Sets sample volume
    this.evKnoSampleVol = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoSampleVol - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        if (g4v.shift) {
            this.controls.pitch.set(script.absoluteNonLin(value, -6, 0, 6));
        } else {
            for (var i = 1; i < 65; i++) {
                engine.setValue("[Sampler"+i+"]", "volume", value/0x7f);
            }
        }
    };

    // FX Mix knob 0xb0/0x04 - 0xb1/0x04
    // Select the effect mix leve
    this.evKnoMix = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoMix - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        var mix = ((value/0x7f)*1);
        engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "mix", mix);
    };

    // FX Meta knob 0xb0/0x05 - 0xb1/0x05
    // Select the effect Meta level
    this.evKnoMeta = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evKnoMeta - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        var mix = ((value/0x7f)*1);
        engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "super1", mix);
    };

    // Jog Wheel encoder 0xb0/0x06 - 0xb1/0x06
    // Scratch or Nuddging
    this.evEncJog = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evEncJog - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch (this.scratching) {
        case true:    // We are scratching, if shift, fast search
            engine.scratchTick(this.deckNum, (value > 0x40 ? 1 : -1)*(g4v.shift === 1 ? 10 : 1.4));
            this.lastScratch = Date.now();
            break;
        case false:    // We are nuddging, pay attention to multiplier to adjust sensitivity
            engine.setValue("[Channel"+this.deckNum+"]", "jog", (value > 0x40 ? 1 : -1)*0.2);
            break;
        }
    };

    // Pad Button 0x90/0x09 - 0x90/0x0a - 0x90/0x0b - 0x90/0x0c - 0x90/0x0d - 0x90/0x0e - 0x90/0x0f - 0x90/0x10 - 0x91/0x09 - 0x91/0x0a - 0x91/0x0b - 0x91/0x0c - 0x91/0x0d - 0x91/0x0e - 0x91/0x0f - 0x91/0x10
    // Press: Different functions depending on pad mode
    this.evBtnPad = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnPad - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch (this.padMode) {
        case 1:    // Hot Cue
            if (midino >= 0x09 && midino <= 0x10) {
                this.controls["hot_cue_"+(midino-0x08)].set((value === 0x7f ? 1 : 0));
            }
            if (midino >= 0x10 && midino <= 0x18) {
                this.controls["clr_cue_"+(midino-0x10)].set(1);
            }
            break;
        case 2:    // Auto Loop
            // Ignore key off
            if (value === 0x00) {
                break;
            }
            // Sets loop
            switch (midino) {
            case 0x09:
                this.controls["aloop_0.125"].set(1);
                break;
            case 0x0a:
                this.controls["aloop_0.25"].set(1);
                break;
            case 0x0b:
                this.controls["aloop_0.5"].set(1);
                break;
            case 0x0c:
                this.controls.aloop_1.set(1);
                break;
            case 0x0d:
                this.controls.aloop_2.set(1);
                break;
            case 0x0e:
                this.controls.aloop_4.set(1);
                break;
            case 0x0f:
                this.controls.aloop_8.set(1);
                break;
            case 0x10:
                this.controls.aloop_16.set(1);
                break;
            }
            break;
        case 3:    // Sampler
            // Ingnore key off
            if (value === 0x00) {
                break;
            }
            // Triggers sample
            switch (midino) {
            case 0x09:
                this.controls.s1_play.set(1);
                break;
            case 0x0a:
                this.controls.s2_play.set(1);
                break;
            case 0x0b:
                this.controls.s3_play.set(1);
                break;
            case 0x0c:
                this.controls.s4_play.set(1);
                break;
            case 0x0d:
                //this.controls.s1_play.set(1);
                break;
            case 0x0e:
                //this.controls.s1_play.set(1);
                break;
            case 0x0f:
                //this.controls.s1_play.set(1);
                break;
            case 0x10:
                //this.controls.s1_play.set(1);
                break;
            }
            break;
        case 4:    // Loop Roll
            switch (midino) {
            case 0x09:
                this.controls["alooproll_0.125"].set(value === 0 ? 0 : 1);
                break;
            case 0x0a:
                this.controls["alooproll_0.25"].set(value === 0 ? 0 : 1);
                break;
            case 0x0b:
                this.controls["alooproll_0.5"].set(value === 0 ? 0 : 1);
                break;
            case 0x0c:
                this.controls.alooproll_1.set(value === 0 ? 0 : 1);
                break;
            case 0x0d:
                this.controls.alooproll_2.set(value === 0 ? 0 : 1);
                break;
            case 0x0e:
                this.controls.alooproll_4.set(value === 0 ? 0 : 1);
                break;
            case 0x0f:
                this.controls.alooproll_8.set(value === 0 ? 0 : 1);
                break;
            case 0x10:
                this.controls.alooproll_16.set(value === 0 ? 0 : 1);
                break;
            }
            break;
        case 5: // Manual loop
            // Ignore key off
            if (value === 0x00) {
                break;
            }
            switch (midino) {
            case 0x09:
                this.controls.loop_in.set(1);
                break;
            case 0x0a:
                this.controls.loop_out.set(1);
                break;
            case 0x0b:
                this.controls.reloop_exit.set(1);
                break;
            case 0x0d:
                this.controls.loop_halve.set(1);
                break;
            case 0x0e:
                this.controls.loop_double.set(1);
                break;
            case 0x0f:
                this.controls.loop_move.set(-1);
                break;
            case 0x10:
                this.controls.loop_move.set(1);
                break;
            }
            break;
        case 6: // Beat Jump mode
            // Ignore key off
            if (value === 0x00) {
                break;
            }
            switch (midino) {
            case 0x09:
                this.controls["beatjump_0.125_f"].set(value === 0 ? 0 : 1);
                break;
            case 0x0a:
                this.controls["beatjump_0.25_f"].set(value === 0 ? 0 : 1);
                break;
            case 0x0b:
                this.controls["beatjump_0.5_f"].set(value === 0 ? 0 : 1);
                break;
            case 0x0c:
                this.controls.beatjump_1_f.set(value === 0 ? 0 : 1);
                break;
            case 0x0d:
                this.controls.beatjump_2_f.set(value === 0 ? 0 : 1);
                break;
            case 0x0e:
                this.controls.beatjump_4_f.set(value === 0 ? 0 : 1);
                break;
            case 0x0f:
                this.controls.beatjump_8_f.set(value === 0 ? 0 : 1);
                break;
            case 0x10:
                this.controls.beatjump_16_f.set(value === 0 ? 0 : 1);
                break;
            case 0x11:
                this.controls["beatjump_0.125_b"].set(value === 0 ? 0 : 1);
                break;
            case 0x12:
                this.controls["beatjump_0.25_b"].set(value === 0 ? 0 : 1);
                break;
            case 0x13:
                this.controls["beatjump_0.5_b"].set(value === 0 ? 0 : 1);
                break;
            case 0x14:
                this.controls.beatjump_1_b.set(value === 0 ? 0 : 1);
                break;
            case 0x15:
                this.controls.beatjump_2_b.set(value === 0 ? 0 : 1);
                break;
            case 0x16:
                this.controls.beatjump_4_b.set(value === 0 ? 0 : 1);
                break;
            case 0x17:
                this.controls.beatjump_8_b.set(value === 0 ? 0 : 1);
                break;
            case 0x18:
                this.controls.beatjump_16_b.set(value === 0 ? 0 : 1);
                break;
            }
            break;
        }
    };

    // Hot Cue buttons 0x90/0x1b - 0x91/0x1b
    // Simple: Activates Hot Cue pad mode
    this.evBtnHotCue = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnHotCue - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(1);
    };

    // Auto Loop buttons 0x90/0x1C - 0x91/0x1C
    // Simple: Activates Auto Loop pad mode
    this.evBtnAutoLoop = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnAutoLoop - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(2);
    };

    // Sample buttons 0x90/0x1d - 0x91/0x1d
    // Simple: Activates pad Sample Mode
    this.evBtnSample = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSample - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(3);
    };

    // Loop Roll buttons 0x90/0x1e - 0x91/0x1e
    // Simple: Activates Loop Roll pad mode
    this.evBtnLoopRoll = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnLoopRoll - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(4);
    };

    // Loop Roll buttons 0x90/0x22 - 0x91/0x22
    // Simple: Activates Manual Loop pad mode
    this.evBtnManualLoop = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnManualLoop - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(5);
    };

    // Loop Roll buttons 0x90/0x21 - 0x91/0x21
    // Shift: Activates Beat Jump pad mode
    this.evBtnBeatJump = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnBeatJump - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(6);
    };

    // Fx Select buttons 0x90/0x27 - 0x91/0x27
    // Shift: Selects FX
    this.evBtnFxSelect = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnFxSelect - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        // Sets Pad mode
        this.controls.fx_select.set(1);
    };
    // Tempo slider 0xb0/0x01 - 0xB1/0x01
    // Simple: Sets tempo rate
    this.evSliTempo = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evSliTempo - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        this.controls.rate.set(-script.absoluteNonLin(value, -1, 0, 1));
    };

    ///////////////////////////
    // Constructs the object //
    ///////////////////////////

    // Creates all the leds
    this.addLed("play", 0x01);
    this.addLed("cue", 0x02);
    this.addLed("cup", 0x03);
    this.addLed("sync", 0x04);
    this.addLed("keylock", 0x05);
    this.addLed("bank", 0x07);
    this.addLed("pad1", 0x09);
    this.addLed("pad2", 0x0a);
    this.addLed("pad3", 0x0b);
    this.addLed("pad4", 0x0c);
    this.addLed("pad5", 0x0d);
    this.addLed("pad6", 0x0e);
    this.addLed("pad7", 0x0f);
    this.addLed("pad8", 0x10);
    this.addLed("pad8", 0x10);
    this.addLed("slip", 0x19);
    this.addLed("fx", 0x1a);
    this.addLed("hot_cue", 0x1b);
    this.addLed("auto_loop", 0x1c);
    this.addLed("sample", 0x1d);
    this.addLed("loop_roll", 0x1e);
    this.addLed("scratch", 0x23);
    this.addLed("deck_select", 0x26);
    this.addLed("shift", 0x28);

    // Creates connections to common controls
    this.addConnection("play_indicator", this.cbPlay);
    this.addConnection("cue_indicator", this.cbCue);
    this.addConnection("sync_mode", this.cbSync);
    this.addConnection("keylock", this.cbKeylock);
    this.addConnection("enabled", this.cbFx, "fx", "[EffectRack1_EffectUnit"+this.deckNum+"]");

    // Creates connections to pad buttons
    this.addPadConnection("hotcue_1_enabled", this.cbPad1);
    this.addPadConnection("hotcue_2_enabled", this.cbPad2);
    this.addPadConnection("hotcue_3_enabled", this.cbPad3);
    this.addPadConnection("hotcue_4_enabled", this.cbPad4);
    this.addPadConnection("hotcue_5_enabled", this.cbPad5);
    this.addPadConnection("hotcue_6_enabled", this.cbPad6);
    this.addPadConnection("hotcue_7_enabled", this.cbPad7);
    this.addPadConnection("hotcue_8_enabled", this.cbPad8);
    this.addPadConnection("beatloop_0.125_enabled", this.cbPad1);
    this.addPadConnection("beatloop_0.25_enabled", this.cbPad2);
    this.addPadConnection("beatloop_0.5_enabled", this.cbPad3);
    this.addPadConnection("beatloop_1_enabled", this.cbPad4);
    this.addPadConnection("beatloop_2_enabled", this.cbPad5);
    this.addPadConnection("beatloop_4_enabled", this.cbPad6);
    this.addPadConnection("beatloop_8_enabled", this.cbPad7);
    this.addPadConnection("beatloop_16_enabled", this.cbPad8);
    this.addPadConnection("beatlooproll_0.125_activate", this.cbPad1);
    this.addPadConnection("beatlooproll_0.25_activate", this.cbPad2);
    this.addPadConnection("beatlooproll_0.5_activate", this.cbPad3);
    this.addPadConnection("beatlooproll_1_activate", this.cbPad4);
    this.addPadConnection("beatlooproll_2_activate", this.cbPad5);
    this.addPadConnection("beatlooproll_4_activate", this.cbPad6);
    this.addPadConnection("beatlooproll_8_activate", this.cbPad7);
    this.addPadConnection("beatlooproll_16_activate", this.cbPad8);
    this.addPadConnection("loop_in", this.cbPad1);
    this.addPadConnection("loop_out", this.cbPad2);
    this.addPadConnection("loop_enabled", this.cbPad3);
    this.addPadConnection("track_samples", this.cbPad1, "s"+(((this.deckNum-1)*16)+1)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+1)+"]");
    this.addPadConnection("track_samples", this.cbPad2, "s"+(((this.deckNum-1)*16)+2)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+2)+"]");
    this.addPadConnection("track_samples", this.cbPad3, "s"+(((this.deckNum-1)*16)+3)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+3)+"]");
    this.addPadConnection("track_samples", this.cbPad4, "s"+(((this.deckNum-1)*16)+4)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+4)+"]");
    this.addPadConnection("track_samples", this.cbPad5, "s"+(((this.deckNum-1)*16)+5)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+5)+"]");
    this.addPadConnection("track_samples", this.cbPad6, "s"+(((this.deckNum-1)*16)+6)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+6)+"]");
    this.addPadConnection("track_samples", this.cbPad7, "s"+(((this.deckNum-1)*16)+7)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+7)+"]");
    this.addPadConnection("track_samples", this.cbPad8, "s"+(((this.deckNum-1)*16)+8)+"_play_indicator", "[Sampler"+(((this.deckNum-1)*16)+8)+"]");

    // Creates all the controls for the deck
    this.addDeckControl("cue", "cue_default");
    this.addDeckControl("play", "play");
    this.addDeckControl("sync", "sync_enabled");
    this.addDeckControl("sync_master", "sync_master");
    this.addDeckControl("cup", "cue_gotoandplay");
    this.addDeckControl("keylock", "keylock");
    this.addDeckControl("slip", "slip_enabled");
    this.addDeckControl("hot_cue_1", "hotcue_1_activate");
    this.addDeckControl("hot_cue_2", "hotcue_2_activate");
    this.addDeckControl("hot_cue_3", "hotcue_3_activate");
    this.addDeckControl("hot_cue_4", "hotcue_4_activate");
    this.addDeckControl("hot_cue_5", "hotcue_5_activate");
    this.addDeckControl("hot_cue_6", "hotcue_6_activate");
    this.addDeckControl("hot_cue_7", "hotcue_7_activate");
    this.addDeckControl("hot_cue_8", "hotcue_8_activate");
    this.addDeckControl("clr_cue_1", "hotcue_1_clear");
    this.addDeckControl("clr_cue_2", "hotcue_2_clear");
    this.addDeckControl("clr_cue_3", "hotcue_3_clear");
    this.addDeckControl("clr_cue_4", "hotcue_4_clear");
    this.addDeckControl("clr_cue_5", "hotcue_5_clear");
    this.addDeckControl("clr_cue_6", "hotcue_6_clear");
    this.addDeckControl("clr_cue_7", "hotcue_7_clear");
    this.addDeckControl("clr_cue_8", "hotcue_8_clear");
    this.addDeckControl("aloop_0.125", "beatloop_0.125_toggle");
    this.addDeckControl("aloop_0.25", "beatloop_0.25_toggle");
    this.addDeckControl("aloop_0.5", "beatloop_0.5_toggle");
    this.addDeckControl("aloop_1", "beatloop_1_toggle");
    this.addDeckControl("aloop_2", "beatloop_2_toggle");
    this.addDeckControl("aloop_4", "beatloop_4_toggle");
    this.addDeckControl("aloop_8", "beatloop_8_toggle");
    this.addDeckControl("aloop_16", "beatloop_16_toggle");
    this.addDeckControl("aloop_0.125_en", "beatloop_0.125_enabled");
    this.addDeckControl("aloop_0.25_en", "beatloop_0.25_enabled");
    this.addDeckControl("aloop_0.5_en", "beatloop_0.5_enabled");
    this.addDeckControl("aloop_1_en", "beatloop_1_enabled");
    this.addDeckControl("aloop_2_en", "beatloop_2_enabled");
    this.addDeckControl("aloop_4_en", "beatloop_4_enabled");
    this.addDeckControl("aloop_8_en", "beatloop_8_enabled");
    this.addDeckControl("aloop_16_en", "beatloop_16_enabled");
    this.addDeckControl("alooproll_0.125", "beatlooproll_0.125_activate");
    this.addDeckControl("alooproll_0.25", "beatlooproll_0.25_activate");
    this.addDeckControl("alooproll_0.5", "beatlooproll_0.5_activate");
    this.addDeckControl("alooproll_1", "beatlooproll_1_activate");
    this.addDeckControl("alooproll_2", "beatlooproll_2_activate");
    this.addDeckControl("alooproll_4", "beatlooproll_4_activate");
    this.addDeckControl("alooproll_8", "beatlooproll_8_activate");
    this.addDeckControl("alooproll_16", "beatlooproll_16_activate");
    this.addDeckControl("alooproll_0.125_en", "beatlooproll_0.125_activate");
    this.addDeckControl("alooproll_0.25_en", "beatlooproll_0.25_activate");
    this.addDeckControl("alooproll_0.5_en", "beatlooproll_0.5_activate");
    this.addDeckControl("alooproll_1_en", "beatlooproll_1_activate");
    this.addDeckControl("alooproll_2_en", "beatlooproll_2_activate");
    this.addDeckControl("alooproll_4_en", "beatlooproll_4_activate");
    this.addDeckControl("alooproll_8_en", "beatlooproll_8_activate");
    this.addDeckControl("alooproll_16_en", "beatlooproll_16_activate");
    this.addDeckControl("loop_in", "loop_in");
    this.addDeckControl("loop_out", "loop_out");
    this.addDeckControl("reloop_exit", "reloop_exit");
    this.addDeckControl("loop_halve", "loop_halve");
    this.addDeckControl("loop_double", "loop_double");
    this.addDeckControl("beatjump_0.125_f", "beatjump_0.125_forward");
    this.addDeckControl("beatjump_0.25_f", "beatjump_0.25_forward");
    this.addDeckControl("beatjump_0.5_f", "beatjump_0.5_forward");
    this.addDeckControl("beatjump_1_f", "beatjump_1_forward");
    this.addDeckControl("beatjump_2_f", "beatjump_2_forward");
    this.addDeckControl("beatjump_4_f", "beatjump_4_forward");
    this.addDeckControl("beatjump_8_f", "beatjump_8_forward");
    this.addDeckControl("beatjump_16_f", "beatjump_16_forward");
    this.addDeckControl("beatjump_0.125_b", "beatjump_0.125_backward");
    this.addDeckControl("beatjump_0.25_b", "beatjump_0.25_backward");
    this.addDeckControl("beatjump_0.5_b", "beatjump_0.5_backward");
    this.addDeckControl("beatjump_1_b", "beatjump_1_backward");
    this.addDeckControl("beatjump_2_b", "beatjump_2_backward");
    this.addDeckControl("beatjump_4_b", "beatjump_4_backward");
    this.addDeckControl("beatjump_8_b", "beatjump_8_backward");
    this.addDeckControl("beatjump_16_b", "beatjump_16_backward");
    this.addDeckControl("rate", "rate");
    this.addDeckControl("pitch", "pitch");
    this.addDeckControl("loop_move_f", "loop_move_1_forward");
    this.addDeckControl("loop_move_b", "loop_move_1_backward");
    this.addDeckControl("show_samplers", "show_samplers", "[Samplers]");
    this.addDeckControl("s1_play", "start_play", "[Sampler1]");
    this.addDeckControl("s2_play", "start_play", "[Sampler2]");
    this.addDeckControl("s3_play", "start_play", "[Sampler3]");
    this.addDeckControl("s4_play", "start_play", "[Sampler4]");
    this.addDeckControl("fx_select", "next_chain", "[EffectRack1_EffectUnit"+this.deckNum+"]");
    this.addDeckControl("loop_move", "loop_move");

    // Applies effect to channel
    engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "group_[Channel"+this.deckNum+"]_enable", 1);

    if (g4v.debug) {
        print("Deck created - DeckNum:" + this.deckNum + " group:" + this.group);
    }
};

// Controller object
var MyController = function() {
    // Set to 1 to debug the class
    this.debug = true;
    // Array of Decks on the controller
    this.decks = [];
    // Array of global controls in the controller (not in a deck)
    this.master = [];
    // Array of global leds (not in a deck)
    this.leds = [];
    // Array of connection objects
    this.connections = [];
    // Shift status
    this.shift = 0;
    // Orientation status
    // 2=left, 0=right, 1 = none
    this.orientation = [2, 2, 0, 0];
    // Active decks (left to right)
    this.aDecks = [1, 2];
    // Browse mode false = tracks lists = lists
    this.browseMode = false;



    // Facility function for adding connection objects
    this.addControllerConnection = function(group, control, func) {
        if (g4v.debug) {
            print("Controller::addConnection - group:" + group + " control:" + control);
        }

        // Insert the control in all the decks controls array
        g4v.connections[group+control] = engine.makeConnection(group, control, func.bind(this));

        // Triggers the connection to sync
        g4v.connections[group+control].trigger();
    };

    // Facility function for adding deck objects
    this.addDeck = function(ID) {
        if (g4v.debug) {
            print("Controller::addDeck - ID:" + ID);
        }

        this.decks[ID] = new Deck(ID);
    };

    // Refreshes decks to ensure control synchronization
    this.refreshDecks = function() {
        this.decks[1].refresh();
        this.decks[2].refresh();
        this.decks[3].refresh();
        this.decks[4].refresh();
    };

    // Init the Controller
    this.init = function(id) {
        if (g4v.debug) {
            print("Init G4V id:" + id);
        }

        // //////////////////////////////
        // Creates all the led objects //
        // //////////////////////////////
        this.leds.LeftOrientation1 = new Led("LeftOrientation1", 0x93, 0x05);
        this.leds.LeftOrientation2 = new Led("LeftOrientation2", 0x93, 0x06);
        this.leds.LeftOrientation3 = new Led("LeftOrientation3", 0x93, 0x07);
        this.leds.LeftOrientation4 = new Led("LeftOrientation4", 0x93, 0x08);
        this.leds.RightOrientation1 = new Led("RightOrientation1", 0x93, 0x09);
        this.leds.RightOrientation2 = new Led("RightOrientation2", 0x93, 0x0a);
        this.leds.RightOrientation3 = new Led("RightOrientation3", 0x93, 0x0b);
        this.leds.RightOrientation4 = new Led("RightOrientation4", 0x93, 0x0c);
        this.leds.M1Load = new Led("M1Load", 0x93, 0x01);
        this.leds.M1Pfl = new Led("M1Pfl", 0x93, 0x0D);
        this.leds.M2Load = new Led("M2Load", 0x93, 0x02);
        this.leds.M2Pfl = new Led("M2Pfl", 0x93, 0x0E);
        this.leds.M3Load = new Led("M3Load", 0x93, 0x03);
        this.leds.M3Pfl = new Led("M3Pfl", 0x93, 0x0F);
        this.leds.M4Load = new Led("M4Load", 0x93, 0x04);
        this.leds.M4Pfl = new Led("M4Pfl", 0x93, 0x10);

        // Sets soft takeover of linear controls the avoid sudden changes on start
        engine.softTakeover("[Master]", "crossfader", true);
        engine.softTakeover("[Master]", "headVolume", true);
        engine.softTakeover("[Master]", "headMix", true);
        engine.softTakeover("[Master]", "volume", true);
        engine.softTakeover("[Channel1]", "volume", true);
        engine.softTakeover("[Channel1]", "pregain", true);
        engine.softTakeover("[Channel1]", "rate", true);
        engine.softTakeover("[Channel1]", "filterHigh", true);
        engine.softTakeover("[Channel1]", "filterMid", true);
        engine.softTakeover("[Channel1]", "filterLow", true);
        engine.softTakeover("[Channel2]", "volume", true);
        engine.softTakeover("[Channel2]", "pregain", true);
        engine.softTakeover("[Channel2]", "rate", true);
        engine.softTakeover("[Channel2]", "filterHigh", true);
        engine.softTakeover("[Channel2]", "filterMid", true);
        engine.softTakeover("[Channel2]", "filterLow", true);
        engine.softTakeover("[Channel3]", "volume", true);
        engine.softTakeover("[Channel3]", "pregain", true);
        engine.softTakeover("[Channel3]", "rate", true);
        engine.softTakeover("[Channel3]", "filterHigh", true);
        engine.softTakeover("[Channel3]", "filterMid", true);
        engine.softTakeover("[Channel3]", "filterLow", true);
        engine.softTakeover("[Channel4]", "volume", true);
        engine.softTakeover("[Channel4]", "pregain", true);
        engine.softTakeover("[Channel4]", "rate", true);
        engine.softTakeover("[Channel4]", "filterHigh", true);
        engine.softTakeover("[Channel4]", "filterMid", true);
        engine.softTakeover("[Channel4]", "filterLow", true);
        // TODO - Refine list of effects controls in softTakeover
        engine.softTakeover("[EffectRack1_EffectUnit1]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit1]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit2]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit2]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit3]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit3]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit4]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit4]", "super1", true);

        // Connects master controls to call backs to allow the controller to
        // visually react to changes in the system and triggers to synchronize
        this.addControllerConnection("[Master]", "VuMeterR", function(value) { midi.sendShortMsg(0xB3, 0x18, value*7); });
        this.addControllerConnection("[Master]", "VuMeterL", function(value) { midi.sendShortMsg(0xB3, 0x19, value*7); });
        this.addControllerConnection("[Channel1]", "VuMeter", function(value) { midi.sendShortMsg(0xB3, 0x14, value*5); });
        this.addControllerConnection("[Channel2]", "VuMeter", function(value) { midi.sendShortMsg(0xB3, 0x15, value*4); });
        this.addControllerConnection("[Channel3]", "VuMeter", function(value) { midi.sendShortMsg(0xB3, 0x16, value*5); });
        this.addControllerConnection("[Channel4]", "VuMeter", function(value) { midi.sendShortMsg(0xB3, 0x17, value*5); });
        this.addControllerConnection("[Channel1]", "track_samples", this.cbDeckLoaded);
        this.addControllerConnection("[Channel1]", "pfl", this.cbPfl);
        this.addControllerConnection("[Channel1]", "orientation", this.cbOrientation);
        this.addControllerConnection("[Channel2]", "track_samples", this.cbDeckLoaded);
        this.addControllerConnection("[Channel2]", "pfl", this.cbPfl);
        this.addControllerConnection("[Channel2]", "orientation", this.cbOrientation);
        this.addControllerConnection("[Channel3]", "track_samples", this.cbDeckLoaded);
        this.addControllerConnection("[Channel3]", "pfl", this.cbPfl);
        this.addControllerConnection("[Channel3]", "orientation", this.cbOrientation);
        this.addControllerConnection("[Channel4]", "track_samples", this.cbDeckLoaded);
        this.addControllerConnection("[Channel4]", "pfl", this.cbPfl);
        this.addControllerConnection("[Channel4]", "orientation", this.cbOrientation);

        // Creating the deck objects.
        this.addDeck(1);
        this.addDeck(2);
        this.addDeck(3);
        this.addDeck(4);

        // Sets the active decks by default
        this.decks[1].activate(1);
        this.decks[2].activate(1);
        this.decks[3].activate(0);
        this.decks[4].activate(0);

        // Refresh decks to ensure proper control defined
        this.refreshDecks();

        print("g4v::init - Finished");
    };

    // Shutdowns the controller
    this.shutdown = function() {
        print("g4v::shutdown - Started");

        // Turn off all controller lights at shutdown.
        var myObj = this;
        Object.keys(this.leds).forEach(function(element) { myObj.leds[element].set(0); });

        // Turn off all decks
        Object.keys(this.decks).forEach(function(element) { myObj.decks[element].shutdown(); });


        print("g4v::shutdown - Finished");
    };

    /////////////////////////////////////////////////////////////////////
    // Call back functions for connecting Mixxx controls to controller //
    /////////////////////////////////////////////////////////////////////

    // Mixxx manages the Deck loaded leds
    // Parameter control can be disregarded, group identifies the deck and value
    // 0 means unload, otherwise load
    this.cbDeckLoaded = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbDeckLoaded - group:" + group + " control:" + control + " value:" + value);
        }

        var sample = group.substring(8, 9);

        this.leds["M" + sample + "Load"].set((value === 0) ? 0 : 1);
    };

    // When Headphone Cue is activated or deactivated
    // Parameter control can be disregarded, group identifies the deck and value
    // indicates on or off.
    this.cbPfl = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbPfl - group" + group + " control:" + control + " value:" + value);
        }

        var deck = group.substring(8, 9);

        this.leds["M" + deck + "Pfl"].set(value);
    };

    // When channel orientation changes
    // Parameter control can be disregarded, group identifies the deck and value
    // indicates on or off.
    this.cbOrientation = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbOrientation - group" + group + " control:" + control + " value:" + value);
        }

        var deck = group.substring(8, 9);

        g4v.leds["LeftOrientation"+deck].set((value === 0 ? 1 : 0));
        g4v.leds["RightOrientation"+deck].set((value === 2 ? 1 : 0));
    };

    ////////////////////////////////////////////////////////////////
    // Main Dispatcher                                            //
    // Receives MID events and dispatches the right event handler //
    ////////////////////////////////////////////////////////////////
    this.evDispatch = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evDispatch - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Selects the right event to trigger (Check documentation for midi values)
        if (status === 0x90 || status === 0x91) {
            // Calculates the deck channel
            if (midino === 0x01) {
                g4v.decks[g4v.aDecks[channel]].evBtnPlay(channel, midino, value, status, group);
            } else if (midino === 0x02) {
                g4v.decks[g4v.aDecks[channel]].evBtnCue(channel, midino, value, status, group);
            } else if (midino === 0x03) {
                g4v.decks[g4v.aDecks[channel]].evBtnCup(channel, midino, value, status, group);
            } else if (midino === 0x04) {
                g4v.decks[g4v.aDecks[channel]].evBtnSync(channel, midino, value, status, group);
            } else if (midino === 0x05) {
                g4v.decks[g4v.aDecks[channel]].evBtnKey(channel, midino, value, status, group);
            } else if (midino >= 0x09 && midino <= 0x18) {
                g4v.decks[g4v.aDecks[channel]].evBtnPad(channel, midino, value, status, group);
            } else if (midino === 0x19) {
                g4v.decks[g4v.aDecks[channel]].evBtnSlip(channel, midino, value, status, group);
            } else if (midino === 0x1a) {
                g4v.decks[g4v.aDecks[channel]].evBtnFx(channel, midino, value, status, group);
            } else if (midino === 0x1b) {
                g4v.decks[g4v.aDecks[channel]].evBtnHotCue(channel, midino, value, status, group);
            } else if (midino === 0x1c) {
                g4v.decks[g4v.aDecks[channel]].evBtnAutoLoop(channel, midino, value, status, group);
            } else if (midino === 0x1d) {
                g4v.decks[g4v.aDecks[channel]].evBtnSample(channel, midino, value, status, group);
            } else if (midino === 0x1e) {
                g4v.decks[g4v.aDecks[channel]].evBtnLoopRoll(channel, midino, value, status, group);
            } else if (midino === 0x21) {
                g4v.decks[g4v.aDecks[channel]].evBtnBeatJump(channel, midino, value, status, group);
            } else if (midino === 0x22) {
                g4v.decks[g4v.aDecks[channel]].evBtnManualLoop(channel, midino, value, status, group);
            } else if (midino === 0x25) {
                g4v.decks[g4v.aDecks[channel]].evBtnJog(channel, midino, value, status, group);
            } else if (midino === 0x26) {
                // Direct Action: Swap deck
                if (value === 0x00) {
                    return;
                }
                g4v.decks[g4v.aDecks[channel]].activate(0);
                switch (g4v.aDecks[channel]) {
                case 1:
                    g4v.aDecks[channel] = 3;
                    break;
                case 2:
                    g4v.aDecks[channel] = 4;
                    break;
                case 3:
                    g4v.aDecks[channel] = 1;
                    break;
                case 4:
                    g4v.aDecks[channel] = 2;
                    break;
                }
                g4v.decks[g4v.aDecks[channel]].activate(1);
            } else if (midino === 0x27) {
                g4v.decks[g4v.aDecks[channel]].evBtnFxSelect(channel, midino, value, status, group);
            } else if (midino === 0x28) {
                g4v.evBtnShift(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if (status === 0x93) {
            if (midino === 0x01 || midino === 0x02 || midino === 0x03 || midino === 0x04) {
                g4v.evBtnLoad(channel, midino, value, status, group);
            } else if (midino >= 0x05 && midino <= 0x0c) {
                g4v.evBtnOrientation(channel, midino, value, status, group);
            } else if (midino === 0x11) {
                g4v.evBtnBrowse(channel, midino, value, status, group);
            } else if (midino === 0x12) {
                g4v.evBtnBack(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if (status === 0xb0 || status === 0xb1) {
            if (midino === 0x01) {
                g4v.decks[g4v.aDecks[channel]].evSliTempo(channel, midino, value, status, group);
            } else if (midino === 0x02) {
                g4v.decks[g4v.aDecks[channel]].evKnoLoopMove(channel, midino, value, status, group);
            } else if (midino === 0x03) {
                g4v.decks[g4v.aDecks[channel]].evKnoSampleVol(channel, midino, value, status, group);
            } else if (midino === 0x04) {
                g4v.decks[g4v.aDecks[channel]].evKnoMix(channel, midino, value, status, group);
            } else if (midino === 0x05) {
                g4v.decks[g4v.aDecks[channel]].evKnoMeta(channel, midino, value, status, group);
            } else if (midino === 0x06) {
                g4v.decks[g4v.aDecks[channel]].evEncJog(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if (status === 0xb3) {
            if (midino === 0x1e) {
                g4v.evEncBrowse(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else {
            print("WARNING: No handler for status:" + status);
        }
    };


    //////////////////////////////////////////////////////////////////
    // Call back functions for reacting to global controller events //
    //////////////////////////////////////////////////////////////////

    // Shift button 0x90/0x28 - 0x80/0x28
    // Shift keys changing the meaning of the controls
    // Has no direct mapping to Mixx control just changes internal status
    this.evBtnShift = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnShift - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        g4v.shift = ((value === "0x7f") ? 1 : 0);
    };

    // Load button 0x93/0x01 - 0x93/0x02 - 0x93/0x3 - 0x93/0x4
    // Simple: Loads currently selected track
    // Shift: Unloads the deck
    this.evBtnLoad = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnLoad - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        if (value === 0x00) {
            return;
        }

        if (g4v.shift) {
            engine.setValue(group, "eject", 1);
            engine.setValue(group, "eject", 0);
        } else {
            engine.setValue(group, "LoadSelectedTrack", 1);
        }
    };

    // Orientation buttons 0x93/0x05 - 0x93/0x06 - 0x93/0x07 - 0x93/0x08 - 0x93/0x09 - 0x93/0x0a - 0x93/0x0b - 0x93/0x0c
    // Simple: Sets orientation
    this.evBtnOrientation = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnOrientation - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Ignores key release
        if (value === 0x00) {
            return;
        }

        var deck;
        var orientation;

        // Calculates the orientation and the deck
        switch (midino) {
        case 0x05:    // Left 1
            deck = 1;
            orientation = 0;
            break;
        case 0x06:    // Left 2
            deck = 2;
            orientation = 0;
            break;
        case 0x07:    // Left 3
            deck = 3;
            orientation = 0;
            break;
        case 0x08:    // Left 4
            deck = 4;
            orientation = 0;
            break;
        case 0x09:    // Right 1
            deck = 1;
            orientation = 2;
            break;
        case 0x0a:    // Right 2
            deck = 2;
            orientation = 2;
            break;
        case 0x0b:    // Right 3
            deck = 3;
            orientation = 2;
            break;
        case 0x0c:    // Right 4
            deck = 4;
            orientation = 2;
            break;
        }
        // Applies the change
        engine.setValue("[Channel"+deck+"]", "orientation", (engine.getValue("[Channel"+deck+"]", "orientation") === orientation ? 1 : orientation));
    };

    // Deck Select buttons 0x90/0x26 - 0x91/0x26
    // Simple: Selects deck
    //
    // Swaps between decks
    this.evBtnDeckSel = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnDeckSel - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Ignores off event
        if (value === 0x00) {
            return;
        }

        // Calculates the decks
        var oldDeck = this.aDecks[channel-1];
        var newDeck = (oldDeck === channel ? channel+1 : channel);

        this.decks[oldDeck].activate(0);
        this.decks[newDeck].activate(1);
    };

    // Browse button 0x93/0x11
    // Simple: Loads track on preview
    this.evBtnBrowse = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnBrowse - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Ignores off event
        if (value === 0x00) {
            return;
        }

        if (engine.getValue("[PreviewDeck1]", "play", 1)) {
            engine.setValue("[PreviewDeck1]", "play", 0);
            engine.setValue("[PreviewDeck]", "show_previewdeck", 0);
        } else {
            engine.setValue("[PreviewDeck]", "show_previewdeck", 1);
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrack", 1);
            //dealy needed to
            engine.beginTimer(100, "engine.setValue(\"[PreviewDeck1]\",\"play\", 1)", true);
        }
    };

    // Browse button 0x93/0x12
    // Simple: Changes browse mode
    this.evBtnBack = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnBack - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        if (value === 0x00) {
            return;
        }

        this.browseMode = (value === 0x7f);

        engine.setValue("[Library]", "MoveFocusForward", 1);
    };

    // Browse Encode 0xb3/0x1e
    // Simple: Browses library
    // Shit: Browses lists
    this.evEncBrowse = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evEncBrowse - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // If playing preview, beatjump. If not, scroll the Library
        if (engine.getValue("[PreviewDeck1]", "play")) {
            engine.setValue("[PreviewDeck1]", (value === 0x41 ? "beatjump_4_forward" : "beatjump_4_backward"), 1);
        } else {
            if (g4v.shift) {
                engine.setValue("[Library]", (value === 0x41 ? "MoveRight" : "MoveLeft"), 1);
            } else {
                engine.setValue("[Library]", (value === 0x41 ? "MoveUp" : "MoveDown"), 1);
            }
        }
    };
};

// Creates Object for Mixxx
try {
    var g4v = new MyController();
} catch (e) {
    print("Exception creating G4V object: "+e);
}

=======
////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 
/**
 * Gemini G4V controller script
 * 
 * For Mixxx 2.0.0+
 * Written by Javier Vilarroig 2018
 *
 **/

/*
 * Mapping architecture
 * 
 * Class controller - The full controller
 *     Contains:
 *         - decks
 *         - common controls (not in a deck)
 *         - common leds (not in a deck)
 *         - common levels (not in a deck)
 * Class deck - One deck, including the mixer
 *     Contains:
 *         - deck controls
 *         - deck leds
 *         - deck levels
 * Class Connection - One connection between a control and a call back
 * Class control - One control
 * Class led - One led
 * Class level - One level
 */

print("Loading G4V script");

// Connection Class
// Manages the events connections from Mixxx engine
//
// Properties:
// controlGroup:        Mixxx group name
// controlName:            Mixxx control name
// callbackFunction:    Name of the function to be called when the control is triggered
//
// Methods:
// activate():        Activates the connection between Mixxx and the callback function.
//                    Connection is not built until the connection is activated.
// deactivate():    Deactivates the connection between Mixxx and the callback function.
// refresh():        Refreshes the status of the controller.
var Connection = function(group, control, callback) {
    this.groupName = group;
    this.controlName = control;
    this.callbackFunction = callback;
    this.conn = 0;

    if (g4v.debug) {
        print("Connection::constructor: control:" + group + "/" + control + " callback:" + callback);
    }

    this.activate = function() {
        if (g4v.debug) {
            print("Connection::activate: control:" + this.groupName + "/" + this.controlName);
        }

        this.conn = engine.makeConnection(this.groupName, this.controlName, this.callbackFunction);
        this.conn.trigger();
    };

    this.deactivate = function() {
        if (g4v.debug) {
            print("Connection::deactivate: control:" + this.groupName + "/" + this.controlName);
        }

        if(this.conn !== 0) {
            this.conn.disconnect();
        }
    };

    this.refresh = function() {
        if (g4v.debug) {
            print("Connection::refresh: control:" + this.groupName + "/" + this.controlName);
        }

        if(this.conn !== 0) {
            this.conn.trigger();
        }
    };
};

// Control class for control objects
// Represents Mixxx controls
// This controls are to be attached to Decks or Controllers to allow simple
// management of Mixxx controls.
//
// Properties:
// key:     Control name in Mixxx
// group:     Group associated to the control in Mixxx
//
// Methods:
// set(value):    Assign the value to the control
// get():        Return the value of the control

var Control = function(group, key) {
    this.key = key;
    this.group = group;

    if (g4v.debug) {
        print("Control::Constructor - key:" + key + " group:" + group);
    }

    this.set = function(value) {
        if (g4v.debug) {
            print("Control::set - group:" + group + " key:" + this.key + " value:" + value);
        }
        
        engine.setValue(this.group, this.key, value);
    };

    this.get = function() {
        if (g4v.debug) {
            print("Control::get - key:" + this.key);
        }
        
        return engine.getValue(this.group, this.key);
    };

    this.id = function() {
        if (g4v.debug) {
            print("Control::id - key:" + this.key);
        }
        
        return (this.group + "/" + this.key);
    };
};

// Class for managing leds
//
// Properties:
// name:            Name of the control. Used to find the light in the for the callback
// midiS:            MIDI Status value to interact with the light
// midiD:            MIDI Data to send to the deck
// objStr:            Object name. To be used in the call backs
// lit:                Status of the light
// freq:            Flashing delay, in ms
// flashTimerOn:    Timer to manage the flash on events
// flashTimerOff:    Timer to manage the flash off events
// counter:            Counter for limited flashing. null if no blinking
//
// Methods:
// set(value):        Sets light status.
//                    value: 1-on, 2-off
// flashOn(cycles): Starts blinking.
//                     cycles: Number of cycles before stop flashing. Forever if 0
// flashOff():        Stops flashing. Allows to define if the light must be left on or off
// (P)flashOnceOn:    Sets light to on in the flash cycle
// (P)flashOnceOff:    Sets the light to off in the flash cycle
var Led = function(nameP, midiS, midiD, object) {
    this.name = nameP;
    this.midiStatus = midiS;
    this.midiData = midiD;
    this.objStr = ((object === undefined) ? "g4v.leds." : object)+ nameP;
    this.lit = false;
    this.freq = 600;
    this.flashTimerOn = 0;
    this.flashTimerOff = 0;
    this.counter = null;
    this.debug = g4v.debug;


    if (this.debug) {
        print("Led::Constructor - name:"+nameP+" MIDI(status:0x"+midiS.toString(16)+" data:0x"+midiD.toString(16)+") object:"+object);
    }

    this.set = function(value) {
        if (this.debug) {
            print("Led::set - name:" + this.name + ((value == 1) ? " ON " : " OFF ") + " MIDI(status:0x"+midiS.toString(16)+" data:0x"+midiD.toString(16)+")");
        }
        midi.sendShortMsg(this.midiStatus, this.midiData, ((value == 1) ? 0x7F : 0x00));
        this.lit = value;
    };

    this.flashOnceOn = function() {
        if (this.debug) {
            print("Led::flashOnceOn");
        }
        print(this.objStr);
        this.set(1);
        // Clears the on timer
        this.flashTimerOn = 0;
        // Set the next cycle off
        this.flashTimerOff = engine.beginTimer(this.freq, this.objStr + ".flashOnceOff()", true);
    };

    this.flashOnceOff = function() {
        if (this.debug) {
            print("Led::flashOnceOff");
        }
        this.set(0);
        // Clears the off timer
        this.flashTimerOff = 0;
        // Sets the on timer
        this.flashTimerOn = engine.beginTimer(this.freq, this.objStr + ".flashOnceOn()",true);
        // If there is a counter decrements, if 0, cancels flashing
        if(this.counter !== null) {
            this.counter--;
        }
        if(this.counter == 0) {
            this.counter = null;
            this.flashOff();
        }
    };

    this.flashOff = function() {
        if (this.debug) {
            print("Led::Flash Off - name:" + this.name);
        }
        // Destroys the timers
        if (this.flashTimerOn !== 0) {
            engine.stopTimer(this.flashTimerOn);
            this.flashTimerOn = 0;
        }
        if (this.flashTimerOff !== 0) {
            engine.stopTimer(this.flashTimerOff);
            this.flashTimerOff = 0;
        }
    };

    this.flash = function(flashNo) {
        if (this.debug) {
            print("Led::Flash On - name:" + this.name + " flashNo:" + flashNo);
        }

        // if defined, sets the number of cycles
        if(flashNo !== undefined) {
            this.counter = flashNo;
        }

        // Stop flashing in case the led was already flashing
        this.flashOff();

        // Starts Flash cycle
        this.flashOnceOn();
    };
};

// Deck class
// The deck class holds properties for checking attributes about the deck (e.g.
// whether track loaded, holds the functions for scratching/pitch bending on the
// jog wheel, and has a 'control' array, which holds all the controls (buttons,etc)
// associated with the deck.
//
// ATTENTION - This class must be customized for the controller
//        - List of controls must be customized
//        - List of connections (including the name of the parent controller variable)
//
// Properties:
// deckNum: Deck Number
// group: The channel associated to that deck
// controls: Array of control objects representing the different controls in the deck

// Methods:
// addConnection: Adds a connection to the deck
// addControl: Adds a control to the deck
// isLoaded: Return true if the deck has a track loaded
// refresh: Refreshes the connections to ensure proper synchronization with the controller
// TODO - To finalize documenting the methods

var Deck = function(deckN) {
    // Deck number
    this.deckNum = deckN;
    // Deck group to wich the deck is associated
    this.group = "[Channel" + this.deckNum + "]";
    // Array of controls that are modified by the controller in Mixxx
    this.controls = [];
    // Array of connections between Mixxx and the controller
    this.connections = [];
    // Array of Pad connections between Mixxx and the controller
    this.padConnections = [];
    // Array of leds on the deck
    this.leds = [];
    // Is this deck active?
    this.active = 0;
    // Is this deck scratching?
    this.scratching = false;
    // Last scratch even
    this.lastScratch = null;
    // Scratch Control Timer
    this.scratchTimer = 0;
    // Pad mode 1 = Hot Cue 2 = Auto Loop 3 = Sample 4 = Loop Roll 5 = Manual Loop 6 = Beat Jump
    this.padMode = 1;
    // Loop Move knob position
    this.loopMovePos = 0x41;
    // Is this deck in slip mode?
    this.slip = 0;

    print("Creating Deck " + deckN);

    ////////////////////////
    // Internal functions //
    ////////////////////////
    
    // Returns true if there is a track loaded on the deck
    this.isLoaded = function() {
        return (engine.getValue(this.group, "track_loaded"));
    };

    // Facility function for adding led objects
    this.addLed = function(ID, key) {
        if (g4v.debug) {
            print("Deck.addled - ID:" + ID + " key:" + key.toString(16));
        }

        // Insert the led in leds array
        this.leds[ID]= new Led(ID,((this.deckNum == 1 || this.deckNum == 3) ? 0x90 : 0x91), key, "g4v.decks["+this.deckNum+"].leds.");
    };

    // Facility function for adding control objects
    this.addControl = function(ID, key, group) {
        if (g4v.debug) {
            print("Deck.addControl - ID:" + ID + " key:" + key + " group:" + group);
        }

        // Insert the control in all the decks controls array
        this.controls[ID] = new Control(((group === undefined) ? this.group : group), key);
    };

    // Facility function for adding common connections objects
    this.addConnection = function(control, callback, name, group) {
        if (g4v.debug) {
            print("Deck.addConnection - key:" + control + " callback:" + callback);
        }

        // Insert the control in the deck controls array
        this.connections[((name === undefined) ? control : name)] = new Connection(((group === undefined) ? this.group : group), control, callback);
    };

    // Facility function for adding pad button connections objects
    this.addPadConnection = function(control, callback, name, group) {
        if (g4v.debug) {
            print("Deck.addPadConnection - key:" + control + " callback:" + callback + " name:" + name + " group:" + group);
        }

        // Insert the control in the deck controls array
        this.padConnections[((name === undefined) ? control : name)] = new Connection(((group === undefined) ? this.group : group), control, callback);
    };

    // Refreshes all the deck connections to ensure synchronization with the controlles
    this.refresh = function() {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").refresh");
        }
        
        var names = Object.keys(this.connections);

        // Refreshes all the connected call backs
        for ( var i = 0; i < names.length; i++) {
            this.connections[names[i]].refresh();
        }
        
        // Shows status only for decks 3 and 4
        this.leds.deck_select.set((this.deckNum == 3 || this.deckNum == 4) && this.active);

        // Shows Slip status
        this.leds.slip.set(this.slip);
    };

    // Activates or deactivates the deck
    // Parameters:
    //    flag:    1 - Activates the deck, 0 - deactivates the deck
    this.activate = function(flag) {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").activate - flag:" + flag);
        }

        var names = Object.keys(this.connections);

        // Manages connected callbacks
        for ( var i = 0; i < names.length; i++) {
            var connection = this.connections[names[i]];
            if (flag) {
                connection.activate();
            } else
                connection.deactivate();
        }
        
        // Sets status
        this.active = flag;
        
        // Sets pad status
        this.setPadGroup(this.padMode);
        
        // Refreshes the deck
        this.refresh();
    };
    
    // Deactivates scratching if jog wheel is stopped
    this.stopScratch = function() {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").stopScratch");
        }
        
        print("Stop? "+(Date.now() - this.lastScratch));
        if((Date.now() - this.lastScratch) < 20) {
            return;
        }
        
        engine.scratchDisable(this.deckNum, 1);
        engine.stopTimer(this.scratchTimer);
        this.scratching = false;
        this.scratchTimer = 0;

        if(this.slip == 1) {
            this.controls.slip.set(0);
        }
    };
    
    // Set pad group
    // Parameters:
    //    set - Group to set
    this.setPadGroup = function(set) {
        if (g4v.debug) {
            print("Deck(" + this.deckNum + ").setPadGroup("+set+")");
        }
        
        if(this.parMode == set) {
            return;
        }
        
        // Sets pad mode led Leds
        this.leds.hot_cue.set(set == 1 ? 1 : 0);
        this.leds.auto_loop.set(set == 2 ? 1 : 0);
        this.leds.sample.set(set == 3 ? 1 : 0);
        this.leds.loop_roll.set(set == 4 ? 1 : 0);
        if(set == 5) {
            this.leds.loop_roll.flash();
        } else {
            this.leds.loop_roll.flashOff();
        }
        if(set == 6) {
            this.leds.sample.flash();
        } else {
            this.leds.sample.flashOff();
        }

        // Deactivates old connections
        var sampleGroup;
        if(this.padMode == 1) {
            this.padConnections.hotcue_1_enabled.deactivate();
            this.padConnections.hotcue_2_enabled.deactivate();
            this.padConnections.hotcue_3_enabled.deactivate();
            this.padConnections.hotcue_4_enabled.deactivate();
            this.padConnections.hotcue_5_enabled.deactivate();
            this.padConnections.hotcue_6_enabled.deactivate();
            this.padConnections.hotcue_7_enabled.deactivate();
            this.padConnections.hotcue_8_enabled.deactivate();
        }
        if(this.padMode == 2) {
            this.padConnections["beatloop_0.125_enabled"].deactivate();
            this.padConnections["beatloop_0.25_enabled"].deactivate();
            this.padConnections["beatloop_0.5_enabled"].deactivate();
            this.padConnections.beatloop_1_enabled.deactivate();
            this.padConnections.beatloop_2_enabled.deactivate();
            this.padConnections.beatloop_4_enabled.deactivate();
            this.padConnections.beatloop_8_enabled.deactivate();
            this.padConnections.beatloop_16_enabled.deactivate();
        }
        if(this.padMode == 3) {
            // Hides Sampler
            this.controls.show_samplers.set(0);
            sampleGroup = 1+(this.deckNum-1)*16;
            print("Group:"+sampleGroup);
            this.padConnections["s"+(sampleGroup+0)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+1)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+2)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+3)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+4)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+5)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+6)+"_play_indicator"].deactivate();
            this.padConnections["s"+(sampleGroup+7)+"_play_indicator"].deactivate();
        }
        if(this.padMode == 4) {
            this.padConnections["beatlooproll_0.125_activate"].deactivate();
            this.padConnections["beatlooproll_0.25_activate"].deactivate();
            this.padConnections["beatlooproll_0.5_activate"].deactivate();
            this.padConnections.beatlooproll_1_activate.deactivate();
            this.padConnections.beatlooproll_2_activate.deactivate();
            this.padConnections.beatlooproll_4_activate.deactivate();
            this.padConnections.beatlooproll_8_activate.deactivate();
            this.padConnections.beatlooproll_16_activate.deactivate();
        }
        if(this.padMode == 5) {
            this.padConnections.loop_in.deactivate();
            this.padConnections.loop_out.deactivate();
            this.padConnections.loop_enabled.deactivate();
        }

        // Activates pad connections
        if(set == 1) {    // Hot Cue Mode
            this.padConnections.hotcue_1_enabled.activate();
            this.padConnections.hotcue_2_enabled.activate();
            this.padConnections.hotcue_3_enabled.activate();
            this.padConnections.hotcue_4_enabled.activate();
            this.padConnections.hotcue_5_enabled.activate();
            this.padConnections.hotcue_6_enabled.activate();
            this.padConnections.hotcue_7_enabled.activate();
            this.padConnections.hotcue_8_enabled.activate();
        } 
        if(set == 2) {    // Auto Loop mode
            this.padConnections["beatloop_0.125_enabled"].activate();
            this.padConnections["beatloop_0.25_enabled"].activate();
            this.padConnections["beatloop_0.5_enabled"].activate();
            this.padConnections.beatloop_1_enabled.activate();
            this.padConnections.beatloop_2_enabled.activate();
            this.padConnections.beatloop_4_enabled.activate();
            this.padConnections.beatloop_8_enabled.activate();
            this.padConnections.beatloop_16_enabled.activate();
        } 
        if(set == 3) {    // Samples mode
            // Shows Sampler
            this.controls.show_samplers.set(1);
            sampleGroup = 1+(this.deckNum-1)*16;
            print("Group:"+sampleGroup);
            this.padConnections["s"+(sampleGroup+0)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+1)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+2)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+3)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+4)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+5)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+6)+"_play_indicator"].activate();
            this.padConnections["s"+(sampleGroup+7)+"_play_indicator"].activate();
        } 
        if(set == 4) { // Loop Roll mode
            this.padConnections["beatlooproll_0.125_activate"].activate();
            this.padConnections["beatlooproll_0.25_activate"].activate();
            this.padConnections["beatlooproll_0.5_activate"].activate();
            this.padConnections.beatlooproll_1_activate.activate();
            this.padConnections.beatlooproll_2_activate.activate();
            this.padConnections.beatlooproll_4_activate.activate();
            this.padConnections.beatlooproll_8_activate.activate();
            this.padConnections.beatlooproll_16_activate.activate();
        } 
        if(set == 5) { // Manual loop mode
            this.padConnections.loop_in.activate();
            this.padConnections.loop_out.activate();
            this.padConnections.loop_enabled.activate();
        } 
        if(set == 6) {    // Beat Jump mode
            // Leds off - no connections
            this.leds.pad1.set(0);
            this.leds.pad2.set(0);
            this.leds.pad3.set(0);
            this.leds.pad4.set(0);
            this.leds.pad5.set(0);
            this.leds.pad6.set(0);
            this.leds.pad7.set(0);
            this.leds.pad8.set(0);
        } 

        // Sets pad mode
        this.padMode = set;
    };
    
    // Shutdown the deck
    this.shutdown = function() {
        var myObj = this;
        Object.keys(this.leds).forEach(function(element,key){myObj.leds[element].set(0);});
    };
    
    //////////////////////////////////////////////////////////////////////////////
    // Call back functions for connecting Mixxx controls to controller controls //
    //////////////////////////////////////////////////////////////////////////////
    this.cbPlay = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.play.set(value == 0 ? 0 : 1);
    };
    
    this.cbCue = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.cue.set(value == 0 ? 0 : 1);
    };
    
    this.cbSync = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.sync.set(value == 0 ? 0 : 1);
    };

    this.cbKeylock = function(value, group, control) {
        g4v.decks[group.substring(8, 9)].leds.keylock.set(value == 0 ? 0 : 1);
    };

    this.cbPad1 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad1.set(value == 0 ? 0 : 1);
    };

    this.cbPad2 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad2.set(value == 0 ? 0 : 1);
    };

    this.cbPad3 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad3.set(value == 0 ? 0 : 1);
    };

    this.cbPad4 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad4.set(value == 0 ? 0 : 1);
    };

    this.cbPad5 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad5.set(value == 0 ? 0 : 1);
    };

    this.cbPad6 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad6.set(value == 0 ? 0 : 1);
    };

    this.cbPad7 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad7.set(value == 0 ? 0 : 1);
    };

    this.cbPad8 = function(value, group, control) {
        g4v.decks[(group.substring(1, 2) == "C") ? group.substring(8, 9) : Math.ceil(group.substring(8, (group.substring(9,10) == "]" ? 9 : 10))/16)].leds.pad8.set(value == 0 ? 0 : 1);
    };

    this.cbFx = function(value, group, control) {
        g4v.decks[group.substring(23, 24)].leds.fx.set(value == 0 ? 0 : 1);
    };

    /////////////////////////////////////////////////////
    // Callback for reacting to deck controller events //
    /////////////////////////////////////////////////////
    
    // Play buttons 0x90/0x01 - 0x91/0x01
    // Simple: Starts playing the deck
    this.evBtnPlay = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnPlay - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Toggles control
        this.controls.play.set(this.controls.play.get() == 1 ? 0 : 1);
    };
    
    // Cue buttons 0x90/0x02 - 0x91/0x02
    // Simple: CUE press
    this.evBtnCue = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnCue - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        // Toggles control
        this.controls.cue.set(value == 0x7f ? 1 : 0);
    };

    // Cue buttons 0x90/0x03 - 0x91/0x03
    // Simple: Play track from CUE point
    this.evBtnCup = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnCup - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Toggles control
        this.controls.cup.set(value == 0x7f ? 1 : 0);
    };

    // Sync buttons 0x90/0x04 - 0x91/0x04
    // Simple: Sync button press
    this.evBtnSync = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSync - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // On release, check if less than a second happened since push deactivates sync
        if(value == 0x00) {
            print("Time:"+(Date.now() - this.sync_press_time));
            if((Date.now() - this.sync_press_time) < 1000) {
                this.controls.sync.set(0);
            }
            this.sync_press_time = 0;
            return;
        }

        // Activates sync
        this.controls.sync.set(1);
        
        // Saves time
        this.sync_press_time = Date.now();
    };

    // Key buttons 0x90/0x05 - 0x91/0x05
    // Simple: Toggle Key lock
    this.evBtnKey = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnKey - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Toggles control
        this.controls.keylock.set(this.controls.keylock.get() == 0 ? 1 : 0);
    };

    // Fx buttons 0x90/0x1a - 0x91/0x1a
    // Fx buttons 0x90/0x1a - 0x91/0x1a
    this.evBtnFx = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnFx - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Toggles control
        script.toggleControl("[EffectRack1_EffectUnit"+this.deckNum+"]","enabled");
    };

    // Slip buttons 0x90/0x19 - 0x91/0x19
    // Simple: Toggle Slip
    this.evBtnSlip = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSlip - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Toggles control
        this.slip = (this.slip == 0 ? 1 : 0);

        // Set led
        this.leds.slip.set(this.slip);
    };

    // Jog Wheel buttons 0x90/0x25 - 0x91/0x25
    // Press: Activate scratching
    // Release: If jog wheel is stopped, cancels scratching
    this.evBtnJog = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnJog - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch(value) {
        case 0x7f:
            var intervalsPerRev = 250;
            var rpm = 30+1/3;
            var alpha = (1.0/4);
            var beta = (alpha / 32);
            engine.scratchEnable(this.deckNum, intervalsPerRev, rpm, alpha, beta);
            this.scratching = true;
            if(this.slip == 1) {
                this.controls.slip.set(1);
            }
            break;
        case 0x00:
            if(this.scratchTimer == 0) {
                this.scratchTimer = engine.beginTimer(20, "g4v.decks["+this.deckNum+"].stopScratch()");
            }
            break;
        }
    };

    // Loop Move knob 0xb0/0x02 - 0xb1/0x02
    // Move Loop position
    this.evKnoLoopMove = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoLoopMove - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        this.controls.loop_move.set(value-this.loopMovePos);
        this.loopMovePos = value;
    };

    // Sample Volume knob 0xb0/0x03 - 0xb1/0x03
    // Sets sample volume
    this.evKnoSampleVol = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoSampleVol - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        if(g4v.shift) {
            this.controls.pitch.set(script.absoluteNonLin(value, -6, 0, 6));
        } else {
            for(var i = 1; i < 65 ; i++) {
                engine.setValue("[Sampler"+i+"]","volume",value/0x7f);
            }
        }
    };

    // FX Mix knob 0xb0/0x04 - 0xb1/0x04
    // Select the effect mix leve
    this.evKnoMix = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Deck::evKnoMix - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        var mix = ((value/0x7f)*1);
        engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "mix", mix);
    };

    // FX Meta knob 0xb0/0x05 - 0xb1/0x05
    // Select the effect Meta level
    this.evKnoMeta = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evKnoMeta - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        var mix = ((value/0x7f)*1);
        engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "super1", mix);
    };

    // Jog Wheel encoder 0xb0/0x06 - 0xb1/0x06
    // Scratch or Nuddging
    this.evEncJog = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evEncJog - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch(this.scratching) {
        case true:    // We are scratching, if shift, fast search
            engine.scratchTick(this.deckNum,(value > 0x40 ? 1 : -1)*(g4v.shift == 1 ? 10 : 1.4));
            this.lastScratch = Date.now();
            break;
        case false:    // We are nuddging, pay attention to multiplier to adjust sensitivity
            engine.setValue("[Channel"+this.deckNum+"]","jog",(value > 0x40 ? 1 : -1)*0.2);
            break;
        }
    };

    // Pad Button 0x90/0x09 - 0x90/0x0a - 0x90/0x0b - 0x90/0x0c - 0x90/0x0d - 0x90/0x0e - 0x90/0x0f - 0x90/0x10 - 0x91/0x09 - 0x91/0x0a - 0x91/0x0b - 0x91/0x0c - 0x91/0x0d - 0x91/0x0e - 0x91/0x0f - 0x91/0x10
    // Press: Different functions depending on pad mode
    this.evBtnPad = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnPad - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        switch(this.padMode) {
        case 1:    // Hot Cue
            if(midino >= 0x09 && midino <= 0x10) {
                this.controls["hot_cue_"+(midino-0x08)].set((value == 0x7f ? 1 : 0));
            }
            if(midino >= 0x10 && midino <= 0x18) {
                this.controls["clr_cue_"+(midino-0x10)].set(1);
            }
            break;
        case 2:    // Auto Loop
            // Ignore key off
            if(value == 0x00) {
                break;
            }
            // Sets loop
            switch(midino) {
            case 0x09:
                this.controls["aloop_0.125"].set(1);
                break;
            case 0x0a:
                this.controls["aloop_0.25"].set(1);
                break;
            case 0x0b:
                this.controls["aloop_0.5"].set(1);
                break;
            case 0x0c:
                this.controls.aloop_1.set(1);
                break;
            case 0x0d:
                this.controls.aloop_2.set(1);
                break;
            case 0x0e:
                this.controls.aloop_4.set(1);
                break;
            case 0x0f:
                this.controls.aloop_8.set(1);
                break;
            case 0x10:
                this.controls.aloop_16.set(1);
                break;
            }
            break;
        case 3:    // Sampler
            // Ingnore key off
            if(value == 0x00) {
                break;
            }
            // Triggers sample
            switch(midino) {
            case 0x09:
                this.controls.s1_play.set(1);
                break;
            case 0x0a:
                this.controls.s2_play.set(1);
                break;
            case 0x0b:
                this.controls.s3_play.set(1);
                break;
            case 0x0c:
                this.controls.s4_play.set(1);
                break;
            case 0x0d:
                //this.controls.s1_play.set(1);
                break;
            case 0x0e:
                //this.controls.s1_play.set(1);
                break;
            case 0x0f:
                //this.controls.s1_play.set(1);
                break;
            case 0x10:
                //this.controls.s1_play.set(1);
                break;
            }
            break;
        case 4:    // Loop Roll
            switch(midino) {
            case 0x09:
                this.controls["alooproll_0.125"].set(value == 0 ? 0 : 1);
                break;
            case 0x0a:
                this.controls["alooproll_0.25"].set(value == 0 ? 0 : 1);
                break;
            case 0x0b:
                this.controls["alooproll_0.5"].set(value == 0 ? 0 : 1);
                break;
            case 0x0c:
                this.controls.alooproll_1.set(value == 0 ? 0 : 1);
                break;
            case 0x0d:
                this.controls.alooproll_2.set(value == 0 ? 0 : 1);
                break;
            case 0x0e:
                this.controls.alooproll_4.set(value == 0 ? 0 : 1);
                break;
            case 0x0f:
                this.controls.alooproll_8.set(value == 0 ? 0 : 1);
                break;
            case 0x10:
                this.controls.alooproll_16.set(value == 0 ? 0 : 1);
                break;
            }
            break;
        case 5: // Manual loop
            // Ignore key off
            if(value == 0x00) {
                break;
            }
            switch(midino) {
            case 0x09:
                this.controls.loop_in.set(1);
                break;
            case 0x0a:
                this.controls.loop_out.set(1);
                break;
            case 0x0b:
                this.controls.reloop_exit.set(1);
                break;
            case 0x0d:
                this.controls.loop_halve.set(1);
                break;
            case 0x0e:
                this.controls.loop_double.set(1);
                break;
            case 0x0f:
                this.controls.loop_move.set(-1);
                break;
            case 0x10:
                this.controls.loop_move.set(1);
                break;
            }
            break;
        case 6: // Beat Jump mode
            // Ignore key off
            if(value == 0x00) {
                break;
            }
            switch(midino) {
            case 0x09:
                this.controls["beatjump_0.125_f"].set(value == 0 ? 0 : 1);
                break;
            case 0x0a:
                this.controls["beatjump_0.25_f"].set(value == 0 ? 0 : 1);
                break;
            case 0x0b:
                this.controls["beatjump_0.5_f"].set(value == 0 ? 0 : 1);
                break;
            case 0x0c:
                this.controls.beatjump_1_f.set(value == 0 ? 0 : 1);
                break;
            case 0x0d:
                this.controls.beatjump_2_f.set(value == 0 ? 0 : 1);
                break;
            case 0x0e:
                this.controls.beatjump_4_f.set(value == 0 ? 0 : 1);
                break;
            case 0x0f:
                this.controls.beatjump_8_f.set(value == 0 ? 0 : 1);
                break;
            case 0x10:
                this.controls.beatjump_16_f.set(value == 0 ? 0 : 1);
                break;
            case 0x11:
                this.controls["beatjump_0.125_b"].set(value == 0 ? 0 : 1);
                break;
            case 0x12:
                this.controls["beatjump_0.25_b"].set(value == 0 ? 0 : 1);
                break;
            case 0x13:
                this.controls["beatjump_0.5_b"].set(value == 0 ? 0 : 1);
                break;
            case 0x14:
                this.controls.beatjump_1_b.set(value == 0 ? 0 : 1);
                break;
            case 0x15:
                this.controls.beatjump_2_b.set(value == 0 ? 0 : 1);
                break;
            case 0x16:
                this.controls.beatjump_4_b.set(value == 0 ? 0 : 1);
                break;
            case 0x17:
                this.controls.beatjump_8_b.set(value == 0 ? 0 : 1);
                break;
            case 0x18:
                this.controls.beatjump_16_b.set(value == 0 ? 0 : 1);
                break;
            }
            break;
        }
    };

    // Hot Cue buttons 0x90/0x1b - 0x91/0x1b
    // Simple: Activates Hot Cue pad mode
    this.evBtnHotCue = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnHotCue - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(1);
    };

    // Auto Loop buttons 0x90/0x1C - 0x91/0x1C
    // Simple: Activates Auto Loop pad mode
    this.evBtnAutoLoop = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnAutoLoop - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(2);
    };

    // Sample buttons 0x90/0x1d - 0x91/0x1d
    // Simple: Activates pad Sample Mode
    this.evBtnSample = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnSample - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(3);
    };

    // Loop Roll buttons 0x90/0x1e - 0x91/0x1e
    // Simple: Activates Loop Roll pad mode
    this.evBtnLoopRoll = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnLoopRoll - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(4);
    };

    // Loop Roll buttons 0x90/0x22 - 0x91/0x22
    // Simple: Activates Manual Loop pad mode
    this.evBtnManualLoop = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnManualLoop - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(5);
    };
    
    // Loop Roll buttons 0x90/0x21 - 0x91/0x21
    // Shift: Activates Beat Jump pad mode
    this.evBtnBeatJump = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnBeatJump - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.setPadGroup(6);
    };

    // Fx Select buttons 0x90/0x27 - 0x91/0x27
    // Shift: Selects FX
    this.evBtnFxSelect = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evBtnFxSelect - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }

        // Sets Pad mode
        this.controls.fx_select.set(1);
    };
    // Tempo slider 0xb0/0x01 - 0xB1/0x01
    // Simple: Sets tempo rate
    this.evSliTempo = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("deck::evSliTempo - channel" + channel + " midino:" + midino + " value:" + value + " status:" + status + " group:" + group);
        }

        this.controls.rate.set(-script.absoluteNonLin(value, -1, 0, 1));
    };

    ///////////////////////////
    // Constructs the object //
    ///////////////////////////
    
    // Creates all the leds
    this.addLed("play", 0x01);
    this.addLed("cue", 0x02);
    this.addLed("cup", 0x03);
    this.addLed("sync", 0x04);
    this.addLed("keylock", 0x05);
    this.addLed("bank", 0x07);
    this.addLed("pad1", 0x09);
    this.addLed("pad2", 0x0a);
    this.addLed("pad3", 0x0b);
    this.addLed("pad4", 0x0c);
    this.addLed("pad5", 0x0d);
    this.addLed("pad6", 0x0e);
    this.addLed("pad7", 0x0f);
    this.addLed("pad8", 0x10);
    this.addLed("pad8", 0x10);
    this.addLed("slip", 0x19);
    this.addLed("fx", 0x1a);
    this.addLed("hot_cue", 0x1b);
    this.addLed("auto_loop", 0x1c);
    this.addLed("sample", 0x1d);
    this.addLed("loop_roll", 0x1e);
    this.addLed("scratch", 0x23);
    this.addLed("deck_select", 0x26);
    this.addLed("shift", 0x28);

    // Creates connections to common controls
    this.addConnection("play_indicator", this.cbPlay);
    this.addConnection("cue_indicator", this.cbCue);
    this.addConnection("sync_mode", this.cbSync);
    this.addConnection("keylock", this.cbKeylock);
    this.addConnection("enabled", this.cbFx,"fx","[EffectRack1_EffectUnit"+this.deckNum+"]");
    
    // Creates connections to pad buttons
    this.addPadConnection("hotcue_1_enabled", this.cbPad1);
    this.addPadConnection("hotcue_2_enabled", this.cbPad2);
    this.addPadConnection("hotcue_3_enabled", this.cbPad3);
    this.addPadConnection("hotcue_4_enabled", this.cbPad4);
    this.addPadConnection("hotcue_5_enabled", this.cbPad5);
    this.addPadConnection("hotcue_6_enabled", this.cbPad6);
    this.addPadConnection("hotcue_7_enabled", this.cbPad7);
    this.addPadConnection("hotcue_8_enabled", this.cbPad8);
    this.addPadConnection("beatloop_0.125_enabled", this.cbPad1);
    this.addPadConnection("beatloop_0.25_enabled", this.cbPad2);
    this.addPadConnection("beatloop_0.5_enabled", this.cbPad3);
    this.addPadConnection("beatloop_1_enabled", this.cbPad4);
    this.addPadConnection("beatloop_2_enabled", this.cbPad5);
    this.addPadConnection("beatloop_4_enabled", this.cbPad6);
    this.addPadConnection("beatloop_8_enabled", this.cbPad7);
    this.addPadConnection("beatloop_16_enabled", this.cbPad8);
    this.addPadConnection("beatlooproll_0.125_activate", this.cbPad1);
    this.addPadConnection("beatlooproll_0.25_activate", this.cbPad2);
    this.addPadConnection("beatlooproll_0.5_activate", this.cbPad3);
    this.addPadConnection("beatlooproll_1_activate", this.cbPad4);
    this.addPadConnection("beatlooproll_2_activate", this.cbPad5);
    this.addPadConnection("beatlooproll_4_activate", this.cbPad6);
    this.addPadConnection("beatlooproll_8_activate", this.cbPad7);
    this.addPadConnection("beatlooproll_16_activate", this.cbPad8);
    this.addPadConnection("loop_in", this.cbPad1);
    this.addPadConnection("loop_out", this.cbPad2);
    this.addPadConnection("loop_enabled", this.cbPad3);
    this.addPadConnection("track_samples", this.cbPad1,"s"+(((this.deckNum-1)*16)+1)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+1)+"]");
    this.addPadConnection("track_samples", this.cbPad2,"s"+(((this.deckNum-1)*16)+2)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+2)+"]");
    this.addPadConnection("track_samples", this.cbPad3,"s"+(((this.deckNum-1)*16)+3)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+3)+"]");
    this.addPadConnection("track_samples", this.cbPad4,"s"+(((this.deckNum-1)*16)+4)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+4)+"]");
    this.addPadConnection("track_samples", this.cbPad5,"s"+(((this.deckNum-1)*16)+5)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+5)+"]");
    this.addPadConnection("track_samples", this.cbPad6,"s"+(((this.deckNum-1)*16)+6)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+6)+"]");
    this.addPadConnection("track_samples", this.cbPad7,"s"+(((this.deckNum-1)*16)+7)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+7)+"]");
    this.addPadConnection("track_samples", this.cbPad8,"s"+(((this.deckNum-1)*16)+8)+"_play_indicator","[Sampler"+(((this.deckNum-1)*16)+8)+"]");
    
    // Creates all the controls for the deck
    this.addControl("cue", "cue_default");
    this.addControl("play", "play");
    this.addControl("sync", "sync_enabled");
    this.addControl("sync_master", "sync_master");
    this.addControl("cup", "cue_gotoandplay");
    this.addControl("keylock", "keylock");
    this.addControl("slip", "slip_enabled");
    this.addControl("hot_cue_1", "hotcue_1_activate");
    this.addControl("hot_cue_2", "hotcue_2_activate");
    this.addControl("hot_cue_3", "hotcue_3_activate");
    this.addControl("hot_cue_4", "hotcue_4_activate");
    this.addControl("hot_cue_5", "hotcue_5_activate");
    this.addControl("hot_cue_6", "hotcue_6_activate");
    this.addControl("hot_cue_7", "hotcue_7_activate");
    this.addControl("hot_cue_8", "hotcue_8_activate");
    this.addControl("clr_cue_1", "hotcue_1_clear");
    this.addControl("clr_cue_2", "hotcue_2_clear");
    this.addControl("clr_cue_3", "hotcue_3_clear");
    this.addControl("clr_cue_4", "hotcue_4_clear");
    this.addControl("clr_cue_5", "hotcue_5_clear");
    this.addControl("clr_cue_6", "hotcue_6_clear");
    this.addControl("clr_cue_7", "hotcue_7_clear");
    this.addControl("clr_cue_8", "hotcue_8_clear");
    this.addControl("aloop_0.125", "beatloop_0.125_toggle");
    this.addControl("aloop_0.25", "beatloop_0.25_toggle");
    this.addControl("aloop_0.5", "beatloop_0.5_toggle");
    this.addControl("aloop_1", "beatloop_1_toggle");
    this.addControl("aloop_2", "beatloop_2_toggle");
    this.addControl("aloop_4", "beatloop_4_toggle");
    this.addControl("aloop_8", "beatloop_8_toggle");
    this.addControl("aloop_16", "beatloop_16_toggle");
    this.addControl("aloop_0.125_en", "beatloop_0.125_enabled");
    this.addControl("aloop_0.25_en", "beatloop_0.25_enabled");
    this.addControl("aloop_0.5_en", "beatloop_0.5_enabled");
    this.addControl("aloop_1_en", "beatloop_1_enabled");
    this.addControl("aloop_2_en", "beatloop_2_enabled");
    this.addControl("aloop_4_en", "beatloop_4_enabled");
    this.addControl("aloop_8_en", "beatloop_8_enabled");
    this.addControl("aloop_16_en", "beatloop_16_enabled");
    this.addControl("alooproll_0.125", "beatlooproll_0.125_activate");
    this.addControl("alooproll_0.25", "beatlooproll_0.25_activate");
    this.addControl("alooproll_0.5", "beatlooproll_0.5_activate");
    this.addControl("alooproll_1", "beatlooproll_1_activate");
    this.addControl("alooproll_2", "beatlooproll_2_activate");
    this.addControl("alooproll_4", "beatlooproll_4_activate");
    this.addControl("alooproll_8", "beatlooproll_8_activate");
    this.addControl("alooproll_16", "beatlooproll_16_activate");
    this.addControl("alooproll_0.125_en", "beatlooproll_0.125_activate");
    this.addControl("alooproll_0.25_en", "beatlooproll_0.25_activate");
    this.addControl("alooproll_0.5_en", "beatlooproll_0.5_activate");
    this.addControl("alooproll_1_en", "beatlooproll_1_activate");
    this.addControl("alooproll_2_en", "beatlooproll_2_activate");
    this.addControl("alooproll_4_en", "beatlooproll_4_activate");
    this.addControl("alooproll_8_en", "beatlooproll_8_activate");
    this.addControl("alooproll_16_en", "beatlooproll_16_activate");
    this.addControl("loop_in", "loop_in");
    this.addControl("loop_out", "loop_out");
    this.addControl("reloop_exit", "reloop_exit");
    this.addControl("loop_halve", "loop_halve");
    this.addControl("loop_double", "loop_double");
    this.addControl("beatjump_0.125_f", "beatjump_0.125_forward");
    this.addControl("beatjump_0.25_f", "beatjump_0.25_forward");
    this.addControl("beatjump_0.5_f", "beatjump_0.5_forward");
    this.addControl("beatjump_1_f", "beatjump_1_forward");
    this.addControl("beatjump_2_f", "beatjump_2_forward");
    this.addControl("beatjump_4_f", "beatjump_4_forward");
    this.addControl("beatjump_8_f", "beatjump_8_forward");
    this.addControl("beatjump_16_f", "beatjump_16_forward");
    this.addControl("beatjump_0.125_b", "beatjump_0.125_backward");
    this.addControl("beatjump_0.25_b", "beatjump_0.25_backward");
    this.addControl("beatjump_0.5_b", "beatjump_0.5_backward");
    this.addControl("beatjump_1_b", "beatjump_1_backward");
    this.addControl("beatjump_2_b", "beatjump_2_backward");
    this.addControl("beatjump_4_b", "beatjump_4_backward");
    this.addControl("beatjump_8_b", "beatjump_8_backward");
    this.addControl("beatjump_16_b", "beatjump_16_backward");
    this.addControl("rate", "rate");
    this.addControl("pitch", "pitch");
    this.addControl("loop_move_f", "loop_move_1_forward");
    this.addControl("loop_move_b", "loop_move_1_backward");
    this.addControl("show_samplers", "show_samplers", "[Samplers]");
    this.addControl("s1_play", "start_play", "[Sampler1]");
    this.addControl("s2_play", "start_play", "[Sampler2]");
    this.addControl("s3_play", "start_play", "[Sampler3]");
    this.addControl("s4_play", "start_play", "[Sampler4]");
    this.addControl("fx_select", "next_chain", "[EffectRack1_EffectUnit"+this.deckNum+"]");
    this.addControl("loop_move", "loop_move");
    
    // Applies effect to channel
    engine.setValue("[EffectRack1_EffectUnit"+this.deckNum+"]", "group_[Channel"+this.deckNum+"]_enable",1);

    if (g4v.debug) {
        print("Deck created - DeckNum:" + this.deckNum + " group:" + this.group);
    }
};

// Controller object
var MyController = function() {
    // Set to 1 to debug the class
    this.debug = true;
    // Array of Decks on the controller
    this.decks = [];
    // Array of global controls in the controller (not in a deck)
    this.master = [];
    // Array of global leds (not in a deck)
    this.leds = [];
    // Array of connection objects
    this.connections = [];
    // Shift status
    this.shift = 0;
    // Orientation status
    // 2=left, 0=right, 1 = none
    this.orientation = [2,2,0,0];
    // Active decks (left to right)
    this.aDecks = [1,2];
    // Browse mode false = tracks lists = lists
    this.browseMode = false;
    

    // Facility function for adding control objects
    this.addControl = function(ID, key) {
        if (g4v.debug) {
            print("Controller::addControl - ID:" + ID + " key:" + key);
        }

        // Insert the control in all the decks controls array
        this.master[ID] = new Control("[Master]", key);
    };
    
    // Facility function for adding connection objects
    this.addConnection = function(group, control, func) {
        if (g4v.debug) {
            print("Controller::addConnection - group:" + group + " control:" + control);
        }

        // Insert the control in all the decks controls array
        g4v.connections[group+control] = engine.makeConnection(group, control, func);
        
        // Triggers the connection to sync
        g4v.connections[group+control].trigger();
    };

    // Facility function for adding deck objects
    this.addDeck = function(ID) {
        if (g4v.debug) {
            print("Controller::addDeck - ID:" + ID);
        }

        this.decks[ID] = new Deck(ID);
    };
    
    // Refreshes decks to ensure control synchronization
    this.refreshDecks = function() {
        this.decks[1].refresh();
        this.decks[2].refresh();
        this.decks[3].refresh();
        this.decks[4].refresh();
    };

    // Init the Controller
    this.init = function(id) {
        if (g4v.debug){
            print("Init G4V id:" + id);
        }

        // //////////////////////////////
        // Creates all the led objects //
        // //////////////////////////////
        this.leds.LeftOrientation1 = new Led("LeftOrientation1",0x93,0x05);
        this.leds.LeftOrientation2 = new Led("LeftOrientation2",0x93,0x06);
        this.leds.LeftOrientation3 = new Led("LeftOrientation3",0x93,0x07);
        this.leds.LeftOrientation4 = new Led("LeftOrientation4",0x93,0x08);
        this.leds.RightOrientation1 = new Led("RightOrientation1",0x93,0x09);
        this.leds.RightOrientation2 = new Led("RightOrientation2",0x93,0x0a);
        this.leds.RightOrientation3 = new Led("RightOrientation3",0x93,0x0b);
        this.leds.RightOrientation4 = new Led("RightOrientation4",0x93,0x0c);
        this.leds.M1Load = new Led("M1Load",0x93,0x01);
        this.leds.M1Pfl = new Led("M1Pfl",0x93,0x0D);
        this.leds.M2Load = new Led("M2Load",0x93,0x02);
        this.leds.M2Pfl = new Led("M2Pfl",0x93,0x0E);
        this.leds.M3Load = new Led("M3Load",0x93,0x03);
        this.leds.M3Pfl = new Led("M3Pfl",0x93,0x0F);
        this.leds.M4Load = new Led("M4Load",0x93,0x04);
        this.leds.M4Pfl = new Led("M4Pfl",0x93,0x10);

        // Sets soft takeover of linear controls the avoid sudden changes on start
        engine.softTakeover("[Master]", "crossfader", true);
        engine.softTakeover("[Master]", "headVolume", true);
        engine.softTakeover("[Master]", "headMix", true);
        engine.softTakeover("[Master]", "volume", true);
        engine.softTakeover("[Channel1]", "volume", true);
        engine.softTakeover("[Channel1]", "pregain", true);
        engine.softTakeover("[Channel1]", "rate", true);
        engine.softTakeover("[Channel1]", "filterHigh", true);
        engine.softTakeover("[Channel1]", "filterMid", true);
        engine.softTakeover("[Channel1]", "filterLow", true);
        engine.softTakeover("[Channel2]", "volume", true);
        engine.softTakeover("[Channel2]", "pregain", true);
        engine.softTakeover("[Channel2]", "rate", true);
        engine.softTakeover("[Channel2]", "filterHigh", true);
        engine.softTakeover("[Channel2]", "filterMid", true);
        engine.softTakeover("[Channel2]", "filterLow", true);
        engine.softTakeover("[Channel3]", "volume", true);
        engine.softTakeover("[Channel3]", "pregain", true);
        engine.softTakeover("[Channel3]", "rate", true);
        engine.softTakeover("[Channel3]", "filterHigh", true);
        engine.softTakeover("[Channel3]", "filterMid", true);
        engine.softTakeover("[Channel3]", "filterLow", true);
        engine.softTakeover("[Channel4]", "volume", true);
        engine.softTakeover("[Channel4]", "pregain", true);
        engine.softTakeover("[Channel4]", "rate", true);
        engine.softTakeover("[Channel4]", "filterHigh", true);
        engine.softTakeover("[Channel4]", "filterMid", true);
        engine.softTakeover("[Channel4]", "filterLow", true);
        // TODO - Refine list of effects controls in softTakeover
        engine.softTakeover("[EffectRack1_EffectUnit1]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit1]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit2]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit2]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit3]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit3]", "super1", true);
        engine.softTakeover("[EffectRack1_EffectUnit4]", "mix", true);
        engine.softTakeover("[EffectRack1_EffectUnit4]", "super1", true);

        // Connects master controls to call backs to allow the controller to
        // visually react to changes in the system and triggers to synchronize
        this.addConnection('[Master]', 'VuMeterR', function(value) {midi.sendShortMsg(0xB3, 0x18, value*7);});
        this.addConnection('[Master]', 'VuMeterL', function(value) {midi.sendShortMsg(0xB3, 0x19, value*7);});
        this.addConnection('[Channel1]', 'VuMeter', function(value) {midi.sendShortMsg(0xB3, 0x14, value*5);});
        this.addConnection('[Channel2]', 'VuMeter', function(value) {midi.sendShortMsg(0xB3, 0x15, value*4);});
        this.addConnection('[Channel3]', 'VuMeter', function(value) {midi.sendShortMsg(0xB3, 0x16, value*5);});
        this.addConnection('[Channel4]', 'VuMeter', function(value) {midi.sendShortMsg(0xB3, 0x17, value*5);});
        this.addConnection("[Channel1]", "track_samples", this.cbDeckLoaded);
        this.addConnection("[Channel1]", "pfl", this.cbPfl);
        this.addConnection("[Channel1]", "orientation", this.cbOrientation);
        this.addConnection("[Channel2]", "track_samples", this.cbDeckLoaded);
        this.addConnection("[Channel2]", "pfl", this.cbPfl);
        this.addConnection("[Channel2]", "orientation", this.cbOrientation);
        this.addConnection("[Channel3]", "track_samples", this.cbDeckLoaded);
        this.addConnection("[Channel3]", "pfl", this.cbPfl);
        this.addConnection("[Channel3]", "orientation", this.cbOrientation);
        this.addConnection("[Channel4]", "track_samples", this.cbDeckLoaded);
        this.addConnection("[Channel4]", "pfl", this.cbPfl);
        this.addConnection("[Channel4]", "orientation", this.cbOrientation);

        // Creating the deck objects.
        this.addDeck("1");
        this.addDeck("2");
        this.addDeck("3");
        this.addDeck("4");

        // Sets the active decks by default
        this.decks[1].activate(1);
        this.decks[2].activate(1);
        this.decks[3].activate(0);
        this.decks[4].activate(0);
        
        // Refresh decks to ensure proper control defined
        this.refreshDecks();
        
        print("g4v::init - Finished");
    };

    // Shutdowns the controller
    this.shutdown = function() {
        print("g4v::shutdown - Started");

        // Turn off all controller lights at shutdown.
        var myObj = this;
        Object.keys(this.leds).forEach(function(element){myObj.leds[element].set(0);});
        
        // Turn off all decks
        Object.keys(this.decks).forEach(function(element){myObj.decks[element].shutdown();});
        
        
        print("g4v::shutdown - Finished");
    };

    /////////////////////////////////////////////////////////////////////
    // Call back functions for connecting Mixxx controls to controller //
    /////////////////////////////////////////////////////////////////////

    // Mixxx manages the Deck loaded leds
    // Parameter control can be disregarded, group identifies the deck and value
    // 0 means unload, otherwise load
    this.cbDeckLoaded = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbDeckLoaded - group:" + group + " control:" + control + " value:" + value);
        }

        var sample = group.substring(8, 9);

        this.leds["M" + sample + "Load"].set((value == 0) ? 0 : 1);
    };
    
    // When Headphone Cue is activated or deactivated
    // Parameter control can be disregarded, group identifies the deck and value
    // indicates on or off.
    this.cbPfl = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbPfl - group" + group + " control:" + control + " value:" + value);
        }

        var deck = group.substring(8, 9);

        this.leds["M" + deck + "Pfl"].set(value);
    };

    // When channel orientation changes
    // Parameter control can be disregarded, group identifies the deck and value
    // indicates on or off.
    this.cbOrientation = function(value, group, control) {
        if (g4v.debug) {
            print("Controller.cbOrientation - group" + group + " control:" + control + " value:" + value);
        }

        var deck = group.substring(8, 9);
        
        g4v.leds["LeftOrientation"+deck].set((value == 0 ? 1 : 0));
        g4v.leds["RightOrientation"+deck].set((value == 2 ? 1 : 0));
    };

    ////////////////////////////////////////////////////////////////
    // Main Dispatcher                                            //
    // Receives MID events and dispatches the right event handler //
    ////////////////////////////////////////////////////////////////
    this.evDispatch = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evDispatch - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Selects the right event to trigger (Check documentation for midi values)
        if(status == 0x90 || status == 0x91) {
            // Calculates the deck channel
            if(midino == 0x01) {
                g4v.decks[g4v.aDecks[channel]].evBtnPlay(channel, midino,value,status,group);
            } else if(midino == 0x02) {
                g4v.decks[g4v.aDecks[channel]].evBtnCue(channel, midino,value,status,group);
            } else if(midino == 0x03) {
                g4v.decks[g4v.aDecks[channel]].evBtnCup(channel, midino,value,status,group);
            } else if(midino == 0x04) {
                g4v.decks[g4v.aDecks[channel]].evBtnSync(channel, midino,value,status,group);
            } else if(midino == 0x05) {
                g4v.decks[g4v.aDecks[channel]].evBtnKey(channel, midino,value,status,group);
            } else if(midino >= 0x09 && midino <= 0x18) {
                g4v.decks[g4v.aDecks[channel]].evBtnPad(channel, midino,value,status,group);
            } else if(midino == 0x19) {
                g4v.decks[g4v.aDecks[channel]].evBtnSlip(channel, midino,value,status,group);
            } else if(midino == 0x1a) {
                g4v.decks[g4v.aDecks[channel]].evBtnFx(channel, midino,value,status,group);
            } else if(midino == 0x1b) {
                g4v.decks[g4v.aDecks[channel]].evBtnHotCue(channel, midino,value,status,group);
            } else if(midino == 0x1c) {
                g4v.decks[g4v.aDecks[channel]].evBtnAutoLoop(channel, midino,value,status,group);
            } else if(midino == 0x1d) {
                g4v.decks[g4v.aDecks[channel]].evBtnSample(channel, midino,value,status,group);
            } else if(midino == 0x1e) {
                g4v.decks[g4v.aDecks[channel]].evBtnLoopRoll(channel, midino,value,status,group);
            } else if(midino == 0x21) {
                g4v.decks[g4v.aDecks[channel]].evBtnBeatJump(channel, midino,value,status,group);
            } else if(midino == 0x22) {
                g4v.decks[g4v.aDecks[channel]].evBtnManualLoop(channel, midino,value,status,group);
            } else if(midino == 0x25) {
                g4v.decks[g4v.aDecks[channel]].evBtnJog(channel, midino,value,status,group);
            } else if(midino == 0x26) {
                // Direct Action: Swap deck
                if(value == 0x00) {
                    return;
                }
                g4v.decks[g4v.aDecks[channel]].activate(0);
                switch(g4v.aDecks[channel]) {
                case 1:
                    g4v.aDecks[channel] = 3;
                    break;
                case 2:
                    g4v.aDecks[channel] = 4;
                    break;
                case 3:
                    g4v.aDecks[channel] = 1;
                    break;
                case 4:
                    g4v.aDecks[channel] = 2;
                    break;
                }
                g4v.decks[g4v.aDecks[channel]].activate(1);
            } else if(midino == 0x27) {
                g4v.decks[g4v.aDecks[channel]].evBtnFxSelect(channel, midino,value,status,group);
            } else if(midino == 0x28) {
                g4v.evBtnShift(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if(status == 0x93) {
            if(midino == 0x01 || midino == 0x02 || midino == 0x03 || midino == 0x04) {
                g4v.evBtnLoad(channel, midino, value, status, group);
            } else if(midino >= 0x05 && midino <= 0x0c) {
                g4v.evBtnOrientation(channel, midino, value, status, group);
            } else if (midino == 0x11) {
                g4v.evBtnBrowse(channel, midino, value, status, group);
            } else if (midino == 0x12) {
                g4v.evBtnBack(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if(status == 0xb0 || status == 0xb1) {
            if(midino == 0x01) {
                g4v.decks[g4v.aDecks[channel]].evSliTempo(channel, midino, value, status, group);
            } else if(midino == 0x02) {
                g4v.decks[g4v.aDecks[channel]].evKnoLoopMove(channel, midino, value, status, group);
            } else if(midino == 0x03) {
                g4v.decks[g4v.aDecks[channel]].evKnoSampleVol(channel, midino, value, status, group);
            } else if(midino == 0x04) {
                g4v.decks[g4v.aDecks[channel]].evKnoMix(channel, midino, value, status, group);
            } else if(midino == 0x05) {
                g4v.decks[g4v.aDecks[channel]].evKnoMeta(channel, midino, value, status, group);
            } else if(midino == 0x06) {
                g4v.decks[g4v.aDecks[channel]].evEncJog(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else if (status == 0xb3) {
            if(midino == 0x1e) {
                g4v.evEncBrowse(channel, midino, value, status, group);
            } else {
                print("WARNING: No handler for status/midino:" + status + "/" + midino);
            }
        } else {
            print("WARNING: No handler for status:" + status);
        }
    };

    
    //////////////////////////////////////////////////////////////////
    // Call back functions for reacting to global controller events //
    //////////////////////////////////////////////////////////////////

    // Shift button 0x90/0x28 - 0x80/0x28
    // Shift keys changing the meaning of the controls
    // Has no direct mapping to Mixx control just changes internal status
    this.evBtnShift = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnShift - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        g4v.shift = ((value == "0x7f") ? 1 : 0);
    };

    // Load button 0x93/0x01 - 0x93/0x02 - 0x93/0x3 - 0x93/0x4
    // Simple: Loads currently selected track
    // Shift: Unloads the deck
    this.evBtnLoad = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnLoad - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }
        
        if(value == 0x00) {
            return;
        }

        if(g4v.shift){
            engine.setValue(group, "eject", 1);
            engine.setValue(group, "eject", 0);
        } else {
            engine.setValue(group, "LoadSelectedTrack", 1);
        }
    };

    // Orientation buttons 0x93/0x05 - 0x93/0x06 - 0x93/0x07 - 0x93/0x08 - 0x93/0x09 - 0x93/0x0a - 0x93/0x0b - 0x93/0x0c
    // Simple: Sets orientation
    this.evBtnOrientation = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnOrientation - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }
        
        // Ignores key release
        if(value == 0x00) {
            return;
        }
        
        var deck;
        var orientation;

        // Calculates the orientation and the deck
        switch(midino) {
        case 0x05:    // Left 1
            deck = 1;
            orientation = 0;
            break;
        case 0x06:    // Left 2
            deck = 2;
            orientation = 0;
            break;
        case 0x07:    // Left 3
            deck = 3;
            orientation = 0;
            break;
        case 0x08:    // Left 4
            deck = 4;
            orientation = 0;
            break;
        case 0x09:    // Right 1
            deck = 1;
            orientation = 2;
            break;
        case 0x0a:    // Right 2
            deck = 2;
            orientation = 2;
            break;
        case 0x0b:    // Right 3
            deck = 3;
            orientation = 2;
            break;
        case 0x0c:    // Right 4
            deck = 4;
            orientation = 2;
            break;
        }
        // Applies the change
        engine.setValue("[Channel"+deck+"]","orientation",(engine.getValue("[Channel"+deck+"]","orientation") == orientation ? 1 : orientation));
    };

    // Deck Select buttons 0x90/0x26 - 0x91/0x26
    // Simple: Selects deck
    //
    // Swaps between decks
    this.evBtnDeckSel = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnDeckSel - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Ignores off event
        if(value == 0x00) {
            return;
        }

        // Calculates the decks
        var oldDeck = this.aDecks[channel-1];
        var newDeck = (oldDeck == channel ? channel+1 : channel);
        
        this.decks[oldDeck].activate(0);
        this.decks[newDeck].activate(1);
    };

    // Browse button 0x93/0x11
    // Simple: Loads track on preview
    this.evBtnBrowse = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnBrowse - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // Ignores off event
        if(value == 0x00) {
            return;
        }

        if(engine.getValue("[PreviewDeck1]", "play", 1)) {
            engine.setValue("[PreviewDeck1]","play", 0);
            engine.setValue("[PreviewDeck]","show_previewdeck", 0 );
        } else {
            engine.setValue("[PreviewDeck]","show_previewdeck", 1 );
            engine.setValue("[PreviewDeck1]","LoadSelectedTrack", 1);
            //dealy needed to
            engine.beginTimer(100, "engine.setValue(\"[PreviewDeck1]\",\"play\", 1)", true);
        }
    };

    // Browse button 0x93/0x12
    // Simple: Changes browse mode
    this.evBtnBack = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evBtnBack - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }
        
        if(value == 0x00) {
            return;
        }

        this.browseMode = (value == 0x7f ? true : false);
        
        engine.setValue("[Library]","MoveFocusForward", 1);
    };

    // Browse Encode 0xb3/0x1e
    // Simple: Browses library
    // Shit: Browses lists
    this.evEncBrowse = function(channel, midino, value, status, group) {
        if (g4v.debug) {
            print("Controller.evEncBrowse - channel:" + channel + " midino:0x" + midino.toString(16) + " value:0x" + value.toString(16) + " status:0x" + status.toString(16) + " group:" + group);
        }

        // If playing preview, beatjump. If not, scroll the Library
        if(engine.getValue("[PreviewDeck1]", "play")) {
            engine.setValue("[PreviewDeck1]", (value == 0x41 ? "beatjump_4_forward" : "beatjump_4_backward"), 1);
        } else {
            if(g4v.shift) {
                engine.setValue("[Library]",(value == 0x41 ? "MoveRight" : "MoveLeft"), 1);
            } else {
                engine.setValue("[Library]",(value == 0x41 ? "MoveUp" : "MoveDown"), 1);
            }
        }
    };
};

// Creates Object for Mixxx
try {
    var g4v = new MyController();
} catch (e) {
    print("Exception creating G4V object: "+e);
}

>>>>>>> a34c8ab3a6e5580bbe8b910b1311236410b078fb
print("G4V Load finished");