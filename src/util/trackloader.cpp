#include "util/trackloader.h"
#include "control/controlobject.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "preferences/configobject.h"
#include "preferences/dialog/dlgprefdeck.h"

namespace mixxx {

struct TrackLoadDecision {
    bool canLoad;
    bool continuePlaying;
};

TrackLoadDecision TrackLoader::canLoadTrackIntoPlayingDeck(const ConfigPointer& pConfig, const QString& group) {
    bool allowLoadTrackIntoPlayingDeck = false;
    bool deckShouldContinuePlaying = true;

    if (pConfig->exists(kConfigKeyLoadWhenDeckPlaying)) {
        int loadWhenDeckPlaying = pConfig->getValueString(kConfigKeyLoadWhenDeckPlaying).toInt();
        switch (static_cast<LoadWhenDeckPlaying>(loadWhenDeckPlaying)) {
        case LoadWhenDeckPlaying::Allow:
            allowLoadTrackIntoPlayingDeck = true;
            deckShouldContinuePlaying = true;
            break;
        case LoadWhenDeckPlaying::AllowButStopDeck:
            allowLoadTrackIntoPlayingDeck = true;
            deckShouldContinuePlaying = false;
            break;
        case LoadWhenDeckPlaying::Reject:
            allowLoadTrackIntoPlayingDeck = false;
            break;
        }
    } else {
        allowLoadTrackIntoPlayingDeck = pConfig->getValue<bool>(kConfigKeyAllowTrackLoadToPlayingDeck);
    }

    // Always allow loading if the deck is not playing
    if (ControlObject::get(ConfigKey(group, "play")) <= 0.0) {
        allowLoadTrackIntoPlayingDeck = true;
    }

    return {allowLoadTrackIntoPlayingDeck, deckShouldContinuePlaying};
}

} // namespace mixxx