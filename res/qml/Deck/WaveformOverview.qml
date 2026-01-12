import QtQuick 2.12
import ".." as Skin
import "../Theme"

Rectangle {
    id: overview

    required property var currentTrack
    required property string group

    color: Theme.deckBackgroundColor
    height: 50
    radius: 5

    Skin.FadeBehavior on visible {
        fadeTarget: overview
    }

    Skin.WaveformOverview {
        anchors.fill: parent
        group: overview.group
    }
}
