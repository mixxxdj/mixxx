import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    id: root
    label: "Sound hardware"

    property list<string> tabs: ["engine", "delays", "stats"]
    property int selectedIndex: 0

    Mixxx.SettingGroup {
        label: "Engine"
        visible: root.selectedIndex == 0
        Mixxx.SettingParameter {
            label: "A cyan square"
            Rectangle {
                width: 20
                height: 20
                color: 'cyan'
            }
        }
    }

    Mixxx.SettingGroup {
        label: "Delays"
        visible: root.selectedIndex == 1
        Mixxx.SettingParameter {
            label: "A magenta square"
            Rectangle {
                width: 20
                height: 20
                color: 'magenta'
            }
        }
    }

    Mixxx.SettingGroup {
        label: "Stats"
        visible: root.selectedIndex == 2
        Mixxx.SettingParameter {
            label: "A white square"
            Rectangle {
                width: 20
                height: 20
                color: 'white'
            }
        }
    }
}
