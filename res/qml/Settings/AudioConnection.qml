import QtQuick 2.12
import QtQuick.Shapes
import "../Theme"

Item {
    id: root

    enum Flags {
        AboutToDelete = 1,
        CannotConnect = 2
    }

    property bool existing: false
    property int flags: 0
    readonly property bool ready: !!source && !!sink
    required property var router
    property var sink: undefined
    property var sinkPosition: sink ? sink.mapToItem(router, sink.width / 2, sink.height / 2) : target
    required property var source
    property var sourcePosition: source.mapToItem(router, source.width / 2, source.height / 2)
    property bool system: false
    property var target: source.mapToItem(router, source.width / 2, source.height / 2)
    property bool vertical: false

    height: Math.max(2, Math.abs(sourcePosition.y - sinkPosition.y))
    visible: !source || !sink || source.visible && sink.visible
    width: Math.max(2, Math.abs(sourcePosition.x - sinkPosition.x))
    x: sourcePosition.x
    y: sourcePosition.y
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
    transform: Scale {
        id: scale

    }

    onSinkChanged: {
        if (sink != null && source != null) {
            // swap entities if the connection was made backward
            if (source.type !== "source") {
                let swap = root.source;
                root.source = root.sink;
                root.sink = swap;
                return;
            }
            target = null;
            if (sink.connections !== undefined) {
                sink.connections.add(root);
            } else {
                sink.connection = root;
            }
            if (source.connections !== undefined) {
                source.connections.add(root);
            } else {
                source.connection = root;
            }
        }
    }
    onSinkPositionChanged: {
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1;
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1;
    }
    onSourceChanged: {
        if (sink != null && source != null) {
            // swap entities if the connection was made backward
            if (source.type !== "source") {
                let swap = root.source;
                root.source = root.sink;
                root.sink = swap;
                return;
            }
            target = null;
            if (sink.connections !== undefined) {
                sink.connections.add(root);
            } else {
                sink.connection = root;
            }
            if (source.connections !== undefined) {
                source.connections.add(root);
            } else {
                source.connection = root;
            }
        }
    }
    onSourcePositionChanged: {
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1;
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1;
    }
    onTargetChanged: {
        if (!source)
            return;
        scale.xScale = sourcePosition.x > sinkPosition.x ? -1 : 1;
        scale.yScale = sourcePosition.y > sinkPosition.y ? -1 : 1;
    }

    Shape {
        anchors.fill: parent
        // anchors.centerIn: parent
        antialiasing: true
        layer.enabled: true

        // FIXME: This is causing GL glitched in Android
        // layer.samples: 16
        // layer.textureMirroring: ShaderEffectSource.MirrorHorizontally
        ShapePath {
            id: line

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            joinStyle: ShapePath.BevelJoin
            startX: 1
            startY: 1
            strokeColor: Theme.midGray
            strokeWidth: 2

            PathQuad {
                controlX: root.width * (root.vertical ? 0 : 0.3)
                controlY: root.height * (root.vertical ? 0.3 : 0)
                x: root.width * 0.5
                y: root.height * 0.5
            }
            PathQuad {
                controlX: root.width * (root.vertical ? 1 : 0.7)
                controlY: root.height * (root.vertical ? 0.7 : 1)
                x: root.width
                y: root.height
            }
        }
    }
}
