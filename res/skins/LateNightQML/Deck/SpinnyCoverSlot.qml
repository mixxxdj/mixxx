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
    // The derived controls choose the slot size, but the two source toggles
    // remain the authority for whether a slot exists at all. This prevents a
    // stale derived value from leaving an empty layout gap.
    readonly property bool showSpinnyOrCover: (showSpinniesProxy.value > 0 || showCoverArtProxy.value > 0)
            && (!showSpinnyOrCoverProxy.initialized || showSpinnyOrCoverProxy.value > 0)
    readonly property bool showSmallSpinnyOrCover: root.showSpinnyOrCover
            && (!showSmallSpinnyOrCoverProxy.initialized
                    ? selectBigSpinnyProxy.value <= 0
                    : showSmallSpinnyOrCoverProxy.value > 0)
    readonly property bool showBigSpinnyOrCover: root.showSpinnyOrCover
            && (!showBigSpinnyOrCoverProxy.initialized
                    ? selectBigSpinnyProxy.value > 0
                    : showBigSpinnyOrCoverProxy.value > 0)
    readonly property bool showCover: root.showSpinnyOrCover && showCoverArtProxy.value > 0
    readonly property bool showSpinny: root.showSpinnyOrCover && showSpinniesProxy.value > 0

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
        id: selectBigSpinnyProxy

        group: "[Skin]"
        key: "select_big_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSmallSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_small_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showBigSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_big_spinny_or_cover"
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

            indicator: Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: LateNightTheme.assetDeckSpinnyIndicator
            }
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
