import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Dialogs

MessageDialog {
    id: root

    buttons: MessageDialog.Ok

    Connections {
        function onLibraryScanSummaryAvailable(title, text, informativeText) {
            root.title = title;
            root.text = text;
            root.informativeText = informativeText;
            root.open();
        }

        target: Mixxx.Library
    }
}
