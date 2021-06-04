import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 1.4
import "Theme"

Item {
    Rectangle {
        color: Theme.deckBackgroundColor
        anchors.fill: parent

        TreeView {
            id: sidebar

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 10
            width: 250
            model: Mixxx.Library.getSidebarModel()

            TableViewColumn {
                title: "Icon"
                role: "iconName"
                width: 50

                delegate: Image {
                    fillMode: Image.PreserveAspectFit
                    source: styleData.value ? "qrc:///images/library/ic_library_" + styleData.value + ".svg" : ""
                }

            }

            TableViewColumn {
                title: "Title"
                role: "display"
            }

        }

        ListView {
            anchors.top: parent.top
            anchors.left: sidebar.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            clip: true
            model: Mixxx.Library.model

            delegate: Item {
                id: itemDelegate

                implicitWidth: 300
                implicitHeight: 30

                Text {
                    anchors.fill: parent
                    text: artist + " - " + title
                    color: Theme.deckTextColor
                }

                Image {
                    id: dragItem

                    Drag.active: dragArea.drag.active
                    Drag.dragType: Drag.Automatic
                    Drag.supportedActions: Qt.CopyAction
                    Drag.mimeData: {
                        "text/uri-list": fileUrl,
                        "text/plain": fileUrl
                    }
                    anchors.fill: parent
                }

                MouseArea {
                    id: dragArea

                    anchors.fill: parent
                    drag.target: dragItem
                    onPressed: parent.grabToImage((result) => {
                        dragItem.Drag.imageSource = result.url;
                    })
                }

            }

        }

    }

}
