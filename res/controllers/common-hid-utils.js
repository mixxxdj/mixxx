HIDController.prototype.softTakeover = function() {
    for (var i = 1; i < arguments.length; i++) {
        engine.softTakeover(arguments[0], arguments[i], true)
    }
}

HIDController.prototype.softTakeoverAll = function() {
    for (var channel = 1; channel < 5; channel++) {
        this.softTakeover('[EqualizerRack1_[Channel' + channel + ']_Effect1]', 'parameter1', 'parameter2', 'parameter3')
        this.softTakeover('[QuickEffectRack1_[Channel' + channel + ']]', 'super1')
        this.softTakeover('[Channel' + channel + ']', 'pregain', 'volume', 'rate')
        for (var i = 1; i < 4; i++) {
            this.softTakeover('[EffectRack1_EffectUnit' + channel + '_Effect' + i + ']', 'meta')
        }
    }
    this.softTakeover('[Master]', 'headMix', 'crossfader')
}

HIDPacket.prototype.clearControls = function() {
    for (var groupName in  this.groups) {
        var group = this.groups[groupName]
        for (var control in group) {
            group[control].value = 0
        }
    }
    this.send()
}

HIDController.prototype.getLightsPacket = function(packetName) {
	return this.getOutputPacket(packetName ? packetName : 'lights')
}

HIDController.prototype.sendLightsUpdate = function(packetName) {
    this.getLightsPacket(packetName).send()
}

HIDController.prototype.connectLight = function(group, name, setter) {
    setter(engine.getValue(group, name), this.getLightsPacket(), group, name)
    var fun = function(value, group, name) {
        setter(value, this.getLightsPacket(), group, name)
        this.sendLightsUpdate()
    }
    engine.makeConnection(group, name, fun)
    this.sendLightsUpdate()
    return fun
}
