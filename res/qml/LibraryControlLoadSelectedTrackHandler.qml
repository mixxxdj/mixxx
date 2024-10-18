import Mixxx 1.0 as Mixxx
import QtQuick 2.12

/// Usually, this component shouldn't be an (visual) `Item` and use something
/// like `QtObject` instead. However, for some reason using `QtObject` here
/// makes Mixxx crash on load (using Qt 5.15.2+kde+r43-1). We can check if this
/// is fixed upstream once we switch to Qt 6.
Item {
    id: root

    required property string group
    property bool enabled: true

    signal loadTrackRequested(bool play)

    Mixxx.ControlProxy {
        group: root.group
        key: "LoadSelectedTrack"
        onValueChanged: (value) => {
            if (value == 0 || !root.enabled)
                return ;

            root.loadTrackRequested(false);
        }
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "LoadSelectedTrackAndPlay"
        onValueChanged: (value) => {
            if (value == 0 || !root.enabled)
                return ;

            root.loadTrackRequested(true);
        }
    }
}
