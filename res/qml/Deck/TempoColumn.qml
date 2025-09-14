import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import "../Theme"

ColumnLayout {
    required property string group
    required property var currentTrack

    Mixxx.ControlProxy {
        id: keylockCO
        group: root.group
        key: pitchKey.key
    }

    Mixxx.ControlProxy {
        id: keyCO
        group: root.group
        key: "key"
    }

    RowLayout {
        height: 22
        Layout.fillWidth: true
        Skin.ControlButton {
            id: pitchDownButton

            implicitWidth: 20
            implicitHeight: 22

            group: root.group
            key: "pitch_down"
            contentItem: Item {
                anchors.fill: parent
                Shape {
                    anchors.centerIn: parent
                    width: 12
                    height: 10
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: '#626262'
                        strokeColor: 'transparent'
                        startX: 0; startY: 5
                        PathLine { x: 12; y: 0 }
                        PathLine { x: 12; y: 10 }
                        PathLine { x: 0; y: 5 }
                    }
                }
            }
            activeColor: Theme.deckActiveColor
        }
        Skin.ControlButton {
            id: pitchKey

            // FIXME: the following map are copied from S4 mapping. Once the interface setting PR is merged, we should use the palette
            readonly property variant colorsMap: [
                                                  "#b960a2",// 1d
                                                  "#9fc516", // 8d
                                                  "#527fc0", // 3d
                                                  "#f28b2e", // 10d
                                                  "#5bc1cf", // 5d
                                                  "#e84c4d", // 12d
                                                  "#73b629", // 7d
                                                  "#8269ab", // 2d
                                                  "#fdd615", // 9d
                                                  "#3cc0f0", // 4d
                                                  "#4cb686", // 11d
                                                  "#4cb686", // 6d
                                                  "#f5a158", // 10m
                                                  "#7bcdd9", // 5m
                                                  "#ed7171", // 12m
                                                  "#8fc555", // 7m
                                                  "#9b86be", // 2m
                                                  "#fcdf45", // 9m
                                                  "#63cdf4", // 4m
                                                  "#f1845f", // 11m
                                                  "#70c4a0", // 6m
                                                  "#c680b6", // 1m
                                                  "#b2d145", // 8m
                                                  "#7499cd"  // 3m
            ]
            readonly property variant textMap: [
                                                "1d",
                                                "8d",
                                                "3d",
                                                "10d",
                                                "5d",
                                                "12d",
                                                "7d",
                                                "2d",
                                                "9d",
                                                "4d",
                                                "11d",
                                                "6d",
                                                "10m",
                                                "5m",
                                                "12m",
                                                "7m",
                                                "2m",
                                                "9m",
                                                "4m",
                                                "11m",
                                                "6m",
                                                "1m",
                                                "8m",
                                                "3m"
            ]

            Layout.fillWidth: true
            Layout.leftMargin: 0
            Layout.rightMargin: 0

            implicitHeight: 22
            group: root.group
            key: "keylock"
            toggleable: true

            contentItem: Text {
                id: item
                text: {
                    if (!trackLoadedControl.value || keyCO.value <= 0) return "-"
                    return pitchKey.textMap[keyCO.value]
                }
                color: {
                    if (!trackLoadedControl.value || keyCO.value <= 0) {
                        return keylockCO.value ? Theme.white : Theme.midGray3
                    }
                    return pitchKey.colorsMap[keyCO.value]
                }
                font.pixelSize: 8
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Skin.ControlButton {
            id: pitchUpButton

            implicitWidth: 20
            implicitHeight: 22

            group: root.group
            key: "pitch_up"
            contentItem: Item {
                anchors.fill: parent
                Shape {
                    anchors.centerIn: parent
                    width: 12
                    height: 10
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: '#626262'
                        strokeColor: 'transparent'
                        startX: 0; startY: 0
                        fillRule: ShapePath.WindingFill
                        capStyle: ShapePath.RoundCap
                        PathLine { x: 12; y: 5 }
                        PathLine { x: 0; y: 10 }
                        PathLine { x: 0; y: 0 }
                    }
                }
            }
            activeColor: Theme.deckActiveColor
        }
    }
    Skin.ControlFader {
        id: rateSlider

        Layout.fillWidth: true
        Layout.fillHeight: true

        visible: !root.minimized
        width: pitchKey.implicitWidth
        group: root.group
        key: "rate"
        barStart: 0.5
        barMargin: 0
        barColor: Theme.bpmSliderBarColor
        bg: Theme.imgBpmSliderBackground

        Skin.FadeBehavior on visible {
            fadeTarget: rateSlider
        }
    }

    Skin.SyncButton {
        id: rangeButton
        Layout.fillWidth: true
        height: 22
        Layout.alignment: Qt.AlignHCenter
        group: root.group
    }

    Skin.RangeButton {
        id: syncButton
        Layout.fillWidth: true
        height: 22
        Layout.alignment: Qt.AlignHCenter
        group: root.group
    }
}
