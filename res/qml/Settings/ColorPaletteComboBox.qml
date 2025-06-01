import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Skin.ComboBox {
    id: root
    implicitWidth: fontMetrics.advanceWidth(root.displayText) + 30 + currentPalette.length * 8 + indicator.width
    readonly property var currentPalette: Mixxx.Config.getHotcueColorPalette(root.displayText)
    model: [
            "Mixxx Hotcue Colors",
            "Serato DJ Pro Hotcue Colors",
            "Rekordbox COLD1 Hotcue Colors",
            "Rekordbox COLD2 Hotcue Colors",
            "Rekordbox COLORFUL Hotcue Colors"
    ]
    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        width: parent.width
        highlighted: root.highlightedIndex === this.index
        text: root.textAt(this.index)

        contentItem: RowLayout {
            anchors.leftMargin: 10
            anchors.rightMargin: 20
            anchors.fill: parent
            Text {
                Layout.fillWidth: true
                text: itemDlgt.text
                color: Theme.deckTextColor
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            Row {
                Repeater {
                    model: Mixxx.Config.getHotcueColorPalette(itemDlgt.text)
                    Rectangle {
                        required property color modelData

                        color: modelData
                        width: 8
                        height: 10
                    }
                }
            }
        }

        background: Rectangle {
            radius: 5
            border.width: itemDlgt.highlighted ? 1 : 0
            border.color: Theme.deckLineColor
            color: "transparent"
        }
    }

    FontMetrics {
        id: fontMetrics
        font: root.font
    }

    contentItem: RowLayout {
        anchors.leftMargin: 10
        anchors.rightMargin: 20
        anchors.fill: parent
        Text {
            Layout.fillWidth: true
            text: root.displayText
            color: Theme.deckTextColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        Row {
            Repeater {
                model: root.currentPalette
                Rectangle {
                    required property color modelData

                    color: modelData
                    width: 8
                    height: 10
                }
            }
        }
    }
}
