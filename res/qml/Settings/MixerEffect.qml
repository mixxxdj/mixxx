import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Mixer & Effects"
    Mixxx.SettingParameter {
        label: "A green square"
        Rectangle {
            width: 20
            height: 20
            color: 'green'
        }
    }
}
