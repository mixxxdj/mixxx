import "." as Skin

Skin.ControlProxyButtonBehavior {
    id: root

    required property string cueType

    key: root.cueType + "_activate"
    rightClickKey: root.cueType + "_clear"
    displayKey: root.cueType + "_enabled"
}
