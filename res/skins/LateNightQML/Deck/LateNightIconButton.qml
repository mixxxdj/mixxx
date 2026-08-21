import QtQuick
import "../LateNightTheme"

Item {
    id: root

    property string activeBackgroundSuffix: ""
    property color activeColor: "transparent"
    property string activeIconSuffix: ""
    property bool activeState: false
    property int backgroundBorderBottom: 0
    property int backgroundBorderLeft: 0
    property int backgroundBorderRight: 0
    property int backgroundBorderTop: 0
    property url backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    property real contentOpacity: 0.82
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
    readonly property color fillColor: root.fillActive && root.activeColor.toString() !== "#00000000" && root.activeColor.toString() !== "transparent" ? root.activeColor : root.inactiveFillEnabled ? root.inactiveColor : "transparent"
    property int fillMargin: 2
    property real fillRadius: 1
    readonly property real iconAvailableHeight: Math.max(0, height - iconTopPadding - iconBottomPadding)
    readonly property real iconAvailableWidth: Math.max(0, width - iconLeftPadding - iconRightPadding)
    property int iconBottomPadding: 0
    property int iconLeftPadding: 0
    property int iconRightPadding: 0
    property url iconSource: ""
    property int iconTopPadding: 0
    property color inactiveColor: LateNightTheme.deckButtonInactiveColor
    property bool inactiveFillEnabled: true
    property string label: ""
    property color labelColor: LateNightTheme.textColorMuted
    property int labelPixelSize: 11
    property url latchOverlayBackgroundSource: backgroundSource
    property color latchOverlayColor: "transparent"
    property url latchOverlayIconSource: iconSource
    property real latchOverlayProgress: 0
    property bool latchOverlayVisible: false
    property bool pressedActivatesFill: false
    property string pressedBackgroundSuffix: ""
    property string pressedIconSuffix: ""
    property bool pressedState: false
    property bool solidFillEnabled: false
    property bool stretchIcon: false
    property bool useBorderImageBackground: false

    implicitHeight: 26
    implicitWidth: 26

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.fillMargin
        radius: root.fillRadius
        visible: !root.solidFillEnabled && root.fillColor.toString() !== "#00000000" && root.fillColor.toString() !== "transparent"

        gradient: Gradient {
            GradientStop {
                color: Qt.lighter(root.fillColor, 1.16)
                position: 0
            }
            GradientStop {
                color: root.fillColor
                position: 0.5
            }
            GradientStop {
                color: Qt.darker(root.fillColor, 1.25)
                position: 1
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        anchors.margins: root.fillMargin
        color: root.fillColor
        radius: root.fillRadius
        visible: root.solidFillEnabled && root.fillColor.toString() !== "#00000000" && root.fillColor.toString() !== "transparent"
    }
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        opacity: 1.0
        source: root.effectiveBackgroundSource
        visible: !root.useBorderImageBackground
    }
    BorderImage {
        anchors.fill: parent
        source: root.effectiveBackgroundSource
        visible: root.useBorderImageBackground

        border {
            bottom: root.backgroundBorderBottom
            left: root.backgroundBorderLeft
            right: root.backgroundBorderRight
            top: root.backgroundBorderTop
        }
    }
    Image {
        id: iconImage

        fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
        height: root.stretchIcon ? root.iconAvailableHeight : Math.min(root.iconAvailableHeight, sourceSize.height > 0 ? sourceSize.height / 2 : root.iconAvailableHeight)
        opacity: root.contentOpacity
        source: root.effectiveIconSource
        visible: root.effectiveIconSource.toString().length > 0
        width: root.stretchIcon ? root.iconAvailableWidth : Math.min(root.iconAvailableWidth, sourceSize.width > 0 ? sourceSize.width / 2 : root.iconAvailableWidth)
        x: root.stretchIcon ? root.iconLeftPadding : root.iconLeftPadding + (root.iconAvailableWidth - width) / 2
        y: root.stretchIcon ? root.iconTopPadding : root.iconTopPadding + (root.iconAvailableHeight - height) / 2
    }
    Item {
        id: latchOverlay

        clip: true
        height: parent.height
        visible: root.latchOverlayVisible && width > 0
        width: root.latchOverlayVisible ? parent.width * (1.0 - root.latchOverlayProgress) : 0
        x: parent.width - width
        y: 0

        Rectangle {
            anchors.fill: parent
            color: root.latchOverlayColor
            visible: root.latchOverlayColor.toString() !== "#00000000" && root.latchOverlayColor.toString() !== "transparent"
        }
        Image {
            fillMode: Image.Stretch
            height: root.height
            opacity: root.contentOpacity
            source: root.latchOverlayBackgroundSource
            width: root.width
            x: -latchOverlay.x
            y: 0
        }
        Image {
            fillMode: root.stretchIcon ? Image.Stretch : Image.PreserveAspectFit
            height: root.stretchIcon ? root.iconAvailableHeight : Math.min(root.iconAvailableHeight, sourceSize.height > 0 ? sourceSize.height / 2 : root.iconAvailableHeight)
            opacity: root.contentOpacity
            source: root.latchOverlayIconSource
            visible: root.latchOverlayIconSource.toString().length > 0
            width: root.stretchIcon ? root.iconAvailableWidth : Math.min(root.iconAvailableWidth, sourceSize.width > 0 ? sourceSize.width / 2 : root.iconAvailableWidth)
            x: root.stretchIcon ? root.iconLeftPadding - latchOverlay.x : root.iconLeftPadding + (root.iconAvailableWidth - width) / 2 - latchOverlay.x
            y: root.stretchIcon ? root.iconTopPadding : root.iconTopPadding + (root.iconAvailableHeight - height) / 2
        }
    }
    Text {
        anchors.centerIn: parent
        color: root.labelColor
        font.bold: true
        font.family: "Open Sans"
        font.pixelSize: root.labelPixelSize
        horizontalAlignment: Text.AlignHCenter
        text: root.label
        verticalAlignment: Text.AlignVCenter
        visible: root.label.length > 0
    }
}
