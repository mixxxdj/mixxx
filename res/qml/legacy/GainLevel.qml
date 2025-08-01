import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E

ColumnLayout {
    spacing: 0

    E.Dial {
        from: -1
        to: 1
        color: "#c06020"
        Layout.alignment: Qt.AlignHCenter
    }
    E.Slider {
        orientation: Qt.Vertical
        backgroundSource: "image://svgmodifier/sliders/level/background.svg"
        handleSource: "image://svgmodifier/sliders/level/handle.svg"
    }
}
