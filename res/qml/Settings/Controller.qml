import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import Qt5Compat.GraphicalEffects
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Category {
    id: root

    property bool dirty: false
    property var editedControllers: new Set()
    property var manager: Mixxx.ControllerManager
    property bool processing: false
    property var selectedController: null

    function load() {
        for (let controller of root.editedControllers) {
            controller.clear();
        }
        root.editedControllers.clear();
        root.dirty = false;
        errorMessage.text = "";
    }
    function save() {
        for (let controller of root.editedControllers) {
            controller.save(Mixxx.Config);
        }
        root.selectedController = null;
        root.load();
    }

    label: "Controllers"

    Component.onCompleted: {
        load();
    }

    ScrollView {
        id: scrollView

        anchors.bottom: controllerAction.top
        anchors.bottomMargin: 12
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        ColumnLayout {
            spacing: root.selectedController ? 18 : 0
            width: root.width

            GridLayout {
                Layout.fillWidth: true
                columnSpacing: 24
                columns: parseInt(root.width / 1050) + 1

                Repeater {
                    id: controllers

                    model: {
                        if (root.selectedController) {
                            if (root.selectedController.hasSettings)
                                return [root.selectedController];
                            else if (manager.knownDevices.indexOf(root.selectedController) == -1)
                                return [...manager.knownDevices, root.selectedController];
                        }
                        return manager.knownDevices;
                    }

                    delegate: ControllerCard {
                        id: controller

                        focused: knownDevice && root.selectedController == modelData
                        knownDevice: manager.knownDevices.indexOf(modelData) > -1
                        opacity: root.selectedController && !root.selectedController.knownDevice && root.selectedController != modelData ? 0.2 : 1

                        onSelect: {
                            modelData.hasSettings = !!controller.settings;
                            root.selectedController = modelData;
                        }
                        onSettingsChanged: {
                            if (!controller.settings && controller.focused) {
                                root.selectedController = null;
                            }
                        }

                        Connections {
                            function onEditedChanged() {
                                if (controller.modelData.edited) {
                                    root.editedControllers.add(controller.modelData);
                                    root.dirty = true;
                                }
                            }

                            target: controller.modelData
                        }
                    }
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    visible: root.selectedController == null || !root.selectedController.hasSettings
                }
            }
            ListView {
                id: unknownDevicesList

                Layout.fillWidth: true
                currentIndex: -1
                implicitHeight: contentHeight
                model: manager.unknownDevices
                visible: root.selectedController == null || !root.selectedController.hasSettings

                delegate: Rectangle {
                    required property int index
                    required property var modelData

                    color: index % 2 == 0 ? '#0C0C0C' : '#272727'
                    height: 32
                    width: ListView.view.width

                    MouseArea {
                        id: unknownDeviceRow

                        anchors.fill: parent
                        hoverEnabled: true
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6

                        Rectangle {
                            Layout.preferredHeight: 18
                            Layout.preferredWidth: 35
                            color: '#393939'
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                color: '#FFFFFF'
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                text: {
                                    switch (modelData.type) {
                                    case Mixxx.ControllerDevice.Type.MIDI:
                                        return "MIDI";
                                    case Mixxx.ControllerDevice.Type.HID:
                                        return "HID";
                                    case Mixxx.ControllerDevice.Type.BULK:
                                        return "BULK";
                                    }
                                    return "UNKN";
                                }
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            color: '#FFFFFF'
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            text: modelData.name
                        }
                        Skin.FormButton {
                            activeColor: "#999999"
                            text: "Create device"
                            visible: unknownDeviceRow.containsMouse || hovered || unknownDevicesList.currentIndex == index

                            onPressed: {
                                root.selectedController = modelData;
                                root.dirty = true;
                            }
                        }
                        TapHandler {
                            onTapped: {
                                unknownDevicesList.currentIndex = index;
                            }
                        }
                    }
                }
                header: Rectangle {
                    color: "#161616"
                    implicitHeight: 32
                    width: ListView.view.width

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.margins: 10
                        anchors.rightMargin: 20
                        color: '#FFFFFF'
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        text: 'Other detected devices'
                    }
                }
            }
        }
    }
    RowLayout {
        id: controllerAction

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: parent.right
        anchors.rightMargin: 14
        visible: root.dirty || root.selectedController

        Skin.FormButton {
            activeColor: "#999999"
            backgroundColor: "#7D3B3B"
            opacity: enabled ? 1.0 : 0.5
            text: "Reset"
            visible: !!root.selectedController?.hasSettings

            onPressed: {
                root.selectedController?.mapping?.resetSettings(root.selectedController);
                root.load();
            }
        }
        Item {
            Layout.fillWidth: true
        }
        Text {
            id: errorMessage

            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 16
            color: "#7D3B3B"
            text: ""
        }
        Skin.FormButton {
            activeColor: "#999999"
            backgroundColor: "#3F3F3F"
            opacity: enabled ? 1.0 : 0.5
            text: "Cancel"

            onPressed: {
                root.selectedController = null;
                root.load();
            }
        }
        Skin.FormButton {
            activeColor: "#999999"
            backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
            opacity: enabled ? 1.0 : 0.5
            text: "Save"

            onPressed: {
                errorMessage.text = "";
                root.save();
            }
        }
    }
}
