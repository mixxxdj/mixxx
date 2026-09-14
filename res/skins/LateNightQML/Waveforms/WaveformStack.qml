pragma ComponentBehavior: Bound

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
    readonly property int bottomGutterHeight: 3
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
    readonly property int waveformContentHeight: Math.max(0, root.height - root.bottomGutterHeight)
    property bool show4decks: false

    Loader {
        id: deck3waveform

        readonly property string group: root.deck3Group

        active: root.show4decks
        anchors.top: parent.top
        height: root.deck3MinimumHeight + root.extraHeightPerDeck
        width: root.width

        sourceComponent: Component {
            DeckWaveform {
                group: deck3waveform.group

                Shared.FadeBehavior on visible {
                    fadeTarget: deck3waveform
                }
            }
        }
    }
    DeckWaveform {
        id: deck1waveform

        anchors.top: root.show4decks ? deck3waveform.bottom : parent.top
        group: root.deck1Group
        height: deck1waveform.minimumHeight + root.extraHeightPerDeck
        width: root.width
    }
    DeckWaveform {
        id: deck2waveform

        anchors.bottom: root.show4decks ? deck4waveform.top : bottomGutter.top
        group: root.deck2Group
        height: deck2waveform.minimumHeight + root.extraHeightPerDeck
        width: root.width
    }
    Loader {
        id: deck4waveform

        readonly property string group: root.deck4Group

        active: root.show4decks
        anchors.bottom: bottomGutter.top
        height: root.deck4MinimumHeight + root.extraHeightPerDeck
        width: root.width

        sourceComponent: Component {
            DeckWaveform {
                group: deck4waveform.group

                Shared.FadeBehavior on visible {
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
