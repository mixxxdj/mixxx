import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    property string leftDeckGroup // required
    property string rightDeckGroup // required
    property alias mixer: mixer

    implicitHeight: mixer.height

    Deck {
        id: leftDeck

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: mixer.left
        anchors.bottom: parent.bottom
        anchors.rightMargin: 5
        group: root.leftDeckGroup
    }

    Mixer {
        id: mixer

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        leftDeckGroup: root.leftDeckGroup
        rightDeckGroup: root.rightDeckGroup
    }

    Deck {
        id: rightDeck

        anchors.top: parent.top
        anchors.left: mixer.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        group: root.rightDeckGroup
    }

}
