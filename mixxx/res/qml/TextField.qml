import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

TextField {
    color: Theme.textColor

    background: Rectangle {
        anchors.fill: parent
        color: Theme.embeddedBackgroundColor
        radius: 5
    }
}
