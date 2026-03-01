import QtQuick 2.12

Rectangle {
    id: root
    property bool vertical: false
    required property var entity
    required property string type

    property var connections: new Set()

    color: 'transparent'
    width: 10
    height: 10

    function updateConnectionPosition() {
        for (let connection of connections) {
            if (connection && connection.source == root) {
                connection.sourcePosition = root.mapToItem(connection.router, root.width/2, root.height/2)
            } else if (connection && connection.sink == root) {
                connection.sinkPosition = root.mapToItem(connection.router, root.width/2, root.height/2)
            }
        }
    }
}
