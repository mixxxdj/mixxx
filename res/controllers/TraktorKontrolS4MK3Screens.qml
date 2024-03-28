import QtQuick 2.15
import QtQuick.Window 2.3
import QtQuick.Scene3D 2.14

import QtQuick.Controls 2.15
import QtQuick.Shapes 1.11
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import Qt5Compat.GraphicalEffects

import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import S4MK3 as S4MK3

Item {
    id: root

    required property string screenId
    property color fontColor: Qt.rgba(242/255,242/255,242/255, 1)
    property color smallBoxBorder: Qt.rgba(44/255,44/255,44/255, 1)

    property string group: screenId == "rightdeck" ? "[Channel2]" : "[Channel1]"

    property var _zones_needing_redraw: new Array()

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
        root.state = "Live"
    }

    function shutdown() {
        console.log(`Screen ${root.screenId} is stopping`)
        root.state = "Stop"
    }

    function redraw(component) {
        const timestamp = Date.now();
        const pos = component.mapToGlobal(0, 0)
        const x = Math.min(Math.max(0, pos.x), 320), y = Math.min(Math.max(0, pos.y), 240);
        const zone = Qt.rect(
            x,
            y,
            Math.min(Math.max(0, component.width), 320 - x),
            Math.min(Math.max(0, component.height), 240 - y )
        );
        if (zone.width * zone.height) {
            _zones_needing_redraw.push([zone, timestamp])
        } else {
            console.log(`Component generated a null rectangle! pos=${pos}, component=${component}, width=${component.width}, height=${component.height}`)
        }
    }

    function contains_rect(rect1, rect2) {
        return rect1.x <= rect2.x &&
        rect1.y <= rect2.y &&
        rect1.width >= rect2.width + (rect2.x - rect1.x) &&
        rect1.height >= rect2.height + (rect2.y - rect1.y);
    }

    function transformFrame(input, timestamp) {
        // First, find all the region that are ready to be redrawn
        const areasToDraw = []
        let totalPixelToDraw = 0;
        let zone_requesting_redraw = new Array()
        let zone_not_ready = new Array()
        _zones_needing_redraw.forEach(function(element, i) {
                const [zone, zoneTimestamp] = element;
                console.debug(`zone: ${zone} ${zoneTimestamp}`)
                if (zoneTimestamp <= timestamp) {
                    for (let i in zone_requesting_redraw) {
                        const existingZone = zone_requesting_redraw[i];
                        if (!existingZone) {
                        // Already nullified
                            continue
                        }
                        console.debug(`zone_requesting_redraw: ${i} ${existingZone}`)
                        if (existingZone === zone || contains_rect(existingZone, zone)) return;
                        if (contains_rect(zone, existingZone)) {
                            totalPixelToDraw -= zone_requesting_redraw[i].width * zone_requesting_redraw[i].height;
                            zone_requesting_redraw[i] = null;
                        }
                    }
                    totalPixelToDraw += zone.width * zone.height;
                    zone_requesting_redraw.push(zone);
                    if (zoneTimestamp == timestamp) zone_not_ready.push([zone, zoneTimestamp]);
                } else {
                    console.log(`Too soon to draw ${zone} (in ${zoneTimestamp-timestamp} cs)`)
                    zone_not_ready.push([zone, zoneTimestamp])
                }
        })
        _zones_needing_redraw = zone_not_ready

        // Depending of the state, force full redraw
        if (root.state === "Stop") {
            zone_requesting_redraw = [Qt.rect(0, 0, 320, 240)];
            totalPixelToDraw = 320 * 240;
        }

        // No redraw needed, stop right there
        if (!zone_requesting_redraw.length) {
            return new ArrayBuffer(0);
        }

        console.log(`Redrawing ${totalPixelToDraw} the following region: ${zone_requesting_redraw}`)

        const screenIdx = screenId === "leftdeck" ? 0 : 1;

        const outputData = new ArrayBuffer(totalPixelToDraw*2 + 20*zone_requesting_redraw.length); // Number of pixel + 20 (header/footer size) x the number of region
        let offset = 0;

        for (const area of zone_requesting_redraw) {
            if (!area) {
                // This happens when the area has been nullified because overlapping with an other region
                continue;
            }
            const header = new Uint8Array(outputData, offset, 16);
            const payload = new Uint8Array(outputData, offset + 16, area.width*area.height*2);
            const footer = new Uint8Array(outputData, offset + area.width*area.height*2 + 16, 4);

            header.fill(0)
            footer.fill(0)
            header[0] = 0x84;
            header[2] = screenIdx;
            header[3] = 0x21;

            header[8] = area.x >> 8;
            header[9] = area.x & 0xff;
            header[10] = area.y >> 8;
            header[11] = area.y & 0xff;

            header[12] = area.width >> 8;
            header[13] = area.width & 0xff;
            header[14] = area.height >> 8;
            header[15] = area.height & 0xff;

            if (area.x === 0 && area.width === 320) {
                payload.set(new Uint8Array(input, area.y * 320 * 2, area.width*area.height*2));
            } else {
                for (let line = 0; line < area.height; line++) {
                    payload.set(new Uint8Array(input,
                            ((area.y + line) * 320 + area.x) * 2,
                            area.width * 2),
                        line * area.width * 2);
                }
            }
            footer[0] = 0x40;
            footer[2] = screenIdx;
            offset += area.width*area.height*2 + 20
        }
        console.log(`Generated ${offset} bytes to be sent`)
        return outputData;
    }

    Component {
        id: splashOff
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

                    opacity: artworkSpacer.visible ? 1 : 0.2
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

                            scrolling: !scrollingWavefom.visible

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

                Repeater {
                    model: scrollingWavefom.visible ? [
                        "playposition",
                        "bpm",
                        "waveform_zoom",
                        "loop_start_position",
                        "loop_end_position",
                        "loop_enabled",
                        "total_gain",
                        "filterHigh",
                        "filterHighKill",
                        "filterMid",
                        "filterMidKill",
                        "filterLow",
                        "filterLowKill",
                        ...Array(16).fill().map((_,i)=>`hotcue_${i+1}_position`),
                        ...Array(16).fill().map((_,i)=>`hotcue_${i+1}_color`),
                    ]: []

                    Item {
                        required property string modelData

                        Mixxx.ControlProxy {
                            group: root.group
                            key: modelData
                            onValueChanged: (value) => {
                                console.log(`${modelData}: ${value}`)
                                root.redraw(scrollingWavefom)
                            }
                        }
                    }
                }

                Item {
                    id: scrollingWavefom

                    Layout.fillWidth: true
                    Layout.minimumHeight: scrollingWavefom.visible ? 120 : 0
                    Layout.leftMargin: 0
                    Layout.rightMargin: 0

                    visible: false

                    Skin.WaveformRow {
                        group: root.group
                        x: 0
                        width: 320
                        height: 100
                        zoomControlRatio: 200
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
                        model: 16

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

                            Mixxx.ControlProxy {
                                id: hotcueColor
                                group: root.group
                                key: `hotcue_${number}_color`
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
                            color: `#${(hotcueColor.value >> 16).toString(16).padStart(2, '0')}${((hotcueColor.value >> 8) & 255).toString(16).padStart(2, '0')}${(hotcueColor.value & 255).toString(16).padStart(2, '0')}`
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
        sourceComponent: splashOff
        onLoaded: {
            redraw(root)
        }
    }

    states: [
        State {
            name: "Live"
            PropertyChanges {
                target: loader
                sourceComponent: live
            }
        },
        State {
            name: "Stop"
            PropertyChanges {
                target: loader
                sourceComponent: splashOff
            }
        }
    ]
}
