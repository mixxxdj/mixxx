/*
This module is used to define the top left section, right under the label.
Currently this section is dedicated to key/pitch information.
*/
import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Rectangle {
    id: root

    required property string group

    enum Key {
        NoKey,
        OneD,
        EightD,
        ThreeD,
        TenD,
        FiveD,
        TwelveD,
        SevenD,
        SecondD,
        NineD,
        FourD,
        ElevenD,
        SixD,
        TenM,
        FiveM,
        TwelveM,
        SevenM,
        TwoM,
        NineM,
        FourM,
        ElevenM,
        SixM,
        OneM,
        EightM,
        ThreeM
    }

    property variant colorsMap: [
                                 "#b09840", // No key
                                 "#b960a2",// 1d
                                 "#9fc516", // 8d
                                 "#527fc0", // 3d
                                 "#f28b2e", // 10d
                                 "#5bc1cf", // 5d
                                 "#e84c4d", // 12d
                                 "#73b629", // 7d
                                 "#8269ab", // 2d
                                 "#fdd615", // 9d
                                 "#3cc0f0", // 4d
                                 "#4cb686", // 11d
                                 "#4cb686", // 6d
                                 "#f5a158", // 10m
                                 "#7bcdd9", // 5m
                                 "#ed7171", // 12m
                                 "#8fc555", // 7m
                                 "#9b86be", // 2m
                                 "#fcdf45", // 9m
                                 "#63cdf4", // 4m
                                 "#f1845f", // 11m
                                 "#70c4a0", // 6m
                                 "#c680b6", // 1m
                                 "#b2d145", // 8m
                                 "#7499cd"  // 3m
    ]

    property variant textMap: [
                               "No key",
                               "1d",
                               "8d",
                               "3d",
                               "10d",
                               "5d",
                               "12d",
                               "7d",
                               "2d",
                               "9d",
                               "4d",
                               "11d",
                               "6d",
                               "10m",
                               "5m",
                               "12m",
                               "7m",
                               "2m",
                               "9m",
                               "4m",
                               "11m",
                               "6m",
                               "1m",
                               "8m",
                               "3m"
    ]

    Mixxx.ControlProxy {
        id: keyProxy
        group: root.group
        key: "key"
    }

    required property color borderColor

    readonly property int key: keyProxy.value > 0 ? keyProxy.value : KeyIndicator.Key.NoKey

    radius: 6
    border.color: colorsMap[key]
    border.width: 2

    color: colorsMap[key]

    Text {
        text: textMap[key]
        font.pixelSize: 17
        color: fontColor
        anchors.centerIn: parent
    }
}
