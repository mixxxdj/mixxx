pragma Singleton
import QtQuick
import Mixxx 1.0 as Mixxx

QtObject {
    id: root

    readonly property string configScheme: Mixxx.Config.configScheme

    readonly property string displayName: {
        if (configScheme === "PaleMoon") {
            return "PaleMoon";
        } else if (configScheme === "Classic") {
            return "Classic";
        } else {
            return "PaleMoon";
        }
    }

    readonly property string name: {
        if (configScheme === "PaleMoon") {
            return "palemoon";
        } else if (configScheme === "Classic") {
            return "classic";
        } else {
            return "palemoon";
        }
    }
}
