import QtQuick
import QtQuick.Controls 2.12
import Qt5Compat.GraphicalEffects
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
    background: Item {
        Rectangle {
            id: content
            anchors.fill: parent
            color: root.category == ActionButton.Category.Action ? '#2D4EA1' : root.category == ActionButton.Category.Danger ? '#7D3B3B' : '#3F3F3F'
            radius: 4
        }
        DropShadow {
            anchors.fill: parent
            source: content
            horizontalOffset: 0
            verticalOffset: 0
            radius: 8.0
            color: "#80000000"
        }
    }
    contentItem: Item {
        Label {
            id: labelField
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: Theme.fontFamily
            font.capitalization: Font.AllUppercase
            font.bold: true
            font.pixelSize: Theme.buttonFontPixelSize
            color: Theme.white
        }
    }
}
