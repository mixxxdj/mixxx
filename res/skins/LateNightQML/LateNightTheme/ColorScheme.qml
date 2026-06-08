pragma Singleton
import QtQuick
import Mixxx 1.0 as Mixxx
import "ColorSchemes"

QtObject {
    id: root

    readonly property string configScheme: Mixxx.Config.configScheme
    readonly property QtObject activeScheme: isClassicScheme(configScheme)
            ? classicScheme
            : paleMoonScheme

    readonly property color accentColor: activeScheme.accentColor
    readonly property string displayName: activeScheme.displayName
    readonly property string name: activeScheme.name

    function isClassicScheme(schemeName) {
        return String(schemeName).toLowerCase() === "classic";
    }

    readonly property QtObject paleMoonScheme: PaleMoon {
        id: paleMoonScheme
    }

    readonly property QtObject classicScheme: Classic {
        id: classicScheme
    }
}
