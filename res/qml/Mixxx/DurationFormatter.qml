pragma Singleton

import QtQml 2.12

QtObject {
    id: root

    enum Mode {
        Traditional,
        TraditionalCoarse,
        Seconds,
        SecondsLong,
        KiloSeconds,
        HectoSeconds
    }

    function format(seconds, mode) {
        if (!Number.isFinite(seconds) || seconds < 0) {
            return "?";
        }

        switch (mode) {
        case DurationFormatter.Mode.TraditionalCoarse:
            return formatTime(seconds, false);
        case DurationFormatter.Mode.Seconds:
            return formatSeconds(seconds, 0);
        case DurationFormatter.Mode.SecondsLong:
            return formatSeconds(seconds, 6);
        case DurationFormatter.Mode.KiloSeconds:
            return formatKiloSeconds(seconds);
        case DurationFormatter.Mode.HectoSeconds:
        case DurationFormatter.Mode.Traditional:
        default:
            return formatTime(seconds, true);
        }
    }
    function formatKiloSeconds(seconds) {
        const kilos = Math.trunc(seconds / 1000);
        const wholeSeconds = Math.floor(seconds % 1000);
        const milliseconds = (seconds % 1).toFixed(3).slice(-3);
        return `${kilos}.${zeroPad(wholeSeconds, 3)}\u2009${milliseconds.slice(0, 2)}`;
    }
    function formatSeconds(seconds, minimumWidth) {
        return seconds.toFixed(2).padStart(minimumWidth, "0");
    }
    function formatTime(seconds, showCentiseconds) {
        const totalMilliseconds = Math.trunc(seconds * 1000);
        const totalHours = Math.floor(totalMilliseconds / 3600000);
        const minutes = Math.floor(totalMilliseconds / 60000) % 60;
        const wholeSeconds = Math.floor(totalMilliseconds / 1000) % 60;

        let result = totalHours > 0 ? `${totalHours}:${zeroPad(minutes, 2)}:${zeroPad(wholeSeconds, 2)}` : `${minutes}:${zeroPad(wholeSeconds, 2)}`;
        if (showCentiseconds) {
            result += `.${zeroPad(Math.floor(totalMilliseconds / 10) % 100, 2)}`;
        }
        return result;
    }
    function zeroPad(value, width) {
        return String(value).padStart(width, "0");
    }
}
