import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "../Theme"
import ".." as Skin

Item {
    id: root

    readonly property real cellSize: {
        Math.max.apply(null, options.map(option => fontMetrics.advanceWidth(option))) + root.spacing * 2;
    }
    property alias content: contentList
    property color inactiveColor: Theme.darkGray2
    property real maxWidth: 0
    property alias metric: fontMetrics
    property bool normalizedWidth: true
    required property list<string> options
    property string selected: options.length ? options[0] : null
    property real spacing: 9
    property list<var> tooltips: []

    implicitHeight: (contentList.visible ? contentList.height : contentSpin.height)
    width: {
        if (root.maxWidth > root.cellSize * root.options.length) {
            contentSpin.width;
        } else if (root.normalizedWidth) {
            root.cellSize * root.options.length + root.spacing;
        } else {
            options.reduce((acc, option) => acc + fontMetrics.advanceWidth(option) + root.spacing * 2, 0) + root.spacing;
        }
    }

    onTooltipsChanged: {
        popup.close();
    }

    FontMetrics {
        id: fontMetrics

        font.capitalization: Font.AllUppercase
        font.pixelSize: 14
    }
    Rectangle {
        id: contentList

        anchors.centerIn: parent
        color: root.inactiveColor
        height: 24
        radius: height / 2
        visible: root.maxWidth == 0 || root.maxWidth > root.cellSize * root.options.length
        width: {
            if (root.normalizedWidth) {
                root.cellSize * root.options.length + root.spacing;
            } else {
                options.reduce((acc, option) => acc + fontMetrics.advanceWidth(option) + root.spacing * 2, 0) + root.spacing;
            }
        }

        RowLayout {
            anchors.centerIn: parent

            Repeater {
                model: options

                Item {
                    required property int index
                    required property var modelData

                    height: contentList.height
                    implicitWidth: root.normalizedWidth ? root.cellSize : fontMetrics.advanceWidth(modelData) + root.spacing * 2

                    Rectangle {
                        id: contentOption

                        anchors.fill: parent
                        color: root.selected == modelData ? Theme.accentColor : 'transparent'
                        radius: height / 2

                        Text {
                            anchors.fill: parent
                            color: Theme.white
                            font: fontMetrics.font
                            horizontalAlignment: Text.AlignHCenter
                            text: modelData
                            verticalAlignment: Text.AlignVCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: !!root.tooltips[index]

                            onEntered: {
                                if (!root.tooltips[index])
                                    return;
                                popup.tooltip = root.tooltips[index] || "";
                                popup.x = Qt.binding(function () {
                                    return contentOption.mapToItem(root, 0, 0).x + contentOption.width / 2 - popup.width / 2;
                                });
                                popup.open();
                            }
                            onExited: {
                                popup.close();
                            }
                            onPressed: {
                                root.selected = modelData;
                            }
                        }
                    }
                    InnerShadow {
                        id: bottomOptionInnerEffect

                        anchors.fill: parent
                        color: "#0E2A54"
                        horizontalOffset: -1
                        radius: 8
                        samples: 32
                        source: contentOption
                        spread: 0.4
                        verticalOffset: -1
                        visible: root.selected == modelData
                    }
                    InnerShadow {
                        id: topOptionInnerEffect

                        anchors.fill: parent
                        color: "#0E2A54"
                        horizontalOffset: 1
                        radius: 8
                        samples: 32
                        source: bottomOptionInnerEffect
                        spread: 0.4
                        verticalOffset: 1
                        visible: root.selected == modelData
                    }
                }
            }
        }
    }
    SpinBox {
        id: contentSpin

        property real textWidth: fontMetrics.advanceWidth(root.options.reduce((accumulator, currentValue) => accumulator.length > currentValue.length ? accumulator : currentValue, ""))

        anchors.centerIn: parent
        font: fontMetrics.font
        from: 0
        padding: 0
        spacing: root.spacing
        textFromValue: function (value) {
            return root.options[value];
        }
        to: root.options.length - 1
        value: root.options.indexOf(root.selected)
        valueFromText: function (text) {
            for (var i = 0; i < root.options.length; ++i) {
                if (root.options[i].toLowerCase().indexOf(text.toLowerCase()) === 0)
                    return i;
            }
            return contentSpin.value;
        }
        visible: !contentList.visible

        background: Rectangle {
            color: root.inactiveColor
            implicitWidth: contentSpin.textWidth + 2 * contentSpin.spacing + 48
            radius: parent.height / 2
        }
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
                    color: Theme.white
                    font: contentSpin.font
                    horizontalAlignment: Text.AlignHCenter
                    text: contentSpin.textFromValue(contentSpin.value, contentSpin.locale) ?? ""
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
            text: "<"
            x: contentSpin.mirrored ? parent.width - width : 0
        }
        up.indicator: Indicator {
            text: ">"
            x: contentSpin.mirrored ? 0 : parent.width - width
        }

        onValueChanged: {
            root.selected = contentSpin.textFromValue(value) ?? "";
            popup.tooltip = root.tooltips[contentSpin.value] ?? "";
            popup.x = contentSpin.width / 2 - popup.width / 2;
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                if (!root.tooltips[contentSpin.value])
                    return;
                popup.x = contentSpin.width / 2 - popup.width / 2;
                popup.open();
            }
            onExited: {
                popup.close();
            }
            onPressed: {
                mouse.accepted = false;
            }
        }
    }
    DropShadow {
        id: dropRatio

        anchors.fill: contentList.visible ? contentList : contentSpin
        anchors.margins: 0
        color: "#80000000"
        horizontalOffset: 0
        radius: 4.0
        source: contentList.visible ? contentList : contentSpin
        verticalOffset: 0
    }
    Popup {
        id: popup

        property string tooltip: ""

        closePolicy: Popup.NoAutoClose
        height: tooltip.implicitHeight + 15
        padding: 0
        width: Math.max(tooltip.implicitWidth * 1.5, 50)
        x: 0
        y: root.height

        background: Item {
        }
        contentItem: Item {
            Item {
                id: contentPopup

                anchors.fill: parent

                Shape {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    antialiasing: true
                    height: width
                    layer.enabled: true
                    layer.samples: 4
                    width: 20

                    ShapePath {
                        capStyle: ShapePath.RoundCap
                        fillColor: Theme.embeddedBackgroundColor
                        fillRule: ShapePath.WindingFill
                        startX: 10
                        startY: 0
                        strokeColor: Theme.deckBackgroundColor
                        strokeWidth: 2

                        PathLine {
                            x: 20
                            y: 10
                        }
                        PathLine {
                            x: 0
                            y: 10
                        }
                        PathLine {
                            x: 10
                            y: 0
                        }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.fill: parent
                    anchors.topMargin: 10

                    Text {
                        id: tooltip

                        anchors.centerIn: parent
                        color: Theme.white
                        text: popup.tooltip
                    }
                }
            }
            DropShadow {
                anchors.fill: parent
                color: "#000000"
                horizontalOffset: 0
                radius: 8.0
                source: contentPopup
                verticalOffset: 0
            }
        }
    }

    component Indicator: Rectangle {
        required property string text

        border.width: 0
        color: root.inactiveColor
        height: implicitHeight
        implicitHeight: 24
        implicitWidth: 24
        radius: parent.height / 2

        Text {
            anchors.fill: parent
            color: Theme.white
            font.pixelSize: contentSpin.font.pixelSize
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            text: parent.text
            verticalAlignment: Text.AlignVCenter
        }
    }
}
