import QtQuick
import "../../../qml" as Skin
import "../LateNightTheme"

Skin.ControlKnob {
    id: root

    readonly property int arcRenderScale: 8
    property url backgroundSource: LateNightTheme.assetRegularKnobBackground
    property bool displayArc: false
    property color displayArcColor: "transparent"
    property real displayArcOffsetY: 1.998
    property real displayArcRadius: 12.5
    property int displayArcStart: 1 // Knob.ArcStart.Center
    property real displayArcWidth: 2
    property string indicatorColor: "orange"
    property string indicatorKind: "regular"

    angle: LateNightTheme.isClassic ? 135 : 130
    arc: false
    arcStart: displayArcStart
    color: displayArcColor
    implicitHeight: backgroundImage.implicitHeight
    implicitWidth: backgroundImage.implicitWidth
    knobCenterOffsetY: displayArcOffsetY
    showDefaultBackground: false
    showDefaultForeground: false

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: root.backgroundSource
    }
    foreground: Item {
        anchors.fill: parent

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: LateNightTheme.mixerKnobIndicator(root.indicatorKind, root.indicatorColor)
        }
    }

    Canvas {
        id: arcCanvas

        readonly property color renderedColor: root.displayArcColor
        readonly property real renderedOffsetY: root.displayArcOffsetY
        readonly property real renderedRadius: root.displayArcRadius
        readonly property int renderedStart: root.displayArcStart
        readonly property real renderedValue: root.value
        readonly property real renderedWidth: root.displayArcWidth

        antialiasing: true
        height: root.height * root.arcRenderScale
        layer.enabled: true
        layer.mipmap: true
        renderStrategy: Canvas.Immediate
        scale: 1 / root.arcRenderScale
        transformOrigin: Item.TopLeft
        visible: root.displayArc
        width: root.width * root.arcRenderScale
        z: 1

        Component.onCompleted: requestPaint()
        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            if (!root.displayArc) {
                return;
            }

            const renderScale = root.arcRenderScale;
            const startAngle = root.angleFrom(root.arcStartValue - root.valueCenter) - 90;
            const sweepAngle = root.angleFrom(root.value - root.arcStartValue);
            const startRadians = startAngle * Math.PI / 180;
            const endRadians = (startAngle + sweepAngle) * Math.PI / 180;

            ctx.beginPath();
            ctx.strokeStyle = root.displayArcColor;
            ctx.lineWidth = root.displayArcWidth * renderScale;
            ctx.lineCap = "round";
            ctx.arc((root.width / 2) * renderScale, (root.height / 2 + root.displayArcOffsetY) * renderScale, root.displayArcRadius * renderScale, startRadians, endRadians, sweepAngle < 0);
            ctx.stroke();
        }
        onRenderedColorChanged: requestPaint()
        onRenderedOffsetYChanged: requestPaint()
        onRenderedRadiusChanged: requestPaint()
        onRenderedStartChanged: requestPaint()
        onRenderedValueChanged: requestPaint()
        onRenderedWidthChanged: requestPaint()
        onVisibleChanged: requestPaint()
    }
}
