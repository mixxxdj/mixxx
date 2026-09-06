import QtQuick
import QtQuick.Window
import "Theme"

Window {
    id: root

    default property alias contentData: surfaceContent.data
    property bool fullscreen: false
    required property var targetScreen

    color: Theme.backgroundColor
    height: fullscreen ? targetScreen.height : Math.round(targetScreen.height * 0.85)
    screen: targetScreen
    visibility: fullscreen ? Window.FullScreen : Window.Windowed
    width: fullscreen ? targetScreen.width : Math.round(targetScreen.width * 0.8)
    x: targetScreen.virtualX + Math.round((targetScreen.width - width) / 2)
    y: targetScreen.virtualY + Math.round((targetScreen.height - height) / 2)

    Item {
        id: surfaceContent

        anchors.fill: parent
    }
}
