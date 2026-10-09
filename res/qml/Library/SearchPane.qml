import ".."
import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQml
import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.6
import "../Theme"

// The collapsible library search bar: a strip of committed search criteria
// tokens, the free text editor and the decoration popups for field, value
// and recent search suggestions. Owned by Library.qml, which provides the
// seams (property bindings) and stays the only component that touches the
// active sidebar's applied tracklist search.
Rectangle {
    id: searchPane

    component SearchIcon: Shape {
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
    objectName: "searchPane"

    // Seam: strips the free text editor, so the owner can sync a sidebar's
    // stored search into the pane.
    property alias textField: searchField
    // Seam: the active sidebar, whose tracklist search the pane applies.
    property var activeSidebar
    // Seam: the item that receives focus back when the pane deactivates.
    property Item focusOutTarget

    property bool activated: false
    property int activeTokenIndex: -1
    property int highlightedIndex: -1
    property int suggestionTotal: recentShown
            ? 0
            : suggestionList.count + suggestionFieldList.count
    property int highlightedRecentIndex: -1
    property int activeRecentIndex: -1
    property string freeSearchText: ""
    property bool selectingAll: false

    readonly property bool hasSearch: activeQuery.length > 0

    // Room kept free right of the token strip for the free
    // text editor and the typing tips.
    readonly property int freeTextReserveWidth: 224

    // The selected criteria that carry a value, in insertion
    // order. Derived purely from selectedCriteria: the loop
    // reads count, which registers the model (including row
    // edits) as a dependency of every binding using it.
    readonly property var activeTokens: {
        const tokens = [];
        for (let i = 0; i < selectedCriteria.count; i++) {
            const token = selectedCriteria.get(i);
            if (token.value.length > 0) {
                tokens.push({
                    name: token.name,
                    query: token.query,
                    value: token.value,
                    keyId: token.keyId
                });
            }
        }
        return tokens;
    }

    readonly property string activeQuery: Mixxx.Library.serializeSearchQuery(activeTokens, freeSearchText)

    // The recent-search list replaces the suggestions while the
    // pane is expanded but idle.
    readonly property bool recentShown: activated && activeTokenIndex < 0 && freeSearchText.length === 0 && selectedCriteria.count === 0

    // Field suggestions for the current input: every search
    // field whose name starts with the typed text. Consumed by
    // suggestionFieldList below.
    readonly property var matchingFields: {
        let data = [];
        const text = searchField.text.toLowerCase()
        if (text.length >= 1) {
            for (let i = 0; i < fieldModel.count; i++) {
                const field = fieldModel.get(i)
                if (field.name.toLowerCase().indexOf(text) === 0) {
                    data.push({
                        display: field.name + ":",
                        name: field.name,
                        query: field.query,
                    })
                }
            }
        }
        return data
    }

    border.color: '#757575'
    border.width: 1

    onActivatedChanged: {
        if (activated) {
            Mixxx.Core.addOpenedPopup(searchPane)
        } else {
            Mixxx.Core.removeOpenedPopup(searchPane)
        }
    }

    onActiveTokenIndexChanged: {
        refreshSuggestions()
    }

    onActiveQueryChanged: searchDebounce.restart()

    Connections {
        target: Qt.inputMethod

        function onVisibleChanged() {
            if (!Qt.inputMethod.visible) {
                searchPane.activated = false
            }
        }
    }

    readonly property var recentSearches: Mixxx.Library.recentSearches

    width: 250
    height: 36

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    color: Theme.white
    topLeftRadius: 16

    Shortcut {
        sequence: "Ctrl+F"
        // Do not steal focus via the shortcut when the library pane
        // itself is not visible (e.g. de-maximized behind the decks).
        enabled: searchPane.visible
        context: Qt.WindowShortcut
        onActivated: searchPane.activateSearch()
    }

    Skin.FocusedWidgetControl {
        id: focusedWidgetControl
    }

    function escapeHtmlText(s) {
        return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
    }

    function highlightMatches(text, needle) {
        let result = ""
        let pos = 0
        let lowerText = text.toLowerCase()
        let lowerNeedle = needle.toLowerCase()
        if (needle.length > 0) {
            while (true) {
                let idx = lowerText.indexOf(lowerNeedle, pos)
                if (idx < 0) {
                    break
                }
                result += escapeHtmlText(text.slice(pos, idx)) +
                        "<b>" + escapeHtmlText(text.slice(idx, idx + needle.length)) + "</b>"
                pos = idx + needle.length
            }
        }
        result += escapeHtmlText(text.slice(pos))
        return result
    }

    function commitCurrentEditor() {
        if (activeTokenIndex >= 0) {
            selectedCriteria.setProperty(activeTokenIndex, "value", searchField.text)
        }
    }

    // The floating search field sits on the active token's
    // value editor, or on the free-text area if none is edited.
    readonly property Item activeEditorHost: {
        if (activeTokenIndex < 0 || criteriaInput.count === 0) {
            return freeTextHost
        }
        const item = criteriaInput.itemAt(activeTokenIndex)
        return item ? item.valueEditorHost : freeTextHost
    }

    readonly property real criteriaMaxContentX: Math.max(0, criteriaRow.childrenRect.width - criteriaViewport.width)

    // Drops the cached values and re-queries the C++ model; the
    // result arrives asynchronously via onModelReset below.
    function refreshSuggestions() {
        suggestionDebounce.stop()
        highlightedIndex = -1
        highlightedRecentIndex = -1
        if (activated) {
            if (activeTokenIndex < 0) {
                const prefix = searchField.text.toLowerCase().length >= 1 ? "track" : ""
                Mixxx.Library.searchSuggestions.setQuery(prefix, searchField.text)
            } else {
                const token = selectedCriteria.get(activeTokenIndex)
                if (token) {
                    Mixxx.Library.searchSuggestions.setQuery(token.query, searchField.text)
                }
            }
        }
    }

    function applySearchQuery(query) {
        activeSidebar.tracklist.search = query
    }

    function setActiveToken(index) {
        commitCurrentEditor()
        // Drop a token left empty by the editor that is being
        // left; its removal shifts the remaining indices.
        if (activeTokenIndex >= 0 && activeTokenIndex !== index && selectedCriteria.get(activeTokenIndex).value.length === 0) {
            const removedIndex = activeTokenIndex
            selectedCriteria.remove(removedIndex)
            if (index > removedIndex) {
                index--
            }
        }
        // The text must be in place before activeTokenIndex
        // changes: the change handler rebuilds the suggestions
        // from it.
        if (index >= 0) {
            searchField.text = selectedCriteria.get(index).value
        } else {
            searchField.text = freeSearchText
        }
        searchField.cursorPosition = searchField.text.length
        activeTokenIndex = index
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
            // See setActiveToken: restore the free text before
            // the index change triggers the suggestion rebuild.
            searchField.text = freeSearchText
            searchField.cursorPosition = searchField.text.length
            activeTokenIndex = -1
        } else if (activeTokenIndex > index) {
            activeTokenIndex--
        }
    }

    function acceptHighlightedSuggestion() {
        if (suggestionTotal === 0) {
            return
        }
        let idx = highlightedIndex
        if (idx < 0 || idx >= suggestionTotal) {
            idx = 0
        }
        let isField = idx < suggestionFieldList.count;
        let s = isField ? suggestionFieldList.model[idx] : suggestionList.model.get(idx - suggestionFieldList.count)
        if (activeTokenIndex < 0) {
            if (isField) {
                freeSearchText = ""
                selectedCriteria.append({ name: s.name, query: s.query, value: "", keyId: 0 })
                setActiveToken(selectedCriteria.count - 1)
            } else {
                searchField.text = s.value
                searchField.cursorPosition = searchField.text.length
                freeSearchText = s.value
                refreshSuggestions()
            }
        } else {
            selectedCriteria.setProperty(activeTokenIndex, "keyId", s.keyId || 0)
            searchField.text = s.value
            setActiveToken(-1)
        }
    }

    function acceptFieldSuggestion() {
        if (highlightedIndex >= suggestionFieldList.count) {
            return
        }
        acceptHighlightedSuggestion()
    }

    function tryConvertToFieldToken() {
        if (activeTokenIndex >= 0) {
            return false
        }
        const text = searchField.text
        // Progressive typing aid: a bare "field:" with no value yet commits
        // an empty chip, so the editor can continue with its value. Pastes
        // and restored queries (containing more than this single word) go
        // through the full parser below.
        const bareFieldMatch = /^([a-zA-Z]+):$/.exec(text)
        if (bareFieldMatch) {
            const lowerBareName = bareFieldMatch[1].toLowerCase()
            for (let i = 0; i < fieldModel.count; i++) {
                const field = fieldModel.get(i)
                if (field.name.toLowerCase() !== lowerBareName) {
                    continue
                }
                freeSearchText = ""
                selectedCriteria.append({
                    name: field.name,
                    query: field.query,
                    value: "",
                    keyId: 0
                })
                setActiveToken(selectedCriteria.count - 1)
                return true
            }
            return false
        }
        // The C++ parser implements the chip dialect of the query strings
        // (quote-aware word split, "=" exact marker, quoted arguments,
        // field aliases); unknown fields, negations and fuzzy words are
        // returned as free text. See SearchQueries::parseQuery.
        const parsed = Mixxx.Library.parseSearchQuery(text)
        const leftover = parsed.freeText.split(' ')
                .filter((word) => word.length > 0)
                .join(' ')
        // fieldModel is the source of truth for the chips the UI supports;
        // drop tokens the pane cannot represent.
        const tokens = []
        for (const parsedToken of parsed.tokens) {
            for (let i = 0; i < fieldModel.count; i++) {
                const field = fieldModel.get(i)
                if (field.name.toLowerCase() !== parsedToken.name.toLowerCase()) {
                    continue
                }
                tokens.push({
                    name: field.name,
                    query: field.query,
                    value: parsedToken.value,
                    keyId: parsedToken.keyId
                })
                break
            }
        }
        if (tokens.length === 0) {
            return false
        }
        for (let i = 0; i < tokens.length; i++) {
            selectedCriteria.append(tokens[i])
        }
        if (tokens.length === 1 && leftover.length === 0) {
            // Apply the value that has already been committed and move the
            // editor into it, as with a single hand-typed criterion.
            freeSearchText = ""
            setActiveToken(selectedCriteria.count - 1)
            return true
        }
        // A multi-chip paste conversion, or one that leaves free text
        // behind, stays in free-text mode.
        searchField.text = leftover
        searchField.cursorPosition = leftover.length
        freeSearchText = leftover
        highlightedIndex = -1
        refreshSuggestions()
        return true
    }

    function activateSearch() {
        selectingAll = false
        searchField.text = freeSearchText
        searchField.cursorPosition = searchField.text.length
        syncSearchUI()
    }

    function syncSearchUI() {
        activated = true
        activeTokenIndex = -1
        focusedWidgetControl.value = Skin.FocusedWidgetControl.WidgetKind.Searchbar
        refreshSuggestions()
        searchField.forceActiveFocus()
    }

    function clearAllCriteria() {
        selectingAll = false
        // The pane state is being wiped for a rebuild (sidebar switch,
        // clear button): a pending "apply the wiped state" must not land
        // on whichever sidebar is active when the debounce fires.
        searchDebounce.stop()
        selectedCriteria.clear()
        activeTokenIndex = -1
        searchField.text = ""
        freeSearchText = ""
        refreshSuggestions()
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
        focusOutTarget.forceActiveFocus()
    }

    function persistSearch() {
        commitCurrentEditor()
        if (activeTokens.length === 0 && freeSearchText.length === 0) {
            return
        }
        activeRecentIndex = searchPane.recentSearches.persist(activeTokens, freeSearchText, activeRecentIndex)
    }

    function applyRecentSearch(index) {
        const entry = searchPane.recentSearches.get(index)
        if (!entry) {
            return
        }
        const tokens = entry.tokens
        if (!tokens) {
            return
        }
        selectedCriteria.clear()
        for (let i = 0; i < tokens.length; i++) {
            const token = tokens[i]
            selectedCriteria.append({
                name: token.name,
                query: token.query,
                value: token.value,
                keyId: token.keyId !== undefined ? token.keyId : 0
            })
        }
        activeRecentIndex = index
        highlightedRecentIndex = -1
        searchField.text = entry.freeText !== undefined ? entry.freeText : ""
        freeSearchText = searchField.text
        syncSearchUI()
    }

    function clearSearch() {
        activeRecentIndex = -1
        highlightedRecentIndex = -1
        clearAllCriteria()
        deactivatePane()
    }

    // =========================================================================
    // E2E test harness surface — do not use from UI code
    // =========================================================================
    // The members below exist solely for the behave E2E tests in
    // src/test/behave/, which reach them via the spix RPC API.
    // Test-invoked core members (activated, activeQuery,
    // clearAllCriteria, deactivatePane) stay among the
    // definitions above on purpose.

    readonly property int criteriaCount: selectedCriteria.count

    // Field values of the first `count` rows of a model with an
    // invocable get() (ListModel/abstract model), or of a plain
    // JS array of rows, as a JSON list, in list order. The count
    // is passed in by the caller: method calls inside this
    // function would not register as binding dependencies.
    function jsonFieldList(rows, key, count) {
        const values = [];
        if (typeof rows.get === "function") {
            for (let i = 0; i < count; i++) {
                values.push(rows.get(i)[key]);
            }
        } else {
            for (const row of rows) {
                if (row && row[key]) {
                    values.push(row[key]);
                }
            }
        }
        return JSON.stringify(values);
    }

    readonly property string criteriaFields: jsonFieldList(selectedCriteria, "name", selectedCriteria.count)

    readonly property string suggestionTexts: jsonFieldList(suggestionList.model, "value", suggestionList.count)
    // =========================================================================

    Timer {
        id: searchDebounce

        interval: 800
        repeat: false
        onTriggered: {
            searchPane.applySearchQuery(searchPane.activeQuery)
        }
    }

    Timer {
        id: suggestionDebounce

        interval: 300
        repeat: false
        onTriggered: searchPane.refreshSuggestions()
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
                // Deactivate first so the persist sees the committed
                // editor state instead of the chip being edited.
                searchPane.deactivateSearch()
                searchPane.persistSearch()
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
                id: collapsedPlaceholder
                objectName: "searchCollapsedPlaceholder"

                visible: !searchPane.hasSearch
                anchors.left: parent.left
                anchors.leftMargin: 11
                anchors.verticalCenter: parent.verticalCenter
                text: "Search..."
                color: '#808080'
                font.italic: true
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

            SearchIcon {
                visible: !searchPane.hasSearch
                anchors.right: parent.right
                anchors.rightMargin: 11
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            id: expandedContent

            visible: searchPane.activated
            anchors.fill: parent

            TapHandler {
                onTapped: {
                    searchField.forceActiveFocus()
                }
            }

            Rectangle {
                id: inputRowBackground

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 1
                anchors.leftMargin: 1
                anchors.rightMargin: 1
                height: 31
                color: '#E0E0E0'
                topLeftRadius: 15

                Item {
                    id: criteriaViewport

                    property int contentX: {
                        if (searchPane.activeTokenIndex < 0) {
                            return searchPane.criteriaMaxContentX
                        }
                        const item = criteriaInput.itemAt(searchPane.activeTokenIndex)
                        if (!item) {
                            return 0
                        }
                        const gradientWidth = criteriaGradient.width
                        let x = 0;
                        if (item.x < x + gradientWidth) {
                            // Hidden behind the left gradient: scroll to the
                            // right, but keep the right edge of the token in
                            // view.
                            x = Math.max(item.x - gradientWidth, item.x + item.width - width)
                        } else if (item.x + item.width > x + width) {
                            // Hidden at the right edge: scroll to the left.
                            x = item.x + item.width
                        }
                        return Math.min(Math.max(x, 0), searchPane.criteriaMaxContentX)
                    }

                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    width: Math.min(criteriaRow.childrenRect.width
                            + (criteriaRow.childrenRect.width > 0 ? criteriaRow.spacing : 0),
                            parent.width - 22 - searchPane.freeTextReserveWidth)
                    clip: true

                    Row {
                        id: criteriaRow

                        x: -criteriaViewport.contentX
                        height: parent.height
                        spacing: 5

                        Repeater {
                            id: criteriaInput

                            model: selectedCriteria

                            Skin.SearchFieldCriteria {
                                anchors.verticalCenter: parent.verticalCenter
                                field: model.name
                                value: model.value
                                active: searchPane.activeTokenIndex === index
                                onActivated: searchPane.setActiveToken(index)
                                onDeleted: searchPane.removeToken(index)
                            }
                        }
                    }

                    // Opaque gradient that hints at tokens
                    // scrolled out of view on the left.
                    Rectangle {
                        id: criteriaGradient

                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 24
                        height: parent.height
                        visible: criteriaViewport.contentX > 0

                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 0.0
                                color: '#E0E0E0'
                            }
                            GradientStop {
                                position: 1.0
                                color: '#00E0E0E0'
                            }
                        }
                    }
                }

                Row {
                    id: freeZone

                    x: criteriaViewport.x + criteriaViewport.width
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 5

                    Text {
                        visible: searchPane.activeTokenIndex >= 0 && searchPane.freeSearchText.length > 0
                        anchors.verticalCenter: parent.verticalCenter
                        text: searchPane.freeSearchText
                        color: '#404040'
                        font.pixelSize: 14
                    }

                    Item {
                        id: freeTextHost

                        width: searchPane.activeTokenIndex < 0 ? searchPane.freeTextReserveWidth : 0
                        height: 32
                    }
                }

                SearchIcon {
                    id: expandedSearchIcon

                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Column {
                anchors.fill: parent
                anchors.topMargin: 32

                Rectangle {
                    width: searchPane.width
                    height: 1
                    color: '#757575'
                }

                ListView {
                    id: suggestionFieldList
                    objectName: "searchSuggestionFieldList"

                    width: searchPane.width - 10
                    anchors.margins: 5
                    height: Math.min(count, 2) * 28
                    visible: count > 0 && !searchPane.recentShown
                    clip: true
                    interactive: false

                    model: searchPane.matchingFields

                    delegate: Item {
                        objectName: "suggestion_"
                                + display

                        required property int index
                        required property string display
                        required property string name
                        required property string query

                        height: 28
                        width: suggestionFieldList.width

                        HoverHandler {
                            id: fieldSuggestionHover
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: searchPane.highlightedIndex === parent.index
                                    ? Theme.lightGray2
                                    : (fieldSuggestionHover.hovered ? '#C6C6C6' : 'transparent')
                        }

                        TapHandler {
                            onTapped: {
                                searchPane.highlightedIndex = parent.index
                                searchPane.acceptHighlightedSuggestion()
                            }
                        }

                        Skin.SearchFieldCriteria {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 5
                            field: parent.name
                            active: false
                            interactive: false
                        }
                    }
                }
                ListView {
                    id: suggestionList
                    objectName: "searchSuggestionList"

                    width: searchPane.width - 10
                    anchors.margins: 5
                    height: Math.min(count, 6) * 24
                    visible: count > 0 && !searchPane.recentShown
                    clip: true
                    interactive: false

                    model: Mixxx.Library.searchSuggestions

                    delegate: Item {
                        id: suggestionDelegate

                        objectName: "suggestion_"
                                + suggestionDelegate.value

                        required property int index
                        required property string value
                        required property string label
                        required property int keyId
                        required property color keyColor

                        height: 24
                        width: suggestionList.width

                        HoverHandler {
                            id: suggestionHover
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: searchPane.highlightedIndex - suggestionFieldList.count === suggestionDelegate.index
                                    ? Theme.lightGray2
                                    : (suggestionHover.hovered ? '#C6C6C6' : 'transparent')
                        }

                        TapHandler {
                            onTapped: {
                                searchPane.highlightedIndex = suggestionDelegate.index + suggestionFieldList.count
                                searchPane.acceptHighlightedSuggestion()
                            }
                        }

                        Rectangle {
                            visible: suggestionDelegate.label.length > 0
                            anchors.left: parent.left
                            anchors.leftMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            width: keyPillText.implicitWidth + 12
                            height: 18
                            radius: 5
                            color: suggestionDelegate.keyColor

                            Text {
                                id: keyPillText
                                anchors.centerIn: parent
                                text: suggestionDelegate.value
                                color: '#FFFFFF'
                                font.pixelSize: 11
                            }
                        }

                        Text {
                            visible: suggestionDelegate.label.length === 0
                            anchors.left: parent.left
                            anchors.leftMargin: 5
                            anchors.right: metaLabel.left
                            anchors.rightMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            text: searchPane.highlightMatches(
                                      suggestionDelegate.value,
                                      searchField.text)
                            textFormat: Text.StyledText
                            color: '#404040'
                            elide: Text.ElideRight
                        }

                        Text {
                            id: metaLabel
                            anchors.right: parent.right
                            anchors.rightMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            text: suggestionDelegate.label
                            color: '#808080'
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                Column {
                    visible: searchPane.recentShown
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
                        objectName: "searchRecentList"

                        width: searchPane.width - 10
                        height: 110
                        clip: true
                        spacing: 4
                        interactive: true
                        model: searchPane.recentSearches
                        delegate: Item {
                            id: recentDelegate

                            objectName: "recent_"
                                + recentDelegate.index

                            required property int index
                            required property var tokens
                            required property string freeText
                            required property string queryString

                            // E2E: see "test harness surface" on searchPane above.
                            readonly property string tokenFieldNames: searchPane.jsonFieldList(recentDelegate.tokens, "name")

                            height: 24
                            width: recentList.width

                            HoverHandler {
                                id: recentHover
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: searchPane.highlightedRecentIndex === recentDelegate.index
                                        ? Theme.lightGray2
                                        : (recentHover.hovered ? '#C6C6C6' : 'transparent')
                            }

                            TapHandler {
                                onTapped: searchPane.applyRecentSearch(recentDelegate.index)
                            }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 5

                                Repeater {
                                    model: recentDelegate.tokens
                                    Skin.SearchFieldCriteria {
                                        objectName: "recentToken_" + index + "_" + modelData.name
                                        field: modelData.name
                                        value: modelData.value
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
            objectName: "searchClearButton"

            visible: searchPane.hasSearch
            anchors.right: parent.right
            anchors.rightMargin: searchPane.activated ? 36 : 13
            y: 7
            text: "✕"
            color: '#808080'
            font.pixelSize: 16

            TapHandler {
                onTapped: searchPane.clearSearch()
            }
        }

        TextInput {
            id: searchField
            objectName: "searchField"

            width: searchPane.activeEditorHost.width
            height: 31

            // The discarded geometric arithmetic registers
            // geometry changes as binding dependencies.
            readonly property var geometryWatchdog: {
                let activeEditorHost = searchPane.activeEditorHost;
                criteriaViewport.x + criteriaViewport.width + activeEditorHost.x + activeEditorHost.width;
                return searchPane.activeEditorHost.mapToItem(parent, 0, 0)
            }
            x: geometryWatchdog.x


            visible: searchPane.activated
            color: '#404040'
            clip: true
            selectByMouse: true
            font.pixelSize: 14
            horizontalAlignment: TextInput.AlignLeft
            verticalAlignment: TextInput.AlignVCenter
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhNoFullscreen
            EnterKey.type: Qt.EnterKeyReturn

            // A click that lands inside the field moves the cursor
            // and must leave the "wipe everything" armed state.
            TapHandler {
                onTapped: searchPane.selectingAll = false
            }

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
                    if (searchPane.suggestionTotal > 0) {
                        searchPane.acceptHighlightedSuggestion()
                    } else if (searchPane.recentShown && searchPane.highlightedRecentIndex >= 0) {
                        searchPane.applyRecentSearch(searchPane.highlightedRecentIndex)
                    }
                    searchPane.persistSearch()
                    event.accepted = true
                    break
                case Qt.Key_Down:
                    if (searchPane.suggestionTotal > 0) {
                        searchPane.highlightedIndex = (searchPane.highlightedIndex + 1) % searchPane.suggestionTotal
                    } else if (searchPane.recentShown && searchPane.recentSearches.rowCount() > 0) {
                        searchPane.highlightedRecentIndex = (searchPane.highlightedRecentIndex + 1) % searchPane.recentSearches.rowCount()
                        recentList.positionViewAtIndex(searchPane.highlightedRecentIndex, ListView.Contain)
                    }
                    event.accepted = true
                    break
                case Qt.Key_Up:
                    if (searchPane.suggestionTotal > 0) {
                        searchPane.highlightedIndex = (searchPane.highlightedIndex - 1 + searchPane.suggestionTotal) % searchPane.suggestionTotal
                    } else if (searchPane.recentShown && searchPane.recentSearches.rowCount() > 0) {
                        searchPane.highlightedRecentIndex = (searchPane.highlightedRecentIndex - 1 + searchPane.recentSearches.rowCount()) % searchPane.recentSearches.rowCount()
                        recentList.positionViewAtIndex(searchPane.highlightedRecentIndex, ListView.Contain)
                    }
                    event.accepted = true
                    break
                case Qt.Key_Left:
                    // Leaving the "wipe everything" armed state by
                    // moving the cursor must not displace it onto the
                    // next keystroke.
                    searchPane.selectingAll = false
                    if (searchField.cursorPosition === 0 && searchPane.activeTokenIndex !== 0) {
                        searchPane.navigateBackward()
                        searchField.cursorPosition = searchField.text.length
                        event.accepted = true
                    }
                    break
                case Qt.Key_Right:
                    searchPane.selectingAll = false
                    if (searchField.cursorPosition === searchField.text.length && searchPane.activeTokenIndex !== -1) {
                        searchPane.navigateForward()
                        searchField.cursorPosition = 0
                        event.accepted = true
                    }
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
                    selectedCriteria.setProperty(searchPane.activeTokenIndex, "keyId", 0)
                } else if (searchPane.tryConvertToFieldToken()) {
                    return
                } else {
                    searchPane.freeSearchText = searchField.text
                }
                suggestionDebounce.restart()
                searchPane.highlightedIndex = -1
            }

            onActiveFocusChanged: {
                if (!activeFocus) {
                    searchPane.deactivateSearch()
                }
            }
        }

        Text {
            id: searchPlaceholder
            objectName: "searchPlaceholder"

            visible: searchPane.activated && searchField.text.length === 0
            color: '#808080'
            font.italic: true
            elide: Text.ElideRight
            text: {
                if (searchPane.activeTokenIndex < 0) {
                    return "Start typing to get suggestion"
                }
                return 'Type "=" for an exact match'
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
            objectName: "searchTabHint"

            readonly property string tip: {
                if (!searchPane.activated || searchField.text.length === 0) {
                    return ""
                }
                if (searchPane.activeTokenIndex < 0) {
                    // Field matches are rebuilt first, so the
                    // head of the list is the top field match.
                    if (suggestionFieldList.count > 0) {
                        const name = suggestionFieldList.model[0].name
                        const pronoun = /^[aeiou]/i.test(name) ? "an" : "a"
                        return 'Press "Tab" to search for ' + pronoun + ' ' + name
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

            x: {
                freeZone.x
                if (searchPane.activeTokenIndex < 0) {
                    return searchField.x + editorFontMetrics.advanceWidth(searchField.text) + 6
                }
                let p = freeTextHost.mapToItem(searchField.parent, 0, 0)
                return p.x + 6
            }
            y: 0
            height: searchField.height
            // Never overlap the search icon on the right; the tip
            // stays within the free text area.
            width: Math.max(0, expandedSearchIcon.mapToItem(searchField.parent, 0, 0).x - x - 6)
        }
    }
}
