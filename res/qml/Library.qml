import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQml.Models 2.12
import QtQuick 2.12
import "Theme"

Item {
    Rectangle {
        color: Theme.deckBackgroundColor
        anchors.fill: parent

        LibraryControl {
            id: libraryControl

            onMoveSelection: listView.moveSelection(offset)
            onLoadSelectedTrack: listView.loadSelectedTrack(group, play)
            onLoadSelectedTrackIntoNextAvailableDeck: listView.loadSelectedTrackIntoNextAvailableDeck(play)
            onFocusWidgetChanged: {
                switch (focusWidget) {
                case FocusedWidgetControl.WidgetKind.LibraryView:
                    listView.forceActiveFocus();
                    break;
                }
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

                currentIndex = Mixxx.MathUtils.positiveModulo(currentIndex + value, rowCount);
            }

            function loadSelectedTrackIntoNextAvailableDeck(play) {
                const url = model.get(currentIndex).fileUrl;
                if (!url)
                    return ;

                Mixxx.PlayerManager.loadLocationUrlIntoNextAvailableDeck(url, play);
            }

            function loadSelectedTrack(group, play) {
                const url = model.get(currentIndex).fileUrl;
                if (!url)
                    return ;

                const player = Mixxx.PlayerManager.getPlayer(group);
                if (!player)
                    return ;

                player.loadTrackFromLocationUrl(url, play);
            }

            anchors.fill: parent
            anchors.margins: 10
            clip: true
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
                    color: (listView.currentIndex == index && listView.activeFocus) ? Theme.blue : Theme.deckTextColor

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
                        listView.forceActiveFocus();
                        listView.currentIndex = index;
                        parent.grabToImage((result) => {
                            dragItem.Drag.imageSource = result.url;
                        });
                    }
                }

            }

            highlight: Rectangle {
                border.color: listView.activeFocus ? Theme.blue : Theme.deckTextColor
                border.width: 1
                color: "transparent"
            }

        }

    }

}
