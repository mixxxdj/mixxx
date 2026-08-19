import QtQuick
import QtQuick.Window
import "Theme"

Window {
    id: root

    default property alias contentData: surfaceContent.data
    property bool fullscreen: false
    required property var targetScreen

    color: Theme.backgroundColor
    height: targetScreen.height
    screen: targetScreen
    visibility: fullscreen ? Window.FullScreen : Window.Windowed
    width: targetScreen.width
    x: targetScreen.virtualX
    y: targetScreen.virtualY

    Item {
        id: surfaceContent

        anchors.fill: parent
    }
}
