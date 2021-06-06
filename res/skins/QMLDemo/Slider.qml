import "." as Skin
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Slider {
    id: root

    property alias fg: handleImage.source
    property alias bg: backgroundImage.source

    bar: true
    barColor: Theme.sliderBarColor
    barMargin: 10
    implicitWidth: handleImage.implicitWidth + 10
    implicitHeight: handleImage.implicitHeight + 10

    handle: Image {
        id: handleImage

        source: "images/slider_handle.svg"
        x: root.leftPadding + (root.availableWidth - width) / 2
        y: root.visualPosition * (root.height - height)
    }

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        anchors.margins: 10
    }

}
