
var MPKmini = {};

MPKmini.init = function() {
    this.currentUnitNum = 1;
    this.currentEffectNum = 1;
    this.currentFlash = true;
    this.timerId = 0;

    MPKmini.updateLed();
    this.timerId = engine.beginTimer(200, "MPKmini.flashLed()");
};

MPKmini.shutdown = function() {
    if (this.timerId !== 0)
        engine.stopTimer(this.timerId);
    else
        print("Shutdown: no timer to stop.");
};

MPKmini.getCurrentGroup = function(withEffect, withUnit) {
    var group = "";
    if (withEffect) {
        group =  "[EffectRack1_EffectUnit" + MPKmini.currentUnitNum + "_Effect" + MPKmini.currentEffectNum + "]";
    } else if (withUnit) {
        group =  "[EffectRack1_EffectUnit" + MPKmini.currentUnitNum + "]";
    } else {
        group =  "[EffectRack1]";
    }
    print("get group: "+group);
    return group;
};

MPKmini.resetLed = function() {
    midi.sendShortMsg(0x90, 13, engine.getValue("[EffectRack1_EffectUnit1_Effect1]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 14, engine.getValue("[EffectRack1_EffectUnit1_Effect2]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 15, engine.getValue("[EffectRack1_EffectUnit1_Effect3]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 16, engine.getValue("[EffectRack1_EffectUnit1_Effect4]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90,  9, engine.getValue("[EffectRack1_EffectUnit2_Effect1]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 10, engine.getValue("[EffectRack1_EffectUnit2_Effect2]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 11, engine.getValue("[EffectRack1_EffectUnit2_Effect3]", "enabled") ? 127 : 0);
    midi.sendShortMsg(0x90, 12, engine.getValue("[EffectRack1_EffectUnit2_Effect4]", "enabled") ? 127 : 0);
};

MPKmini.updateLed = function() {
    MPKmini.resetLed();
    MPKmini.flashLed();
};

MPKmini.flashLed = function() {
    var start = (MPKmini.currentUnitNum === 1) ? 12 : 8;
    midi.sendShortMsg(0x90, start + MPKmini.currentEffectNum, MPKmini.currentFlash ? 127 : 0);
    MPKmini.currentFlash = !MPKmini.currentFlash;
};

// Control mapping
MPKmini.setValue = function(group, key, newValue) {
    var withEffect = false;

    if (group === "[Effect]") {
        withEffect = true;
    }

    newValue = newValue / 127;

    var threshold = 0.07; //on the CMD Studio 4a this threshold got the right balance between smooth takeover and keeping up with quick turns, but you can adjust the value to suit your needs

    var currentParamValue = engine.getParameter(MPKmini.getCurrentGroup(withEffect, true), key);
    var spread = Math.abs(currentParamValue - newValue);

    if (spread < threshold) {
        engine.setParameter(MPKmini.getCurrentGroup(withEffect, true), key, newValue);
    } else {
        return;
    }
};

MPKmini.parameter1 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter1", value);
};
MPKmini.parameter2 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter2", value);
};
MPKmini.parameter3 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter3", value);
};
MPKmini.parameter4 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter4", value);
};
MPKmini.parameter5 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter5", value);
};
MPKmini.parameter6 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter6", value);
};
MPKmini.parameter7 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter7", value);
};
MPKmini.parameter8 = function(channel, control, value, status, group) {
    MPKmini.setValue(group, "parameter8", value);
};

MPKmini.parameter1Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 1, 1);
};
MPKmini.parameter2Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 1, 2);
};
MPKmini.parameter3Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 1, 3);
};
MPKmini.parameter4Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 1, 4);
};
MPKmini.parameter5Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 2, 1);
};
MPKmini.parameter6Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 2, 2);
};
MPKmini.parameter7Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 2, 3);
};
MPKmini.parameter8Enabled = function(channel, control, value, status, group) {
    MPKmini.effectSelectEnable(group, value, 2, 4);
};

MPKmini.effectSelectEnable = function(group, value, unit, effect) {

    if (value < 80) {
        MPKmini.selectUnitEffect(unit, effect);

        print("Selected "+group+" is now "+unit+","+effect);
    } else {
        engine.setValue(group, "enabled", !engine.getValue(group, "enabled"));

        print("Enable "+group+" is now "+engine.getValue(group, "enabled"));

        if (engine.getValue(group, "enabled")) {
            MPKmini.selectUnitEffect(unit, effect);

            print("Selected "+group+" is now "+unit+","+effect);
        }
    }

    MPKmini.updateLed();
};

MPKmini.samplerPlay = function(channel, control, value, status, group) {
    group = "[Sampler" + (control - 0x30 + 1) + "]";

    var pregain = value / 127.0;
    engine.setValue(group, "pregain", pregain);
    engine.setValue(group, "reverse", 0);
    engine.setValue(group, "start_play", true);

    print("Start Play " + group + " pregain " + pregain);
};

MPKmini.selectUnitEffect = function(unitNum, effectNum) {
    MPKmini.currentUnitNum = unitNum;
    MPKmini.currentEffectNum = effectNum;
    MPKmini.updateLed();
};
