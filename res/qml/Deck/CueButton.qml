import ".." as Skin
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Skin.ControlButton {
    id: root

    property bool minimized: false

    key: "cue_default"
    text: "Cue"
    activeColor: Theme.deckActiveColor
}
