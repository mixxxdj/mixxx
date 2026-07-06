import \".\" as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.12

ColumnLayout {
    spacing: 0

    Skin.SamplerRow {}
    Skin.SamplerRow {
        // Second row: samplers 9-16
        samplerOffset: 8
    }
}
