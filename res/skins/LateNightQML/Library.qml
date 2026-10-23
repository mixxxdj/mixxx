import Mixxx 1.0 as Mixxx
import QtQuick
import "LateNightTheme"

Item {
    id: root

    function focusSearch() {
        legacyLibrary.focusSearch();
    }

    Rectangle {
        id: legacyLibraryGutter

        anchors.fill: parent
        color: LateNightTheme.libraryPanelSplitterBackground
    }

    Mixxx.LegacyLibraryItem {
        id: legacyLibrary

        anchors.fill: parent
        z: 1
    }

    Mixxx.PlayerDropArea {
        id: previewDeckDropArea

        x: legacyLibrary.previewDeckDropRect.x
        y: legacyLibrary.previewDeckDropRect.y
        width: legacyLibrary.previewDeckDropRect.width
        height: legacyLibrary.previewDeckDropRect.height
        visible: legacyLibrary.previewDeckDropEnabled
        group: "[PreviewDeck1]"
    }
}
