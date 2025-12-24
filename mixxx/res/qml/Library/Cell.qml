import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Layouts
import "../Theme"

Rectangle {
    id: root

    readonly property alias dragImage: dragImageEffect

    anchors.fill: parent

    color: selected ? Theme.accent : (row % 2 == 0 ? Theme.sunkenBackgroundColor : Theme.backgroundColor)

    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    Drag.mimeData: {
        "text/uri-list": file_url.toString(),
        "text/plain": file_url.toString(),
    }
    Item {
        id: dragImageSource
        width: 190
        height: 85
        visible: false
        Rectangle {
            color: Theme.sunkenBackgroundColor
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                margins: 5
            }
            radius: 12
            RowLayout {
                anchors.fill: parent
                Image {
                    id: cover
                    Layout.fillHeight: true
                    Layout.preferredWidth: cover_art ? 75 : 0
                    fillMode: Image.PreserveAspectFit
                    source: cover_art
                    clip: true
                    asynchronous: true
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Text {
                        text: track ? track.title : 'Unknown title'
                        color: Theme.textColor
                    }
                    Text {
                        text: track ? track.artist : 'Unknown artist'
                        color: Theme.midGray
                    }
                }
            }
            Rectangle {
                width: 20
                anchors {
                    top: parent.top
                    right: parent.right
                    bottom:parent.bottom
                }
                gradient: Gradient {
                    orientation: Gradient.Horizontal

                    GradientStop {
                        position: 1
                        color: Theme.darkGray
                    }

                    GradientStop {
                        position: 0
                        color: 'transparent'
                    }
                }
            }
        }
    }
    DropShadow {
        id: dragImageEffect
        visible: false
        anchors.fill: dragImageSource
        source: dragImageSource
        horizontalOffset: 0
        verticalOffset: 0
        radius: 10.0
        color: "#80000000"
    }

    Rectangle {
        id: border
        color: Theme.darkGray2
        width: 1
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
    }
}
