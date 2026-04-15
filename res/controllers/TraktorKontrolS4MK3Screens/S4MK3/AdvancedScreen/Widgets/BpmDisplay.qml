import QtQuick 2.15

Item {
    anchors.fill: parent
    property int deckId: 0

    Rectangle {
        id: bpmBackground
        width: 60
        height: 20
        color: colors.grayBackground
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Text {
        text: deckInfo.bpmString
        color: "white"
        font.pixelSize: 17
        font.family: "Pragmatica"
        anchors.fill: bpmBackground
        anchors.rightMargin: 2
        anchors.topMargin: 1
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
