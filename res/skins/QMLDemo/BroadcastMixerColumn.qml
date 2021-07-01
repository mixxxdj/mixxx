import "." as Skin
import QtQuick 2.12
import "Theme"

Item {
    id: root

    property string group // required
    property alias label: labelText.text
    property alias color: mixerColumn.color

    Text {
        id: labelText

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: Theme.fontFamily
        font.bold: true
        font.pixelSize: Theme.textFontPixelSize
        color: Theme.buttonNormalColor
    }

    Skin.MixerColumn {
        id: mixerColumn

        group: root.group
        anchors.top: labelText.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

}
