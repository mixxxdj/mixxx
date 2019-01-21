HIDController.prototype.defaultScalers = {
    'volume': function (value) { return script.absoluteLin(value, 0, 1, 0, 4096) },
    'volumefader': function (value) { return Math.pow(HIDController.prototype.defaultScalers.volume(value), 2) },
    'eq': function (value) { return script.absoluteLin(value, 0, 2, 0, 4096) },
    'pregain': function (value) { return script.absoluteNonLin(value, 0, 1, 4, 0, 4096) },
    'rate': function (value) { return script.absoluteLin(value, -1, 1, 0, 4096) },
    'plusminus': function (value) { return script.absoluteLin(value, -1, 1, -4096, 4096) }
}

HIDController.prototype.softTakeover = function() {
    for(var i = 1; i < arguments.length; i++) {
        engine.softTakeover(arguments[0], arguments[i], true);
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

HIDController.prototype.clearLights = function(packetName) {
    const groups = this.getLightsPacket(packetName).groups
    for (var groupName in groups) {
        const group = groups[groupName]
        for (var control in group) {
            group[control].value = 0
        }
    }
    this.sendLightsUpdate()
}

HIDController.prototype.sendLightsUpdate = function (packetName) {
    this.getLightsPacket(packetName).send()
}

HIDController.prototype.getLightsPacket = function (packetName) {
    return this.getOutputPacket(packetName ? packetName : 'lights')
}


HIDController.prototype.connectLight = function (group, name, setter) {
    setter(engine.getValue(group, name), this.getLightsPacket(), group, name)
    const fun = function (value, group, name) {
        setter(value, this.getLightsPacket(), group, name)
        this.sendLightsUpdate()
    }
    engine.connectControl(group, name, fun)
    this.sendLightsUpdate()
    return fun
}


function tostring(obj, maxdepth, showFunctions, checked) {
    if (!checked)
        checked = []
    if (!maxdepth)
        maxdepth = 1
    if (maxdepth > 0 && typeof obj === 'object' && obj !== null && Object.getPrototypeOf(obj) != '' && !contains(checked, obj)) {
        checked.push(obj)
        var output = '{'
        for (var property in obj) {
            const value = obj[property]
            output += property + ': ' + (typeof value === 'function' && !showFunctions ? '[function]' : tostring(value, maxdepth - 1, showFunctions, checked)) + (maxdepth < 2 ? '; ' : '\n')
        }
        return output + '}'
    }
    return obj
}

function contains(array, elem) {
    for (var i = 0; i < array.length; i++) {
        if (array[i] == elem)
            return true
    }
    return false
}