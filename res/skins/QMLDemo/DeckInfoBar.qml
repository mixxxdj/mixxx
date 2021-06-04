import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property string group // required
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property color textColor: Theme.deckTextColor
    property color lineColor: Theme.deckLineColor

    radius: 5
    height: 56

    Rectangle {
        id: coverArt

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: height
        radius: height / 2
        border.width: 2
        border.color: Theme.deckLineColor
        color: "transparent"
    }

    Item {
        id: spinny

        anchors.fill: coverArt

        // The Spinnies are automatically hidden if the track
        // is stopped. This is not really useful, but is nice for
        // demo'ing transitions.
        Mixxx.ControlProxy {
            group: root.group
            key: "play"
            onValueChanged: spinnyIndicator.indicatorVisible = (value > 0)
        }

        MixxxControls.Spinny {
            id: spinnyIndicator

            anchors.fill: parent
            group: root.group
            indicatorVisible: false

            indicator: Item {
                width: spinnyIndicator.width
                height: spinnyIndicator.height

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 2
                    height: parent.height / 2
                    color: Theme.deckTextColor
                }

            }

        }

    }

    Text {
        id: infoBarTitle

        text: root.deckPlayer.title
        anchors.top: infoBarHSeparator1.top
        anchors.left: infoBarVSeparator.left
        anchors.right: infoBarHSeparator1.left
        anchors.bottom: infoBarVSeparator.bottom
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: infoBar.textColor
    }

    Rectangle {
        id: infoBarVSeparator

        anchors.left: coverArt.right
        anchors.right: infoBar.right
        anchors.verticalCenter: infoBar.verticalCenter
        anchors.margins: 5
        height: 2
        color: infoBar.lineColor
    }

    Text {
        id: infoBarArtist

        text: root.deckPlayer.artist
        anchors.top: infoBarVSeparator.bottom
        anchors.left: infoBarVSeparator.left
        anchors.right: infoBarHSeparator1.left
        anchors.bottom: infoBarHSeparator1.bottom
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: infoBar.textColor
    }

    Rectangle {
        id: infoBarHSeparator1

        anchors.top: infoBar.top
        anchors.bottom: infoBar.bottom
        anchors.right: infoBarKey.left
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        width: 2
        color: infoBar.lineColor
    }

    Text {
        id: infoBarKey

        anchors.top: infoBarHSeparator1.top
        anchors.bottom: infoBarVSeparator.top
        width: rateSlider.width
        anchors.right: infoBarHSeparator2.left
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: root.deckPlayer.keyText
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: infoBar.textColor
    }

    Rectangle {
        id: infoBarHSeparator2

        anchors.top: infoBar.top
        anchors.bottom: infoBar.bottom
        anchors.right: infoBarRateRatio.left
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        width: 2
        color: infoBar.lineColor
    }

    Text {
        id: infoBarRate

        anchors.top: infoBarHSeparator2.top
        anchors.bottom: infoBarVSeparator.top
        width: rateSlider.width
        anchors.right: infoBar.right
        anchors.rightMargin: 5
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: bpmControl.value.toFixed(2)
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: infoBar.textColor

        Mixxx.ControlProxy {
            id: bpmControl

            group: root.group
            key: "bpm"
        }

    }

    Text {
        id: infoBarRateRatio

        property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

        anchors.top: infoBarVSeparator.bottom
        anchors.bottom: infoBarHSeparator1.bottom
        width: rateSlider.width
        anchors.right: infoBar.right
        anchors.rightMargin: 5
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: infoBar.textColor
        text: (ratio > 0) ? "+" + ratio.toFixed(2) : ratio.toFixed(2)

        Mixxx.ControlProxy {
            id: rateRatioControl

            group: root.group
            key: "rate_ratio"
        }

    }

    gradient: Gradient {
        orientation: Gradient.Horizontal

        GradientStop {
            position: -0.75
            color: {
                let trackColor = root.deckPlayer.color;
                return trackColor ? trackColor : Theme.deckBackgroundColor;
            }
        }

        GradientStop {
            position: 0.5
            color: Theme.deckBackgroundColor
        }

    }

}
