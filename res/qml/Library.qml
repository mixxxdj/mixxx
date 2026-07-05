import "." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.6
import "Theme"
import "Library" as LibraryComponent

Item {
    id: root

    property var activeSidebar: libraryLeftSources.sidebar()

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

        handle: Rectangle {
            id: handleDelegate

            property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
            property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

            clip: true
            color: Theme.panelSplitterBackground
            implicitHeight: 8
            implicitWidth: 8

            containmentMask: Item {
                height: librarySplitView.height
                width: 8
                x: (handleDelegate.width - width) / 2
            }

            ColumnLayout {
                anchors.centerIn: parent

                Repeater {
                    model: 3

                    Rectangle {
                        color: handleColor
                        height: handleSize
                        radius: handleSize
                        width: handleSize
                    }
                }
            }
        }

        SplitView {
            id: sideBarSplitView

            SplitView.maximumWidth: 550
            SplitView.minimumWidth: 150
            SplitView.preferredWidth: root.width * 0.15
            orientation: Qt.Vertical

            handle: Rectangle {
                id: handleDelegate

                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                clip: true
                color: Theme.panelSplitterBackground
                implicitHeight: 8
                implicitWidth: 8

                containmentMask: Item {
                    height: 8
                    width: sideBarSplitView.width
                    x: (handleDelegate.width - width) / 2
                }

                RowLayout {
                    anchors.centerIn: parent

                    Repeater {
                        model: 3

                        Rectangle {
                            color: handleColor
                            height: handleSize
                            radius: handleSize
                            width: handleSize
                        }
                    }
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

                handle: Rectangle {
                    id: handleDelegate

                    property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                    property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                    clip: true
                    color: Theme.panelSplitterBackground
                    implicitHeight: 8
                    implicitWidth: 8

                    containmentMask: Item {
                        height: root.width < 800 ? 8 : librarySplitView.height
                        width: root.width < 800 ? sideBarSplitView.width : 8
                        x: (handleDelegate.width - width) / 2
                    }

                    GridLayout {
                        anchors.centerIn: parent
                        columns: root.width < 800 ? 3 : 1

                        Repeater {
                            model: 3

                            Rectangle {
                                color: handleColor
                                height: handleSize
                                radius: handleSize
                                width: handleSize
                            }
                        }
                    }
                }
                LibraryComponent.TrackList {
                    opacity: root.activeSidebar == model.sidebar() ? 1 : 0.6
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5

                    focus: true
                    model: libraryLeftSources

                    TapHandler {
                        onTapped: {
                            root.activeSidebar = parent.model.sidebar()
                        }
                    }
                }

                Loader {
                    visible: splitViewButton.checked
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5
                    active: splitViewButton.checked
                    opacity: status == Loader.Ready ? 1 : 0
                    asynchronous: true

                    sourceComponent: Component {
                        LibraryComponent.TrackList {
                            opacity: root.activeSidebar == model.sidebar() ? 1 : 0.6

                            focus: true
                            model: libraryRightSources

                            TapHandler {
                                onTapped: {
                                    root.activeSidebar = parent.model.sidebar()
                                }
                            }
                        }
                    }
                }
            }
            }
        }
    }
}
