import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import QtCore
import Qt5Compat.GraphicalEffects
import Mixxx 1.0 as Mixxx
import "." as Setting
import "." as SettingComponents
import ".." as Skin
import "../Theme"

Category {
    id: root

    property bool dirty: false

    function load() {
        loadSources();
        integrationRepeater.model = [
            {
                service: "Rhythmbox",
                enabled: Mixxx.Config.libraryRhythmboxEnabled
            },
            {
                service: "Banshee",
                enabled: Mixxx.Config.libraryBansheeEnabled
            },
            {
                service: "iTunes",
                enabled: Mixxx.Config.libraryITunesEnabled
            },
            {
                service: "Traktor",
                enabled: Mixxx.Config.libraryTraktorEnabled
            },
            {
                service: "Rekordbox",
                enabled: Mixxx.Config.libraryRekordboxEnabled
            },
            {
                service: "Serato",
                enabled: Mixxx.Config.librarySeratoEnabled
            },
        ];
        metadataSynchroniseEnabled.selected = Mixxx.Config.librarySyncTrackMetadataExport ? "on" : "off";
        metadataSynchroniseSeratoEnabled.selected = Mixxx.Config.librarySeratoMetadataExport ? "on" : "off";
        metadataRelativePathEnabled.selected = Mixxx.Config.libraryUseRelativePathOnExport ? "on" : "off";
        searchCompletionEnabled.selected = Mixxx.Config.librarySearchCompletionsEnable ? "on" : "off";
        searchHistoryKeyboardEnabled.selected = Mixxx.Config.librarySearchHistoryShortcutsEnable ? "on" : "off";
        historyDuplicateDistanceInput.value = Mixxx.Config.libraryHistoryTrackDuplicateDistance;
        historyDeleteLessThanInput.value = Mixxx.Config.libraryHistoryMinTracksToKeep;
        searchAsYouTypeTimeoutInput.setValue(Mixxx.Config.librarySearchDebouncingTimeout / 1000);
        pitchSliderFuzzBPMInput.setValue(Mixxx.Config.librarySearchBpmFuzzyRange);
        root.dirty = false;
        errorMessage.text = "";
    }
    function loadSources() {
        let rootDirs = [];
        for (let source of Object.values(Mixxx.Library.sources)) {
            rootDirs.push({
                path: source.path,
                trackCount: source.trackCount,
                totalMinute: Math.round(source.totalSecond / 60)
            });
        }
        sourceListView.model = rootDirs;
    }
    function reset() {
    }
    function save() {
        let requestedDirs = [...sourceListView.model];
        let changed = false;

        for (let source of requestedDirs) {
            if (source.trackCount === undefined) {
                // Handle addition
                switch (Mixxx.Library.addSource(source.path)) {
                case Mixxx.Library.AddResult.AlreadyWatching:
                    errorMessage.text = qsTr("This or a parent directory is already in your library.");
                    return;
                case Mixxx.Library.AddResult.InvalidOrMissingDirectory:
                    errorMessage.text = qsTr("This or a listed directory does not exist or is inaccessible.\nAborting the operation to avoid library inconsistencies");
                    return;
                case Mixxx.Library.AddResult.UnreadableDirectory:
                    errorMessage.text = qsTr("This directory can not be read.");
                    return;
                case Mixxx.Library.AddResult.SqlError:
                    errorMessage.text = qsTr("An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                    return;
                }
            } else if (source.deleting !== undefined) {
                // Handle removal
                switch (Mixxx.Library.removeSource(source.path, source.deleting)) {
                case Mixxx.Library.RemoveResult.NotFound:
                    errorMessage.text = qsTr("This directory can not be found.");
                    return;
                case Mixxx.Library.RemoveResult.SqlError:
                    errorMessage.text = qsTr("An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                    return;
                }
            } else if (source.relink) {
                // Handle relinking
                switch (Mixxx.Library.relinkSource(source.path, source.relink)) {
                case Mixxx.Library.RelocateResult.InvalidOrMissingDirectory:
                    errorMessage.text = qsTr("This or a listed directory does not exist or is inaccessible.\nAborting the operation to avoid library inconsistencies");
                    return;
                case Mixxx.Library.RelocateResult.UnreadableDirectory:
                    errorMessage.text = qsTr("This directory can not be read.");
                    return;
                case Mixxx.Library.RelocateResult.SqlError:
                    errorMessage.text = qsTr("An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                    return;
                }
            } else {
                continue;
            }
            changed = true;
        }

        if (changed) {
            Mixxx.Library.scanner.start();
        }

        let integrations = {};
        for (let i = 0; i < integrationRepeater.count; i++) {
            integrations[integrationRepeater.itemAt(i).modelData.service] = integrationRepeater.itemAt(i).enabled;
        }
        Mixxx.Config.libraryRhythmboxEnabled = integrations.Rhythmbox;
        Mixxx.Config.libraryBansheeEnabled = integrations.Banshee;
        Mixxx.Config.libraryITunesEnabled = integrations.iTunes;
        Mixxx.Config.libraryTraktorEnabled = integrations.Traktor;
        Mixxx.Config.libraryRekordboxEnabled = integrations.Rekordbox;
        Mixxx.Config.librarySeratoEnabled = integrations.Serato;
        Mixxx.Config.librarySyncTrackMetadataExport = metadataSynchroniseEnabled.enabled;
        Mixxx.Config.librarySeratoMetadataExport = metadataSynchroniseSeratoEnabled.enabled;
        Mixxx.Config.libraryUseRelativePathOnExport = metadataRelativePathEnabled.enabled;
        Mixxx.Config.librarySearchCompletionsEnable = searchCompletionEnabled.enabled;
        Mixxx.Config.librarySearchHistoryShortcutsEnable = searchHistoryKeyboardEnabled.enabled;
        Mixxx.Config.libraryHistoryTrackDuplicateDistance = historyDuplicateDistanceInput.value;
        Mixxx.Config.libraryHistoryMinTracksToKeep = historyDeleteLessThanInput.value;
        Mixxx.Config.librarySearchDebouncingTimeout = searchAsYouTypeTimeoutInput.value * 1000;
        Mixxx.Config.librarySearchBpmFuzzyRange = pitchSliderFuzzBPMInput.value;
        load();
    }

    label: qsTr("Library")

    Component.onCompleted: {
        root.load();
    }

    ScrollView {
        id: scrollView

        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        ColumnLayout {
            width: root.width - 10

            RowLayout {
                Text {
                    Layout.bottomMargin: 14
                    Layout.leftMargin: 17
                    color: Theme.white
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    text: qsTr("Sources")
                }
            }
            Mixxx.SettingGroup {
                Layout.bottomMargin: 6
                anchors.left: parent.left
                anchors.right: parent.right
                implicitHeight: sources.implicitHeight
                label: qsTr("Sources")

                GridLayout {
                    id: sources

                    columnSpacing: 20
                    columns: width > 800 ? 2 : 1
                    width: parent.width

                    Rectangle {
                        Layout.fillWidth: true
                        // anchors.left: parent.left
                        // anchors.right: parent.right
                        Layout.minimumWidth: sourcePane.implicitWidth
                        color: '#0E0E0E'
                        implicitHeight: sourcePane.implicitHeight + 20

                        ColumnLayout {
                            id: sourcePane

                            anchors.fill: parent
                            spacing: 0

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.leftMargin: 8
                                Layout.rightMargin: 8
                                Layout.topMargin: 10

                                Text {
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 17
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    text: qsTr('Music Directory')
                                }
                                FolderDialog {
                                    id: addDialog

                                    currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
                                    title: qsTr("Choose a music directory")

                                    onAccepted: {
                                        let model = sourceListView.model;
                                        let path = addDialog.selectedFolder.toString();
                                        path = path.replace(/^(file:\/\/)/, ""); // FIXME does this work on Windows ?
                                        model.push({
                                            path: decodeURIComponent(path)
                                        });
                                        root.dirty = true;
                                        sourceListView.model = model;
                                    }
                                }
                                SettingComponents.FormButton {
                                    activeColor: "#999999"
                                    backgroundColor: "#3F3F3F"
                                    opacity: enabled ? 1.0 : 0.5
                                    text: qsTr("Add")

                                    onPressed: addDialog.open()
                                }
                            }
                            ListView {
                                id: sourceListView

                                Layout.fillWidth: true
                                Layout.preferredHeight: 240
                                clip: true
                                focus: true
                                model: []

                                // anchors.fill: parent
                                // anchors.topMargin: 10
                                // anchors.bottomMargin: 10
                                // anchors.leftMargin: 17
                                // anchors.rightMargin: 17

                                delegate: MouseArea {
                                    id: mouse

                                    required property int index
                                    required property var modelData
                                    readonly property bool selected: ListView.view.currentIndex == index && modelData.deleting === undefined && !modelData.relink && modelData.trackCount !== undefined

                                    height: 34
                                    hoverEnabled: true
                                    width: ListView.view.width

                                    onPressed: {
                                        ListView.view.currentIndex = index;
                                    }
                                    onSelectedChanged: {
                                        removeButton.confirming = false;
                                    }

                                    Timer {
                                        id: cancelDeletion

                                        interval: 1000
                                        running: removeButton.confirming && !mouse.containsMouse

                                        onTriggered: {
                                            removeButton.confirming = false;
                                        }
                                    }
                                    Rectangle {
                                        anchors.fill: parent
                                        color: index % 2 == 0 ? '#3F3F3F' : '#2B2B2B'

                                        // width: 180; height: 40
                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 8
                                            anchors.rightMargin: 8

                                            Text {
                                                text: qsTr("Icon")
                                                width: 160
                                            }
                                            Text {
                                                Layout.fillWidth: true
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                opacity: modelData.deleting !== undefined ? 0.4 : 1
                                                text: modelData.path
                                            }
                                            FolderDialog {
                                                id: relinkDialog

                                                currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
                                                title: qsTr("Relink music directory to new location")

                                                onAccepted: {
                                                    let model = sourceListView.model;
                                                    let path = selectedFolder.toString().replace(/^(file:\/\/)/, ""); // FIXME does this work on Windows ?
                                                    model[mouse.index].relink = decodeURIComponent(path);
                                                    root.dirty = true;
                                                    sourceListView.model = model;
                                                }
                                            }
                                            SettingComponents.FormButton {
                                                id: relinkButton

                                                activeColor: "#999999"
                                                backgroundColor: "#3F3F3F"
                                                opacity: enabled ? 1.0 : 0.5
                                                text: modelData.relink ? qsTr("Save to proceed") : qsTr("Relink")
                                                visible: selected && modelData.trackCount !== undefined && !Mixxx.Library.scanner.running

                                                onPressed: {
                                                    relinkDialog.open();
                                                }
                                            }
                                            Item {
                                                id: removeButton

                                                property bool confirming: false

                                                clip: true
                                                implicitHeight: Math.max(actionButton.implicitHeight, removeModeSelector.implicitHeight)
                                                implicitWidth: removeButton.confirming ? removeModeSelector.implicitWidth : actionButton.implicitWidth
                                                visible: selected && !Mixxx.Library.scanner.running

                                                Behavior on implicitWidth {
                                                    PropertyAnimation {
                                                    }
                                                }

                                                SettingComponents.FormButton {
                                                    id: actionButton

                                                    activeColor: "#999999"
                                                    anchors.centerIn: parent
                                                    backgroundColor: "#7D3B3B"
                                                    opacity: enabled ? 1.0 : 0.5
                                                    text: qsTr("Remove")
                                                    visible: !removeButton.confirming

                                                    onPressed: {
                                                        if (modelData.trackCount == 0) {
                                                            let model = sourceListView.model;
                                                            model[mouse.index].deleting = Mixxx.Library.PurgeTracks;
                                                            root.dirty = true;
                                                            sourceListView.model = model;
                                                        } else {
                                                            removeButton.confirming = !removeButton.confirming;
                                                        }
                                                    }
                                                }
                                                RatioChoice {
                                                    id: removeModeSelector

                                                    anchors.centerIn: parent
                                                    inactiveColor: Theme.darkGray4
                                                    normalizedWidth: false
                                                    options: ["keep tracks", "hide tracks", "purge tracks"]
                                                    selected: null
                                                    visible: removeButton.confirming

                                                    onSelectedChanged: {
                                                        let model = sourceListView.model;
                                                        switch (options.indexOf(selected)) {
                                                        case 0:
                                                            model[mouse.index].deleting = Mixxx.Library.KeepTracks;
                                                            break;
                                                        case 1:
                                                            model[mouse.index].deleting = Mixxx.Library.HideTracks;
                                                            break;
                                                        case 2:
                                                            model[mouse.index].deleting = Mixxx.Library.PurgeTracks;
                                                            break;
                                                        default:
                                                            console.warn(`unknown value deletion mode ${selected}. Ignoring.`);
                                                            return;
                                                        }
                                                        root.dirty = true;
                                                        sourceListView.model = model;
                                                    }
                                                }
                                            }
                                            Text {
                                                color: '#626262'
                                                font.italic: modelData.trackCount === undefined || modelData.totalMinute === undefined || !!modelData.relink
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                text: {
                                                    if (modelData.relink) {
                                                        return qsTr(`Save to relink to ${modelData.relink}`);
                                                    } else if (modelData.deleting !== undefined) {
                                                        let action = {
                                                            0: qsTr("keep already imported tracks"),
                                                            1: qsTr("hide imported tracks"),
                                                            2: qsTr("purge imported tracks")
                                                        }[modelData.deleting] || 'perform an unknown action';
                                                        return qsTr(`Save to delete and ${action}`);
                                                    } else if (modelData.trackCount !== undefined && modelData.totalMinute !== undefined) {
                                                        return qsTr(`${modelData.trackCount} tracks, ${modelData.totalMinute} minutes`);
                                                    } else {
                                                        return qsTr('Save to import new folder');
                                                    }
                                                }
                                                visible: !selected && !Mixxx.Library.scanner.running
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Rectangle {
                            anchors.fill: parent
                            color: Qt.alpha('black', 0.4)
                            visible: Mixxx.Library.scanner.running

                            MouseArea {
                                anchors.fill: parent
                            }
                            Text {
                                id: scanProgress

                                color: Theme.white
                                font.pixelSize: 14
                                text: qsTr("File scanning...")

                                anchors {
                                    bottom: parent.bottom
                                    left: parent.left
                                    margins: 5
                                }
                                Connections {
                                    function onProgress(currentPath) {
                                        scanProgress.text = qsTr(`File scanning: ${currentPath}`);
                                    }

                                    target: Mixxx.Library.scanner
                                }
                            }
                            SettingComponents.FormButton {
                                activeColor: "#999999"
                                backgroundColor: Mixxx.Library.scanner.cancelling ? "#999999" : "#3a60be"
                                enabled: !Mixxx.Library.scanner.cancelling
                                text: qsTr("Cancel")

                                onPressed: {
                                    Mixxx.Library.scanner.cancel();
                                }

                                anchors {
                                    bottom: parent.bottom
                                    margins: 5
                                    right: parent.right
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        Rectangle {
                            Layout.preferredWidth: root.width * (sources.columns == 2 ? 0.35 : 1)
                            anchors.left: parent.left
                            anchors.right: parent.right
                            color: Theme.darkGray2
                            implicitHeight: integrationPane.implicitHeight + 20

                            GridLayout {
                                id: integrationPane

                                anchors.bottomMargin: 10
                                anchors.fill: parent
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17
                                anchors.topMargin: 10
                                columns: sources.columns == 2 ? 1 : 2

                                Repeater {
                                    id: integrationRepeater

                                    RowLayout {
                                        property alias enabled: integrationEnabled.enabled
                                        required property var modelData

                                        Layout.preferredWidth: sourcePane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: modelData.service

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: integrationEnabled

                                            readonly property bool enabled: selected == "on"

                                            inactiveColor: Theme.darkGray4
                                            maxWidth: parent.width * 0.5
                                            options: ["on", "off"]
                                            selected: modelData.enabled ? "on" : "off"

                                            onSelectedChanged: root.dirty = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                Layout.bottomMargin: 6
                Layout.topMargin: 40
                anchors.left: parent.left
                anchors.right: parent.right
                implicitHeight: metadataColumn.height
                label: qsTr("Metadata")

                Column {
                    id: metadataColumn

                    anchors.left: parent.left
                    anchors.right: parent.right

                    Text {
                        color: Theme.white
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        text: qsTr("Metadata")
                    }
                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 10
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        color: Theme.darkGray2
                        implicitHeight: metadataPane.implicitHeight + 20

                        GridLayout {
                            id: metadataPane

                            anchors.bottomMargin: 10
                            anchors.fill: parent
                            anchors.leftMargin: 17
                            anchors.rightMargin: 17
                            anchors.topMargin: 10
                            columnSpacing: 20
                            columns: metadataColumn.width < 850 ? 1 : 2
                            rowSpacing: 15

                            RowLayout {
                                Layout.preferredWidth: metadataPane.width / metadataPane.columns

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Synchronise metadata with file")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                RatioChoice {
                                    id: metadataSynchroniseEnabled

                                    readonly property bool enabled: selected == "on"

                                    inactiveColor: Theme.darkGray4
                                    maxWidth: parent.width * 0.5
                                    options: ["on", "off"]

                                    onSelectedChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: metadataPane.width / metadataPane.columns

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Synchronise metadata with Serato library")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                RatioChoice {
                                    id: metadataSynchroniseSeratoEnabled

                                    readonly property bool enabled: selected == "on"

                                    inactiveColor: Theme.darkGray4
                                    maxWidth: parent.width * 0.5
                                    options: ["on", "off"]

                                    onSelectedChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: metadataPane.width / metadataPane.columns

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Prefer relative path on playlist export")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                RatioChoice {
                                    id: metadataRelativePathEnabled

                                    readonly property bool enabled: selected == "on"

                                    inactiveColor: Theme.darkGray4
                                    maxWidth: parent.width * 0.5
                                    options: ["on", "off"]

                                    onSelectedChanged: root.dirty = true
                                }
                            }
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                Layout.bottomMargin: 6
                Layout.topMargin: 40
                anchors.left: parent.left
                anchors.right: parent.right
                implicitHeight: historyColumn.height
                label: qsTr("History")

                Column {
                    id: historyColumn

                    anchors.left: parent.left
                    anchors.right: parent.right

                    Text {
                        color: Theme.white
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        text: qsTr("History")
                    }
                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 10
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        color: Theme.darkGray2
                        implicitHeight: historyPane.implicitHeight + 20

                        GridLayout {
                            id: historyPane

                            anchors.bottomMargin: 10
                            anchors.fill: parent
                            anchors.leftMargin: 17
                            anchors.rightMargin: 17
                            anchors.topMargin: 10
                            columnSpacing: 20
                            columns: historyColumn.width < 800 ? 1 : 2
                            rowSpacing: 15

                            RowLayout {
                                Layout.preferredWidth: historyPane.width / historyPane.columns

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Track duplicate distance")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                SettingComponents.SpinBox {
                                    id: historyDuplicateDistanceInput

                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.leftMargin: 20
                                    Layout.rightMargin: 20
                                    max: 1000
                                    min: 1
                                    precision: 0
                                    realValue: 2
                                    suffix: value > 1 ? qsTr(" tracks") : qsTr(" track")

                                    onValueChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: historyPane.width / historyPane.columns

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Delete history playlist with less than")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                SettingComponents.SpinBox {
                                    id: historyDeleteLessThanInput

                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.leftMargin: 20
                                    Layout.rightMargin: 20
                                    max: 1000
                                    min: 1
                                    precision: 0
                                    realValue: 2
                                    suffix: value > 1 ? qsTr(" tracks") : qsTr(" track")

                                    onValueChanged: root.dirty = true
                                }
                            }
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                Layout.bottomMargin: 6
                Layout.topMargin: 40
                anchors.left: parent.left
                anchors.right: parent.right
                implicitHeight: searchColumn.height
                label: qsTr("Search")

                Column {
                    id: searchColumn

                    anchors.left: parent.left
                    anchors.right: parent.right

                    Text {
                        color: Theme.white
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        text: qsTr("Search")
                    }
                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 10
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        color: Theme.darkGray2
                        implicitHeight: searchPane.implicitHeight + 20

                        GridLayout {
                            id: searchPane

                            anchors.bottomMargin: 10
                            anchors.fill: parent
                            anchors.leftMargin: 17
                            anchors.rightMargin: 17
                            anchors.topMargin: 10
                            columnSpacing: 20
                            columns: 2
                            rowSpacing: 15

                            RowLayout {
                                Layout.preferredWidth: searchPane.width * 0.5

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Search completion")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                RatioChoice {
                                    id: searchCompletionEnabled

                                    readonly property bool enabled: selected == "on"

                                    inactiveColor: Theme.darkGray4
                                    maxWidth: parent.width * 0.5
                                    options: ["on", "off"]

                                    onSelectedChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: searchPane.width * 0.5

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Search history keyboard shortcuts")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                RatioChoice {
                                    id: searchHistoryKeyboardEnabled

                                    readonly property bool enabled: selected == "on"

                                    inactiveColor: Theme.darkGray4
                                    maxWidth: parent.width * 0.5
                                    options: ["on", "off"]

                                    onSelectedChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: searchPane.width * 0.5

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Search-as-you-type timeout")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                SettingComponents.Slider {
                                    id: searchAsYouTypeTimeoutInput

                                    Layout.preferredWidth: 400
                                    decimals: 1
                                    markers: [0.1, 0.5, 1, 5, 10]
                                    max: 10
                                    min: 0.1
                                    suffix: qsTr("sec")
                                    value: 0.3

                                    onValueChanged: root.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: searchPane.width * 0.5

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: qsTr("Pitch slider for fuzz BPM search")

                                    Text {
                                        anchors.fill: parent
                                        color: Theme.white
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                        horizontalAlignment: Text.AlignLeft
                                        text: parent.label
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                SettingComponents.Slider {
                                    id: pitchSliderFuzzBPMInput

                                    Layout.preferredWidth: 400
                                    markers: [0, 25, 50, 75, 100]
                                    max: 100
                                    min: 0
                                    suffix: "%"
                                    value: 8

                                    onValueChanged: root.dirty = true
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Item {
        id: buttonActions

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20

        SettingComponents.FormButton {
            activeColor: "#999999"
            anchors.left: parent.left
            backgroundColor: "#7D3B3B"
            opacity: enabled ? 1.0 : 0.5
            text: qsTr("Reset")

            onPressed: {
                root.reset();
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 10

            Text {
                id: errorMessage

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                color: "#7D3B3B"
                text: ""
            }
            SettingComponents.FormButton {
                activeColor: "#999999"
                backgroundColor: "#3F3F3F"
                opacity: enabled ? 1.0 : 0.5
                text: qsTr("Cancel")
                visible: root.dirty

                onPressed: {
                    root.load();
                }
            }
            SettingComponents.FormButton {
                activeColor: "#999999"
                backgroundColor: root.dirty ? "#3a60be" : "#3F3F3F"
                enabled: root.dirty
                opacity: enabled ? 1.0 : 0.5
                text: qsTr("Save")

                onPressed: {
                    errorMessage.text = "";
                    root.save();
                }
            }
        }
    }
    Connections {
        function onRunningChanged(running) {
            if (!Mixxx.Library.scanner.running) {
                root.loadSources();
            }
        }

        target: Mixxx.Library.scanner
    }
}
