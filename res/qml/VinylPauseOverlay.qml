import QtQuick 2.12
import Mixxx 1.0 as Mixxx

/**
 * Vinyl deceleration overlay — Rekordbox-style pause animation.
 *
 * When play stops, instead of snapping the vinyl still, the record
 * decelerates from full speed to a halt over ~800ms, matching the
 * audio deceleration in EngineBuffer.
 *
 * The overlay is a thin ring centered in the deck area. Inactive
 * decks show nothing.
 */
Item {
    id: root
    anchors.fill: parent
    visible: opacity > 0
    opacity: 0.0

    // ─── Config ────────────────────────────────────────────────────
    readonly property real vinylSize: 120
    readonly property int decelDurationMs: 800  // matches EngineBuffer::kVinylPauseDuration

    // ─── Deck play state ───────────────────────────────────────────
    property bool wasPlaying: false
    property real rotationSpeed: 0  // degrees per animation tick

    Mixxx.ControlProxy {
        id: deck1Play
        group: "[Channel1]"
        key: "play"

        onValueChanged: {
            var nowPlaying = (value !== 0);
            if (wasPlaying && !nowPlaying) {
                // Play → stop: start deceleration
                fadeInAnimation.start();
                decelAnimation.start();
            } else if (nowPlaying) {
                // Stop → play: immediately visible and spinning
                decelAnimation.stop();
                rotationSpeed = 360;  // full speed
                root.opacity = 1.0;
                // Direct rotation jump to avoid stale position
                vinylIcon.rotation = 0;
                spinAnimation.start();
            }
            wasPlaying = nowPlaying;
        }
    }

    // ─── Opacity transitions ──────────────────────────────────────
    PropertyAnimation on opacity {
        id: fadeInAnimation
        from: 0.0
        to: 1.0
        duration: 150
    }

    PropertyAnimation on opacity {
        id: fadeOutAnimation
        from: 1.0
        to: 0.0
        duration: 300
    }

    // ─── Normal spinning (while playing) ───────────────────────────
    PropertyAnimation {
        id: spinAnimation
        target: vinylIcon
        property: "rotation"
        from: 0
        to: 360
        duration: 1000  // 1 RPM ≈ 60 BPM
        loops: Animation.Infinite
        running: false
    }

    // ─── Deceleration animation (audible vinyl stop) ───────────────
    // Uses a custom animation that decreases rotation speed gradually,
    // matching the audio pitch drop from the engine.
    NumberAnimation {
        id: decelAnimation
        target: root
        property: "rotationSpeed"
        from: 360
        to: 0
        duration: decelDurationMs
        easing.type: Easing.OutCubic  // Smooth deceleration curve

        onStarted: {
            // Stop the normal spin and begin deceleration
            spinAnimation.stop();
            // Reset to current rotation so the transition is seamless
        }

        onStopped: {
            if (rotationSpeed <= 0) {
                // Fully stopped — fade out the overlay
                fadeOutAnimation.start();
            }
        }
    }

    // ─── Manual rotation driven by rotationSpeed ──────────────────
    // Since we need variable-speed rotation during deceleration, we
    // drive it with a timer instead of a fixed-duration animation.
    Timer {
        id: rotationTimer
        interval: 16  // ~60 fps
        repeat: true
        running: root.opacity > 0

        onTriggered: {
            if (rotationSpeed > 0 && spinAnimation.running === false) {
                // During deceleration: rotate by current speed * dt
                vinylIcon.rotation = (vinylIcon.rotation + rotationSpeed * 0.016) % 360;
            }
        }
    }

    // ─── Visual: vinyl record ─────────────────────────────────────
    Rectangle {
        id: vinylIcon
        anchors.centerIn: parent
        width: root.vinylSize
        height: root.vinylSize
        radius: width / 2
        color: "#1a1a1a"
        border.width: 2
        border.color: "#333"

        // Grooves
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.9
            height: parent.height * 0.9
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: "#2a2a2a"
        }
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: parent.height * 0.75
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: "#2a2a2a"
        }
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: parent.height * 0.5
            radius: width / 2
            color: "#222"
            border.width: 1
            border.color: "#444"

            // Center label
            Text {
                anchors.centerIn: parent
                text: "●"
                color: "#888"
                font.pixelSize: 14
            }
        }
    }
}
