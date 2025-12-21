import QtQuick 2.12
import ".." as Skin
import "../Theme"

Rectangle {
    id: overview

    required property string group
    required property var currentTrack

    radius: 5
    color: Theme.deckBackgroundColor
    height: 50

    Skin.WaveformOverview {
        group: root.group
        anchors.fill: parent
    }

    Skin.FadeBehavior on visible {
        fadeTarget: overview
    }
}
