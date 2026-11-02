import "." as Skin
import QtQuick 2.12

import "Theme"

Skin.Button {
    id: root

    required property string keyPrefix
    required property string group

    activeColor: Theme.deckActiveColor
    highlight: cueBehavior.isActive

    IntroOutroButtonBehavior {
        id: cueBehavior

        group: root.group
        cueType: root.keyPrefix
        handlePointerInput: false
    }

    onPressed: {
        cueBehavior.pressPrimary();
    }
    onReleased: {
        cueBehavior.releasePrimary();
    }
    onCanceled: {
        cueBehavior.releasePrimary();
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onPressed: function(mouse) {
            cueBehavior.pressSecondary(mouse.x, mouse.y);
        }
        onReleased: {
            cueBehavior.releaseSecondary();
        }
        onCanceled: {
            cueBehavior.releaseSecondary();
        }
    }
}
