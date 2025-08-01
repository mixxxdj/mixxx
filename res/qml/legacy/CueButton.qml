import QtQuick
import QtQuick.Controls

import "." as E

E.Button {
    id: control
    property int cueId: 0
    implicitWidth: 24
    implicitHeight: 24
    color: checked ? "#c06020" : "#202020"
    text: cueId
    font.pixelSize: 14
    MouseArea {
        width: parent.width
        height: parent.height
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: mouse => {
            if ((mouse.button == Qt.RightButton) && (mouse.modifiers & Qt.ShiftModifier)) {
                if (control.checked) {
                    control.toggle();
                }
            } else if (!control.checked) {
                control.toggle();
            }
            // propagate to Button for pressed state
            mouse.accepted = false;
        }
    }
    onPressed: {
    }
    onReleased: {
    }
    onClicked: {
    }
}
