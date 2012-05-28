// Functions common to all controllers go in this file

nop = function () {}    // Only here so you don't get a syntax error on load if the file was otherwise empty

// ----------------- Prototype enhancements ---------------------

// Returns an ASCII byte array for the string
String.prototype.toInt = function() {
    var a = new Array();
    for (var i = 0; i < this.length; i++) {
        a[i] = this.charCodeAt(i);
    }
    return a;
}

// ----------------- Function overloads ---------------------

// Causes script print() calls to appear in the log file as well
print = function(string) {
	engine.log(string);
}

// ----------------- Generic functions ---------------------

function secondstominutes(secs)
{
   var m = (secs / 60) | 0;

   return (m < 10 ? "0" + m : m) 
          + ":"
          + ( ( secs %= 60 ) < 10 ? "0" + secs : secs);
}

function msecondstominutes(msecs)
{
    var m = (msecs / 60000) | 0;
    msecs %= 60000;
    var secs = (msecs / 1000) | 0;
    msecs %= 1000;
    msecs = Math.round(msecs * 100 / 1000);
    if (msecs==100) msecs=99;
    
//     print("secs="+secs+", msecs="+msecs);

    return (m < 10 ? "0" + m : m) 
        + ":"
        + ( secs < 10 ? "0" + secs : secs )
        + "."
        + ( msecs < 10 ? "0" + msecs : msecs);
}

function script() {}

// DEPRECATED -- use script.midiDebug() instead
script.debug = function (channel, control, value, status, group) {
    script.midiDebug(channel, control, value, status, group);
}

script.midiDebug = function (channel, control, value, status, group) {
    print("Script.midiDebug - channel: 0x" + channel.toString(16) +
          " control: 0x" + control.toString(16) + " value: 0x" + value.toString(16) +
          " status: 0x" + status.toString(16) + " group: " + group);
}

// DEPRECATED -- use script.absoluteLin() instead
script.absoluteSlider = function (group, key, value, low, high, min, max) {
    engine.setValue(group, key, script.absoluteLin(value, low, high, min, max));
}

// Returns a value for a linear Mixxx control (like Volume: 0..1) from an absolute control
script.absoluteLin = function (value, low, high, min, max) {
    if (!min) min = 0;
    if (!max) max = 127;
    if (value==max) return high;
    else return ((((high - low) / max) * value) + low);
}

// Returns a value for a non-linear Mixxx control (like EQs: 0..1..4) from an absolute control
script.absoluteNonLin = function (value, low, mid, high, min, max) {
    if (!min) min = 0;
    if (!max) max = 127;
    var center = (max-min)/2;
    if (value==center || value==Math.round(center))
        return mid;
    if (value<center)
        return low+(value/(center/(mid-low)));
    return mid+((value-center)/(center/(high-mid)));
}

script.pitch = function (LSB, MSB, status) {
    return script.midiPitch(LSB, MSB, status);
}

script.crossfaderCurve = function (value, min, max) {
    if (engine.getValue("[Mixer Profile]", "xFaderMode")==1) {
        // Constant Power
        engine.setValue("[Mixer Profile]", "xFaderCalibration", script.absoluteLin(value, 0.5, 0.962, min, max));
    } else {
        // Additive
        engine.setValue("[Mixer Profile]", "xFaderCurve", script.absoluteLin(value, 1, 2, min, max));
    }
}

/* -------- ------------------------------------------------------
     script.midiPitch
   Purpose: Takes the value from a little-endian 14-bit MIDI pitch
            wheel message and returns the value for a "rate" (pitch
            slider) Mixxx control
   Input:   Least significant byte, most sig. byte, MIDI status byte
   Output:  Value for a "rate" control, or false if the input MIDI
            message was not a Pitch message (0xE#)
   -------- ------------------------------------------------------ */
script.midiPitch = function (LSB, MSB, status) {
    if ((status & 0xF0) != 0xE0) {  // Mask the upper nybble so we can check the opcode regardless of the channel
        print("Script.midiPitch: Error, not a MIDI pitch (0xEn) message: "+status);
        return false;
    }
    var value = (MSB << 7) | LSB;  // Construct the 14-bit number
    // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
    var rate = (value-8192)/8191;
//     print("Script.Pitch: MSB="+MSB+", LSB="+LSB+", value="+value+", rate="+rate);
    return rate;
}

