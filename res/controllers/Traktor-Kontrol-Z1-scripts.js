function KontrolZ1Controller() {
    this.controller = new HIDController()

    this.controller.softTakeoverAll()

    this.mode = 'decks'

    // region CONTROLS

    this.registerInputPackets = function () {
        var packet = new HIDPacket('control', [0x1], 30)

        for (var c = 1; c < 3; c++) {
            var o = c * 10
            packet.addControl('hid', c + '_gain', o - 9, 'H')
            packet.addControl('hid', c + '_hi', o - 7, 'H')
            packet.addControl('hid', c + '_mid', o - 5, 'H')
            packet.addControl('hid', c + '_low', o - 3, 'H')
            packet.addControl('hid', c + '_fx', o - 1, 'H')
        }
        packet.addControl('hid', 'cue_mix', 21, 'H')
        packet.addControl('hid', '1_vol', 23, 'H')
        packet.addControl('hid', '2_vol', 25, 'H')
        packet.addControl('hid', 'crossfader', 27, 'H')

        var button = function (name, code) {
            packet.addControl('hid', name, 29, 'B', code)
        }
        button('mode', 0x2)
        button('1_headphone', 0x10)
        button('2_headphone', 0x1)
        button('1_button_fx', 0x4)
        button('2_button_fx', 0x8)

        this.controller.registerInputPacket(packet)
    }

    this.switchMode = function () {

    }

    this.registerCallbacks = function () {
        HIDDebug('Registering HID callbacks')

        var controller = this.controller

        for (var channel = 1; channel < 3; channel++) {
            controller.linkControl('hid', channel + '_headphone', '[Channel' + channel + ']', 'pfl')
            controller.setCallback('control', 'hid', channel + '_button_fx', function (button) {
                const ch = button.name.substr(0, 1)
                if (button.value === 1) {
                    engine.setValue('[QuickEffectRack1_[Channel' + ch + ']_Effect1]', 'enabled', engine.getValue('[QuickEffectRack1_[Channel' + ch + ']_Effect1]', 'enabled') ? 0 : 1)
                }
                controller.getOutputPacket()
            })

            this.linkKnob('decks', channel + '_gain', '[Channel' + channel + ']', 'pregain', 'pregain')
            this.linkKnob('decks', channel + '_hi', '[EqualizerRack1_[Channel' + channel + ']_Effect1]', 'parameter3', 'eq')
            this.linkKnob('decks', channel + '_mid', '[EqualizerRack1_[Channel' + channel + ']_Effect1]', 'parameter2', 'eq')
            this.linkKnob('decks', channel + '_low', '[EqualizerRack1_[Channel' + channel + ']_Effect1]', 'parameter1', 'eq')
            this.linkKnob('decks', channel + '_fx', '[QuickEffectRack1_[Channel' + channel + ']]', 'super1', 'plusminus')

            controller.setCallback('control', 'hid', channel + '_gain', this.knob)
            controller.setCallback('control', 'hid', channel + '_hi', this.knob)
            controller.setCallback('control', 'hid', channel + '_mid', this.knob)
            controller.setCallback('control', 'hid', channel + '_low', this.knob)
            controller.setCallback('control', 'hid', channel + '_fx', this.knob)

            this.linkKnob('decks', channel + '_vol', '[Channel' + channel + ']', 'volume', 'volumefader')
            controller.setCallback('control', 'hid', channel + '_vol', this.knob)
        }

        controller.setCallback('control', 'hid', 'mode', this.switchMode)

        this.linkKnob('decks', 'cue_mix', '[Master]', 'headMix', 'rate')
        controller.setCallback('control', 'hid', 'cue_mix', this.knob)
        this.linkKnob('decks', 'crossfader', '[Master]', 'crossfader', 'rate')
        controller.setCallback('control', 'hid', 'crossfader', this.knob)
    }

    // endregion

    // region LIGHTS

    this.registerOutputPackets = function () {
        var packet = new HIDPacket('lights', [0x80], 22)

        for (var c = 1; c < 3; c++) {
            for (var i = 1; i < 8; i++) {
                packet.addControl('hid', 'ch' + c + '_segment' + i, i + (c - 1) * 7, 'B')
            }
            packet.addControl('hid', c + '_headphone', 14 + c, 'B')
            packet.addControl('hid', c + '_button_fx_red', 14 + c * 3, 'B')
            packet.addControl('hid', c + '_button_fx_blue', 15 + c * 3, 'B')
        }
        packet.addControl('hid', 'mode', 19, 'B')

        this.controller.registerOutputPacket(packet)
    }

    this.sendLightsUpdate = function () {
        this.controller.getOutputPacket('lights').send()
    }

    // Mixxx VUMeter turns yellow starting at 0.9 / -1 db, with 1.0 / 0 db being the highest
    // todo dynamic brightness
    this.segmentBorders = [-30, -15, -8, -4, -2, -0.8, -0.2]
    this.brightness = 0x7f
    this.refreshVolumeLights = function (value, group, key) {
        var packet = this.controller.lightsPacket()
        const channel = group.substr(8, 1)
        for (var i = 0; i < 7; i++) {
            packet.getField('hid', 'ch' + channel + '_segment' + (i + 1)).value = 10 * Math.log(value) > this.segmentBorders[i] ? this.brightness : 0x00
        }
        this.sendLightsUpdate()
    }

    // endregion

}

