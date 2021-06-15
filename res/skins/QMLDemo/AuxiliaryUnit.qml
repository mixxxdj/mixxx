import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Row {
    id: root

    property int unitNumber // required
    property string group: "[Auxiliary" + unitNumber + "]"

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
        Text {
            width: parent.width
            height: root.height / 2
            text: "AUX " + root.unitNumber
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            font.bold: true
            color: Theme.buttonNormalColor
        }

        Skin.ControlButton {
            id: pflButton

            group: root.group
            key: "pfl"
            text: "PFL"
            activeColor: Theme.pflActiveButtonColor
            toggleable: true
        }

    }

    Skin.EmbeddedBackground {
        id: embedded

        height: parent.height
        width: 56

        Skin.OrientationToggleButton {
            id: orientationButton

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.verticalCenter
            group: root.group
            key: "orientation"
            color: Theme.crossfaderOrientationColor
        }

        Skin.InfoBarButton {
            id: fx1Button

            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            group: "[EffectRack1_EffectUnit1]"
            key: "group_" + root.group + "_enable"
            activeColor: Theme.deckActiveColor

            foreground: Text {
                anchors.centerIn: parent
                text: "FX1"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: Theme.fontFamily
                font.pixelSize: Theme.buttonFontPixelSize
                font.bold: true
                color: Theme.deckTextColor
            }

        }

        Skin.InfoBarButton {
            group: "[EffectRack1_EffectUnit2]"
            anchors.left: parent.horizontalCenter
            anchors.right: parent.right
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            key: "group_" + root.group + "_enable"
            activeColor: Theme.deckActiveColor

            foreground: Text {
                anchors.centerIn: parent
                text: "FX2"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: Theme.fontFamily
                font.pixelSize: Theme.buttonFontPixelSize
                font.bold: true
                color: Theme.deckTextColor
            }

        }

    }

}
