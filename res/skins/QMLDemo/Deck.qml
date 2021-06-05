import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Item {
    id: root

    property string group // required
    property bool minimized: false

    Skin.DeckInfoBar {
        id: infoBar

        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        group: root.group
    }

    Skin.Slider {
        id: rateSlider

        visible: !root.minimized
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        anchors.top: infoBar.bottom
        anchors.right: parent.right
        anchors.bottom: buttonBar.top
        width: syncButton.width
        group: root.group
        key: "rate"
        barStart: 0.5
        barColor: Theme.bpmSliderBarColor
        bg: "images/slider_bpm.svg"

        FadeBehavior on visible {
            fadeTarget: rateSlider
        }

    }

    Rectangle {
        id: overview

        visible: !root.minimized
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        anchors.top: rateSlider.top
        anchors.bottom: buttonBar.top
        anchors.left: parent.left
        anchors.right: rateSlider.left
        radius: 5
        color: Theme.deckBackgroundColor
        height: 56

        Skin.WaveformOverview {
            group: root.group
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 26
        }

        Item {
            id: waveformBar

            height: 26
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Rectangle {
                id: waveformBarVSeparator

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.leftMargin: 5
                height: 2
                color: infoBar.lineColor
            }

            InfoBarButton {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: rateSlider.width
                group: "[EffectRack1_EffectUnit1]"
                key: "group_" + root.group + "_enable"
                activeColor: Theme.deckActiveColor

                foreground: Text {
                    anchors.centerIn: parent
                    text: "FX 1"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.family: Theme.fontFamily
                    font.bold: true
                    font.pixelSize: Theme.textFontPixelSize
                    color: infoBar.textColor
                }

            }

            Rectangle {
                id: waveformBarHSeparator1

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: waveformBarVSeparator.left
                anchors.leftMargin: rateSlider.width
                width: 2
                color: infoBar.lineColor
            }

            InfoBarButton {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: waveformBarHSeparator1.left
                width: rateSlider.width
                group: "[EffectRack1_EffectUnit2]"
                key: "group_" + root.group + "_enable"
                activeColor: Theme.deckActiveColor

                foreground: Text {
                    anchors.centerIn: parent
                    text: "FX 2"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.family: Theme.fontFamily
                    font.bold: true
                    font.pixelSize: Theme.textFontPixelSize
                    color: infoBar.textColor
                }

            }

            Rectangle {
                id: waveformBarHSeparator2

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: waveformBarHSeparator1.right
                anchors.leftMargin: rateSlider.width
                width: 2
                color: infoBar.lineColor
            }

            Text {
                id: waveformBarPosition

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: waveformBarHSeparator2.right
                anchors.leftMargin: 5
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textFontPixelSize
                color: infoBar.textColor
                text: {
                    const positionSeconds = samplesControl.value / 2 / sampleRateControl.value * playPositionControl.value;
                    if (isNaN(positionSeconds))
                        return "";

                    let minutes = Math.floor(positionSeconds / 60);
                    let seconds = positionSeconds - (minutes * 60);
                    let centiseconds = Math.trunc((seconds - Math.trunc(seconds)) * 100);
                    seconds = Math.trunc(seconds);
                    if (minutes < 10)
                        minutes = "0" + minutes;

                    if (seconds < 10)
                        seconds = "0" + seconds;

                    if (centiseconds < 10)
                        centiseconds = "0" + centiseconds;

                    return minutes + ':' + seconds + "." + centiseconds;
                }

                Mixxx.ControlProxy {
                    id: playPositionControl

                    group: root.group
                    key: "playposition"
                }

                Mixxx.ControlProxy {
                    id: sampleRateControl

                    group: root.group
                    key: "track_samplerate"
                }

                Mixxx.ControlProxy {
                    id: samplesControl

                    group: root.group
                    key: "track_samples"
                }

            }

            Item {
                id: waveformBarRightSpace

                anchors.top: waveformBar.top
                anchors.bottom: waveformBar.bottom
                anchors.right: waveformBar.right
                width: rateSlider.width
            }

            Rectangle {
                id: waveformBarHSeparator

                anchors.top: waveformBar.top
                anchors.bottom: waveformBar.bottom
                anchors.right: waveformBarRightSpace.left
                anchors.bottomMargin: 5
                width: 2
                color: infoBar.lineColor
            }

            InfoBarButton {
                anchors.top: waveformBarVSeparator.bottom
                anchors.bottom: waveformBar.bottom
                anchors.left: waveformBarRightSpace.left
                anchors.right: waveformBarRightSpace.right
                group: root.group
                key: "quantize"
                activeColor: Theme.deckActiveColor

                foreground: Image {
                    anchors.centerIn: parent
                    source: "images/icon_quantize.svg"
                }

            }

            Item {
                id: waveformBarLeftSpace

                anchors.top: waveformBar.top
                anchors.bottom: waveformBar.bottom
                anchors.right: waveformBarHSeparator.left
                width: rateSlider.width
            }

            Rectangle {
                id: waveformBarHSeparator3

                anchors.top: waveformBar.top
                anchors.bottom: waveformBar.bottom
                anchors.right: waveformBarLeftSpace.left
                anchors.bottomMargin: 5
                width: 2
                color: infoBar.lineColor
            }

            InfoBarButton {
                anchors.top: waveformBarVSeparator.bottom
                anchors.bottom: waveformBar.bottom
                anchors.left: waveformBarLeftSpace.left
                anchors.right: waveformBarLeftSpace.right
                group: root.group
                key: "passthrough"
                activeColor: Theme.deckActiveColor

                foreground: Image {
                    anchors.centerIn: parent
                    source: "images/icon_passthrough.svg"
                }

            }

        }

        FadeBehavior on visible {
            fadeTarget: overview
        }

    }

    Item {
        id: buttonBar

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 5
        height: 56
        visible: !root.minimized

        Skin.ControlButton {
            id: cueButton

            anchors.left: parent.left
            anchors.bottom: playButton.top
            anchors.bottomMargin: 5
            group: root.group
            key: "cue_default"
            text: "Cue"
            activeColor: Theme.deckActiveColor
        }

        Skin.ControlButton {
            id: playButton

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.topMargin: 5
            group: root.group
            key: "play"
            text: "Play"
            toggleable: true
            activeColor: Theme.deckActiveColor
        }

        Row {
            anchors.left: cueButton.right
            anchors.top: parent.top
            anchors.leftMargin: 10
            spacing: -1

            Repeater {
                model: 8

                Skin.HotcueButton {
                    // TODO: Once we require Qt >= 5.14, we're going to re-add
                    // the `required` keyword. If the component has any
                    // required properties, we'll stumble over a Qt bug and
                    // need the following workaround:
                    //     required property int index
                    // See this for details:
                    // https://bugreports.qt.io/browse/QTBUG-86009, and need

                    hotcueNumber: index + 1
                    group: root.group
                }

            }

        }

        Skin.ControlButton {
            id: syncButton

            anchors.right: parent.right
            anchors.top: parent.top
            text: "Sync"
            group: root.group
            key: "sync_enabled"
            toggleable: true
            activeColor: Theme.deckActiveColor
        }

        FadeBehavior on visible {
            fadeTarget: buttonBar
        }

    }

}
