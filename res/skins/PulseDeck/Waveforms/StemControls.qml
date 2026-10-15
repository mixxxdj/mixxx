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
    implicitHeight: root.hasStems ? root.stemChannelHeight * root.stemChannelCount : 0
    implicitWidth: 234
    leftBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"
    rightBorderColor: "#111111"
    topBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"

    /// Number of stems of the loaded file, so a two stem file does not leave
    /// empty rows behind the real ones.
    readonly property int stemChannelCount: stemCountProxy.value > 0 ? stemCountProxy.value : 4
    readonly property int stemChannelHeight: 24
    /// Stem that is currently soloed, -1 when none is.
    property int soloStemIndex: -1
    property var muteValuesBeforeSolo: [0, 0, 0, 0]

    function refreshStemInfos() {
        const infos = [];
        for (let stemIdx = 0; stemIdx < root.stemChannelCount; stemIdx++) {
            infos.push(root.currentTrack ? root.currentTrack.stemsModel.get(stemIdx) : null);
        }
        root.stemInfos = infos;
    }

    /// Solo mutes the other stems of the deck and, when it is switched off
    /// again, puts back the mutes that were in place before.
    function toggleSolo(stemIndex) {
        if (root.soloStemIndex === stemIndex) {
            for (let stemIdx = 0; stemIdx < root.stemChannelCount; stemIdx++) {
                const proxy = stemMutes.objectAt(stemIdx);
                if (proxy) {
                    proxy.value = root.muteValuesBeforeSolo[stemIdx];
                }
            }
            root.soloStemIndex = -1;
            return;
        }
        if (root.soloStemIndex < 0) {
            const saved = [];
            for (let stemIdx = 0; stemIdx < root.stemChannelCount; stemIdx++) {
                const proxy = stemMutes.objectAt(stemIdx);
                saved.push(proxy ? proxy.value : 0);
            }
            root.muteValuesBeforeSolo = saved;
        }
        for (let stemIdx = 0; stemIdx < root.stemChannelCount; stemIdx++) {
            const proxy = stemMutes.objectAt(stemIdx);
            if (proxy) {
                proxy.value = stemIdx === stemIndex ? 0 : 1;
            }
        }
        root.soloStemIndex = stemIndex;
    }

    Mixxx.ControlProxy {
        id: stemCountProxy

        group: root.group
        key: "stem_count"
    }
    // One proxy per stem so solo can drive the mutes of the whole deck.
    // ControlProxy is not an Item, so this needs an Instantiator rather than a
    // Repeater.
    Instantiator {
        id: stemMutes

        model: root.stemChannelCount

        delegate: Mixxx.ControlProxy {
            required property int index

            group: `${root.group.substring(0, root.group.length - 1)}_Stem${index + 1}]`
            key: "mute"
        }
    }
    Connections {
        function onStemsChanged() {
            root.refreshStemInfos();
            root.soloStemIndex = -1;
        }

        target: root.currentTrack
    }
    Connections {
        function onTrackChanged() {
            root.refreshStemInfos();
            root.soloStemIndex = -1;
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
        font.family: LateNightTheme.fontFamily
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
                        Layout.preferredWidth: 46
                        color: stem.stemColor
                        elide: Text.ElideRight
                        font.family: LateNightTheme.fontFamily
                        font.pixelSize: 12
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
                    StemSoloButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 18
                        active: root.soloStemIndex === stem.index

                        onToggled: root.toggleSolo(stem.index)
                    }
                    Item {
                        Layout.preferredWidth: 4
                    }
                    Controls.Knob {
                        Layout.preferredHeight: 22
                        Layout.preferredWidth: 22
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
                        Layout.preferredWidth: 74
                        arrowOnRight: true
                        group: stem.stemGroup
                        popupWidth: 160
                        textAlignRight: false
                    }
                    Item {
                        Layout.preferredWidth: 2
                    }
                    Mixer.QuickEffectEnableButton {
                        Layout.preferredHeight: 16
                        Layout.preferredWidth: 16
                        quickEffectGroup: stem.quickEffectGroup
                    }
                    Item {
                        Layout.preferredWidth: 2
                    }
                    Controls.Knob {
                        Layout.preferredHeight: 22
                        Layout.preferredWidth: 22
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
