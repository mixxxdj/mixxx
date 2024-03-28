/*
This module is used to draw an overlay on the waveform overview in order to highlight better the playback progression.
As the native Mixxx QML component involves, this component might become redundant and should be replaces with native modules.
*/
import QtQuick 2.15
import QtQuick.Window 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string group

    property real windowWidth: Window.width

    width: 0
    signal updated

    Mixxx.ControlProxy {
        group: root.group
        key: "track_loaded"
        onValueChanged: (value) => {
            if (value === root.visible) return;
            root.visible = value
            root.updated()
        }
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "playposition"
        onValueChanged: (value) => {
            const newValue = Math.round(value * (320 - 12));
            if (newValue === root.width) return;
            root.width = newValue;
            root.updated()
        }
    }

    clip: true

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: -border.width
        anchors.topMargin: -border.width
        anchors.bottomMargin: -border.width
        border.width: 2
        border.color:"black"
        color: Qt.rgba(0.39, 0.80, 0.96, 0.3)
    }
}
