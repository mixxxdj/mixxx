import QtQuick
import "../LateNightTheme"

Item {
    id: root

    property url backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    property url iconSource: ""
    property string label: ""
    property color labelColor: LateNightTheme.textColorMuted
    property int labelPixelSize: 11
    property real contentOpacity: 0.82

    property string activeBackgroundSuffix: ""
    property string activeIconSuffix: ""
    property string pressedBackgroundSuffix: ""
    property string pressedIconSuffix: ""
    property color activeColor: "transparent"
    property color inactiveColor: LateNightTheme.deckButtonInactiveColor
    property bool inactiveFillEnabled: true
    property bool pressedActivatesFill: false
    property int fillMargin: 2
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

    property bool stretchIcon: false
    property int iconLeftPadding: 0
    property int iconRightPadding: 0
    property int iconTopPadding: 0
    property int iconBottomPadding: 0

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
    readonly property color fillColor: root.fillActive && root.activeColor.toString() !== "#00000000" && root.activeColor.toString() !== "transparent"
            ? root.activeColor
            : root.inactiveFillEnabled
                ? root.inactiveColor
                : "transparent"
    readonly property real iconAvailableWidth: Math.max(0, width - iconLeftPadding - iconRightPadding)
    readonly property real iconAvailableHeight: Math.max(0, height - iconTopPadding - iconBottomPadding)

    implicitWidth: 26
    implicitHeight: 26

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.fillMargin
        radius: 1
        visible: root.fillColor.toString() !== "#00000000" && root.fillColor.toString() !== "transparent"

        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.lighter(root.fillColor, 1.16)
            }

            GradientStop {
                position: 0.5
                color: root.fillColor
            }

            GradientStop {
                position: 1
                color: Qt.darker(root.fillColor, 1.25)
            }
        }
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

    Image {
        id: iconImage
        x: root.stretchIcon ? root.iconLeftPadding : root.iconLeftPadding + (root.iconAvailableWidth - width) / 2
        y: root.stretchIcon ? root.iconTopPadding : root.iconTopPadding + (root.iconAvailableHeight - height) / 2
        width: root.stretchIcon ? root.iconAvailableWidth : Math.min(root.iconAvailableWidth, sourceSize.width > 0 ? sourceSize.width / 2 : root.iconAvailableWidth)
        height: root.stretchIcon ? root.iconAvailableHeight : Math.min(root.iconAvailableHeight, sourceSize.height > 0 ? sourceSize.height / 2 : root.iconAvailableHeight)
        source: root.effectiveIconSource
        fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
        opacity: root.contentOpacity
        visible: root.effectiveIconSource.toString().length > 0
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
            fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
            opacity: root.contentOpacity
            visible: root.latchOverlayIconSource.toString().length > 0
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.label
        font.family: "Open Sans"
        font.pixelSize: root.labelPixelSize
        font.bold: true
        color: root.labelColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: root.label.length > 0
    }
}
