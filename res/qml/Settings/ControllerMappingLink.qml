import QtQuick
import "../Theme"

Text {
    id: root

    property var url: null
    required property var value

    color: url?.hostname ? Theme.accentColor : Theme.white
    font.italic: !url?.hostname
    font.pixelSize: 12
    font.underline: !!url?.hostname
    font.weight: url?.hostname ? Font.Normal : Font.Light
    text: url?.hostname ?? 'N/A'

    onValueChanged: {
        try {
            root.url = new URL(root.value);
        } catch (_) {}
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        enabled: !!parent.url

        onPressed: {
            Qt.openUrlExternally(parent.url);
        }
    }
}
