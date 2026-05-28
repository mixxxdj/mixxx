import ".." as Skin
import "../Theme"

Skin.ControlButton {
    id: root

    property bool minimized: false

    activeColor: Theme.deckActiveColor
    key: "cue_default"
    text: "Cue"
}
