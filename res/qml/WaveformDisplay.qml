import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import "Theme"

Item {
    id: root

    required property string group

    MixxxControls.WaveformDisplay {
        anchors.fill: parent
        group: root.group
    }
}
