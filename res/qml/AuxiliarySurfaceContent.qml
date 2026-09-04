import QtQuick
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        AuxiliaryDeck {
            Layout.fillHeight: true
            Layout.fillWidth: true
            group: "[Channel1]"
            label: "Deck 1"
        }
        AuxiliaryDeck {
            Layout.fillHeight: true
            Layout.fillWidth: true
            group: "[Channel2]"
            label: "Deck 2"
        }
    }
}
