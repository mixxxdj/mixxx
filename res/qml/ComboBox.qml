import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"

ComboBox {
    id: root

    property alias popupWidth: popup.width
    property bool clip: false

    background: Skin.EmbeddedBackground {
    }

    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        highlighted: root.highlightedIndex === this.index
        text: root.textAt(this.index)
        padding: 4
        verticalPadding: 8

        contentItem: Text {
            text: itemDlgt.text
            font: root.font
            color: Theme.deckTextColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            radius: 5
            border.width: 1
            border.color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
            color: "transparent"
        }
    }

    indicator.width: 20

    contentItem: Text {
        leftPadding: 5
        text: root.displayText
        font: root.font
        color: Theme.deckTextColor
        verticalAlignment: Text.AlignVCenter
        elide: root.clip ? Text.ElideNone : Text.ElideRight
        clip: root.clip
    }

    popup: Popup {
        id: popup
        y: root.height/2
        width: root.width - root.indicator.width / 2
        x: root.indicator.width / 2
        height: Math.min(root.indicator.implicitHeight*3 + root.indicator.width, 150)

        padding: 0

        contentItem: Item {
            // implicitHeight: contentHeight
            Item {
                id: content
                anchors.fill: parent
                Shape {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: 3
                    width: root.indicator.width-3
                    height: width
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: Theme.embeddedBackgroundColor
                        strokeColor: Theme.deckBackgroundColor
                        strokeWidth: 2
                        startX: parent.width/2; startY: 0
                        fillRule: ShapePath.WindingFill
                        capStyle: ShapePath.RoundCap
                        PathLine { x: root.indicator.width; y: root.indicator.width }
                        PathLine { x: 0; y: root.indicator.width }
                        PathLine { x: (root.indicator.width) / 2; y: 0 }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.topMargin: root.indicator.width
                    anchors.fill: parent
                    ListView {
                        clip: true

                        anchors.fill: parent

                        bottomMargin: 0
                        leftMargin: 0
                        rightMargin: 0
                        topMargin: 0

                        model: root.popup.visible ? root.delegateModel : null
                        currentIndex: root.highlightedIndex

                        ScrollIndicator.vertical: ScrollIndicator {
                        }
                    }
                }
            }
            DropShadow {
                anchors.fill: parent
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8.0
                color: "#000000"
                source: content
            }
        }

        background: Item {}
    }
}
