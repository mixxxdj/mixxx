import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 1.4 as QtQuickControls
import "Theme"

Item {
    id: root

    QtQuickControls.TreeView {
        id: sidebar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: 250
        model: Mixxx.Library.getSidebarModel()
        onClicked: {
            let featureModel = model.getModel(index);
            featureModel.select();
            console.warn(featureModel, featureModel.columnCount(), featureModel.rowCount());
            trackTable.model = featureModel;
        }

        QtQuickControls.TableViewColumn {
            title: "Icon"
            role: "iconUrl"
            width: 50

            delegate: Image {
                fillMode: Image.PreserveAspectFit
                source: styleData.value
            }

        }

        QtQuickControls.TableViewColumn {
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

        delegate: Item {
            implicitWidth: 100
            implicitHeight: 50
            Drag.active: dragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.mimeData: {
                "text/plain": trackTable.model.data(trackTable.model.index(row, 25))
            }

            Text {
                text: display
            }

            MouseArea {
                id: dragArea

                anchors.fill: parent
                drag.target: parent
            }

        }

    }

}
