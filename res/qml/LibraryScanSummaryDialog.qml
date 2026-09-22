import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme"

Dialog {
    id: root

    property string informativeText: ""
    property string text: ""

    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape
    dim: true
    focus: true
    header: null
    modal: true
    padding: 24

    Overlay.modal: Rectangle {
        color: "#80000000"
    }

    background: Rectangle {
        id: bgRect

        color: Theme.backgroundColor
        border.color: Theme.darkGray3
        border.width: 1
        radius: 10

        layer.enabled: true
        layer.effect: DropShadow {
            color: "#80000000"
            horizontalOffset: 0
            radius: 12.0
            samples: 24
            verticalOffset: 4
        }
    }

    contentItem: ColumnLayout {
        id: contentLayout

        spacing: 16

        Label {
            id: textLabel

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            color: Theme.white
            font.bold: true
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize + 1
            horizontalAlignment: Text.AlignHCenter
            text: root.text
            visible: text.length > 0
            wrapMode: Text.WordWrap
        }

        Label {
            id: informativeTextLabel

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            color: Theme.white
            font.bold: true
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.4
            text: root.informativeText
            visible: text.length > 0
            wrapMode: Text.WordWrap
        }

        Button {
            id: okButton

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            implicitHeight: 32
            implicitWidth: 84
            text: qsTr("OK")

            background: Rectangle {
                id: buttonBg

                color: okButton.down
                    ? Qt.darker(Theme.accentColor, 1.2)
                    : (okButton.hovered ? Qt.lighter(Theme.accentColor, 1.15) : Theme.accentColor)
                radius: 6

                Behavior on color {
                    ColorAnimation {
                        duration: 100
                    }
                }
            }

            contentItem: Text {
                color: "#ffffff"
                font.bold: true
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textFontPixelSize
                horizontalAlignment: Text.AlignHCenter
                text: okButton.text
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: root.accept()
        }
    }

    onOpened: {
        okButton.forceActiveFocus();
    }

    Connections {
        function onLibraryScanSummaryAvailable(title, text, informativeText) {
            root.title = title;
            root.text = text;
            root.informativeText = informativeText;
            root.open();
        }

        target: Mixxx.Library
    }
}
