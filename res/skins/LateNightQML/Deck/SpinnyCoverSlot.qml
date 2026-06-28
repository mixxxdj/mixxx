import QtQuick
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import "../LateNightTheme"

Item {
    id: root

    required property string group

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property var currentTrack: deckPlayer?.currentTrack
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

    readonly property bool showSpinny: showSpinniesProxy.value > 0 || showCoverArtProxy.value <= 0
    readonly property bool showCover: !showSpinny && showCoverArtProxy.value > 0

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

        // Rotating Platter Indicator (Active when track is loaded)
        MixxxControls.Spinny {
            id: spinnyIndicator
            anchors.fill: parent
            group: root.group
            indicatorVisible: root.isLoaded

            indicator: Image {
                anchors.fill: parent
                source: LateNightTheme.assetDeckSpinnyIndicator
                fillMode: Image.PreserveAspectFit
            }
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
