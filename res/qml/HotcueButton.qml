import "." as Skin
import QtQuick 2.12

Skin.Button {
    id: root

    required property int hotcueNumber
    required property string group

    text: hotcueNumber
    activeColor: hotcueBehavior.hotcueColor
    highlight: hotcueBehavior.isSet

    HotcueButtonBehavior {
        id: hotcueBehavior

        group: root.group
        hotcueNumber: root.hotcueNumber
        handlePointerInput: false

        onCleared: {
            popup.close();
        }

        onPopupRequested: function(mouseX, mouseY) {
            popup.x = mouseX;
            popup.y = mouseY;
            popup.open();
        }
    }

    Skin.HotcuePopup {
        id: popup

        hotcue: hotcue
    }

    Skin.Hotcue {
        id: hotcue

        group: root.group
        hotcueNumber: root.hotcueNumber
    }

    onPressed: {
        hotcueBehavior.pressPrimary();
    }
    onReleased: {
        hotcueBehavior.releasePrimary();
    }
    onCanceled: {
        hotcueBehavior.releasePrimary();
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onPressed: function(mouse) {
            hotcueBehavior.pressSecondary(mouse.x, mouse.y);
        }
        onReleased: {
            hotcueBehavior.releaseSecondary();
        }
        onCanceled: {
            hotcueBehavior.releaseSecondary();
        }
    }

    // A touchscreen has no right mouse button, so a long press stands in for
    // it and opens the hotcue popup. The default DragThreshold policy only
    // takes a passive grab, leaving the button itself to handle the press.
    TapHandler {
        id: longPressHandler

        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.DragThreshold

        onLongPressed: {
            const position = longPressHandler.point.position;
            hotcueBehavior.pressSecondary(position.x, position.y);
            hotcueBehavior.releaseSecondary();
        }
    }
}
