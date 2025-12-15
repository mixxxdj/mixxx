import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Category {
    id: root

    label: "Sound hardware"
    tabs: ["engine", "delays", "stats"]

    property bool hasChanges: router.hasChanges
    property bool committing: false

    Mixxx.ControlProxy {
        id: mainEnabled
        group: "[Master]"
        key: "enabled"
    }
    Mixxx.ControlProxy {
        id: headEnabled
        group: "[Master]"
        key: "headEnabled"
    }
    Mixxx.ControlProxy {
        id: boothEnabled
        group: "[Master]"
        key: "booth_enabled"
    }
    Mixxx.ControlProxy {
        id: mainDelay
        group: "[Master]"
        key: "delay"
    }
    Mixxx.ControlProxy {
        id: headDelay
        group: "[Master]"
        key: "headDelay"
    }
    Mixxx.ControlProxy {
        id: boothDelay
        group: "[Master]"
        key: "boothDelay"
    }
    Mixxx.ControlProxy {
        id: monoMix
        group: "[Master]"
        key: "mono_mixdown"
    }
    Mixxx.ControlProxy {
        id: micMonitorMode
        group: "[Master]"
        key: "talkover_mix"
    }

    function save() {
        const manager = Mixxx.SoundManager;
        mainEnabled.value = mainMixEnabled.options.indexOf(mainMixEnabled.selected)
        monoMix.value = !mainOutputMode.options.indexOf(mainOutputMode.selected)
        manager.setForceNetworkClock(soundClock.options[1] == soundClock.selected)
        manager.setSampleRate(parseInt(sampleRate.selected))
        manager.setAudioBufferSizeIndex(audioBuffer.currentIndex + 1)
        micMonitorMode.value = microphoneMonitorMode.currentIndex
        manager.setAPI(soundApi.selected)
        manager.setKeylockEngine(keylock.options.indexOf(keylock.selected))

        // Router
        manager.setSyncBuffers(router.multiSoundcard.options.indexOf(router.multiSoundcard.selected))

        let connectionsHandler = (connections, device) => {
            for (let channel of Object.keys(connections)) {
                let connection = connections[channel]
                let type;
                let index = 0;
                let isOutput = true
                if (connection.source.entity.name == "Mixer") {
                    switch (connection.source.address) {
                        case "Main":
                            type = 0;
                            break;
                        case "PFL":
                            type = 1;
                            break;
                        case "Booth":
                            type = 2;
                            break;
                        case "Left Bus":
                        case "Center Bus":
                        case "Right Bus":
                            index = connection.source.address.startsWith("Left") ? 0 : connection.source.address.startsWith("Right") ? 2 : 1;
                            type = 3;
                            break;
                        default:
                            console.error(`unsupported address: ${connection.source.address}`)
                            continue;
                    }
                } else if (connection.sink.entity.name == "Mixer") {
                    isOutput = false;
                    type = connection.sink.address == "Auxiliary" ? 7 : 6;
                    index = connection.sink.instance;
                } else if (connection.source.entity.name.startsWith("Deck ") && connection.source.address == "Output") {
                    type = 4;
                    index = parseInt(connection.source.entity.name.split(' ')[1])-1;
                } else if (connection.sink.entity.name.startsWith("Deck ")) {
                    isOutput = false;
                    type = connection.source.address == "Output" ? 4 : 5;
                    index = parseInt(connection.sink.entity.name.split(' ')[1])-1;
                } else if (connection.sink.entity.name == "Microphone") {
                    isOutput = false;
                    type = 6
                    index = connection.sink.instance;
                } else if (connection.sink.entity.name == "Auxiliary") {
                    isOutput = false;
                    type = 7;
                    index = connection.sink.instance;
                } else if (connection.sink.entity.name == "RecordBroadcast") {
                    isOutput = false;
                    type = 8;
                    index = connection.sink.instance;
                } else {
                    console.error(`unsupported entity: ${connection.source.entity.name} ${connection.sink.entity.name}`)
                    continue;
                }
                console.log(isOutput ? "addOutput" : "addInput", device, type, channel * 2, index)
                if (isOutput) {
                    manager.addOutput(device, type, channel * 2, index)
                } else {
                    manager.addInput(device, type, channel * 2, index)
                }
            }
        };
        manager.clearOutputs()
        for (let device of Object.keys(router.outputs)) {
            for (let address of Object.keys(router.outputs[device].gateways)) {
                let gateway = router.outputs[device].gateways[address]
                let connections = gateway.node && gateway.node.assignedEdges ? gateway.node.assignedEdges() : {};
                connectionsHandler(connections, gateway.device)
            }
        }
        manager.clearInputs()
        for (let device of Object.keys(router.inputs)) {
            for (let address of Object.keys(router.inputs[device].gateways)) {
                let gateway = router.inputs[device].gateways[address]
                let connections = gateway.node && gateway.node.assignedEdges ? gateway.node.assignedEdges() : {};
                connectionsHandler(connections, gateway.device)
            }
        }

        mainDelay.value = mainDelaySlider.value
        boothDelay.value = boothDelaySlider.value
        headDelay.value = headphoneDelaySlider.value

        root.committing = true
        manager.commit()
    }

    function load() {
        const manager = Mixxx.SoundManager;
        mainMixEnabled.selected = mainMixEnabled.options[mainEnabled.value ? 0 : 1 ]
        mainOutputMode.selected = mainOutputMode.options[monoMix.value ? 0 : 1 ]
        soundClock.selected = soundClock.options[manager.getForceNetworkClock() ? 1 : 0 ]
        sampleRate.update(manager.getAPI())
        sampleRate.selected = qsTr("%1 Hz").arg(manager.getSampleRate())
        audioBuffer.currentIndex = manager.getAudioBufferSizeIndex() - 1
        microphoneMonitorMode.enabled = manager.hasMicInputs()
        microphoneMonitorMode.currentIndex = micMonitorMode.value
        soundApi.options = manager.getHostAPIList()
        soundApi.selected = manager.getAPI()
        keylock.update()
        keylock.selected = keylock.options[manager.getKeylockEngine()]

        // Router
        router.multiSoundcard.selected = router.multiSoundcard.options[manager.getSyncBuffers()]
        router.update(manager.getAPI())

        //Delays
        mainDelayLabel.enabled = mainEnabled.value
        mainDelaySlider.enabled = mainEnabled.value
        mainDelaySlider.value = mainDelay.value
        boothDelayLabel.enabled = boothEnabled.value
        boothDelaySlider.enabled = boothEnabled.value
        boothDelaySlider.value = boothDelay.value
        headphoneDelayLabel.enabled = headEnabled.value
        headphoneDelaySlider.enabled = headEnabled.value
        headphoneDelaySlider.value = headDelay.value

        root.hasChanges = Qt.binding(function() { return router.hasChanges; });
    }

    Component.onCompleted: {
        load()
    }

    ColumnLayout {
        anchors.fill: parent

        Item {
            id: tabSection
            Layout.fillWidth: true
            Layout.preferredHeight: root.selectedIndex == 0 ? engine.height : delays.height
            Mixxx.SettingGroup {
                label: "Engine"
                visible: root.selectedIndex == 0
                onActivated: {
                    root.selectedIndex = 0
                }
                anchors.left: parent.left
                anchors.right: parent.right
                RowLayout {
                    id: engine
                    anchors.left: parent.left
                    anchors.right: parent.right
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Main Mix"
                                }
                                Layout.fillWidth: true
                                text: "Main Mix"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: mainMixEnabled
                                options: [
                                          "on",
                                          "off"
                                ]
                                selected: options[mainEnabled.value ? 0 : 1 ]
                                onSelectedChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }

                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Main Output Mode"
                                }
                                Layout.fillWidth: true
                                text: "Main Output Mode"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: mainOutputMode
                                options: [
                                          "mono",
                                          "stereo"
                                ]
                                selected: options[monoMix.value ? 0 : 1 ]
                                onSelectedChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }

                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Sound Clock"
                                }
                                Layout.fillWidth: true
                                text: "Sound Clock"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: soundClock
                                options: [
                                          "soundcard",
                                          "network"
                                ]
                                onSelectedChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }

                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                text: "Keylock engine"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: keylock
                                normalizedWidth: false
                                maxWidth: tabSection.width * 0.4
                                options: []
                                tooltips: []

                                function update() {
                                    let options = []
                                    let tooltips = []
                                    for (let engine of Mixxx.SoundManager.getKeylockEngines()) {
                                        switch (engine) {
                                            case 0:
                                                options.push(qsTr("Soundtouch"))
                                                tooltips.push(qsTr("Faster"))
                                                break
                                            case 1:
                                                options.push(qsTr("Rubberband"))
                                                tooltips.push(qsTr("Better"))
                                                break
                                            case 2:
                                                options.push(qsTr("Rubberband R3"))
                                                tooltips.push(qsTr("Near-hi-fi quality"))
                                                break
                                        }
                                    }
                                    keylock.options = options
                                    keylock.tooltips = tooltips
                                }

                                onSelectedChanged: {
                                    root.hasChanges = true
                                }

                                Mixxx.SettingParameter {
                                    label: "Keylock engine"
                                }
                            }
                        }
                    }
                    Item {
                        Layout.preferredWidth: 70
                    }
                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                text: "Sound API"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: soundApi
                                maxWidth: tabSection.width * 0.4
                                options: []

                                onSelectedChanged: {
                                    root.hasChanges = true
                                    router.update(soundApi.selected)
                                    sampleRate.update(soundApi.selected)
                                }

                                Mixxx.SettingParameter {
                                    label: "Sound API"
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Sample Rate"
                                }
                                Layout.fillWidth: true
                                text: "Sample Rate"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            RatioChoice {
                                id: sampleRate
                                Layout.minimumWidth: sampleRate.implicitWidth
                                options: []
                                function update(api) {
                                    let data = []
                                    for (let sampleRate of Mixxx.SoundManager.getSampleRates(api)) {
                                        data.push(qsTr("%1 Hz").arg(sampleRate));
                                    }
                                    sampleRate.options =  data
                                }
                                onSelectedChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }

                        Connections {
                            target: sampleRate
                            function onSelectedChanged() {
                                let sampleRateValue = parseInt(sampleRate.selected)
                                audioBuffer.update(sampleRateValue)
                            }
                        }

                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Audio Buffer"
                                }
                                Layout.fillWidth: true
                                text: "Audio Buffer"
                                color: Theme.white
                                font.pixelSize: 14
                            }
                            Skin.ComboBox {
                                id: audioBuffer
                                spacing: 2
                                clip: true

                                font.pixelSize: 12

                                function update(sampleRate) {
                                    let data = []
                                    let framesPerBuffer = 1;
                                    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {
                                    }
                                    for (let i = 0; i < 7; i++) {
                                        const latency = framesPerBuffer / sampleRate * 1000;
                                        // i + 1 in the next line is a latency index as described in SSConfig
                                        data.push(qsTr("%1 ms").arg(latency.toFixed(1)));
                                        framesPerBuffer *= 2
                                    }
                                    let currentIndex = audioBuffer.currentIndex
                                    audioBuffer.model = data
                                    audioBuffer.currentIndex = currentIndex
                                }
                                onCurrentIndexChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }

                        RowLayout {
                            Text {
                                Mixxx.SettingParameter {
                                    label: "Microphone Monitor Mode"
                                }
                                Layout.fillWidth: true
                                text: "Microphone Monitor Mode"
                                color: Theme.white
                                opacity: Mixxx.SoundManager.hasMicInputs() ? 1.0 : 0.5
                                font.pixelSize: 14
                            }
                            Skin.ComboBox {
                                id: microphoneMonitorMode
                                spacing: 2
                                clip: true
                                opacity: enabled ? 1.0 : 0.5

                                font.pixelSize: 12
                                model: [
                                        "Main output only",
                                        "Main and booth outputs",
                                        "Direct monitor (recording and broadcasting only)"
                                ]
                                onCurrentIndexChanged: {
                                    root.hasChanges = true
                                }
                            }
                        }
                    }
                }
            }

            Mixxx.SettingGroup {
                label: "Delays"
                visible: root.selectedIndex == 1
                onActivated: {
                    root.selectedIndex = 1
                }
                anchors.left: parent.left
                anchors.right: parent.right
                GridLayout {
                    id: delays
                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 2
                    rowSpacing: 0
                    Text {
                        Mixxx.SettingParameter {
                            label: "Main Output"
                        }
                        id: mainDelayLabel
                        Layout.fillWidth: true
                        text: "Main Output"
                        color: Theme.white
                        opacity: enabled ? 1 : 0.5
                        font.pixelSize: 14
                    }
                    Skin.Slider {
                        id: mainDelaySlider
                        Layout.fillWidth: true
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        suffix: "ms"
                        slider.to: 1000

                        onValueChanged: {
                            root.hasChanges = true
                        }
                    }
                    Text {
                        Mixxx.SettingParameter {
                            label: "Booth Output"
                        }
                        id: boothDelayLabel
                        Layout.fillWidth: true
                        text: "Booth Output"
                        color: Theme.white
                        opacity: enabled ? 1 : 0.5
                        enabled: boothEnabled.value
                        font.pixelSize: 14
                    }
                    Skin.Slider {
                        id: boothDelaySlider
                        Layout.fillWidth: true
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        suffix: "ms"
                        enabled: boothEnabled.value
                        value: boothDelay.value
                        slider.to: 1000

                        onValueChanged: {
                            root.hasChanges = true
                        }
                    }
                    Text {
                        Mixxx.SettingParameter {
                            label: "Headphone Output"
                        }
                        id: headphoneDelayLabel
                        Layout.fillWidth: true
                        text: "Headphone Output"
                        color: Theme.white
                        opacity: enabled ? 1 : 0.5
                        enabled: headEnabled.value
                        font.pixelSize: 14
                    }
                    Skin.Slider {
                        id: headphoneDelaySlider
                        Layout.fillWidth: true
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        suffix: "ms"
                        enabled: headEnabled.value
                        value: headDelay.value
                        slider.to: 1000

                        onValueChanged: {
                            root.hasChanges = true
                        }
                    }
                }
            }

            Mixxx.SettingGroup {
                label: "Stats"
                visible: root.selectedIndex == 2
                onActivated: {
                    root.selectedIndex = 2
                }
                Mixxx.SettingParameter {
                    label: "A white square"
                    Rectangle {
                        width: 20
                        height: 20
                        color: 'white'
                    }
                }
            }
        }
        Mixxx.SettingGroup {
            label: "Router"
            Layout.fillWidth: true
            Layout.fillHeight: true
            AudioRouter {
                id: router
                anchors.fill: parent
            }
            Rectangle {
                anchors.fill: parent
                visible: root.committing
                color: Qt.alpha('grey', 0.3)
                MouseArea {
                    anchors.fill: parent
                    preventStealing: true
                    hoverEnabled: true

                    onWheel: (mouse)=> {
                        mouse.accepted = true
                    }
                }
            }
        }
        RowLayout {
            Layout.topMargin: 4
            Skin.FormButton {
                enabled: !root.committing
                visible: root.hasChanges
                text: "Cancel"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#7D3B3B"
                activeColor: "#999999"
                onPressed: {
                    root.load()
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                id: errorMessage
                text: ""
                color: "#7D3B3B"
            }
            Skin.FormButton {
                enabled: root.hasChanges && !root.committing
                text: "Save"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    errorMessage.text = ""
                    root.save()
                }
            }
        }
    }
    Connections {
        target: Mixxx.SoundManager
        function onCommitted(error) {
            root.committing = false
            if (error) {
                errorMessage.text = error
            }
            root.load()
        }
    }
}
