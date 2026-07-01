import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.12
import Qt5Compat.GraphicalEffects
import "../Theme"

Rectangle {
    id: root

    readonly property var featureSelection: ItemSelectionModel {
    }
    required property var model

    color: Theme.backgroundColor

    Component.onCompleted: {
        root.model.activate(root.model.index(0, 0));
    }

    Rectangle {
        anchors.bottomMargin: 40
        anchors.fill: parent
        anchors.leftMargin: 7
        anchors.rightMargin: 15
        anchors.topMargin: 7
        color: Theme.sunkenBackgroundColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            ScrollView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                TreeView {
                    id: featureView

                    Layout.fillWidth: true
                    clip: true
                    model: root.model
                    selectionModel: featureSelection

                    delegate: FocusScope {
                        required property int column
                        required property bool current
                        required property int depth
                        required property bool expanded
                        required property int hasChildren
                        required property var icon
                        readonly property real indentation: 40
                        // FIXME The signature for that function has changed after Qt 6.4.2 (currently shipped on Ubuntu 24.04)
                        // See https://github.com/mixxxdj/mixxx/pull/14514#issuecomment-2770811094 for further details
                        readonly property var index: treeView.modelIndex(column, row)

                        // Rotate indicator when expanded by the user
                        // (requires TreeView to have a selectionModel)
                        property Animation indicatorAnimation: NumberAnimation {
                            duration: 100
                            easing.type: Easing.OutQuart
                            from: expanded ? 0 : 90
                            property: "rotation"
                            target: indicator
                            to: expanded ? 90 : 0
                        }
                        required property bool isTreeNode
                        required property string label
                        readonly property real padding: 5
                        required property int row

                        // Assigned to by TreeView:
                        required property TreeView treeView

                        implicitHeight: depth == 0 ? 42 : 35
                        implicitWidth: treeView.width

                        TableView.onPooled: indicatorAnimation.complete()
                        TableView.onReused: if (current)
                            indicatorAnimation.start()
                        onExpandedChanged: indicator.rotation = expanded ? 90 : 0

                        Rectangle {
                            id: background

                            anchors.fill: parent
                            color: depth == 0 ? Theme.midGray2 : 'transparent'

                            MouseArea {
                                id: rowMouseArea

                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                anchors.fill: parent
                                hoverEnabled: true

                                onClicked: event => {
                                    treeView.selectionModel.select(treeView.selectionModel.model.index(row, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select | ItemSelectionModel.Clear | ItemSelectionModel.Current);
                                    treeView.model.activate(index);
                                    if (isTreeNode && hasChildren) {
                                        treeView.toggleExpanded(row);
                                    }
                                    event.accepted = true;
                                }
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.leftMargin: 10 + 15 * depth
                                anchors.right: parent.right
                                anchors.top: parent.top
                                color: current ? Theme.midGray : 'transparent'
                                width: 25

                                Repeater {
                                    id: lineIcon

                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    model: !!icon ? 1 : 0

                                    Image {
                                        height: 25
                                        source: icon
                                        visible: depth == 0 && icon
                                        width: 25
                                    }
                                }
                                Label {
                                    id: indicator

                                    Layout.preferredWidth: indicator.implicitWidth
                                    color: Theme.textColor
                                    text: "▶"
                                    visible: isTreeNode && hasChildren

                                    anchors {
                                        left: parent.left
                                        verticalCenter: lineIcon.bottom
                                    }
                                }
                                Label {
                                    id: labelItem

                                    anchors.left: parent.left
                                    anchors.leftMargin: depth == 0 && row == 0 ? 10 : 34
                                    anchors.verticalCenter: parent.verticalCenter
                                    clip: true
                                    color: Theme.textColor
                                    font.pixelSize: 14
                                    font.weight: depth == 0 ? Font.Bold : Font.Medium
                                    text: label
                                }
                                Item {
                                    id: newItem

                                    height: parent.height
                                    visible: rowMouseArea.containsMouse && isTreeNode && hasChildren

                                    anchors {
                                        right: parent.right
                                        rightMargin: 10
                                        verticalCenter: parent.verticalCenter
                                    }
                                    Rectangle {
                                        anchors.centerIn: parent
                                        height: parent.height
                                        width: 30

                                        gradient: Gradient {
                                            orientation: Gradient.Horizontal

                                            GradientStop {
                                                color: Theme.sunkenBackgroundColor
                                                position: 1
                                            }
                                            GradientStop {
                                                color: 'transparent'
                                                position: 0
                                            }
                                        }
                                    }
                                    Rectangle {
                                        anchors.right: parent.right
                                        anchors.rightMargin: 5
                                        anchors.verticalCenter: parent.verticalCenter
                                        border.color: Theme.white
                                        border.width: 2
                                        color: 'transparent'
                                        height: 20
                                        radius: 20
                                        width: 20

                                        Shape {
                                            anchors.fill: parent
                                            anchors.margins: 4

                                            ShapePath {
                                                capStyle: ShapePath.RoundCap
                                                fillColor: Theme.white
                                                startX: 6
                                                startY: 0
                                                strokeWidth: 2

                                                PathLine {
                                                    x: 6
                                                    y: 12
                                                }
                                                PathLine {
                                                    x: 6
                                                    y: 8
                                                }
                                            }
                                            ShapePath {
                                                capStyle: ShapePath.RoundCap
                                                fillColor: Theme.white
                                                startX: 0
                                                startY: 6
                                                strokeWidth: 2

                                                PathLine {
                                                    x: 12
                                                    y: 6
                                                }
                                                PathLine {
                                                    x: 8
                                                    y: 6
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
        }
    }
}
