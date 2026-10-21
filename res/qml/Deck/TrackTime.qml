import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Skin.EmbeddedText {
    id: root

    enum Display {
        Elapsed,
        Remaining,
        Both
    }
    enum Mode {
        Traditional,
        TraditionalCoarse,
        Seconds,
        SecondsLong,
        KiloSeconds,
        HectoSeconds
    }

    property var display: TrackTime.Mode.Display
    property double elapsed: durationControl.value
    property string group: "[Channel1]"
    property var mode: TrackTime.Mode.Traditional
    property double remaining: durationControl.value * (1 - playPositionControl.value)

    function toTime(value) {
        if (!Number.isFinite(value)) {
            return "";
        }

        const sign = value < 0 ? "-" : "";
        return sign + Mixxx.DurationFormatter.format(Math.abs(value), root.mode);
    }

    text: {
        switch (root.display) {
        case TrackTime.Display.Remaining:
            return `-${toTime(root.remaining)}`;
        case TrackTime.Display.Both:
            return `${toTime(root.elapsed)}  -${toTime(root.remaining)}`;
        default:
            console.warn(`Unsupported track time display: ${root.display}. Defaulting to elapsed`);
        case TrackTime.Display.Elapsed:
            return toTime(root.elapsed);
        }
    }

    Mixxx.ControlProxy {
        id: durationControl

        group: root.group
        key: "duration"
    }
    Mixxx.ControlProxy {
        id: playPositionControl

        group: root.group
        key: "playposition"
    }
}
