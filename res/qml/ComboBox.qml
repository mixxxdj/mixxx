import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"

ComboBox {
    id: root

    property alias popupWidth: popupItem.width
    property int popupMaxItems: 6
    property bool clip: false
    property list<var> footerItems: []

    signal activateFooter(int index)

    background: Item {
        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            id: background
            border.width: 1
            border.color: '#000000'
            radius: 4
            color: '#232323'
        }
        InnerShadow {
            id: bottomInnerEffect
            anchors.fill: parent
            radius: 4
            samples: 16
            spread: 0.3
            horizontalOffset: -2
            verticalOffset: -2
            color: "#40000000"
            source: background
        }
        InnerShadow {
            id: topInnerEffect
            anchors.fill: parent
            radius: 4
            samples: 16
            spread: 0.3
            horizontalOffset: 2
            verticalOffset: 2
            color: "#40000000"
            source: bottomInnerEffect
        }

        DropShadow {
            anchors.fill: parent
            horizontalOffset: 0
            verticalOffset: 0
            radius: 4.0
            color: "#40000000"
            source: topInnerEffect
        }
    }

    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        width: root.width
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
        id: popupItem
        y: root.height/2
        width: root.width
        height: root.contentItem.height*Math.min(root.popupMaxItems, Math.max(root.count, 1)) + 20

        padding: 0

        contentItem: Item {
            Item {
                id: content
                anchors.fill: parent
                Shape {
                    id: listIndicator
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: 3
                    width: 20
                    height: 20
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: Theme.embeddedBackgroundColor
                        strokeColor: Theme.embeddedBackgroundColor
                        strokeWidth: 2
                        startX: listIndicator.width/2; startY: 0
                        fillRule: ShapePath.WindingFill
                        capStyle: ShapePath.RoundCap
                        PathLine { x: listIndicator.width; y: listIndicator.height }
                        PathLine { x: 0; y: listIndicator.height }
                        PathLine { x: listIndicator.width/2; y: 0 }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.top: listIndicator.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true
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

                        footer: Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            Repeater {
                                model: root.footerItems
                                Rectangle {
                                    color: "transparent"
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    height: root.contentItem.height

                                    Item {
                                        anchors.fill: parent
                                        anchors.margins: 6
                                        Text {
                                            anchors.top: parent.top
                                            anchors.bottom: parent.bottom
                                            text: modelData.text
                                            font: root.font
                                            color: Theme.deckTextColor
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        Text {
                                            anchors.top: parent.top
                                            anchors.bottom: parent.bottom
                                            anchors.right: parent.right
                                            text: modelData?.suffix || ''
                                            font.pixelSize: 16
                                            color: Theme.deckTextColor
                                        }
                                    }

                                    MouseArea {
                                        id: footerItemMouseArea
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onPressed: {
                                            root.activateFooter(index)
                                        }
                                    }
                                }
                            }
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
