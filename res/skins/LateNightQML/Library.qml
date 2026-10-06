import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    function focusSearch() {
        legacyLibrary.focusSearch();
    }

    Mixxx.LegacyLibraryItem {
        id: legacyLibrary

        anchors.fill: parent
    }
}
