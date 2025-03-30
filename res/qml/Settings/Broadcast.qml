import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Broadcast"
    Mixxx.SettingParameter {
        label: "A yellow square"
        Rectangle {
            width: 20
            height: 20
            color: 'yellow'
        }
    }
}
