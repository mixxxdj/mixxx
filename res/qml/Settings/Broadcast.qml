import QtQuick
import Mixxx 1.0 as Mixxx

Category {
    label: "Broadcast"

    Mixxx.SettingParameter {
        label: "A yellow square"

        Rectangle {
            color: 'yellow'
            height: 20
            width: 20
        }
    }
}
