pragma ComponentBehavior: Bound

import QtQuick
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import "../LateNightTheme"

Item {
    id: root

    readonly property var currentTrack: deckPlayer?.currentTrack
    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    required property string group
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false
    readonly property bool showSpinnyOrCover: SpinnyCoverState.showSpinnyOrCover
    readonly property bool showSmallSpinnyOrCover: SpinnyCoverState.showSmallSpinnyOrCover
    readonly property bool showBigSpinnyOrCover: SpinnyCoverState.showBigSpinnyOrCover
    readonly property bool showCover: SpinnyCoverState.showCover
    readonly property bool showSpinny: SpinnyCoverState.showSpinny

    // Maintain a 1:1 aspect ratio (square)
    width: height

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
    // Spinny Platter Mode
    Item {
        id: spinnyContainer

        anchors.fill: parent
        visible: root.showSpinny

        // Platter Background
        Image {
            id: spinnyBg

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: LateNightTheme.assetDeckSpinnyBackground
        }
        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            // In legacy Spinny mode an unloaded deck remains the dark platter;
            // the default cover is used only by the standalone cover-art mode.
            source: root.currentTrack?.coverArtUrl ?? ""
            visible: root.showCover && root.isLoaded && !!root.currentTrack?.coverArtUrl
        }

        // Rotating Platter Indicator (Active when track is loaded)
        MixxxControls.Spinny {
            id: spinnyIndicator

            anchors.fill: parent
            group: root.group
            indicatorVisible: root.isLoaded

            ghostIndicatorVisible: root.isLoaded

            ghostIndicator: Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: LateNightTheme.assetDeckSpinnyGhostIndicator
            }

            indicator: Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: LateNightTheme.assetDeckSpinnyIndicator
            }

            z: 2
        }

        // Vinyl Grooves Overlay (Mask)
        Image {
            id: spinnyMask

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: {
                const isDeck12 = root.group === "[Channel1]" || root.group === "[Channel2]";
                return isDeck12 ? LateNightTheme.assetDeckSpinnyMask12 : LateNightTheme.assetDeckSpinnyMask34;
            }
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

    }

    // Cover Art Mode
    Item {
        id: coverArtContainer

        anchors.fill: parent
        visible: !root.showSpinny && root.showCover

        Image {
            id: coverArtImage

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: (root.isLoaded && currentTrack?.coverArtUrl) ? currentTrack.coverArtUrl : LateNightTheme.assetDeckCoverDefault
        }
    }
}
