import QtQuick.Layouts 1.12

RowLayout {
    id: root

    property string leftDeckGroup // required
    property string rightDeckGroup // required

    height: leftDeck.height

    MiniDeck {
        id: leftDeck

        Layout.fillWidth: true
        group: root.leftDeckGroup
    }

    MiniDeck {
        id: rightDeck

        Layout.fillWidth: true
        group: root.rightDeckGroup
    }

}
