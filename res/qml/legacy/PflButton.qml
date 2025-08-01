import QtQuick
import QtQuick.Controls

import "." as E

E.Button {
    id: control
    implicitWidth: 24
    implicitHeight: 24
    color: control.checked ? "#808080" : "transparent"
    onPressed: control.toggle()
    onReleased: {
    }
    onClicked: {
    }
    icon.source: "image://svgmodifier/icons/pfl.svg?#ff0000/" + (control.checked ? "#000000" : "#908070")
}
