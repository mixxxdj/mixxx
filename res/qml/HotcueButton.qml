import "." as Skin
import QtQuick 2.12
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts
import QtQuick.Controls 2

Item {
    id: root

    required property string group
    required property int hotcueNumber
    required property var label

    Skin.Hotcue {
        id: hotcue

        activate: activator.pressedButtons == Qt.LeftButton
        group: root.group
        hotcueNumber: root.hotcueNumber

        onIsSetChanged: {
            if (!isSet)
                popup.close();
        }
    }
    Skin.HotcuePopup {
        id: popup

        hotcue: hotcue
    }
    Rectangle {
        id: backgroundImage

        anchors.fill: parent
        color: hotcue.isSet ? hotcue.color : '#2B2B2B'

        MouseArea {
            id: activator

            acceptedButtons: Qt.LeftButton | Qt.RightButton
            anchors.fill: parent

            onClicked: mouse => {
                if (hotcue.isSet && mouse.button != Qt.LeftButton) {
                    popup.x = mouse.x;
                    popup.y = mouse.y;
                    popup.open();
                }
            }
        }
    }
    DropShadow {
        id: effect1

        anchors.fill: backgroundImage
        color: "#80000000"
        horizontalOffset: 0
        radius: 1.0
        source: backgroundImage
        verticalOffset: 0
    }
    InnerShadow {
        id: effect2

        anchors.fill: parent
        color: "#353535"
        horizontalOffset: 1
        radius: 12
        samples: 24
        source: effect1
        spread: 0.2
        verticalOffset: 1
    }
    InnerShadow {
        anchors.fill: parent
        color: "#353535"
        horizontalOffset: -1
        radius: 12
        samples: 24
        source: effect2
        spread: 0.2
        verticalOffset: -1
    }
    ColumnLayout {
        anchors.centerIn: backgroundImage
        spacing: 0

        Label {
            Layout.alignment: Qt.AlignHCenter
            color: "#626262"
            font.pixelSize: 14
            font.weight: Font.Bold
            text: `${index + 1}`
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            color: "#626262"
            font.pixelSize: 12
            text: root.label ?? ""
            visible: !!root.label
        }
    }
}
