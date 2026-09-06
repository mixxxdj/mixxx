import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Skin.ComboBox {
    id: root

    property bool canDisable: false
    readonly property var currentPalette: Mixxx.Config.colorPalette(root.displayText)

    implicitWidth: Math.max.apply(null, model.map(palette => fontMetrics.advanceWidth(palette) + 30 + Mixxx.Config.colorPalette(palette).length * 8 + indicator.width))
    model: Mixxx.Config.paletteNames()

    contentItem: RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 20

        Text {
            Layout.fillWidth: true
            color: Theme.deckTextColor
            elide: Text.ElideRight
            text: root.displayText
            verticalAlignment: Text.AlignVCenter
        }
        Row {
            Repeater {
                model: root.currentPalette

                Rectangle {
                    required property color modelData

                    color: modelData
                    height: 10
                    width: 8
                }
            }
        }
    }
    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        height: 30
        highlighted: root.highlightedIndex === this.index
        text: root.textAt(this.index)
        width: parent.width

        background: Rectangle {
            border.color: Theme.deckLineColor
            border.width: itemDlgt.highlighted ? 1 : 0
            color: "transparent"
            radius: 5
        }
        contentItem: RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 40
            anchors.topMargin: 8

            Text {
                Layout.fillWidth: true
                color: Theme.deckTextColor
                elide: Text.ElideRight
                text: itemDlgt.text
                verticalAlignment: Text.AlignVCenter
            }
            Row {
                anchors.topMargin: 5

                Repeater {
                    model: Mixxx.Config.colorPalette(itemDlgt.text)

                    Rectangle {
                        required property color modelData

                        color: modelData
                        height: 10
                        width: 8
                    }
                }
            }
        }
    }

    FontMetrics {
        id: fontMetrics

        font: root.font
    }
}
