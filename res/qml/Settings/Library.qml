import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import QtCore
import Qt5Compat.GraphicalEffects
import Mixxx 1.0 as Mixxx
import "." as Setting
import "../Deck" as DeckComponents
import "." as SettingComponents
import ".." as Skin
import "../Theme"

Category {
    id: root

    label: "Library"

    Component.onCompleted: {
        root.load()
    }

    property bool dirty: false

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18
        spacing: 0

        RowLayout {
            Text {
                Layout.leftMargin: 17
                Layout.bottomMargin: 14
                text: "Sources"
                color: Theme.white
                font.pixelSize: 16
                font.weight: Font.DemiBold
            }
        }

        Mixxx.SettingGroup {
            Layout.bottomMargin: 6
            label: "Sources"
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                id: theme
                spacing: 20
                anchors.fill: parent
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        color: '#0E0E0E'
                        implicitHeight: sourcePane.implicitHeight + 20
                        anchors.left: parent.left
                        anchors.right: parent.right

                        ColumnLayout {
                            id: sourcePane
                            anchors.fill: parent
                            spacing: 0

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 10
                                Layout.leftMargin: 8
                                Layout.rightMargin: 8

                                Text {
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 17
                                    text: 'Music Directory'
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                }

                                FolderDialog {
                                    id: addDialog
                                    title: qsTr("Choose a music directory")
                                    currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
                                    onAccepted: {
                                        let model = sourceListView.model
                                        let path = addDialog.selectedFolder.toString();
                                        path = path.replace(/^(file:\/\/)/,""); // FIXME does this work on Windows ?
                                        model.push({path: decodeURIComponent(path)})
                                        root.dirty = true;
                                        sourceListView.model = model
                                    }
                                }

                                SettingComponents.FormButton {
                                    text: "Add"
                                    opacity: enabled ? 1.0 : 0.5
                                    backgroundColor: "#3F3F3F"
                                    activeColor: "#999999"
                                    onPressed: addDialog.open()
                                }
                            }

                            ListView {
                                id: sourceListView
                                Layout.fillWidth: true
                                Layout.preferredHeight: 240
                                clip: true
                                model: []

                                // anchors.fill: parent
                                // anchors.topMargin: 10
                                // anchors.bottomMargin: 10
                                // anchors.leftMargin: 17
                                // anchors.rightMargin: 17

                                delegate: MouseArea {
                                    required property int index
                                    required property var modelData
                                    readonly property bool selected: ListView.view.currentIndex == index && modelData.deleting === undefined && !modelData.relink && modelData.trackCount !== undefined

                                    id: mouse

                                    hoverEnabled: true
                                    width: ListView.view.width
                                    height: 34

                                    Timer {
                                        id: cancelDeletion
                                        interval: 1000
                                        running: removeButton.confirming && !mouse.containsMouse

                                        onTriggered: {
                                            removeButton.confirming = false
                                        }
                                    }

                                    onPressed: {
                                        ListView.view.currentIndex = index
                                    }

                                    onSelectedChanged: {
                                        removeButton.confirming = false
                                    }
                                    Rectangle {
                                        anchors.fill: parent
                                        color: index % 2 == 0 ? '#3F3F3F' : '#2B2B2B'
                                        // width: 180; height: 40
                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 8
                                            anchors.rightMargin: 8
                                            Text { text: 'Icon'; width: 160 }
                                            Text {
                                                Layout.fillWidth: true
                                                text: modelData.path
                                                font.weight: Font.Medium
                                                color: Theme.white
                                                font.pixelSize: 14
                                                opacity: modelData.deleting !== undefined ? 0.4 : 1
                                            }

                                            FolderDialog {
                                                id: relinkDialog
                                                title: qsTr("Relink music directory to new location")
                                                currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
                                                onAccepted: {
                                                    let model = sourceListView.model
                                                    let path = selectedFolder.toString().replace(/^(file:\/\/)/,""); // FIXME does this work on Windows ?
                                                    model[mouse.index].relink = decodeURIComponent(path);
                                                    root.dirty = true;
                                                    sourceListView.model = model
                                                }
                                            }

                                            SettingComponents.FormButton {
                                                id: relinkButton
                                                visible: selected && modelData.trackCount !== undefined && !Mixxx.Library.scanner.running
                                                text: modelData.relink ? "Save to proceed" : "Relink"
                                                opacity: enabled ? 1.0 : 0.5
                                                backgroundColor: "#3F3F3F"
                                                activeColor: "#999999"
                                                onPressed: {
                                                    relinkDialog.open()
                                                }
                                            }

                                            Item {
                                                property bool confirming: false

                                                visible: selected && !Mixxx.Library.scanner.running
                                                clip: true
                                                id: removeButton

                                                implicitHeight: Math.max(actionButton.implicitHeight, removeModeSelector.implicitHeight)
                                                implicitWidth: removeButton.confirming ? removeModeSelector.implicitWidth : actionButton.implicitWidth

                                                SettingComponents.FormButton {
                                                    id: actionButton
                                                    anchors.centerIn: parent
                                                    visible: !removeButton.confirming
                                                    text: "Remove"
                                                    opacity: enabled ? 1.0 : 0.5
                                                    backgroundColor: "#7D3B3B"
                                                    activeColor: "#999999"
                                                    onPressed: {
                                                        if (modelData.trackCount == 0) {
                                                            let model = sourceListView.model
                                                            model[mouse.index].deleting = Mixxx.Library.PurgeTracks
                                                            root.dirty = true;
                                                            sourceListView.model = model
                                                        } else {
                                                            removeButton.confirming = !removeButton.confirming;
                                                        }
                                                    }
                                                }
                                                RatioChoice {
                                                    id: removeModeSelector
                                                    visible: removeButton.confirming
                                                    anchors.centerIn: parent
                                                    onSelectedChanged: {
                                                        let model = sourceListView.model
                                                        switch (options.indexOf(selected)) {
                                                            case 0:
                                                                model[mouse.index].deleting = Mixxx.Library.KeepTracks
                                                                break
                                                            case 1:
                                                                model[mouse.index].deleting = Mixxx.Library.HideTracks
                                                                break
                                                            case 2:
                                                                model[mouse.index].deleting = Mixxx.Library.PurgeTracks
                                                                break
                                                            default:
                                                                console.warn(`unknown value deletion mode ${selected}. Ignoring.`)
                                                                return
                                                        }
                                                        root.dirty = true;
                                                        sourceListView.model = model
                                                    }
                                                    normalizedWidth: false
                                                    inactiveColor: Theme.darkGray4
                                                    selected: null
                                                    options: [
                                                              "keep tracks",
                                                              "hide tracks",
                                                              "purge tracks"
                                                    ]
                                                }

                                                Behavior on implicitWidth { PropertyAnimation {} }
                                            }
                                            Text {
                                                visible: !selected && !Mixxx.Library.scanner.running
                                                text: {
                                                    if (modelData.relink) {
                                                        return `Save to relink to ${modelData.relink}`
                                                    } else if (modelData.deleting !== undefined) {
                                                        let action = {
                                                            0: "keep already imported tracks",
                                                            1: "hide imported tracks",
                                                            2: "purge imported tracks",
                                                        }[modelData.deleting] || 'perform an unknown action';
                                                        return `Save to delete and ${action}`
                                                    } else if (modelData.trackCount !== undefined && modelData.totalMinute !== undefined) {
                                                        return `${modelData.trackCount} tracks, ${modelData.totalMinute} minutes`
                                                    } else {
                                                        return 'Save to import new folder'
                                                    }
                                                }
                                                font.weight: Font.Medium
                                                font.italic: modelData.trackCount === undefined || modelData.totalMinute === undefined || !!modelData.relink
                                                color: '#626262'
                                                font.pixelSize: 14
                                            }
                                        }
                                    }
                                }
                                focus: true
                            }
                        }
                        Rectangle {
                            anchors.fill: parent
                            visible: Mixxx.Library.scanner.running
                            color: Qt.alpha('black', 0.4)
                            MouseArea {
                                anchors.fill: parent
                            }
                            Text {
                                id: scanProgress
                                text: "File scanning..."
                                color: Theme.white
                                font.pixelSize: 14

                                anchors {
                                    margins: 5
                                    left: parent.left
                                    bottom: parent.bottom
                                }

                                Connections {
                                    function onProgress(currentPath) {
                                        scanProgress.text = `File scanning: ${currentPath}`
                                    }

                                    target: Mixxx.Library.scanner
                                }
                            }
                            SettingComponents.FormButton {
                                text: "Cancel"
                                enabled: !Mixxx.Library.scanner.cancelling
                                backgroundColor: Mixxx.Library.scanner.cancelling ? "#999999" : "#3a60be"
                                activeColor: "#999999"
                                onPressed: {
                                    Mixxx.Library.scanner.cancel()
                                }

                                anchors {
                                    margins: 5
                                    right: parent.right
                                    bottom: parent.bottom
                                }
                            }
                        }
                    }
                }
                ColumnLayout {
                    Item {
                        Layout.preferredWidth: root.width * 0.35
                        Layout.fillHeight: true

                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: integrationPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            ColumnLayout {
                                id: integrationPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17

                                Repeater {
                                    id: integrationRepeater

                                    RowLayout {
                                        required property var modelData
                                        property alias enabled: integrationEnabled.enabled
                                        Layout.preferredWidth: sourcePane.width * 0.5
                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: modelData.service
                                            Text {
                                                anchors.fill: parent

                                                horizontalAlignment: Text.AlignLeft
                                                verticalAlignment: Text.AlignVCenter

                                                text: parent.label
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                            }
                                        }
                                        RatioChoice {
                                            onSelectedChanged: root.dirty = true
                                            maxWidth: parent.width * 0.5
                                            id: integrationEnabled
                                            inactiveColor: Theme.darkGray4
                                            selected: modelData.enabled ? "on" : "off"
                                            readonly property bool enabled: selected == "on"
                                            options: [
                                                      "on",
                                                      "off"
                                            ]
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Mixxx.SettingGroup {
            Layout.topMargin: 40
            Layout.bottomMargin: 6
            label: "Metadata"
            implicitHeight: metadataColumn.height
            Layout.fillWidth: true
            Column {
                id: metadataColumn
                anchors.left: parent.left
                anchors.right: parent.right
                Text {
                    text: "Metadata"
                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 10
                }
                Rectangle {
                    color: Theme.darkGray2
                    implicitHeight: metadataPane.implicitHeight + 20
                    anchors.left: parent.left
                    anchors.right: parent.right
                    GridLayout {
                        id: metadataPane
                        anchors.fill: parent
                        anchors.topMargin: 10
                        anchors.bottomMargin: 10
                        anchors.leftMargin: 17
                        anchors.rightMargin: 17
                        rowSpacing: 15
                        columnSpacing: 20
                        columns: 2

                        RowLayout {
                            Layout.preferredWidth: metadataPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Synchronise metadata with file"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: root.dirty = true
                                maxWidth: parent.width * 0.5
                                id: metadataSynchroniseEnabled
                                inactiveColor: Theme.darkGray4
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: metadataPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Synchronise metadata with Serato library"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: root.dirty = true
                                maxWidth: parent.width * 0.5
                                id: metadataSynchroniseSeratoEnabled
                                inactiveColor: Theme.darkGray4
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: metadataPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Prefer relative path on playlist export"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: root.dirty = true
                                maxWidth: parent.width * 0.5
                                id: metadataRelativePathEnabled
                                inactiveColor: Theme.darkGray4
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }
                    }
                }
            }
        }
        Mixxx.SettingGroup {
            Layout.topMargin: 40
            Layout.bottomMargin: 6
            label: "History"
            implicitHeight: historyColumn.height
            Layout.fillWidth: true
            Column {
                id: historyColumn
                anchors.left: parent.left
                anchors.right: parent.right
                Text {
                    text: "History"
                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 10
                }
                Rectangle {
                    color: Theme.darkGray2
                    implicitHeight: historyPane.implicitHeight + 20
                    anchors.left: parent.left
                    anchors.right: parent.right
                    GridLayout {
                        id: historyPane
                        anchors.fill: parent
                        anchors.topMargin: 10
                        anchors.bottomMargin: 10
                        anchors.leftMargin: 17
                        anchors.rightMargin: 17
                        rowSpacing: 15
                        columnSpacing: 20
                        columns: 2

                        RowLayout {
                            Layout.preferredWidth: historyPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Track duplicate distance"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            SettingComponents.SpinBox {
                                onValueChanged: root.dirty = true
                                id: historyDuplicateDistanceInput
                                Layout.alignment: Qt.AlignHCenter
                                Layout.leftMargin: 20
                                Layout.rightMargin: 20
                                realValue: 2
                                precision: 0
                                min: 1
                                max: 1000
                                suffix: value > 1 ? " tracks" : " track"
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: historyPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Delete history playlist with less than"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            SettingComponents.SpinBox {
                                onValueChanged: root.dirty = true
                                id: historyDeleteLessThanInput
                                Layout.alignment: Qt.AlignHCenter
                                Layout.leftMargin: 20
                                Layout.rightMargin: 20
                                realValue: 2
                                precision: 0
                                min: 1
                                max: 1000
                                suffix: value > 1 ? " tracks" : " track"
                            }
                        }
                    }
                }
            }
        }
        Mixxx.SettingGroup {
            Layout.topMargin: 40
            Layout.bottomMargin: 6
            label: "Search"
            implicitHeight: searchColumn.height
            Layout.fillWidth: true
            Column {
                id: searchColumn
                anchors.left: parent.left
                anchors.right: parent.right
                Text {
                    text: "Search"
                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 10
                }
                Rectangle {
                    color: Theme.darkGray2
                    implicitHeight: searchPane.implicitHeight + 20
                    anchors.left: parent.left
                    anchors.right: parent.right
                    GridLayout {
                        id: searchPane
                        anchors.fill: parent
                        anchors.topMargin: 10
                        anchors.bottomMargin: 10
                        anchors.leftMargin: 17
                        anchors.rightMargin: 17
                        rowSpacing: 15
                        columnSpacing: 20
                        columns: 2

                        RowLayout {
                            Layout.preferredWidth: searchPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Search completion"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: root.dirty = true
                                maxWidth: parent.width * 0.5
                                id: searchCompletionEnabled
                                inactiveColor: Theme.darkGray4
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: searchPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Search history keyboard shortcuts"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: root.dirty = true
                                maxWidth: parent.width * 0.5
                                id: searchHistoryKeyboardEnabled
                                inactiveColor: Theme.darkGray4
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: searchPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Search-as-you-type timeout"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            SettingComponents.Slider {
                                onValueChanged: root.dirty = true
                                id: searchAsYouTypeTimeoutInput
                                Layout.preferredWidth: 400
                                markers: [0.1, 0.5, 1, 5, 10]
                                suffix: "sec"
                                value: 0.3
                                decimals: 1
                                min: 0.1
                                max: 10
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: searchPane.width * 0.5
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Pitch slider for fuzz BPM search"
                                Text {
                                    anchors.fill: parent

                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                }
                            }
                            SettingComponents.Slider {
                                onValueChanged: root.dirty = true
                                id: pitchSliderFuzzBPMInput
                                Layout.preferredWidth: 400
                                markers: [0, 25, 50, 75, 100]
                                suffix: "%"
                                value: 8
                                min: 0
                                max: 100
                            }
                        }
                    }
                }
            }
        }
    }

    Item {
        id: buttonActions
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 20
        SettingComponents.FormButton {
            anchors.left: parent.left
            text: "Reset"
            opacity: enabled ? 1.0 : 0.5
            backgroundColor: "#7D3B3B"
            activeColor: "#999999"
            onPressed: {
                root.reset()
            }
        }
        Row {
            spacing: 10
            anchors.right: parent.right
            Text {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                id: errorMessage
                text: ""
                color: "#7D3B3B"
            }
            SettingComponents.FormButton {
                visible: root.dirty
                text: "Cancel"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    root.load()
                }
            }
            SettingComponents.FormButton {
                enabled: root.dirty
                text: "Save"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: root.dirty ? "#3a60be" : "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    errorMessage.text = ""
                    root.save()
                }
            }
        }
    }

    function reset() {
    }

    Connections {
        function onRunningChanged(running) {
            if (!Mixxx.Library.scanner.running) {
                root.loadSources()
            }
        }

        target: Mixxx.Library.scanner
    }

    function loadSources() {
        let rootDirs = []
        for (let source of Object.values(Mixxx.Library.sources)) {
            rootDirs.push({
                    path: source.path,
                    trackCount: source.trackCount,
                    totalMinute: Math.round(source.totalSecond/60)
            })
        }
        sourceListView.model = rootDirs
    }
    function load() {
        loadSources();
        integrationRepeater.model = [
            {service: "Rhythmbox", enabled: Mixxx.Config.libraryRhythmboxEnabled},
            {service: "Banshee", enabled: Mixxx.Config.libraryBansheeEnabled},
            {service: "iTunes", enabled: Mixxx.Config.libraryITunesEnabled},
            {service: "Traktor", enabled: Mixxx.Config.libraryTraktorEnabled},
            {service: "Rekordbox", enabled: Mixxx.Config.libraryRekordboxEnabled},
            {service: "Serato", enabled: Mixxx.Config.librarySeratoEnabled},
        ]
        metadataSynchroniseEnabled.selected = Mixxx.Config.librarySyncTrackMetadataExport ? "on" : "off"
        metadataSynchroniseSeratoEnabled.selected = Mixxx.Config.librarySeratoMetadataExport ? "on" : "off"
        metadataRelativePathEnabled.selected = Mixxx.Config.libraryUseRelativePathOnExport ? "on" : "off"
        searchCompletionEnabled.selected = Mixxx.Config.librarySearchCompletionsEnable ? "on" : "off"
        searchHistoryKeyboardEnabled.selected = Mixxx.Config.librarySearchHistoryShortcutsEnable ? "on" : "off"
        historyDuplicateDistanceInput.value = Mixxx.Config.libraryHistoryTrackDuplicateDistance;
        historyDeleteLessThanInput.value = Mixxx.Config.libraryHistoryMinTracksToKeep;
        searchAsYouTypeTimeoutInput.setValue(Mixxx.Config.librarySearchDebouncingTimeout / 1000);
        pitchSliderFuzzBPMInput.setValue(Mixxx.Config.librarySearchBpmFuzzyRange);
        root.dirty = false
        errorMessage.text = ""
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
                        return
                    case Mixxx.Library.AddResult.InvalidOrMissingDirectory:
                        errorMessage.text = qsTr(
                            "This or a listed directory does not exist or is inaccessible.\nAborting the operation to avoid library inconsistencies");
                        return
                    case Mixxx.Library.AddResult.UnreadableDirectory:
                        errorMessage.text = qsTr(
                            "This directory can not be read.");
                        return
                    case Mixxx.Library.AddResult.SqlError:
                        errorMessage.text = qsTr(
                            "An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                        return
                }
            } else if (source.deleting !== undefined) {
                // Handle removal
                switch (Mixxx.Library.removeSource(source.path, source.deleting)) {
                    case Mixxx.Library.RemoveResult.NotFound:
                        errorMessage.text = qsTr("This directory can not be found.");
                        return
                    case Mixxx.Library.RemoveResult.SqlError:
                        errorMessage.text = qsTr(
                            "An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                        return
                }
            } else if (source.relink) {
                // Handle relinking
                switch (Mixxx.Library.relinkSource(source.path, source.relink)) {
                    case Mixxx.Library.RelocateResult.InvalidOrMissingDirectory:
                        errorMessage.text = qsTr(
                            "This or a listed directory does not exist or is inaccessible.\nAborting the operation to avoid library inconsistencies");
                        return
                    case Mixxx.Library.RelocateResult.UnreadableDirectory:
                        errorMessage.text = qsTr(
                            "This directory can not be read.");
                        return
                    case Mixxx.Library.RelocateResult.SqlError:
                        errorMessage.text = qsTr(
                            "An unknown error occurred.\nAborting the operation to avoid library inconsistencies");
                        return
                }
            } else {
                continue
            }
            changed = true
        }

        if (changed) {
            Mixxx.Library.scanner.start()
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
        load()
    }
}
