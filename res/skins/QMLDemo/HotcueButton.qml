import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Skin.Button {
    id: root

    property int hotcueNumber // required
    property string group // required

    text: hotcueNumber
    width: playButton.height
    height: playButton.height
    activeColor: {
        if (hotcueColorControl.value < 0)
            return Theme.deckActiveColor;

        return "#" + hotcueColorControl.value.toString(16).padStart(6, "0");
    }
    highlight: hotcueControl.value

    Mixxx.ControlProxy {
        id: hotcueColorControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_color"
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "hotcue_" + hotcueNumber + "_activate"
        value: root.down
    }

    Mixxx.ControlProxy {
        id: hotcueControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_enabled"
    }

    Mixxx.ControlProxy {
        id: hotcueClearControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_clear"
    }

    MouseArea {
        id: mousearea

        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            popup.x = mouse.x;
            popup.y = mouse.y;
            popup.open();
        }
    }

    Popup {
        id: popup

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        Text {
            text: "Cue Popup Placeholder"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            color: Theme.deckTextColor
            anchors.centerIn: parent
        }

        background: BorderImage {
            id: backgroundImage

            anchors.fill: parent
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            source: "images/button.svg"

            border {
                top: 10
                left: 20
                right: 20
                bottom: 10
            }

        }

    }

}
