pragma ComponentBehavior: Bound

import QtQuick
import Mixxx 1.0 as Mixxx

Item {
    id: root

    readonly property int minimumHeight: Math.max(
        30,
        beatgridControls.visible ? beatgridControls.implicitHeight : 0,
        stemControls.visible && stemControls.hasStems ? stemControls.implicitHeight : 0)
    implicitHeight: root.minimumHeight
    readonly property bool beatgridControlsVisible: beatgridControls.visible
    readonly property real beatgridControlsWidth: beatgridControls.width
    readonly property real beatgridControlsX: beatgridControls.x
    required property string group
    readonly property bool stemControlsVisible: stemControls.visible
    readonly property real stemControlsWidth: stemControls.width
    readonly property real stemControlsX: stemControls.x

    Mixxx.ControlProxy {
        id: showBeatgridControlsProxy

        group: "[Skin]"
        key: "show_beatgrid_controls"
    }
    Mixxx.ControlProxy {
        id: showStemControlsProxy

        group: "[Skin]"
        key: "show_stem_controls"
    }
    Mixxx.ControlProxy {
        id: splitStemTracksProxy

        group: "[Waveform]"
        key: "stem_split_tracks"
    }
    LateNightWaveformDisplay {
        id: waveformDisplay

        anchors.fill: parent
        group: root.group
        splitStemTracks: splitStemTracksProxy.value > 0

        onSplitStemTracksToggleRequested: splitStemTracksProxy.value = splitStemTracksProxy.value > 0 ? 0 : 1
    }
    StemControls {
        id: stemControls

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 26
        anchors.top: parent.top
        group: root.group
        visible: showStemControlsProxy.value > 0
        width: Math.min(implicitWidth, Math.max(0, parent.width - 26))
        z: 1
    }
    BeatgridControls {
        id: beatgridControls

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 26
        anchors.top: parent.top
        group: root.group
        visible: showBeatgridControlsProxy.value > 0
        width: Math.min(implicitWidth, Math.max(0, parent.width - 26))
        z: 1
    }
}
