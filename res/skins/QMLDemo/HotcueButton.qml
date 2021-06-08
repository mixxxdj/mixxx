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
    activeColor: hotcueColorControl.color
    highlight: hotcueStatusControl.value

    Mixxx.ControlProxy {
        id: hotcueColorControl

        readonly property color color: {
            if (hotcueColorControl.value < 0)
                return Theme.deckActiveColor;

            return "#" + hotcueColorControl.value.toString(16).padStart(6, "0");
        }

        function setColor(newColor) {
            value = (parseInt(newColor.r * 255) << 16) | (parseInt(newColor.g * 255) << 8) | parseInt(newColor.b * 255);
        }

        group: root.group
        key: "hotcue_" + hotcueNumber + "_color"
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "hotcue_" + hotcueNumber + "_activate"
        value: root.down
    }

    Mixxx.ControlProxy {
        id: hotcueStatusControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_enabled"
        onValueChanged: {
            if (hotcueStatusControl.value == 0)
                popup.close();

        }
    }

    MouseArea {
        id: mousearea

        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            if (hotcueStatusControl.value == 0)
                return ;

            popup.x = mouse.x;
            popup.y = mouse.y;
            popup.open();
        }
    }

    Popup {
        id: popup

        dim: false
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        contentWidth: colorGrid.implicitWidth
        contentHeight: colorGrid.implicitHeight + clearButton.implicitHeight

        Grid {
            id: colorGrid

            columns: 4
            spacing: 2

            Repeater {
                model: Mixxx.Config.getHotcueColorPalette()

                Rectangle {
                    height: 24
                    width: 24
                    color: modelData

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            hotcueColorControl.setColor(parent.color);
                            popup.close();
                        }
                    }

                }

            }

        }

        Skin.ControlButton {
            id: clearButton

            anchors.top: colorGrid.bottom
            anchors.left: parent.left
            anchors.topMargin: 5
            group: root.group
            key: "hotcue_" + hotcueNumber + "_clear"
            text: "Clear"
            activeColor: Theme.deckActiveColor
        }

        enter: Transition {
            NumberAnimation {
                properties: "opacity"
                from: 0
                to: 1
                duration: 100
            }

        }

        exit: Transition {
            NumberAnimation {
                properties: "opacity"
                from: 1
                to: 0
                duration: 100
            }

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
