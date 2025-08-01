import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E
import Mixxx 1.0 as Mixxx

GridLayout {
    Layout.alignment: Qt.AlignTop
    rowSpacing: 0
    columnSpacing: 2
    columns: 2
    E.EqKillButton {
        band: "H"
        Layout.alignment: Qt.AlignVCenter
        onCheckedChanged: filterHighKill.parameter = checked

        Mixxx.ControlProxy {
            id: filterHighKill

            group: "[Channel1]"
            key: "filterHighKill"
        }
    }
    E.EqDial {
        Mixxx.ControlProxy {
            id: filterHigh

            group: "[Channel1]"
            key: "filterHigh"
        }

        value: filterHigh.parameter
        onMoved: filterHigh.parameter = value

        TapHandler {
            onDoubleTapped: filterHigh.reset()
        }
    }
    E.EqKillButton {
        band: "M"
        Layout.alignment: Qt.AlignVCenter
    }
    E.EqDial {
    }
    E.EqKillButton {
        band: "L"
        Layout.alignment: Qt.AlignVCenter
    }
    E.EqDial {
    }
    E.FxButton {
    }
    E.FxDial {
    }
}
