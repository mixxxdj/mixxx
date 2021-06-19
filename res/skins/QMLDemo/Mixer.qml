import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: mixer

    property string leftDeckGroup // required
    property string rightDeckGroup // required
    property bool show4decks: false

    implicitWidth: content.width + 10
    implicitHeight: content.height + 10

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Row {
        id: content

        spacing: 5
        anchors.centerIn: parent
        anchors.margins: 5

        EqColumn {
            group: root.leftDeckGroup
        }

        MixerColumn {
            width: 56
            height: parent.height
            group: root.leftDeckGroup
        }

        MixerColumn {
            width: 56
            height: parent.height
            group: root.rightDeckGroup
        }

        EqColumn {
            width: 56
            height: parent.height
            group: root.rightDeckGroup
        }

    }

}
