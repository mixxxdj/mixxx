import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Fader {
    id: root

    property alias bg: backgroundImage.source
    property alias fg: handleImage.source
    property alias handleImage: handleImage

    bar: true
    barMargin: 10
    implicitHeight: backgroundImage.implicitHeight
    implicitWidth: backgroundImage.implicitWidth

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        anchors.margins: root.barMargin
    }
    handle: Item {
        id: handleItem

        height: handleImage.implicitHeight
        width: handleImage.implicitWidth
        x: root.horizontal ? (root.visualPosition * (root.width - width)) : ((root.width - width) / 2)
        y: root.vertical ? (root.visualPosition * (root.height - height)) : ((root.height - height) / 2)

        Rectangle {
            y: 5
            width: parent.width + 5
            height: parent.height + 5
            color: "#40000000"
            radius: 3
        }
        Image {
            id: handleImage

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: Theme.imgSliderHandle
        }
    }
}
