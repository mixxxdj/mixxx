import QtQuick
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import "../LateNightTheme"

Item {
    id: root

    required property string group

    readonly property Mixxx.Player deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property Mixxx.Track currentTrack: deckPlayer?.currentTrack
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false

    // Maintain a 1:1 aspect ratio (square)
    width: height

    Mixxx.ControlProxy {
        id: showSpinniesProxy
        group: "[Skin]"
        key: "show_spinnies"
    }

    Mixxx.ControlProxy {
        id: showCoverArtProxy
        group: "[Skin]"
        key: "show_coverart"
    }

    Mixxx.ControlProxy {
        id: vinylControlEnabledControl
        group: root.group
        key: "vinylcontrol_enabled"
    }

    Mixxx.ControlProxy {
        id: vinylSignalEnabledControl
        group: root.group
        key: "vinylcontrol_signal_enabled"
    }

    readonly property bool showSpinny: showSpinniesProxy.value > 0
    readonly property bool showCoverArt: showCoverArtProxy.value > 0
    readonly property bool showCover: !showSpinny && root.showCoverArt

    // Spinny Platter Mode
    Item {
        id: spinnyContainer
        anchors.fill: parent
        visible: root.showSpinny

        // Platter Background
        Image {
            id: spinnyBg
            anchors.fill: parent
            source: LateNightTheme.assetDeckSpinnyBackground
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: spinnyCoverArt
            anchors.fill: parent
            source: (root.isLoaded && root.currentTrack?.coverArtUrl)
                    ? root.currentTrack.coverArtUrl
                    : ""
            fillMode: Image.PreserveAspectFit
            visible: root.showCoverArt
        }

        // Vinyl Grooves Overlay (Mask)
        Image {
            id: spinnyMask
            anchors.fill: parent
            source: {
                const isDeck12 = root.group === "[Channel1]" || root.group === "[Channel2]";
                return isDeck12 ? LateNightTheme.assetDeckSpinnyMask12 : LateNightTheme.assetDeckSpinnyMask34;
            }
            fillMode: Image.PreserveAspectFit
        }

        Mixxx.VinylSignalQuality {
            id: vinylSignalQuality
            anchors.fill: parent
            group: root.group
            visible: root.showSpinny
            active: root.showSpinny
                    && vinylControlEnabledControl.value > 0
                    && vinylSignalEnabledControl.value > 0
            z: 1
        }

        // Rotating Platter Indicator (Active when track is loaded)
        MixxxControls.Spinny {
            id: spinnyIndicator
            anchors.fill: parent
            group: root.group
            indicatorVisible: root.isLoaded
            ghostIndicatorVisible: root.isLoaded
            z: 2

            ghostIndicator: Image {
                anchors.fill: parent
                source: LateNightTheme.assetDeckSpinnyGhostIndicator
                fillMode: Image.PreserveAspectFit
            }

            indicator: Image {
                anchors.fill: parent
                source: LateNightTheme.assetDeckSpinnyIndicator
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    // Cover Art Mode
    Item {
        id: coverArtContainer
        anchors.fill: parent
        visible: !root.showSpinny && root.showCover

        Image {
            id: coverArtImage
            anchors.fill: parent
            source: (root.isLoaded && currentTrack?.coverArtUrl) ? currentTrack.coverArtUrl : LateNightTheme.assetDeckCoverDefault
            fillMode: Image.PreserveAspectFit
        }
    }
}
