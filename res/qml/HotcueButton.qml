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
}
