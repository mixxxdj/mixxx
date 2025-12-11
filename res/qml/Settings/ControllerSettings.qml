import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import Qt5Compat.GraphicalEffects

import ".." as Skin
import "../Theme"

ColumnLayout {
    id: root
    required property var settings

    DelegateChooser {
        id: item
        role: "type"
        DelegateChoice {
            roleValue: "enum"
            Skin.ComboBox {
                id: combobox
                required property var modelData
                delegate: ItemDelegate {
                    id: itemDlgt

                    required property int index
                    readonly property var color: combobox.model[index].color
                    readonly property var label: combobox.model[index].label

                    width: combobox.width
                    highlighted: combobox.highlightedIndex === index
                    text: label
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
                            font: combobox.font
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
                        color: combobox.model[combobox.currentIndex].color
                    }
                    Text {
                        Layout.fillWidth: true
                        text: combobox.model[combobox.currentIndex].label
                        font: combobox.font
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
                currentIndex: modelData.currentIndex

                Binding {
                    target: modelData
                    property: 'currentIndex'
                    value: combobox.currentIndex
                }
            }
        }
        DelegateChoice {
            roleValue: "color"
            Skin.FormButton {
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
                        modelData.value = selectedColor
                    }
                    selectedColor: modelData.value
                }

                onPressed: {
                    colorDialog.open()
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
                    modelData.value = selected == "on"
                }
            }
        }
        DelegateChoice {
            roleValue: "number"
            SpinBox {
                id: spinBox
                required property var modelData
                from: decimalToInt(modelData.min)
                value: {
                    decimalToInt(modelData.value)
                }
                to: decimalToInt(modelData.max)
                stepSize: modelData.step * decimalFactor
                editable: true

                property int decimals: modelData.precision ?? 0
                property real realValue: value / decimalFactor
                readonly property int decimalFactor: Math.pow(10, decimals)

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
                    modelData.value = Number(value / decimalFactor)
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

                component Indicator: Item {
                    id: indicator
                    required property string text
                    height: implicitHeight
                    implicitWidth: 24
                    implicitHeight: 24

                    Rectangle {
                        id: content
                        anchors.fill: parent
                        radius: 2
                        color: Theme.darkGray2
                        border.width: 0
                        Text {
                            text: indicator.text
                            font.pixelSize: spinBox.font.pixelSize
                            color: Theme.white
                            anchors.fill: parent
                            fontSizeMode: Text.Fit
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    InnerShadow {
                        id: bottomInnerEffect
                        anchors.fill: parent
                        radius: 4
                        samples: 16
                        spread: 0.3
                        horizontalOffset: -2
                        verticalOffset: -2
                        color: "#40000000"
                        source: content
                    }
                    InnerShadow {
                        id: topInnerEffect
                        anchors.fill: parent
                        radius: 4
                        samples: 16
                        spread: 0.3
                        horizontalOffset: 2
                        verticalOffset: 2
                        color: "#40000000"
                        source: bottomInnerEffect
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

                background: Item {
                    implicitWidth: 140
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
                    color: Theme.white
                }
                Skin.FormButton {
                    Layout.minimumWidth: 100
                    activeColor: modelData.value
                    text: "Select file"
                    FileDialog {
                        id: fileDialog
                        title: qsTr("Select a file")
                        nameFilters:[modelData.fileFilter]
                        onAccepted: {
                            modelData.value = fileDialog.selectedFile
                        }
                    }
                    onPressed: {
                        fileDialog.open()
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
                Layout.preferredWidth: 100
                required property var modelData
                columns: modelData.preferredOrientation == 0 ? 2 : 1
                rowSpacing: 10
                Text {
                    Layout.fillWidth: true
                    text: modelData.properties.label
                    color: Theme.white
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
                color: Theme.darkGray2
                implicitHeight: column.implicitHeight + 30
                Layout.fillWidth: true
                Layout.topMargin: 20
                ColumnLayout {
                    anchors.margins: 10
                    anchors.fill: parent
                    Layout.fillWidth: true
                    spacing: 4
                    id: column
                    Text {
                        text: modelData.label
                        color: Theme.white
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

    Repeater {
        model: root.settings
        delegate: chooser
    }
}
