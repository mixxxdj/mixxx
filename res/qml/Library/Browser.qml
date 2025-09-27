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

    required property var model
    readonly property var featureSelection: ItemSelectionModel {}

    color: Theme.backgroundColor

    Component.onCompleted: {
        root.model.activate(root.model.index(0, 0))
    }

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 7
        anchors.leftMargin: 7
        anchors.rightMargin: 25
        anchors.bottomMargin: 40
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
                        required property string label
                        required property string itemName
                        required property var icon
                        required property var capabilities

                        readonly property bool canCreate: capabilities & Mixxx.LibrarySource.Capability.Create
                        readonly property bool canAddTrack: capabilities & Mixxx.LibrarySource.Capability.AddTrack

                        readonly property real indentation: 40
                        readonly property real padding: 5

                        // Assigned to by TreeView:
                        required property TreeView treeView
                        required property bool isTreeNode
                        required property bool expanded
                        required property int hasChildren
                        required property int depth
                        required property int row
                        required property int column
                        required property bool current
                        // FIXME The signature for that function has changed after Qt 6.4.2 (currently shipped on Ubuntu 24.04)
                        // See https://github.com/mixxxdj/mixxx/pull/14514#issuecomment-2770811094 for further details
                        readonly property var index: treeView.modelIndex(column, row)

                        implicitWidth: treeView.width
                        implicitHeight: depth == 0 ? 42 : 35

                        // Rotate indicator when expanded by the user
                        // (requires TreeView to have a selectionModel)
                        property Animation indicatorAnimation: NumberAnimation {
                            target: indicator
                            property: "rotation"
                            from: expanded ? 0 : 90
                            to: expanded ? 90 : 0
                            duration: 100
                            easing.type: Easing.OutQuart
                        }
                        TableView.onPooled: indicatorAnimation.complete()
                        TableView.onReused: if (current) indicatorAnimation.start()
                        onExpandedChanged: indicator.rotation = expanded ? 90 : 0

                        Rectangle {
                            id: background
                            anchors.fill: parent
                            color: depth == 0 ? Theme.darkGray3 : 'transparent'

                            MouseArea {
                                id: rowMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: (event) => {
                                    treeView.selectionModel.select(treeView.selectionModel.model.index(row, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select | ItemSelectionModel.Clear | ItemSelectionModel.Current);
                                    treeView.model.activate(index)
                                    if (isTreeNode && hasChildren) {
                                        treeView.toggleExpanded(row)
                                    }
                                    event.accepted = true
                                }
                            }

                            Rectangle {
                                width: 25
                                anchors.left: parent.left
                                anchors.leftMargin: 10 + 15 * depth
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.right: parent.right
                                color: current ? Theme.midGray : 'transparent'

                                Repeater {
                                    id: lineIcon
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    model: !!icon ? 1 : 0
                                    Image {
                                        visible: depth == 0 && icon
                                        source: icon
                                        height: 25
                                        width: 25
                                    }
                                }

                                Label {
                                    id: indicator
                                    Layout.preferredWidth: indicator.implicitWidth
                                    visible: isTreeNode && hasChildren
                                    color: Theme.textColor
                                    text: "▶"

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
                                    font.weight: depth == 0 ? Font.Bold : Font.Medium
                                    font.pixelSize: 14
                                    text: label
                                    color: Theme.textColor
                                }
                                Item {
                                    visible: (rowMouseArea.containsMouse || popup.opened) && isTreeNode && canCreate
                                    id: newItem
                                    height: parent.height
                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        right: parent.right
                                        rightMargin: 10
                                    }
                                    Rectangle {
                                        width: 30
                                        height: parent.height
                                        anchors.centerIn: parent
                                        gradient: Gradient {
                                            orientation: Gradient.Horizontal

                                            GradientStop {
                                                position: 1
                                                color: Theme.sunkenBackgroundColor
                                            }

                                            GradientStop {
                                                position: 0
                                                color: 'transparent'
                                            }
                                        }
                                    }
                                    Rectangle {
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.rightMargin: 5
                                        width: 20
                                        height: 20
                                        border.width: 2
                                        border.color: Theme.white
                                        radius: 20
                                        color: 'transparent'
                                        Shape {
                                            anchors.fill: parent
                                            anchors.margins: 4
                                            ShapePath {
                                                strokeWidth: 2
                                                fillColor: Theme.white
                                                capStyle: ShapePath.RoundCap

                                                startX: 6
                                                startY: 0
                                                PathLine { x: 6; y: 12 }
                                                PathLine { x: 6; y: 8 }
                                            }
                                            ShapePath {
                                                strokeWidth: 2
                                                fillColor: Theme.white
                                                capStyle: ShapePath.RoundCap

                                                startX: 0
                                                startY: 6
                                                PathLine { y: 6; x: 12 }
                                                PathLine { y: 6; x: 8 }
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
