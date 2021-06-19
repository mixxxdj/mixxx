import Mixxx 0.1 as Mixxx
import QtQuick 2.12

// Handles drops on decks and samplers
DropArea {
    property string group // required
    property var player: Mixxx.PlayerManager.getPlayer(group)

    onDropped: {
        if (drop.hasUrls) {
            let url = drop.urls[0];
            console.log("Dropped URL '" + url + "' on deck " + group);
            player.loadTrackFromLocationUrl(url);
        }
    }
}
