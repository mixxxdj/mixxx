import QtQuick
import QtQuick.Controls

MenuItem {
    id: root

    property url checkedSource
    property color disabledTextColor: "#494949"
    property string fontFamily: "Open Sans"
    property int fontPixelSize: 14
    property color hoverColor: "#2c454f"
    property color hoverTextColor: "#ffffff"
    property url submenuArrowSource
    property color textColor: "#c2b3a5"

    implicitHeight: visible ? 22 : 0
    implicitWidth: Math.max(160, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: checkable ? 22 : 6
    rightPadding: subMenu ? 22 : 6

    arrow: Image {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        height: 10
        source: root.submenuArrowSource
        visible: root.subMenu && root.enabled
        width: 6
    }
    background: Rectangle {
        color: root.highlighted && root.enabled ? root.hoverColor : "transparent"
        radius: 1
    }
    contentItem: Text {
        color: !root.enabled ? root.disabledTextColor : root.highlighted ? root.hoverTextColor : root.textColor
        elide: Text.ElideRight
        font.family: root.fontFamily
        font.pixelSize: root.fontPixelSize
        horizontalAlignment: Text.AlignLeft
        text: root.text
        verticalAlignment: Text.AlignVCenter
    }
    indicator: Image {
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        height: 14
        source: root.checked ? root.checkedSource : ""
        visible: root.checkable && root.checked
        width: 14
    }
}
