import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls
import "../Theme"

Item {
    id: root

    readonly property int decimalFactor: Math.pow(10, decimals)
    property int decimals: 0
    property list<double> markers: []
    property double max: markers.length ? markers[markers.length - 1] : 1
    property double min: markers.length ? markers[0] : 0
    property alias slider: control
    property alias suffix: textInputSection.suffix
    property double value: 0
    property double wheelStep: 1

    function ratioToValue(ratio) {
        if (!root.markers?.length) {
            return root.min + (root.max - root.min) * ratio;
        }
        let valueCount = root.markers.length ? root.markers[root.markers.length - 1] === root.max ? root.markers.length - 1 : root.markers.length : 0;
        let value = valueCount * ratio;
        let lower = Math.min(Math.floor(value), root.markers.length - 2);
        let upper = Math.min(Math.ceil(value), root.markers.length - 1);
        let delta = root.markers[upper] - root.markers[lower];
        return root.markers[lower] + (value - lower) * delta;
    }
    function setValue(value) {
        root.value = value;
        control.value = valueToRatio(root.value);
    }
    function textFromValue(value, locale) {
        return Number(Math.round(value * decimalFactor) / decimalFactor).toLocaleString(locale, 'f', root.decimals);
    }
    function valueFromText(text, locale) {
        return Math.round(Number.fromLocaleString(locale, text) * decimalFactor) / decimalFactor;
    }
    function valueToRatio(value) {
        if (!root.markers?.length) {
            return (value - root.min) / (root.max - root.min);
        }
        let markers = [...root.markers];
        if (markers.length && markers[markers.length - 1] !== root.max) {
            markers.push(root.max);
        }
        let valueCount = markers.length - 1;
        let stepIdx = 0;
        for (let step of markers) {
            if (step >= value)
                break;
            stepIdx++;
        }
        stepIdx = Math.min(stepIdx - 1, valueCount - 1);
        let delta = markers[stepIdx + 1] - markers[stepIdx];
        return (stepIdx + (value - markers[stepIdx]) / delta) / valueCount;
    }

    height: 30
    width: 100

    Component.onCompleted: {
        setValue(root.value);
    }

    MouseArea {
        anchors.fill: root

        onWheel: mouse => {
            if (mouse.angleDelta.y < 0) {
                setValue(Math.max(root.min, root.value - root.wheelStep));
            } else if (mouse.angleDelta.y > 0) {
                setValue(Math.min(root.max, root.value + root.wheelStep));
            }
        }
    }
    Slider {
        id: control

        anchors.left: root.left
        anchors.right: textInputSection.left
        anchors.rightMargin: 10
        anchors.verticalCenter: root.verticalCenter
        from: 0
        to: 1

        background: Item {
            height: control.availableHeight
            implicitHeight: 4
            implicitWidth: 200
            width: control.availableWidth - 14
            x: control.leftPadding + 7

            Rectangle {
                color: "#181818"
                height: 4
                radius: 2
                width: parent.width
            }
            Repeater {
                id: delegate

                anchors.fill: parent
                anchors.leftMargin: 7
                model: !markers?.length ? 2 : markers[markers.length - 1] === root.max ? markers.length : markers.length + 1

                Item {
                    required property int index
                    readonly property var modelData: !markers?.length ? "" : index < markers.length ? `${markers[index]}${textInputSection.suffix}` : null

                    height: control.availableHeight
                    x: parent.width * (index / (delegate.count - 1))
                    y: -4

                    Rectangle {
                        id: mark

                        color: Qt.alpha(Theme.white, 0.25)
                        height: 11
                        visible: modelData != null
                        width: 1

                        anchors {
                            top: parent.top
                        }
                    }
                    Text {
                        id: label

                        color: Qt.alpha(Theme.white, 0.25)
                        font.pixelSize: 9
                        text: modelData ?? ""
                        visible: modelData != null

                        anchors {
                            horizontalCenter: mark.left
                            top: mark.bottom
                            topMargin: 4
                        }
                    }
                }
            }
        }
        handle: Item {
            height: 14
            width: 14
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: -5

            Rectangle {
                id: handle

                anchors.fill: parent
                color: Theme.accentColor
                radius: 7
            }
            InnerShadow {
                id: handleEffect1

                anchors.fill: parent
                color: "#0E2A54"
                horizontalOffset: 0
                radius: 16.0
                samples: 16
                source: handle
                verticalOffset: 0
            }
            DropShadow {
                id: handleEffect2

                anchors.fill: parent
                color: Qt.alpha(Theme.darkGray, 0.25)
                horizontalOffset: 0
                radius: 12.0
                source: handleEffect1
                verticalOffset: 0
            }
        }

        onValueChanged: {
            root.setValue(ratioToValue(control.value));
        }
    }
    FocusScope {
        id: textInputSection

        property string suffix: ""

        anchors.leftMargin: 17
        anchors.margins: 4
        anchors.right: root.right
        height: 30
        width: fontMetrics.advanceWidth + 8

        Rectangle {
            id: backgroundInput

            anchors.fill: parent
            anchors.margins: 4
            color: Theme.darkGray2
            radius: 4
        }
        DropShadow {
            id: dropSetting

            anchors.fill: backgroundInput
            color: Theme.darkGray
            horizontalOffset: 0
            radius: 4.0
            source: backgroundInput
            verticalOffset: 0
        }
        InnerShadow {
            id: effect2

            anchors.fill: backgroundInput
            color: "#353535"
            horizontalOffset: 0
            radius: 12
            samples: 24
            source: dropSetting
            spread: 0.2
            verticalOffset: 0
        }
        Item {
            anchors.fill: parent
            anchors.margins: 4

            TextInput {
                anchors.left: parent.left
                anchors.margins: 3
                anchors.right: inputField.left
                anchors.verticalCenter: parent.verticalCenter
                color: Qt.alpha(acceptableInput ? Theme.white : Theme.warningColor, root.enabled ? 1 : 0.5)
                focus: true
                horizontalAlignment: TextInput.AlignRight
                text: textFromValue(root.value, control.locale)

                validator: DoubleValidator {
                    bottom: Math.min(root.min, root.max)
                    decimals: root.decimals
                    notation: DoubleValidator.StandardNotation
                    top: Math.max(root.min, root.max)
                }

                onAccepted: {
                    root.setValue(valueFromText(text, control.locale));
                }
            }
            Text {
                id: inputField

                anchors.margins: textInputSection.suffix.length > 0 ? 10 : 0
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: Qt.alpha(Theme.white, root.enabled ? 1 : 0.5)
                text: textInputSection.suffix

                TextMetrics {
                    id: fontMetrics

                    font.family: inputField.font.family
                    text: `${control.to} ${parent.text}`
                }
            }
        }
    }
}
