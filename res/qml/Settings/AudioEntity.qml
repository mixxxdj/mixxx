import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import ".." as Skin
import "../Theme"

Item {
    id: root

    property bool advanced: false
    property list<var> gateways: []
    required property string group
    property alias handleSink: handleSinkEdge
    property alias handleSource: handleSourceEdge
    property var metaType: null
    required property string name

    signal connect(var entity)
    signal disconnect(var entity)
    signal gatewayReady(string address, Item node)
    signal scrolled

    implicitHeight: 45 + 28 * gatewayRepeater.visibleChannels
    width: 105
    z: 10

    onGatewaysChanged: {
        gatewayRepeater.visibleChannels = root.gateways.length;
    }

    Rectangle {
        id: content

        anchors.fill: parent
        anchors.margins: 8
        color: Theme.darkGray3
        radius: 15

        Column {
            id: gatewayColumn

            anchors.fill: parent
            padding: 0
            spacing: 4

            Item {
                height: nameLabel.implicitHeight + 9
                width: parent.width

                Label {
                    id: nameLabel

                    anchors.fill: parent
                    anchors.margins: 9
                    color: Theme.white
                    elide: Text.ElideRight
                    font.pixelSize: 15
                    fontSizeMode: Text.Fit
                    horizontalAlignment: Text.AlignHCenter
                    text: name
                    verticalAlignment: Text.AlignTop
                }
            }
            Repeater {
                id: gatewayRepeater

                property int visibleChannels: root.gateways.length

                model: root.gateways

                Repeater {
                    id: node

                    readonly property string address: root.gateways[index].address || root.gateways[index].name
                    readonly property bool advanced: root.gateways[index].advanced || false
                    property list<int> channelAssignation: [...Array(node.channels.length / 2)].map((_, i) => i)
                    readonly property var channels: root.gateways[index].channels || [0, 1]
                    property int connectionCount: 0
                    required property int index
                    readonly property var instances: root.gateways[index].instances || 1
                    readonly property string label: root.gateways[index].name
                    readonly property bool required: !!root.gateways[index].required
                    readonly property string type: root.gateways[index].type

                    function assignedEdges() {
                        let assignation = {};
                        for (let i = 0; i < node.count; i++) {
                            let current = node.itemAt(i);
                            if (current.edgeItem.connection) {
                                assignation[node.channelAssignation[i]] = current.edgeItem.connection;
                            }
                        }
                        return assignation;
                    }
                    function availableEdge() {
                        for (let i = 0; i < node.count; i++) {
                            let current = node.itemAt(i);
                            if (current.edgeItem.connection)
                                continue;
                            return i;
                        }
                    }

                    model: node.channels.length / 2 * instances

                    Component.onCompleted: {
                        root.gatewayReady(address, node);
                    }

                    Item {
                        id: channel

                        property bool counted: channel.index == 0
                        property alias edgeItem: edge
                        required property int index

                        height: 22
                        visible: (edgeItem.connection?.ready || index == node.connectionCount) && (!node.advanced || root.advanced)
                        width: parent.width

                        onVisibleChanged: {
                            if (counted != channel.visible)
                                gatewayRepeater.visibleChannels += channel.visible ? 1 : -1;
                            counted = channel.visible;
                        }

                        RowLayout {
                            id: inputLabel

                            anchors {
                                left: parent.left
                                leftMargin: 15
                                right: parent.right
                                rightMargin: 15
                            }
                            Label {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.fillWidth: true
                                Layout.preferredHeight: 28
                                color: Theme.white
                                elide: Text.ElideRight
                                font.pixelSize: 10
                                fontSizeMode: Text.Fit
                                text: node.instances == 1 ? label : `${label} #${index + 1}`
                                verticalAlignment: Text.AlignVCenter
                            }
                            // Item {
                            //     Layout.fillWidth: true
                            // }
                            Skin.ComboBox {
                                id: channelSelector

                                property int previousIndex: node.channelAssignation[channel.index] ?? 0

                                Layout.minimumWidth: implicitWidth
                                clip: true
                                currentIndex: node.channelAssignation[channel.index] ?? 0
                                font.pixelSize: 12
                                model: {
                                    return [...Array(node.channels.length / 2)].map((e, i) => `Ch ${i * 2 + node.channels[0] + 1} - ${i * 2 + node.channels[0] + 2}`);
                                }
                                spacing: 2
                                visible: node.count > 1 && node.channels.length > 2

                                onActivated: activatedIndex => {
                                    let alreadyAssigned = node.channelAssignation.indexOf(activatedIndex);
                                    node.channelAssignation[alreadyAssigned] = previousIndex;
                                    node.channelAssignation[channel.index] = activatedIndex;
                                }
                            }
                        }
                        Rectangle {
                            id: edge

                            property var address: node.address
                            property var advanced: node.advanced
                            property bool connecting: false
                            property var connection: null
                            property bool counted: false
                            property var entity: root
                            property string group: root.group
                            property int instance: index / (node.channels.length / 2)
                            property string type: node.type

                            function updateConnectionPosition() {
                                if (edge.connection && edge.connection.source == edge) {
                                    edge.connection.sourcePosition = edge.mapToItem(edge.connection.router, edge.width / 2, edge.height / 2);
                                } else if (edge.connection && edge.connection.sink == edge) {
                                    edge.connection.sinkPosition = edge.mapToItem(edge.connection.router, edge.width / 2, edge.height / 2);
                                }
                            }

                            anchors.horizontalCenter: type == "source" ? parent.right : parent.left
                            anchors.verticalCenter: inputLabel.verticalCenter
                            color: Theme.midGray
                            height: width
                            radius: width / 2
                            width: 15
                            z: 100

                            states: [
                                State {
                                    name: "idle"
                                },
                                State {
                                    name: "warning"
                                    when: (!edge.connection && node.required) || (edge.connection && edge.connection.state == "warning")

                                    PropertyChanges {
                                        edge.color: Theme.warningColor
                                        edge.width: 20
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
                                        edge.color: Theme.accentColor
                                        edge.width: 15
                                    }
                                }
                            ]

                            onConnectionChanged: {
                                if (counted != !!edge.connection)
                                    node.connectionCount += edge.connection ? 1 : -1;
                                counted = !!edge.connection;
                            }

                            Connections {
                                function onScrolled() {
                                    edge.updateConnectionPosition();
                                }
                                function onXChanged() {
                                    edge.updateConnectionPosition();
                                }
                                function onYChanged() {
                                    edge.updateConnectionPosition();
                                }

                                target: root
                            }
                            Connections {
                                function onHeightChanged() {
                                    edge.updateConnectionPosition();
                                }
                                function onYChanged() {
                                    edge.updateConnectionPosition();
                                }

                                target: channel
                            }
                            MouseArea {
                                id: edgeMouseArea

                                anchors.fill: parent
                                hoverEnabled: edge.connection != null && edge.connection.visible

                                onEntered: {
                                    if (edge.connection) {
                                        edge.connection.flags |= AudioConnection.Flags.AboutToDelete;
                                    }
                                }
                                onExited: {
                                    if (edge.connection) {
                                        edge.connection.flags &= ~AudioConnection.Flags.AboutToDelete;
                                    }
                                }
                                onPressed: {
                                    if (edge.connection && edge.connection.flags & AudioConnection.Flags.AboutToDelete) {
                                        root.disconnect(edge.connection);
                                    } else if (edge.connection == null) {
                                        root.connect(parent);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        AudioEntityEdge {
            id: handleSourceEdge

            anchors.horizontalCenter: handleSourceEdge.vertical ? parent.horizontalCenter : parent.right
            anchors.top: handleSourceEdge.vertical ? undefined : parent.top
            anchors.topMargin: handleSourceEdge.vertical ? 0 : 16
            anchors.verticalCenter: handleSourceEdge.vertical ? parent.bottom : undefined
            entity: root
            type: "source"

            Connections {
                function onImplicitHeightChanged() {
                    handleSourceEdge.updateConnectionPosition();
                }
                function onXChanged() {
                    handleSourceEdge.updateConnectionPosition();
                }
                function onYChanged() {
                    handleSourceEdge.updateConnectionPosition();
                }

                target: root
            }
        }
        AudioEntityEdge {
            id: handleSinkEdge

            anchors.horizontalCenter: handleSinkEdge.vertical ? parent.horizontalCenter : parent.left
            anchors.verticalCenter: handleSinkEdge.vertical ? parent.top : parent.verticalCenter
            entity: root
            type: "sink"

            Connections {
                function onImplicitHeightChanged() {
                    handleSinkEdge.updateConnectionPosition();
                }
                function onXChanged() {
                    handleSinkEdge.updateConnectionPosition();
                }
                function onYChanged() {
                    handleSinkEdge.updateConnectionPosition();
                }

                target: root
            }
        }
    }
}
