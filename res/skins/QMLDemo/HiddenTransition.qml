import QtQuick 2.12

Transition {
    id: root

    property var target // required

    to: "hidden"
    reversible: true

    SequentialAnimation {
        NumberAnimation {
            target: root.target
            property: "opacity"
            duration: 150
        }

        PropertyAction {
            target: root.target
            property: "visible"
        }

    }

}
