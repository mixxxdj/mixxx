pragma Singleton
import QtQuick
import Mixxx 1.0 as Mixxx

QtObject {
    id: root

    readonly property string configScheme: Mixxx.Config.configScheme
    readonly property var schemes: ({
        "PaleMoon": {
            name: "palemoon",
            displayName: "PaleMoon",
            accentColor: "#d9b28c"
        },
        "Classic": {
            name: "classic",
            displayName: "Classic",
            accentColor: "#e7c413"
        }
    })

    readonly property var activeScheme: schemes[configScheme] || schemes["PaleMoon"]

    readonly property color accentColor: activeScheme.accentColor
    readonly property string displayName: activeScheme.displayName
    readonly property string name: activeScheme.name
}
