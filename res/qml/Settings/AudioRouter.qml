import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import "../Theme"
import ".." as Skin

Rectangle {
    id: root
    color: '#0E0E0E'
    radius: 5
    clip: true

    enum Mode {
        Simple,
        Advanced,
        Legacy
    }

    property var connections: new Set()
    property var selectedTab: ""
    property var newConnection: null
    readonly property var mode: modeChoice.selected == "simple" ? AudioRouter.Mode.Simple : modeChoice.selected == "legacy" ? AudioRouter.Mode.Legacy : AudioRouter.Mode.Advanced

    property int hiddenConnections: 2

    onModeChanged: {
        updateHiddenConnectionCount()
    }

    property var manager: Mixxx.SoundManager

    property Component connectionEdge: Qt.createComponent("AudioConnection.qml")

    property var inputDevices: new Object()
    property var outputDevices: new Object()

    property var system: new Object()
    property bool hasChanges: false

    property alias multiSoundcard: multiSoundcardChoice

    property var inputs: new Object()
    property var outputs: new Object()

    function updateHiddenConnectionCount() {
        root.hiddenConnections = 0
        if (root.mode == AudioRouter.Mode.Simple) {
            for (let connection of root.connections) {
                console.log("advanced?", connection.source?.advanced, connection.sink?.advanced)
                if (connection.source?.advanced || connection.sink?.advanced) {
                    root.hiddenConnections += 1
                }
            }
        }
    }

    function generateDeviceList(devices, existing) {
        let cards = {}
        const hwAlsa = / (\(hw:\d+,\d+\))/;
        for (let device of devices) {
            let components = device.displayName.split(':')
            let cardName = components.shift().trim()
            let deviceName = components.length > 0 ? components.join(":").trim() : "Default"
            let hw = deviceName.match(hwAlsa, "")
            if (!cards[cardName]) {
                cards[cardName] = {
                    gateways: {},
                    channelCount: 0,
                }
            }

            cards[cardName].gateways[hw ? hw[1] : deviceName] = {
                name: deviceName.replace(hwAlsa, ""),
                node: existing[cardName] ? .gateways ? .[hw ? hw[1] : deviceName] ? .node ?? null,
                address: hw ? hw[1] : null,
                device: device,
                channels: device.channelCount
            }
            cards[cardName].channelCount += device.channelCount
        }
        return cards
    }

    function update(api) {
        root.inputs = generateDeviceList(manager.availableInputDevices(api), root.inputs)
        root.outputs = generateDeviceList(manager.availableOutputDevices(api), root.outputs)
        root.loadConnections()
    }

    function registerInputEdge(device, address, node) {
        root.inputs[device].gateways[address].node = node
        if (root.inputs[device].gateways[address].delayedConnections) {
            addExistingConnection(node, root.inputs[device].gateways[address].delayedConnections)
            root.inputs[device].gateways[address].delayedConnections = undefined
        }
    }
    function registerOutputEdge(device, address, node) {
        root.outputs[device].gateways[address].node = node
        if (root.outputs[device].gateways[address].delayedConnections) {
            addExistingConnection(node, root.outputs[device].gateways[address].delayedConnections)
            root.outputs[device].gateways[address].delayedConnections = undefined
        }
    }

    readonly property list<var> audioTypeMap: [
        {entity: "Mixer",channel: "Main"}, // Main,
        {entity: "Mixer",channel: "PFL"}, // Headphones,
        {entity: "Mixer",channel: "Booth"}, // Booth,
        {entity: "Mixer",channel: "Bus"}, // Bus,
        {entity: "Deck",channel: "Output"}, // Deck,
        {entity: "Deck",channel: "Vinyl Control"}, // VinylControl,
        {entity: "Mixer",channel: "Microphone"}, // Microphone,
        {entity: "Mixer",channel: "Auxiliary"}, // Auxiliary,
        {entity: "Record",channel: "Additional input"}, // RecordBroadcast,
    ]

    function addExistingConnection(node, connections) {
        if (!connections) return;
        for (let connection of connections) {
            let typeDef = audioTypeMap[connection.type]
            let source = root.system[typeDef.entity].gateways[typeDef.channel][connection.index].edgeItem
            let availableEdge = node.availableEdge()
            let connectionItem = root.connectionEdge.createObject(root, {
                    "existing": true,
                    "router": root,
                    "source": source,
                    "sink": node.itemAt(availableEdge).edgeItem,
            })
            root.connections.add(connectionItem)
            node.channelAssignation[availableEdge] = connection.channelGroup / 2
        }
        root.updateHiddenConnectionCount()
    }

    function loadConnections() {
        while (root.connections.size) {
            let connection = root.connections.keys().next().value;
            root.entityOnDisconnect(connection)
        }

        for (let device of Object.keys(root.outputs)) {
            for (let address of Object.keys(root.outputs[device].gateways)) {
                let gateway = root.outputs[device].gateways[address]
                let node = gateway.node
                let connections = root.outputs[device].gateways[address].device.connections(Mixxx.SoundManager)

                if (!connections) continue;
                if (node) {
                    addExistingConnection(node, connections)
                } else if (connections) {
                    root.outputs[device].gateways[address].delayedConnections = connections
                }
            }
        }

        for (let device of Object.keys(root.inputs)) {
            for (let address of Object.keys(root.inputs[device].gateways)) {
                let gateway = root.inputs[device].gateways[address]
                let node = gateway.node
                let connections = root.inputs[device].gateways[address].device.connections(Mixxx.SoundManager)
                console.log("INPUT", gateway, node, connections)

                if (!connections) continue;
                if (node) {
                    addExistingConnection(node, connections)
                } else if (connections) {
                    root.inputs[device].gateways[address].delayedConnections = connections
                }
            }
        }

        root.hasChanges = false
    }

    MouseArea {
        enabled: root.newConnection != null
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        onPositionChanged: (mouse) => {
            if (root.newConnection) {
                root.newConnection.target = Qt.point(mouse.x, mouse.y)
            }
        }
        onExited: {
            if (root.newConnection) {
                root.newConnection.destroy();
                root.newConnection.source.connection = null
                root.newConnection = null
            }
        }
        onPressed: {
            if (root.newConnection) {
                root.newConnection.destroy();
                root.newConnection.source.connection = null
                root.newConnection = null
            }
        }
    }

    function entityOnConnect(edge) {
        if (root.newConnection != null) {
            if (edge.type == root.newConnection.source.type || edge.group == root.newConnection.source.group || edge.entity == root.newConnection.source.entity) {
                root.newConnection.flags |= AudioConnection.Flags.CannotConnect
                return;
            }
            root.newConnection.source.connecting = false
            if (root.newConnection.source == edge) {
                root.newConnection.destroy();
            } else {
                root.newConnection.sink = edge
                root.connections.add(root.newConnection)
                root.hasChanges = true
            }
            root.newConnection = null
        } else {
            root.newConnection = connectionEdge.createObject(root, {"router": root, "source": edge});
        }
    }

    function entityOnDisconnect(connection) {
        var sink = connection.sink;
        var source = connection.source;
        root.connections.delete(connection)
        if (connection.existing)
            root.hasChanges = true

        if (source != null) {
            if (source.connection !== undefined) {
                source.connection = null
            } else {
                source.connections.delete(connection)
            }
        }

        if (sink != null) {
            if (sink.connection !== undefined) {
                sink.connection = null
            } else {
                sink.connections.delete(connection)
            }
        }
        connection.destroy()
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            visible: root.mode == AudioRouter.Mode.Advanced
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.maximumWidth: 220
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: 15
                text: "Inputs"
                color: '#626262'
                font.pixelSize: 14
            }

            ListView {
                id: inputList
                Layout.margins: 15
                Layout.fillHeight: true
                Layout.fillWidth: true
                model: Object.keys(root.inputs)
                clip: true
                reuseItems: false
                spacing: 15
                delegate: AudioEntity {
                    id: entity
                    required property var modelData
                    width: ListView.view.width

                    name: modelData
                    group: "external"
                    gateways: {
                        let channels = []
                        let maxChannelPerInput = 8 / root.inputs[modelData].channelCount;
                        for (let item of Object.values(root.inputs[modelData].gateways)) {
                            let channel = 0
                            for (; channel < item.channels && channel <= maxChannelPerInput; channel += 2) {
                                channels.push({
                                        name: item.name,
                                        address: item.address,
                                        channels: [channel, channel+1],
                                                   type: "source",
                                                   advanced: true
                                                   });
                                                   }
                                                   
                            if (channel < item.channels) {
                                let start = channels[channels.length-1].channels[0]
                                let channelPicker = [...Array(item.channels - start)]
                                channels[channels.length-1].channels = channelPicker.map((_, i) => start+i)
                            }
                        }
                        return channels;
                    }
                    advanced: root.mode == AudioRouter.Mode.Advanced

                    onGatewayReady: (address, node) => {
                        root.registerInputEdge(modelData, address, node)
                    }

                    onConnect: (point) => root.entityOnConnect(point)
                    onDisconnect: (point) => root.entityOnDisconnect(point)
                    Connections {
                        target: inputList
                        function onContentYChanged() {
                            entity.scrolled()
                        }
                        function onXChanged() {
                            entity.scrolled()
                        }
                        function onYChanged() {
                            entity.scrolled()
                        }
                        function onWidthChanged() {
                            entity.scrolled()
                        }
                        function onHeightChanged() {
                            entity.scrolled()
                        }
                    }
                }
            }
        }
        Rectangle {
            visible: root.mode == AudioRouter.Mode.Advanced
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            width: 1
            color: '#626262'
        }
        ColumnLayout {
            id: mainCanvas
            RowLayout {
                Layout.topMargin: 6
                Layout.bottomMargin: 3
                Item {
                    Layout.fillWidth: true
                }
                RatioChoice {
                    id: modeChoice
                    options: [
                              "simple",
                              !root.hiddenConnections ? "advanced" : "advanced (!)",
                              "legacy"
                    ]
                    onOptionsChanged: {
                        if (modeChoice.selected.startsWith("advanced")) {
                            modeChoice.selected = options[1]
                        }
                    }
                    tooltips: !root.hiddenConnections ? [] : [null, `${root.hiddenConnections} connection${root.hiddenConnections > 1 ? 's' : ''} hidden\nUse the advanced mode to view them`, null]
                }
            }
            Item {
                Layout.fillHeight: true
            }
            RowLayout {
                Layout.bottomMargin: 3
                RatioChoice {
                    id: multiSoundcardChoice
                    normalizedWidth: false
                    options: [
                              "experimental",
                              "default",
                              "disabled"
                    ]
                    tooltips: [
                               "No delay",
                               "Long delay",
                               "Short delay"
                    ]

                    onSelectedChanged: {
                        root.hasChanges = true
                    }

                    Mixxx.SettingParameter {
                        label: "Multi-Soundcard Synchronization"
                    }
                }
                Text {
                    text: "Multi-Soundcard Synchronization"
                    color: Theme.white
                    font.pixelSize: 14
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
        Rectangle {
            visible: root.mode != AudioRouter.Mode.Legacy
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            width: 1
            color: '#626262'
        }

        ColumnLayout {
            visible: root.mode != AudioRouter.Mode.Legacy
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.maximumWidth: 220
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: 15
                text: "Outputs"
                color: '#626262'
                font.pixelSize: 14
            }

            ListView {
                id: outputList
                Layout.margins: 15
                Layout.fillHeight: true
                Layout.fillWidth: true
                model: Object.keys(root.outputs)
                clip: true
                reuseItems: false
                spacing: 15
                cacheBuffer: Math.max(0, contentHeight) // Disable lazy loading to make sure all item are loaded and can be bounded to connection
                delegate: AudioEntity {
                    id: entity
                    required property var modelData
                    width: ListView.view.width

                    name: modelData
                    group: "external"
                    gateways: {
                        let channels = []
                        let maxChannelPerOutput = 8 / root.outputs[modelData].channelCount;
                        for (let item of Object.values(root.outputs[modelData].gateways)) {
                            let channel = 0
                            for (; channel < item.channels && channel <= maxChannelPerOutput; channel += 2) {
                                channels.push({
                                        name: item.name,
                                        address: item.address,
                                        channels: [channel, channel+1],
                                                   type: "sink"
                                                   });
                                                   }
                                                   
                            if (channel < item.channels) {
                                let start = channels[channels.length-1].channels[0]
                                let channelPicker = [...Array(item.channels - start)]
                                channels[channels.length-1].channels = channelPicker.map((_, i) => start+i)
                            }
                        }
                        return channels;
                    }

                    onGatewayReady: (address, node) => {
                        root.registerOutputEdge(modelData, address, node)
                    }

                    onConnect: (point) => root.entityOnConnect(point)
                    onDisconnect: (point) => root.entityOnDisconnect(point)
                    Connections {
                        target: outputList
                        function onContentYChanged() {
                            entity.scrolled()
                        }
                        function onXChanged() {
                            entity.scrolled()
                        }
                        function onYChanged() {
                            entity.scrolled()
                        }
                        function onWidthChanged() {
                            entity.scrolled()
                        }
                        function onHeightChanged() {
                            entity.scrolled()
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    Repeater {
        id: decks
        model: 4
        AudioEntity {
            visible: root.mode != AudioRouter.Mode.Legacy
            required property int index
            id: deck
            x: root.width / (root.mode == AudioRouter.Mode.Advanced ? 4 : 5)
            y: (root.height / 5) * (1 + index) - implicitHeight / 2

            name: `Deck ${index+1}`
            group: "internal"
            gateways: [{
                       name: "Output",
                       type: "source",
                       advanced: true
                }, {
                    name: "Vinyl Control",
                    type: "sink",
                    advanced: true
                }
            ]
            advanced: root.mode == AudioRouter.Mode.Advanced

            onConnect: (point) => root.entityOnConnect(point)
            onDisconnect: (point) => root.entityOnDisconnect(point)

            onGatewayReady: (address, node) => {
                if (!root.system["Deck"]) {
                    root.system["Deck"] = {
                        gateways: {}
                    }
                }
                if (!root.system["Deck"].gateways[address]) {
                    root.system["Deck"].gateways[address] = []
                }
                root.system["Deck"].gateways[address][deck.index] = node.itemAt(0)
            }
        }
        onItemAdded: (index, item) => {
            deckConnections.items.push(item)
        }
        onItemRemoved: (index, item) => {
            deckConnections.items.slice(deckConnections.items.indexOf(item), 1)
        }
    }

    AudioEntity {
        visible: root.mode != AudioRouter.Mode.Legacy
        id: mixer

        x: root.width / 2
        y: Math.max(root.height / 16 , root.height / (root.mode == AudioRouter.Mode.Advanced ? 3 : 2) - implicitHeight / 2)

        name: "Mixer"
        group: "internal"

        advanced: root.mode == AudioRouter.Mode.Advanced

        gateways: [{
                   name: "PFL",
                   type: "source"
            }, {
                name: "Main",
                type: "source",
                required: true
            }, {
                name: "Booth",
                type: "source"
            }, {
                name: "Left Bus",
                type: "source",
                advanced: true
            },  {
                name: "Center Bus",
                type: "source",
                advanced: true
            },  {
                name: "Right Bus",
                type: "source",
                advanced: true
            }, {
                name: "Auxiliary",
                type: "sink",
                instances: 4,
                advanced: true
            }, {
                name: "Microphone",
                type: "sink",
                instances: 4,
                advanced: true
            }
        ]

        Mixxx.SettingParameter {
            label: "PFL"
        }
        Mixxx.SettingParameter {
            label: "Main"
        }
        Mixxx.SettingParameter {
            label: "Booth"
        }
        Mixxx.SettingParameter {
            label: "Mixer"
        }
        Mixxx.SettingParameter {
            label: "Decks"
        }
        Mixxx.SettingParameter {
            label: "Input"
        }
        Mixxx.SettingParameter {
            label: "Output"
        }
        Mixxx.SettingParameter {
            label: "Broadcast"
        }

        handleSource.vertical: mixer.height < root.height*0.75

        onConnect: (point) => root.entityOnConnect(point)
        onDisconnect: (point) => root.entityOnDisconnect(point)

        onGatewayReady: (address, node) => {
            if (!root.system["Mixer"]) {
                root.system["Mixer"] = {
                    gateways: {}
                }
            }
            let nodeIdxOffset = 0
            if (address.endsWith(" Bus")) {
                nodeIdxOffset = address.startsWith("Left ") ? 0 : address.startsWith("Right ") ? 2 : 1
                address = "Bus"
            }
            console.log("register", address, nodeIdxOffset)
            if (!root.system["Mixer"].gateways[address]) {
                root.system["Mixer"].gateways[address] = []
            }
            for (let nodeIdx = 0; nodeIdx < node.count; nodeIdx++) {
                root.system["Mixer"].gateways[address][nodeIdxOffset + nodeIdx] = node.itemAt(nodeIdx)
            }
        }
    }
    Repeater {
        id: deckConnections
        property list<var> items: []
        model: items
        AudioConnection {
            visible: root.mode != AudioRouter.Mode.Legacy
            required property var modelData
            router: root
            source: modelData.handleSource
            sink: mixer.handleSink
            system: true
        }
    }
    AudioEntity {
        id: record
        visible: root.mode == AudioRouter.Mode.Advanced

        x: root.width / 5 * 3
        y: root.height / 5 * 4 - implicitHeight / 2

        name: "Record/Broadcast"
        group: "internal"
        metaType: "sink"

        handleSink.vertical: true

        gateways: [{
                   name: "Alternative input",
                   type: "sink"
                   }
        ]
        property var alternativeConnection: null
        readonly property bool hasAlternativeConnection: alternativeConnection && alternativeConnection.ready

        onConnect: (point) => {
            if (root.newConnection != null) {
                record.alternativeConnection = root.newConnection
            }
            root.entityOnConnect(point)
            if (root.newConnection != null) {
                record.alternativeConnection = root.newConnection
            }
        }
        onDisconnect: (point) => {
            record.alternativeConnection = null
            root.entityOnDisconnect(point)
        }

        onGatewayReady: (address, node) => {
            if (!root.system["Record"]) {
                root.system["Record"] = {
                    gateways: {}
                }
            }
            if (!root.system["Record"].gateways[address]) {
                root.system["Record"].gateways[address] = []
            }
            node = node.itemAt(0)
            root.system["Record"].gateways[address][node.index] = node
        }
    }

    AudioConnection {
        visible: root.mode == AudioRouter.Mode.Advanced && !record.hasAlternativeConnection

        router: root
        source: mixer.handleSource
        sink: record.handleSink
        system: true
        vertical: true
    }
}
