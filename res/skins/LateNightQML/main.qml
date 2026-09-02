import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: root

    property int displayedProgress: 0

    color: startupScreen.backgroundColor
    height: 1008
    minimumHeight: 668
    minimumWidth: 1280
    visible: true
    width: 1792

    function updateProgress() {
        if (!Mixxx.Core.ready) {
            displayedProgress = Math.max(displayedProgress,
                                         Mixxx.Core.initializationProgress);
        } else if (mainWindowLoader.status === Loader.Ready) {
            displayedProgress = 100;
        } else {
            displayedProgress = Math.max(displayedProgress,
                                         65 + Math.round(mainWindowLoader.progress * 34));
        }
    }

    Connections {
        target: Mixxx.Core

        function onInitializationProgressChanged() {
            root.updateProgress();
        }
        function onReadyChanged() {
            root.updateProgress();
        }
    }

    Loader {
        id: mainWindowLoader

        anchors.fill: parent
        active: Mixxx.Core.ready
        asynchronous: true

        onProgressChanged: root.updateProgress()
        onStatusChanged: root.updateProgress()

        sourceComponent: Component {
            MainWindow {
                anchors.fill: parent
            }
        }
    }

    StartupScreen {
        id: startupScreen

        anchors.fill: parent
        opacity: mainWindowLoader.status === Loader.Ready ? 0 : 1
        progress: root.displayedProgress
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutQuad
            }
        }
    }

    Component.onCompleted: updateProgress()
}
