import QtQuick
import "../LateNightTheme"

Rectangle {
    id: root

    property bool borderVisible: true
    property color topBorderColor: LateNightTheme.deckPanelBorderLight
    property color leftBorderColor: LateNightTheme.deckPanelBorderLeft
    property color bottomBorderColor: LateNightTheme.deckPanelBorderDark
    property color rightBorderColor: LateNightTheme.deckPanelBorderRight

    color: LateNightTheme.backgroundColor
    // Subtle top down shading, like VirtualDJ's translucent panels.
    gradient: Gradient {
        GradientStop {
            color: Qt.lighter(root.color, 1.07)
            position: 0.0
        }

        GradientStop {
            color: root.color
            position: 0.55
        }

        GradientStop {
            color: Qt.darker(root.color, 1.18)
            position: 1.0
        }
    }
    radius: 1

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: root.topBorderColor
        visible: root.borderVisible
        z: 1000
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: root.leftBorderColor
        visible: root.borderVisible
        z: 1000
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: root.bottomBorderColor
        visible: root.borderVisible
        z: 1001
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: root.rightBorderColor
        visible: root.borderVisible
        z: 1000
    }
}
