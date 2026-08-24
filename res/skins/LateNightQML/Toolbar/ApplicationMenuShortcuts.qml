pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    signal applicationMenuRequested

    height: 0
    width: 0

    Connections {
        function onApplicationMenuRequested() {
            root.applicationMenuRequested();
        }

        target: Mixxx.Application
    }
    Shortcut {
        context: Qt.ApplicationShortcut
        enabled: Qt.platform.os !== "osx"
        sequence: "F10"

        onActivated: root.applicationMenuRequested()
    }
}
