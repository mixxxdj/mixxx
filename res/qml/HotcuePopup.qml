import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Popup {
    id: root

    required property Hotcue hotcue

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    contentHeight: colorGrid.implicitHeight + clearButton.implicitHeight
    contentWidth: colorGrid.implicitWidth
    dim: false
    focus: true
    modal: true

    background: BorderImage {
        id: backgroundImage

        anchors.fill: parent
        horizontalTileMode: BorderImage.Stretch
        source: Theme.imgPopupBackground
        verticalTileMode: BorderImage.Stretch

        border {
            bottom: 10
            left: 20
            right: 20
            top: 10
        }
    }
    enter: Transition {
        NumberAnimation {
            duration: 100
            from: 0
            properties: "opacity"
            to: 1
        }
    }
    exit: Transition {
        NumberAnimation {
            duration: 100
            from: 1
            properties: "opacity"
            to: 0
        }
    }

    Grid {
        id: colorGrid

        columns: 4
        spacing: 2

        Repeater {
            model: Mixxx.Config.hotcueColorPalette

            Rectangle {
                required property color modelData

                color: modelData
                height: 24
                width: 24

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        root.hotcue.setColor(parent.color);
                        root.close();
                    }
                }
            }
        }
    }
    Skin.Button {
        id: clearButton

        activeColor: Theme.deckActiveColor
        anchors.left: parent.left
        anchors.top: colorGrid.bottom
        anchors.topMargin: 5
        text: "Clear"

        onPressed: root.hotcue.clear = 1
        onReleased: root.hotcue.clear = 0
    }
}
