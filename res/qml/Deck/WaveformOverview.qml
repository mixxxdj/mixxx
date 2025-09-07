import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
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
