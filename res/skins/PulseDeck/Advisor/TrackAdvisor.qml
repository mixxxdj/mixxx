pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

/// Suggests what to play next: the library tracks that mix harmonically with the
/// track on the deck that is playing. The candidates are the keys of the circle
/// of fifths and the tempo windows around the reference tempo, its half and its
/// double, which need only a small pitch adjustment to be beat matched.
Controls.Panel {
    id: root

    readonly property int rowHeight: 21
    /// Rows of a list that is not stretched, i.e. the height the panel asks for
    /// when nothing constrains it.
    readonly property int preferredRows: 8
    /// Suggestions that fit in the stretched panel, so that hiding the samplers
    /// fills the freed column with the whole list instead of leaving it short.
    readonly property int suggestionCount: Math.max(3, Math.min(30, Math.floor((root.height - header.implicitHeight - 2) / root.rowHeight)))
    /// Following the deck that is playing keeps the list in step with the set.
    /// Deck 1 is the fallback when neither deck is playing.
    readonly property string referenceGroup: deck2PlayControl.value > 0.0 && deck1PlayControl.value <= 0.0 ? "[Channel2]" : "[Channel1]"
    readonly property var referencePlayer: Mixxx.PlayerManager.getPlayer(root.referenceGroup)
    readonly property Mixxx.Track referenceTrack: root.referencePlayer?.currentTrack ?? null
    /// An unloaded deck still hands out an empty track object, so the control
    /// decides whether there is anything to suggest from.
    readonly property bool hasReferenceTrack: root.referenceTrack !== null && referenceLoadedControl.value > 0
    readonly property string referenceText: root.hasReferenceTrack ? (root.referenceTrack.artist ? root.referenceTrack.artist + " - " : "") + root.referenceTrack.title : ""
    property var suggestions: []

    color: LateNightTheme.deckPanelColor
    implicitHeight: header.implicitHeight + root.preferredRows * root.rowHeight + 2

    /// A taller panel holds a longer list, so the row count follows the height.
    onSuggestionCountChanged: root.refresh()

    /// The list is built once at start up, when the decks may already hold a
    /// track restored from the last session, and again whenever that track
    /// changes. The refresh button covers the library itself changing.
    Component.onCompleted: root.refresh()
    onReferenceTrackChanged: root.refresh()

    function refresh() {
        root.suggestions = root.hasReferenceTrack
                ? Mixxx.Library.findCompatibleTracks(root.referenceTrack, root.suggestionCount) : [];
    }

    function loadSuggestion(locationUrl) {
        if (!locationUrl) {
            return;
        }
        // The reference deck is left alone so that the suggestion lands on the
        // deck that is free to prepare it and the list stays valid.
        const targetDeck = root.referenceGroup === "[Channel1]" ? 2 : 1;
        Mixxx.PlayerManager.loadLocationUrlToDeck(locationUrl, targetDeck);
    }

    Rectangle {
        id: header

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: LateNightTheme.deckTopRowBackgroundColor
        implicitHeight: 24

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: LateNightTheme.deckPanelBorderDark
            height: 1
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 4
            spacing: 6

            Controls.IconLabel {
                iconColor: LateNightTheme.samplerColor
                iconName: "wand-magic-sparkles"
                iconSize: 11
            }
            Text {
                color: LateNightTheme.primaryDeckTextColor
                font.bold: true
                font.family: LateNightTheme.fontFamily
                font.pixelSize: 12
                text: "TRACK ADVISOR"
            }
            Text {
                Layout.fillWidth: true
                color: LateNightTheme.textColorMuted
                elide: Text.ElideRight
                font.family: LateNightTheme.fontFamily
                font.pixelSize: 10
                text: root.referenceText
            }
            Rectangle {
                Layout.preferredHeight: 18
                Layout.preferredWidth: 20
                color: refreshMouseArea.containsMouse ? LateNightTheme.deckButtonInactiveColor : "transparent"
                radius: 2

                Controls.IconLabel {
                    anchors.centerIn: parent
                    iconColor: LateNightTheme.samplerColor
                    iconName: "arrows-rotate"
                    iconSize: 11
                }
                MouseArea {
                    id: refreshMouseArea

                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.refresh()
                }
            }
        }
    }
    ListView {
        id: list

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        clip: true
        model: root.suggestions
        spacing: 1

        delegate: Rectangle {
            id: row

            required property int index
            required property var modelData

            color: rowMouseArea.containsMouse ? LateNightTheme.deckButtonInactiveColor : "transparent"
            height: root.rowHeight
            width: list.width

            MouseArea {
                id: rowMouseArea

                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.loadSuggestion(row.modelData.locationUrl)
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 6
                spacing: 6

                Text {
                    Layout.preferredWidth: 30
                    color: LateNightTheme.textColorMuted
                    font.family: LateNightTheme.fontFamily
                    font.pixelSize: 10
                    text: "(" + (row.modelData.timesPlayed ?? 0) + "x)"
                }
                Text {
                    Layout.preferredWidth: 36
                    color: LateNightTheme.deckAColor
                    font.family: LateNightTheme.fontFamily
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignRight
                    text: row.modelData.bpm > 0 ? Math.round(row.modelData.bpm).toString() : "---"
                }
                Text {
                    Layout.preferredWidth: 32
                    color: LateNightTheme.secondaryDeckTextColor
                    font.bold: true
                    font.family: LateNightTheme.fontFamily
                    font.pixelSize: 11
                    text: row.modelData.key > 0 ? Mixxx.KeyUtils.keyToString(row.modelData.key, keyNotationControl.value) : "---"
                }
                Text {
                    Layout.fillWidth: true
                    color: LateNightTheme.textColor
                    elide: Text.ElideRight
                    font.family: LateNightTheme.fontFamily
                    font.pixelSize: 11
                    text: (row.modelData.artist ? row.modelData.artist + " - " : "") + row.modelData.title
                }
            }
        }
    }
    Text {
        // Aligned over the list instead of using anchors.centerIn plus a
        // computed width, which Qt Quick reports as a binding loop.
        anchors.fill: list
        anchors.margins: 6
        color: LateNightTheme.textColorMuted
        font.family: LateNightTheme.fontFamily
        font.pixelSize: 10
        horizontalAlignment: Text.AlignHCenter
        text: root.hasReferenceTrack ? "No compatible tracks in the library" : "Load a track to see suggestions"
        verticalAlignment: Text.AlignVCenter
        visible: root.suggestions.length === 0
        wrapMode: Text.WordWrap
    }
    Mixxx.ControlProxy {
        id: deck1PlayControl

        group: "[Channel1]"
        key: "play"
    }
    Mixxx.ControlProxy {
        id: deck2PlayControl

        group: "[Channel2]"
        key: "play"
    }
    Mixxx.ControlProxy {
        id: keyNotationControl

        group: "[Library]"
        key: "key_notation"
    }
    Mixxx.ControlProxy {
        id: referenceLoadedControl

        group: root.referenceGroup
        key: "track_loaded"
    }
}
