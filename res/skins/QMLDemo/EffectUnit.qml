import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Item {
    id: root

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

            EffectSlot {
                unitNumber: root.unitNumber
                effectNumber: index + 1
                Layout.fillWidth: true
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
