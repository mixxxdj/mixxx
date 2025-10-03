import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

import "../Theme"

SpinBox {
    id: root

    property double min: 0
    property double max: 1
    property double step: 1 / decimalFactor
    property int precision: 2
    property string suffix: ""

    from: decimalToInt(root.min)
    to: decimalToInt(root.max)
    stepSize: root.step * decimalFactor
    editable: true

    property int decimals: root.precision ?? 0
    property real realValue: 0
    value: decimalToInt(realValue)
    readonly property int decimalFactor: Math.pow(10, decimals)

    function decimalToInt(decimal) {
        return decimal * decimalFactor
    }

    validator: DoubleValidator {
        bottom: Math.min(root.from, root.to)
        top: Math.max(root.from, root.to)
        decimals: root.precision
        notation: DoubleValidator.StandardNotation
    }

    textFromValue: function(value, locale) {
        return Number(value / decimalFactor).toLocaleString(locale, 'f', root.precision)
    }

    valueFromText: function(text, locale) {
        return Math.round(Number.fromLocaleString(locale, text) * decimalFactor)
    }

    onValueChanged: {
        root.value = value
    }
    padding: 0
    spacing: 2
    contentItem: Item {
        width: root.textWidth + 2 * root.spacing
        Rectangle {
            id: content
            anchors.fill: parent
            color: Theme.accentColor
            Text {
                id: textLabel
                anchors.fill: parent
                text: `${root.textFromValue(root.value, root.locale)}${root.suffix}` ?? ""
                color: Theme.white
                font: root.font
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
                font.pixelSize: root.font.pixelSize
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
        x: root.mirrored ? 0 : parent.width - width
        text: "+"
    }

    down.indicator: Indicator {
        x: root.mirrored ? parent.width - width : 0
        text: "-"
    }

    background: Item {
        implicitWidth: 140
    }
}
