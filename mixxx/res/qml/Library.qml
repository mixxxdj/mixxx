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

    property var sidebar: librarySources.sidebar()

    LibraryComponent.SourceTree {
        id: librarySources
    }

    SplitView {
        id: librarySplitView
        orientation: Qt.Horizontal
        anchors.fill: parent

        handle: Rectangle {
            id: handleDelegate
            implicitWidth: 8
            implicitHeight: 8
            color: Theme.panelSplitterBackground
            clip: true
            property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
            property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

            ColumnLayout {
                anchors.centerIn: parent
                Repeater {
                    model: 3
                    Rectangle {
                        width: handleSize
                        height: handleSize
                        radius: handleSize
                        color: handleColor
                    }
                }
            }

            containmentMask: Item {
                x: (handleDelegate.width - width) / 2
                width: 8
                height: librarySplitView.height
            }
        }

        SplitView {
            id: sideBarSplitView
            SplitView.minimumWidth: 100
            SplitView.preferredWidth: 415
            SplitView.maximumWidth: 600

            orientation: Qt.Vertical

            handle: Rectangle {
                id: handleDelegate
                implicitWidth: 8
                implicitHeight: 8
                color: Theme.panelSplitterBackground
                clip: true
                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                RowLayout {
                    anchors.centerIn: parent
                    Repeater {
                        model: 3
                        Rectangle {
                            width: handleSize
                            height: handleSize
                            radius: handleSize
                            color: handleColor
                        }
                    }
                }

                containmentMask: Item {
                    x: (handleDelegate.width - width) / 2
                    height: 8
                    width: sideBarSplitView.width
                }
            }
            LibraryComponent.Browser {
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 500
                SplitView.fillHeight: true

                model: root.sidebar
            }

            Skin.PreviewDeck {
                SplitView.minimumHeight: 100
                SplitView.preferredHeight: 100
                SplitView.maximumHeight: 200
            }
        }
        LibraryComponent.TrackList {
            SplitView.fillHeight: true

            // FIXME: this is necessary to prevent the header label to render outside of the table when horizontally scrolling: https://github.com/mixxxdj/mixxx/pull/14514#issuecomment-3311914346
            clip: true

            model: root.sidebar.tracklist
        }
    }
}
