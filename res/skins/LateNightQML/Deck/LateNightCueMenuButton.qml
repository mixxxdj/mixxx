import QtQuick
import "../LateNightTheme"

LateNightIconButton {
    id: root

    property bool checked: false
    property bool impossible: false
    property bool deleteButton: false
    property int iconSize: LateNightTheme.isClassic ? 20 : 24
    property color checkedColor: LateNightTheme.isClassic ? "#db0000" : "#b24c12"
    property color deleteHoverColor: "#6c2e2e"
    property color deletePressedColor: "#dc4141"

    signal clicked
    signal rightClicked

    width: 28
    height: deleteButton ? 28 : 46
    backgroundSource: LateNightTheme.lateNightButton("btn_embedded_library.svg")
    useBorderImageBackground: true
    backgroundBorderTop: 2
    backgroundBorderBottom: 2
    backgroundBorderLeft: 2
    backgroundBorderRight: 2
    activeBackgroundSuffix: "active"
    pressedBackgroundSuffix: "active"
    inactiveColor: impossible ? "#787878" : "#262626"
    activeColor: deleteButton && interactionArea.containsMouse ? deleteHoverColor : checkedColor
    pressedColor: deleteButton ? deletePressedColor : activeColor
    activeState: checked || (deleteButton && interactionArea.containsMouse)
    pressedState: interactionArea.pressed
    pressedActivatesFill: true
    inactiveFillEnabled: true
    fillMargin: 2
    contentOpacity: enabled ? 1.0 : 0.62
    stretchIcon: true
    iconLeftPadding: Math.max(0, (width - iconSize) / 2)
    iconRightPadding: iconLeftPadding
    iconTopPadding: Math.max(0, (height - iconSize) / 2)
    iconBottomPadding: iconTopPadding

    MouseArea {
        id: interactionArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: root.enabled
        hoverEnabled: root.deleteButton

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                root.rightClicked();
            } else {
                root.clicked();
            }
        }
    }
}
