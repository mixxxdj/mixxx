import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.12
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
        anchors.bottomMargin: 1
        anchors.fill: parent
        anchors.leftMargin: 7
        anchors.rightMargin: 1
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
                        required property string itemName

                        readonly property bool canCreate: capabilities & Mixxx.LibrarySource.Capability.Create
                        readonly property bool canAddTrack: capabilities & Mixxx.LibrarySource.Capability.AddTrack
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
                            color: row == 0 ? Theme.midGray : depth == 0 ? Theme.darkGray2 : 'transparent'

                            TapHandler {
                                id: rowTapHandler

                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                onTapped: (eventPoint, button) => {
                                    treeView.selectionModel.select(treeView.selectionModel.model.index(row, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select | ItemSelectionModel.Clear | ItemSelectionModel.Current);
                                    treeView.model.activate(index);
                                    if (isTreeNode && hasChildren) {
                                        treeView.toggleExpanded(row);
                                    }
                                }
                            }
                            HoverHandler {
                                id: rowHoverHandler
                            }
                            Rectangle {
                                color: current ? Theme.midGray : 'transparent'
                                anchors.fill: parent
                            }
                            Item {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.leftMargin: 10 + 15 * depth
                                anchors.right: parent.right
                                anchors.top: parent.top
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
                                    visible: (rowHoverHandler.hovered || popup.opened) && isTreeNode && canCreate
                                    id: newItem

                                    height: parent.height

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
                                        MouseArea {
                                            anchors.fill: parent
                                            onPressed: {
                                                popup.x = parent.width
                                                popup.y = parent.height / 2 - popup.height / 2
                                                popup.open()
                                                popup.forceActiveFocus(Qt.PopupFocusReason)
                                            }
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                        Skin.ActionPopup {
                                            id: popup
                                            padding: 6
                                            focus: true
                                            Text {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: qsTr("New %1").arg(itemName)
                                                font.weight: Font.Bold
                                                font.pixelSize: 14
                                                color: Theme.white
                                            }
                                            Skin.InputField {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 36
                                                Layout.margins: 4
                                                focus: true
                                                id: newItemName
                                                input.onAccepted: {
                                                    if (input.text) {
                                                        create(input.text)
                                                    }
                                                    popup.close()
                                                }
                                            }
                                            RowLayout {
                                                Layout.fillWidth: true
                                                Skin.ActionButton {
                                                    Layout.fillWidth: true
                                                    label.text: qsTr("Cancel")
                                                    onPressed: {
                                                        popup.close()
                                                    }
                                                }
                                                Skin.ActionButton {
                                                    Layout.fillWidth: true
                                                    opacity: newItemName.text || newItemName.input.text ? 1 : 0.4
                                                    category: Skin.ActionButton.Action
                                                    label.text: qsTr("Create")
                                                    onPressed: {
                                                        if (newItemName.text) {
                                                            create(input.text)
                                                            popup.close()
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
    }
}
