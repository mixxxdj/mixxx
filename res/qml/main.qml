import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Window 2.12
import "Theme"

ApplicationWindow {
    id: root

    readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property int designWidth: 1792
    readonly property int designHeight: 1008

    color: Theme.backgroundColor
    height: isMobile ? Screen.height : designHeight
    menuBar: content.item ? content.item.menuBar : null
    minimumHeight: isMobile ? 0 : 300
    minimumWidth: isMobile ? 0 : 680
    visible: true
    // Qt cascades a new window near the top-left of its screen and, on a
    // multi-monitor desktop, can leave the title bar above the screen edge
    // (observed at y = -45 on a 4K primary) so it can't be grabbed. Geometry
    // isn't persisted between launches, so rather than merely clamping the
    // cascade position, place the window deterministically: centred on its
    // screen, then clamped so the title bar is always reachable. Runs
    // deferred -- at Component.onCompleted the window manager hasn't
    // assigned the real screen/position yet.
    function clampOntoScreen() {
        if (root.isMobile)
            return; // fullscreen on mobile, no window placement needed
        // root.screen can still be null when this runs (observed: every pass
        // early-returned and Qt's cascade position stuck). Fall back to the
        // primary (virtual origin 0,0), then to the first screen.
        let s = root.screen;
        if (!s) {
            const all = Qt.application.screens;
            for (let i = 0; i < all.length; ++i) {
                if (all[i].virtualX === 0 && all[i].virtualY === 0) {
                    s = all[i];
                    break;
                }
            }
            if (!s && all.length > 0)
                s = all[0];
        }
        if (!s)
            return ;

        // NOTE: Screen.desktopAvailableWidth/Height is the size of the whole
        // VIRTUAL desktop (all monitors), not this screen. Using it here
        // centred the window on the multi-monitor union, landing it in the
        // primary's bottom-right corner (observed: 5120x2880 "available" on a
        // 2560x1440-logical screen). Screen.width/height is this screen only.
        const sw = s.width;
        const sh = s.height;

        root.width = Math.min(root.width, sw);
        root.height = Math.min(root.height, sh);

        // Centre on this screen.
        root.x = s.virtualX + Math.round((sw - root.width) / 2);
        root.y = s.virtualY + Math.round((sh - root.height) / 2);

        // Clamp inside this screen so the title bar is always reachable, even
        // if the window is as large as the screen.
        root.x = Math.min(Math.max(root.x, s.virtualX), s.virtualX + sw - root.width);
        root.y = Math.min(Math.max(root.y, s.virtualY), s.virtualY + sh - root.height);
    }

    Component.onCompleted: clampTimer.start()

    // Qt assigns the final screen and applies its own initial (cascade)
    // placement asynchronously, and on a multi-monitor mixed-DPI desktop that
    // can land AFTER a single early pass -- observed overwriting a 250ms
    // placement, leaving the window in the primary's bottom-right corner.
    // So re-place when the screen changes, and repeat the deferred pass a few
    // times over the first ~1.2s. Placement is deterministic (centre + clamp),
    // so repeated passes converge and the last one wins.
    onScreenChanged: root.clampOntoScreen()

    Timer {
        id: clampTimer

        property int passes: 0

        interval: 400
        repeat: true
        onTriggered: {
            root.clampOntoScreen();
            if (++passes >= 3)
                stop();
        }
    }

    width: isMobile ? Screen.width : designWidth

    function updateVisibility() {
        if (!Mixxx.Core.ready) {
            return;
        }
        root.visibility = Mixxx.Config.configStartInFullscreenKey || isMobile
                ? Window.FullScreen
                : Window.Windowed;
    }

    Connections {
        target: Mixxx.Core
        function onReadyChanged() {
            root.updateVisibility();
        }
    }

    Component.onCompleted: {
        clampTimer.start()
        root.updateVisibility()
    }

    Loader {
        id: content

        anchors.fill: parent

        active: Mixxx.Core.ready
        asynchronous: true
        onStatusChanged: {
            if (status === Loader.Error) {
                console.error("Failed to load the Mixxx main window")
                Qt.quit()
            }
        }
        sourceComponent: Component {
            MainWindow {
                applicationWindow: root
            }
        }
    }
    Rectangle {
        id: splash
        visible: opacity > 0
        color: Theme.backgroundColor
        anchors.fill: parent

        property bool ready: false

        Component.onCompleted: {
            ready = true
        }

        states: [
            State {
                when: splash.ready && content.status != Loader.Ready

                PropertyChanges {
                    text.opacity: 1
                    logo.opacity: 1
                    logo.y: root.height / 2 - logo.height / 2
                }
            },
            State {
                when: content.status == Loader.Ready && content.active

                PropertyChanges {
                    splash.opacity: 0
                }
            }
        ]
        Image {
            id: logo
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/images/mixxx-icon-logo-symbolic.svg"
            // height: 64
            opacity: 0
            y: root.height / 2

            Behavior on opacity {
                NumberAnimation { duration: 1500; easing.type: Easing.InOutQuad }
            }

            Behavior on y {
                NumberAnimation { duration: 1500; easing.type: Easing.InOutQuad }
            }
        }
        Text {
            id: text
            opacity: 0
            y: logo.y + logo.height*2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 20
            font.pixelSize: 12
            color: Theme.lightGray3
            text: "DJ your way"
            Behavior on opacity {
                SequentialAnimation {
                    PauseAnimation { duration: 1000 }
                    NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }
}
