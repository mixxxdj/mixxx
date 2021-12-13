import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import "Theme"

Item {
    Rectangle {
        color: Theme.deckBackgroundColor
        anchors.fill: parent

        Mixxx.ControlProxy {
            id: focusedWidgetControl

            group: "[Library]"
            key: "focused_widget"
            Component.onCompleted: value = 3
        }

        Mixxx.ControlProxy {
            group: "[Playlist]"
            key: "SelectTrackKnob"
            onValueChanged: {
                listView.moveSelection(value);
            }
        }

        Mixxx.ControlProxy {
            group: "[Playlist]"
            key: "SelectPrevTrack"
            onValueChanged: {
                if (value != 0)
                    listView.moveSelection(-1);

            }
        }

        Mixxx.ControlProxy {
            group: "[Playlist]"
            key: "SelectNextTrack"
            onValueChanged: {
                if (value != 0)
                    listView.moveSelection(1);

            }
        }

        Mixxx.ControlProxy {
            group: "[Library]"
            key: "MoveVertical"
            onValueChanged: {
                if (focusedWidgetControl.value == 3)
                    listView.moveSelection(value);

            }
        }

        Mixxx.ControlProxy {
            group: "[Library]"
            key: "MoveUp"
            onValueChanged: {
                if (value != 0 && focusedWidgetControl.value == 3)
                    listView.moveSelection(-1);

            }
        }

        Mixxx.ControlProxy {
            group: "[Library]"
            key: "MoveDown"
            onValueChanged: {
                if (value != 0 && focusedWidgetControl.value == 3)
                    listView.moveSelection(1);

            }
        }

        ListView {
            id: listView

            function moveSelection(value) {
                if (value == 0)
                    return ;

                const rowCount = model.rowCount();
                if (rowCount == 0)
                    return ;

                let newIndex = currentIndex = (currentIndex + value) % rowCount;
                while (newIndex < 0)newIndex += rowCount
                currentIndex = newIndex;
            }

            anchors.fill: parent
            anchors.margins: 10
            clip: true
            focus: true
            highlightMoveDuration: 250
            highlightResizeDuration: 50
            model: Mixxx.Library.model

            delegate: Item {
                id: itemDelegate

                implicitWidth: listView.width
                implicitHeight: 30

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: artist + " - " + title
                    color: listView.currentIndex == index ? Theme.blue : Theme.deckTextColor

                    Behavior on color {
                        ColorAnimation {
                            duration: listView.highlightMoveDuration
                        }

                    }

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
                    onPressed: {
                        listView.currentIndex = index;
                        parent.grabToImage((result) => {
                            dragItem.Drag.imageSource = result.url;
                        });
                    }
                }

            }

            highlight: Rectangle {
                border.color: Theme.blue
                border.width: 1
                color: "transparent"
            }

        }

    }

}
