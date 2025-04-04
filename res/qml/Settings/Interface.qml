import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    label: "Interface"

    property list<string> tabs: ["theme & colour", "waveform", "decks"]

    Mixxx.SettingParameter {
        label: "A pink square"
        Rectangle {
            width: 20
            height: 20
            color: 'pink'
        }
    }
}
