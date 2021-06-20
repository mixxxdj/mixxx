import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12

Row {
    id: root

    Skin.EffectUnit {
        id: effectUnit1

        width: root.width / 2
        unitNumber: 1
    }

    Skin.EffectUnit {
        id: effectUnit2

        width: root.width / 2
        unitNumber: 2
    }

}
