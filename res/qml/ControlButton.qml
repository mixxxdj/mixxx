import "." as Skin

Skin.Button {
    id: root

    required property string group
    required property string key
    property bool toggleable: false

    function toggle() {
        controlBehavior.toggleControl();
    }

    highlight: controlBehavior.isActive
    onPressed: {
        controlBehavior.pressPrimary();
    }
    onReleased: {
        controlBehavior.releasePrimary();
    }

    ControlProxyButtonBehavior {
        id: controlBehavior

        group: root.group
        key: root.key
        toggleable: root.toggleable
        handlePointerInput: false
    }
}
