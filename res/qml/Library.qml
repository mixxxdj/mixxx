import "." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
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

        handle: Skin.SplitViewHandle {
            id: librarySplitHandle

            containmentMask: Item {
                height: librarySplitView.height
                width: 8
                x: (librarySplitHandle.width - width) / 2
            }
        }

        SplitView {
            id: sideBarSplitView

            SplitView.maximumWidth: 550
            SplitView.minimumWidth: 150
            SplitView.preferredWidth: root.width * 0.15
            orientation: Qt.Vertical

            handle: Skin.SplitViewHandle {
                id: sideBarSplitHandle

                dotColumns: 3

                containmentMask: Item {
                    height: 8
                    width: sideBarSplitView.width
                    x: (sideBarSplitHandle.width - width) / 2
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

                handle: Skin.SplitViewHandle {
                    id: trackListSplitHandle

                    dotColumns: root.width < 800 ? 3 : 1

                    containmentMask: Item {
                        height: root.width < 800 ? 8 : librarySplitView.height
                        width: root.width < 800 ? sideBarSplitView.width : 8
                        x: (trackListSplitHandle.width - width) / 2
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
                property int activeTokenIndex: -1
                property int highlightedIndex: -1
                property int highlightedRecentIndex: -1
                property int activeRecentIndex: -1
                property string activeQuery: ""
                property string freeSearchText: ""
                property bool selectingAll: false

                readonly property bool hasSearch: activeQuery.length > 0

                onActivatedChanged: {
                    if (activated) {
                        Mixxx.Core.addOpenedPopup(searchPane)
                    } else {
                        Mixxx.Core.removeOpenedPopup(searchPane)
                    }
                }

                Connections {
                    target: Qt.inputMethod

                    function onVisibleChanged() {
                        if (!Qt.inputMethod.visible) {
                            searchPane.activated = false
                        }
                    }
                }

                ListModel {
                    id: recentSearchesModel
                    ListElement { tokensJson: '[{"name":"Artist","value":"A Super Artist"}]'; freeText: "" }
                    ListElement { tokensJson: '[{"name":"Artist","value":"super artist"},{"name":"BPM","value":"100"}]'; freeText: "Foo" }
                }

                width: 250
                height: 36

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: '#D9D9D9'
                topLeftRadius: 16

                Shortcut {
                    sequence: "Ctrl+F"
                    context: Qt.WindowShortcut
                    onActivated: searchPane.activateSearch()
                }

                Skin.FocusedWidgetControl {
                    id: focusedWidgetControl
                }

                function quote(value) {
                    if (/\s/.test(value)) {
                        return '"' + value + '"'
                    }
                    return value
                }

                function isRecentShown() {
                    return activated && activeTokenIndex < 0 && freeSearchText.length === 0 && selectedCriteria.count === 0
                }

                function commitCurrentEditor() {
                    if (activeTokenIndex >= 0) {
                        selectedCriteria.setProperty(activeTokenIndex, "value", searchField.text)
                    }
                }

                function editorHost() {
                    if (activeTokenIndex < 0) {
                        return freeTextHost
                    }
                    let item = criteriaInput.itemAt(activeTokenIndex)
                    return item ? item.valueEditorHost : freeTextHost
                }

                function computeEditorX() {
                    let x = criteriaRow.mapToItem(searchField.parent, 0, 0).x
                    for (let i = 0; i < selectedCriteria.count; i++) {
                        let item = criteriaInput.itemAt(i)
                        if (!item) {
                            continue
                        }
                        if (activeTokenIndex === i) {
                            x += item.valueAreaX
                            break
                        }
                        x += item.width + criteriaRow.spacing
                    }
                    return x
                }

                function updateEditorPosition() {
                    if (!activated) {
                        return
                    }
                    let host = editorHost()
                    if (!host) {
                        return
                    }
                    let p = host.mapToItem(searchField.parent, 0, 0)
                    searchField.x = computeEditorX()
                    searchField.y = p.y
                    searchField.width = host.width
                    searchField.height = host.height
                }

                function appendSearchSuggestions(field) {
                    Mixxx.Library.searchSuggestions.setQuery(field, searchField.text)
                    for (let j = 0; j < Mixxx.Library.searchSuggestions.rowCount(); j++) {
                        let s = Mixxx.Library.searchSuggestions.get(j)
                        suggestionModel.append({ display: s.value, meta: s.label, isField: false })
                    }
                }

                function refreshSuggestions() {
                    suggestionModel.clear()
                    highlightedIndex = -1
                    highlightedRecentIndex = -1
                    if (!activated) {
                        return
                    }
                    if (activeTokenIndex < 0) {
                        let text = searchField.text.toLowerCase()
                        if (text.length >= 1) {
                            for (let i = 0; i < fieldModel.count; i++) {
                                let f = fieldModel.get(i)
                                if (f.name.toLowerCase().indexOf(text) === 0) {
                                    suggestionModel.append({ display: f.name + ":", meta: "", isField: true, name: f.name, query: f.query })
                                }
                            }
                            appendSearchSuggestions("track")
                        }
                    } else {
                        let tok = selectedCriteria.get(activeTokenIndex)
                        if (tok) {
                            appendSearchSuggestions(tok.query)
                        }
                    }
                }

                function updateSearchQuery() {
                    let query = []
                    for (let i = 0; i < selectedCriteria.count; i++) {
                        let item = selectedCriteria.get(i)
                        let value = item.value
                        if (!value.length) {
                            continue
                        }
                        let v = quote(value)
                        if (item.exact) {
                            v = "=" + v
                        }
                        query.push(item.query + ":" + v)
                    }
                    if (activeTokenIndex < 0 && searchField.text.length) {
                        query.push(searchField.text)
                    }
                    if (activeTokenIndex < 0) {
                        freeSearchText = searchField.text
                    }
                    activeQuery = query.join(' ')
                    searchDebounce.query = activeQuery
                }

                function setActiveToken(index) {
                    commitCurrentEditor()
                    if (activeTokenIndex >= 0 && activeTokenIndex !== index) {
                        if (selectedCriteria.get(activeTokenIndex).value.length === 0) {
                            let removedIndex = activeTokenIndex
                            activeTokenIndex = -1
                            selectedCriteria.remove(removedIndex)
                            if (index > removedIndex) {
                                index--
                            }
                        }
                    }
                    activeTokenIndex = index
                    if (index >= 0) {
                        searchField.text = selectedCriteria.get(index).value
                    } else {
                        searchField.text = freeSearchText
                    }
                    searchField.cursorPosition = searchField.text.length
                    refreshSuggestions()
                    updateSearchQuery()
                    updateEditorPosition()
                    searchField.forceActiveFocus()
                }

                function navigateBackward() {
                    if (activeTokenIndex < 0) {
                        if (selectedCriteria.count > 0) {
                            setActiveToken(selectedCriteria.count - 1)
                        }
                    } else if (activeTokenIndex === 0) {
                        setActiveToken(-1)
                    } else {
                        setActiveToken(activeTokenIndex - 1)
                    }
                }

                function navigateForward() {
                    if (activeTokenIndex < 0) {
                        if (selectedCriteria.count > 0) {
                            setActiveToken(0)
                        }
                    } else if (activeTokenIndex === selectedCriteria.count - 1) {
                        setActiveToken(-1)
                    } else {
                        setActiveToken(activeTokenIndex + 1)

                    }
                }

                function removeToken(index) {
                    selectedCriteria.remove(index)
                    if (activeTokenIndex === index) {
                        activeTokenIndex = -1
                        searchField.text = freeSearchText
                        searchField.cursorPosition = searchField.text.length
                        refreshSuggestions()
                    } else if (activeTokenIndex > index) {
                        activeTokenIndex--
                    }
                    updateSearchQuery()
                    updateEditorPosition()
                }

                function acceptHighlightedSuggestion() {
                    if (suggestionModel.count === 0) {
                        return
                    }
                    let idx = highlightedIndex
                    if (idx < 0 || idx >= suggestionModel.count) {
                        idx = 0
                    }
                    let s = suggestionModel.get(idx)
                    if (activeTokenIndex < 0) {
                        if (s.isField) {
                            freeSearchText = ""
                            selectedCriteria.append({ name: s.name, query: s.query, value: "", exact: false })
                            setActiveToken(selectedCriteria.count - 1)
                        } else {
                            searchField.text = s.display
                            searchField.cursorPosition = searchField.text.length
                            refreshSuggestions()
                            updateSearchQuery()
                            updateEditorPosition()
                        }
                    } else {
                        selectedCriteria.setProperty(activeTokenIndex, "value", s.display)
                        searchField.text = s.display
                        refreshSuggestions()
                        updateSearchQuery()
                    }
                }

                function acceptFieldSuggestion() {
                    if (suggestionModel.count === 0) {
                        return
                    }
                    let idx = highlightedIndex
                    if (idx < 0 || idx >= suggestionModel.count) {
                        idx = 0
                    }
                    if (!suggestionModel.get(idx).isField) {
                        return
                    }
                    acceptHighlightedSuggestion()
                }

                function toggleExactMatch() {
                    if (activeTokenIndex < 0) {
                        return
                    }
                    let exact = !selectedCriteria.get(activeTokenIndex).exact
                    selectedCriteria.setProperty(activeTokenIndex, "exact", exact)
                    updateSearchQuery()
                }

                function activateSearch() {
                    searchField.text = freeSearchText
                    searchField.cursorPosition = searchField.text.length
                    syncSearchUI()
                }

                function syncSearchUI() {
                    activated = true
                    activeTokenIndex = -1
                    focusedWidgetControl.value = Skin.FocusedWidgetControl.WidgetKind.Searchbar
                    refreshSuggestions()
                    updateSearchQuery()
                    updateEditorPosition()
                    searchField.forceActiveFocus()
                }

                function clearAllCriteria() {
                    selectingAll = false
                    selectedCriteria.clear()
                    activeTokenIndex = -1
                    searchField.text = ""
                    refreshSuggestions()
                    updateSearchQuery()
                    updateEditorPosition()
                }

                function deactivateSearch() {
                    Qt.inputMethod.hide()
                    commitCurrentEditor()
                    for (let i = selectedCriteria.count - 1; i >= 0; i--) {
                        if (selectedCriteria.get(i).value.length === 0) {
                            selectedCriteria.remove(i)
                        }
                    }
                    deactivatePane()
                }

                function deactivatePane() {
                    activated = false
                    activeTokenIndex = -1
                    highlightedIndex = -1
                    focusedWidgetControl.value = Skin.FocusedWidgetControl.WidgetKind.LibraryView
                    browsingView.forceActiveFocus()
                }

                function fieldNameToQuery(name) {
                    for (let i = 0; i < fieldModel.count; i++) {
                        let f = fieldModel.get(i)
                        if (f.name === name) {
                            return f.query
                        }
                    }
                    return name.toLowerCase()
                }

                function saveCurrentSearch() {
                    // TODO: Persist the current search as a "smart" playlist in the
                    // library sidebar.
                }

                function persistSearch() {
                    let tokens = []
                    for (let i = 0; i < selectedCriteria.count; i++) {
                        let item = selectedCriteria.get(i)
                        if (item.value.length > 0) {
                            tokens.push({ name: item.name, value: item.value, exact: item.exact })
                        }
                    }
                    let freeText = activeTokenIndex < 0 ? searchField.text : ""
                    if (tokens.length === 0 && freeText.length === 0) {
                        return
                    }
                    let tokensJson = JSON.stringify(tokens)
                    if (activeRecentIndex >= 0 && activeRecentIndex < recentSearchesModel.count) {
                        recentSearchesModel.set(activeRecentIndex, { freeText: freeText, tokensJson: tokensJson })
                    } else {
                        recentSearchesModel.insert(0, { freeText: freeText, tokensJson: tokensJson })
                        activeRecentIndex = 0
                    }
                }

                function applyRecentSearch(index) {
                    let entry = recentSearchesModel.get(index)
                    if (!entry) {
                        return
                    }
                    let tokens = JSON.parse(entry.tokensJson)
                    selectedCriteria.clear()
                    for (let i = 0; i < tokens.length; i++) {
                        let t = tokens[i]
                        selectedCriteria.append({
                            name: t.name,
                            query: fieldNameToQuery(t.name),
                            value: t.value,
                            exact: t.exact !== undefined ? t.exact : false
                        })
                    }
                    activeRecentIndex = index
                    highlightedRecentIndex = -1
                    searchField.text = entry.freeText !== undefined ? entry.freeText : ""
                    syncSearchUI()
                }

                function clearSearch() {
                    activeRecentIndex = -1
                    highlightedRecentIndex = -1
                    clearAllCriteria()
                    deactivatePane()
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
                        root.activeSidebar.tracklist.search(query)
                    }
                }

                ListModel {
                    id: fieldModel
                    ListElement { name: "Artist"; query: "artist" }
                    ListElement { name: "Album"; query: "album" }
                    ListElement { name: "Title"; query: "title" }
                    ListElement { name: "Genre"; query: "genre" }
                    ListElement { name: "Composer"; query: "composer" }
                    ListElement { name: "Comment"; query: "comment" }
                    ListElement { name: "BPM"; query: "bpm" }
                    ListElement { name: "Key"; query: "key" }
                    ListElement { name: "Year"; query: "year" }
                }

                ListModel {
                    id: selectedCriteria
                }

                ListModel {
                    id: suggestionModel
                }

                states: [
                    State {
                        name: "expanded"
                        when: searchPane.activated

                        PropertyChanges {
                            searchPane.width: 490
                            searchPane.height: 220
                        }
                    }
                ]

                Behavior on width {
                    NumberAnimation {}
                }

                Behavior on height {
                    NumberAnimation {}
                }

                FocusScope {
                    anchors.fill: parent
                    focus: true

                    Keys.onPressed: (event) => {
                        if (event.key == Qt.Key_Escape) {
                            searchPane.persistSearch()
                            searchPane.deactivateSearch()
                            event.accepted = true
                        }
                    }

                    Item {
                        id: collapsedContent

                        visible: !searchPane.activated
                        anchors.fill: parent

                        TapHandler {
                            onTapped: {
                                searchPane.activateSearch()
                            }
                        }

                        Text {
                            visible: !searchPane.hasSearch
                            anchors.left: parent.left
                            anchors.leftMargin: 11
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Search..."
                            color: '#808080'
                        }

                        Row {
                            visible: searchPane.hasSearch
                            anchors.left: parent.left
                            anchors.leftMargin: 11
                            anchors.right: parent.right
                            anchors.rightMargin: 58
                            anchors.verticalCenter: parent.verticalCenter
                            clip: true
                            spacing: 5

                            Repeater {
                                model: selectedCriteria
                                Skin.SearchFieldCriteria {
                                    field: model.name
                                    value: model.value
                                    exact: model.exact
                                    active: false
                                    interactive: false
                                }
                            }

                            Text {
                                visible: searchPane.freeSearchText.length > 0
                                anchors.verticalCenter: parent.verticalCenter
                                text: searchPane.freeSearchText
                                color: '#404040'
                                font.pixelSize: 14
                            }
                        }

                        Shape {
                            visible: !searchPane.hasSearch
                            anchors.right: parent.right
                            anchors.rightMargin: 11
                            anchors.verticalCenter: parent.verticalCenter
                            width: 16
                            height: 16

                            ShapePath {
                                strokeWidth: 2
                                strokeColor: "#808080"
                                fillColor: "transparent"
                                capStyle: ShapePath.RoundCap

                                PathSvg {
                                    path: "M 6 2 A 4 4 0 1 0 6 10 A 4 4 0 1 0 6 2 Z"
                                }
                            }

                            ShapePath {
                                strokeWidth: 2
                                strokeColor: "#808080"
                                capStyle: ShapePath.RoundCap

                                PathSvg {
                                    path: "M 9.5 9.5 L 14 14"
                                }
                            }
                        }
                    }

                    Item {
                        id: expandedContent

                        visible: searchPane.activated
                        anchors.fill: parent

                        TapHandler {
                            onTapped: {
                                if (searchPane.activeTokenIndex >= 0) {
                                    searchPane.setActiveToken(-1)
                                } else {
                                    searchField.forceActiveFocus()
                                }
                            }
                        }

                        Column {
                            anchors.fill: parent

                            Row {
                                id: criteriaRow

                                anchors.left: parent.left
                                anchors.leftMargin: 11

                                width: searchPane.width - 22
                                spacing: 5

                                Repeater {
                                    id: criteriaInput

                                    model: selectedCriteria

                                    Skin.SearchFieldCriteria {
                                        anchors.verticalCenter: parent.verticalCenter
                                        field: model.name
                                        value: model.value
                                        exact: model.exact
                                        active: searchPane.activeTokenIndex === index
                                        onActivated: searchPane.setActiveToken(index)
                                        onDeleted: searchPane.removeToken(index)
                                    }
                                }

                                Text {
                                    visible: searchPane.activeTokenIndex >= 0 && searchPane.freeSearchText.length > 0
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: searchPane.freeSearchText
                                    color: '#404040'
                                    font.pixelSize: 14
                                }

                                Item {
                                    id: freeTextHost

                                    width: searchPane.activeTokenIndex < 0 ? 224 : 0
                                    height: 32
                                }
                            }

                            Rectangle {
                                width: searchPane.width
                                height: 1
                                color: '#757575'
                            }

                            ListView {
                                id: suggestionList

                                width: searchPane.width - 10
                                anchors.margins: 5
                                height: Math.min(suggestionModel.count, 6) * 24
                                visible: suggestionModel.count > 0
                                clip: true
                                interactive: false

                                model: suggestionModel

                                delegate: Item {
                                    height: 24
                                    width: suggestionList.width

                                    Rectangle {
                                        anchors.fill: parent
                                        color: searchPane.highlightedIndex === index ? '#B0B0B0' : 'transparent'
                                    }

                                    TapHandler {
                                        onTapped: {
                                            searchPane.highlightedIndex = index
                                            searchPane.acceptHighlightedSuggestion()
                                        }
                                    }

                                    Skin.SearchFieldCriteria {
                                        visible: isField
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        field: name
                                        active: false
                                        interactive: false
                                    }

                                    Text {
                                        visible: !isField
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        anchors.right: metaLabel.left
                                        anchors.rightMargin: 5
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: display
                                        color: '#404040'
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        id: metaLabel
                                        visible: !isField
                                        anchors.right: parent.right
                                        anchors.rightMargin: 5
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: meta
                                        color: '#808080'
                                        horizontalAlignment: Text.AlignRight
                                    }
                                }
                            }

                            Column {
                                visible: searchPane.isRecentShown()
                                width: searchPane.width - 10
                                anchors.margins: 5
                                spacing: 2

                                Text {
                                    height: 16
                                    text: "Recent searches"
                                    color: '#808080'
                                    font.weight: Font.DemiBold
                                }

                                ListView {
                                    id: recentList

                                    width: searchPane.width - 10
                                    height: 110
                                    clip: true
                                    spacing: 4
                                    interactive: true
                                    model: recentSearchesModel
                                    delegate: Item {
                                        id: recentDelegate

                                        required property int index
                                        required property string tokensJson
                                        required property string freeText

                                        height: 24
                                        width: recentList.width

                                        Rectangle {
                                            anchors.fill: parent
                                            color: searchPane.highlightedRecentIndex === recentDelegate.index ? '#B0B0B0' : 'transparent'
                                        }

                                        TapHandler {
                                            onTapped: searchPane.applyRecentSearch(recentDelegate.index)
                                        }

                                        Row {
                                            property var tokens: JSON.parse(recentDelegate.tokensJson)

                                            anchors.fill: parent
                                            anchors.leftMargin: 5
                                            anchors.rightMargin: 5
                                            anchors.verticalCenter: parent.verticalCenter
                                            spacing: 5

                                            Repeater {
                                                model: parent.tokens
                                                Skin.SearchFieldCriteria {
                                                    field: modelData.name
                                                    value: modelData.value
                                                    exact: modelData.exact !== undefined ? modelData.exact : false
                                                    active: false
                                                    interactive: false
                                                }
                                            }

                                            Text {
                                                visible: recentDelegate.freeText?.length > 0
                                                anchors.verticalCenter: parent.verticalCenter
                                                text: recentDelegate.freeText ?? ""
                                                color: '#404040'
                                                font.pixelSize: 14
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }

                    Text {
                        id: clearButton

                        visible: searchPane.hasSearch
                        anchors.right: parent.right
                        anchors.rightMargin: 13
                        y: searchPane.activated ? 4 : (parent.height - height) / 2
                        text: "✕"
                        color: '#808080'
                        font.pixelSize: 16

                        TapHandler {
                            onTapped: searchPane.clearSearch()
                        }
                    }

                    Item {
                        id: saveButton

                        visible: searchPane.hasSearch && !searchPane.activated
                        anchors.right: clearButton.left
                        anchors.rightMargin: 8
                        y: (parent.height - height) / 2
                        width: 16
                        height: 16

                        TapHandler {
                            onTapped: searchPane.saveCurrentSearch()
                        }

                        Shape {
                            anchors.fill: parent

                            ShapePath {
                                strokeWidth: 1.5
                                strokeColor: "#808080"
                                fillColor: "transparent"
                                capStyle: ShapePath.RoundCap
                                startX: 2
                                startY: 2

                                PathLine { x: 9; y: 2 }
                                PathLine { x: 9; y: 5 }
                                PathLine { x: 14; y: 5 }
                                PathLine { x: 14; y: 14 }
                                PathLine { x: 2; y: 14 }
                                PathLine { x: 2; y: 2 }
                            }

                            ShapePath {
                                fillColor: "#808080"
                                startX: 4
                                startY: 10

                                PathLine { x: 12; y: 10 }
                                PathLine { x: 12; y: 13 }
                                PathLine { x: 4; y: 13 }
                                PathLine { x: 4; y: 10 }
                            }
                        }
                    }

                    TextInput {
                        id: searchField

                        visible: searchPane.activated
                        color: '#404040'
                        clip: true
                        selectByMouse: true
                        font.pixelSize: 14
                        horizontalAlignment: TextInput.AlignLeft
                        verticalAlignment: TextInput.AlignVCenter
                        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhNoFullscreen
                        EnterKey.type: Qt.EnterKeyReturn

                        Keys.onTabPressed: (event) => {
                            if (searchPane.activeTokenIndex < 0) {
                                searchPane.acceptFieldSuggestion()
                            } else {
                                searchPane.setActiveToken(-1)
                            }
                            event.accepted = true
                        }

                        Keys.onBacktabPressed: (event) => {
                            searchPane.navigateBackward()
                            event.accepted = true
                        }

                        Keys.onPressed: (event) => {
                            switch (event.key) {
                            case Qt.Key_Enter:
                            case Qt.Key_Return:
                                if (suggestionModel.count > 0) {
                                    searchPane.acceptHighlightedSuggestion()
                                } else if (searchPane.isRecentShown() && searchPane.highlightedRecentIndex >= 0) {
                                    searchPane.applyRecentSearch(searchPane.highlightedRecentIndex)
                                }
                                searchPane.persistSearch()
                                event.accepted = true
                                break
                            case Qt.Key_Down:
                                if (suggestionModel.count > 0) {
                                    searchPane.highlightedIndex = (searchPane.highlightedIndex + 1) % suggestionModel.count
                                } else if (searchPane.isRecentShown() && recentSearchesModel.count > 0) {
                                    searchPane.highlightedRecentIndex = (searchPane.highlightedRecentIndex + 1) % recentSearchesModel.count
                                    recentList.positionViewAtIndex(searchPane.highlightedRecentIndex, ListView.Contain)
                                }
                                event.accepted = true
                                break
                            case Qt.Key_Up:
                                if (suggestionModel.count > 0) {
                                    searchPane.highlightedIndex = (searchPane.highlightedIndex - 1 + suggestionModel.count) % suggestionModel.count
                                } else if (searchPane.isRecentShown() && recentSearchesModel.count > 0) {
                                    searchPane.highlightedRecentIndex = (searchPane.highlightedRecentIndex - 1 + recentSearchesModel.count) % recentSearchesModel.count
                                    recentList.positionViewAtIndex(searchPane.highlightedRecentIndex, ListView.Contain)
                                }
                                event.accepted = true
                                break
                            case Qt.Key_Left:
                                if (searchField.cursorPosition === 0 && searchPane.activeTokenIndex !== 0) {
                                    searchPane.navigateBackward()
                                    searchField.cursorPosition = searchField.text.length
                                    event.accepted = true
                                }
                                break
                            case Qt.Key_Right:
                                if (searchField.cursorPosition === searchField.text.length && searchPane.activeTokenIndex !== -1) {
                                    searchPane.navigateForward()
                                    searchField.cursorPosition = 0
                                    event.accepted = true
                                }
                                break
                            case Qt.Key_Equal:
                                searchPane.toggleExactMatch()
                                event.accepted = true
                                break
                            case Qt.Key_A:
                                if (event.modifiers & Qt.ControlModifier) {
                                    searchField.selectAll()
                                    if (searchPane.activeTokenIndex < 0) {
                                        searchPane.selectingAll = true
                                    }
                                    event.accepted = true
                                }
                                break
                            case Qt.Key_Backspace:
                                if (searchPane.selectingAll) {
                                    searchPane.clearAllCriteria()
                                    event.accepted = true
                                    break
                                }
                                if (searchField.text.length === 0) {
                                    if (searchPane.activeTokenIndex >= 0) {
                                        searchPane.removeToken(searchPane.activeTokenIndex)
                                    } else if (selectedCriteria.count > 0) {
                                        searchPane.removeToken(selectedCriteria.count - 1)
                                    }
                                    event.accepted = true
                                }
                                break
                            case Qt.Key_Delete:
                                if (searchPane.selectingAll) {
                                    searchPane.clearAllCriteria()
                                    event.accepted = true
                                    break
                                }
                                if (searchPane.activeTokenIndex >= 0) {
                                    searchPane.removeToken(searchPane.activeTokenIndex)
                                    event.accepted = true
                                }
                                break
                            }
                        }

                        onTextEdited: {
                            if (searchPane.selectingAll) {
                                searchPane.clearAllCriteria()
                                return
                            }
                            if (searchPane.activeTokenIndex >= 0) {
                                selectedCriteria.setProperty(searchPane.activeTokenIndex, "value", searchField.text)
                            }
                            searchPane.refreshSuggestions()
                            searchPane.updateSearchQuery()
                            searchPane.updateEditorPosition()
                        }

                        onActiveFocusChanged: {
                            if (!activeFocus) {
                                searchPane.deactivateSearch()
                            }
                        }
                    }

                    Text {
                        id: searchPlaceholder

                        visible: searchPane.activated && searchField.text.length === 0
                        color: '#808080'
                        elide: Text.ElideRight
                        text: {
                            if (searchPane.activeTokenIndex < 0) {
                                return "Start typing to get suggestion"
                            }
                            let exact = selectedCriteria.get(searchPane.activeTokenIndex).exact
                            return exact ? "Press = to disable exact match" : "Press = for an exact match"
                        }

                        x: searchField.x
                        y: searchField.y
                        width: searchField.width
                        height: searchField.height
                        verticalAlignment: Text.AlignVCenter
                    }

                    FontMetrics {
                        id: editorFontMetrics

                        font: searchField.font
                    }

                    Text {
                        id: typingTip

                        readonly property string tip: {
                            if (!searchPane.activated || searchField.text.length === 0) {
                                return ""
                            }
                            if (searchPane.activeTokenIndex < 0) {
                                for (let i = 0; i < suggestionModel.count; i++) {
                                    let f = suggestionModel.get(i)
                                    if (f.isField) {
                                        let pronoun = /^[aeiou]/i.test(f.name) ? "an" : "a"
                                        return 'Press "Tab" to search for ' + pronoun + ' ' + f.name
                                    }
                                }
                                return ""
                            }
                            return 'Press "Tab" to add more criteria'
                        }

                        visible: tip.length > 0
                        color: '#808080'
                        font.italic: true
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: tip

                        x: searchField.x + editorFontMetrics.advanceWidth(searchField.text) + 6
                        y: searchField.y
                        height: searchField.height
                        width: Math.max(0, searchPane.width - x - 12)
                    }
                }
            }
        }
    }
}
