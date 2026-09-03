import QtQuick
import Mixxx 1.0 as Mixxx

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
