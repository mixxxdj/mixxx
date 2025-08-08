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
    Layout.preferredHeight: root.focused ? 0 : 180
    Layout.fillHeight: root.focused

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

        GridLayout {
            anchors.fill: parent
            columns: 5

            Item {
                id: label
                Layout.columnSpan: root.focused ? 1 : 2
                Layout.leftMargin: 22
                Layout.rightMargin: 10
                Layout.topMargin: 17
                Layout.fillWidth: root.focused
                implicitHeight: nameInput.visible ? nameInput.implicitHeight : labelText.implicitHeight
                implicitWidth: nameInput.visible ? nameInput.width + 35 : labelText.implicitWidth + 35
                property string value: modelData.name
                clip: true
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
                        width: 300
                        focus: true
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
                Layout.rightMargin: root.focused ? undefined : 10
                Layout.columnSpan: root.focused ? 1 : 3
                Layout.fillWidth: root.focused
                Layout.column: root.focused ? 0 : 2
                Layout.row: root.focused ? 1 : 0
                Layout.alignment: root.focused ? Qt.AlignTop : Qt.AlignRight
                Layout.leftMargin: 22
                color: '#FFFFFF'
                text: root.editable ? 'User device' : `Supported since ${modelData.sinceVersion}`
                font.pixelSize: 10
                font.weight: Font.Light
            }
            Rectangle {
                Layout.topMargin: root.focused ? 17 : undefined
                Layout.rightMargin: root.focused ? 16 : 20
                Layout.leftMargin: root.focused ? 16 : 34
                Layout.bottomMargin: root.focused ? 20 : 6
                Layout.column: root.focused ? 2 : 0
                Layout.row: root.focused ? 0 : 1
                Layout.rowSpan: root.focused ? 2 : 1
                Layout.preferredWidth: 162
                Layout.preferredHeight: 100
                color: 'transparent'
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
                visible: !root.focused && !!root.modelData.mapping?.hasSettings
                Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                Layout.preferredHeight: 26
                Layout.bottomMargin: root.focused ? 5 : 16
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
                Layout.alignment: root.focused ? Qt.AlignVCenter : Qt.AlignBottom
                Layout.bottomMargin: root.focused ? 0 : 13
                Layout.column: root.focused ? 1 : 3
                Layout.row: root.focused ? 0 : 1
                Layout.fillWidth: !root.focused
                Layout.minimumWidth: 180
                Layout.topMargin: 17
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
                contentItem: RowLayout {
                    anchors.leftMargin: 10
                    anchors.fill: parent
                    Text {
                        id: mappingLabel
                        Layout.fillWidth: true
                        text: mappingsCombobox.model[mappingsCombobox.currentIndex].name
                        font: mappingsCombobox.font
                        color: Theme.deckTextColor
                    }
                    InfoBubble {
                        id: infoBubble
                        Layout.preferredWidth: 18
                        Layout.rightMargin: 18

                        height: 18

                        active: mappingInfoPopup.opened

                        onPressed: {
                            if (mappingInfoPopup.opened) {
                                mappingInfoPopup.close()
                            } else {
                                mappingInfoPopup.x = infoBubble.width
                                mappingInfoPopup.y = infoBubble.height / 2 - mappingInfoPopup.height / 2
                                mappingInfoPopup.open()
                                mappingInfoPopup.forceActiveFocus(Qt.PopupFocusReason)
                            }
                        }
                        Skin.ActionPopup {
                            id: mappingInfoPopup
                            padding: 6
                            width: 300
                            focus: true

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
                                    Layout.maximumWidth: 180
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
                                    Layout.maximumWidth: 180
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
                                    Layout.maximumWidth: 180
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
                                    Layout.maximumWidth: 180
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
            }
            Skin.Button {
                id: enabled
                Layout.alignment: root.focused ? Qt.AlignTop : Qt.AlignBottom
                Layout.column: root.focused ? 1 : 4
                Layout.row: 1
                Layout.rightMargin: root.focused ? undefined : 25
                Layout.bottomMargin: root.focused ? 3 : 16
                Layout.preferredWidth: root.focused ? 180 : 40
                activeColor: Theme.white
                checkable: true
                checked: modelData.enabled

                onCheckedChanged: {
                    modelData.enabled = checked
                }
                text: checked ? "On" : "Off"
            }
            Rectangle {
                visible: root.focused && root.settings?.length + root.screens?.length > 0
                implicitHeight: 1
                Layout.fillWidth: true
                Layout.row: 2
                Layout.columnSpan: 3
                color: Theme.midGray
            }
            Item {
                id: settingsAndScreenItem
                visible: root.focused && root.settings?.length + root.screens?.length > 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.row: 5
                Layout.columnSpan: 3
                clip: true
                Flickable {
                    id: settingFlicker
                    anchors {
                        leftMargin: 20
                        rightMargin: 20
                    }
                    anchors.fill: parent

                    boundsMovement: Flickable.StopAtBounds
                    boundsBehavior: Flickable.DragOverBounds

                    contentWidth: 200
                    contentHeight: settingColumn.height

                    Column {
                        id: settingColumn

                        ControllerScreens {
                            visible: !!root.screens?.length
                            width: settingFlicker.width
                            height: implicitHeight
                            screens: root.screens
                        }

                        Rectangle {
                            visible: root.screens?.length > 0 && root.settings?.length > 0
                            width: settingFlicker.width
                            height: 1
                            color: Theme.midGray
                        }

                        ControllerSettings {
                            visible: root.settings?.length > 0
                            width: settingFlicker.width
                            height: implicitHeight
                            settings: root.settings
                        }
                    }
                    ScrollBar.vertical: ScrollBar {
                        anchors {
                            top: parent.top
                            left: parent.right
                            leftMargin: 6
                            bottom: parent.bottom
                        }

                        id: scrollbar

                        contentItem: Rectangle {
                            implicitWidth: 10
                            radius: 4
                            z: 100
                            color: Theme.white
                            opacity: scrollbar.policy === ScrollBar.AlwaysOn || scrollbar.size < 1.0 ? (scrollbar.pressed ? 1 : 0.75) : 0
                        }
                        background: Rectangle {
                            implicitWidth: 14
                            z: 100
                            color: scrollbar.policy === ScrollBar.AlwaysOn || scrollbar.size < 1.0 ? Theme.midGray : 'transparent'
                        }
                    }
                }
            }
        }
        Rectangle {
            visible: root.focused
            height: 40
            z: 50
            anchors {
                right: parent.right
                left: parent.left
                bottom:parent.bottom
            }
            radius: content.radius
            gradient: Gradient {
                orientation: Gradient.Vertical

                GradientStop {
                    position: 0.6
                    color: 'transparent'
                }

                GradientStop {
                    position: 1
                    color: Theme.darkGray
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