var KontrolZ1 = new KontrolZ1Controller()

KontrolZ1.init = function (id) {
    KontrolZ1.id = id

    KontrolZ1.registerInputPackets()
    KontrolZ1.registerOutputPackets()
    KontrolZ1.registerCallbacks()

    for (var c = 1; c < 3; c++) {
        engine.connectControl('[Channel' + c + ']', 'VuMeter', KontrolZ1.refreshVolumeLights)
        KontrolZ1.controller.connectLight('[Channel' + c + ']', 'pfl', function (value, packet, group, name) {
            const channel = group.substr(8, 1)
            packet.getField('hid', channel + '_headphone').value = value * 0x7F
        })
        KontrolZ1.controller.connectLight('[QuickEffectRack1_[Channel' + c + ']_Effect1]', 'enabled', function (value, packet, group, name) {
            const channel = group.substr(26, 1)
            packet.getField('hid', channel + '_button_fx_red').value = value * 0x7F
            packet.getField('hid', channel + '_button_fx_blue').value = value * 0x7F
        })
    }

    // region test lights

    /*KontrolZ1.segments = new Object()
    KontrolZ1.segments['empty'] = [0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f]
    KontrolZ1.segments[0] = [0x0, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f]
    KontrolZ1.segments[1] = [0x0, 0x7f, 0x7f, 0x0, 0x0, 0x0, 0x0]
    KontrolZ1.segments[2] = [0x7f, 0x0, 0x7f, 0x7f, 0, 0x7f, 0x7f]
    KontrolZ1.segments[3] = [0x7f, 0x7f, 0x7f, 0x7f, 0, 0x0, 0x7f]
    KontrolZ1.segments[4] = [0x7f, 0x7f, 0x7f, 0x0, 0x7f, 0x0, 0x0]
    KontrolZ1.segments[5] = [0x7f, 0x7f, 0x0, 0x7f, 0x7f, 0x0, 0x7f]
    KontrolZ1.segments[6] = [0x7f, 0x7f, 0x0, 0x7f, 0x7f, 0x7f, 0x7f]
    KontrolZ1.segments[7] = [0x0, 0x7f, 0x7f, 0x7f, 0x0, 0x0, 0x0]
    KontrolZ1.segments[8] = [0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f]
    KontrolZ1.segments[9] = [0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x0, 0x7f]

    KontrolZ1.set7SegmentNumber = function (number) {
        var controller = this.controller
        var packet = controller.getOutputPacket('lights')
        if (number != undefined) {
            left = KontrolZ1.segments[Math.floor(number / 10)]
            right = KontrolZ1.segments[number % 10]
        } else {
            left = KontrolZ1.segments['empty']
            right = KontrolZ1.segments['empty']
        }

        for (var i = 0; i < 7; i++) {
            packet.getField('hid', 'ch1_segment' + (i + 1)).value = left[i]
            packet.getField('hid', 'ch2_segment' + (i + 1)).value = right[i]
        }
        HIDDebug('segments left ' + left + ' right ' + right)
    }

    KontrolZ1.testSegments = function () {
        if (KontrolZ1.testSegment < 100) {
            KontrolZ1.set7SegmentNumber(KontrolZ1.testSegment)
            KontrolZ1.testSegment += 1
        } else {
            engine.stopTimer(KontrolZ1.testTimer)
            KontrolZ1.set7SegmentNumber(undefined)
        }
        KontrolZ1.controller.sendLightsUpdate()
    }

    KontrolZ1.testUpdateInterval = 2
    KontrolZ1.testSegment = 0
    KontrolZ1.testTimer = engine.beginTimer(
        KontrolZ1.testUpdateInterval,
        'KontrolZ1.testSegments()'
    )*/

    //endregion

    print('NI Traktor Kontrol Z1 ' + KontrolZ1.id + ' initialized!')
}


// region knobs

KontrolZ1.knobs = {}
KontrolZ1.linkKnob = function (mode, knob, group, name, scaler) {
    if (!(mode in KontrolZ1.knobs))
        KontrolZ1.knobs[mode] = {}
    KontrolZ1.knobs[mode][knob] = {
        'mode': mode,
        'knob': knob,
        'group': group,
        'name': name,
        'scaler': scaler
    }
}

KontrolZ1.control = function (control, field) {
    if (control.callback !== undefined) {
        control.callback(control, field)
        return
    }
    var scaler = KontrolZ1.controller.defaultScalers[control.scaler]
    engine.setValue(control.group, control.name, scaler(field.value))
}

KontrolZ1.knob = function (field) {
    var mode = KontrolZ1.knobs[KontrolZ1.mode]
    if (mode === undefined) {
        HIDDebug('Knob group not mapped in mode ' + KontrolZ1.mode)
        return
    }
    var knob = mode[field.name]
    if (knob === undefined) {
        HIDDebug('Fader ' + field.name + ' not mapped in ' + KontrolZ1.mode)
        return
    }
    return KontrolZ1.control(knob, field)
}

// endregion

KontrolZ1.shutdown = function () {
    KontrolZ1.controller.clearLights()
    print('NI Traktor Kontrol Z1 ' + KontrolZ1.id + ' shut down!')
}

KontrolZ1.incomingData = function (data, length) {
    KontrolZ1.controller.parsePacket(data, length)
}
