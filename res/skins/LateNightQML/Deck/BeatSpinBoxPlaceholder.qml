import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

Item {
    id: root

    property bool interactive: false
    property int preferredWidth: 78
    property string valueText: "4"

    signal stepRequested(int delta)

    implicitHeight: 26
    implicitWidth: preferredWidth

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: "#0f0f0f"
    }
    BorderImage {
        anchors.fill: parent
        opacity: 1.0
        source: LateNightTheme.assetDeckBeatSpinBoxBorder

        border {
            bottom: 2
            left: 2
            right: 19
            top: 2
        }
    }
    RowLayout {
        anchors.bottomMargin: 2
        anchors.fill: parent
        anchors.leftMargin: 3
        anchors.rightMargin: 0
        spacing: 0

        Text {
            Layout.fillWidth: true
            color: LateNightTheme.deckBeatSpinBoxTextColor
            font.bold: true
            font.family: "Open Sans"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            text: root.valueText
            verticalAlignment: Text.AlignVCenter
        }
        Column {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 17
            spacing: 0

            Image {
                fillMode: Image.PreserveAspectFit
                height: 11
                source: LateNightTheme.assetDeckBeatSpinBoxUpButton
                visible: source.toString().length > 0
                width: parent.width
            }
            Image {
                fillMode: Image.PreserveAspectFit
                height: 11
                source: LateNightTheme.assetDeckBeatSpinBoxDownButton
                visible: source.toString().length > 0
                width: parent.width
            }
            Text {
                color: LateNightTheme.textColorMuted
                font.pixelSize: 8
                height: 11
                horizontalAlignment: Text.AlignHCenter
                text: "▲"
                visible: LateNightTheme.assetDeckBeatSpinBoxUpButton.toString().length <= 0
                width: parent.width
            }
            Text {
                color: LateNightTheme.textColorMuted
                font.pixelSize: 8
                height: 11
                horizontalAlignment: Text.AlignHCenter
                text: "▼"
                visible: LateNightTheme.assetDeckBeatSpinBoxDownButton.toString().length <= 0
                width: parent.width
            }
        }
    }
    TapHandler {
        enabled: root.interactive

        onTapped: (eventPoint, button) => {
            root.stepRequested(eventPoint.position.y < root.height / 2 ? 1 : -1);
        }
    }
    WheelHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        enabled: root.interactive

        onWheel: event => {
            if (event.angleDelta.y !== 0) {
                root.stepRequested(event.angleDelta.y > 0 ? 1 : -1);
            }
        }
    }
}
