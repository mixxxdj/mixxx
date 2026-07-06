import "../Controls" as Controls
import "../Theme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property alias editDeck: editDeckButton.checked
    property alias maximizeLibrary: maximizeLibraryButton.checked
    readonly property bool show4decks: show4DecksButton.checked && show4DecksButton.visible
    property bool show4decksAvailable: true
    property alias showEffects: showEffectsButton.checked
    property alias showSamplers: showSamplersButton.checked
    property bool settingsOpened: false

    signal openSettingsRequested

    color: Theme.toolbarBackgroundColor
    height: 36
    radius: 1

    RowLayout {
        anchors.fill: parent

        Controls.ToggleButton {
            id: show4DecksButton

            activeColor: Theme.toolbarActiveColor
            text: "4 Decks"
            visible: root.show4decksAvailable
        }
        Controls.ToggleButton {
            id: maximizeLibraryButton

            activeColor: Theme.toolbarActiveColor
            text: "Library"

            onCheckedChanged: () => {
                showMaximizedLibrary.value = checked;
            }

            Mixxx.ControlProxy {
                id: showMaximizedLibrary

                group: "[Skin]"
                key: "show_maximized_library"

                onValueChanged: () => {
                    maximizeLibraryButton.checked = value;
                }
            }
        }
        Controls.ToggleButton {
            id: showEffectsButton

            activeColor: Theme.toolbarActiveColor
            text: "Effects"
        }
        Controls.ToggleButton {
            id: showAuxButton

            activeColor: Theme.toolbarActiveColor
            text: "Aux"
        }
        Controls.ToggleButton {
            id: showSamplersButton

            activeColor: Theme.toolbarActiveColor
            text: "Sampler"
        }
        Item {
            Layout.fillWidth: true
        }
        Controls.ToggleButton {
            id: editDeckButton

            activeColor: Theme.toolbarActiveColor
            text: "Edit"
        }
        Controls.ToggleButton {
            id: showDevToolsButton

            activeColor: Theme.toolbarActiveColor
            checked: devToolsWindow.visible
            text: "Develop"

            onClicked: {
                if (devToolsWindow.visible) {
                    devToolsWindow.close();
                } else {
                    devToolsWindow.show();
                }
            }

            Shared.DeveloperToolsWindow {
                id: devToolsWindow

                height: 480
                width: 640
            }
        }
        Controls.Button {
            id: showPreferencesButton

            activeColor: Theme.toolbarActiveColor
            checked: root.settingsOpened
            icon.height: 16
            icon.source: Theme.sharedImage("gear.svg")
            icon.width: 16
            implicitWidth: implicitHeight

            onClicked: {
                if (!root.settingsOpened) {
                    root.openSettingsRequested();
                }
            }
            onPressAndHold: {
                Mixxx.PreferencesDialog.show();
            }
        }
    }
}
