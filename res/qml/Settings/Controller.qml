import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Controllers"

    Mixxx.SettingParameter {
        label: "A orange square"

        Rectangle {
            color: 'orange'
            height: 20
            width: 20
        }
    }
}
