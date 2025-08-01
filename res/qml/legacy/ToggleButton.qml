import QtQuick
import QtQuick.Controls

import "." as E

E.Button {
    id: control
    color: checked ? "#808080" : "#202020"
    onPressed: control.toggle()
    onReleased: {
    }
    onClicked: {
    }
}
