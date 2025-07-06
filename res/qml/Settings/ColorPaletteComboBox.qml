import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Skin.ComboBox {
    id: root
    implicitWidth: Math.max.apply(null, model.map(palette => fontMetrics.advanceWidth(palette) + 30 + Mixxx.Config.colorPalette(palette).length * 8 + indicator.width))
    readonly property var currentPalette: Mixxx.Config.colorPalette(root.displayText)
    property bool canDisable: false
    model: Mixxx.Config.paletteNames()

    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        width: parent.width
        height: 30
        highlighted: root.highlightedIndex === this.index
        text: root.textAt(this.index)

        contentItem: RowLayout {
            anchors.topMargin: 8
            anchors.leftMargin: 10
            anchors.rightMargin: 40
            anchors.fill: parent
            Text {
                Layout.fillWidth: true
                text: itemDlgt.text
                color: Theme.deckTextColor
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            Row {
                anchors.topMargin: 5
                Repeater {
                    model: Mixxx.Config.colorPalette(itemDlgt.text)
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
