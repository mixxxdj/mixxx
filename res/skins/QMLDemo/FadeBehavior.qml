import QtQuick 2.12

Behavior {
    id: root

    // Starting with QtQuick 2.15, we can use targetProperty.object here
    property Item fadeTarget

    SequentialAnimation {
        // If the opacity is 1, animate it down to 0
        NumberAnimation {
            target: root.fadeTarget
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InQuad
            duration: 150
        }

        // Actually change the property value (i.e. "visible")
        PropertyAction {
        }

        // If the opacity is 0, animate it up to 1
        NumberAnimation {
            target: root.fadeTarget
            from: 0
            to: 1
            property: "opacity"
            easing.type: Easing.OutQuad
            duration: 150
        }

    }

}
