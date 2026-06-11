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
        let result = "";
        switch (root.mode) {
        case TrackTime.Mode.Seconds:
        case TrackTime.Mode.SecondsLong:
            {
                let seconds = parseInt(value).toString();
                let subs = value % 1;
                return `${seconds.padStart(root.mode === TrackTime.Mode.SecondsLong ? 3 : 0, '0')}.${subs.toFixed(2).slice(-2)}`;
            }
        case TrackTime.Mode.KiloSeconds:
            {
                let kilos = parseInt(value / 1000);
                let seconds = parseInt(value % 1000).toString();
                let subs = value % 1;
                return `${kilos}.${seconds.padStart(3, '0')} ${subs.toFixed(2).slice(-2)}`;
            }
        case TrackTime.Mode.HectoSeconds:
            return `???`;
        default:
            console.warn(`Unsupported track time mode: ${root.mode}. Defaulting to traditional`);
        case TrackTime.Mode.Traditional:
        case TrackTime.Mode.TraditionalCoarse:
            {
                let component = [];
                if (remaining + elapsed > 3600) {
                    component.push(parseInt(value / 3600).toString().padStart(2, '0'));
                }
                component.push(parseInt(value / 60).toString().padStart(2, '0'));
                component.push(parseInt(value % 60).toString().padStart(2, '0'));
                if (root.mode !== TrackTime.Mode.TraditionalCoarse) {
                    component[component.length - 1] += `.${(value % 1).toFixed(2).slice(-2)}`;
                }
                return component.join(':');
            }
        }
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
