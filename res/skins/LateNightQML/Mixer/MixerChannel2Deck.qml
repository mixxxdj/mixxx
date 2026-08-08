import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool mirror: false
    property bool showEqKillButtons: true
    property bool showEqKnobs: true

    implicitHeight: 203
    implicitWidth: root.showEqKillButtons ? 109 : 91

    Item {
        anchors.left: root.mirror ? parent.left : undefined
        anchors.right: root.mirror ? undefined : parent.right
        height: parent.height
        width: 109

        EqQuickColumn {
            id: eqColumn

            alignRight: root.mirror
            group: root.group
            showEqKillButtons: root.showEqKillButtons
            showEqKnobs: root.showEqKnobs
            x: root.mirror ? 42 : 0
            y: 10
        }
        LateNightControls.Fader {
            backgroundSource: LateNightTheme.assetMixerVolumeSliderBackground
            group: root.group
            handleSource: LateNightTheme.assetMixerVolumeSliderHandle
            height: 107
            key: "volume"
            orientation: Qt.Vertical
            width: 42
            x: root.mirror ? 3 : 64
            y: 49
        }
        QuickEffectSelector {
            arrowOnRight: root.mirror
            group: root.group
            visible: root.showEqKnobs
            width: root.showEqKillButtons ? 62 : 40
            x: root.mirror ? 42 : (root.showEqKillButtons ? 5 : 18)
            y: 156
        }
        Row {
            visible: root.showEqKnobs
            x: root.mirror ? 46 : 22
            y: 180

            CrossfaderAssignButton {
                activeStyle: root.mirror ? "warning" : "default"
                group: root.group
                label: "left"
                side: 0
            }
            CrossfaderAssignButton {
                activeStyle: "warning"
                group: root.group
                label: "mid"
                side: 1
            }
            CrossfaderAssignButton {
                activeStyle: root.mirror ? "default" : "warning"
                group: root.group
                label: "right"
                side: 2
            }
        }
    }
}
