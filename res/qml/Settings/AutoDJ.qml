import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "AutoDJ"

    Mixxx.SettingParameter {
        label: "A black square"

        Rectangle {
            color: 'black'
            height: 20
            width: 20
        }
    }
}
