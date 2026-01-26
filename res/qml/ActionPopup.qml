import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.12
import Qt5Compat.GraphicalEffects
import "Theme"

Popup {
    id: root
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    width: 200

    padding: 0
    margins: 0
    leftInset: 0

    enum Facing {
        Left,
        Top,
        Right,
        Bottom
    }

    default property alias children: content.children
    property variant facing: ActionPopup.Facing.Left

    contentItem: Item {
        ColumnLayout {
            spacing: 2
            anchors.fill: parent
            anchors.leftMargin: root.facing == ActionPopup.Facing.Left ? 20 : 0
            anchors.rightMargin: root.facing == ActionPopup.Facing.Right ? 20 : 0
            id: content
        }
    }

    background: Item {
        Item {
            id: content3
            anchors.fill: parent
            Shape {
                anchors.right: root.facing == ActionPopup.Facing.Right ? parent.right : undefined
                anchors.left: root.facing == ActionPopup.Facing.Left ? parent.left : undefined
                anchors.verticalCenter: parent.verticalCenter
                implicitHeight: 20
                ShapePath {
                    strokeWidth: 0
                    strokeColor: 'transparent'
                    fillColor: Theme.backgroundColor
                    fillRule: ShapePath.OddEvenFill

                    startX: 0
                    startY: 10
                    PathLine { x: 20; y: 0 }
                    PathLine { x: 20; y: 20 }
                    PathLine { x: 0; y: 10 }
                }
                transformOrigin: Item.Center
                rotation: {
                    switch (root.facing) {
                        case ActionPopup.Facing.Right:
                            return 180
                        default:
                            return 0
                    }
                }
            }
            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: root.facing == ActionPopup.Facing.Left ? 20 : 0
                anchors.rightMargin: root.facing == ActionPopup.Facing.Right ? 20 : 0
                border.width: 0
                radius: 8
                color: Theme.backgroundColor
            }
        }
        DropShadow {
            anchors.fill: parent
            source: content3
            horizontalOffset: 0
            verticalOffset: 0
            radius: 8.0
            color: "#80000000"
        }
    }
}
