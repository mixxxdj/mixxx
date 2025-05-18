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
    required property var modelData
    property alias mappings: mappingsCombobox
    property bool focused
    property bool knownDevice: false
    readonly property bool editable: !knownDevice || root.modelData.mapping?.isUserMapping(Mixxx.Config)
    property var settings: null
    property var screens: null

    function loadSettingLayout() {
        root.settings = root.modelData.mapping?.loadSettings(Mixxx.Config, modelData)?.children
        root.screens = Mixxx.ControllerManager.showControllerScreen ? root.modelData.mapping?.loadScreens(Mixxx.Config, modelData) : []
    }

    function saveSettingLayout() {
        root.modelData.mapping?.saveSettingTree(root.settings)
    }

    Connections {
        target: modelData
        function onMappingChanged() {
            loadSettingLayout();
        }
    }
    Component.onCompleted: {
        loadSettingLayout()
    }

    Layout.alignment: Qt.AlignTop
    Layout.fillWidth: true
    Layout.preferredHeight: root.focused ? settingsAndScreenItem.y + settingsAndScreenItem.height + 40 : 180

    signal select

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
            anchors.left: content.left
            anchors.leftMargin: 22
            anchors.rightMargin: 10
            anchors.topMargin: root.focused ? 0 : 17
            implicitHeight: nameInput.visible ? nameInput.implicitHeight : labelText.implicitHeight
            implicitWidth: nameInput.visible ? nameInput.width + 35 : labelText.implicitWidth + 35
            property string value: modelData.name
            clip: true

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        target: label
                        anchors.top: content.top
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        target: label
                        anchors.top: mappingsCombobox.top
                    }
                }
            ]

            TextInput {
                id: nameInput
                visible: false
                text: label.value
                width: 200
                color: Theme.white

                font.pixelSize: 16
                font.weight: Font.DemiBold

                onActiveFocusChanged: {
                    visible = activeFocus
                }
                onAccepted: {
                    visible = false
                    modelData.name = text
                    label.value = modelData.name
                }
            }
            Text {
                id: labelText
                visible: !nameInput.visible
                color: Theme.white
                text: label.value
                font.pixelSize: 16
                font.weight: Font.DemiBold
            }
            Shape {
                id: editButton
                visible: root.editable && !nameInput.visible
                anchors.right: label.right
                width: 20
                ShapePath {
                    strokeColor: Theme.midGray
                    strokeWidth: 1
                    fillColor: "white"
                    capStyle: ShapePath.RoundCap

                    startX: 0
                    startY: 20
                    PathLine { x: 5; y: 18 }
                    PathLine { x: 20; y: 4 }
                    PathLine { x: 16; y: 0 }
                    PathLine { x: 3; y: 13 }
                    PathLine { x: 0; y: 20 }
                }
            }
            InfoBubble {
                id: infoBubbleDevice

                visible: !nameInput.visible
                anchors.leftMargin: 10
                anchors.left: editButton.visible ? editButton.right : labelText.right

                width: labelText.height
                height: labelText.height

                active: deviceInfoPopup.opened

                onPressed: {
                    if (deviceInfoPopup.opened) {
                        deviceInfoPopup.close()
                    } else {
                        deviceInfoPopup.x = infoBubbleDevice.width
                        deviceInfoPopup.y = infoBubbleDevice.height / 2 - deviceInfoPopup.height / 2
                        deviceInfoPopup.open()
                        deviceInfoPopup.forceActiveFocus(Qt.PopupFocusReason)
                    }
                }
                Skin.ActionPopup {
                    id: deviceInfoPopup
                    padding: 6
                    width: 400
                    focus: true
                    facing: Skin.ActionPopup.Facing.Right
                    GridLayout {
                        columns: 2
                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft
                            text: "Vendor"
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
                            text: modelData.vendor
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft
                            text: "Product"
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.alignment: Qt.AlignLeft
                            text: modelData.product
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft
                            text: "Serial number"
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
                            text: modelData.serialNumber
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft
                            text: "Type"
                            font.pixelSize: 12
                            color: Theme.white
                        }
                        Text {
                            Layout.alignment: Qt.AlignRight
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
                            font.pixelSize: 12
                            color: Theme.white
                        }
                    }
                }
            }
            MouseArea {
                enabled: root.editable
                anchors.fill: parent
                cursorShape: root.editable ? Qt.IBeamCursor : Qt.ArrowCursor
                onPressed: {
                    nameInput.visible = true
                    nameInput.forceActiveFocus();
                }
            }
        }
        Text {
            id: versionOrType
            anchors.rightMargin: root.focused ? undefined : 10
            anchors.topMargin: root.focused ? undefined : 17
            anchors.leftMargin: 22
            color: '#FFFFFF'
            text: root.editable ? 'User device' : `Supported since ${modelData.sinceVersion}`
            font.pixelSize: 10
            font.weight: Font.Light

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        target: versionOrType
                        anchors.right: content.right
                        anchors.top: content.top
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        target: versionOrType
                        anchors.left: content.left
                        anchors.bottom: enabledBtn.bottom
                    }
                }
            ]
        }
        Rectangle {
            id: artwork
            anchors.topMargin: root.focused ? 17 : undefined
            anchors.rightMargin: root.focused ? 16 : 20
            anchors.leftMargin: root.focused ? 16 : 34
            anchors.bottomMargin: root.focused ? 20 : 6
            width: 162
            height: 100
            color: 'transparent'

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        target: artwork
                        anchors.bottom: content.bottom
                        anchors.left: content.left
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        target: artwork
                        anchors.right: content.right
                        anchors.top: content.top
                    }
                }
            ]
            Image {
                id: controllerVisual
                visible: !!modelData.sinceVersion
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: modelData.visualUrl
            }
            Item {
                anchors.fill: parent
                visible: controllerVisual.status != Image.Ready || root.editable
                Shape {
                    visible: controllerVisual.status != Image.Ready || !controllerVisual.source
                    anchors.fill: parent
                    ShapePath {
                        strokeColor: Theme.midGray
                        strokeWidth: 1
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        startX: 0
                        startY: 0
                        PathLine { x: controllerVisual.width; y: 0 }
                        PathLine { x: controllerVisual.width; y: controllerVisual.height }
                        PathLine { x: 0; y: controllerVisual.height }
                        PathLine { x: 0; y: 0 }
                        PathLine { x: controllerVisual.width; y: controllerVisual.height }
                        PathLine { x: 0; y: controllerVisual.height }
                        PathLine { x: controllerVisual.width; y: 0 }
                    }
                }

                Rectangle {
                    visible: controllerVisual.status == Image.Ready && visualMouseArea.containsMouse
                    anchors.fill: parent
                    color: Qt.alpha('black', 0.3)
                }

                Text {
                    visible: controllerVisual.status != Image.Ready || visualMouseArea.containsMouse
                    anchors.centerIn: parent
                    color: Theme.white
                    text: root.editable ? `Press to\nset ${!controllerVisual.source.toString() ? 'a' : 'another'} visual` : "No visual"
                    horizontalAlignment: Text.AlignHCenter
                }
                FileDialog {
                    id: fileDialog
                    title: qsTr("Select an image")
                    nameFilters:["Images (*.png *.gif *.jpg)"]
                    onAccepted: {
                        modelData.visualUrl = fileDialog.selectedFile
                    }
                }
                MouseArea {
                    id: visualMouseArea
                    hoverEnabled: true
                    enabled: root.editable
                    anchors.fill: parent
                    cursorShape: !root.editable ? Qt.ArrowCursor : Qt.PointingHandCursor
                    onPressed: {
                        fileDialog.open()
                    }
                }
            }
        }
        Skin.Button {
            id: controllerSettings
            visible: !root.focused
            anchors.bottom: content.bottom
            anchors.right: mappingsCombobox.left
            anchors.bottomMargin: root.focused ? 5 : 16
            anchors.rightMargin: root.focused ? 0 : 3
            activeColor: Theme.white
            icon.height: 16
            icon.source: "images/gear.svg"
            icon.width: 16
            implicitWidth: implicitHeight
            onPressed: {
                root.select()
            }
        }
        Skin.ComboBox {
            id: mappingsCombobox
            anchors.bottomMargin: root.focused ? 0 : 13
            anchors.rightMargin: root.focused ? 15 : 3
            anchors.topMargin: root.focused ? 16 : 17
            width: 180
            implicitHeight: 32
            spacing: 2
            indicator.width: 0
            clip: true
            font.pixelSize: 10

            property bool showAllMappings: !matchingMappings?.length || modelData.availableMappings.indexOf(root.modelData.mapping) == -1
            readonly property var matchingMappings: modelData.availableMappings

            popupMaxItems: showAllMappings ? 6 : matchingMappings?.length + 1

            onCurrentIndexChanged: {
                if (currentIndex == -1) return;

                root.modelData.mapping = mappingsCombobox.model[currentIndex]
            }
            currentIndex: model.indexOf(root.modelData.mapping)
            contentItem: Item {
                Text {
                    id: mappingLabel
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 6
                    anchors.verticalCenter: parent.verticalCenter
                    text: mappingsCombobox.model[mappingsCombobox.currentIndex].name
                    font: mappingsCombobox.font
                    color: Theme.deckTextColor
                }
                InfoBubble {
                    id: infoBubble
                    width: 18
                    height: 18
                    visible: mappingsCombobox.currentIndex >= 0

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 10

                    active: mappingInfoPopup.opened

                    onPressed: {
                        if (mappingInfoPopup.opened) {
                            mappingInfoPopup.close()
                        } else {
                            mappingInfoPopup.x = -mappingInfoPopup.width // - infoBubble.width
                            mappingInfoPopup.y = infoBubble.height / 2 - mappingInfoPopup.height / 2
                            mappingInfoPopup.open()
                            mappingInfoPopup.forceActiveFocus(Qt.PopupFocusReason)
                        }
                    }
                    Skin.ActionPopup {
                        id: mappingInfoPopup
                        padding: 6
                        width: 400
                        focus: true
                        facing: Skin.ActionPopup.Facing.Right

                        GridLayout {
                            columns: 2
                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignLeft
                                text: "Author"
                                font.pixelSize: 12
                                color: Theme.white
                            }
                            Text {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                wrapMode: Text.WordWrap
                                text: (root.modelData.mapping?.author || 'N/A') + (root.modelData.mapping?.isUserMapping(Mixxx.Config) ? ' (edited by you)' : '')
                                font.pixelSize: 12
                                color: Theme.white
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignLeft
                                text: "Forum Link"
                                font.pixelSize: 12
                                color: Theme.white
                            }
                            ControllerMappingLink {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                value: root.modelData.mapping?.forumLink
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignLeft
                                text: "Wiki Link"
                                font.pixelSize: 12
                                color: Theme.white
                            }
                            ControllerMappingLink {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                value: root.modelData.mapping?.wikiLink
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                text: "Description"
                                font.pixelSize: 12
                                color: Theme.white
                            }
                            Text {
                                Layout.alignment: Qt.AlignRight
                                Layout.maximumWidth: 280
                                text: root.modelData.mapping?.description || 'N/A'
                                font.pixelSize: 12
                                color: Theme.white
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignJustify
                            }
                        }
                    }
                }
            }
            delegate: ItemDelegate {
                id: itemDlgt

                required property int index
                required property var modelData

                width: mappingsCombobox.width
                height: visible ? 28 : 0
                highlighted: mappingsCombobox.highlightedIndex === this.index
                text: mappingsCombobox.textAt(this.index)
                padding: 4
                verticalPadding: 8

                visible: mappingsCombobox.matchingMappings.indexOf(modelData) != -1 || mappingsCombobox.showAllMappings

                contentItem: Row {
                    width: itemDlgt.width
                    height: itemDlgt.height
                    Text {
                        text: modelData.name
                        font: mappingsCombobox.font
                        color: Theme.deckTextColor
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    Image {
                        visible: modelData.isUserMapping(Mixxx.Config)
                        anchors {
                            right: parent.right
                            margins: 4
                        }
                        width: 18
                        height: 18
                        source: "../images/work@2x.png"
                    }
                }

                background: Rectangle {
                    color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
                }
            }
            footerItems: [{
                          text: qsTr("Show all mappings..."),
                          suffix: "+"
                          }
            ]
            onActivateFooter: (footerIndex) => {
                mappingsCombobox.footerItems = []
                mappingsCombobox.showAllMappings = true
            }
            model: Mixxx.ControllerManager.mappings(modelData.type)

            states: [
                State {
                    name: "idle"
                    when: !root.focused

                    AnchorChanges {
                        target: mappingsCombobox

                        anchors.right: enabledBtn.left
                        anchors.bottom: content.bottom
                        anchors.top: undefined
                    }
                },
                State {
                    name: "focused"
                    when: root.focused

                    AnchorChanges {
                        target: mappingsCombobox

                        anchors.right: artwork.left
                        anchors.top: artwork.top
                        anchors.bottom: undefined
                    }
                }
            ]
        }
        Skin.Button {
            id: enabledBtn
            anchors.right: root.focused ? artwork.left : content.right
            anchors.bottom: root.focused ? artwork.bottom : content.bottom
            anchors.left: root.focused ? mappingsCombobox.left : undefined
            anchors.rightMargin: root.focused ? 15 : 25
            anchors.bottomMargin: 16
            activeColor: Theme.white
            checkable: true
            checked: modelData.enabled

            onCheckedChanged: {
                modelData.enabled = checked
            }
            text: checked ? "On" : "Off"
        }
        Rectangle {
            id: horizontalRow
            visible: root.focused && root.settings?.length + root.screens?.length > 0
            anchors.topMargin: 17
            implicitHeight: 1
            anchors.top: artwork.bottom
            anchors.left: content.left
            anchors.right: content.right
            color: Theme.midGray
        }
        Item {
            id: settingsAndScreenItem
            visible: root.focused && root.settings?.length + root.screens?.length > 0
            anchors.left: content.left
            anchors.right: content.right
            anchors.top: horizontalRow.bottom
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.topMargin: 10
            height: settingColumn.height
            width: settingColumn.width
            clip: true

            Column {
                id: settingColumn

                ControllerScreens {
                    visible: !!root.screens?.length
                    width: settingsAndScreenItem.width
                    height: implicitHeight
                    screens: root.screens
                }

                Rectangle {
                    visible: root.screens?.length > 0 && root.settings?.length > 0
                    width: settingsAndScreenItem.width
                    height: 1
                    color: Theme.midGray
                }

                component Settings: ControllerSettings {
                    settings: root.settings
                }

                Item {
                    visible: settings.status != Loader.Ready
                    width: settingsAndScreenItem.width
                    height: 100
                    BusyIndicator {
                        anchors.centerIn: parent
                    }
                }

                Loader {
                    id: settings
                    visible: root.settings?.length > 0 && settings.status == Loader.Ready
                    width: settingsAndScreenItem.width
                    height: implicitHeight
                    asynchronous: true
                    sourceComponent: Settings { }
                }
            }
        }
    }
    DropShadow {
        anchors.fill: root
        anchors.margins: 8
        horizontalOffset: 0
        verticalOffset: 0
        radius: root.focused ? 0 : 8.0
        color: "#000000"
        source: content
    }
}
