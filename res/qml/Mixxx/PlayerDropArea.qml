import Mixxx 1.0 as Mixxx
import QtQuick 2.12

// Handles drops on decks and samplers
DropArea {
    id: root

    required property string group
    property var player: Mixxx.PlayerManager.getPlayer(group)

    function firstDroppedUrl(drop) {
        if (drop.hasUrls && drop.urls.length > 0) {
            return drop.urls[0];
        }

        const formats = drop.formats || [];
        for (const format of ["text/uri-list", "text/plain"]) {
            if (!formats.includes(format)) {
                continue;
            }
            const data = drop.getDataAsString(format) || "";
            const firstLine = data.split(/\r?\n/)
                    .map(line => line.trim())
                    .find(line => line.length > 0 && !line.startsWith("#"));
            if (firstLine) {
                return Qt.resolvedUrl(firstLine);
            }
        }
        return null;
    }

    onDropped: (drop) => {
        if (drop.formats.includes("mixxx/player")) {
            const sourceGroup = drop.getDataAsString("mixxx/player");
            // Prevent dropping a deck onto itself
            if (sourceGroup == root.group)
                return ;

            console.log("Drag from group " + sourceGroup);
            player.cloneFromGroup(sourceGroup);
            drop.accepted = true;
            return ;
        }
        const url = root.firstDroppedUrl(drop);
        if (url) {
            console.log("Dropped URL '" + url + "' on deck " + group);
            player.loadTrackFromLocationUrl(url);
            drop.accepted = true;
            return ;
        }
    }
}
