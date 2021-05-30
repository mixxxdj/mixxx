import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Item {
    id: root

    required property string group

    Skin.DeckInfoBar {
        id: infoBar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.deckBackgroundColor
        group: root.group
    }

    Skin.Slider {
        id: rateSlider

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
    }

    Rectangle {
        id: overview

        anchors.topMargin: 5
        anchors.bottomMargin: 5
        anchors.rightMargin: 5
        anchors.top: infoBar.bottom
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

            Text {
                id: waveformBarPosition

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 5
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textFontPixelSize
                color: infoBar.textColor
                text: {
                    let positionSeconds = samplesControl.value / 2 / sampleRateControl.value * playPositionControl.value;
                    if (isNaN(positionSeconds))
                        return "";

                    var minutes = Math.floor(positionSeconds / 60);
                    var seconds = positionSeconds - (minutes * 60);
                    var subSeconds = Math.trunc((seconds - Math.trunc(seconds)) * 100);
                    seconds = Math.trunc(seconds);
                    if (minutes < 10)
                        minutes = "0" + minutes;

                    if (seconds < 10)
                        seconds = "0" + seconds;

                    if (subSeconds < 10)
                        subSeconds = "0" + subSeconds;

                    return minutes + ':' + seconds + "." + subSeconds;
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
                checkable: true
                activeColor: Theme.deckActiveColor
                foreground: "images/icon_quantize.svg"
            }

            Item {
                id: waveformBarLeftSpace

                anchors.top: waveformBar.top
                anchors.bottom: waveformBar.bottom
                anchors.right: waveformBarHSeparator.left
                width: rateSlider.width
            }

            Rectangle {
                id: waveformBarHSeparator2

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
                checkable: true
                activeColor: Theme.deckActiveColor
                foreground: "images/icon_passthrough.svg"
            }

        }

    }

    Item {
        id: buttonBar

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 56

        Skin.ControlButton {
            id: cueButton

            anchors.left: parent.left
            anchors.top: parent.top
            group: root.group
            key: "cue_default"
            text: "Cue"
            activeColor: Theme.deckActiveColor
        }

        Skin.ControlButton {
            id: playButton

            anchors.top: cueButton.bottom
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.topMargin: 5
            group: root.group
            key: "play"
            text: "Play"
            checkable: true
            activeColor: Theme.deckActiveColor
        }

        Row {
            anchors.left: cueButton.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 10
            spacing: -1

            Repeater {
                model: 8

                Skin.HotcueButton {
                    // Make index a required property, workaround for
                    // https://bugreports.qt.io/browse/QTBUG-86009
                    required property int index

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
            checkable: true
            activeColor: Theme.deckActiveColor
        }

    }

}
