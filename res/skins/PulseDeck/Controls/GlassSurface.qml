import QtQuick
import "../LateNightTheme"

// Reusable "frosted glass" overlay for a control surface: a translucent tint, a
// hairline border and a top reflection line, so controls read as glossy glass
// instead of flat blocks. Place it over the surface it belongs to, with the same
// corner radius and margin.
Item {
    id: root

    property real cornerRadius: 2
    property real margin: 0
    property real tintOpacity: 0
    property color tintColor: "transparent"
    property bool borderVisible: true
    property color borderColor: LateNightTheme.glassBorderColor
    property bool highlightVisible: true
    property bool shadeVisible: false

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.margin
        color: root.tintOpacity > 0
                ? LateNightTheme.withAlpha(root.tintColor, root.tintOpacity)
                : "transparent"
        radius: root.cornerRadius
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.margin
        border.color: root.borderColor
        border.width: 1
        color: "transparent"
        radius: root.cornerRadius
        visible: root.borderVisible
    }

    // Reflection along the top edge, inset so it follows the rounded corners.
    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: root.margin + root.cornerRadius
        anchors.right: parent.right
        anchors.rightMargin: root.margin + root.cornerRadius
        anchors.top: parent.top
        anchors.topMargin: root.margin + 1
        color: LateNightTheme.glassHighlightColor
        height: 1
        visible: root.highlightVisible
    }

    // Shade along the bottom edge, which is what gives the surface its volume.
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.margin + 1
        anchors.left: parent.left
        anchors.leftMargin: root.margin + root.cornerRadius
        anchors.right: parent.right
        anchors.rightMargin: root.margin + root.cornerRadius
        color: LateNightTheme.glassShadeColor
        height: 1
        visible: root.shadeVisible
    }
}
