import \".\" as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.12

RowLayout {
    spacing: 0

    // Offset to support multiple rows: first row has offset 0 (samplers 1-8),
    // second row has offset 8 (samplers 9-16)
    property int samplerOffset: 0

    Repeater {
        model: 8

        Skin.Sampler {
            required property int index

            Layout.fillWidth: true
            group: "[Sampler" + (index + 1 + samplerOffset) + "]"
        }
    }
}