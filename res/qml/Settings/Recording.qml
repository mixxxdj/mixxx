import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Recording"
    Mixxx.SettingParameter {
        label: "A red square"
        Rectangle {
            width: 20
            height: 20
            color: 'red'
        }
    }
}
