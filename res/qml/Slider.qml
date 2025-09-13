import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import "Theme"

RowLayout {
    id: root
    property list<var> markers: ["", ""]

    property alias suffix: textInputSection.suffix
    property alias slider: control
    property alias value: control.value

    height: 30

    Slider {
        id: control

        Layout.fillWidth: true

        background: Item {
            x: control.leftPadding + 7
            implicitWidth: 200
            implicitHeight: 4
            width: control.availableWidth - 7
            height: control.availableHeight
            Rectangle {
                width: parent.width
                height: 4
                radius: 2
                color: "#181818"
            }
            Repeater {
                id: delegate
                model: markers
                anchors.fill: parent
                anchors.leftMargin: 7
                Item {
                    required property int index
                    required property var modelData
                    x: parent.width * (index / (delegate.model.length - 1))
                    y: -4
                    height: control.availableHeight

                    Rectangle {
                        id: mark
                        visible: modelData != null
                        anchors {
                            top: parent.top
                        }
                        width: 1
                        height: 11
                        color: Qt.alpha(Theme.white, 0.25)
                    }
                    Text {
                        id: label
                        visible: modelData != null
                        anchors {
                            top: mark.bottom
                            topMargin: 4
                            horizontalCenter: mark.left
                        }
                        color: Qt.alpha(Theme.white, 0.25)
                        font.pixelSize: 9
                        text: modelData ?? ""
                    }
                }
            }
        }
        handle: Item {
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: -5
            width: 14
            height: 14
            Rectangle {
                id: handle
                anchors.fill: parent
                radius: 7
                color: Theme.accentColor
            }
            InnerShadow {
                id: handleEffect1
                anchors.fill: parent
                samples: 16
                horizontalOffset: 0
                verticalOffset: 0
                radius: 16.0
                color: "#0E2A54"
                source: handle
            }
            DropShadow {
                id: handleEffect2
                anchors.fill: parent
                source: handleEffect1
                horizontalOffset: 0
                verticalOffset: 0
                radius: 12.0
                color: Qt.alpha(Theme.darkGray, 0.25)
            }
        }
    }
    FocusScope {
        id: textInputSection
        Layout.leftMargin: 17
        Layout.minimumWidth: fontMetrics.advanceWidth + 8
        Layout.preferredHeight: 30
        Layout.margins: 4

        property string suffix: ""
        visible: suffix.length > 0

        Rectangle {
            id: backgroundInput
            radius: 4
            color: Theme.darkGray2
            anchors.fill: parent
            anchors.margins: 4
        }
        DropShadow {
            id: dropSetting
            anchors.fill: parent
            horizontalOffset: 0
            verticalOffset: 0
            radius: 4.0
            color: Theme.darkGray
            source: backgroundInput
        }
        InnerShadow {
            id: effect2
            anchors.fill: parent
            source: dropSetting
            spread: 0.2
            radius: 12
            samples: 24
            horizontalOffset: 0
            verticalOffset: 0
            color: "#353535"
        }
        Item {
            anchors.fill: parent
            anchors.margins: 4
            TextInput {
                anchors.left: parent.left
                anchors.right: inputField.left
                anchors.margins: 3
                focus: true
                color: Qt.alpha(acceptableInput ? Theme.white : Theme.warningColor, root.enabled ? 1 : 0.5)
                onAccepted: {
                    control.value = parseInt(text)
                }
                text: Math.round(control.value)
                horizontalAlignment: TextInput.AlignRight
                validator: IntValidator {bottom: control.from; top: control.to}
            }
            Text {
                id: inputField
                anchors.right: parent.right
                anchors.margins: textInputSection.suffix.length > 0 ? 10 : 0
                text: textInputSection.suffix
                color: Qt.alpha(Theme.white, root.enabled ? 1 : 0.5)
                TextMetrics  {
                    id: fontMetrics
                    font.family: inputField.font.family
                    text: `${control.to} ${parent.text}`
                }
            }
        }
    }
}
