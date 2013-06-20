MP3e2 = Object()

blink = new Object();

function light(id, enable) {
    if (enable == blink) {
        bval = 0x7f;
        val = 0;
    } else {
        bval = 0;
        val = enable?0x7f:0;
    }
    midi.sendShortMsg(0x90, id, val);
    midi.sendShortMsg(0x90, id + 0x30, bval);
}

function simpleButton(id) {
    return function(val, group) {
        var btn = (group == "[Channel1]")?(id):(id+20);

        light(btn, !!val);
    }
}

LEDs = [
    ["loop_start_position", simpleButton(1)],
    ["loop_end_position", simpleButton(2)],
    ["loop_enabled", simpleButton(51)],
    ["loop_enabled", simpleButton(52)],
    ["hotcue_1_enabled", simpleButton(5)],
    ["hotcue_2_enabled", simpleButton(6)],
    ["hotcue_3_enabled", simpleButton(7)],
    ["hotcue_4_enabled", simpleButton(8)],
    ["flanger", simpleButton(67)],
    ["play", simpleButton(15)],
    ["pfl", simpleButton(16)],
    ["beat_active", simpleButton(18)],
];

MP3e2.init = function(id) {
    HerculesMP3e2.init(id);

    // Set up LEDs
    for (var i = 0; i < 2; i += 1) {
        group = "[Channel" + (i+1) + "]";

        for (var j in LEDs) {
            var control = LEDs[j][0];
            var func = LEDs[j][1];

            engine.connectControl(group, control, func);
            engine.trigger(group, control);
        }
    }
}

var c1 = '[Channel1]'
var c2 = '[Channel2]'

MP3e2.incomingData = function(data, length) {
    for (var i = 0; i < length; i += 3) {
        var status = data[i];
        var midino = data[i+1];
        var value = data[i+2];
        var group;
        var f = null;

        if (status == 0xb0) {
            if ((midino > 0x38) || 
                ((midino < 0x34) && (midino & 1))) {
                group = c2;
            } else {
                group = c1;
            }
        } else if (status == 0x90) {
            if (midino <= 20) {
                group = c1;
            } else if (midino < 40) {
                group = c2;
            }
        }

        switch ((status<<8) | midino) {
            case 0x9001: case 0x9015:
            case 0x9002: case 0x9016:
            case 0x9003: case 0x9017:
            case 0x9004: case 0x9018:
            case 0x9005: case 0x9019:
            case 0x9006: case 0x901a:
            case 0x9007: case 0x901b:
            case 0x9008: case 0x901c:
                f = HerculesMP3e2.keyButton;
                break;
            case 0x900a: case 0x901e:
            case 0x900b: case 0x901f:
                f = HerculesMP3e2.pitchbend;
                break;
            case 0x900c: case 0x9020:
                f = "back";
                break;
            case 0x900d: case 0x9021:
                f = "fwd";
                break;
            case 0x900e: case 0x9022:
                f = HerculesMP3e2.cue;
                break;
            case 0x900f: case 0x9023:
                if (value == 0) return;
                f = "play";
                value = ! engine.getValue(group, f);
                break;
            case 0x9010: case 0x9024:
                if (value == 0) return;
                f = "pfl";
                value = ! engine.getValue(group, f);
                break;
            case 0x9011: case 0x9025:
                f = HerculesMP3e2.loadTrack;
                break;
            case 0x9012: case 0x9026:
                f = HerculesMP3e2.sync;
                break;
            case 0x9013: case 0x9027:
                f = HerculesMP3e2.masterTempo;
                break;


            case 0x9029:
                group = '[Playlist]';
                f = 'SelectPrevTrack';
                break;
            case 0x902a:
                group = '[Playlist]';
                f = 'SelectNextTrack';
                break;
            case 0x902b:
            case 0x902c:
                group = '[Playlist]';
                f = HerculesMP3e2.scroll;
                break;
            case 0x902d:
                f = HerculesMP3e2.scratch;
                break;
            case 0x902e:
                f = HerculesMP3e2.automix;
                break;

            case 0xb030: case 0xb031:
                f = HerculesMP3e2.jogWheel;
                break;
            case 0xb032: case 0xb033:
                f = HerculesMP3e2.pitch;
                break;
            case 0xb034: case 0xb039:
                engine.setValue(group, "volume", script.absoluteLin(value, 0, 1));
                break;
            case 0xb035: case 0xb03a:
                engine.setValue(group, "filterHigh", script.absoluteNonLin(value, 0, 1, 4));
                break;
            case 0xb036: case 0xb03b:
                engine.setValue(group, "filterMid", script.absoluteNonLin(value, 0, 1, 4));
                break;
            case 0xb037: case 0xb03c:
                engine.setValue(group, "filterLow", script.absoluteNonLin(value, 0, 1, 4));
                break;
            case 0xb038:
                engine.setValue('[Master]', 'crossfader', script.absoluteLin(value, -1, 1));
                break;
        }

        if (typeof(f) == 'string') {
            engine.setValue(group, f, (value>0)?1:0);
        } else if (f) {
            f(0, midino, value, status, group);
        }
    }
}

