import QtQuick
import RenderGraph

Item {
    id: root

    width: 680
    height: 520

    CustomItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        anchors.leftMargin: 20
        anchors.rightMargin: 20
    }
}
