import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import "Theme"

Item {
    LibraryControl {
        id: libraryControl

        onMoveVertical: (offset) => {
            listView.moveSelectionVertical(offset);
        }
        onLoadSelectedTrack: (group, play) => {
            listView.loadSelectedTrack(group, play);
        }
        onLoadSelectedTrackIntoNextAvailableDeck: (play) => {
            listView.loadSelectedTrackIntoNextAvailableDeck(play);
        }
        onFocusWidgetChanged: {
            switch (focusWidget) {
                case FocusedWidgetControl.WidgetKind.LibraryView:
                    listView.forceActiveFocus();
                    break;
            }
        }
    }

    SplitView {
        anchors.fill: parent

        Rectangle {
            id: sidebarBox

            color: Theme.deckBackgroundColor

            SplitView.preferredWidth: 200

            LibrarySidebar {
                model: Mixxx.Library.sidebarModel
            }
        }

        Rectangle {
            id: tableBox

            color: Theme.deckBackgroundColor

            ListView {
                id: listView

                function moveSelectionVertical(value) {
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
                keyNavigationWraps: true
                highlightMoveDuration: 250
                highlightResizeDuration: 50
                model: Mixxx.Library.model
                Keys.onPressed: (event) => {
                    switch (event.key) {
                        case Qt.Key_Enter:
                            case Qt.Key_Return:
                                listView.loadSelectedTrackIntoNextAvailableDeck(false);
                            break;
                    }
                }

                delegate: Item {
                    id: itemDlgt

                    required property int index
                    required property url fileUrl
                    required property string artist
                    required property string title

                    implicitWidth: listView.width
                    implicitHeight: 30

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: itemDlgt.artist + " - " + itemDlgt.title
                        color: (listView.currentIndex == itemDlgt.index && listView.activeFocus) ? Theme.blue : Theme.deckTextColor

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
                            "text/uri-list": itemDlgt.fileUrl,
                            "text/plain": itemDlgt.fileUrl
                        }
                        anchors.fill: parent
                    }

                    MouseArea {
                        id: dragArea

                        anchors.fill: parent
                        drag.target: dragItem
                        onPressed: {
                            listView.forceActiveFocus();
                            listView.currentIndex = itemDlgt.index;
                            parent.grabToImage((result) => {
                                    dragItem.Drag.imageSource = result.url;
                            });
                        }
                        onDoubleClicked: listView.loadSelectedTrackIntoNextAvailableDeck(false)
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
}
