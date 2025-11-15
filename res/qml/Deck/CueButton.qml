import ".." as Skin
import "../Theme"

Skin.ControlButton {
    id: root

    property bool minimized: false

    key: "cue_default"
    text: "Cue"
    activeColor: Theme.deckActiveColor
}
