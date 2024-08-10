import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string leftDeckGroup
    required property string rightDeckGroup
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

        Skin.EqColumn {
            group: root.leftDeckGroup
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: root.leftDeckGroup
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: root.rightDeckGroup
        }

        Skin.EqColumn {
            width: 56
            height: parent.height
            group: root.rightDeckGroup
        }
    }
}
