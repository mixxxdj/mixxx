import QtQuick
import QtQuick.Layouts
import "." as LibraryComponent
import "../Theme"

Rectangle {
    id: root

    required property bool selected

    property alias capabilities: track.capabilities

    color: selected ? Theme.accentColor : (row % 2 == 0 ? Theme.sunkenBackgroundColor : Theme.backgroundColor)

    LibraryComponent.Track {
        id: track

        view: TableView.view
        anchors.fill: parent
    }

    Item {
        id: dragImageSource
        height: 85
        visible: false
        width: 190

        Rectangle {
            color: Theme.sunkenBackgroundColor
            radius: 12

            anchors {
                bottom: parent.bottom
                left: parent.left
                margins: 5
                right: parent.right
                top: parent.top
            }
            RowLayout {
                anchors.fill: parent

                Image {
                    id: cover

                    Layout.fillHeight: true
                    Layout.preferredWidth: cover_art ? 75 : 0
                    asynchronous: true
                    clip: true
                    fillMode: Image.PreserveAspectFit
                    source: cover_art
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    // Text {
                    //     color: Theme.textColor
                    //     text: track ? track.title : 'Unknown title'
                    // }
                    // Text {
                    //     color: Theme.midGray
                    //     text: track ? track.artist : 'Unknown artist'
                    // }
                }
            }
            Rectangle {
                width: 20

                gradient: Gradient {
                    orientation: Gradient.Horizontal

                    GradientStop {
                        color: Theme.darkGray
                        position: 1
                    }
                    GradientStop {
                        color: 'transparent'
                        position: 0
                    }
                }

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    top: parent.top
                }
            }
        }
    }

    Drag.dragType: Qt.platform.os === "android" ? Drag.None : Drag.Automatic
    Drag.mimeData: {
        "text/uri-list": file_url.toString(),
        "text/plain": file_url.toString()
    }
    Drag.supportedActions: Qt.CopyAction

    DragHandler {
        id: dragHandler
        // enabled: false
        enabled: Qt.platform.os !== "android"
        onActiveChanged: {
            root.Drag.imageSource = cover_art
            root.Drag.active = true
            console.log(`dragHandler: ${root.Drag.active}`)
        //     if (active) {
        //         dragImageSource.grabToImage(function(result) {
        //             console.log(`dragHandler: ${result}`)
        //             parent.Drag.imageSource = result.url
        //             parent.Drag.active = true
        //         }, Qt.size(dragImageSource.width, dragImageSource.height))
        //     } else {
        //         parent.Drag.active = false
            // }
        }
    }
    Rectangle {
        id: border

        color: Theme.darkGray2
        width: 1

        anchors {
            bottom: parent.bottom
            right: parent.right
            top: parent.top
        }
    }
}
