import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Analyzer"
    Mixxx.SettingParameter {
        label: "A grey square"
        Rectangle {
            width: 20
            height: 20
            color: 'grey'
        }
    }
}
