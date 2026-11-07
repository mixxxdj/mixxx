import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import "Theme"

MixxxControls.Fader {
    id: root

    property real backgroundMargin: bar.margin
    property alias bg: backgroundImage.source
    property alias fg: handleImage.source
    property alias handleImage: handleImage
    property bool showDefaultHandle: true
    property bool showHandleShadow: true

    bar.enabled: true
    bar.margin: 10
    implicitHeight: backgroundImage.implicitHeight
    implicitWidth: backgroundImage.implicitWidth

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        anchors.margins: root.backgroundMargin
    }
    handle: Item {
        id: handleItem

        height: handleImage.paintedHeight
        visible: root.showDefaultHandle
        width: handleImage.paintedWidth
        x: root.horizontal ? (root.visualPosition * (root.width - width)) : ((root.width - width) / 2)
        y: root.vertical ? (root.visualPosition * (root.height - height)) : ((root.height - height) / 2)

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: handleImage.source
            visible: !root.showHandleShadow
        }
        DropShadow {
            color: "#80000000"
            height: parent.height + 5
            radius: 5
            source: handleImage
            verticalOffset: 5
            visible: root.showHandleShadow
            width: parent.width + 5
        }
    }

    Image {
        id: handleImage

        fillMode: Image.PreserveAspectFit
        source: Theme.imgSliderHandle
        visible: false
    }
}
