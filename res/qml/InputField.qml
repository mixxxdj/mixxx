import QtQuick
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects
import "Theme"

FocusScope {
    id: root

    property alias input: inputField

    Rectangle {
        id: backgroundInput
        radius: 4
        color: '#232323'
        anchors.fill: parent
    }
    DropShadow {
        id: dropSetting
        anchors.fill: parent
        horizontalOffset: 0
        verticalOffset: 0
        radius: 4.0
        color: "#000000"
        source: backgroundInput
    }
    InnerShadow {
        id: effect2
        anchors.fill: parent
        source: dropSetting
        spread: 0.2
        radius: 12
        samples: 24
        horizontalOffset: 0
        verticalOffset: 0
        color: "#353535"
    }
    TextInput {
        id: inputField
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 7
        focus: true
        clip: true
        color: acceptableInput ? "#FFFFFF" : "#7D3B3B"
        horizontalAlignment: TextInput.AlignLeft
    }
}
