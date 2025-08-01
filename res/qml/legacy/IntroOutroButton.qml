import QtQuick
import QtQuick.Controls

import "." as E

E.ToggleButton {
    id: control
    implicitWidth: 24
    implicitHeight: 24
    color: checked ? "#257b82" : "#202020"
}
