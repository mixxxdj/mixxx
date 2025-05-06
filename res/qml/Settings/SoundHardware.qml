import QtQuick
import Mixxx 1.0 as Mixxx

Category {
    id: root

    label: "Sound hardware"
    tabs: ["engine", "delays", "stats"]

    Mixxx.SettingGroup {
        label: "Engine"
        visible: root.selectedIndex == 0

        onActivated: {
            root.selectedIndex = 0;
        }

        Mixxx.SettingParameter {
            label: "A cyan square"

            Rectangle {
                color: 'cyan'
                height: 20
                width: 20
            }
        }
    }
    Mixxx.SettingGroup {
        label: "Delays"
        visible: root.selectedIndex == 1

        onActivated: {
            root.selectedIndex = 1;
        }

        Mixxx.SettingParameter {
            label: "A magenta square"

            Rectangle {
                color: 'magenta'
                height: 20
                width: 20
            }
        }
    }
    Mixxx.SettingGroup {
        label: "Stats"
        visible: root.selectedIndex == 2

        onActivated: {
            root.selectedIndex = 2;
        }

        Mixxx.SettingParameter {
            label: "A white square"

            Rectangle {
                color: 'white'
                height: 20
                width: 20
            }
        }
    }
}
