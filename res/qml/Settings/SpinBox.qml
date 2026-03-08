import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "../Theme"

SpinBox {
    id: root

    readonly property int decimalFactor: Math.pow(10, decimals)
    property int decimals: root.precision ?? 0
    property double max: 1
    property double min: 0
    property int precision: 2
    property real realValue: 0
    property double step: 1 / decimalFactor
    property string suffix: ""

    function decimalToInt(decimal) {
        return decimal * decimalFactor;
    }

    editable: true
    from: decimalToInt(root.min)
    padding: 0
    spacing: 2
    stepSize: root.step * decimalFactor
    textFromValue: function (value, locale) {
        return Number(value / decimalFactor).toLocaleString(locale, 'f', root.precision);
    }
    to: decimalToInt(root.max)
    value: decimalToInt(realValue)
    valueFromText: function (text, locale) {
        return Math.round(Number.fromLocaleString(locale, text) * decimalFactor);
    }

    background: Item {
        implicitWidth: 140
    }
    contentItem: Item {
        width: root.textWidth + 2 * root.spacing

        Rectangle {
            id: content

            anchors.fill: parent
            color: Theme.accentColor

            Text {
                id: textLabel

                anchors.fill: parent
                color: Theme.white
                font: root.font
                horizontalAlignment: Text.AlignHCenter
                text: `${root.textFromValue(root.value, root.locale)}${root.suffix}` ?? ""
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
        x: root.mirrored ? parent.width - width : 0
    }
    up.indicator: Indicator {
        text: "+"
        x: root.mirrored ? 0 : parent.width - width
    }
    validator: DoubleValidator {
        bottom: Math.min(root.from, root.to)
        decimals: root.precision
        notation: DoubleValidator.StandardNotation
        top: Math.max(root.from, root.to)
    }

    onValueChanged: {
        root.value = value;
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
                font.pixelSize: root.font.pixelSize
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
