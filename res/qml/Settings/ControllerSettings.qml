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

                Layout.fillWidth: true
                clip: true
                currentIndex: modelData.currentValueIndex
                font.pixelSize: 10
                implicitHeight: 32
                indicator.width: 0
                model: modelData.options
                spacing: 2

                contentItem: RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10

                    Rectangle {
                        Layout.minimumHeight: 15
                        Layout.minimumWidth: 15
                        color: combobox.model[combobox.currentIndex].color
                        visible: color.valid
                    }
                    Text {
                        Layout.fillWidth: true
                        color: Theme.deckTextColor
                        elide: Text.ElideRight
                        font: combobox.font
                        text: combobox.model[combobox.currentIndex].label
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                delegate: ItemDelegate {
                    id: itemDlgt

                    readonly property var color: combobox.model[index].color
                    required property int index
                    readonly property var label: combobox.model[index].label

                    highlighted: combobox.highlightedIndex === index
                    padding: 4
                    text: label
                    verticalPadding: 8
                    width: combobox.width

                    background: Rectangle {
                        color: itemDlgt.highlighted ? Theme.deckLineColor : "transparent"
                    }
                    contentItem: RowLayout {
                        Rectangle {
                            Layout.minimumHeight: 15
                            Layout.minimumWidth: 15
                            color: itemDlgt.color
                            visible: color.valid
                        }
                        Text {
                            Layout.fillWidth: true
                            color: Theme.deckTextColor
                            elide: Text.ElideRight
                            font: combobox.font
                            text: itemDlgt.text
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Binding {
                    property: 'currentValueIndex'
                    target: modelData
                    value: combobox.currentIndex
                }
            }
        }
        DelegateChoice {
            roleValue: "color"

            Skin.FormButton {
                required property var modelData

                Layout.fillWidth: true
                activeColor: modelData.currentValue
                text: "Change color"

                onPressed: {
                    colorDialog.open();
                }

                Rectangle {
                    id: indicator

                    anchors.left: parent.left
                    anchors.margins: (parent.height - 10) / 2
                    anchors.top: parent.top
                    color: modelData.currentValue
                    height: 10
                    width: 10
                }
                ColorDialog {
                    id: colorDialog

                    selectedColor: modelData.currentValue
                    title: "Please choose a color"

                    onAccepted: {
                        modelData.currentValue = selectedColor;
                    }
                }
            }
        }
        DelegateChoice {
            roleValue: "boolean"

            RatioChoice {
                required property var modelData

                options: ["off", "on"]
                selected: modelData.currentValue ? "on" : "off"

                onSelectedChanged: {
                    modelData.currentValue = selected == "on";
                }
            }
        }
        DelegateChoice {
            roleValue: "number"

            SpinBox {
                id: spinBox

                readonly property int decimalFactor: Math.pow(10, decimals)
                property int decimals: modelData.precision ?? 0
                required property var modelData
                property real realValue: value / decimalFactor

                function decimalToInt(decimal) {
                    return decimal * decimalFactor;
                }

                editable: true
                from: decimalToInt(modelData.min)
                padding: 0
                spacing: 2
                stepSize: modelData.step * decimalFactor
                textFromValue: function (value, locale) {
                    return Number(value / decimalFactor).toLocaleString(locale, 'f', spinBox.decimals);
                }
                to: decimalToInt(modelData.max)
                value: {
                    decimalToInt(modelData.currentValue);
                }
                valueFromText: function (text, locale) {
                    return Math.round(Number.fromLocaleString(locale, text) * decimalFactor);
                }

                background: Item {
                    implicitWidth: 140
                }
                contentItem: Item {
                    width: spinBox.textWidth + 2 * spinBox.spacing

                    Rectangle {
                        id: content

                        anchors.fill: parent
                        color: Theme.accentColor

                        Text {
                            id: textLabel

                            anchors.fill: parent
                            color: Theme.white
                            font: spinBox.font
                            horizontalAlignment: Text.AlignHCenter
                            text: spinBox.textFromValue(spinBox.value, spinBox.locale) ?? ""
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    InnerShadow {
                        id: bottomInnerEffect

                        anchors.fill: parent
                        color: "#0E2A54"
                        horizontalOffset: -1
                        radius: 8
                        samples: 32
                        source: content
                        spread: 0.4
                        verticalOffset: -1
                    }
                    InnerShadow {
                        id: topInnerEffect

                        anchors.fill: parent
                        color: "#0E2A54"
                        horizontalOffset: 1
                        radius: 8
                        samples: 32
                        source: bottomInnerEffect
                        spread: 0.4
                        verticalOffset: 1
                    }
                }
                down.indicator: Indicator {
                    text: "-"
                    x: spinBox.mirrored ? parent.width - width : 0
                }
                up.indicator: Indicator {
                    text: "+"
                    x: spinBox.mirrored ? 0 : parent.width - width
                }
                validator: DoubleValidator {
                    bottom: Math.min(spinBox.from, spinBox.to)
                    decimals: spinBox.decimals
                    notation: DoubleValidator.StandardNotation
                    top: Math.max(spinBox.from, spinBox.to)
                }

                onValueChanged: {
                    modelData.currentValue = Number(value / decimalFactor);
                }
            }
        }
        DelegateChoice {
            roleValue: "file"

            RowLayout {
                required property var modelData

                Layout.fillWidth: true

                Text {
                    id: label

                    Layout.fillWidth: true
                    color: Theme.white
                    elide: Text.ElideLeft
                    font.italic: !modelData.currentValue
                    text: modelData.currentValue ? modelData.currentValue : "No file selected"
                }
                Skin.FormButton {
                    Layout.minimumWidth: 100
                    activeColor: modelData.currentValue
                    text: "Select file"

                    onPressed: {
                        fileDialog.open();
                    }

                    FileDialog {
                        id: fileDialog

                        nameFilters: [modelData.fileFilter]
                        title: qsTr("Select a file")

                        onAccepted: {
                            modelData.currentValue = fileDialog.selectedFile;
                        }
                    }
                }
            }
        }
        DelegateChoice {
            Text {
                required property var modelData

                Layout.fillWidth: true
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
                required property var modelData

                Layout.preferredWidth: 100
                columns: modelData.preferredOrientation == 0 ? 2 : 1
                rowSpacing: 10

                Text {
                    Layout.fillWidth: true
                    color: Theme.white
                    text: modelData.properties.label
                }
                Repeater {
                    Layout.preferredWidth: 100
                    delegate: item
                    model: [modelData.properties]
                }
            }
        }
        DelegateChoice {
            roleValue: "container"

            GridLayout {
                required property var modelData

                Layout.bottomMargin: 10
                Layout.preferredWidth: 100
                Layout.topMargin: 10
                columnSpacing: 60
                columns: modelData.disposition == 0 ? modelData.children.length : 1

                Repeater {
                    delegate: chooser
                    model: modelData.children
                }
            }
        }
        DelegateChoice {
            roleValue: "group"

            Rectangle {
                required property var modelData

                Layout.fillWidth: true
                Layout.topMargin: 20
                color: Theme.darkGray2
                implicitHeight: column.implicitHeight + 30

                ColumnLayout {
                    id: column

                    Layout.fillWidth: true
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    Text {
                        color: Theme.white
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        text: modelData.label
                    }
                    Repeater {
                        delegate: chooser
                        model: modelData.children
                    }
                }
            }
        }
    }
    Repeater {
        delegate: chooser
        model: root.settings
    }

    component Indicator: Item {
        id: indicator

        required property string text

        height: implicitHeight
        implicitHeight: 24
        implicitWidth: 24

        Rectangle {
            id: content

            anchors.fill: parent
            border.width: 0
            color: Theme.darkGray2
            radius: 2

            Text {
                anchors.fill: parent
                color: Theme.white
                font.pixelSize: spinBox.font.pixelSize
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                text: indicator.text
                verticalAlignment: Text.AlignVCenter
            }
        }
        InnerShadow {
            id: bottomInnerEffect

            anchors.fill: parent
            color: "#40000000"
            horizontalOffset: -2
            radius: 4
            samples: 16
            source: content
            spread: 0.3
            verticalOffset: -2
        }
        InnerShadow {
            id: topInnerEffect

            anchors.fill: parent
            color: "#40000000"
            horizontalOffset: 2
            radius: 4
            samples: 16
            source: bottomInnerEffect
            spread: 0.3
            verticalOffset: 2
        }
    }
}
