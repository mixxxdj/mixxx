import QtQuick
import "../Theme"

Text {
    id: root

    required property var value
    property var url: null

    onValueChanged: {
        try {
            root.url = new URL(root.value)
        } catch (_) {}
    }
    text: url?.hostname ?? 'N/A'
    font.underline: !!url?.hostname
    font.italic: !url?.hostname
    font.weight: url?.hostname ? Font.Normal : Font.Light
    font.pixelSize: 12
    color: url?.hostname ? Theme.accentColor : Theme.white
    MouseArea {
        enabled: !!parent.url
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onPressed: {
            Qt.openUrlExternally(parent.url)
        }
    }
}
