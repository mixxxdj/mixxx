import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Window 2.12
import "Theme"

ApplicationWindow {
    id: root
    objectName: "mainWindow"

    readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property int designWidth: 1792
    readonly property int designHeight: 1008

    // Used to show click interaction on the Window. Mainly relevant on automated testing
    property bool enableDiagnosticClick: false

    color: Theme.backgroundColor
    height: isMobile ? Screen.height : designHeight
    minimumHeight: isMobile ? 0 : 300
    minimumWidth: isMobile ? 0 : 680
    visible: true
    width: isMobile ? Screen.width : designWidth

    Connections {
        target: Mixxx.Core
        function onReadyChanged(){
            root.visibility = Mixxx.Config.configStartInFullscreenKey || isMobile ? Window.FullScreen : Window.Windowed
        }
    }

    Component.onCompleted: {
        Mixxx.Core.clearOpenedPopup()
    }

    Loader {
        id: content

        anchors.fill: parent

        active: Mixxx.Core.ready
        asynchronous: true
        sourceComponent: Component {
            MainWindow {
                enableDiagnosticClick: root.enableDiagnosticClick
            }
        }

    }
    Rectangle {
        id: splash
        objectName: "splashScreen"
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
    // Testing automation resources
    Rectangle {
        id: click
        color: clickDetector.clicked ? 'red' : 'green'
        radius: width / 2
        visible: opacity != 0 && root.enableDiagnosticClick

        NumberAnimation on width {
            running: clickDetector.clicked
            from: 20; to: 0
        }
        NumberAnimation on height {
            running: clickDetector.clicked
            from: 20; to: 0
        }
        NumberAnimation on opacity {
            running: clickDetector.clicked
            from: 1; to: 0
        }
    }
    MouseArea {
        property bool clicked: false
        enabled: root.enableDiagnosticClick
        id: clickDetector
        anchors.fill: parent
        onPressed: (mouse)=> {
            clicked = false
            click.width = 20
            click.height = 20
            click.opacity = 1
            click.x = Qt.binding(() => mouse.x - click.width / 2);
            click.y = Qt.binding(() => mouse.y - click.height / 2);
            clicked = true
            mouse.accepted = false
        }
    }
}
