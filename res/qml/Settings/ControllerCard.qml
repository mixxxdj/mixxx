import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Shapes 1.6
import Qt.labs.qmlmodels
import QtMultimedia
import Qt5Compat.GraphicalEffects
import ".." as Skin
import "../Theme"

Item {
    id: root

    readonly property bool editable: !knownDevice || root.modelData.mapping?.isUserMapping(Mixxx.Config)
    property bool focused
    property bool knownDevice: false
    property alias mappings: mappingsCombobox
    required property var modelData
    property var screens: null
    property var settings: null

    signal select

    function loadSettingLayout() {
        root.settings = root.modelData.mapping?.loadSettings(Mixxx.Config, modelData)?.children;
        root.screens = Mixxx.ControllerManager.showControllerScreen ? root.modelData.mapping?.loadScreens(Mixxx.Config, modelData) : [];
    }
    function saveSettingLayout() {
        root.modelData.mapping?.saveSettingTree(root.settings);
    }

    Layout.alignment: Qt.AlignTop
    Layout.fillWidth: true
    Layout.preferredHeight: root.focused ? settingsAndScreenItem.y + settingsAndScreenItem.height + 40 : 180

    Component.onCompleted: {
        loadSettingLayout();
    }

    Connections {
        function onMappingChanged() {
            loadSettingLayout();
        }

        target: modelData
    }
    Mixxx.SettingParameter {
        label: modelData.name
    }
    Rectangle {
        id: content

        anchors.fill: parent
        anchors.margins: 8
        color: '#181818'
        radius: root.focused ? 18 : 22

        Item {
            id: label

            property string value: modelData.name

            anchors.left: content.left
            anchors.leftMargin: 22
            anchors.rightMargin: 10
            anchors.topMargin: root.focused ? 0 : 17
            clip: true
            implicitHeight: nameInput.visible ? nameInput.implicitHeight : labelText.implicitHeight
            implicitWidth: nameInput.visible ? nameInput.width + 35 : labelText.implicitWidth + 35

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        anchors.top: content.top
                        target: label
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        anchors.top: mappingsCombobox.top
                        target: label
                    }
                }
            ]

            TextInput {
                id: nameInput

                color: Theme.white
                font.pixelSize: 16
                font.weight: Font.DemiBold
                text: label.value
                visible: false
                width: 200

                onAccepted: {
                    visible = false;
                    modelData.name = text;
                    label.value = modelData.name;
                }
                onActiveFocusChanged: {
                    visible = activeFocus;
                }
            }
            Text {
                id: labelText

                color: Theme.white
                font.pixelSize: 16
                font.weight: Font.DemiBold
                text: label.value
                visible: !nameInput.visible
            }
            Shape {
                id: editButton

                anchors.right: label.right
                visible: root.editable && !nameInput.visible
                width: 20

                ShapePath {
                    capStyle: ShapePath.RoundCap
                    fillColor: "white"
                    startX: 0
                    startY: 20
                    strokeColor: Theme.midGray
                    strokeWidth: 1

                    PathLine {
                        x: 5
                        y: 18
                    }
                    PathLine {
                        x: 20
                        y: 4
                    }
                    PathLine {
                        x: 16
                        y: 0
                    }
                    PathLine {
                        x: 3
                        y: 13
                    }
                    PathLine {
                        x: 0
                        y: 20
                    }
                }
            }
            InfoBubble {
                id: infoBubbleDevice

                active: deviceInfoPopup.opened
                anchors.left: editButton.visible ? editButton.right : labelText.right
                anchors.leftMargin: 10
                height: labelText.height
                visible: !nameInput.visible
                width: labelText.height

                onPressed: {
                    if (deviceInfoPopup.opened) {
                        deviceInfoPopup.close();
                    } else {
                        deviceInfoPopup.x = infoBubbleDevice.width;
                        deviceInfoPopup.y = infoBubbleDevice.height / 2 - deviceInfoPopup.height / 2;
                        deviceInfoPopup.open();
                        deviceInfoPopup.forceActiveFocus(Qt.PopupFocusReason);
                    }
                }

                Skin.ActionPopup {
                    id: deviceInfoPopup

                    facing: Skin.ActionPopup.Facing.Right
                    focus: true
                    padding: 6
                    width: 400

                    GridLayout {
                        columns: 2

                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            color: Theme.white
                            font.pixelSize: 12
                            text: "Vendor"
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
                            color: Theme.white
                            font.pixelSize: 12
                            text: modelData.vendor
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            color: Theme.white
                            font.pixelSize: 12
                            text: "Product"
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            color: Theme.white
                            font.pixelSize: 12
                            text: modelData.product
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            color: Theme.white
                            font.pixelSize: 12
                            text: "Serial number"
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
                            color: Theme.white
                            font.pixelSize: 12
                            text: modelData.serialNumber
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            color: Theme.white
                            font.pixelSize: 12
                            text: "Type"
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
                            color: Theme.white
                            font.pixelSize: 12
                            text: {
                                switch (modelData.type) {
                                case Mixxx.ControllerDevice.Type.MIDI:
                                    return "MIDI";
                                case Mixxx.ControllerDevice.Type.HID:
                                    return "HID";
                                case Mixxx.ControllerDevice.Type.BULK:
                                    return "BULK";
                                }
                                return "N/A";
                            }
                        }
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: root.editable ? Qt.IBeamCursor : Qt.ArrowCursor
                enabled: root.editable

                onPressed: {
                    nameInput.visible = true;
                    nameInput.forceActiveFocus();
                }
            }
        }
        Text {
            id: versionOrType

            anchors.leftMargin: 22
            anchors.rightMargin: root.focused ? undefined : 10
            anchors.topMargin: root.focused ? undefined : 17
            color: '#FFFFFF'
            font.pixelSize: 10
            font.weight: Font.Light
            text: root.editable ? 'User device' : `Supported since ${modelData.sinceVersion}`

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        anchors.right: content.right
                        anchors.top: content.top
                        target: versionOrType
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        anchors.bottom: enabledBtn.bottom
                        anchors.left: content.left
                        target: versionOrType
                    }
                }
            ]
        }
        Rectangle {
            id: artwork

            anchors.bottomMargin: root.focused ? 20 : 6
            anchors.leftMargin: root.focused ? 16 : 34
            anchors.rightMargin: root.focused ? 16 : 20
            anchors.topMargin: root.focused ? 17 : undefined
            color: 'transparent'
            height: 100
            width: 162

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        anchors.bottom: content.bottom
                        anchors.left: content.left
                        target: artwork
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        anchors.right: content.right
                        anchors.top: content.top
                        target: artwork
                    }
                }
            ]

            Image {
                id: controllerVisual

                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: modelData.visualUrl
                visible: !!modelData.sinceVersion
            }
            Item {
                anchors.fill: parent
                visible: controllerVisual.status != Image.Ready || root.editable

                Shape {
                    anchors.fill: parent
                    visible: controllerVisual.status != Image.Ready || !controllerVisual.source

                    ShapePath {
                        capStyle: ShapePath.RoundCap
                        fillColor: "transparent"
                        startX: 0
                        startY: 0
                        strokeColor: Theme.midGray
                        strokeWidth: 1

                        PathLine {
                            x: controllerVisual.width
                            y: 0
                        }
                        PathLine {
                            x: controllerVisual.width
                            y: controllerVisual.height
                        }
                        PathLine {
                            x: 0
                            y: controllerVisual.height
                        }
                        PathLine {
                            x: 0
                            y: 0
                        }
                        PathLine {
                            x: controllerVisual.width
                            y: controllerVisual.height
                        }
                        PathLine {
                            x: 0
                            y: controllerVisual.height
                        }
                        PathLine {
                            x: controllerVisual.width
                            y: 0
                        }
                    }
                }
                Rectangle {
                    anchors.fill: parent
                    color: Qt.alpha('black', 0.3)
                    visible: controllerVisual.status == Image.Ready && visualMouseArea.containsMouse
                }
                Text {
                    anchors.centerIn: parent
                    color: Theme.white
                    horizontalAlignment: Text.AlignHCenter
                    text: root.editable ? `Press to\nset ${!controllerVisual.source.toString() ? 'a' : 'another'} visual` : "No visual"
                    visible: controllerVisual.status != Image.Ready || visualMouseArea.containsMouse
                }
                FileDialog {
                    id: fileDialog

                    nameFilters: ["Images (*.png *.gif *.jpg)"]
                    title: qsTr("Select an image")

                    onAccepted: {
                        modelData.visualUrl = fileDialog.selectedFile;
                    }
                }
                MouseArea {
                    id: visualMouseArea

                    anchors.fill: parent
                    cursorShape: !root.editable ? Qt.ArrowCursor : Qt.PointingHandCursor
                    enabled: root.editable
                    hoverEnabled: true

                    onPressed: {
                        fileDialog.open();
                    }
                }
            }
        }
        Skin.Button {
            id: controllerSettings

            activeColor: Theme.white
            anchors.bottom: content.bottom
            anchors.bottomMargin: root.focused ? 5 : 16
            anchors.right: mappingsCombobox.left
            anchors.rightMargin: root.focused ? 0 : 3
            icon.height: 16
            icon.source: "images/gear.svg"
            icon.width: 16
            implicitWidth: implicitHeight
            visible: !root.focused

            onPressed: {
                root.select();
            }
        }
        Skin.ComboBox {
            id: mappingsCombobox

            readonly property var matchingMappings: modelData.availableMappings
            property bool showAllMappings: !matchingMappings?.length || modelData.availableMappings.indexOf(root.modelData.mapping) == -1

            anchors.bottomMargin: root.focused ? 0 : 13
            anchors.rightMargin: root.focused ? 15 : 3
            anchors.topMargin: root.focused ? 16 : 17
            clip: true
            currentIndex: model.indexOf(root.modelData.mapping)
            font.pixelSize: 10
            footerItems: [
                {
                    text: qsTr("Show all mappings..."),
                    suffix: "+"
                }
            ]
            implicitHeight: 32
            indicator.width: 0
            model: Mixxx.ControllerManager.mappings(modelData.type)
            popupMaxItem: showAllMappings ? 6 : matchingMappings?.length + 1
            spacing: 2
            width: 180

            contentItem: Item {
                Text {
                    id: mappingLabel

                    anchors.left: parent.left
                    anchors.margins: 6
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.deckTextColor
                    font: mappingsCombobox.font
                    text: mappingsCombobox.model[mappingsCombobox.currentIndex].name
                }
                InfoBubble {
                    id: infoBubble

                    active: mappingInfoPopup.opened
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    height: 18
                    visible: mappingsCombobox.currentIndex >= 0
                    width: 18

                    onPressed: {
                        if (mappingInfoPopup.opened) {
                            mappingInfoPopup.close();
                        } else {
                            mappingInfoPopup.x = -mappingInfoPopup.width; // - infoBubble.width
                            mappingInfoPopup.y = infoBubble.height / 2 - mappingInfoPopup.height / 2;
                            mappingInfoPopup.open();
                            mappingInfoPopup.forceActiveFocus(Qt.PopupFocusReason);
                        }
                    }

                    Skin.ActionPopup {
                        id: mappingInfoPopup

                        facing: Skin.ActionPopup.Facing.Right
                        focus: true
                        padding: 6
                        width: 400

                        GridLayout {
                            columns: 2

                            Text {
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 12
                                text: "Author"
                            }
                            Text {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                color: Theme.white
                                font.pixelSize: 12
                                text: (root.modelData.mapping?.author || 'N/A') + (root.modelData.mapping?.isUserMapping(Mixxx.Config) ? ' (edited by you)' : '')
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 12
                                text: "Forum Link"
                            }
                            ControllerMappingLink {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                value: root.modelData.mapping?.forumLink
                            }
                            Text {
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 12
                                text: "Wiki Link"
                            }
                            ControllerMappingLink {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                value: root.modelData.mapping?.wikiLink
                            }
                            Text {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.fillWidth: true
                                color: Theme.white
                                font.pixelSize: 12
                                text: "Description"
                            }
                            Text {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                color: Theme.white
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignJustify
                                text: root.modelData.mapping?.description || 'N/A'
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
            delegate: ItemDelegate {
                id: itemDlgt

                required property int index
                required property var modelData

                height: visible ? 28 : 0
                highlighted: mappingsCombobox.highlightedIndex === this.index
                padding: 4
                text: mappingsCombobox.textAt(this.index)
                verticalPadding: 8
                visible: mappingsCombobox.matchingMappings.indexOf(modelData) != -1 || mappingsCombobox.showAllMappings
                width: mappingsCombobox.width

                background: Rectangle {
                    color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
                }
                contentItem: Row {
                    height: itemDlgt.height
                    width: itemDlgt.width

                    Text {
                        color: Theme.deckTextColor
                        elide: Text.ElideRight
                        font: mappingsCombobox.font
                        text: modelData.name
                        verticalAlignment: Text.AlignVCenter
                    }
                    Image {
                        height: 18
                        source: "../images/work@2x.png"
                        visible: modelData.isUserMapping(Mixxx.Config)
                        width: 18

                        anchors {
                            margins: 4
                            right: parent.right
                        }
                    }
                }
            }
            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        anchors.bottom: content.bottom
                        anchors.right: enabledBtn.left
                        anchors.top: undefined
                        target: mappingsCombobox
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        anchors.bottom: undefined
                        anchors.right: artwork.left
                        anchors.top: artwork.top
                        target: mappingsCombobox
                    }
                }
            ]

            onActivateFooter: footerIndex => {
                mappingsCombobox.footerItems = [];
                mappingsCombobox.showAllMappings = true;
            }
            onCurrentIndexChanged: {
                if (currentIndex == -1)
                    return;

                root.modelData.mapping = mappingsCombobox.model[currentIndex];
            }
        }
        Skin.Button {
            id: enabledBtn

            activeColor: Theme.white
            anchors.bottom: root.focused ? artwork.bottom : content.bottom
            anchors.bottomMargin: 16
            anchors.left: root.focused ? mappingsCombobox.left : undefined
            anchors.right: root.focused ? artwork.left : content.right
            anchors.rightMargin: root.focused ? 15 : 25
            checkable: true
            checked: modelData.enabled
            text: checked ? "On" : "Off"

            onCheckedChanged: {
                modelData.enabled = checked;
            }
        }
        Rectangle {
            id: horizontalRow

            anchors.left: content.left
            anchors.right: content.right
            anchors.top: artwork.bottom
            anchors.topMargin: 17
            color: Theme.midGray
            implicitHeight: 1
            visible: root.focused && root.settings?.length + root.screens?.length > 0
        }
        Item {
            id: settingsAndScreenItem

            anchors.left: content.left
            anchors.leftMargin: 20
            anchors.right: content.right
            anchors.rightMargin: 20
            anchors.top: horizontalRow.bottom
            anchors.topMargin: 10
            clip: true
            height: settingColumn.height
            visible: root.focused && root.settings?.length + root.screens?.length > 0
            width: settingColumn.width

            Column {
                id: settingColumn

                ControllerScreens {
                    height: implicitHeight
                    screens: root.screens
                    visible: !!root.screens?.length
                    width: settingsAndScreenItem.width
                }
                Rectangle {
                    color: Theme.midGray
                    height: 1
                    visible: root.screens?.length > 0 && root.settings?.length > 0
                    width: settingsAndScreenItem.width
                }
                Item {
                    height: 100
                    visible: settings.status != Loader.Ready
                    width: settingsAndScreenItem.width

                    BusyIndicator {
                        anchors.centerIn: parent
                    }
                }
                Loader {
                    id: settings

                    asynchronous: true
                    height: implicitHeight
                    visible: root.settings?.length > 0 && settings.status == Loader.Ready
                    width: settingsAndScreenItem.width

                    sourceComponent: Settings {
                    }
                }
            }
        }
    }
    DropShadow {
        anchors.fill: root
        anchors.margins: 8
        color: "#000000"
        horizontalOffset: 0
        radius: root.focused ? 0 : 8.0
        source: content
        verticalOffset: 0
    }

    component Settings: ControllerSettings {
        settings: root.settings
    }
}
