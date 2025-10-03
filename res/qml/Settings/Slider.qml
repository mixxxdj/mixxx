import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls
import "../Theme"

Item {
    id: root
    property list<double> markers: []

    property alias suffix: textInputSection.suffix
    property alias slider: control

    property double value: 0
    property double wheelStep: 1
    property double min: markers.length ? markers[0] : 0
    property double max: markers.length ? markers[markers.length-1] : 1
    readonly property int decimalFactor: Math.pow(10, decimals)
    property int decimals: 0

    Component.onCompleted: {
        setValue(root.value)
    }

    function setValue(value) {
        root.value = value
        control.value = valueToRatio(root.value)
    }

    height: 30
    width: 100

    function textFromValue(value, locale) {
        return Number(Math.round(value * decimalFactor) / decimalFactor).toLocaleString(locale, 'f', root.decimals)
    }

    function valueFromText(text, locale) {
        return Math.round(Number.fromLocaleString(locale, text) * decimalFactor) / decimalFactor
    }

    function ratioToValue(ratio) {
        if (!root.markers?.length) {
            return root.min + (root.max - root.min) * ratio;
        }
        let valueCount = root.markers.length ? root.markers[root.markers.length-1] === root.max ? root.markers.length - 1 : root.markers.length : 0
        let value = valueCount * ratio
        let lower = Math.min(Math.floor(value), root.markers.length - 2)
        let upper = Math.min(Math.ceil(value), root.markers.length - 1)
        let delta = root.markers[upper] - root.markers[lower]
        return root.markers[lower] + (value - lower) * delta
    }

    function valueToRatio(value) {
        if (!root.markers?.length) {
            return (value - root.min) / (root.max - root.min);
        }
        let markers = [...root.markers];
        if (markers.length && markers[markers.length-1] !== root.max) {
            markers.push(root.max);
        }
        let valueCount = markers.length - 1;
        let stepIdx = 0;
        for (let step of markers) {
            if (step >= value) break;
            stepIdx++;
        }
        stepIdx = Math.min(stepIdx - 1, valueCount - 1)
        let delta = markers[stepIdx+1] - markers[stepIdx]
        return (stepIdx + (value - markers[stepIdx]) / delta) / valueCount
    }

    MouseArea {
        anchors.fill: root

        onWheel: (mouse) => {
            if (mouse.angleDelta.y < 0) {
                setValue(Math.max(root.min, root.value - root.wheelStep));
            } else if (mouse.angleDelta.y > 0) {
                setValue(Math.min(root.max, root.value + root.wheelStep));
            }
        }
    }

    Slider {
        id: control

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.rightMargin: 10
        anchors.right: textInputSection.left

        from: 0
        to: 1

        onValueChanged: {
            root.setValue(ratioToValue(control.value))
        }

        background: Item {
            x: control.leftPadding + 7
            implicitWidth: 200
            implicitHeight: 4
            width: control.availableWidth - 14
            height: control.availableHeight
            Rectangle {
                width: parent.width
                height: 4
                radius: 2
                color: "#181818"
            }
            Repeater {
                id: delegate
                model: !markers?.length ? 2 : markers[markers.length-1] === root.max ? markers.length : markers.length + 1
                anchors.fill: parent
                anchors.leftMargin: 7
                Item {
                    required property int index
                    readonly property var modelData: !markers?.length ? "" : index < markers.length ? `${markers[index]}${textInputSection.suffix}` : null
                    x: parent.width * (index / (delegate.count - 1))
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

        anchors.leftMargin: 17
        anchors.margins: 4
        anchors.right: root.right

        width: fontMetrics.advanceWidth + 8
        height: 30

        property string suffix: ""

        Rectangle {
            id: backgroundInput
            radius: 4
            color: Theme.darkGray2
            anchors.fill: parent
            anchors.margins: 4
        }
        DropShadow {
            id: dropSetting
            anchors.fill: backgroundInput
            horizontalOffset: 0
            verticalOffset: 0
            radius: 4.0
            color: Theme.darkGray
            source: backgroundInput
        }
        InnerShadow {
            id: effect2
            anchors.fill: backgroundInput
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
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 3
                focus: true
                color: Qt.alpha(acceptableInput ? Theme.white : Theme.warningColor, root.enabled ? 1 : 0.5)
                onAccepted: {
                    root.setValue(valueFromText(text, control.locale))
                }
                text: textFromValue(root.value, control.locale)
                horizontalAlignment: TextInput.AlignRight
                validator: DoubleValidator {
                    bottom: Math.min(root.min, root.max)
                    top: Math.max(root.min, root.max)
                    decimals: root.decimals
                    notation: DoubleValidator.StandardNotation
                }
            }
            Text {
                id: inputField
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
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
