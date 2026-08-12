import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitHeight: 18
    implicitWidth: 42

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: {
            switch (Math.round(orientationControl.value)) {
            case 0:
                return LateNightTheme.assetAuxXfaderLeft;
            case 2:
                return LateNightTheme.assetAuxXfaderRight;
            default:
                return LateNightTheme.assetAuxXfaderMain;
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor

        onClicked: orientationControl.value = (Math.round(orientationControl.value) + 1) % 3
    }
    Mixxx.ControlProxy {
        id: orientationControl

        group: root.group
        key: "orientation"
    }
}
