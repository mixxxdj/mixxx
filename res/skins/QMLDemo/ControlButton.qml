import "." as Skin
import Mixxx 0.1 as Mixxx

Skin.Button {
    id: root

    property string group // required
    property string key // required

    Mixxx.ControlProxy {
        group: root.group
        key: root.key
        value: root.checked || root.down
    }

}
