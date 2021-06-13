import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Item {
    property int unitNumber // required

    Skin.SectionBackground {
        anchors.fill: parent
    }

    RowLayout {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: effectSuperKnobFrame.left
        anchors.rightMargin: 5

        Repeater {
            model: 3

            Item {
                id: effect

                property string group: "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (index + 1) + "]"

                height: 50
                Layout.fillWidth: true

                Skin.ControlButton {
                    id: effectEnableButton

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: 5
                    width: 40
                    group: parent.group
                    key: "enabled"
                    toggleable: true
                    text: "ON"
                    activeColor: Theme.effectColor
                }

                Skin.ComboBox {
                    id: effectSelector

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: effectEnableButton.right
                    anchors.right: effectMetaKnob.left
                    anchors.margins: 5
                    // TODO: Add a way to retrieve effect names here
                    model: ["---", "Effect 1", "Effect 2", "Effect 3", "Effect 4"]
                }

                Skin.ControlMiniKnob {
                    id: effectMetaKnob

                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: 5
                    arcStart: 0
                    width: 40
                    group: parent.group
                    key: "meta"
                    color: Theme.effectColor
                }

            }

        }

    }

    Rectangle {
        id: effectSuperKnobFrame

        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        color: Theme.knobBackgroundColor
        radius: 5

        Skin.ControlKnob {
            id: effectSuperKnob

            anchors.centerIn: parent
            width: 48
            height: 48
            arcStart: 0
            group: "[EffectRack1_EffectUnit" + unitNumber + "]"
            key: "super1"
            color: Theme.effectUnitColor
        }

    }

}
