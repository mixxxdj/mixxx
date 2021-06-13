import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Row {
    id: root

    property string group // required

    spacing: 5

    Skin.VuMeter {
        id: vuMeter

        group: root.group
        key: "VuMeter"
        width: 4
        height: parent.height
    }

    Rectangle {
        id: gainKnobFrame

        width: 52
        height: width
        color: Theme.knobBackgroundColor
        radius: 5

        Skin.ControlKnob {
            id: gainKnob

            anchors.centerIn: parent
            width: 48
            height: width
            arcStart: 0
            group: root.group
            key: "pregain"
            color: Theme.gainKnobColor
        }

    }

    Column {
        Skin.ControlButton {
            id: pflButton

            group: root.group
            key: "pfl"
            text: "PFL"
            activeColor: Theme.pflActiveButtonColor
            toggleable: true
        }

        Skin.ControlButton {
            id: talkButton

            group: root.group
            key: "talkover"
            text: "Talk"
            activeColor: Theme.pflActiveButtonColor
            toggleable: true
        }

    }

}
