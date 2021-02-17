import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: window
    width: 34
    height: 100
    visible: true

    MixxxSlider {
        width: window.width
        height: window.height
        group: "[Channel1]"
        key: "volume"
    }
}
