import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick

SamplerGutter {
    id: root

    required property string controlKey
    readonly property bool expanded: expandControl.value > 0.5

    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        fillMode: Image.PreserveAspectFit
        height: 18
        source: root.expanded ? LateNightTheme.assetSamplerCollapseButton : LateNightTheme.assetSamplerExpandButton
        width: 16
    }
    TapHandler {
        onTapped: expandControl.value = root.expanded ? 0 : 1
    }
    Mixxx.ControlProxy {
        id: expandControl

        group: "[Skin]"
        key: root.controlKey
    }
}
