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

    enum Cardinality {
        North,
        East,
        South,
        West
    }

    readonly property var currentTrack: deckPlayer.currentTrack
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property bool editMode: false
    required property string group
    property bool minimized: false

    signal toggleFocus

    function serializeModel(item) {
        let result = new Array();
        for (let i = 0; i < item.count; i++) {
            let child = item.get(i);
            let object = {
                type: child.type
            };
            if (child.items !== undefined) {
                object.items = serializeModel(child.items);
            }
            result.push(object);
        }
        return result;
    }

    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        let data = {
            "mixxx/player": group
        };
        const trackLocationUrl = currentTrack.trackLocationUrl;
        if (trackLocationUrl)
            data["text/uri-list"] = trackLocationUrl;

        return data;
    }
    Drag.supportedActions: Qt.CopyAction

    Component.onCompleted: {
        itemModel.append({
            "type": "column",
            "items": [
                {
                    "type": "info"
                },
                {
                    "type": "row",
                    "items": [
                        {
                            "type": "column",
                            "items": [
                                {
                                    "type": "waveformOverview"
                                },
                                {
                                    "type": "row",
                                    "items": [
                                        {
                                            "type": "column",
                                            "items": [
                                                {
                                                    "type": "play"
                                                },
                                                {
                                                    "type": "cue"
                                                }
                                            ]
                                        },
                                        {
                                            "type": "column",
                                            "items": [
                                                {
                                                    "type": "toolbar"
                                                },
                                                {
                                                    "type": "row",
                                                    "items": [
                                                        {
                                                            "type": "hotcueAndStem"
                                                        },
                                                        {
                                                            "type": "beatjump"
                                                        },
                                                        {
                                                            "type": "loop"
                                                        }
                                                    ]
                                                }
                                            ]
                                        }
                                    ]
                                }
                            ]
                        },
                        {
                            "type": "column",
                            "items": [
                                {
                                    "type": "spinny"
                                },
                                {
                                    "type": "fxAssign"
                                }
                            ]
                        },
                        {
                            "type": "tempo"
                        }
                    ]
                }
            ]
        });
        minimizedItemModel.append({
            "type": "row",
            "items": [
                {
                    "type": "column",
                    "items": [
                        {
                            "type": "play"
                        },
                        {
                            "type": "cue"
                        }
                    ]
                },
                {
                    "type": "column",
                    "items": [
                        {
                            "type": "info"
                        },
                        {
                            "type": "waveformOverview"
                        }
                    ]
                }
            ]
        });
    }
    Component.onDestruction: {
        // TODO to be saved by setting editor
        console.log(`normal interface for ${root.group}:`, JSON.stringify(serializeModel(itemModel)));
        console.log(`minimized interface for ${root.group}:`, JSON.stringify(serializeModel(minimizedItemModel)));
    }

    Skin.SectionBackground {
        anchors.fill: parent
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

    }
    ListModel {
        id: minimizedItemModel

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
                    editMode: root.editMode
                    group: root.group
                    minimized: root.minimized
                    rightColumnWidth: 105

                    TapHandler {
                        onDoubleTapped: root.toggleFocus()
                    }
                }
            }
        }
        DelegateChoice {
            roleValue: "loop"

            LayoutItem {
                Layout.fillWidth: true
                Layout.maximumWidth: 185
                Layout.minimumWidth: 120
                editLabel.color: Theme.white
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.text: "Loop"
                editOverlay.color: Theme.midGray
                height: 92

                DeckComponent.Loop {
                    anchors.fill: parent
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "beatjump"

            LayoutItem {
                Layout.preferredWidth: 120
                editLabel.color: Theme.white
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.text: "Beatjump"
                editOverlay.color: Theme.midGray
                height: 92

                DeckComponent.BeatJump {
                    anchors.fill: parent
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "waveformOverview"

            LayoutItem {
                Layout.fillHeight: true
                Layout.fillWidth: true
                blurRadius: 4
                editLabel.color: Theme.white
                editLabel.font.pixelSize: 14
                editLabel.text: "Waveform overview"
                editOverlay.color: Qt.alpha('black', 0.5)

                DeckComponent.WaveformOverview {
                    anchors.fill: parent
                    currentTrack: root.currentTrack
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "fxAssign"

            LayoutItem {
                id: holder

                required property int index

                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                editLabel.color: Theme.midGray
                editLabel.font.capitalization: Font.AllUppercase
                editLabel.text: "FX Assign"
                editOverlay.color: Theme.darkGray2
                width: 135

                DeckComponent.FXAssign {
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "tempo"

            LayoutItem {
                Layout.fillHeight: true
                Layout.preferredWidth: 75
                blurRadius: 4
                editLabel.color: Theme.white
                editLabel.text: "Tempo"
                editOverlay.color: Qt.alpha('black', 0.5)

                DeckComponent.TempoColumn {
                    anchors.fill: parent
                    currentTrack: root.currentTrack
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "play"

            LayoutItem {
                Layout.preferredHeight: root.minimized ? 32 : 60
                Layout.preferredWidth: 60
                editLabel.color: Theme.white
                editLabel.horizontalAlignment: Text.AlignHCenter
                editLabel.text: "Play\nButton"
                editOverlay.border.color: '#000000'
                editOverlay.border.width: 2
                editOverlay.color: Theme.darkGray2

                DeckComponent.PlayButton {
                    anchors.fill: parent
                    group: root.group
                    minimized: root.minimized
                }
            }
        }
        DelegateChoice {
            roleValue: "cue"

            LayoutItem {
                Layout.preferredHeight: root.minimized ? 32 : 60
                Layout.preferredWidth: 60
                editLabel.color: Theme.white
                editLabel.horizontalAlignment: Text.AlignHCenter
                editLabel.text: "Cue\nButton"
                editOverlay.border.color: '#000000'
                editOverlay.border.width: 2
                editOverlay.color: Theme.darkGray2

                DeckComponent.CueButton {
                    anchors.fill: parent
                    group: root.group
                    minimized: root.minimized
                }
            }
        }
        DelegateChoice {
            roleValue: "spinny"

            LayoutItem {
                Layout.alignment: Qt.AlignCenter
                editLabel.color: Theme.midGray
                editLabel.font.pixelSize: 14
                editLabel.text: "Spinny"
                editOverlay.color: "#BDBDBD"
                editOverlay.radius: height
                height: 130
                width: 130

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
                Layout.fillWidth: true
                Layout.minimumHeight: 22
                blurRadius: 4
                editLabel.color: Theme.white
                editLabel.font.pixelSize: 14
                editLabel.text: "Toolbar"
                editOverlay.color: Qt.alpha('black', 0.5)

                DeckComponent.Toolbar {
                    anchors.fill: parent
                    group: root.group
                }
            }
        }
        DelegateChoice {
            roleValue: "hotcueAndStem"

            LayoutItem {
                Layout.fillWidth: true
                Layout.minimumWidth: 250
                blurRadius: 4
                editLabel.color: Theme.white
                editLabel.font.pixelSize: 14
                editLabel.text: "Hotcue & Stem"
                editOverlay.color: Qt.alpha('black', 0.5)
                height: 92

                DeckComponent.HotcueAndStem {
                    anchors.fill: parent
                    currentTrack: root.currentTrack
                    group: root.group
                }
            }
        }
    }
    ColumnLayout {
        id: grid

        anchors.fill: parent
        anchors.margins: 4
        visible: !root.minimized

        FadeBehavior on visible {
            fadeTarget: grid
        }

        Repeater {
            delegate: deckItemDelegate
            model: itemModel
        }
    }
    ColumnLayout {
        id: minimizedGrid

        anchors.fill: parent
        anchors.margins: 4
        visible: root.minimized

        FadeBehavior on visible {
            fadeTarget: minimizedGrid
        }

        Repeater {
            delegate: deckItemDelegate
            model: minimizedItemModel
        }
    }
    Mixxx.PlayerDropArea {
        property var candidate: null

        function handleCardinalityEast(drag, modelRef, child, i, sourceIdx) {
            let spacing = drag.source.beginDrag.x - child.x - child.width;
            child.x += drag.source.width + spacing;
            drag.source.beginDrag.x -= child.width + spacing;

            if (drag.source.move) {
                drag.source.move.child = i;
                drag.source.move.count++;
            } else {
                drag.source.move = {
                    ref: modelRef,
                    source: sourceIdx,
                    target: i,
                    count: 1
                };
            }
        }
        function handleCardinalityNorth(drag, modelRef, child, i, sourceIdx) {
            let spacing = child.y - drag.source.beginDrag.y - drag.source.height;
            child.y = drag.source.beginDrag.y;
            drag.source.beginDrag.y += child.height + spacing;

            if (drag.source.move) {
                drag.source.move.source++;
            } else {
                drag.source.move = {
                    ref: modelRef,
                    source: i,
                    target: sourceIdx,
                    count: 1
                };
            }
        }
        function handleCardinalitySouth(drag, modelRef, child, i, sourceIdx) {
            let spacing = drag.source.beginDrag.y - child.y - child.height;
            drag.source.beginDrag.y = child.y;
            child.y += drag.source.height + spacing;

            if (drag.source.move) {
                drag.source.move.target = i;
                drag.source.move.count++;
            } else {
                drag.source.move = {
                    ref: modelRef,
                    source: sourceIdx,
                    target: i,
                    count: 1
                };
            }
        }
        function handleCardinalityWest(drag, modelRef, child, i, sourceIdx) {
            let spacing = child.x - drag.source.beginDrag.x - drag.source.width;
            child.x -= drag.source.width + spacing;
            drag.source.beginDrag.x += child.width + spacing;

            if (drag.source.move) {
                drag.source.move.source++;
            } else {
                drag.source.move = {
                    ref: modelRef,
                    source: i,
                    target: sourceIdx,
                    count: 1
                };
            }
        }
        function handleChild(i, child, sourceIdx, position) {
            let topParent = root.minimized ? minimizedGrid : grid;
            let itemArea = child.mapToItem(topParent, 0, 0, child.width, child.height);

            if (position.x >= itemArea.x && position.x < itemArea.x + itemArea.width && position.y >= itemArea.y && position.y < itemArea.y + itemArea.height) {
                let currentCenter = Qt.point(itemArea.x + itemArea.width / 2, itemArea.y + itemArea.height / 2);
                let sourceCenter = drag.source.mapToItem(topParent, drag.source.width / 2, drag.source.height / 2);

                if (candidate == null || candidate.target != child) {
                    let distance = Qt.point(Math.abs(sourceCenter.x - currentCenter.x), Math.abs(sourceCenter.y - currentCenter.y));

                    let cardinality;
                    if (distance.x >= distance.y) {
                        cardinality = sourceCenter.x > currentCenter.x ? Deck.Cardinality.East : Deck.Cardinality.West;
                    } else {
                        cardinality = sourceCenter.y > currentCenter.y ? Deck.Cardinality.South : Deck.Cardinality.North;
                    }
                    candidate = {
                        target: child,
                        cardinality: cardinality,
                        completed: false
                    };
                    return true;
                }

                if (candidate.completed) {
                    return true;
                }

                switch (candidate.cardinality) {
                case Deck.Cardinality.West:
                    candidate.completed = sourceCenter.x + drag.source.width / 2 > currentCenter.x;
                    break;
                case Deck.Cardinality.North:
                    candidate.completed = drag.source.y + drag.source.height > currentCenter.y - parent.height / 2;
                    break;
                case Deck.Cardinality.East:
                    candidate.completed = sourceCenter.x - drag.source.width / 2 < currentCenter.x;
                    break;
                case Deck.Cardinality.South:
                    candidate.completed = drag.source.y < currentCenter.y;
                    break;
                default:
                    console.error(`Unhandled cardinality value: '${candidate.cardinality}'`);
                    return true;
                }

                if (!candidate.completed) {
                    return true;
                }

                let delta = Qt.point(drag.source.beginDrag.x - child.x, drag.source.beginDrag.y - child.y);
                let currentParent = drag.source.parent;
                let reverse = [];
                while (currentParent != topParent) {
                    reverse.push(currentParent.index);
                    currentParent = currentParent.parent;
                }
                let modelRef = root.minimized ? minimizedItemModel : itemModel;
                while (reverse.length > 0) {
                    modelRef = modelRef.get(reverse.pop()).items;
                }

                switch (candidate.cardinality) {
                case Deck.Cardinality.West:
                    handleCardinalityWest(drag, modelRef, child, i, sourceIdx);
                    break;
                case Deck.Cardinality.East:
                    handleCardinalityEast(drag, modelRef, child, i, sourceIdx);
                    break;
                case Deck.Cardinality.North:
                    handleCardinalityNorth(drag, modelRef, child, i, sourceIdx);
                    break;
                case Deck.Cardinality.South:
                    handleCardinalitySouth(drag, modelRef, child, i, sourceIdx);
                    break;
                }
                return true;
            }
        }

        anchors.fill: parent
        group: root.group

        onEntered: {
            candidate = null;
        }
        onPositionChanged: drag => {
            if (drag.formats.includes("mixxx/player")) {
                return;
            }

            let parentArea = drag.source.parent.mapToItem(grid, 0, 0, drag.source.parent.width, drag.source.parent.height);
            let position = Qt.point(drag.x, drag.y);
            if (position.x < parentArea.x || position.x >= parentArea.x + parentArea.width || position.y < parentArea.y || position.y > parentArea.y + parentArea.height) {
                candidate = null;
                return;
            }

            if (!drag.source.parent.items) {
                console.error(`No items list on the parent of ${drag.source}`);
                candidate = null;
                return;
            }
            let sourceIdx;
            for (let i = 0; i < drag.source.parent.children.length; i++) {
                if (drag.source.parent.children[i] == drag.source) {
                    sourceIdx = i;
                    break;
                }
            }
            if (sourceIdx === null) {
                console.error("Cannot find the source item in the parent's children");
                candidate = null;
                return;
            }
            for (let i = 0; i < drag.source.parent.children.length; i++) {
                if (drag.source.parent.children[i] == drag.source) {
                    continue;
                }
                if (handleChild(i, drag.source.parent.children[i], sourceIdx, position))
                    break;
            }
        }
    }

    component LayoutContainer: GridLayout {
        id: item

        property var beginDrag: null
        property var disposition: GridLayout.TopToBottom
        property alias editOverlay: overlay
        required property int index
        required property var items
        property var move: null
        property bool selected: false

        function complete() {
            if (item.move) {
                item.move.ref.move(item.move.target, item.move.source - item.move.count + 1, item.move.count);
            }
            item.move = null;
        }
        function updateOverlayPosition() {
            let updatedRecursive;
            let updatedItem = item => {
                if (typeof item.selected !== "boolean")
                    return;
                let overlayRect = root.mapFromItem(item, 0, 0, item.width, item.height);
                item.editOverlay.x = overlayRect.x;
                item.editOverlay.y = overlayRect.y;
                item.editOverlay.width = overlayRect.width;
                item.editOverlay.height = overlayRect.height;
                updatedRecursive(item.children);
            };
            updatedRecursive = children => {
                for (let i = 1; i < children.length; i++) {
                    updatedItem(children[i]);
                }
            };
            updatedItem(this);
        }

        Drag.active: mouseArea.drag.active
        Drag.hotSpot: Qt.point(width / 2, height / 2)
        columns: disposition == GridLayout.TopToBottom ? 1 : items.count

        Behavior on x {
            SpringAnimation {
                id: xAnimation

                damping: 0.2
                duration: 500
                spring: 2

                onRunningChanged: {
                    if (running)
                        return;
                    item.complete();
                }
            }
        }
        Behavior on y {
            SpringAnimation {
                id: yAnimation

                damping: 0.2
                duration: 500
                spring: 2

                onRunningChanged: {
                    if (running)
                        return;
                    item.complete();
                }
            }
        }

        onHeightChanged: {
            updateOverlayPosition();
        }
        onWidthChanged: {
            updateOverlayPosition();
        }
        onXChanged: {
            updateOverlayPosition();
        }
        onYChanged: {
            updateOverlayPosition();
        }

        Repeater {
            delegate: deckItemDelegate
            model: parent.visible ? items : []
        }
        Rectangle {
            id: overlay

            color: Qt.alpha(Theme.accentColor, 0.2)
            parent: root
            visible: root.editMode && selected

            LayoutMouseArea {
                id: mouseArea

                anchors.fill: parent
                drag.target: item
                target: item
            }
        }
    }
    component LayoutItem: Item {
        id: layoutItem

        property var beginDrag: null
        property int blurRadius: 0
        default property var contentChildren
        property alias editLabel: labelItem
        property alias editOverlay: overlayItem
        property alias innerItem: content
        property var move: null

        function complete() {
            if (layoutItem.move) {
                layoutItem.move.ref.move(layoutItem.move.target, layoutItem.move.source - layoutItem.move.count + 1, layoutItem.move.count);
            }
            layoutItem.move = null;
        }

        Drag.active: mouseArea.drag.active
        Drag.hotSpot: Qt.point(width / 2, height / 2)

        Behavior on x {
            SpringAnimation {
                id: xAnimation

                damping: 0.2
                duration: 500
                spring: 2

                onRunningChanged: {
                    if (running)
                        return;
                    layoutItem.complete();
                }
            }
        }
        Behavior on y {
            SpringAnimation {
                id: yAnimation

                damping: 0.2
                duration: 500
                spring: 2

                onRunningChanged: {
                    if (running)
                        return;
                    layoutItem.complete();
                }
            }
        }

        Loader {
            id: content

            active: layoutItem.visible
            anchors.fill: layoutItem

            sourceComponent: Component {
                Item {
                    children: layoutItem.contentChildren
                }
            }
        }
        GaussianBlur {
            anchors.fill: layoutItem
            deviation: 4
            radius: blurRadius
            samples: 16
            source: content
            visible: root.editMode
        }
        Rectangle {
            id: overlayItem

            anchors.fill: layoutItem
            color: Qt.alpha("black", 0.4)
            visible: root.editMode
            z: 90

            FadeBehavior on visible {
                fadeTarget: overlayItem
            }

            Text {
                id: labelItem

                anchors.centerIn: parent
                visible: !!text.length
            }
            LayoutMouseArea {
                id: mouseArea

                // enabled: overlayItem.visible
                anchors.fill: overlayItem
                drag.target: layoutItem
                target: layoutItem
            }
        }
    }
    component LayoutMouseArea: MouseArea {
        property Item target: parent

        function selectParent() {
            let currentParent = target.parent;
            while (currentParent != root) {
                if (currentParent.selected === false) {
                    let updatedRecursive = children => {
                        for (let i = 1; i < children.length; i++) {
                            if (children[i].selected === true)
                                children[i].selected = false;
                            updatedRecursive(children[i].children);
                        }
                    };
                    updatedRecursive(currentParent.children);
                    currentParent.selected = true;
                    break;
                }
                currentParent = currentParent.parent;
            }
        }

        onPressAndHold: () => {
            selectParent();
            if (!target.beginDrag) {
                target.beginDrag = Qt.point(target.x, target.y);
            }
            target.z = 100;
        }
        onPressed: event => {
            if (event.modifiers & Qt.ControlModifier) {
                selectParent();
            }

            if (!target.beginDrag) {
                target.beginDrag = Qt.point(target.x, target.y);
            }
            target.z = 100;
        }
        onReleased: event => {
            if (target.beginDrag) {
                target.x = target.beginDrag.x;
                target.y = target.beginDrag.y;
                target.beginDrag = null;
            }
            if (!(event.modifiers & Qt.ControlModifier) && target.selected) {
                target.selected = false;
            }

            target.z = 1;
        }
    }
}
