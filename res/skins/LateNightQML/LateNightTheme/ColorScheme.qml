pragma Singleton
import QtQuick
import Mixxx 1.0 as Mixxx
import "ColorSchemes"

QtObject {
    readonly property string configScheme: Mixxx.Config.configScheme
    readonly property QtObject activeScheme: String(configScheme).toLowerCase() === "classic"
            ? classicScheme
            : paleMoonScheme

    readonly property color accentColor: activeScheme.accentColor
    readonly property string displayName: activeScheme.displayName
    readonly property string name: activeScheme.name

    readonly property QtObject paleMoonScheme: PaleMoon {}
    readonly property QtObject classicScheme: Classic {}
}
