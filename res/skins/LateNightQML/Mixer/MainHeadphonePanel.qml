import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    readonly property int separatorY: root.show4decks ? 210 : 100
    property bool show4decks: false

    implicitHeight: root.show4decks ? 405 : 203
    implicitWidth: 96

    Rectangle {
        anchors.fill: parent
        color: "#171719"
    }
    Rectangle {
        color: LateNightTheme.mixerPanelBorderDark
        height: 1
        width: parent.width
        y: root.separatorY
    }
    Rectangle {
        color: LateNightTheme.mixerPanelBorderLight
        height: 1
        width: parent.width
        y: root.separatorY + 1
    }
    Item {
        height: root.separatorY
        width: parent.width

        MainHeadphoneKnob {
            group: "[Master]"
            indicatorColor: "orange"
            key: "gain"
            label: "MAIN"
            x: 7
            y: root.show4decks ? 10 : 20
        }
        MainHeadphoneKnob {
            group: "[Master]"
            indicatorColor: "red"
            key: "balance"
            label: "BAL"
            x: 49
            y: root.show4decks ? 10 : 20
        }
        Rectangle {
            color: "#040404"
            height: 96
            visible: root.show4decks
            width: 14
            x: 38
            y: 91
        }
        LateNightControls.ImageVuMeter {
            backgroundVariant: 0
            drawGroove: false
            group: "[Main]"
            height: 96
            levelKey: "vu_meter_left"
            peakKey: "peak_indicator_left"
            visible: root.show4decks
            width: 6
            x: 39
            y: 91
        }
        LateNightControls.ImageVuMeter {
            backgroundVariant: 0
            drawGroove: false
            group: "[Main]"
            height: 96
            levelKey: "vu_meter_right"
            peakKey: "peak_indicator_right"
            visible: root.show4decks
            width: 6
            x: 45
            y: 91
        }
        FxAssignButtons {
            anchors.horizontalCenter: parent.horizontalCenter
            groupName: "[MasterOutput]"
            y: root.show4decks ? 56 : 76
        }
    }
    Item {
        height: parent.height - y
        width: parent.width
        y: root.show4decks ? root.separatorY : root.separatorY + 2

        MainHeadphoneKnob {
            group: "[Master]"
            indicatorColor: "orange"
            key: "headGain"
            label: "HEAD"
            x: 7
            y: root.show4decks ? 58 : 9
        }
        MainHeadphoneKnob {
            group: "[Master]"
            indicatorColor: "red"
            key: "headMix"
            label: "MIX"
            x: 49
            y: root.show4decks ? 58 : 9
        }
        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 22
            width: 48
            y: root.show4decks ? 106 : 56

            Rectangle {
                anchors.fill: parent
                color: splitControl.value > 0 ? LateNightTheme.mixerSplitActiveColor : LateNightTheme.mixerSplitInactiveColor
            }

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: splitControl.value > 0 ? LateNightTheme.assetMixerSplitActiveIcon : LateNightTheme.assetMixerSplitIcon
            }
            Text {
                anchors.centerIn: parent
                color: LateNightTheme.mixerControlTextColor
                font.bold: true
                font.pixelSize: 12
                text: "SPLIT"
            }
            TapHandler {
                onTapped: splitControl.value = splitControl.value > 0 ? 0 : 1
            }
        }
        FxAssignButtons {
            anchors.horizontalCenter: parent.horizontalCenter
            groupName: "[Headphone]"
            y: root.show4decks ? 135 : 80
        }
    }
    Mixxx.ControlProxy {
        id: splitControl

        group: "[Master]"
        key: "headSplit"
    }
}
