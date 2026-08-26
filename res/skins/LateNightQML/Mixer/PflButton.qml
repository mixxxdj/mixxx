import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property string key: "pfl"

    implicitHeight: 26
    implicitWidth: 26

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: pflControl.value > 0 ? LateNightTheme.mixerPflActiveFillColor : LateNightTheme.deckEmbeddedButtonInactiveColor
    }
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        source: LateNightTheme.assetMixerPflBackground
    }
    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        height: 26
        source: pflControl.value > 0 ? LateNightTheme.assetMixerPflActiveIcon : LateNightTheme.assetMixerPflIcon
        width: 26
    }
    TapHandler {
        onTapped: pflControl.value = pflControl.value > 0 ? 0 : 1
    }
    Mixxx.ControlProxy {
        id: pflControl

        group: root.group
        key: root.key
    }
}
