pragma ComponentBehavior: Bound

import "../LateNightTheme"
import QtQuick
import Mixxx 1.0 as Mixxx

Item {
    id: root

    readonly property int minimumHeight: Math.max(
        30,
        beatgridControls.visible ? beatgridControls.implicitHeight : 0,
        stemControlsExpanded && stemControls.hasStems ? stemControls.implicitHeight : 0)
    implicitHeight: root.minimumHeight
    readonly property bool beatgridControlsExpanded: showBeatgridControlsProxy.value > 0
    property real beatgridControlsRevealProgress: beatgridControlsExpanded ? 1 : 0
    readonly property bool beatgridControlsVisible: beatgridControlsClip.visible
    readonly property real beatgridControlsWidth: beatgridControlsClip.width
    readonly property real beatgridControlsX: beatgridControlsClip.x
    required property string group
    readonly property bool stemControlsExpanded: showStemControlsProxy.value > 0
    property real stemControlsRevealProgress: stemControlsExpanded ? 1 : 0
    readonly property bool stemControlsVisible: stemControlsClip.visible
    readonly property real stemControlsWidth: stemControlsClip.width
    readonly property real stemControlsX: stemControlsClip.x

    Behavior on beatgridControlsRevealProgress {
        enabled: LateNightTheme.layoutAnimationsEnabled

        NumberAnimation {
            duration: 260
            easing.type: Easing.OutCubic
        }
    }
    Behavior on stemControlsRevealProgress {
        enabled: LateNightTheme.layoutAnimationsEnabled

        NumberAnimation {
            duration: 260
            easing.type: Easing.OutCubic
        }
    }

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
    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
        z: 100
    }
    Item {
        id: stemControlsClip

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 26
        anchors.top: parent.top
        clip: true
        visible: root.stemControlsExpanded || root.stemControlsRevealProgress > 0
        width: Math.min(stemControls.implicitWidth, Math.max(0, root.width - 26))
                * root.stemControlsRevealProgress
        z: 1

        StemControls {
            id: stemControls

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            group: root.group
            visible: stemControlsClip.visible
            width: Math.min(implicitWidth, Math.max(0, root.width - 26))
        }
    }
    Item {
        id: beatgridControlsClip

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 26
        anchors.top: parent.top
        clip: true
        visible: root.beatgridControlsExpanded || root.beatgridControlsRevealProgress > 0
        width: Math.min(beatgridControls.implicitWidth, Math.max(0, root.width - 26))
                * root.beatgridControlsRevealProgress
        z: 1

        BeatgridControls {
            id: beatgridControls

            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            group: root.group
            visible: beatgridControlsClip.visible
            width: Math.min(implicitWidth, Math.max(0, root.width - 26))
        }
    }
}
