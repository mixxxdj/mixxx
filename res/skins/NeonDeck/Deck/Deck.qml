import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    enum DeckState {
        Mini = 0,
        Compact = 1,
        Full = 2
    }

    required property int deckState
    property bool editMode: false
    required property string group
    readonly property bool contentReady: deckLoader.status === Loader.Ready

    signal toggleFocus

    implicitHeight: deckLoader.item?.implicitHeight ?? 0
    implicitWidth: deckLoader.item?.implicitWidth ?? 0

    Loader {
        id: deckLoader

        anchors.fill: parent
        sourceComponent: {
            switch (root.deckState) {
            case Deck.Mini:
                return miniDeckComponent;
            case Deck.Compact:
                return compactDeckComponent;
            case Deck.Full:
            default:
                return fullDeckComponent;
            }
        }
    }
    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
    }
    // Serato style per-deck accent along the top edge of the deck.
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: LateNightTheme.deckColorForGroup(root.group)
        height: 2
        visible: LateNightTheme.usePerDeckColors
        z: 1000
    }
    Component {
        id: fullDeckComponent

        FullDeck {
            editMode: root.editMode
            group: root.group

            onToggleFocus: root.toggleFocus()
        }
    }
    Component {
        id: compactDeckComponent

        CompactDeck {
            group: root.group

            onToggleFocus: root.toggleFocus()
        }
    }
    Component {
        id: miniDeckComponent

        MiniDeck {
            group: root.group

            onToggleFocus: root.toggleFocus()
        }
    }
}
