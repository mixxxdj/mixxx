import QtQuick 2.12

Slider {
    id: root

    enum SnapMode {
        NoSnap,
        SnapAlways,
        SnapOnRelease
    }

    orientation: Qt.Vertical
    wheelEnabled: true
}
