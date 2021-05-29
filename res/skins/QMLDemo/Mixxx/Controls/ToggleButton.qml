import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    property alias group: control.group
    property alias key: control.key
    property alias icon: button.icon
    property alias background: button.background
    property alias state: buttonRect.state
    default property alias states: buttonRect.states

    Rectangle {
        id: buttonRect

        anchors.fill: parent
        color: "transparent"

        Button {
            id: button

            anchors.fill: parent
            onPressed: control.value ? control.value = 0 : control.value = 1

            Mixxx.ControlProxy {
                id: control

                onValueChanged: root.state = control.value
            }

        }

    }

}
