import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToggleButton {
    id: control
    leftPadding: 3
    rightPadding: 3
    implicitHeight: 20
    font.pixelSize: 12
    color: control.checked ? "#207000" : "#121212"
}
