import "." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Controls 2.15
import QtQuick.Effects
import QtQuick.Shapes 1.6
import "Theme"
import "Library" as LibraryComponent

Item {
    id: root


    property var activeSidebar: libraryLeftSources.sidebar()

    onActiveSidebarChanged: {
        if (!root.activeSidebar.tracklist) return;
        print("onActiveSidebarChanged", root.activeSidebar.tracklist.search)

        searchPane.clearAllCriteria()
        searchPane.textField.text = root.activeSidebar.tracklist.search
        searchPane.tryConvertToFieldToken()
    }

    Connections {
        target: root.activeSidebar

        function onTracklistChanged(){
            print("onTracklistChanged", root.activeSidebar.tracklist.search)

            searchPane.clearAllCriteria()
            searchPane.textField.text = root.activeSidebar.tracklist.search
            searchPane.tryConvertToFieldToken()
        }
    }

    // One side of the split track list: tapping it makes it the active
    // sidebar, the inactive side dims.
    component ActiveTrackList: LibraryComponent.TrackList {
        focus: true
        opacity: root.activeSidebar == sidebar ? 1 : 0.6

        TapHandler {
            onTapped: {
                root.activeSidebar = parent.sidebar
            }
        }
    }

    LibraryComponent.SourceTree {
        id: libraryLeftSources
    }
    LibraryComponent.SourceTree {
        id: libraryRightSources
    }
    SplitView {
        id: librarySplitView

        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Skin.SplitViewHandle {
            id: librarySplitHandle

            containmentMask: Item {
                height: librarySplitView.height
                width: 8
                x: (librarySplitHandle.width - width) / 2
            }
        }

        SplitView {
            id: sideBarSplitView

            SplitView.maximumWidth: 550
            SplitView.minimumWidth: 150
            SplitView.preferredWidth: root.width * 0.15
            orientation: Qt.Vertical

            handle: Skin.SplitViewHandle {
                id: sideBarSplitHandle

                dotColumns: 3

                containmentMask: Item {
                    height: 8
                    width: sideBarSplitView.width
                    x: (sideBarSplitHandle.width - width) / 2
                }
            }

            LibraryComponent.Browser {
                id: sidebarTree
                SplitView.fillHeight: true
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 500
                model: root.activeSidebar
            }
            Skin.PreviewDeck {
                SplitView.maximumHeight: 200
                SplitView.minimumHeight: 100
                SplitView.preferredHeight: 100
            }
        }
        Item {
            id: browsingView
            objectName: "browsingView"

            SplitView.fillHeight: true
            SplitView.minimumHeight: 200
            SplitView.preferredWidth: root.width * 0.75

            SplitView {
                id: trackListSplitView

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: tracklistMenu.left
                anchors.bottom: parent.bottom

                orientation: root.width < 800 ? Qt.Vertical: Qt.Horizontal

                handle: Skin.SplitViewHandle {
                    id: trackListSplitHandle

                    dotColumns: root.width < 800 ? 3 : 1

                    containmentMask: Item {
                        height: root.width < 800 ? 8 : librarySplitView.height
                        width: root.width < 800 ? sideBarSplitView.width : 8
                        x: (trackListSplitHandle.width - width) / 2
                    }
                }
                ActiveTrackList {
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5

                    model: libraryLeftSources
                }

                Loader {
                    objectName: "rightTrackListLoader"
                    visible: splitViewButton.checked
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5
                    active: splitViewButton.checked
                    opacity: status == Loader.Ready ? 1 : 0
                    asynchronous: true

                    sourceComponent: Component {
                        ActiveTrackList {
                            objectName: "rightTrackList"
                            model: libraryRightSources
                        }
                    }
                }
            }
            Column {
                id: tracklistMenu
                objectName: "tracklistMenu"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 36
                spacing: 11
                padding: 4

                Skin.Button {
                    id: splitViewButton
                    objectName: "splitViewButton"

                    activeColor: Theme.white
                    checkable: true
                    icon.height: 16
                    icon.source: "images/splitview.svg"
                    icon.width: 16
                    implicitWidth: implicitHeight

                    onCheckedChanged: {
                        if (checked) {
                            let rightSidebar = libraryRightSources.sidebar()
                            rightSidebar.activate(rightSidebar.index(sidebarTree.currentSelectedIndex?.top ?? 0, 0));
                            root.activeSidebar = rightSidebar
                        } else {
                            root.activeSidebar = libraryLeftSources.sidebar()
                        }
                    }
                }

                Item {
                    height: historyButton.implicitWidth
                    width: historyButton.implicitHeight
                    Skin.Button {
                        id: historyButton

                        activeColor: Theme.white
                        checkable: true
                        text: "HISTORY"

                        transform: Rotation {
                            origin.x: historyButton.height / 2
                            origin.y: historyButton.height / 2
                            angle: 90
                        }

                    }
                }

                Item {
                    height: prepButton.implicitWidth
                    width: prepButton.implicitHeight
                    Skin.Button {
                        id: prepButton

                        activeColor: Theme.white
                        checkable: true
                        text: "PREPARATION"
                        implicitWidth: 82

                        transform: Rotation {
                            origin.x: prepButton.height / 2
                            origin.y: prepButton.height / 2
                            angle: 90
                        }
                    }
                }
            }

            MultiEffect {
                anchors.fill: searchPane
                source: searchPane
                shadowEnabled: true
                shadowColor: '#66000000'
                shadowBlur: 0.25
                shadowVerticalOffset: 2
            }

            LibraryComponent.SearchPane {
                id: searchPane
                objectName: "searchPane"

                activeSidebar: root.activeSidebar
                focusOutTarget: browsingView
            }
        }
    }
}
