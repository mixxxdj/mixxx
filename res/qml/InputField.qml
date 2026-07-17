import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Effects
import "Theme"

FocusScope {
    id: root

    property alias input: inputField

    Rectangle {
        id: backgroundInput
        radius: 4
        color: '#232323'
        anchors.fill: parent
        border {
            color: '#353535'
            width: 1
        }
    }
    MultiEffect {
        anchors.fill: parent
        source: backgroundInput
        shadowEnabled: true
        shadowColor: "#000000"
        shadowBlur: 0.15
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
