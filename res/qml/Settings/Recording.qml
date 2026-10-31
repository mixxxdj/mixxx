import QtQuick
import Mixxx 1.0 as Mixxx

Category {
    label: "Recording"

    Mixxx.SettingParameter {
        label: "A red square"

        Rectangle {
            color: 'red'
            height: 20
            width: 20
        }
    }
}
