import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitHeight: 18
    implicitWidth: 21

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: {
            switch (Math.round(orientationControl.value)) {
            case 0:
                return LateNightTheme.assetSamplerXfaderLeft;
            case 2:
                return LateNightTheme.assetSamplerXfaderRight;
            default:
                return LateNightTheme.assetSamplerXfaderMain;
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
