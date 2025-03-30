import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "AutoDJ"
    Mixxx.SettingParameter {
        label: "A black square"
        Rectangle {
            width: 20
            height: 20
            color: 'black'
        }
    }
}
