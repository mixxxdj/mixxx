import QtQuick 2.12

State {
    property alias target: changes.target

    name: "hidden"

    PropertyChanges {
        id: changes

        opacity: 0
        visible: false
    }

}
