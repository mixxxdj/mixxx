import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Library"
    Mixxx.SettingParameter {
        label: "A blue square"
        Rectangle {
            width: 20
            height: 20
            color: 'blue'
        }
    }
}
