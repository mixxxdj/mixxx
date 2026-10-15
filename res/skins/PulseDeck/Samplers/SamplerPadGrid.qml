pragma ComponentBehavior: Bound

import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

// Compact sampler pad grid that sits next to the browser, the way VirtualDJ
// and Serato keep their pads beside the track list instead of in a full width
// row underneath the decks.
Rectangle {
    id: root

    // The grid is built from preloaded samplers, so nothing has to be waited
    // for before the layout can be shown.
    readonly property bool contentReady: true
    // Keep the grid to at most two rows of four, which is what still fits at
    // the minimum window height without squeezing the pads.
    readonly property int padCount: Math.max(4, Math.min(8, Math.round(numSamplersControl.value)))

    color: LateNightTheme.layoutGutterColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LateNightTheme.isClassic ? 3 : 2
        spacing: LateNightTheme.deckRowGutter

        Text {
            Layout.fillWidth: true
            color: LateNightTheme.samplerTitleColor
            elide: Text.ElideRight
            font.bold: true
            font.family: LateNightTheme.fontFamily
            font.pixelSize: 13
            text: qsTr("Samplers")
        }
        GridLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            columnSpacing: LateNightTheme.deckRowGutter
            columns: 2
            rowSpacing: LateNightTheme.deckRowGutter

            Repeater {
                model: root.padCount

                SamplerMini {
                    required property int index

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    group: "[Sampler" + (index + 1) + "]"
                }
            }
        }
    }
    Mixxx.ControlProxy {
        id: numSamplersControl

        group: "[App]"
        key: "num_samplers"
    }
}
