import QtQuick
import QtQuick.Window
import "../Controls" as Controls
import "../LateNightTheme"

Item {
    id: root

    property url backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    property url iconSource: ""
    /// Font Awesome icon name. Takes precedence over iconSource when set.
    property string iconName: ""
    property color iconColor: LateNightTheme.textColor
    /// Glyph size in pixels. Zero fits the glyph to the button, leaving a
    /// margin so that it never spills over the bezel.
    property real iconSize: 0
    property string label: ""
    property color labelColor: LateNightTheme.textColorMuted
    property int labelPixelSize: 11
    property real contentOpacity: 0.82

    property string activeBackgroundSuffix: ""
    property string activeIconSuffix: ""
    property string pressedBackgroundSuffix: ""
    property string pressedIconSuffix: ""
    property color activeColor: "transparent"
    property color pressedColor: "transparent"
    property color inactiveColor: LateNightTheme.deckButtonInactiveColor
    property bool inactiveFillEnabled: true
    property bool pressedActivatesFill: false
    property int fillMargin: 2
    property real fillRadius: 1
    property bool solidFillEnabled: false
    property bool useBorderImageBackground: false
    property int backgroundBorderTop: 0
    property int backgroundBorderBottom: 0
    property int backgroundBorderLeft: 0
    property int backgroundBorderRight: 0
    property bool activeState: false
    property bool pressedState: false
    property bool latchOverlayVisible: false
    property real latchOverlayProgress: 0
    property color latchOverlayColor: "transparent"
    property url latchOverlayBackgroundSource: backgroundSource
    property url latchOverlayIconSource: iconSource
    property string latchOverlayIconName: ""

    property bool stretchIcon: false
    property int iconLeftPadding: 0
    property int iconRightPadding: 0
    property int iconTopPadding: 0
    property int iconBottomPadding: 0
    property bool rasterizeIconAtPaintedSize: false

    readonly property url effectiveBackgroundSource: {
        var src = backgroundSource.toString();
        if (root.pressedState && pressedBackgroundSuffix.length > 0 && src.endsWith(".svg")) {
            return src.substring(0, src.length - 4) + "_" + pressedBackgroundSuffix + ".svg";
        }
        if (root.activeState && activeBackgroundSuffix.length > 0 && src.endsWith(".svg")) {
            return src.substring(0, src.length - 4) + "_" + activeBackgroundSuffix + ".svg";
        }
        return backgroundSource;
    }

    readonly property url effectiveIconSource: {
        var src = iconSource.toString();
        if (root.pressedState && pressedIconSuffix.length > 0 && src.endsWith(".svg")) {
            return src.substring(0, src.length - 4) + "_" + pressedIconSuffix + ".svg";
        }
        if (root.activeState && activeIconSuffix.length > 0 && src.endsWith(".svg")) {
            return src.substring(0, src.length - 4) + "_" + activeIconSuffix + ".svg";
        }
        return iconSource;
    }
    readonly property bool fillActive: root.activeState || (root.pressedState && root.pressedActivatesFill)
    readonly property color fillColor: {
        if (root.pressedState && root.pressedActivatesFill) {
            if (root.pressedColor.toString() !== "#00000000" && root.pressedColor.toString() !== "transparent") {
                return root.pressedColor;
            }
            if (root.activeColor.toString() !== "#00000000" && root.activeColor.toString() !== "transparent") {
                return root.activeColor;
            }
        }
        if (root.activeState && root.activeColor.toString() !== "#00000000" && root.activeColor.toString() !== "transparent") {
            return root.activeColor;
        }
        return root.inactiveFillEnabled ? root.inactiveColor : "transparent";
    }
    readonly property real iconAvailableWidth: Math.max(0, width - iconLeftPadding - iconRightPadding)
    readonly property real iconAvailableHeight: Math.max(0, height - iconTopPadding - iconBottomPadding)
    // The accent colour of the active/pressed state, or transparent when the
    // button has none and only uses the plain inactive fill.
    readonly property color activeAccentColor: {
        if (root.pressedState && root.pressedActivatesFill && root.pressedColor.toString() !== "#00000000" && root.pressedColor.toString() !== "transparent") {
            return root.pressedColor;
        }
        if (root.activeColor.toString() !== "#00000000" && root.activeColor.toString() !== "transparent") {
            return root.activeColor;
        }
        return "transparent";
    }
    // Active surfaces are translucent tints of the accent colour, so the panel
    // shows through and the control reads as glass instead of a solid block.
    readonly property color surfaceFillColor: root.fillActive && root.activeAccentColor.toString() !== "transparent"
            ? LateNightTheme.withAlpha(root.activeAccentColor,
                      root.pressedState
                              ? LateNightTheme.activePressedFillOpacity
                              : LateNightTheme.activeFillOpacity)
            : root.fillColor

    implicitWidth: 26
    implicitHeight: 26

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.fillMargin
        radius: root.fillRadius
        visible: !root.solidFillEnabled && root.surfaceFillColor.toString() !== "#00000000" && root.surfaceFillColor.toString() !== "transparent"

        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.lighter(root.surfaceFillColor, 1.08)
            }

            GradientStop {
                position: 0.5
                color: root.surfaceFillColor
            }

            GradientStop {
                position: 1
                color: Qt.darker(root.surfaceFillColor, 1.14)
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.fillMargin
        color: root.surfaceFillColor
        radius: root.fillRadius
        visible: root.solidFillEnabled && root.surfaceFillColor.toString() !== "#00000000" && root.surfaceFillColor.toString() !== "transparent"
    }

    Image {
        anchors.fill: parent
        source: root.effectiveBackgroundSource
        fillMode: Image.Stretch
        opacity: 1.0
        visible: !root.useBorderImageBackground
    }

    BorderImage {
        anchors.fill: parent
        source: root.effectiveBackgroundSource
        visible: root.useBorderImageBackground
        border {
            top: root.backgroundBorderTop
            bottom: root.backgroundBorderBottom
            left: root.backgroundBorderLeft
            right: root.backgroundBorderRight
        }
    }

    // Hairline border and top reflection, so every control reads as glass.
    Controls.GlassSurface {
        anchors.fill: parent
        borderColor: root.surfaceFillColor.toString() !== "transparent" && root.activeAccentColor.toString() !== "transparent"
                ? LateNightTheme.withAlpha(root.activeAccentColor, 0.75)
                : LateNightTheme.glassBorderColor
        cornerRadius: root.fillRadius
        margin: root.fillMargin
        shadeVisible: !root.fillActive
    }

    Image {
        id: iconImage
        x: root.stretchIcon ? root.iconLeftPadding : root.iconLeftPadding + (root.iconAvailableWidth - width) / 2
        y: root.stretchIcon ? root.iconTopPadding : root.iconTopPadding + (root.iconAvailableHeight - height) / 2
        width: root.stretchIcon ? root.iconAvailableWidth : Math.min(root.iconAvailableWidth, sourceSize.width > 0 ? sourceSize.width / 2 : root.iconAvailableWidth)
        height: root.stretchIcon ? root.iconAvailableHeight : Math.min(root.iconAvailableHeight, sourceSize.height > 0 ? sourceSize.height / 2 : root.iconAvailableHeight)
        source: root.effectiveIconSource
        sourceSize: root.rasterizeIconAtPaintedSize
                ? Qt.size(Math.ceil(width * Screen.devicePixelRatio), Math.ceil(height * Screen.devicePixelRatio))
                : Qt.size(-1, -1)
        fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
        opacity: root.contentOpacity
        visible: root.iconName.length === 0 && root.effectiveIconSource.toString().length > 0
    }

    Controls.IconLabel {
        anchors.bottomMargin: root.iconBottomPadding
        anchors.fill: parent
        anchors.leftMargin: root.iconLeftPadding
        anchors.rightMargin: root.iconRightPadding
        anchors.topMargin: root.iconTopPadding
        iconColor: root.iconColor
        iconName: root.iconName
        iconSize: root.iconSize > 0
                ? root.iconSize
                : Math.min(root.iconAvailableWidth, root.iconAvailableHeight) * 0.72
        opacity: root.contentOpacity
        visible: root.iconName.length > 0
    }

    Item {
        id: latchOverlay

        x: parent.width - width
        y: 0
        width: root.latchOverlayVisible ? parent.width * (1.0 - root.latchOverlayProgress) : 0
        height: parent.height
        clip: true
        visible: root.latchOverlayVisible && width > 0

        Rectangle {
            anchors.fill: parent
            color: root.latchOverlayColor
            visible: root.latchOverlayColor.toString() !== "#00000000" && root.latchOverlayColor.toString() !== "transparent"
        }

        Image {
            x: -latchOverlay.x
            y: 0
            width: root.width
            height: root.height
            source: root.latchOverlayBackgroundSource
            fillMode: Image.Stretch
            opacity: root.contentOpacity
        }

        Image {
            x: root.stretchIcon ? root.iconLeftPadding - latchOverlay.x : root.iconLeftPadding + (root.iconAvailableWidth - width) / 2 - latchOverlay.x
            y: root.stretchIcon ? root.iconTopPadding : root.iconTopPadding + (root.iconAvailableHeight - height) / 2
            width: root.stretchIcon ? root.iconAvailableWidth : Math.min(root.iconAvailableWidth, sourceSize.width > 0 ? sourceSize.width / 2 : root.iconAvailableWidth)
            height: root.stretchIcon ? root.iconAvailableHeight : Math.min(root.iconAvailableHeight, sourceSize.height > 0 ? sourceSize.height / 2 : root.iconAvailableHeight)
            source: root.latchOverlayIconSource
            sourceSize: root.rasterizeIconAtPaintedSize
                    ? Qt.size(Math.ceil(width * Screen.devicePixelRatio), Math.ceil(height * Screen.devicePixelRatio))
                    : Qt.size(-1, -1)
            fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
            opacity: root.contentOpacity
            visible: root.latchOverlayIconName.length === 0
                    && root.latchOverlayIconSource.toString().length > 0
        }

        Controls.IconLabel {
            anchors.bottomMargin: root.iconBottomPadding
            anchors.fill: parent
            anchors.leftMargin: root.iconLeftPadding - latchOverlay.x
            anchors.rightMargin: root.iconRightPadding
            anchors.topMargin: root.iconTopPadding
            iconColor: root.iconColor
            iconName: root.latchOverlayIconName
            iconSize: root.iconSize > 0
                    ? root.iconSize
                    : Math.min(root.iconAvailableWidth, root.iconAvailableHeight) * 0.72
            opacity: root.contentOpacity
            visible: root.latchOverlayIconName.length > 0
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.label
        font.family: LateNightTheme.fontFamily
        font.pixelSize: root.labelPixelSize
        font.bold: true
        color: root.labelColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: root.label.length > 0
    }
}
