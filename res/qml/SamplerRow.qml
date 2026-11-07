pragma ComponentBehavior: Bound

import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.12

GridLayout {
    id: root

    property int firstSampler: 1
    property int fxUnitCount: 4
    property int hotcueCount: 8
    property bool minimized: false
    property int samplerCount: 8
    property bool showFxAssignments: true
    property bool showHotcues: true
    property bool showRateControl: true

    columnSpacing: 0
    columns: Math.max(1, root.samplerCount)
    rowSpacing: 0

    Repeater {
        model: Math.max(0, root.samplerCount)

        Skin.Sampler {
            required property int index

            Layout.fillWidth: true
            fxUnitCount: root.fxUnitCount
            group: "[Sampler" + (root.firstSampler + index) + "]"
            hotcueCount: root.hotcueCount
            minimized: root.minimized
            showFxAssignments: root.showFxAssignments
            showHotcues: root.showHotcues
            showRateControl: root.showRateControl
        }
    }
}
