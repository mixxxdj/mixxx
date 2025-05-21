import QtQuick
import Mixxx 1.0 as Mixxx

Category {
    label: "Analyzer"

    Mixxx.SettingParameter {
        label: "A grey square"

        Rectangle {
            color: 'grey'
            height: 20
            width: 20
        }
    }
}
