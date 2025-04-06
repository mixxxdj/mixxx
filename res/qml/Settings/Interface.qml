import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    property list<string> tabs: ["theme & colour", "waveform", "decks"]

    label: "Interface"

    Mixxx.SettingParameter {
        label: "A pink square"

        Rectangle {
            color: 'pink'
            height: 20
            width: 20
        }
    }
}
