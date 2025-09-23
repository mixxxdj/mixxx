import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import QtQml.Models // DelegateChoice for Qt >= 6.9
import Qt.labs.qmlmodels // DelegateChooser
import "Theme"

import "Deck" as DeckComponent

Item {
    id: root

    required property string group
    property bool editMode: false
    property bool minimized: false
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    readonly property var currentTrack: deckPlayer.currentTrack

    function serializeModel(item) {
        let result = new Array();
        for (let i = 0; i < item.count; i++) {
            let child = item.get(i)
            let object = {
                type: child.type
            }
            if (child.items !== undefined) {
                object.items = serializeModel(child.items)
            }
            result.push(object)
        }
        return result
    }

    Component.onDestruction: {
        // TODO to be saved by setting editor
        console.log(`normal interface for ${root.group}:`, JSON.stringify(serializeModel(itemModel)))
        console.log(`minimized interface for ${root.group}:`, JSON.stringify(serializeModel(minimizedItemModel)))
    }

    Component.onCompleted: {
        // TODO initiate the layer based on the screensize
        itemModel.append({
                "type": "column",
                "items": [{
                        "type": "info"
                    }, {
                        "type": "row",
                        "items": [{
                                "type": "column",
                                "items": [{
                                        "type": "waveformOverview"
                                    }, {
                                        "type": "row",
                                        "items": [{
                                                "type": "column",
                                                "items": [{
                                                        "type": "play"
                                                    }, {
                                                        "type": "cue"
                                                    }
                                                ]
                                            }, {
                                                "type": "column",
                                                "items": [{
                                                        "type": "toolbar" // My also act as tab to pick component that don't fit on the screen
                                                    }, {
                                                        "type": "row",
                                                        "items": [{
                                                                "type": "hotcueAndStem",
                                                                "minWidth": 520
                                                            }, {
                                                                "type": "beatjump",
                                                                "minWidth": 650
                                                            }, {
                                                                "type": "loop"
                                                            }
                                                        ]
                                                    }
                                                ]
                                            }
                                        ]
                                    }
                                ]
                            }, {
                                "type": "column",
                                "minWidth": 800,
                                "items": [{
                                        "type": "spinny"
                                    }, {
                                        "type": "fxAssign"
                                    }
                                ]
                            }, {
                                "type": "tempo",
                                "minWidth": 270
                            }
                        ]
                    }
                ]
            }
        )
        minimizedItemModel.append({
                "type": "row",
                "items": [{
                        "type": "column",
                        "items": [{
                                "type": "play"
                            }, {
                                "type": "cue"
                            }
                        ]
                    }, {
                        "type": "column",
                        "items": [{
                                "type": "info"
                            }, {
                                "type": "waveformOverview"
                            }
                        ]
                    }
                ]
            }
        )
    }

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    Drag.mimeData: {
        let data = {
            "mixxx/player": group
        };
        const trackLocationUrl = currentTrack.trackLocationUrl;
        if (trackLocationUrl)
            data["text/uri-list"] = trackLocationUrl;

        return data;
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    MouseArea {
        id: dragArea

        anchors.fill: root
        drag.target: root
    }

    Skin.SectionBackground {
        anchors.fill: parent
    }

    ListModel {
        id: itemModel
        dynamicRoles: true
    }

    ListModel {
        id: minimizedItemModel
    }

    enum Cardinality {
        North,
        East,
        South,
        West
    }

    component LayoutMouseArea: MouseArea {
        property Item target: parent
        onPressed: (event) => {
            if (event.modifiers & Qt.ControlModifier) {
                let currentParent = target.parent
                while (currentParent != root) {
                    if (currentParent.selected === false) {
                        let updatedRecursive = (children) => {
                            for (let i = 1; i < children.length; i++) {
                                if (children[i].selected === true) children[i].selected = false
                                updatedRecursive(children[i].children)
                            }
                        }
                        updatedRecursive(currentParent.children)
                        currentParent.selected = true
                        break;
                    }
                    currentParent = currentParent.parent
                }
            }

            if (!target.beginDrag) {
                target.beginDrag = Qt.point(target.x, target.y);
            }
            target.z = 100
        }
        onReleased: (event) => {
            if (target.beginDrag) {
                target.x = target.beginDrag.x;
                target.y = target.beginDrag.y;
                target.beginDrag = null
            }
            if (!(event.modifiers & Qt.ControlModifier) && target.selected) {
                target.selected = false
            }

            target.z = 1
        }
    }
    component LayoutContainer: GridLayout {
        id: item

        property var beginDrag: null
        property var move: null

        visible: minWidth ? minWidth < root.width : true

        property var disposition: GridLayout.TopToBottom
        property bool selected: false

        property alias editOverlay: overlay

        Drag.active: mouseArea.drag.active
        Drag.hotSpot: Qt.point(width/2, height/2)

        columns: disposition == GridLayout.TopToBottom ? 1 : items.count

        Repeater {
            model: items
            delegate: deckItemDelegate
        }

        Rectangle {
            id: overlay
            parent: root
            visible: root.editMode && selected
            color: Qt.alpha(Theme.accentColor, 0.2)
            LayoutMouseArea {
                id: mouseArea
                drag.target: item
                target: item
                anchors.fill: parent
            }
        }

        onXChanged: {
            updateOverlayPosition()
        }
        onYChanged: {
            updateOverlayPosition()
        }
        onWidthChanged: {
            updateOverlayPosition()
        }
        onHeightChanged: {
            updateOverlayPosition()
        }

        function updateOverlayPosition() {
            let updatedRecursive;
            let updatedItem = (item) => {
                if (typeof item.selected !== "boolean") return;
                let overlayRect = root.mapFromItem(item, 0, 0, item.width, item.height)
                item.editOverlay.x = overlayRect.x
                item.editOverlay.y = overlayRect.y
                item.editOverlay.width = overlayRect.width
                item.editOverlay.height = overlayRect.height
                updatedRecursive(item.children)
            }
            updatedRecursive = (children) => {
                for (let i = 1; i < children.length; i++) {
                    updatedItem(children[i])
                }
            }
            updatedItem(this)
        }

        function complete() {
            if (item.move) {
                item.move.ref.move(item.move.target, item.move.source - item.move.count + 1, item.move.count)
            }
            item.move = null
        }

        Behavior on x {
            SpringAnimation {
                id: xAnimation
                duration: 500
                spring: 2
                damping: 0.2
                onRunningChanged: {
                    if (running) return;
                    item.complete()
                }
            }
        }
        Behavior on y {
            SpringAnimation {
                id: yAnimation
                duration: 500
                spring: 2
                damping: 0.2
                onRunningChanged: {
                    if (running) return;
                    item.complete()
                }
            }
        }
    }
    component LayoutItem: Item {
        id: item
        property var beginDrag: null
        property var move: null

        property int blurRadius: 0
        property alias editOverlay: overlayItem
        property alias editLabel: labelItem

        Drag.active: mouseArea.drag.active
        Drag.hotSpot: Qt.point(width/2, height/2)

        visible: minWidth ? minWidth < root.width : true

        property alias innerItem: content
        default property alias children: content.children

        Item {
            id: content
            anchors.fill: item
        }

        GaussianBlur {
            visible: root.editMode
            anchors.fill: item
            source: content
            radius: blurRadius
            samples: 16
            deviation: 4
        }

        Rectangle {
            id: overlayItem
            z: 90
            visible: root.editMode
            anchors.fill: item
            color: Qt.alpha("black", 0.4)
            Text {
                id: labelItem
                anchors.centerIn: parent
                visible: !!text.length
            }

            LayoutMouseArea {
                id: mouseArea
                drag.target: item
                target: item
                // enabled: overlayItem.visible
                anchors.fill: overlayItem
            }

            FadeBehavior on visible {
                fadeTarget: overlayItem
            }
        }

        function complete() {
            if (item.move) {
                item.move.ref.move(item.move.target, item.move.source - item.move.count + 1, item.move.count)
            }
            item.move = null
        }

        Behavior on x {
            SpringAnimation {
                id: xAnimation
                duration: 500
                spring: 2
                damping: 0.2
                onRunningChanged: {
                    if (running) return;
                    item.complete()
                }
            }
        }
        Behavior on y {
            SpringAnimation {
                id: yAnimation
                duration: 500
                spring: 2
                damping: 0.2
                onRunningChanged: {
                    if (running) return;
                    item.complete()
                }
            }
        }
    }

    DelegateChooser {
        id: deckItemDelegate
        role: "type"
        DelegateChoice {
            roleValue: "info"
            LayoutItem {
                Layout.fillWidth: true
                Layout.preferredHeight: root.minimized ? 28 : 56

                editOverlay.visible: false

                DeckComponent.InfoBar {
                    id: infoBar

                    anchors.fill: parent

                    minimized: root.minimized
                    group: root.group
                    editMode: root.editMode

                    rightColumnWidth: 105
                }
            }
        }
        DelegateChoice {
            roleValue: "loop"

            LayoutItem {
                Layout.maximumWidth: 185
                Layout.minimumWidth: 120
                Layout.fillWidth: true
                height: 92

                editOverlay.color: Theme.midGray
                editLabel.text: "Loop"
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.color: Theme.white
                DeckComponent.Loop {
                    group: root.group
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            roleValue: "beatjump"

            LayoutItem {
                Layout.preferredWidth: 120
                height: 92

                editOverlay.color: Theme.midGray
                editLabel.text: "Beatjump"
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.color: Theme.white

                DeckComponent.BeatJump {
                    group: root.group
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            roleValue: "waveformOverview"
            LayoutItem {
                Layout.fillWidth: true
                Layout.fillHeight: true

                blurRadius: 4
                editOverlay.color: Qt.alpha('black', 0.5)
                editLabel.text: "Waveform overview"
                editLabel.font.pixelSize: 14
                editLabel.color: Theme.white

                DeckComponent.WaveformOverview {
                    group: root.group
                    currentTrack: root.currentTrack
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            roleValue: "fxAssign"
            LayoutItem {
                id: holder

                required property int index

                width: 135

                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                editOverlay.color: Theme.darkGray2
                editLabel.text: "FX Assign"
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.color: Theme.midGray

                DeckComponent.FXAssign {
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "tempo"
            LayoutItem {
                Layout.preferredWidth: 75

                Layout.fillHeight: true

                blurRadius: 4
                editOverlay.color: Qt.alpha('black', 0.5)
                editLabel.text: "Tempo"
                editLabel.color: Theme.white

                DeckComponent.TempoColumn {
                    group: root.group
                    currentTrack: root.currentTrack
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            roleValue: "play"

            LayoutItem {
                Layout.preferredHeight: root.minimized ? 32 : 60
                Layout.preferredWidth: 60

                editOverlay.color: Theme.darkGray2
                editOverlay.border.color: '#000000'
                editOverlay.border.width: 2
                editLabel.text: "Play\nButton"
                editLabel.horizontalAlignment: Text.AlignHCenter
                editLabel.color: Theme.white

                DeckComponent.PlayButton {
                    group: root.group
                    anchors.fill: parent
                    minimized: root.minimized
                }
            }
        }
        DelegateChoice {
            roleValue: "cue"

            LayoutItem {
                Layout.preferredHeight: root.minimized ? 32 : 60
                Layout.preferredWidth: 60

                editOverlay.color: Theme.darkGray2
                editOverlay.border.color: '#000000'
                editOverlay.border.width: 2
                editLabel.text: "Cue\nButton"
                editLabel.horizontalAlignment: Text.AlignHCenter
                editLabel.color: Theme.white

                DeckComponent.CueButton {
                    group: root.group
                    anchors.fill: parent
                    minimized: root.minimized
                }
            }
        }
        DelegateChoice {
            roleValue: "spinny"
            LayoutItem {
                height: 140
                width: 140

                editOverlay.color: "#BDBDBD"
                editOverlay.radius: height
                editLabel.text: "Spinny"
                editLabel.font.pixelSize: 14
                editLabel.color: Theme.midGray

                Layout.alignment: Qt.AlignTop

                DeckComponent.Spinny {
                    anchors.fill: parent
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "row"
            LayoutContainer {
                Layout.fillHeight: true
                disposition: GridLayout.LeftToRight
            }
        }
        DelegateChoice {
            roleValue: "column"
            LayoutContainer {
                Layout.fillWidth: true
                disposition: GridLayout.TopToBottom
            }
        }
        DelegateChoice {
            roleValue: "toolbar"
            LayoutItem {
                Layout.minimumHeight: 22

                Layout.fillWidth: true

                blurRadius: 4
                editOverlay.color: Qt.alpha('black', 0.5)
                editLabel.text: "Toolbar"
                editLabel.font.pixelSize: 14
                editLabel.color: Theme.white

                DeckComponent.Toolbar {
                    group: root.group
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            roleValue: "hotcueAndStem"
            LayoutItem {
                Layout.fillWidth: true
                Layout.minimumWidth: 250
                height: 92

                blurRadius: 4
                editOverlay.color: Qt.alpha('black', 0.5)
                editLabel.text: "Hotcue & Stem"
                editLabel.font.pixelSize: 14
                editLabel.color: Theme.white

                DeckComponent.HotcueAndStem {
                    group: root.group
                    currentTrack: root.currentTrack
                    anchors.fill: parent
                }
            }
        }
    }

    ColumnLayout {
        id: grid

        anchors.fill: parent
        anchors.margins: 4

        visible: !root.minimized

        Repeater {
            model: itemModel
            delegate: deckItemDelegate
        }

        FadeBehavior on visible {
            fadeTarget: grid
        }
    }

    ColumnLayout {
        id: minimizedGrid

        anchors.fill: parent
        anchors.margins: 4

        visible: root.minimized

        Repeater {
            model: minimizedItemModel
            delegate: deckItemDelegate
        }

        FadeBehavior on visible {
            fadeTarget: minimizedGrid
        }
    }

    Mixxx.PlayerDropArea {
        group: root.group
        anchors.fill: parent

        property var candidate: null

        onEntered: {
            candidate = null
        }

        onPositionChanged: (drag) => {
            let parentArea = drag.source.parent.mapToItem(grid, 0, 0, drag.source.parent.width, drag.source.parent.height)
            let position = Qt.point(drag.x, drag.y)
            if (position.x < parentArea.x || position.x >= parentArea.x + parentArea.width || position.y < parentArea.y || position.y > parentArea.y + parentArea.height) {
                candidate = null
                return
            }

            if (!drag.source.parent.items) {
                console.error(`No items list on the parent of ${drag.source}`)
                candidate = null
                return
            }
            let target = null
            let sourceIdx
            for (let i = 0; i < drag.source.parent.children.length; i++) {
                if (drag.source.parent.children[i] == drag.source) {
                    sourceIdx = i;
                    break
                }
            }
            if (sourceIdx === null) {
                console.error("Cannot find the source item in the parent's children")
                candidate = null
                return
            }
            for (let i = 0; i < drag.source.parent.children.length; i++) {
                if (drag.source.parent.children[i] == drag.source) {
                    continue
                }
                let topParent = root.minimized ? minimizedGrid : grid
                let itemArea = drag.source.parent.children[i].mapToItem(topParent, 0, 0, drag.source.parent.children[i].width, drag.source.parent.children[i].height)

                if (position.x >= itemArea.x && position.x < itemArea.x + itemArea.width && position.y >= itemArea.y && position.y < itemArea.y + itemArea.height) {
                    target = drag.source.parent.children[i]

                    let currentCenter = Qt.point(itemArea.x + itemArea.width/2, itemArea.y + itemArea.height/2)
                    let sourceCenter = drag.source.mapToItem(topParent, drag.source.width/2, drag.source.height/2)

                    if (candidate == null || candidate.target != target) {
                        let distance = Qt.point(Math.abs(sourceCenter.x - currentCenter.x), Math.abs(sourceCenter.y - currentCenter.y))

                        let cardinality
                        if (distance.x >= distance.y) {
                            cardinality = sourceCenter.x > currentCenter.x ? Deck.Cardinality.East : Deck.Cardinality.West
                        } else if (distance.y > distance.x) {
                            cardinality = sourceCenter.y > currentCenter.y ? Deck.Cardinality.South : Deck.Cardinality.North
                        }
                        candidate = {
                            target: target,
                            cardinality: cardinality,
                            completed: false,
                        }
                        return
                    }

                    if (candidate.completed) {
                        return;
                    }

                    switch (candidate.cardinality) {
                        case Deck.Cardinality.West:
                            candidate.completed = sourceCenter.x+ drag.source.width/2 > currentCenter.x
                            break;
                        case Deck.Cardinality.North:
                            candidate.completed = drag.source.y + drag.source.height > currentCenter.y - parent.height/2
                            break;
                        case Deck.Cardinality.East:
                            candidate.completed = sourceCenter.x - drag.source.width/2 < currentCenter.x
                            break;
                        case Deck.Cardinality.South:
                            candidate.completed = drag.source.y < currentCenter.y
                            break;
                    }

                    if (!candidate.completed) {
                        return
                    }

                    let delta = Qt.point(drag.source.beginDrag.x - target.x, drag.source.beginDrag.y - target.y)
                    let currentParent = drag.source.parent;
                    let reverse = []
                    while (currentParent != topParent) {
                        reverse.push(currentParent.index)
                        currentParent = currentParent.parent
                    }
                    let modelRef = root.minimized ? minimizedItemModel : itemModel
                    while (reverse.length > 0) {
                        modelRef = modelRef.get(reverse.pop()).items
                    }
                    let spacing

                    switch (candidate.cardinality) {
                        case Deck.Cardinality.West: {
                            spacing = target.x - drag.source.beginDrag.x - drag.source.width
                            target.x -= drag.source.width + spacing
                            drag.source.beginDrag.x += target.width + spacing

                            if (drag.source.move) {
                                drag.source.move.source++
                                } else {
                                drag.source.move = {
                                    ref: modelRef,
                                    source: i,
                                    target: sourceIdx,
                                    count: 1
                                }
                            }
                        }
                        break;
                        case Deck.Cardinality.East: {
                            spacing = drag.source.beginDrag.x - target.x - target.width
                            target.x += drag.source.width + spacing
                            drag.source.beginDrag.x -= target.width + spacing

                            if (drag.source.move) {
                                drag.source.move.target = i
                                drag.source.move.count++
                                } else {
                                drag.source.move = {
                                    ref: modelRef,
                                    source: sourceIdx,
                                    target: i,
                                    count: 1
                                }
                            }
                        }
                        break;
                        case Deck.Cardinality.North: {
                            spacing = target.y - drag.source.beginDrag.y - drag.source.height
                            target.y = drag.source.beginDrag.y
                            drag.source.beginDrag.y += target.height + spacing

                            if (drag.source.move) {
                                drag.source.move.source++
                                } else {
                                drag.source.move = {
                                    ref: modelRef,
                                    source: i,
                                    target: sourceIdx,
                                    count: 1
                                }
                            }
                        }
                        break;
                        case Deck.Cardinality.South: {
                            spacing = drag.source.beginDrag.y - target.y - target.height
                            drag.source.beginDrag.y = target.y
                            target.y += drag.source.height + spacing

                            if (drag.source.move) {
                                drag.source.move.target = i
                                drag.source.move.count++
                                } else {
                                drag.source.move = {
                                    ref: modelRef,
                                    source: sourceIdx,
                                    target: i,
                                    count: 1
                                }
                            }
                        }
                        break;
                    }
                    break
                }
            }
        }
    }
}
