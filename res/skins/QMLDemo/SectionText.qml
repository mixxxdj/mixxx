import QtQuick 2.12
import "Theme"

Text {
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    font.family: Theme.fontFamily
    font.pixelSize: Theme.textFontPixelSize
    font.bold: true
    color: Theme.buttonNormalColor
}
