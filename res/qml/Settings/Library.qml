import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Library"

    Mixxx.SettingParameter {
        label: "A blue square"

        Rectangle {
            color: 'blue'
            height: 20
            width: 20
        }
    }
}
