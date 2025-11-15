import ".." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts
import "../Theme"

Item {
    id: root

    required property string group

    height: 35
    width: 135

    states: [
        State {
            when: holder.index == 0
            AnchorChanges { target: root; anchors.top: parent.top; anchors.bottom: undefined }
        },
        State {
            when: holder.index != 0
            AnchorChanges { target: root; anchors.top: undefined; anchors.bottom: parent.bottom }
        }
    ]

    transitions: Transition {
        AnchorAnimation { duration: 250 }
    }

    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "FX Assign"
        font.pixelSize: 8
        color: '#626262'
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Repeater {
            model: 4
            Skin.ControlButton {
                required property int index
                Layout.maximumWidth: 30
                Layout.minimumWidth: 22
                id: fx1AssignButton

                implicitHeight: 22

                group: `[EffectRack1_EffectUnit${index+1}]`
                key: `group_${root.group}_enable`
                text: `${index+1}`
                toggleable: true
                activeColor: Theme.deckActiveColor
            }
        }
    }
}
