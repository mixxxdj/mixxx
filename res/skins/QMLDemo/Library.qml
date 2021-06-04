import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 1.4
import "Theme"

Item {
    id: root

    TreeView {
        id: sidebar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: 250
        model: Mixxx.Library.getSidebarModel()

        TableViewColumn {
            title: "Icon"
            role: "iconUrl"
            width: 50

            delegate: Image {
                fillMode: Image.PreserveAspectFit
                source: styleData.value
            }

        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

    }

    TableView {
        id: trackTable

        anchors.top: parent.top
        anchors.left: sidebar.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 5
        model: Mixxx.Library.getLibraryModel()

        TableViewColumn {
            title: "Icon"
            role: "display"
            width: 50
        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

        TableViewColumn {
            title: "Title"
            role: "display"
        }

    }

}
