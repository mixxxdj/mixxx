import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Mixer & Effects"

    Mixxx.SettingParameter {
        label: "A green square"

        Rectangle {
            color: 'green'
            height: 20
            width: 20
        }
    }
}
