import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string group
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)

    enum TimerStatus {
        Forward,
        Cooldown,
        Backward
    }

    signal updated

    property string fullText
    property int index

    Connections {
        function onGroupChanged(group) {
            timer.stop()

            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group)

            deckPlayer.artistChanged.connect(onAir.update)
            deckPlayer.titleChanged.connect(onAir.update)

            root.update()
        }
    }

    Rectangle {
        id: frame
        anchors.top: root.top
        anchors.bottom: root.bottom
        width: 1000
        x: 6
        color: 'transparent'

        Text {
            id: text
            text: qsTr("No Track Loaded")
            font.pixelSize: 24
            font.family: "Noto Sans"
            font.letterSpacing: -1
            color: fontColor
        }
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"

        onValueChanged: (value) => {
            timer.stop()

            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group)

            deckPlayer.artistChanged.connect(onAir.update)
            deckPlayer.titleChanged.connect(onAir.update)
        }
    }

    function update() {
        let newTitle = `${root.deckPlayer.title} - ${root.deckPlayer.artist}`.trim()
        frame.width = newTitle.length * 12
        if (newTitle.length > 29) {
            frame.x = 6
            timer.status = OnAirTrack.TimerStatus.Cooldown
            timer.backward = false
            timer.interval = 2000
            timer.start()
        } else {
            timer.stop()
        }
        if (newTitle == "-" && !trackLoadedControl.value) {
            newTitle = qsTr("No Track Loaded")
        } else if (newTitle == "-") {
            console.warn(`No track title found for ${root.deckPlayer.title}`)
        }
        if (text.text == newTitle) return;
        text.text = newTitle;
        root.updated()
    }

    Timer {
        id: timer

        property int status: OnAirTrack.TimerStatus.Cooldown
        property bool backward: false

        interval: 2000
        repeat: true
        running: false

        onTriggered: {
            if (status = OnAirTrack.TimerStatus.Cooldown) {
                status += backward ? -1 : 1
                interval = 15
            }
            frame.x -= backward ? -1 : 1;
            root.updated()
            if (-frame.x >= (text.text.length - 29) * 11) {
                backward = true
                status = OnAirTrack.TimerStatus.Cooldown
                interval = 2000
            } else if (frame.x >= 6) {
                backward = false
                status = OnAirTrack.TimerStatus.Cooldown
                interval = 2000
            }
        }
    }
}
