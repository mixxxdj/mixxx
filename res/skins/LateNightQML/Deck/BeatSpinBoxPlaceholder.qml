import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

Item {
    id: root

    property string valueText: "4"
    property int preferredWidth: 78

    implicitWidth: preferredWidth
    implicitHeight: 26

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: "#0f0f0f"
    }

    BorderImage {
        anchors.fill: parent
        source: LateNightTheme.assetDeckBeatSpinBoxBorder
        border {
            top: 2
            left: 2
            right: 19
            bottom: 2
        }
        opacity: 1.0
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 3
        anchors.rightMargin: 0
        anchors.bottomMargin: 2
        spacing: 0

        Text {
            Layout.fillWidth: true
            text: root.valueText
            font.family: "Open Sans"
            font.pixelSize: 13
            font.bold: true
            color: LateNightTheme.deckBeatSpinBoxTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Column {
            Layout.preferredWidth: 17
            Layout.alignment: Qt.AlignVCenter
            spacing: 0

            Image {
                width: parent.width
                height: 11
                source: LateNightTheme.assetDeckBeatSpinBoxUpButton
                fillMode: Image.PreserveAspectFit
                visible: source.toString().length > 0
            }

            Image {
                width: parent.width
                height: 11
                source: LateNightTheme.assetDeckBeatSpinBoxDownButton
                fillMode: Image.PreserveAspectFit
                visible: source.toString().length > 0
            }

            Text {
                width: parent.width
                height: 11
                text: "▲"
                font.pixelSize: 8
                color: LateNightTheme.textColorMuted
                horizontalAlignment: Text.AlignHCenter
                visible: LateNightTheme.assetDeckBeatSpinBoxUpButton.toString().length <= 0
            }

            Text {
                width: parent.width
                height: 11
                text: "▼"
                font.pixelSize: 8
                color: LateNightTheme.textColorMuted
                horizontalAlignment: Text.AlignHCenter
                visible: LateNightTheme.assetDeckBeatSpinBoxDownButton.toString().length <= 0
            }
        }
    }
}
