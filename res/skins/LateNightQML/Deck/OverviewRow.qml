import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../../../qml/Mixxx/Controls" as MixxxControls

Item {
    id: root

    required property string group
    readonly property bool useSecondaryDeckColors: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color overviewBackgroundColor: useSecondaryDeckColors ? LateNightTheme.secondaryOverviewBackgroundColor : LateNightTheme.primaryOverviewBackgroundColor
    readonly property color waveformSignalColor: useSecondaryDeckColors ? LateNightTheme.secondaryWaveformSignalColor : LateNightTheme.primaryWaveformSignalColor
    readonly property int waveformOverviewType: Math.round(waveformOverviewTypeProxy.value)
    readonly property bool useFilteredOverview: waveformOverviewType === 0

    Mixxx.ControlProxy {
        id: waveformOverviewTypeProxy
        group: "[Waveform]"
        key: "WaveformOverviewType"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.overviewBackgroundColor

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: "#0d0d0d"
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#121212"
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#2a2a2a"
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#252525"
            }

            MixxxControls.WaveformOverview {
                id: waveformOverview

                anchors.fill: parent
                anchors.margins: 1
                group: root.group
                colorLow: root.useFilteredOverview ? root.waveformSignalColor : "#0000ff"
                colorMid: root.useFilteredOverview ? root.waveformSignalColor : "#00ff00"
                colorHigh: root.useFilteredOverview ? root.waveformSignalColor : "#ff0000"
                renderer: root.useFilteredOverview ? Mixxx.WaveformOverview.Renderer.Filtered : Mixxx.WaveformOverview.Renderer.RGB
            }
        }

        // Settings panel.
        Rectangle {
            Layout.preferredWidth: 76
            Layout.fillHeight: true
            color: "#19191a"

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: "#0d0d0d"
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#121212"
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#2a2a2a"
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#252525"
            }

            DeckSettingsPlaceholder {
                anchors.fill: parent
                group: root.group
            }
        }
    }
}
