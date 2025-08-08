import QtQuick 2.12
import QtQuick.Shapes
import "../Theme"

Item {
    id: root

    enum Flags {
        AboutToDelete = 1,
        CannotConnect = 2
    }

    required property var source
    required property var router
    property var sink: undefined
    property var target: source.mapToItem(router, source.width/2, source.height/2)

    property bool system: false
    property bool vertical: false
    property bool existing: false
    property int flags: 0

    readonly property bool ready: !!source && !!sink

    visible: !source || !sink || source.visible && sink.visible

    z: 0

    states: [
        State {
            name: "warning"
            when: root.flags

            PropertyChanges {
                line.strokeColor: Theme.warningColor
            }

            PropertyChanges {
                root.z: 50
            }
        },
        State {
            name: "system"
            when: root.system

            PropertyChanges {
                line.strokeColor: Theme.darkGray2
            }
        },
        State {
            name: "existing"
            when: root.existing

            PropertyChanges {
                line.strokeColor: Theme.midGray
            }
        },
        State {
            name: "setting"
            when: root.sink === undefined

            PropertyChanges {
                line.strokeColor: Theme.accentColor
            }

            PropertyChanges {
                root.z: 50
            }
        },
        State {
            name: "set"
            when: root.sink != undefined && !root.existing

            PropertyChanges {
                line.strokeColor: Theme.accentColor
            }
        }
    ]

    property var sourcePosition: source.mapToItem(router, source.width/2, source.height/2)
    property var sinkPosition: sink ? sink.mapToItem(router, sink.width/2, sink.height/2) : target

    onSinkPositionChanged: {
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1
    }

    onSourcePositionChanged: {
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1
    }

    x: sourcePosition.x
    y: sourcePosition.y
    width: Math.max(2, Math.abs(sourcePosition.x - sinkPosition.x))
    height: Math.max(2, Math.abs(sourcePosition.y - sinkPosition.y))

    onSinkChanged: {
        if (sink != null && source != null) {
            // swap entities if the connection was made backward
            if (source.type !== "source") {
                let swap = root.source
                root.source = root.sink
                root.sink = swap
                return;
            }
            target = null
            if (sink.connections !== undefined) {
                sink.connections.add(root)
            } else {
                sink.connection = root
            }
            if (source.connections !== undefined) {
                source.connections.add(root)
            } else {
                source.connection = root
            }
        }
    }
    onSourceChanged: {
        if (sink != null && source != null) {
            // swap entities if the connection was made backward
            if (source.type !== "source") {
                let swap = root.source
                root.source = root.sink
                root.sink = swap
                return;
            }
            target = null
            if (sink.connections !== undefined) {
                sink.connections.add(root)
            } else {
                sink.connection = root
            }
            if (source.connections !== undefined) {
                source.connections.add(root)
            } else {
                source.connection = root
            }
        }
    }
    onTargetChanged: {
        if (!source)
            return
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1
    }

    Shape {
        anchors.fill: parent
        // anchors.centerIn: parent
        antialiasing: true
        layer.enabled: true
        layer.samples: 16
        layer.textureMirroring: ShaderEffectSource.MirrorHorizontally
        ShapePath {
            id: line
            strokeColor: Theme.midGray
            strokeWidth: 2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.BevelJoin

            startX: 1
            startY: 1
            PathQuad { x: root.width * 0.5; y: root.height * 0.5; controlX: root.width * (root.vertical ? 0 : 0.3); controlY : root.height * (root.vertical ? 0.3 : 0) }
            PathQuad { x: root.width; y: root.height; controlX: root.width * (root.vertical ? 1 : 0.7); controlY : root.height * (root.vertical ? 0.7 : 1) }
        }
    }
    transform: Scale {
        id: scale
    }
}
