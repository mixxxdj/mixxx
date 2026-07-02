import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2
import QtQuick.Effects
import QtQuick.Shapes
import QtQuick.Controls
import "Theme"

Item {
    id: root

    required property string group
    property var modes: [0.04, 0.06, 0.08, 0.10, 0.16, 0.24, 0.50, 1]
    property alias range: rangeControl.value

    function nextRange() {
        const currentRange = rangeControl.value;
        let currentIdx = 0;
        let currentDiff = Math.abs(currentRange - root.modes[0]);
        for (let i = 1; i < root.modes.length; i++) {
            let diff = Math.abs(currentRange - root.modes[i]);
            if (currentDiff > diff) {
                currentIdx = i;
                currentDiff = diff;
            }
        }
        rangeControl.value = root.modes[(currentIdx + 1) % root.modes.length];
    }

    Skin.Button {
        id: button
        text: "Range"

        anchors.fill: parent

        onReleased: {
            if (popup.opened) {
                popup.close()
            } else {
                nextRange()
            }
        }
        onPressAndHold: {
            popup.open()
        }

        Mixxx.ControlProxy {
            id: rangeControl

            group: root.group
            key: "rateRange"
        }
    }
    Popup {
        id: popup

        closePolicy: Popup.NoAutoClose
        height: button.implicitHeight + 20
        padding: 0
        width: Math.max(button.implicitWidth * 1.5, 50)
        x: button.width / 2 - popup.width / 2;
        y: -height-10

        background: Item {
        }
        contentItem: Item {
            Item {
                id: contentPopup

                anchors.fill: parent

                layer.effect: MultiEffect {
                    autoPaddingEnabled: true
                    shadowEnabled: true
                    shadowColor: "#B0000000"
                    shadowBlur: 0.24
                    blurMultiplier: 0.24
                }

                Shape {
                    property int multiSamplingLevel: Mixxx.Config.multiSamplingLevel

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.bottom
                    antialiasing: true
                    height: width
                    layer.enabled: multiSamplingLevel > 1
                    layer.samples: multiSamplingLevel
                    width: 20

                    ShapePath {
                        capStyle: ShapePath.RoundCap
                        fillColor: Theme.embeddedBackgroundColor
                        fillRule: ShapePath.WindingFill
                        startX: 10
                        startY: 10
                        strokeColor: Theme.deckBackgroundColor
                        strokeWidth: 2

                        PathLine {
                            x: 20
                            y: 0
                        }
                        PathLine {
                            x: 0
                            y: 0
                        }
                        PathLine {
                            x: 10
                            y: 10
                        }
                    }
                }
                Skin.EmbeddedBackground {
                    anchors.fill: parent
                    anchors.topMargin: 10

                    Text {
                        anchors.centerIn: parent
                        color: Theme.white
                        text: `${parseInt(rangeControl.value*100)}%`
                    }
                }
            }
        }
    }
}