/* -------- ------------------------------------------------------
     script.spinbackDefault
   Purpose: wrapper around spinback() that can be directly mapped 
            from xml for a spinback effect
   Input:   channel, control, value, status, group
   Output:  none
   -------- ------------------------------------------------------ */
script.spinbackDefault = function(channel, control, value, status, group) {
	// disable on note-off or zero value note/cc
    script.spinback(group, ((status & 0xF0) != 0x80 && value > 0));
}

/* -------- ------------------------------------------------------
     script.brakeDefault
   Purpose: wrapper around brake() that can be directly mapped 
            from xml for a brake effect
   Input:   channel, control, value, status, group
   Output:  none
   -------- ------------------------------------------------------ */
script.brakeDefault = function(channel, control, value, status, group) {
	// disable on note-off or zero value note/cc
    script.brake(group, ((status & 0xF0) != 0x80 && value > 0));
}

/* -------- ------------------------------------------------------
     script.spinback
   Purpose: Activate or disable a spinback effect on the chosen deck
   Input:   group, enable/disable, [delay], [factor], [inital rate]
   Output:  None
   -------- ------------------------------------------------------ */
script.spinback = function(group, activate, factor, rate, delay) {
    if (factor == undefined) factor = 0.8;
    if (rate == undefined) rate = -10;
    if (delay == undefined) delay = 5;
    script.deckSpinbackBrake(group, activate, factor, rate, delay);
}

/* -------- ------------------------------------------------------
     script.brake
   Purpose: Activate or disable a brake effect on the chosen deck
   Input:   group, enable/disable, [delay], [factor], [inital rate]
   Output:  None
   -------- ------------------------------------------------------ */
script.brake = function(group, activate, factor, rate, delay) {
    if (factor == undefined) factor = 0.95;
    if (rate == undefined) rate = 1;
    if (delay == undefined) delay = 0;
    script.deckSpinbackBrake(group, activate, factor, rate, delay);
}

script.deckSpinbackBrakeData = {};

script.deckSpinbackBrake = function(group, activate, factor, rate, delay) {

    if (activate != undefined) {

        // store the current settings

        if (script.deckSpinbackBrakeData[group] == undefined) {
            script.deckSpinbackBrakeData[group] = { timer: null, delay: delay, factor: factor, rate: rate };
        }
        else {
            script.deckSpinbackBrakeData[group].delay = delay;
            script.deckSpinbackBrakeData[group].factor = factor;
            script.deckSpinbackBrakeData[group].rate = rate;
        }

        // kill timer when both enabling or disabling

        if (script.deckSpinbackBrakeData[group].timer != null) {
            engine.stopTimer(script.deckSpinbackBrakeData[group].timer);
            script.deckSpinbackBrakeData[group].timer = null;
        }

        // enable/disable scratch2 mode

        engine.setValue(group, 'scratch2_enable', activate ? 1 : 0);

        if (activate) {
            // save keylock status and disable it
            if ((script.deckSpinbackBrakeData[group].keylock = engine.getValue(group, "keylock")) > 0) {
                engine.setValue(group, "keylock", 0);
            }

            // setup timer and send first scratch2 'tick' if activating
            script.deckSpinbackBrakeData[group].timer = engine.beginTimer(50, 'script.deckSpinbackBrake("' + group + '")');
            engine.setValue(group, 'scratch2', script.deckSpinbackBrakeData[group].rate);
        }

        // re-enable keylock if needed

        else if (script.deckSpinbackBrakeData[group].keylock) {
            engine.setValue(group, "keylock", 1);
        }
    }
    else {
        // being called from a timer

        engine.setValue(group, 'scratch2', script.deckSpinbackBrakeData[group].rate);

        if (script.deckSpinbackBrakeData[group].delay > 0) {
            script.deckSpinbackBrakeData[group].delay--;
        }
        else {
            script.deckSpinbackBrakeData[group].rate *= script.deckSpinbackBrakeData[group].factor;
        }
    }
}

// bpm - Used for tapping the desired BPM for a deck
function bpm() {}

bpm.tapTime = 0.0;
bpm.tap = [];   // Tap sample values

/* -------- ------------------------------------------------------
        bpm.tapButton
   Purpose: Sets the bpm of the track on a deck by tapping the beats.
            This only works if the track's original BPM value is correct.
            Call this each time the tap button is pressed.
   Input:   Mixxx deck to adjust
   Output:  -
   -------- ------------------------------------------------------ */
