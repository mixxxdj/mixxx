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

    enum Facing {
        Left,
        Top,
        Right,
        Bottom
    }

    default property alias children: content.children
    property variant facing: ActionPopup.Facing.Left

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    leftInset: 0
    margins: 0
    padding: 0
    width: 200

    background: Item {
        Item {
            id: content3

            anchors.fill: parent

            Shape {
                anchors.left: root.facing == ActionPopup.Facing.Left ? parent.left : undefined
                anchors.right: root.facing == ActionPopup.Facing.Right ? parent.right : undefined
                anchors.verticalCenter: parent.verticalCenter
                implicitHeight: 20
                rotation: {
                    switch (root.facing) {
                    case ActionPopup.Facing.Right:
                        return 180;
                    default:
                        return 0;
                    }
                }
                transformOrigin: Item.Center

                ShapePath {
                    fillColor: Theme.backgroundColor
                    fillRule: ShapePath.OddEvenFill
                    startX: 0
                    startY: 10
                    strokeColor: 'transparent'
                    strokeWidth: 0

                    PathLine {
                        x: 20
                        y: 0
                    }
                    PathLine {
                        x: 20
                        y: 20
                    }
                    PathLine {
                        x: 0
                        y: 10
                    }
                }
            }
            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: root.facing == ActionPopup.Facing.Left ? 20 : 0
                anchors.rightMargin: root.facing == ActionPopup.Facing.Right ? 20 : 0
                border.width: 0
                color: Theme.backgroundColor
                radius: 8
            }
        }
        DropShadow {
            anchors.fill: parent
            color: "#80000000"
            horizontalOffset: 0
            radius: 8.0
            source: content3
            verticalOffset: 0
        }
    }
    contentItem: Item {
        ColumnLayout {
            id: content

            anchors.fill: parent
            anchors.leftMargin: root.facing == ActionPopup.Facing.Left ? 20 : 0
            anchors.rightMargin: root.facing == ActionPopup.Facing.Right ? 20 : 0
            spacing: 2
        }
    }
}
