import Mixxx 1.0 as Mixxx
import QtQuick 2.12

// Handles drops on decks and samplers
DropArea {
    id: root

    required property string group
    property var player: Mixxx.PlayerManager.getPlayer(group)

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
        if (drop.hasUrls && drop.urls.length > 0) {
            let url = drop.urls[0];
            console.log("Dropped URL '" + url + "' on deck " + group);
            player.loadTrackFromLocationUrl(url);
            drop.accepted = true;
            return ;
        }
    }
}
