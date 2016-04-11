MP3e2 = Object()

MP3e2.init = function(id) {
    HerculesMP3e2.init(id);
}

// Debug switch. set to true to print debug log messages in console.
var debug=false;

var c1 = '[Channel1]'
var c2 = '[Channel2]'

MP3e2.incomingData = function(data, length) {
    if (debug)
        print("******* Incoming Data from controller: data="+JSON.stringify(data)+", length="+length+" ;");
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
            if ((midino <= 20) || (midino >= 45)) {
                group = c1;
            } else if (midino < 40) {
                group = c2;
            } else if (midino < 45) {
                group = "[Paylist]";
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
                f = HerculesMP3e2.wind;
                break;
            case 0x900d: case 0x9021:
                f = HerculesMP3e2.wind;
                break;
            case 0x900e: case 0x9022:
                f = HerculesMP3e2.cue;
                break;
            case 0x900f: case 0x9023:
                f = HerculesMP3e2.play;
                break;
            case 0x9010: case 0x9024:
                f = HerculesMP3e2.pfl;
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
                f = HerculesMP3e2.selectTrack;
                break;
            case 0x902a:
                group = '[Playlist]';
                f = HerculesMP3e2.mic;
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
                f = HerculesMP3e2.volume;
                break;
            case 0xb035: case 0xb03a:
                f = HerculesMP3e2.filterHigh;
                break;
            case 0xb036: case 0xb03b:
                f = HerculesMP3e2.filterMid;
                break;
            case 0xb037: case 0xb03c:
                f = HerculesMP3e2.filterLow;
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

