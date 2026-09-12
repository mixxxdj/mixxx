pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"
import "../Mixer" as Mixer

Controls.Panel {
    id: root

    readonly property Mixxx.Track currentTrack: root.deckPlayer?.currentTrack ?? null
    readonly property Mixxx.Player deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    required property string group
    readonly property bool hasStems: stemCountProxy.value > 0 && root.currentTrack !== null
    property var stemInfos: []

    bottomBorderColor: LateNightTheme.isPaleMoon ? "#020202" : "#111111"
    clip: true
    color: LateNightTheme.waveformOverlayColor
    implicitHeight: root.hasStems ? stemChannelHeight * stemChannelCount : 0
    implicitWidth: 240
    leftBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"
    rightBorderColor: "#111111"
    topBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"

    readonly property int stemChannelCount: 4
    readonly property int stemChannelHeight: 26

    function refreshStemInfos() {
        const infos = [];
        for (let stemIdx = 0; stemIdx < root.stemChannelCount; stemIdx++) {
            infos.push(root.currentTrack ? root.currentTrack.stemsModel.get(stemIdx) : null);
        }
        root.stemInfos = infos;
    }

    Mixxx.ControlProxy {
        id: stemCountProxy

        group: root.group
        key: "stem_count"
    }
    Connections {
        function onStemsChanged() {
            root.refreshStemInfos();
        }

        target: root.currentTrack
    }
    Connections {
        function onTrackChanged() {
            root.refreshStemInfos();
        }

        target: root.deckPlayer
    }
    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent

        onWheel: wheel => wheel.accepted = true
    }
    Text {
        anchors.centerIn: parent
        color: LateNightTheme.waveformNoStemLabelColor
        font.family: "Open Sans"
        font.pixelSize: 13
        font.weight: Font.DemiBold
        text: qsTr("Track has no STEMs")
        visible: !root.hasStems
    }
    Column {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 0
        spacing: 0
        visible: root.hasStems

        Repeater {
            model: root.hasStems ? root.stemChannelCount : 0

            delegate: Item {
                id: stem

                required property int index
                readonly property var stemInfo: root.stemInfos[index] ?? null
                readonly property color stemColor: stemInfo && stemInfo.color
                        ? stemInfo.color
                        : "transparent"
                readonly property string label: stemInfo && stemInfo.label
                        ? stemInfo.label
                        : ""
                readonly property string quickEffectGroup: `[QuickEffectRack1_${stemGroup}]`
                readonly property string stemGroup: `${root.group.substring(0, root.group.length - 1)}_Stem${index + 1}]`

                height: Math.max(root.stemChannelHeight, parent.height / root.stemChannelCount)
                width: parent.width

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 50
                        color: stem.stemColor
                        elide: Text.ElideRight
                        font.family: "Open Sans"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        text: stem.label
                        verticalAlignment: Text.AlignVCenter
                    }
                    StemMuteButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 18
                        group: stem.stemGroup
                    }
                    Item {
                        Layout.preferredWidth: 2
                    }
                    Controls.Knob {
                        Layout.preferredHeight: 26
                        Layout.preferredWidth: 26
                        backgroundSource: LateNightTheme.assetFxKnobBackground
                        color: stem.stemColor
                        displayArc: true
                        displayArcColor: LateNightTheme.mixerArcGainColor
                        displayArcStart: Controls.Knob.ArcStart.Minimum
                        group: stem.stemGroup
                        indicatorColor: "orange"
                        key: "volume"
                    }
                    Item {
                        Layout.preferredWidth: 4
                    }
                    Mixer.QuickEffectSelector {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 85
                        arrowOnRight: true
                        group: stem.stemGroup
                        popupWidth: 160
                        textAlignRight: false
                    }
                    Item {
                        Layout.preferredWidth: 2
                    }
                    Mixer.QuickEffectEnableButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 18
                        quickEffectGroup: stem.quickEffectGroup
                    }
                    Item {
                        Layout.preferredWidth: 2
                    }
                    Controls.Knob {
                        Layout.preferredHeight: 26
                        Layout.preferredWidth: 26
                        backgroundSource: LateNightTheme.assetFxKnobBackground
                        color: stem.stemColor
                        displayArc: true
                        displayArcColor: LateNightTheme.mixerArcQuickEffectColor
                        group: stem.quickEffectGroup
                        indicatorColor: "green"
                        key: "super1"
                    }
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: LateNightTheme.deckPanelBorderDark
                    height: 1
                }
            }
        }
    }

    Component.onCompleted: root.refreshStemInfos()
}
