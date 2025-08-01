import QtQuick
import QtQuick.Controls

import "." as E

E.Button {
    id: control
    property string band
    implicitWidth: 18
    implicitHeight: 18
    color: control.checked ? "#b00000" : "#121212"
    text: control.checked ? "" : band
    font.family: "Open Sans"
    font.weight: Font.DemiBold
    onPressed: control.toggle()
}
