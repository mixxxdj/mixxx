import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Shapes 1.6
import Qt.labs.qmlmodels
import Qt5Compat.GraphicalEffects
import ".." as Skin
import "../Theme"

Item {
    id: root
    required property var modelData
    property alias mappings: mappingsCombobox
    property bool focused
    property bool knownDevice: false
    readonly property bool dirty: enabled.changed || settingsItem.changed
    property var selectedMapping: null
    property var settings: null
    onSelectedMappingChanged: {
        //     if (root.mappings.currentIndex)
        console.log("selectedMapping", root.selectedMapping)
        root.settings = root.selectedMapping?.getSettingTree(Mixxx.Config)?.children
        console.log("root.settings", root.settings)
    }
    Layout.alignment: Qt.AlignTop
    Layout.fillWidth: true
    Layout.preferredHeight: root.focused ? undefined : 180
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
                Layout.leftMargin: 34
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

                    onTextEdited: {
                        label.value = nameInput.text
                    }

                    onActiveFocusChanged: {
                        visible = activeFocus
                    }
                    onAccepted: {
                        visible = false
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
                    visible: !root.knownDevice && !nameInput.visible
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
                Rectangle {
                    visible: !nameInput.visible
                    anchors.leftMargin: 10
                    anchors.left: editButton.visible ? editButton.right : labelText.right
                    color: 'transparent'
                    border.color: 'white'
                    border.width: 1
                    radius: labelText.height/2
                    width: labelText.height
                    height: labelText.height
                    MouseArea {
                        hoverEnabled: true
                        anchors.fill: parent
                        onEntered: {
                            popup.x = parent.width
                            popup.y = parent.height / 2 - popup.height / 2
                            popup.open()
                            popup.forceActiveFocus(Qt.PopupFocusReason)
                        }
                        onExited: {
                            popup.close()
                        }
                        onPressed: {
                            popup.x = parent.width
                            popup.y = parent.height / 2 - popup.height / 2
                            popup.open()
                            popup.forceActiveFocus(Qt.PopupFocusReason)
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "!"
                        color: 'white'
                    }
                    Skin.ActionPopup {
                        id: popup
                        padding: 6
                        width: 300
                        focus: true
                        GridLayout {
                            anchors.fill: parent
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
                    enabled: !root.knownDevice
                    anchors.fill: parent
                    cursorShape: !root.knownDevice ? Qt.IBeamCursor : Qt.ArrowCursor
                    onPressed: {
                        console.log("edit")
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
                Layout.leftMargin: 34
                color: '#FFFFFF'
                text: !root.knownDevice ? 'User device' : `Supported since ${modelData.sinceVersion}`
                font.pixelSize: 10
                font.weight: Font.Light
            }
                // Layout.fillWidth: true
                // Layout.fillHeight: true
                // Layout.rightMargin: 10
                // spacing: 7
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
                    visible: controllerVisual.status != Image.Ready || !root.knownDevice
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

                    Text {
                        anchors.centerIn: parent
                        color: 'white'
                        text: !root.knownDevice ? `Press to\nset ${!controllerVisual.source.toString() ? 'a' : 'another'} visual` : "No visual"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    FileDialog {
                        id: fileDialog
                        title: qsTr("Select an image")
                        nameFilters:["Images (*.png *.gif *.jpg)"]
                        onAccepted: {
                            controllerVisual.source = fileDialog.selectedFile
                            console.log("You chose: " + controllerVisual.source)
                        }
                    }
                    MouseArea {
                        enabled: !root.knownDevice
                        anchors.fill: parent
                        cursorShape: root.knownDevice ? Qt.ArrowCursor : Qt.PointingHandCursor
                        onPressed: {
                            fileDialog.open()
                        }
                    }
                }
            }
            Skin.Button {
                visible: !root.focused && !!root.selectedMapping?.hasSettings
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

                property bool showingAllMappingOption: root.knownDevice

                onCurrentIndexChanged: {
                    if (showingAllMappingOption && currentIndex == count-1) {
                        return
                    } else if (showingAllMappingOption) {
                        root.selectedMapping = modelData.availableMappings[currentIndex]
                    } else {
                        root.selectedMapping = Mixxx.ControllerManager.mappings(modelData.type)[currentIndex]
                    }
                }

                popup.onClosed: {
                    if (mappingsCombobox.showingAllMappingOption && mappingsCombobox.currentIndex == mappingsCombobox.count-1) {
                        mappingsCombobox.showingAllMappingOption = false
                        mappingsCombobox.model = Mixxx.ControllerManager.mappings(modelData.type).map(item => item.name)
                        mappingsCombobox.popup.open()
                    }
                }
                contentItem: RowLayout {
                    anchors.leftMargin: 10
                    anchors.fill: parent
                    Text {
                        id: mappingLabel
                        Layout.fillWidth: true
                        text: mappingsCombobox.displayText
                        font: mappingsCombobox.font
                        color: Theme.deckTextColor
                    }
                    Rectangle {
                        color: 'transparent'
                        border.color: 'white'
                        border.width: 1
                        radius: mappingLabel.height/2
                        Layout.preferredWidth: mappingLabel.height
                        Layout.rightMargin: 20
                        height: mappingLabel.height
                        MouseArea {
                            hoverEnabled: true
                            anchors.fill: parent
                            onEntered: {
                                mappingInfoPopup.x = parent.width
                                mappingInfoPopup.y = parent.height / 2 - mappingInfoPopup.height / 2
                                mappingInfoPopup.open()
                                mappingInfoPopup.forceActiveFocus(Qt.PopupFocusReason)
                            }
                            onExited: {
                                mappingInfoPopup.close()
                            }
                            onPressed: {
                                mappingInfoPopup.x = parent.width
                                mappingInfoPopup.y = parent.height / 2 - mappingInfoPopup.height / 2
                                mappingInfoPopup.open()
                                mappingInfoPopup.forceActiveFocus(Qt.PopupFocusReason)
                            }
                        }
                        Text {
                            anchors.centerIn: parent
                            text: "!"
                            color: 'white'
                        }
                        Skin.ActionPopup {
                            id: mappingInfoPopup
                            padding: 6
                            width: 300
                            focus: true
                            GridLayout {
                                anchors.fill: parent
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
                                    text: root.selectedMapping?.author || 'N/A'
                                    font.pixelSize: 12
                                    color: Theme.white
                                }
                                Text {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignLeft
                                    text: "Description"
                                    font.pixelSize: 12
                                    color: Theme.white
                                }
                                Text {
                                    Layout.alignment: Qt.AlignLeft
                                    text: root.selectedMapping?.description || 'N/A'
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
                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    text: root.selectedMapping?.forumLink || 'N/A'
                                    font.pixelSize: 12
                                    color: Theme.white
                                }
                                Text {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignLeft
                                    text: "Wiki Link"
                                    font.pixelSize: 12
                                    color: Theme.white
                                }
                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    text: root.selectedMapping?.wikiLink || 'N/A'
                                    font.pixelSize: 12
                                    color: Theme.white
                                }
                            }
                        }
                    }
                }
                delegate: ItemDelegate {
                    id: itemDlgt

                    required property int index

                    width: mappingsCombobox.width
                    highlighted: mappingsCombobox.highlightedIndex === this.index
                    text: mappingsCombobox.textAt(this.index)
                    padding: 4
                    verticalPadding: 8

                    contentItem: RowLayout {
                        Text {
                            text: itemDlgt.text
                            font: mappingsCombobox.font
                            color: Theme.deckTextColor
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Text {
                            visible: index == mappingsCombobox.count - 1 && mappingsCombobox.showingAllMappingOption
                            text: "+"
                            font.pixelSize: 16
                            color: Theme.deckTextColor
                        }
                    }

                    background: Rectangle {
                        color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
                    }
                }
                model: showingAllMappingOption ? [...modelData.availableMappings.map(item => item.name), "Show all mappings..."] : Mixxx.ControllerManager.mappings(modelData.type).map(item => item.name)
            }
            Skin.Button {
                id: enabled
                Layout.alignment: root.focused ? Qt.AlignTop : Qt.AlignBottom
                Layout.column: root.focused ? 1 : 4
                Layout.row: 1
                Layout.rightMargin: root.focused ? undefined : 25
                Layout.bottomMargin: root.focused ? 3 : 16
                Layout.preferredWidth: root.focused ? 180 : 40
                // Layout.fillWidth: !root.focused
                activeColor: Theme.white
                checkable: true
                checked: modelData.enabled

                property bool changed: false

                onCheckedChanged: {
                    console.log("CHECKED", modelData.enabled, enabled.checked != modelData.enabled)
                    enabled.changed = enabled.checked != modelData.enabled
                }
                text: checked ? "On" : "Off"
            }
            Rectangle {
                visible: root.focused
                implicitHeight: 1
                Layout.fillWidth: true
                Layout.row: 2
                Layout.columnSpan: 3
                color: '#626262'
            }
            Item {
                id: settingsItem
                property bool changed: false
                visible: root.focused && settings?.length > 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.row: 3
                Layout.columnSpan: 3
                clip: true
                Component.onCompleted: {
                    console.log("Item", implicitHeight, implicitWidth, width)
                }
                Flickable {
                    id: settingFlicker
                    anchors.margins: 20
                    anchors.fill: parent
                    boundsMovement: Flickable.StopAtBounds
                    boundsBehavior: Flickable.DragOverBounds

                    // width: 400
                    // height: 200

                    contentWidth: 200
                    contentHeight: settingColumn.height

                    DelegateChooser {
                        id: item
                        role: "type"
                        DelegateChoice {
                            roleValue: "enum"
                            Skin.ComboBox {
                                id: settingCombobox
                                required property var modelData

                                // Component.onCompleted: {
                                //     console.log(`ComboBox ${JSON.stringify(modelData)}`)
                                // }
                                delegate: ItemDelegate {
                                    id: itemDlgt

                                    required property int index
                                    readonly property var color: settingCombobox.model[this.index].color

                                    width: settingCombobox.width
                                    highlighted: settingCombobox.highlightedIndex === this.index
                                    text: settingCombobox.model[this.index].label
                                    padding: 4
                                    verticalPadding: 8

                                    contentItem: RowLayout {
                                        Rectangle {
                                            visible: color.valid
                                            Layout.minimumWidth: 15
                                            Layout.minimumHeight: 15
                                            color: itemDlgt.color
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: itemDlgt.text
                                            font: settingCombobox.font
                                            color: Theme.deckTextColor
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }

                                    background: Rectangle {
                                        color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
                                    }
                                }
                                contentItem: RowLayout {
                                    anchors.leftMargin: 10
                                    anchors.fill: parent
                                    Rectangle {
                                        visible: color.valid
                                        Layout.minimumWidth: 15
                                        Layout.minimumHeight: 15
                                        color: settingCombobox.model[settingCombobox.currentIndex].color
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: settingCombobox.model[settingCombobox.currentIndex].label
                                        font: settingCombobox.font
                                        color: Theme.deckTextColor
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                Layout.fillWidth: true
                                implicitHeight: 32
                                spacing: 2
                                indicator.width: 0
                                clip: true
                                font.pixelSize: 10
                                model: modelData.options
                                currentIndex: modelData.options.map(option=>option.value).indexOf(modelData.value)
                                onCurrentIndexChanged: {
                                    settingsItem.changed |= currentIndex != modelData.options.map(option=>option.value).indexOf(modelData.value)
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "color"
                            Skin.Button {
                                Layout.fillWidth: true
                                required property var modelData
                                activeColor: modelData.value
                                text: "Change color"
                                Rectangle {
                                    id: indicator
                                    width: 10
                                    height: 10
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.margins: (parent.height - 10) / 2
                                    color: modelData.value
                                }

                                ColorDialog {
                                    id: colorDialog
                                    title: "Please choose a color"
                                    onAccepted: {
                                        indicator.color = Qt.color(selectedColor);
                                    }
                                    selectedColor: modelData.value
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onPressed: {
                                        colorDialog.open()
                                    }
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "boolean"
                            RatioChoice {
                                required property var modelData
                                options: ["off", "on"]
                                selected: modelData.value ? "on" : "off"
                                onSelectedChanged: {
                                    settingsItem.changed |= selected != modelData.value
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "number"
                            SpinBox {
                                id: spinBox
                                required property var modelData
                                from: modelData.min
                                value: decimalToInt(modelData.value)
                                to: decimalToInt(modelData.max)
                                stepSize: modelData.step * decimalFactor
                                editable: true

                                property int decimals: modelData.precision ?? 0
                                property real realValue: value / decimalFactor
                                readonly property int decimalFactor: Math.pow(10, decimals)

                                // Component.onCompleted: {
                                //     console.log(`SpinBox ${JSON.stringify(modelData)} ${typeof modelData} from=${from}(${modelData.min}),value=${value}(${modelData.value}),to=${to}(${modelData.max}),decimals=${decimals},realValue=${realValue},decimalFactor=${decimalFactor}`)
                                // }

                                function decimalToInt(decimal) {
                                    return decimal * decimalFactor
                                }

                                validator: DoubleValidator {
                                    bottom: Math.min(spinBox.from, spinBox.to)
                                    top: Math.max(spinBox.from, spinBox.to)
                                    decimals: spinBox.decimals
                                    notation: DoubleValidator.StandardNotation
                                }

                                textFromValue: function(value, locale) {
                                    return Number(value / decimalFactor).toLocaleString(locale, 'f', spinBox.decimals)
                                }

                                valueFromText: function(text, locale) {
                                    return Math.round(Number.fromLocaleString(locale, text) * decimalFactor)
                                }

                                onValueChanged: {
                                    settingsItem.changed |= value != decimalToInt(modelData.value)
                                }
                                padding: 0
                                spacing: 2
                                contentItem: Item {
                                    width: spinBox.textWidth + 2 * spinBox.spacing
                                    Rectangle {
                                        id: content
                                        anchors.fill: parent
                                        color: Theme.accentColor
                                        Text {
                                            id: textLabel
                                            anchors.fill: parent
                                            text: spinBox.textFromValue(spinBox.value, spinBox.locale) ?? ""
                                            color: Theme.white
                                            font: spinBox.font
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                    InnerShadow {
                                        id: bottomInnerEffect
                                        anchors.fill: parent
                                        radius: 8
                                        samples: 32
                                        spread: 0.4
                                        horizontalOffset: -1
                                        verticalOffset: -1
                                        color: "#0E2A54"
                                        source: content
                                    }
                                    InnerShadow {
                                        id: topInnerEffect
                                        anchors.fill: parent
                                        radius: 8
                                        samples: 32
                                        spread: 0.4
                                        horizontalOffset: 1
                                        verticalOffset: 1
                                        color: "#0E2A54"
                                        source: bottomInnerEffect
                                    }
                                }

                                component Indicator: Rectangle {
                                    required property string text
                                    height: implicitHeight
                                    implicitWidth: 24
                                    implicitHeight: 24
                                    radius: 2
                                    color: '#2B2B2B'
                                    border.width: 0

                                    Text {
                                        text: parent.text
                                        font.pixelSize: spinBox.font.pixelSize
                                        color: Theme.white
                                        anchors.fill: parent
                                        fontSizeMode: Text.Fit
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                up.indicator: Indicator {
                                    x: spinBox.mirrored ? 0 : parent.width - width
                                    text: "+"
                                }

                                down.indicator: Indicator {
                                    x: spinBox.mirrored ? parent.width - width : 0
                                    text: "-"
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "file"
                            RowLayout {
                                Layout.fillWidth: true
                                required property var modelData

                                Text {
                                    Layout.fillWidth: true
                                    id: label
                                    text: modelData.value ? modelData.value : "No file selected"
                                    font.italic: !modelData.value
                                    elide: Text.ElideLeft
                                    color: 'white'
                                }
                                Skin.Button {
                                    Layout.minimumWidth: 100
                                    activeColor: modelData.value
                                    Component.onCompleted: {
                                        console.log("You chose: " + JSON.stringify(modelData))
                                    }
                                    text: "Select file"
                                    FileDialog {
                                        id: fileDialog
                                        title: qsTr("Select a file")
                                        nameFilters:[modelData.fileFilter]
                                        onAccepted: {
                                            label.text = fileDialog.selectedFile
                                            label.font.italic = false
                                            console.log("You chose: " + fileDialog.selectedFile)
                                        }
                                        onRejected: {
                                            console.log("Canceled")
                                        }
                                        // Component.onCompleted: visible = true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onPressed: {
                                            fileDialog.open()
                                        }
                                    }
                                }
                            }
                        }
                        DelegateChoice {
                            Text {
                                Layout.fillWidth: true
                                required property var modelData
                                text: `Unsupported setting of type ${modelData.type}`
                            }
                        }
                    }

                    DelegateChooser {
                        id: chooser
                        role: "type"
                        DelegateChoice {
                            roleValue: "item"
                            GridLayout {
                                // implicitWidth: 40
                                Layout.preferredWidth: 100
                                // Layout.fillWidth: true
                                required property var modelData
                                columns: modelData.preferredOrientation == 0 ? 2 : 1
                                rowSpacing: 10
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.properties.label
                                    color: 'white'
                                }
                                Repeater {
                                    Layout.preferredWidth: 100
                                    model: [modelData.properties]
                                    delegate: item
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "container"

                            GridLayout {
                                Layout.preferredWidth: 100
                                Layout.topMargin: 10
                                Layout.bottomMargin: 10
                                columnSpacing: 60
                                required property var modelData
                                // columns: 2
                                columns: modelData.disposition == 0 ? modelData.children.length : 1
                                Repeater {
                                    model: modelData.children
                                    delegate: chooser
                                }
                            }
                        }
                        DelegateChoice {
                            roleValue: "group"
                            Rectangle {
                                required property var modelData
                                color: '#282828'
                                implicitHeight: column.implicitHeight + 30
                                Layout.fillWidth: true
                                Layout.topMargin: 32
                                ColumnLayout {
                                    anchors.margins: 10
                                    anchors.fill: parent
                                    Layout.fillWidth: true
                                    spacing: 4
                                    id: column
                                    Text {
                                        text: modelData.label
                                        color: 'white'
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                    }
                                    Repeater {
                                        model: modelData.children
                                        delegate: chooser
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        id: settingColumn
                        width: settingFlicker.width
                        // Component.onCompleted: {
                        //     console.log("settings: " + JSON.stringify(root.settings))
                        // }

                        Repeater {
                            model: root.settings
                            delegate: chooser
                        }
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
                // InnerShadow {
                //     id: bottomInnerEffect
                //     anchors.fill: parent
                //     radius: 24
                //     samples: 16
                //     spread: 0.6
                //     horizontalOffset: 0
                //     verticalOffset: -2
                //     color: "#000000"
                //     source: settingFlicker.contentItem
                // }
                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: 10
                    color: '#696968'
                    Rectangle {
                        id: scrollbar
                        anchors.right: parent.right
                        anchors.left: parent.left
                        y: settingFlicker.visibleArea.yPosition * settingFlicker.height
                        // width: 10
                        height: settingFlicker.visibleArea.heightRatio * settingFlicker.height
                        color: "#D9D9D9"
                        radius: 4
                    }
                }
            }
        }
    }
    DropShadow {
        anchors.fill: parent
        anchors.margins: 8
        horizontalOffset: 0
        verticalOffset: 0
        radius: 8.0
        color: "#000000"
        source: content
    }
}
