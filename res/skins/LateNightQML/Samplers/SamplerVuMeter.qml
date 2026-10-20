import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    required property string group

    implicitHeight: 62
    implicitWidth: 8

    Column {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 2
        z: 1

        Item {
            height: 9
            width: 6

            Image {
                anchors.fill: parent
                smooth: false
                source: LateNightTheme.assetSamplerVuClippingBackground
            }
            Image {
                anchors.fill: parent
                smooth: false
                source: LateNightTheme.assetSamplerVuClippingActive
                visible: peakControl.value > 0
            }
        }
        Item {
            height: 49
            width: 6

            Image {
                anchors.fill: parent
                smooth: false
                source: LateNightTheme.assetSamplerVuLevelBackground
            }
            Item {
                id: activeClip

                anchors.bottom: parent.bottom
                clip: true
                height: Math.max(0, Math.min(1, vuControl.parameter)) * parent.height
                width: parent.width

                Image {
                    anchors.bottom: parent.bottom
                    height: 49
                    smooth: false
                    source: LateNightTheme.assetSamplerVuLevelActive
                    width: 6
                }
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        color: "#040404"
    }
    Mixxx.ControlProxy {
        id: peakControl

        group: root.group
        key: "peak_indicator"
    }
    Mixxx.ControlProxy {
        id: vuControl

        group: root.group
        key: "vu_meter"
    }
}
