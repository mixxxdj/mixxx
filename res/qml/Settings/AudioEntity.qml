import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import ".." as Skin
import "../Theme"

Item {
    id: root

    signal connect(var entity)
    signal disconnect(var entity)

    signal scrolled()
    signal gatewayReady(string address, Item node)

    required property string name
    required property string group
    property list<var> gateways: []
    property bool advanced: false

    implicitHeight: 54 + 32 * gatewayRepeater.visibleChannels
    width: 135
    z: 10

    onGatewaysChanged: {
        gatewayRepeater.visibleChannels = root.gateways.length
    }

    property alias handleSource: handleSourceEdge
    property alias handleSink: handleSinkEdge

    property var metaType: null
    Rectangle {
        id: content
        radius: 15
        color: Theme.darkGray3
        anchors.fill: parent
        anchors.margins: 8
        Column {
            id: gatewayColumn
            anchors.fill: parent
            padding: 0
            spacing: 4

            Item {
                height: nameLabel.implicitHeight + 18
                width: parent.width
                Label {
                    id: nameLabel
                    anchors.fill: parent
                    anchors.margins: 9
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignTop
                    text: name
                    color: Theme.white
                    elide: Text.ElideRight
                    font.pixelSize: 15
                    fontSizeMode: Text.Fit
                }
            }

            Repeater {
                id: gatewayRepeater
                model: root.gateways
                property int visibleChannels: root.gateways.length
                Repeater {
                    id: node

                    required property int index
                    readonly property string label: root.gateways[index].name
                    readonly property string address: root.gateways[index].address || root.gateways[index].name
                    readonly property var channels: root.gateways[index].channels || [0, 1]
                    readonly property var instances: root.gateways[index].instances || 1
                    readonly property string type: root.gateways[index].type
                    readonly property bool advanced: root.gateways[index].advanced || false
                    readonly property bool required: !!root.gateways[index].required

                    model: node.channels.length/2 * instances

                    property list<int> channelAssignation: [...Array(node.channels.length/2)].map((_, i) => i)

                    function availableEdge() {
                        for (let i = 0; i < node.count; i++) {
                            let current = node.itemAt(i);
                            if (current.edgeItem.connection) continue;
                            return i;
                        }
                    }
                    function assignedEdges() {
                        let assignation = {}
                        for (let i = 0; i < node.count; i++) {
                            let current = node.itemAt(i);
                            if (current.edgeItem.connection) {
                                assignation[node.channelAssignation[i]] = current.edgeItem.connection
                            }
                        }
                        return assignation
                    }
                    property int connectionCount: 0
                    Item {
                        id: channel

                        required property int index
                        property alias edgeItem: edge
                        property bool counted: channel.index == 0

                        visible: (edgeItem.connection?.ready || index == node.connectionCount) && (!node.advanced || root.advanced)
                        onVisibleChanged: {
                            if (counted != channel.visible)
                                gatewayRepeater.visibleChannels += channel.visible ? 1 : -1
                            counted = channel.visible
                        }

                        width: parent.width
                        height: 28
                        RowLayout {
                            anchors {
                                left: parent.left
                                right: parent.right
                                leftMargin: 15
                                rightMargin: 15
                            }
                            id: inputLabel
                            Label {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredHeight: 28
                                verticalAlignment: Text.AlignVCenter
                                text: node.instances == 1 ? label : `${label} #${index+1}`
                                color: Theme.white
                                elide: Text.ElideRight
                                font.pixelSize: 10
                                fontSizeMode: Text.Fit
                            }
                            // Item {
                            //     Layout.fillWidth: true
                            // }
                            Skin.ComboBox {
                                Layout.minimumWidth: implicitWidth
                                id: channelSelector
                                property int previousIndex: node.channelAssignation[channel.index] ?? 0
                                visible: node.count > 1 && node.channels.length > 2
                                spacing: 2
                                clip: true

                                font.pixelSize: 12
                                model: {
                                    return [...Array(node.channels.length/2)].map((e, i) => `Ch ${i * 2 + node.channels[0] + 1} - ${i * 2 + node.channels[0] + 2}`);
                                }
                                currentIndex: node.channelAssignation[channel.index] ?? 0
                                onActivated: (activatedIndex) => {
                                    let alreadyAssigned = node.channelAssignation.indexOf(activatedIndex)
                                    node.channelAssignation[alreadyAssigned] = previousIndex
                                    node.channelAssignation[channel.index] = activatedIndex
                                }
                            }
                        }
                        Rectangle {
                            id: edge
                            property var entity: root
                            property var advanced: node.advanced
                            property int instance: index / (node.channels.length/2)
                            property string type: node.type
                            property string group: root.group
                            property var address: node.address

                            anchors.horizontalCenter: type == "source" ? parent.right : parent.left
                            anchors.verticalCenter: inputLabel.verticalCenter

                            property var connection: null
                            property bool counted: false
                            property bool connecting: false

                            onConnectionChanged: {
                                if (counted != !!edge.connection)
                                    node.connectionCount += edge.connection ? 1 : -1
                                counted = !!edge.connection
                            }

                            function updateConnectionPosition() {
                                if (edge.connection && edge.connection.source == edge) {
                                    edge.connection.sourcePosition = edge.mapToItem(edge.connection.router, edge.width/2, edge.height/2)
                                } else if (edge.connection && edge.connection.sink == edge) {
                                    edge.connection.sinkPosition = edge.mapToItem(edge.connection.router, edge.width/2, edge.height/2)
                                }
                            }

                            Connections {
                                target: root
                                function onScrolled() {
                                    edge.updateConnectionPosition()
                                }
                                function onXChanged() {
                                    edge.updateConnectionPosition()
                                }
                                function onYChanged() {
                                    edge.updateConnectionPosition()
                                }
                            }

                            Connections {
                                target: channel
                                function onHeightChanged() {
                                    edge.updateConnectionPosition()
                                }
                                function onYChanged() {
                                    edge.updateConnectionPosition()
                                }
                            }

                            color: Theme.midGray
                            width: 10
                            height: width
                            radius: width/2
                            z: 100

                            states: [
                                State {
                                    name: "idle"
                                },
                                State {
                                    name: "warning"
                                    when: (!edge.connection && node.required) || (edge.connection && edge.connection.state == "warning")

                                    PropertyChanges {
                                        edge.width: 15
                                        edge.color: Theme.warningColor
                                    }
                                },
                                State {
                                    name: "hidden"
                                    when: edge.connection && !edge.connection.visible

                                    PropertyChanges {
                                        channel.opacity: 0.5
                                    }
                                },
                                State {
                                    name: "setting"
                                    when: edge.connection && !edge.connection.existing || edge.connecting

                                    PropertyChanges {
                                        edge.width: 15
                                        edge.color: Theme.accentColor
                                    }
                                }
                            ]

                            MouseArea {
                                id: edgeMouseArea
                                hoverEnabled: edge.connection != null && edge.connection.visible
                                anchors.fill: parent
                                onPressed: {
                                    if (edge.connection && edge.connection.flags & AudioConnection.Flags.AboutToDelete) {
                                        root.disconnect(edge.connection)
                                    } else if (edge.connection == null) {
                                        root.connect(parent)
                                    }
                                }
                                onEntered: {
                                    if (edge.connection) {
                                        edge.connection.flags |= AudioConnection.Flags.AboutToDelete
                                    }
                                }
                                onExited: {
                                    if (edge.connection) {
                                        edge.connection.flags &= ~AudioConnection.Flags.AboutToDelete
                                    }
                                }
                            }
                        }
                    }
                    Component.onCompleted: {
                        root.gatewayReady(address, node)
                    }
                }
            }
        }
        AudioEntityEdge {
            id: handleSourceEdge
            entity: root
            type: "source"

            Connections {
                target: root

                function onImplicitHeightChanged() {
                    handleSourceEdge.updateConnectionPosition()
                }
                function onXChanged() {
                    handleSourceEdge.updateConnectionPosition()
                }
                function onYChanged() {
                    handleSourceEdge.updateConnectionPosition()
                }
            }

            anchors.horizontalCenter: handleSourceEdge.vertical ? parent.horizontalCenter : parent.right
            anchors.verticalCenter: handleSourceEdge.vertical ? parent.bottom : undefined
            anchors.top: handleSourceEdge.vertical ? undefined :parent.top
            anchors.topMargin: handleSourceEdge.vertical ? 0 :16
        }
        AudioEntityEdge {
            id: handleSinkEdge
            entity: root
            type: "sink"

            Connections {
                target: root

                function onImplicitHeightChanged() {
                    handleSinkEdge.updateConnectionPosition()
                }
                function onXChanged() {
                    handleSinkEdge.updateConnectionPosition()
                }
                function onYChanged() {
                    handleSinkEdge.updateConnectionPosition()
                }
            }

            anchors.horizontalCenter: handleSinkEdge.vertical ? parent.horizontalCenter : parent.left
            anchors.verticalCenter: handleSinkEdge.vertical ? parent.top : parent.verticalCenter
        }
    }
}
