import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"

ComboBox {
    id: root

    property bool clip: false
    property list<var> footerItems: []
    property int popupMaxItem: 6
    property alias popupWidth: popupItem.width

    signal activateFooter(int index)

    indicator.width: 20

    background: Item {
        Rectangle {
            id: background

            anchors.fill: parent
            anchors.margins: 4
            border.color: '#000000'
            border.width: 1
            color: '#232323'
            radius: 4
        }
        InnerShadow {
            id: bottomInnerEffect

            anchors.fill: parent
            color: "#40000000"
            horizontalOffset: -2
            radius: 4
            samples: 16
            source: background
            spread: 0.3
            verticalOffset: -2
        }
        InnerShadow {
            id: topInnerEffect

            anchors.fill: parent
            color: "#40000000"
            horizontalOffset: 2
            radius: 4
            samples: 16
            source: bottomInnerEffect
            spread: 0.3
            verticalOffset: 2
        }
        DropShadow {
            anchors.fill: parent
            color: "#40000000"
            horizontalOffset: 0
            radius: 4.0
            source: topInnerEffect
            verticalOffset: 0
        }
    }
    contentItem: Text {
        clip: root.clip
        color: Theme.deckTextColor
        elide: root.clip ? Text.ElideNone : Text.ElideRight
        font: root.font
        leftPadding: 5
        text: root.displayText
        verticalAlignment: Text.AlignVCenter
    }
    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        highlighted: root.highlightedIndex === this.index
        padding: 4
        text: root.textAt(this.index)
        verticalPadding: 8
        width: root.width

        background: Rectangle {
            border.color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
            border.width: 1
            color: "transparent"
            radius: 5
        }
        contentItem: Text {
            color: Theme.deckTextColor
            elide: Text.ElideRight
            font: root.font
            text: itemDlgt.text
            verticalAlignment: Text.AlignVCenter
        }
    }
    popup: Popup {
        id: popupItem

        height: root.contentItem.height * Math.min(root.popupMaxItem, Math.max(root.count, 1)) + 20
        padding: 0
        width: root.width
        x: root.width - width
        y: root.height - 4

        background: Item {
        }
        contentItem: Item {
            Item {
                id: content

                anchors.fill: parent

                Shape {
                    id: listIndicator

                    anchors.right: parent.right
                    anchors.rightMargin: 3
                    anchors.top: parent.top
                    antialiasing: true
                    height: 20
                    layer.enabled: true
                    layer.samples: 4
                    width: 20

                    ShapePath {
                        capStyle: ShapePath.RoundCap
                        fillColor: Theme.embeddedBackgroundColor
                        fillRule: ShapePath.WindingFill
                        startX: listIndicator.width / 2
                        startY: 0
                        strokeColor: Theme.embeddedBackgroundColor
                        strokeWidth: 2

                        PathLine {
                            x: listIndicator.width
                            y: listIndicator.height
                        }
                        PathLine {
                            x: 0
                            y: listIndicator.height
                        }
                        PathLine {
                            x: listIndicator.width / 2
                            y: 0
                        }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: listIndicator.bottom
                    clip: true

                    ListView {
                        anchors.fill: parent
                        bottomMargin: 0
                        clip: true
                        currentIndex: root.highlightedIndex
                        leftMargin: 0
                        model: root.popup.visible ? root.delegateModel : null
                        rightMargin: 0
                        topMargin: 0

                        ScrollIndicator.vertical: ScrollIndicator {
                        }
                        footer: Item {
                            anchors.left: parent.left
                            anchors.right: parent.right

                            Repeater {
                                model: root.footerItems

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    color: "transparent"
                                    height: root.contentItem.height

                                    Item {
                                        anchors.fill: parent
                                        anchors.margins: 6

                                        Text {
                                            anchors.bottom: parent.bottom
                                            anchors.top: parent.top
                                            color: Theme.deckTextColor
                                            elide: Text.ElideRight
                                            font: root.font
                                            text: modelData.text
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        Text {
                                            anchors.bottom: parent.bottom
                                            anchors.right: parent.right
                                            anchors.top: parent.top
                                            color: Theme.deckTextColor
                                            font.pixelSize: 16
                                            text: modelData?.suffix || ''
                                        }
                                    }
                                    MouseArea {
                                        id: footerItemMouseArea

                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor

                                        onPressed: {
                                            root.activateFooter(index);
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
                color: "#000000"
                horizontalOffset: 0
                radius: 8.0
                source: content
                verticalOffset: 0
            }
        }
    }
}
