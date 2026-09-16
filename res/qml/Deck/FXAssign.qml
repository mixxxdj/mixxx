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

            AnchorChanges {
                anchors.bottom: undefined
                anchors.top: parent.top
                target: root
            }
        },
        State {
            when: holder.index != 0

            AnchorChanges {
                // anchors.bottom: parent.bottom
                anchors.top: undefined
                target: root
            }
        }
    ]
    transitions: Transition {
        AnchorAnimation {
            duration: 250
        }
    }

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: '#626262'
        font.pixelSize: 8
        text: "FX Assign"
    }
    RowLayout {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Repeater {
            model: 4

            Skin.ControlButton {
                id: fx1AssignButton

                required property int index

                Layout.maximumWidth: 30
                Layout.minimumWidth: 22
                activeColor: Theme.deckActiveColor
                group: `[EffectRack1_EffectUnit${index + 1}]`
                implicitHeight: 22
                key: `group_${root.group}_enable`
                text: `${index + 1}`
                toggleable: true
            }
        }
    }
}
