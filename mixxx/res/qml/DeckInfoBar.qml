import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    required property string group
    required property int rightColumnWidth
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property var currentTrack: deckPlayer.currentTrack
    property color lineColor: Theme.deckLineColor

    border.width: 2
    border.color: Theme.deckBackgroundColor
    radius: 5
    height: 56

    Image {
        id: coverArt

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: height
        source: root.currentTrack.coverArtUrl
        visible: false
        asynchronous: true
    }

    Rectangle {
        id: coverArtCircle

        anchors.fill: coverArt
        radius: height / 2
        visible: false
    }

    OpacityMask {
        anchors.fill: coverArt
        source: coverArt
        maskSource: coverArtCircle
    }

    Rectangle {
        id: spinnyCircle

        anchors.fill: coverArt
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
            onValueChanged: (value) => {
                spinnyIndicator.indicatorVisible = (value > 0);
            }
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

    Skin.EmbeddedText {
        id: infoBarTitle

        text: root.currentTrack.title
        anchors.top: infoBarHSeparator1.top
        anchors.left: infoBarVSeparator.left
        anchors.right: infoBarHSeparator1.left
        anchors.bottom: infoBarVSeparator.bottom
        horizontalAlignment: Text.AlignLeft
        font.bold: false
        font.pixelSize: Theme.textFontPixelSize
    }

    Rectangle {
        id: infoBarVSeparator

        anchors.left: coverArt.right
        anchors.right: root.right
        anchors.verticalCenter: root.verticalCenter
        anchors.margins: 5
        height: 2
        color: root.lineColor
    }

    Skin.EmbeddedText {
        id: infoBarArtist

        text: root.currentTrack.artist
        anchors.top: infoBarVSeparator.bottom
        anchors.left: infoBarVSeparator.left
        anchors.right: infoBarHSeparator1.left
        anchors.bottom: infoBarHSeparator1.bottom
        horizontalAlignment: Text.AlignLeft
        font.bold: false
        font.pixelSize: Theme.textFontPixelSize
    }

    Rectangle {
        id: infoBarHSeparator1

        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.right: infoBarKey.left
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        width: 2
        color: root.lineColor
    }

    Skin.EmbeddedText {
        id: infoBarKey

        text: root.currentTrack.keyText
        anchors.top: infoBarHSeparator1.top
        anchors.bottom: infoBarVSeparator.top
        anchors.right: infoBarHSeparator2.left
        width: root.rightColumnWidth
    }

    Rectangle {
        id: infoBarHSeparator2

        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.right: infoBarRateRatio.left
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        width: 2
        color: root.lineColor
    }

    Skin.EmbeddedText {
        id: infoBarRate

        anchors.top: infoBarHSeparator2.top
        anchors.bottom: infoBarVSeparator.top
        anchors.right: root.right
        anchors.rightMargin: 5
        width: root.rightColumnWidth

        Mixxx.ControlProxy {
            id: bpmControl

            group: root.group
            key: "bpm"
        }
    }

    Skin.EmbeddedText {
        id: infoBarRateRatio

        property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

        anchors.top: infoBarVSeparator.bottom
        anchors.bottom: infoBarHSeparator1.bottom
        width: root.rightColumnWidth
        anchors.right: root.right
        anchors.rightMargin: 5
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
            position: 0
            color: {
                const trackColor = root.currentTrack.color;
                if (!trackColor.valid)
                    return Theme.deckBackgroundColor;

                return Qt.darker(root.currentTrack.color, 2);
            }
        }

        GradientStop {
            position: 1
            color: Theme.deckBackgroundColor
        }
    }
}
