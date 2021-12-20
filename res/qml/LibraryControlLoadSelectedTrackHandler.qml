import Mixxx 0.1 as Mixxx
import QtQuick 2.12

Item {
    id: root

    property string group // required
    property bool enabled: true

    signal loadTrackRequested(bool play)

    Mixxx.ControlProxy {
        group: root.group
        key: "LoadSelectedTrack"
        onValueChanged: {
            if (value == 0 || !root.enabled)
                return ;

            root.loadTrackRequested(false);
        }
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "LoadSelectedTrackAndPlay"
        onValueChanged: {
            if (value == 0 || !root.enabled)
                return ;

            root.loadTrackRequested(true);
        }
    }

}
