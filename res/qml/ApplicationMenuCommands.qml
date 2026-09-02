import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window

Item {
    id: root

    required property ApplicationWindow applicationWindow
    readonly property bool fullScreen: applicationWindow.visibility === Window.FullScreen
    property int pendingDeck: 1

    signal showDeveloperToolsRequested

    function loadTrackToDeck(deck) {
        pendingDeck = deck;
        const playControl = [deck1PlayControl, deck2PlayControl, deck3PlayControl, deck4PlayControl][deck - 1];
        if (playControl && playControl.value > 0) {
            confirmLoadDialog.text = qsTr("Deck %1 is currently playing a track.\nAre you sure you want to load a new track?").arg(deck);
            confirmLoadDialog.open();
        } else {
            openTrackFileDialog();
        }
    }
    function openTrackFileDialog() {
        trackFileDialog.title = qsTr("Load track to Deck %1").arg(pendingDeck);
        trackFileDialog.currentFolder = Mixxx.PlayerManager.initialTrackDirectoryUrl;
        Qt.callLater(function() {
            trackFileDialog.open();
        });
    }
    function toggleFullScreen() {
        applicationWindow.visibility = applicationWindow.visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen;
    }
    function showAbout() {
        aboutDialog.open();
    }
    function showKeywheel() {
        keywheelDialog.open();
    }

    FileDialog {
        id: trackFileDialog

        fileMode: FileDialog.OpenFile
        nameFilters: Mixxx.PlayerManager.supportedAudioFileNameFilters

        onAccepted: Mixxx.PlayerManager.loadLocationUrlToDeck(selectedFile, root.pendingDeck)
    }
    MessageDialog {
        id: confirmLoadDialog

        buttons: MessageDialog.Yes | MessageDialog.No
        title: Mixxx.Application.applicationName

        onAccepted: root.openTrackFileDialog()
    }
    AboutDialog {
        id: aboutDialog
    }
    Dialog {
        id: keywheelDialog

        height: Math.min(620, root.applicationWindow.height - 80)
        modal: false
        standardButtons: Dialog.Close
        title: qsTr("Keywheel")
        width: height
        x: Math.round((root.applicationWindow.width - width) / 2)
        y: Math.round((root.applicationWindow.height - height) / 2)

        contentItem: Image {
            fillMode: Image.PreserveAspectFit
            source: "../images/keywheel/keywheel.svg"
        }
    }
    Mixxx.ControlProxy {
        id: deck1PlayControl

        group: "[Channel1]"
        key: "play"
    }
    Mixxx.ControlProxy {
        id: deck2PlayControl

        group: "[Channel2]"
        key: "play"
    }
    Mixxx.ControlProxy {
        id: deck3PlayControl

        group: "[Channel3]"
        key: "play"
    }
    Mixxx.ControlProxy {
        id: deck4PlayControl

        group: "[Channel4]"
        key: "play"
    }
}
