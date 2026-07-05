import "." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.6
import "Theme"
import "Library" as LibraryComponent

Item {
    id: root

    property var activeSidebar: libraryLeftSources.sidebar()

    LibraryComponent.SourceTree {
        id: libraryLeftSources
    }
    LibraryComponent.SourceTree {
        id: libraryRightSources
    }
    SplitView {
        id: librarySplitView

        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            id: handleDelegate

            property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
            property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

            clip: true
            color: Theme.panelSplitterBackground
            implicitHeight: 8
            implicitWidth: 8

            containmentMask: Item {
                height: librarySplitView.height
                width: 8
                x: (handleDelegate.width - width) / 2
            }

            ColumnLayout {
                anchors.centerIn: parent

                Repeater {
                    model: 3

                    Rectangle {
                        color: handleColor
                        height: handleSize
                        radius: handleSize
                        width: handleSize
                    }
                }
            }
        }

        SplitView {
            id: sideBarSplitView

            SplitView.maximumWidth: 550
            SplitView.minimumWidth: 150
            SplitView.preferredWidth: root.width * 0.15
            orientation: Qt.Vertical

            handle: Rectangle {
                id: handleDelegate

                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                clip: true
                color: Theme.panelSplitterBackground
                implicitHeight: 8
                implicitWidth: 8

                containmentMask: Item {
                    height: 8
                    width: sideBarSplitView.width
                    x: (handleDelegate.width - width) / 2
                }

                RowLayout {
                    anchors.centerIn: parent

                    Repeater {
                        model: 3

                        Rectangle {
                            color: handleColor
                            height: handleSize
                            radius: handleSize
                            width: handleSize
                        }
                    }
                }
            }

            LibraryComponent.Browser {
                id: sidebarTree
                SplitView.fillHeight: true
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 500
                model: root.activeSidebar
            }
            Skin.PreviewDeck {
                SplitView.maximumHeight: 200
                SplitView.minimumHeight: 100
                SplitView.preferredHeight: 100
            }
        }
        Item {
            id: browsingView
            SplitView.fillHeight: true
            SplitView.minimumHeight: 200
            SplitView.preferredWidth: root.width * 0.75

            SplitView {
                id: trackListSplitView

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: tracklistMenu.left
                anchors.bottom: parent.bottom

                orientation: root.width < 800 ? Qt.Vertical: Qt.Horizontal

                handle: Rectangle {
                    id: handleDelegate

                    property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                    property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                    clip: true
                    color: Theme.panelSplitterBackground
                    implicitHeight: 8
                    implicitWidth: 8

                    containmentMask: Item {
                        height: root.width < 800 ? 8 : librarySplitView.height
                        width: root.width < 800 ? sideBarSplitView.width : 8
                        x: (handleDelegate.width - width) / 2
                    }

                    GridLayout {
                        anchors.centerIn: parent
                        columns: root.width < 800 ? 3 : 1

                        Repeater {
                            model: 3

                            Rectangle {
                                color: handleColor
                                height: handleSize
                                radius: handleSize
                                width: handleSize
                            }
                        }
                    }
                }
                LibraryComponent.TrackList {
                    opacity: root.activeSidebar == model.sidebar() ? 1 : 0.6
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5

                    focus: true
                    model: libraryLeftSources

                    TapHandler {
                        onTapped: {
                            root.activeSidebar = parent.model.sidebar()
                        }
                    }
                }

                Loader {
                    visible: splitViewButton.checked
                    SplitView.preferredHeight: trackListSplitView.height * 0.5
                    SplitView.preferredWidth: trackListSplitView.width * 0.5
                    active: splitViewButton.checked
                    opacity: status == Loader.Ready ? 1 : 0
                    asynchronous: true

                    sourceComponent: Component {
                        LibraryComponent.TrackList {
                            opacity: root.activeSidebar == model.sidebar() ? 1 : 0.6

                            focus: true
                            model: libraryRightSources

                            TapHandler {
                                onTapped: {
                                    root.activeSidebar = parent.model.sidebar()
                                }
                            }
                        }
                    }
                }
            }
            Column {
                id: tracklistMenu
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 36
                spacing: 11
                padding: 4

                Skin.Button {
                    id: splitViewButton

                    activeColor: Theme.white
                    checkable: true
                    icon.height: 16
                    icon.source: "images/splitview.svg"
                    icon.width: 16
                    implicitWidth: implicitHeight

                    onCheckedChanged: {
                        if (checked) {
                            let rightSidebar = libraryRightSources.sidebar()
                            rightSidebar.activate(rightSidebar.index(sidebarTree.currentSelectedIndex?.top ?? 0, 0));
                            root.activeSidebar = rightSidebar
                        } else {
                            root.activeSidebar = libraryLeftSources.sidebar()
                        }
                    }
                }

                Item {
                    height: historyButton.implicitWidth
                    width: historyButton.implicitHeight
                    Skin.Button {
                        id: historyButton

                        activeColor: Theme.white
                        checkable: true
                        text: "HISTORY"

                        transform: Rotation {
                            origin.x: historyButton.height / 2
                            origin.y: historyButton.height / 2
                            angle: 90
                        }

                    }
                }

                Item {
                    height: prepButton.implicitWidth
                    width: prepButton.implicitHeight
                    Skin.Button {
                        id: prepButton

                        activeColor: Theme.white
                        checkable: true
                        text: "PREPARATION"
                        implicitWidth: 82

                        transform: Rotation {
                            origin.x: prepButton.height / 2
                            origin.y: prepButton.height / 2
                            angle: 90
                        }
                    }
                }
            }

            Rectangle {
                id: searchPane

                property bool activated: false
                property int currentlyFocusedInput: 0

                width: 250
                height: 36

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: '#D9D9D9'
                topLeftRadius: 16
                focusPolicy: Qt.ClickFocus

                function selectInput(){
                    if (searchPane.currentlyFocusedInput === 0) {
                        searchField.forceActiveFocus()
                    } else {
                        let criteria = criteriaInput.itemAt(searchPane.currentlyFocusedInput - 1);
                        criteria.textInput.forceActiveFocus()
                    }
                }

                function updateSearchQuery(){
                    let query = []

                    for (let i = 0; i < selectedCriteria.count; i++){
                        let criteria = selectedCriteria.get(i)
                        let item = criteriaInput.itemAt(i);
                        let value = item.textInput.text
                        if (value.length == 0) {
                            continue
                        }
                        query.push(`${criteria.name.toLowerCase()}:${value}`)
                    }

                    if (searchField.text.length) {
                        query.push(searchField.text)
                    }

                    searchDebounce.query = query.join(' ')
                }

                TapHandler {
                    onTapped: event => {
                        searchPane.selectInput()
                        searchPane.activated = true
                        event.accepted = true
                    }
                }

                Timer {
                    id: searchDebounce

                    property var query: ""

                    onQueryChanged: {
                        restart()
                    }

                    interval: 800
                    repeat: false
                    onTriggered: {
                        console.log(`searching: ${query}`)
                        root.activeSidebar.tracklist.search(query)
                    }
                }

                ListModel {
                    id : fieldModel
                    ListElement { name: "Artist" }
                    ListElement { name: "Album" }
                    ListElement { name: "BPM" }
                    ListElement { name: "Title" }
                    ListElement { name: "Filename" }
                }

                ListModel {
                    id : selectedCriteria
                }

                SortFilterProxyModel {
                    id: fieldFilter
                    model: fieldModel
                    filters: [
                        FunctionFilter {
                            component CustomData: QtObject { property string name }
                            property var regExp: new RegExp(searchField.text, "i")
                            onRegExpChanged: invalidate()
                            function filter(data: CustomData): bool {
                                return regExp.test(data.name);
                            }
                        }
                    ]
                }

                FocusScope {
                    width: parent.width
                    height: parent.height
                    focus: true

                    Keys.onPressed: (event) => {
                        switch(event.key) {
                        case Qt.Key_Escape:
                            browsingView.forceActiveFocus()
                            break;
                        }
                    }

                    onActiveFocusChanged: {
                        if (!searchPane.activeFocus){
                            searchPane.activated = false
                        }
                    }

                    states: [
                        State {
                            when: searchPane.activated

                            PropertyChanges {
                                searchPane.width: 490
                                searchPane.height: 144
                            }
                        }
                    ]

                    Behavior on width {
                        NumberAnimation {}
                    }

                    Behavior on height {
                        NumberAnimation {}
                    }

                    Column {
                        anchors.fill: parent
                        Row {
                            anchors.left: parent.left
                            anchors.leftMargin: 11

                            width: searchPane.width - 22
                            spacing: 5
                            Repeater {
                                id: criteriaInput
                                model: selectedCriteria

                                Skin.SearchFieldCriteria {
                                    required property int index
                                    required property var modelData
                                    anchors.margins: 5
                                    anchors.verticalCenter: parent.verticalCenter
                                    interactive: true
                                    focus: true
                                    focusPolicy: Qt.StrongFocus
                                    field: modelData
                                    onActiveFocusChanged: {
                                        if (this.activeFocus){
                                            searchPane.activated = true
                                        }
                                    }

                                    onDeleted: {
                                        searchPane.currentlyFocusedInput = (searchPane.currentlyFocusedInput - 1) % (selectedCriteria.rowCount() + 1);
                                        searchPane.selectInput()
                                        selectedCriteria.remove(index)
                                    }

                                    onEdited: {
                                        searchPane.updateSearchQuery()
                                    }
                                    Component.onCompleted: {
                                        if (searchPane.currentlyFocusedInput === index + 1){
                                            this.textInput.forceActiveFocus()
                                        }
                                    }
                                }
                            }
                            Item {
                                width: 224
                                height: 38
                                TextInput {
                                    id: searchField
                                    visible: searchPane.activated
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    focus: true
                                    focusPolicy: Qt.StrongFocus
                                    clip: true
                                    color: "#808080"
                                    horizontalAlignment: TextInput.AlignLeft

                                    Keys.onTabPressed: (event) => {
                                        if (fieldFilter.rowCount() == 1 && fieldFilter.hasIndex(0, 0)) {
                                            searchPane.currentlyFocusedInput = selectedCriteria.rowCount() + 1
                                            selectedCriteria.append({"name": fieldFilter.data(fieldFilter.index(0, 0))})
                                            searchField.clear()
                                        } else {
                                            searchPane.currentlyFocusedInput = (searchPane.currentlyFocusedInput + 1) % (selectedCriteria.rowCount() + 1)
                                            searchPane.selectInput()
                                        }
                                        event.accepted = true

                                    }
                                    Keys.onPressed: (event) => {
                                        if (event.key == Qt.Key_Backspace && searchField.text.length == 0 && selectedCriteria.rowCount() > 0) {
                                            selectedCriteria.remove(selectedCriteria.rowCount() - 1)
                                            searchPane.currentlyFocusedInput = (searchPane.currentlyFocusedInput - 1) % (selectedCriteria.rowCount() + 1);
                                            searchPane.selectInput()
                                            event.accepted = true
                                        }
                                    }

                                    onTextChanged: {
                                        searchPane.updateSearchQuery()
                                    }
                                }
                                FontMetrics {
                                    id: fontMetrics
                                    font: searchField.font
                                }
                                Text {
                                    id: searchPlaceholder
                                    readonly property double cursorOffset: fontMetrics.advanceWidth(searchField.text)
                                    visible: (!searchPane.activated && criteriaInput.count == 0) || searchPane.activated && searchField.text.length == 0 || searchField.text.length >= 3 && suggestedFilters.count >= 1

                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: searchField.text.length >= 3 && suggestedFilters.count >= 1 ? cursorOffset : 0

                                    color: "#808080"
                                    text: {
                                        if (searchPane.activated && searchField.text.length >= 3 && suggestedFilters.count >= 1) {
                                            let field = fieldFilter.data(fieldFilter.index(0, 0))
                                            let pronoun = /^[aeiou]/i.test(field) ? "an" : "a"
                                            return `Press “Tab” to search for ${pronoun} ${field}`
                                        }
                                        return searchPane.activated ? "Start typing to get suggestion" : "Search..."
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: searchPane.width
                            height: 1
                            color: '#757575'
                        }
                        Row {
                            padding: 8
                            visible: searchField.text.length >= 3
                            Repeater {
                                id: suggestedFilters
                                model: fieldFilter
                                Skin.SearchFieldCriteria {
                                    required property var modelData
                                    field: modelData
                                    query: searchField.text
                                }
                            }
                        }
                        Text {
                            width: searchPane.width - 10
                            anchors.margins: 5
                            height: 16
                            text: "Recent searches"
                            font.weight: Font.DemiBold
                        }
                        ListView {
                            width: searchPane.width - 10
                            anchors.margins: 5
                            height: 100
                            model: ["artist:=Gorillaz client eastwood", "artist:BLACKPINK album:JUMP"]
                            delegate: Row {
                                height: 20
                                width: ListView.view.width - 10
                                anchors.margins: 5

                                readonly property var fields: Array(fieldModel.count).map(i => fieldModel.get(i))
                                readonly property var criteriaRegex: new RegExp(/(artist|album):([^\s]+)/i)
                                readonly property var criteria: modelData.split(' ').map(i => criteriaRegex.exec(i)).filter(i => i).map(r => ({name:r[1].charAt(0).toUpperCase() + r[1].substring(1).toLowerCase(), query:r[2]}))
                                readonly property var query: modelData.split(' ').filter(i => !criteriaRegex.test(i)).join(' ')

                                Repeater {
                                    model: criteria
                                    Skin.SearchFieldCriteria {
                                        required property var modelData
                                        field: modelData.name
                                        textInput.text: modelData.query
                                    }
                                }

                                Text {
                                    text: query
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
