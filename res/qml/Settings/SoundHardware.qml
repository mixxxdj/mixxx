import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Category {
    id: root

    property bool committing: false
    property bool hasChanges: router.hasChanges

    function load() {
        const manager = Mixxx.SoundManager;
        mainMixEnabled.selected = mainMixEnabled.options[mainEnabled.value ? 0 : 1];
        mainOutputMode.selected = mainOutputMode.options[monoMix.value ? 0 : 1];
        soundClock.selected = soundClock.options[manager.getForceNetworkClock() ? 1 : 0];
        sampleRate.update(manager.getAPI());
        sampleRate.selected = qsTr("%1 Hz").arg(manager.getSampleRate());
        audioBuffer.currentIndex = manager.getAudioBufferSizeIndex() - 1;
        microphoneMonitorMode.enabled = manager.hasMicInputs();
        microphoneMonitorMode.currentIndex = micMonitorMode.value;
        soundApi.options = manager.getHostAPIList();
        soundApi.selected = manager.getAPI();
        keylock.update();
        keylock.selected = keylock.options[manager.getKeylockEngine()];

        // Router
        router.multiSoundcard.selected = router.multiSoundcard.options[manager.getSyncBuffers()];
        router.update(manager.getAPI());

        //Delays
        mainDelayLabel.enabled = mainEnabled.value;
        mainDelaySlider.enabled = mainEnabled.value;
        mainDelaySlider.value = mainDelay.value;
        boothDelayLabel.enabled = boothEnabled.value;
        boothDelaySlider.enabled = boothEnabled.value;
        boothDelaySlider.value = boothDelay.value;
        headphoneDelayLabel.enabled = headEnabled.value;
        headphoneDelaySlider.enabled = headEnabled.value;
        headphoneDelaySlider.value = headDelay.value;

        root.hasChanges = Qt.binding(function () {
            return router.hasChanges;
        });
    }
    function save() {
        const manager = Mixxx.SoundManager;
        mainEnabled.value = mainMixEnabled.options.indexOf(mainMixEnabled.selected);
        monoMix.value = !mainOutputMode.options.indexOf(mainOutputMode.selected);
        manager.setForceNetworkClock(soundClock.options[1] == soundClock.selected);
        manager.setSampleRate(parseInt(sampleRate.selected));
        manager.setAudioBufferSizeIndex(audioBuffer.currentIndex + 1);
        micMonitorMode.value = microphoneMonitorMode.currentIndex;
        manager.setAPI(soundApi.selected);
        manager.setKeylockEngine(keylock.options.indexOf(keylock.selected));

        // Router
        manager.setSyncBuffers(router.multiSoundcard.options.indexOf(router.multiSoundcard.selected));

        let connectionsHandler = (connections, device) => {
            for (let channel of Object.keys(connections)) {
                let connection = connections[channel];
                let type;
                let index = 0;
                let isOutput = true;
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
                        console.error(`unsupported address: ${connection.source.address}`);
                        continue;
                    }
                } else if (connection.sink.entity.name == "Mixer") {
                    isOutput = false;
                    type = connection.sink.address == "Auxiliary" ? 7 : 6;
                    index = connection.sink.instance;
                } else if (connection.source.entity.name.startsWith("Deck ") && connection.source.address == "Output") {
                    type = 4;
                    index = parseInt(connection.source.entity.name.split(' ')[1]) - 1;
                } else if (connection.sink.entity.name.startsWith("Deck ")) {
                    isOutput = false;
                    type = connection.source.address == "Output" ? 4 : 5;
                    index = parseInt(connection.sink.entity.name.split(' ')[1]) - 1;
                } else if (connection.sink.entity.name == "Microphone") {
                    isOutput = false;
                    type = 6;
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
                    console.error(`unsupported entity: ${connection.source.entity.name} ${connection.sink.entity.name}`);
                    continue;
                }
                console.log(isOutput ? "addOutput" : "addInput", device, type, channel * 2, index);
                if (isOutput) {
                    manager.addOutput(device, type, channel * 2, index);
                } else {
                    manager.addInput(device, type, channel * 2, index);
                }
            }
        };
        manager.clearOutputs();
        for (let device of Object.keys(router.outputs)) {
            for (let address of Object.keys(router.outputs[device].gateways)) {
                let gateway = router.outputs[device].gateways[address];
                let connections = gateway.node && gateway.node.assignedEdges ? gateway.node.assignedEdges() : {};
                connectionsHandler(connections, gateway.device);
            }
        }
        manager.clearInputs();
        for (let device of Object.keys(router.inputs)) {
            for (let address of Object.keys(router.inputs[device].gateways)) {
                let gateway = router.inputs[device].gateways[address];
                let connections = gateway.node && gateway.node.assignedEdges ? gateway.node.assignedEdges() : {};
                connectionsHandler(connections, gateway.device);
            }
        }

        mainDelay.value = mainDelaySlider.value;
        boothDelay.value = boothDelaySlider.value;
        headDelay.value = headphoneDelaySlider.value;

        root.committing = true;
        manager.commit();
    }

    label: "Sound hardware"
    tabs: ["engine", "delays", "stats"]

    Component.onCompleted: {
        load();
    }

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
    ColumnLayout {
        anchors.fill: parent

        Item {
            id: tabSection

            Layout.fillWidth: true
            Layout.preferredHeight: root.selectedIndex == 0 ? engine.height : delays.height

            Mixxx.SettingGroup {
                anchors.left: parent.left
                anchors.right: parent.right
                label: "Engine"
                visible: root.selectedIndex == 0

                onActivated: {
                    root.selectedIndex = 0;
                }

                RowLayout {
                    id: engine

                    anchors.left: parent.left
                    anchors.right: parent.right

                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true

                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Main Mix"

                                Mixxx.SettingParameter {
                                    label: "Main Mix"
                                }
                            }
                            RatioChoice {
                                id: mainMixEnabled

                                options: ["on", "off"]
                                selected: options[mainEnabled.value ? 0 : 1]

                                onSelectedChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Main Output Mode"

                                Mixxx.SettingParameter {
                                    label: "Main Output Mode"
                                }
                            }
                            RatioChoice {
                                id: mainOutputMode

                                options: ["mono", "stereo"]
                                selected: options[monoMix.value ? 0 : 1]

                                onSelectedChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Sound Clock"

                                Mixxx.SettingParameter {
                                    label: "Sound Clock"
                                }
                            }
                            RatioChoice {
                                id: soundClock

                                options: ["soundcard", "network"]

                                onSelectedChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Keylock engine"
                            }
                            RatioChoice {
                                id: keylock

                                function update() {
                                    let options = [];
                                    let tooltips = [];
                                    for (let engine of Mixxx.SoundManager.getKeylockEngines()) {
                                        switch (engine) {
                                        case 0:
                                            options.push(qsTr("Soundtouch"));
                                            tooltips.push(qsTr("Faster"));
                                            break;
                                        case 1:
                                            options.push(qsTr("Rubberband"));
                                            tooltips.push(qsTr("Better"));
                                            break;
                                        case 2:
                                            options.push(qsTr("Rubberband R3"));
                                            tooltips.push(qsTr("Near-hi-fi quality"));
                                            break;
                                        case 3:
                                            options.push(qsTr("SiS (Default)"));
                                            tooltips.push(qsTr("Near-hi-fi quality"));
                                            break;
                                        case 4:
                                            options.push(qsTr("SiS (Cheaper)"));
                                            tooltips.push(qsTr("Near-hi-fi quality"));
                                            break;
                                        }
                                    }
                                    keylock.options = options;
                                    keylock.tooltips = tooltips;
                                }

                                maxWidth: tabSection.width * 0.4
                                normalizedWidth: false
                                options: []
                                tooltips: []

                                onSelectedChanged: {
                                    root.hasChanges = true;
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
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Sound API"
                            }
                            RatioChoice {
                                id: soundApi

                                maxWidth: tabSection.width * 0.4
                                options: []

                                onSelectedChanged: {
                                    root.hasChanges = true;
                                    router.update(soundApi.selected);
                                    sampleRate.update(soundApi.selected);
                                }

                                Mixxx.SettingParameter {
                                    label: "Sound API"
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Sample Rate"

                                Mixxx.SettingParameter {
                                    label: "Sample Rate"
                                }
                            }
                            RatioChoice {
                                id: sampleRate

                                function update(api) {
                                    let data = [];
                                    for (let sampleRate of Mixxx.SoundManager.getSampleRates(api)) {
                                        data.push(qsTr("%1 Hz").arg(sampleRate));
                                    }
                                    sampleRate.options = data;
                                }

                                Layout.minimumWidth: sampleRate.implicitWidth
                                options: []

                                onSelectedChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                        Connections {
                            function onSelectedChanged() {
                                let sampleRateValue = parseInt(sampleRate.selected);
                                audioBuffer.update(sampleRateValue);
                            }

                            target: sampleRate
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                text: "Audio Buffer"

                                Mixxx.SettingParameter {
                                    label: "Audio Buffer"
                                }
                            }
                            Skin.ComboBox {
                                id: audioBuffer

                                function update(sampleRate) {
                                    let data = [];
                                    let framesPerBuffer = 1;
                                    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {}
                                    for (let i = 0; i < 7; i++) {
                                        const latency = framesPerBuffer / sampleRate * 1000;
                                        // i + 1 in the next line is a latency index as described in SSConfig
                                        data.push(qsTr("%1 ms").arg(latency.toFixed(1)));
                                        framesPerBuffer *= 2;
                                    }
                                    let currentIndex = audioBuffer.currentIndex;
                                    audioBuffer.model = data;
                                    audioBuffer.currentIndex = currentIndex;
                                }

                                clip: true
                                font.pixelSize: 12
                                spacing: 2

                                onCurrentIndexChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                        RowLayout {
                            Text {
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 14
                                opacity: Mixxx.SoundManager.hasMicInputs() ? 1.0 : 0.5
                                text: "Microphone Monitor Mode"

                                Mixxx.SettingParameter {
                                    label: "Microphone Monitor Mode"
                                }
                            }
                            Skin.ComboBox {
                                id: microphoneMonitorMode

                                clip: true
                                font.pixelSize: 12
                                model: ["Main output only", "Main and booth outputs", "Direct monitor (recording and broadcasting only)"]
                                opacity: enabled ? 1.0 : 0.5
                                spacing: 2

                                onCurrentIndexChanged: {
                                    root.hasChanges = true;
                                }
                            }
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                anchors.left: parent.left
                anchors.right: parent.right
                label: "Delays"
                visible: root.selectedIndex == 1

                onActivated: {
                    root.selectedIndex = 1;
                }

                GridLayout {
                    id: delays

                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 2
                    rowSpacing: 0

                    Text {
                        id: mainDelayLabel

                        Layout.fillWidth: true
                        color: Theme.white
                        font.pixelSize: 14
                        opacity: enabled ? 1 : 0.5
                        text: "Main Output"

                        Mixxx.SettingParameter {
                            label: "Main Output"
                        }
                    }
                    Skin.Slider {
                        id: mainDelaySlider

                        Layout.fillWidth: true
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        slider.to: 1000
                        suffix: "ms"

                        onValueChanged: {
                            root.hasChanges = true;
                        }
                    }
                    Text {
                        id: boothDelayLabel

                        Layout.fillWidth: true
                        color: Theme.white
                        enabled: boothEnabled.value
                        font.pixelSize: 14
                        opacity: enabled ? 1 : 0.5
                        text: "Booth Output"

                        Mixxx.SettingParameter {
                            label: "Booth Output"
                        }
                    }
                    Skin.Slider {
                        id: boothDelaySlider

                        Layout.fillWidth: true
                        enabled: boothEnabled.value
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        slider.to: 1000
                        suffix: "ms"
                        value: boothDelay.value

                        onValueChanged: {
                            root.hasChanges = true;
                        }
                    }
                    Text {
                        id: headphoneDelayLabel

                        Layout.fillWidth: true
                        color: Theme.white
                        enabled: headEnabled.value
                        font.pixelSize: 14
                        opacity: enabled ? 1 : 0.5
                        text: "Headphone Output"

                        Mixxx.SettingParameter {
                            label: "Headphone Output"
                        }
                    }
                    Skin.Slider {
                        id: headphoneDelaySlider

                        Layout.fillWidth: true
                        enabled: headEnabled.value
                        markers: ["0ms", "100ms", "1s", "10s", null]
                        slider.to: 1000
                        suffix: "ms"
                        value: headDelay.value

                        onValueChanged: {
                            root.hasChanges = true;
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                label: "Stats"
                visible: root.selectedIndex == 2

                onActivated: {
                    root.selectedIndex = 2;
                }

                Mixxx.SettingParameter {
                    label: "A white square"

                    Rectangle {
                        color: 'white'
                        height: 20
                        width: 20
                    }
                }
            }
        }
        Mixxx.SettingGroup {
            Layout.fillHeight: true
            Layout.fillWidth: true
            label: "Router"

            AudioRouter {
                id: router

                anchors.fill: parent
            }
            Rectangle {
                anchors.fill: parent
                color: Qt.alpha('grey', 0.3)
                visible: root.committing

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    preventStealing: true

                    onWheel: mouse => {
                        mouse.accepted = true;
                    }
                }
            }
        }
        RowLayout {
            Layout.topMargin: 4

            Skin.FormButton {
                activeColor: "#999999"
                backgroundColor: "#7D3B3B"
                enabled: !root.committing
                opacity: enabled ? 1.0 : 0.5
                text: "Cancel"
                visible: root.hasChanges

                onPressed: {
                    root.load();
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Text {
                id: errorMessage

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                color: "#7D3B3B"
                text: ""
            }
            Skin.FormButton {
                activeColor: "#999999"
                backgroundColor: root.hasChanges ? "#3a60be" : Theme.darkGray3
                enabled: root.hasChanges && !root.committing
                opacity: enabled ? 1.0 : 0.5
                text: "Save"

                onPressed: {
                    errorMessage.text = "";
                    root.save();
                }
            }
        }
    }
    Connections {
        function onCommitted(error) {
            root.committing = false;
            if (error) {
                errorMessage.text = error;
            }
            root.load();
        }

        target: Mixxx.SoundManager
    }
}
