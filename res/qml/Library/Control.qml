import ".." as Skin
import "." as LibraryComponent
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    property alias focusWidget: focusedWidgetControl.value

    signal moveVertical(int offset)
    signal loadSelectedTrack(string group, bool play)
    signal loadSelectedTrackIntoNextAvailableDeck(bool play)

    Skin.FocusedWidgetControl {
        id: focusedWidgetControl

        Component.onCompleted: this.value = Skin.FocusedWidgetControl.WidgetKind.LibraryView
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "GoToItem"
        onValueChanged: (value) => {
            if (value != 0 && root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView)
                root.loadSelectedTrackIntoNextAvailableDeck(false);
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "LoadSelectedIntoFirstStopped"
        onValueChanged: (value) => {
            if (value != 0 && root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView)
                root.loadSelectedTrackIntoNextAvailableDeck(false);
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectTrackKnob"
        onValueChanged: (value) => {
            if (value != 0) {
                root.focusWidget = Skin.FocusedWidgetControl.WidgetKind.LibraryView;
                root.moveVertical(value);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectPrevTrack"
        onValueChanged: (value) => {
            if (value != 0) {
                root.focusWidget = Skin.FocusedWidgetControl.WidgetKind.LibraryView;
                root.moveVertical(-1);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectNextTrack"
        onValueChanged: (value) => {
            if (value != 0) {
                root.focusWidget = Skin.FocusedWidgetControl.WidgetKind.LibraryView;
                root.moveVertical(1);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveVertical"
        onValueChanged: (value) => {
            if (value != 0 && root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView)
                root.moveVertical(value);
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveUp"
        onValueChanged: (value) => {
            if (value != 0 && root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView)
                root.moveVertical(-1);
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveDown"
        onValueChanged: (value) => {
            if (value != 0 && root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView)
                root.moveVertical(1);
        }
    }

    Mixxx.ControlProxy {
        id: numDecksControl

        group: "[App]"
        key: "num_decks"
    }

    Instantiator {
        model: numDecksControl.value

        delegate: LibraryComponent.ControlLoadSelectedTrackHandler {
            required property int index

            group: "[Channel" + (index + 1) + "]"
            enabled: root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView
            onLoadTrackRequested: (play) => {
                root.loadSelectedTrack(this.group, play);
            }
        }
    }

    Mixxx.ControlProxy {
        id: numPreviewDecksControl

        group: "[App]"
        key: "num_preview_decks"
    }

    Instantiator {
        model: numPreviewDecksControl.value

        delegate: LibraryComponent.ControlLoadSelectedTrackHandler {
            required property int index

            group: "[PreviewDeck" + (index + 1) + "]"
            enabled: root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView
            onLoadTrackRequested: (play) => {
                root.loadSelectedTrack(this.group, play);
            }
        }
    }

    Mixxx.ControlProxy {
        id: numSamplersControl

        group: "[App]"
        key: "num_samplers"
    }

    Instantiator {
        model: numSamplersControl.value

        delegate: LibraryComponent.ControlLoadSelectedTrackHandler {
            required property int index

            group: "[Sampler" + (index + 1) + "]"
            enabled: root.focusWidget == Skin.FocusedWidgetControl.WidgetKind.LibraryView
            onLoadTrackRequested: (play) => {
                root.loadSelectedTrack(this.group, play);
            }
        }
    }
}