bpm.tapButton = function(deck) {
    var now = new Date()/1000;   // Current time in seconds
    var tapDelta = now - bpm.tapTime;
    bpm.tapTime=now;
    if (tapDelta>2.0) { // reset if longer than two seconds between taps
        bpm.tap=[];
        return;
    }
    bpm.tap.push(60/tapDelta);
    if (bpm.tap.length>8) bpm.tap.shift();  // Keep the last 8 samples for averaging
    var sum = 0;
    for (i=0; i<bpm.tap.length; i++) {
        sum += bpm.tap[i];
    }
    var average = sum/bpm.tap.length;
    
    var fRateScale = average/engine.getValue("[Channel"+deck+"]","bpm");
    
    // Adjust the rate:
    fRateScale = (fRateScale-1.)/engine.getValue("[Channel"+deck+"]","rateRange");
    
    engine.setValue("[Channel"+deck+"]","rate",fRateScale * engine.getValue("[Channel"+deck+"]","rate_dir"));
//     print("Script: BPM="+average);
}

// ----------------- Object definitions --------------------------


ButtonState = {"released":0x00, "pressed":0x7F};
LedState =  {"off": 0x00, "on": 0x7F};

//Controller
function Controller () {
   this.group = "[Master]";
   this.Controls = [];
   this.Buttons = [];
};

Controller.prototype.addButton = function(buttonName, button, eventHandler) {
   if(eventHandler) {
      var executionEnvironment = this;
      function handler(value) {
         button.state = value;
         executionEnvironment[eventHandler](value);
      }
      button.handler = handler;
   }
   this.Buttons[buttonName] = button; 
}

Controller.prototype.setControlValue = function(control, value) {
   this.Controls[control].setValue(this.group, value);
}

//Button
function Button(controlId) {
   this.controlId = controlId;
   this.state = ButtonState.released;
}
Button.prototype.handleEvent = function(value) {
   this.handler(value);
};

//Control
function Control(mappedFunction, softMode) {
   // These defaults are for MIDI controllers
   this.minInput = 0;
   this.midInput = 0x3F;
   this.maxInput = 0x7F;
   // ----
   
   this.minOutput = -1.0;
   this.midOutput = 0.0;
   this.maxOutput = 1.0;
   this.mappedFunction = mappedFunction;
   this.softMode = softMode;
   this.maxJump = 10;
}

Control.prototype.setValue = function(group, inputValue){
   var outputValue = 0;
   if(inputValue <= this.midInput){
      outputValue = this.minOutput + ((inputValue - this.minInput) / (this.midInput - this.minInput)) * (this.midOutput - this.minOutput);
   } else {
      outputValue = this.midOutput + ((inputValue - this.midInput) / (this.maxInput - this.midInput)) * (this.maxOutput - this.midOutput);
   }
   if(this.softMode){ 
      var currentValue = engine.getValue(group, this.mappedFunction);
      var currentRelative = 0.0;
      if(currentValue <= this.midOutput){
         currentRelative = this.minInput + ((currentValue - this.minOutput) / (this.midOutput - this.minOutput)) * (this.midInput - this.minInput);
      } else {
         currentRelative = this.midInput + ((currentValue - this.midOutput) / (this.maxOutput - this.midOutput)) * (this.maxInput - this.midInput);
      }
      if(inputValue > currentRelative - this.maxJump && inputValue < currentRelative + this.maxJump) {
         engine.setValue(group, this.mappedFunction, outputValue);
      }
   } else {
      engine.setValue(group, this.mappedFunction, outputValue);
   }
}

//Deck
Deck = function (deckNumber, group) {
   this.deckNumber = deckNumber;
   this.group = group;
   this.Buttons = [];
}
Deck.prototype.setControlValue = Controller.prototype.setControlValue;
Deck.prototype.addButton = Controller.prototype.addButton;

// Data packet
function Packet(length, initialValue) {
    this.length = length;
    this.data = new Array(length);  // Size the array

    if (!initialValue) initialValue=0;

    // Initialize data values
    for (i=0; i<this.length; i++) {
        this.data[i]=initialValue;
    }
}

// ----------------- END Object definitions ----------------------
