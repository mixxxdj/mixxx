import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "../Theme"
import ".." as Skin

Item {
    id: root
    required property list<string> options
    property list<var> tooltips: []
    property string selected: options.length ? options[0] : null
    property real spacing: 9
    property real maxWidth: 0
    property bool normalizedWidth: true

    onTooltipsChanged: {
        popup.close()
    }

    FontMetrics {
        id: fontMetrics
        font.pixelSize: 14
        font.capitalization: Font.AllUppercase
    }

    implicitHeight: (contentList.visible ? contentList.height : contentSpin.height) + dropRatio.radius * 2
    implicitWidth: (contentList.visible ? contentList.width : contentSpin.width) + dropRatio.radius * 2
    readonly property real cellSize: {
        Math.max.apply(null, options.map((option) => fontMetrics.advanceWidth(option))) + root.spacing*2
    }

    Rectangle {
        id: contentList
        visible: root.maxWidth == 0 || root.maxWidth > root.cellSize * root.options.length
        anchors.centerIn: parent
        height: 24
        width: {
            if (root.normalizedWidth) {
                root.cellSize * root.options.length + root.spacing
            } else {
                options.reduce((acc, option) => acc + fontMetrics.advanceWidth(option) + root.spacing*2, 0) + root.spacing
            }
        }
        color: '#2B2B2B'
        radius: height / 2
        RowLayout {
            anchors.fill: parent
            Repeater {
                model: options
                Item {
                    required property int index
                    required property var modelData
                    width: root.normalizedWidth ? root.cellSize : fontMetrics.advanceWidth(modelData) + root.spacing*2
                    height: contentList.height
                    Rectangle {
                        anchors.fill: parent
                        color: root.selected == modelData ? Theme.accentColor : 'transparent'
                        radius: height / 2
                        id: contentOption
                        Text {
                            text: modelData
                            color: Theme.white
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font: fontMetrics.font
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: !!root.tooltips[index]
                            onPressed: {
                                root.selected = modelData
                            }

                            onEntered: {
                                if (!root.tooltips[index]) return;
                                popup.tooltip = root.tooltips[index] || ""
                                popup.x = Qt.binding(function() { return contentOption.mapToItem(root, 0, 0).x + contentOption.width / 2 - popup.width / 2; })
                                popup.open()
                            }
                            onExited: {
                                popup.close()
                            }
                        }
                    }
                    InnerShadow {
                        visible: root.selected == modelData
                        id: bottomOptionInnerEffect
                        anchors.fill: parent
                        radius: 8
                        samples: 32
                        spread: 0.4
                        horizontalOffset: -1
                        verticalOffset: -1
                        color: "#0E2A54"
                        source: contentOption
                    }
                    InnerShadow {
                        visible: root.selected == modelData
                        id: topOptionInnerEffect
                        anchors.fill: parent
                        radius: 8
                        samples: 32
                        spread: 0.4
                        horizontalOffset: 1
                        verticalOffset: 1
                        color: "#0E2A54"
                        source: bottomOptionInnerEffect
                    }
                }
            }
        }
    }
    SpinBox {
        id: contentSpin
        visible: !contentList.visible
        anchors.centerIn: parent
        from: 0
        padding: 0
        spacing: root.spacing
        to: root.options.length - 1
        font: fontMetrics.font
        value: root.options.indexOf(root.selected)

        property real textWidth: fontMetrics.advanceWidth(root.options.reduce((accumulator, currentValue) => accumulator.length > currentValue.length ? accumulator : currentValue, ""))

        contentItem: Item {
            width: contentSpin.textWidth + 2 * contentSpin.spacing
            Rectangle {
                id: content
                anchors.fill: parent
                color: Theme.accentColor
                radius: height / 2
                Text {
                    id: textLabel
                    anchors.fill: parent
                    text: contentSpin.textFromValue(contentSpin.value, contentSpin.locale) ?? ""
                    color: Theme.white
                    font: contentSpin.font
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
            radius: parent.height / 2
            color: '#2B2B2B'
            border.width: 0

            Text {
                text: parent.text
                font.pixelSize: contentSpin.font.pixelSize
                color: Theme.white
                anchors.fill: parent
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        up.indicator: Indicator {
            x: contentSpin.mirrored ? 0 : parent.width - width
            text: ">"
        }

        down.indicator: Indicator {
            x: contentSpin.mirrored ? parent.width - width : 0
            text: "<"
        }

        background: Rectangle {
            implicitWidth: contentSpin.textWidth + 2 * contentSpin.spacing + 48
            radius: parent.height / 2
            color: '#2B2B2B'
        }

        textFromValue: function(value) {
            return root.options[value];
        }

        valueFromText: function(text) {
            for (var i = 0; i < root.options.length; ++i) {
                if (root.options[i].toLowerCase().indexOf(text.toLowerCase()) === 0)
                    return i
            }
            return contentSpin.value
        }

        onValueChanged: {
            root.selected = contentSpin.textFromValue(value) ?? ""
            popup.tooltip = root.tooltips[contentSpin.value] ?? ""
            popup.x = contentSpin.width / 2 - popup.width / 2
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                if (!root.tooltips[contentSpin.value]) return;
                popup.x = contentSpin.width / 2 - popup.width / 2
                popup.open()
            }
            onExited: {
                popup.close()
            }
            onPressed: {
                mouse.accepted = false
            }
        }
    }
    DropShadow {
        id: dropRatio
        anchors.margins: dropRatio.radius
        anchors.fill: root
        horizontalOffset: 0
        verticalOffset: 0
        radius: 4.0
        color: "#80000000"
        source: contentList.visible ? contentList : contentSpin
    }
    Popup {
        id: popup
        y: root.height
        x: 0
        width: Math.max(tooltip.implicitWidth* 1.5, 50)
        height: tooltip.implicitHeight + 15
        closePolicy: Popup.NoAutoClose

        property string tooltip: ""

        padding: 0

        contentItem: Item {
            Item {
                id: contentPopup
                anchors.fill: parent
                Shape {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 20
                    height: width
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: Theme.embeddedBackgroundColor
                        strokeColor: Theme.deckBackgroundColor
                        strokeWidth: 2
                        startX: 10; startY: 0
                        fillRule: ShapePath.WindingFill
                        capStyle: ShapePath.RoundCap
                        PathLine { x: 20; y: 10 }
                        PathLine { x: 0; y: 10 }
                        PathLine { x: 10; y: 0 }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.topMargin: 10
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        id: tooltip
                        color: Theme.white
                        text: popup.tooltip
                    }
                }
            }
            DropShadow {
                anchors.fill: parent
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8.0
                color: "#000000"
                source: contentPopup
            }
        }

        background: Item {}
    }
}
