import Mixxx 0.1 as Mixxx
import QtQuick 2.12

Item {
    id: root

    property bool focused: focusedWidgetControl.value == 3

    signal moveSelection(int offset)
    signal loadSelectedTrack(string group, bool play)
    signal loadSelectedTrackIntoNextAvailableDeck(bool play)

    Mixxx.ControlProxy {
        id: focusedWidgetControl

        group: "[Library]"
        key: "focused_widget"
        Component.onCompleted: value = 3
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "GoToItem"
        onValueChanged: {
            if (value != 0 && root.focused)
                root.loadSelectedTrackIntoNextAvailableDeck(false);

        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "LoadSelectedIntoFirstStopped"
        onValueChanged: {
            if (value != 0)
                root.loadSelectedTrackIntoNextAvailableDeck(false);

        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectTrackKnob"
        onValueChanged: {
            if (value != 0)
                root.moveSelection(value);

        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectPrevTrack"
        onValueChanged: {
            if (value != 0)
                root.moveSelection(-1);

        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectNextTrack"
        onValueChanged: {
            if (value != 0)
                root.moveSelection(1);

        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveVertical"
        onValueChanged: {
            if (root.focused && value != 0)
                root.moveSelection(value);

        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveUp"
        onValueChanged: {
            if (root.focused && value != 0)
                root.moveSelection(-1);

        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveDown"
        onValueChanged: {
            if (root.focused && value != 0)
                root.moveSelection(1);

        }
    }

    Mixxx.ControlProxy {
        id: numDecksControl

        group: "[Master]"
        key: "num_decks"
    }

    Instantiator {
        model: numDecksControl.value

        delegate: LibraryControlLoadSelectedTrackHandler {
            group: "[Channel" + (index + 1) + "]"
            enabled: root.focused
            onLoadTrackRequested: root.loadSelectedTrack(group, play)
        }

    }

    Mixxx.ControlProxy {
        id: numPreviewDecksControl

        group: "[Master]"
        key: "num_preview_decks"
    }

    Instantiator {
        model: numPreviewDecksControl.value

        delegate: LibraryControlLoadSelectedTrackHandler {
            group: "[PreviewDeck" + (index + 1) + "]"
            enabled: root.focused
            onLoadTrackRequested: root.loadSelectedTrack(group, play)
        }

    }

    Mixxx.ControlProxy {
        id: numSamplersControl

        group: "[Master]"
        key: "num_samplers"
    }

    Instantiator {
        model: numSamplersControl.value

        delegate: LibraryControlLoadSelectedTrackHandler {
            group: "[Sampler" + (index + 1) + "]"
            enabled: root.focused
            onLoadTrackRequested: root.loadSelectedTrack(group, play)
        }

    }

}
