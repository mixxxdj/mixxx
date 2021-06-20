import "." as Skin
import Mixxx.Controls 0.1 as MixxxControls
import QtGraphicalEffects 1.12
import QtQuick 2.12
import "Theme"

MixxxControls.Slider {
    id: root

    property alias fg: handleImage.source
    property alias bg: backgroundImage.source

    bar: true
    barColor: Theme.sliderBarColor
    barMargin: 10
    implicitWidth: backgroundImage.implicitWidth
    implicitHeight: backgroundImage.implicitHeight

    Image {
        id: handleImage

        visible: false
        source: Theme.imgSliderHandle
        fillMode: Image.PreserveAspectFit
    }

    handle: Item {
        id: handleItem

        width: handleImage.paintedWidth
        height: handleImage.paintedHeight
        anchors.horizontalCenter: root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.horizontal ? parent.verticalCenter : undefined
        x: root.horizontal ? (root.visualPosition * (root.width - width)) : ((root.width - width) / 2)
        y: root.vertical ? (root.visualPosition * (root.height - height)) : ((root.height - height) / 2)

        DropShadow {
            source: handleImage
            width: parent.width + 5
            height: parent.height + 5
            radius: 5
            verticalOffset: 5
            color: "#80000000"
        }

    }

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        anchors.margins: root.barMargin
    }

}
