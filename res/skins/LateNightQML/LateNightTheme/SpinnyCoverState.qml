pragma Singleton

import QtQml
import QtQuick
import Mixxx 1.0 as Mixxx

Item {
    visible: false

    readonly property bool showSpinnyOrCover: (showSpinniesProxy.value > 0 || showCoverArtProxy.value > 0)
            && (!showSpinnyOrCoverProxy.initialized || showSpinnyOrCoverProxy.value > 0)
    readonly property bool showSmallSpinnyOrCover: showSpinnyOrCover
            && (!showSmallSpinnyOrCoverProxy.initialized
                    ? selectBigSpinnyProxy.value <= 0
                    : showSmallSpinnyOrCoverProxy.value > 0)
    readonly property bool showBigSpinnyOrCover: showSpinnyOrCover
            && (!showBigSpinnyOrCoverProxy.initialized
                    ? selectBigSpinnyProxy.value > 0
                    : showBigSpinnyOrCoverProxy.value > 0)
    readonly property bool showCover: showSpinnyOrCover && showCoverArtProxy.value > 0
    readonly property bool showSpinny: showSpinnyOrCover && showSpinniesProxy.value > 0

    Mixxx.ControlProxy {
        id: showSpinniesProxy

        group: "[Skin]"
        key: "show_spinnies"
    }
    Mixxx.ControlProxy {
        id: showCoverArtProxy

        group: "[Skin]"
        key: "show_coverart"
    }
    Mixxx.ControlProxy {
        id: selectBigSpinnyProxy

        group: "[Skin]"
        key: "select_big_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSmallSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_small_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showBigSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_big_spinny_or_cover"
    }
}
