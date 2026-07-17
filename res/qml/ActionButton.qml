import QtQuick
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    enum Category {
        None,
        Danger,
        Action
    }

    property var category: ActionButton.Category.None
    property alias label: labelField

    implicitHeight: 24

    background: Rectangle {
        id: content

        anchors.fill: parent
        color: root.category == ActionButton.Category.Action ? '#2D4EA1' : root.category == ActionButton.Category.Danger ? '#7D3B3B' : Theme.darkGray3
        radius: 4
        border {
            color: '#1C1C1C'
            width: 1
        }
    }
    contentItem: Item {
        Label {
            id: labelField

            anchors.fill: parent
            color: Theme.white
            font.bold: true
            font.capitalization: Font.AllUppercase
            font.family: Theme.fontFamily
            font.pixelSize: Theme.buttonFontPixelSize
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
