import QtQuick 2.15
import QtQuick.Window 2.3
import QtQuick.Scene3D 2.14

import QtQuick.Controls 2.15
import QtQuick.Shapes 1.11
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import Qt5Compat.GraphicalEffects

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import "TraktorKontrolS4MK3Screens" as S4MK3

Item {
    id: root

    required property string screenId
    property color fontColor: Qt.rgba(242/255,242/255,242/255, 1)
    property color smallBoxBorder: Qt.rgba(44/255,44/255,44/255, 1)

    property string group: "[Channel1]"

    property var _items_needing_redraw: new Map()

    Timer {
        id: timer
    }
    function setTimeout(cb, delayTime) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }

    function init(controlerName, isDebug) {
        console.log(`Screen ${root.screenId} has started`)
        loader.sourceComponent = live
        group = screenId === "rightdeck" ? "[Channel2]" : "[Channel1]"
        onAir.update()
    }

    function shutdown() {
        console.log(`Screen ${root.screenId} is stopping`)
        loader.sourceComponent = splashoff
    }

    function redraw(component) {
        _items_needing_redraw.set(component, Date.now())
    }

    function transformFrame(input, timestamp) {
        const areasToDraw = []
        let totalPixelToDraw = 0;

        if (!_items_needing_redraw.size) { // No redraw needed
            return new ArrayBuffer(0);
        }

        const item_requesting_redraw = new Array()
        _items_needing_redraw.forEach(function(value, key, map) {
                if (value < timestamp) {
                    item_requesting_redraw.push(key);
                    _items_needing_redraw.delete(key);
                }
        })

        if (!item_requesting_redraw.length) { // No redraw needed
            return new ArrayBuffer(0);
        } else if (item_requesting_redraw.indexOf(root) !== -1) { // Full redraw needed
            areasToDraw.push([0, 0, 320, 240]);
            totalPixelToDraw += 320 * 240;
        } else { // Partial redraw needed
            item_requesting_redraw.forEach(function(item) {
                    const pos = item.mapToGlobal(0, 0)
                    console.log(`Redrawing item ${item}`)
                    let x = pos.x, y = pos.y, width = item.width, height = item.height;
                    areasToDraw.push([pos.x, pos.y, item.width, item.height])
                    totalPixelToDraw += item.width * item.height;
            });
            // Note: Some area could overlap and this could be optimised, but
            // the cost of checking that every time vs. the optimisation for
            // when it is happening is likely not worth it
        }

        // console.log(`Redrawing ${totalPixelToDraw} the following region: ${areasToDraw}`)

        const screenIdx = screenId === "leftdeck" ? 0 : 1;

        const outputData = new ArrayBuffer(totalPixelToDraw*2 + 20*areasToDraw.length);
        let offset = 0;

        for (const area of areasToDraw) {
            const [x, y, width, height] = area;
            const header = new Uint8Array(outputData, offset, 16);
            const payload = new Uint8Array(outputData, offset + 16, width*height*2);
            const footer = new Uint8Array(outputData, offset + width*height*2 + 16, 4);

            header.fill(0)
            footer.fill(0)
            header[0] = 0x84;
            header[2] = screenIdx;
            header[3] = 0x21;

            header[8] = x >> 8;
            header[9] = x & 0xff;
            header[10] = y >> 8;
            header[11] = y & 0xff;

            header[12] = width >> 8;
            header[13] = width & 0xff;
            header[14] = height >> 8;
            header[15] = height & 0xff;

            if (x === 0 && width === 320) {
                payload.set(new Uint8Array(input, y * 320 * 2, width*height*2));
            } else {
                for (let line = 0; line < height; line++) {
                    payload.set(new Uint8Array(input,
                            ((y + line) * 320 + x) * 2,
                            width * 2),
                        line * width * 2);
                }
            }
            footer[0] = 0x40;
            footer[2] = screenIdx;
            offset += width*height*2 + 20
        }
        // console.log(`Generated ${offset} bytes to be sent`)
        return outputData;
    }

    Component {
        id: splashoff
        Rectangle {
            anchors.fill: parent
            color: "black"

            Image {
                anchors.centerIn: parent
                width: root.width*0.8
                height: root.height
                fillMode: Image.PreserveAspectFit
                source: "../images/templates/logo_mixxx.png"
            }
        }
    }
    Component {
        id: live

        Rectangle {
            anchors.fill: parent
            color: "black"

            function onRuntimeDataUpdate(data) {
                if (!root) return;

                console.log(`Received data on screen#${root.screenId} while currently bind to ${root.group}: ${JSON.stringify(data)}`);
                if (typeof data === "object" && typeof data.group[root.screenId] === "string" && root.group !== data.group[root.screenId]) {
                    root.group = data.group[root.screenId]
                    waveformOverview.player = Mixxx.PlayerManager.getPlayer(root.group)
                    artwork.player = Mixxx.PlayerManager.getPlayer(root.group)
                    console.log(`Changed group for screen ${root.screenId} to ${root.group}`);
                }
                var shouldBeCompacted = false;
                if (typeof data.scrollingWavefom === "object") {
                    shouldBeCompacted |= data.scrollingWavefom[root.group]
                    scrollingWavefom.visible = data.scrollingWavefom[root.group]
                }
                if (typeof data.viewArtwork === "object") {
                    shouldBeCompacted |= data.viewArtwork[root.group]
                    artworkSpacer.visible = data.viewArtwork[root.group] && !scrollingWavefom.visible
                }
                if (typeof data.keyboardMode === "object") {
                    shouldBeCompacted |= data.keyboardMode[root.group]
                    keyboard.visible = !!data.keyboardMode[root.group]
                }
                deckInfo.state = shouldBeCompacted ? "compacted" : ""
                if (typeof data.displayBeatloopSize === "object") {
                    timeIndicator.mode = data.displayBeatloopSize[root.group] ? S4MK3.TimeAndBeatloopIndicator.Mode.BeetjumpSize : S4MK3.TimeAndBeatloopIndicator.Mode.RemainingTime
                    timeIndicator.update()
                }
                root.redraw(root);
            }

            Mixxx.ControlProxy {
                id: trackLoadedControl

                group: root.group
                key: "track_loaded"

                onValueChanged: (value) => {
                    if (!value && deckInfo) {
                        deckInfo.state = ""
                        scrollingWavefom.visible = false
                    }
                    redraw(parent)
                }
            }

            Component.onCompleted: {
                engine.onRuntimeDataUpdate(onRuntimeDataUpdate)

                // if (root.screenId === "rightdeck") {
                //     root.group = "[Channel2]"
                // }

                onRuntimeDataUpdate({
                        group: {
                            "leftdeck": "[Channel1]",
                            "rightdeck": "[Channel2]",
                        },
                        scrollingWavefom: {
                            "[Channel1]": false,
                            "[Channel2]": false,
                            "[Channel3]": false,
                            "[Channel4]": false,
                        },
                        keyboardMode: {
                            "[Channel1]": false,
                            "[Channel2]": false,
                            "[Channel3]": false,
                            "[Channel4]": false,
                        },
                        displayBeatloopSize: {
                            "[Channel1]": false,
                            "[Channel2]": false,
                            "[Channel3]": false,
                            "[Channel4]": false,
                        },
                });
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"

                Image {
                    id: artwork
                    anchors.fill: parent

                    property var player: Mixxx.PlayerManager.getPlayer(root.group)

                    source: player.coverArtUrl
                    height: 100
                    width: 100
                    fillMode: Image.PreserveAspectFit

                    opacity: artworkSpacer.visible ? 1 : 0.4
                    z: -1

                    onStatusChanged: {
                        redraw(parent)
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 6

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: "transparent"

                    RowLayout {
                        anchors.fill: parent
                        spacing: 1

                        S4MK3.OnAirTrack {
                            id: onAir
                            group: root.group
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            onUpdated: {
                                root.redraw(this)
                            }
                        }
                    }
                }

                // Indicator
                Rectangle {
                    id: deckInfo

                    Layout.fillWidth: true
                    Layout.preferredHeight: 105
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6
                    color: "transparent"

                    GridLayout {
                        id: gridLayout
                        anchors.fill: parent
                        columnSpacing: 6
                        rowSpacing: 6
                        columns: 2

                        // Section: Key
                        S4MK3.KeyIndicator {
                            id: keyIndicator
                            group: root.group
                            borderColor: smallBoxBorder

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            onUpdated: {
                                root.redraw(this)
                            }
                        }

                        // Section: Bpm
                        S4MK3.BPMIndicator {
                            id: bpmIndicator
                            group: root.group
                            borderColor: smallBoxBorder

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            onUpdated: {
                                root.redraw(this)
                            }
                        }

                        // Section: Key
                        S4MK3.TimeAndBeatloopIndicator {
                            id: timeIndicator
                            group: root.group

                            Layout.fillWidth: true
                            Layout.preferredHeight: 72
                            timeColor: smallBoxBorder

                            onUpdated: {
                                root.redraw(this)
                            }
                        }

                        // Section: Bpm
                        S4MK3.LoopSizeIndicator {
                            id: loopSizeIndicator
                            group: root.group

                            Layout.fillWidth: true
                            Layout.preferredHeight: 72

                            onUpdated: {
                                root.redraw(this)
                            }
                        }
                    }
                    states: State {
                        name: "compacted"

                        PropertyChanges {
                            target:deckInfo
                            Layout.preferredHeight: 28
                        }
                        PropertyChanges {
                            target: gridLayout
                            columns: 4
                        }
                        PropertyChanges {
                            target: bpmIndicator
                            state: "compacted"
                        }
                        PropertyChanges {
                            target: timeIndicator
                            Layout.preferredHeight: -1
                            Layout.fillHeight: true
                            state: "compacted"
                        }
                        PropertyChanges {
                            target: loopSizeIndicator
                            Layout.preferredHeight: -1
                            Layout.fillHeight: true
                            state: "compacted"
                        }
                    }

                    onStateChanged: {
                        root.redraw(root)
                    }
                }

                Item {
                    id: scrollingWavefom

                    Layout.fillWidth: true
                    Layout.minimumHeight: scrollingWavefom.visible ? 120 : 0
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6

                    visible: false

                    Mixxx.ControlProxy {
                        group: root.group
                        key: "playposition"
                        onValueChanged: (value) => {
                            if (!scrollingWavefom.visible) return;
                            root.redraw(scrollingWavefom)
                        }
                    }

                    MixxxControls.WaveformDisplay {
                        anchors.fill: parent
                        group: root.group
                        player: Mixxx.PlayerManager.getPlayer(root.group)
                    }

                    Rectangle {
                        color: "white"
                        visible: scrollingWavefom.visible
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        x: 153
                        width: 2
                    }
                }

                // Spacer
                Item {
                    id: artworkSpacer

                    Layout.fillWidth: true
                    Layout.minimumHeight: artworkSpacer.visible ? 120 : 0
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6

                    visible: false

                    Rectangle {
                        color: "transparent"
                        visible: parent.visible
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        x: 153
                        width: 2
                    }
                }

                // Track progress
                Item {
                    id: waveform
                    // Layout.preferredHeight: 40
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6
                    layer.enabled: true

                    S4MK3.Progression {
                        id: progression
                        group: root.group

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom

                        onUpdated: {
                            root.redraw(waveform)
                        }
                    }

                    // MixxxControls.WaveformOverview {
                    //     group: root.group
                    //     anchors.fill: parent
                    //     channels: Mixxx.WaveformOverview.Channels.BothChannels
                    //     renderer: Mixxx.WaveformOverview.Renderer.RGB
                    //     colorHigh: 'white'
                    //     colorMid: 'blue'
                    //     colorLow: 'green'
                    // }
                    Mixxx.WaveformOverview {
                        id: waveformOverview
                        anchors.fill: parent
                        player: Mixxx.PlayerManager.getPlayer(root.group)
                    }

                    Mixxx.ControlProxy {
                        id: samplesControl

                        group: root.group
                        key: "track_samples"
                    }

                    // Hotcue
                    Repeater {
                        model: 8

                        S4MK3.HotcuePoint {
                            required property int index

                            Mixxx.ControlProxy {
                                id: samplesControl

                                group: root.group
                                key: "track_samples"

                                onValueChanged: (value) => {
                                    redraw(waveform)
                                }
                            }

                            Mixxx.ControlProxy {
                                id: hotcueEnabled
                                group: root.group
                                key: `hotcue_${index + 1}_status`

                                onValueChanged: (value) => {
                                    redraw(waveform)
                                }
                            }

                            Mixxx.ControlProxy {
                                id: hotcuePosition
                                group: root.group
                                key: `hotcue_${index + 1}_position`

                                onValueChanged: (value) => {
                                    redraw(waveform)
                                }
                            }

                            anchors.top: parent.top
                            // anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            visible: hotcueEnabled.value

                            number: this.index + 1
                            type: S4MK3.HotcuePoint.Type.OneShot
                            position: hotcuePosition.value / samplesControl.value
                        }
                    }

                    // Intro
                    S4MK3.HotcuePoint {

                        Mixxx.ControlProxy {
                            id: introStartEnabled
                            group: root.group
                            key: `intro_start_enabled`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        Mixxx.ControlProxy {
                            id: introStartPosition
                            group: root.group
                            key: `intro_start_position`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        visible: introStartEnabled.value

                        type: S4MK3.HotcuePoint.Type.IntroIn
                        position: introStartPosition.value / samplesControl.value
                    }

                    // Extro
                    S4MK3.HotcuePoint {

                        Mixxx.ControlProxy {
                            id: introEndEnabled
                            group: root.group
                            key: `intro_end_enabled`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        Mixxx.ControlProxy {
                            id: introEndPosition
                            group: root.group
                            key: `intro_end_position`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        visible: introEndEnabled.value

                        type: S4MK3.HotcuePoint.Type.IntroOut
                        position: introEndPosition.value / samplesControl.value
                    }

                    // Loop in
                    S4MK3.HotcuePoint {
                        Mixxx.ControlProxy {
                            id: loopStartPosition
                            group: root.group
                            key: `loop_start_position`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        visible: loopStartPosition.value > 0

                        type: S4MK3.HotcuePoint.Type.LoopIn
                        position: loopStartPosition.value / samplesControl.value
                    }

                    // Loop out
                    S4MK3.HotcuePoint {
                        Mixxx.ControlProxy {
                            id: loopEndPosition
                            group: root.group
                            key: `loop_end_position`

                            onValueChanged: (value) => {
                                redraw(waveform)
                            }
                        }

                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        visible: loopEndPosition.value > 0

                        type: S4MK3.HotcuePoint.Type.LoopOut
                        position: loopEndPosition.value / samplesControl.value
                    }
                }

                S4MK3.Keyboard {
                    id: keyboard
                    group: root.group
                    visible: false
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6

                    onUpdated: (value) => {
                        redraw(this)
                    }
                }
            }
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: live
        onLoaded: {
            redraw(root)
        }
    }
}
