import "." as Skin
import QtQuick 2.12

Skin.Button {
    id: root

    required property int hotcueNumber
    required property string group

    text: hotcueNumber
    activeColor: hotcue.color
    highlight: hotcue.isSet

    Skin.Hotcue {
        id: hotcue

        group: root.group
        hotcueNumber: root.hotcueNumber
        activate: root.down
        onIsSetChanged: {
            if (!isSet)
                popup.close();
        }
    }

    Skin.HotcuePopup {
        id: popup

        hotcue: hotcue
    }

    MouseArea {
        id: mousearea

        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: (mouse) => {
            if (hotcue.isSet) {
                popup.x = mouse.x;
                popup.y = mouse.y;
                popup.open();
            }
        }
    }
}
