import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Popup {
    id: root

    property Hotcue hotcue // required

    dim: false
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
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
                        hotcue.setColor(parent.color);
                        root.close();
                    }
                }

            }

        }

    }

    Skin.Button {
        id: clearButton

        anchors.top: colorGrid.bottom
        anchors.left: parent.left
        anchors.topMargin: 5
        text: "Clear"
        activeColor: Theme.deckActiveColor
        onDownChanged: hotcue.clear = down
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
        source: Theme.imgPopupBackground

        border {
            top: 10
            left: 20
            right: 20
            bottom: 10
        }

    }

}
