import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.12

RowLayout {
    spacing: 0

    Repeater {
        model: 8

        Skin.Sampler {
            required property int index

            Layout.fillWidth: true
            group: "[Sampler" + (index + 1) + "]"
        }
    }
}
