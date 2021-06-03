import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Item {
    id: root

    property string group // required

    height: buttonColumn.implicitHeight

    Column {
        id: buttonColumn

        anchors.left: parent.left
        width: 56
        spacing: 5

        Skin.ControlButton {
            id: cueButton

            group: root.group
            key: "cue_default"
            text: "Cue"
            activeColor: Theme.deckActiveColor
        }

        Skin.ControlButton {
            id: playButton

            group: root.group
            key: "play"
            text: "Play"
            checkable: true
            activeColor: Theme.deckActiveColor
        }

    }

    Skin.WaveformOverview {
        group: root.group
        anchors.top: buttonColumn.top
        anchors.bottom: buttonColumn.bottom
        anchors.left: buttonColumn.right
        anchors.right: parent.right
    }

}
