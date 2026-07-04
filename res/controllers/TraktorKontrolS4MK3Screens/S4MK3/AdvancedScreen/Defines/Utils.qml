import QtQuick 2.15

QtObject {

    function convertToTimeString(inSeconds) {
        var neg = (inSeconds < 0);
        var roundedSec = Math.floor(inSeconds);

        if (neg) {
            roundedSec = -roundedSec;
        }

        var sec = roundedSec % 60;
        var min = (roundedSec - sec) / 60;

        var secStr = sec.toString();
        if (sec < 10) secStr = "0" + secStr;

        var minStr = min.toString();
        if (min < 10) minStr = "0" + minStr;

        return (neg ? "-" : "") + minStr + ":" + secStr;
    }

    function computeRemainingTimeString(length, elapsed) {
        return ((elapsed > length) ? convertToTimeString(0) : convertToTimeString( Math.floor(elapsed) - Math.floor(length)));
    }

    function camelotConvert(keyToConvert) {
        if (keyToConvert == "") return "-";

        switch(keyToConvert) {
            case "1d": return "8B";
            case "2d": return "9B";
            case "3d": return "10B";
            case "4d": return "11B";
            case "5d": return "12B";
            case "6d": return "1B";
            case "7d": return "2B";
            case "8d": return "3B";
            case "9d": return "4B";
            case "10d": return "5B";
            case "11d": return "6B";
            case "12d": return "7B";

            case "1m": return "8A";
            case "2m": return "9A";
            case "3m": return "10A";
            case "4m": return "11A";
            case "5m": return "12A";
            case "6m": return "1A";
            case "7m": return "2A";
            case "8m": return "3A";
            case "9m": return "4A";
            case "10m": return "5A";
            case "11m": return "6A";
            case "12m": return "7A";

            case "1D": return "8B";
            case "2D": return "9B";
            case "3D": return "10B";
            case "4D": return "11B";
            case "5D": return "12B";
            case "6D": return "1B";
            case "7D": return "2B";
            case "8D": return "3B";
            case "9D": return "4B";
            case "10D": return "5B";
            case "11D": return "6B";
            case "12D": return "7B";

            case "1M": return "8A";
            case "2M": return "9A";
            case "3M": return "10A";
            case "4M": return "11A";
            case "5M": return "12A";
            case "6M": return "1A";
            case "7M": return "2A";
            case "8M": return "3A";
            case "9M": return "4A";
            case "10M": return "5A";
            case "11M": return "6A";
            case "12M": return "7A";

            case "B": return "1B";
            case "F#": return "2B";
            case "C#": return "3B";
            case "G#": return "4B";
            case "D#": return "5B";
            case "A#": return "6B";
            case "F": return "7B";
            case "C": return "8B";
            case "G": return "9B";
            case "D": return "10B";
            case "A": return "11B";
            case "E": return "12B";

            case "G#m": return "1A";
            case "D#m": return "2A";
            case "A#m": return "3A";
            case "Fm": return "4A";
            case "Cm": return "5A";
            case "Gm": return "6A";
            case "Dm": return "7A";
            case "Am": return "8A";
            case "Em": return "9A";
            case "Bm": return "10A";
            case "F#m": return "11A";
            case "C#m": return "12A";

            case "G#M": return "1A";
            case "D#M": return "2A";
            case "A#M": return "3A";
            case "FM": return "4A";
            case "CM": return "5A";
            case "GM": return "6A";
            case "DM": return "7A";
            case "AM": return "8A";
            case "EM": return "9A";
            case "BM": return "10A";
            case "F#M": return "11A";
            case "C#M": return "12A";
        }
        return "ERR";
    }
}
