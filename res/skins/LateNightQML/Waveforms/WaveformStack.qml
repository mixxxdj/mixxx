pragma ComponentBehavior: Bound

import ".." as LateNight
import "../LateNightTheme"
import "../../../qml" as Shared
import QtQuick
import Mixxx 1.0 as Mixxx
import "../Deck"

Item {
    id: root

    property string deck1Group: "[Channel1]"
    property string deck2Group: "[Channel2]"
    property string deck3Group: "[Channel3]"
    property string deck4Group: "[Channel4]"
    readonly property int bottomGutterHeight: 2
    readonly property var bottomDeck: root.show4decks ? deck4waveform.item : deck2waveform
    readonly property int deckCount: root.show4decks ? 4 : 2
    readonly property real deck3MinimumHeight: deck3waveform.item ? deck3waveform.item.minimumHeight : 30
    readonly property real deck4MinimumHeight: deck4waveform.item ? deck4waveform.item.minimumHeight : 30
    readonly property real minimumWaveformHeight: root.show4decks
            ? root.deck3MinimumHeight + deck1waveform.minimumHeight + deck2waveform.minimumHeight + root.deck4MinimumHeight
            : deck1waveform.minimumHeight + deck2waveform.minimumHeight
    readonly property real extraHeightPerDeck: Math.max(
            0,
            root.waveformContentHeight - Math.max(52, root.minimumWaveformHeight)) / root.deckCount
    readonly property int minimumContentHeight: root.bottomGutterHeight + Math.max(52, root.minimumWaveformHeight)
    implicitHeight: root.minimumContentHeight
    readonly property real waveformContentHeight: Math.max(0, root.height - root.bottomGutterHeight)
    property bool show4decks: false
    property bool splitterResizing: false
    property bool layoutTransitioning: false

    Loader {
        id: deck3waveform

        readonly property string group: root.deck3Group

        active: root.show4decks || height > 0 || opacity > 0
        anchors.top: parent.top
        height: deck3HeightAnimation.value
        opacity: root.show4decks ? 1 : 0
        width: root.width

        LateNight.LayoutAnimation {
            id: deck3HeightAnimation

            targetValue: root.show4decks ? root.deck3MinimumHeight + root.extraHeightPerDeck : 0
            animationEnabled: LateNightTheme.layoutAnimationsEnabled && !root.splitterResizing && !root.layoutTransitioning
        }
        Behavior on opacity {
            enabled: LateNightTheme.layoutAnimationsEnabled

            NumberAnimation {
                duration: 150
            }
        }

        sourceComponent: Component {
            DeckWaveform {
                group: deck3waveform.group

                Shared.FadeBehavior on visible {
                    enabled: LateNightTheme.layoutAnimationsEnabled
                    fadeTarget: deck3waveform
                }
            }
        }
    }
    DeckWaveform {
        id: deck1waveform

        anchors.top: deck3waveform.bottom
        group: root.deck1Group
        height: deck1HeightAnimation.value
        width: root.width

        LateNight.LayoutAnimation {
            id: deck1HeightAnimation

            targetValue: deck1waveform.minimumHeight + root.extraHeightPerDeck
            animationEnabled: LateNightTheme.layoutAnimationsEnabled && !root.splitterResizing && !root.layoutTransitioning
        }
    }
    DeckWaveform {
        id: deck2waveform

        anchors.bottom: deck4waveform.top
        group: root.deck2Group
        height: deck2HeightAnimation.value
        width: root.width

        LateNight.LayoutAnimation {
            id: deck2HeightAnimation

            targetValue: deck2waveform.minimumHeight + root.extraHeightPerDeck
            animationEnabled: LateNightTheme.layoutAnimationsEnabled && !root.splitterResizing && !root.layoutTransitioning
        }
    }
    Loader {
        id: deck4waveform

        readonly property string group: root.deck4Group

        active: root.show4decks || height > 0 || opacity > 0
        anchors.bottom: bottomGutter.top
        height: deck4HeightAnimation.value
        opacity: root.show4decks ? 1 : 0
        width: root.width

        LateNight.LayoutAnimation {
            id: deck4HeightAnimation

            targetValue: root.show4decks ? root.deck4MinimumHeight + root.extraHeightPerDeck : 0
            animationEnabled: LateNightTheme.layoutAnimationsEnabled && !root.splitterResizing && !root.layoutTransitioning
        }
        Behavior on opacity {
            enabled: LateNightTheme.layoutAnimationsEnabled

            NumberAnimation {
                duration: 150
            }
        }

        sourceComponent: Component {
            DeckWaveform {
                group: deck4waveform.group

                Shared.FadeBehavior on visible {
                    enabled: LateNightTheme.layoutAnimationsEnabled
                    fadeTarget: deck4waveform
                }
            }
        }
    }
    Item {
        id: bottomGutter

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.bottomGutterHeight
        z: 20

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.waveformContainerColor
            height: 1
        }
        Rectangle {
            anchors.top: parent.top
            color: LateNightTheme.waveformDeckBottomBorderColor
            height: 1
            x: 26
            width: Math.max(0, parent.width - 52)
        }
        Rectangle {
            anchors.top: parent.top
            color: LateNightTheme.waveformContainerColor
            height: 1
            visible: bottomDeck ? bottomDeck.stemControlsVisible : false
            width: bottomDeck ? bottomDeck.stemControlsWidth : 0
            x: bottomDeck ? bottomDeck.stemControlsX : 0
        }
        Rectangle {
            anchors.top: parent.top
            color: LateNightTheme.waveformContainerColor
            height: 1
            visible: bottomDeck ? bottomDeck.beatgridControlsVisible : false
            width: bottomDeck ? bottomDeck.beatgridControlsWidth : 0
            x: bottomDeck ? bottomDeck.beatgridControlsX : 0
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 1
            color: LateNightTheme.waveformContainerColor
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: LateNightTheme.waveformContainerBottomBorderColor
            height: 1
        }
    }
    Item {
        anchors.bottom: bottomGutter.top
        anchors.left: parent.left
        anchors.top: parent.top
        width: 26
        z: 10

        Rectangle {
            anchors.fill: parent
            color: LateNightTheme.waveformContainerColor
        }
        LateNightControlButton {
            activeOpacity: 1.0
            anchors.verticalCenter: parent.verticalCenter
            backgroundSource: ""
            group: "[Skin]"
            height: 52
            iconSource: isActive ? LateNightTheme.assetDeckStemControlsCollapseButton : LateNightTheme.assetDeckStemControlsExpandButton
            inactiveFillEnabled: false
            inactiveOpacity: 1.0
            key: "show_stem_controls"
            stretchIcon: true
            toggleable: true
            width: 26
        }
    }
    Item {
        anchors.bottom: bottomGutter.top
        anchors.right: parent.right
        anchors.top: parent.top
        width: 26
        z: 10

        Rectangle {
            anchors.fill: parent
            color: LateNightTheme.waveformContainerColor
        }
        LateNightControlButton {
            activeOpacity: 1.0
            anchors.verticalCenter: parent.verticalCenter
            backgroundSource: ""
            group: "[Skin]"
            height: 52
            iconSource: isActive ? LateNightTheme.assetDeckBeatgridControlsCollapseButton : LateNightTheme.assetDeckBeatgridControlsExpandButton
            inactiveFillEnabled: false
            inactiveOpacity: 1.0
            key: "show_beatgrid_controls"
            stretchIcon: true
            toggleable: true
            width: 26
        }
    }
}
