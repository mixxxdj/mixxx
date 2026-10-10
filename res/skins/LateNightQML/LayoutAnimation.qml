import QtQuick

NumberAnimation {
    id: root

    required property real targetValue
    property bool animationEnabled: true
    property real value: 0

    target: root
    property: "value"
    duration: 180
    easing.type: Easing.OutCubic

    onTargetValueChanged: {
        stop();
        if (animationEnabled) {
            from = value;
            to = targetValue;
            restart();
        } else
            value = targetValue;
    }
    onAnimationEnabledChanged: {
        if (!animationEnabled) {
            stop();
            value = targetValue;
        }
    }
    Component.onCompleted: {
        stop();
        value = targetValue;
    }
}
