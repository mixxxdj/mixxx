import Mixxx 1.0 as Mixxx
import QtQuick

Image {
    id: root

    property url chargedSource
    property var chargingSources: []
    property var dischargingSources: []

    function batterySource() {
        if (!Mixxx.Battery.isBatteryAvailable) {
            return "";
        }

        const percentage = Math.max(0, Math.min(100, Mixxx.Battery.percentage));
        if (Mixxx.Battery.isCharging && percentage >= 99) {
            return chargedSource;
        }

        const sources = Mixxx.Battery.isCharging ? chargingSources : dischargingSources;
        if (sources.length === 0) {
            return "";
        }

        const level = Math.max(0, Math.min(sources.length - 1, Math.floor(percentage / (100 / sources.length))));
        return sources[level];
    }

    source: batterySource()
}
