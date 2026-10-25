import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Skin.Fader {
    id: root

    required property string group
    required property string key

    value: control.parameter

    onMoved: control.parameter = value

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

    // Tap-to-seek + drag tracking for touchscreens: the base Fader only
    // tracks drags from the handle grab point, so a tap or a finger-slide
    // anywhere on the track does nothing. This overlays a touch area that
    // maps the touch position directly to the parameter while keeping the
    // double-tap reset (idea from JaviLabarumDJ/mixxx-android, adapted to
    // also work with the existing vertical/horizontal layout).
    MouseArea {
        id: touchArea
        anchors.fill: parent
        preventStealing: true

        function updateFromTouch(mouse) {
            var pos = (root.vertical !== undefined && root.vertical)
                    ? 1.0 - Math.max(0.0, Math.min(1.0, mouse.y / height))
                    : Math.max(0.0, Math.min(1.0, mouse.x / width));
            control.parameter = pos;
        }

        onPressed: mouse => updateFromTouch(mouse)
        onPositionChanged: mouse => {
            if (pressed) {
                updateFromTouch(mouse);
            }
        }
        onDoubleClicked: control.reset()
    }
}
