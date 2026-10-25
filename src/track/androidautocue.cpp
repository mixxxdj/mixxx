#include "track/androidautocue.h"

#include <cmath>

#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "track/beats.h"
#include "track/cue.h"
#include "track/cueinfo.h"
#include "track/track.h"

namespace mixxx {
namespace android {

namespace {

bool samePosition(mixxx::audio::FramePos a, mixxx::audio::FramePos b) {
    return a.isValid() && b.isValid() && std::abs(a.value() - b.value()) < 1;
}

void connectDeck(PlayerManager* pPlayerManager, BaseTrackPlayer* pDeck) {
    // The deck is only connected once
    if (!pDeck || pDeck->property("androidAutoCue").toBool()) {
        return;
    }
    pDeck->setProperty("androidAutoCue", true);
    // loadingTrack is emitted before the engine loads the track and jumps to
    // its main cue
    QObject::connect(pDeck,
            &BaseTrackPlayer::loadingTrack,
            pPlayerManager,
            [](TrackPointer pNewTrack, TrackPointer) {
                if (pNewTrack) {
                    snapAutoCueToBeat(pNewTrack.get());
                }
            });
}

void connectDecks(PlayerManager* pPlayerManager) {
    for (int i = 0; i < pPlayerManager->numberOfDecks(); ++i) {
        connectDeck(pPlayerManager, pPlayerManager->getDeckBase(i));
    }
}

} // namespace

bool snapAutoCueToBeat(Track* pTrack) {
    const CuePointer pFirstSound = pTrack->findCueByType(CueType::N60dBSound);
    const BeatsPointer pBeats = pTrack->getBeats();
    if (!pFirstSound || !pBeats) {
        return false;
    }
    const mixxx::audio::FramePos firstSound = pFirstSound->getPosition();
    if (!firstSound.isValid()) {
        return false;
    }
    mixxx::audio::FramePos previousBeat;
    mixxx::audio::FramePos nextBeat;
    if (!pBeats->findPrevNextBeats(firstSound, &previousBeat, &nextBeat, false) ||
            !previousBeat.isValid() || !nextBeat.isValid()) {
        return false;
    }
    const double beatLength = nextBeat - previousBeat;
    const mixxx::audio::FramePos beat =
            (firstSound - previousBeat <= nextBeat - firstSound) ? previousBeat : nextBeat;
    if (beatLength <= 0 || samePosition(beat, firstSound) ||
            std::abs(beat.value() - firstSound.value()) > beatLength / 4 || beat.value() < 0) {
        return false;
    }

    bool moved = false;
    if (samePosition(pTrack->getMainCuePosition(), firstSound)) {
        pTrack->setMainCuePosition(beat);
        moved = true;
    }
    const CuePointer pIntro = pTrack->findCueByType(CueType::Intro);
    if (pIntro && samePosition(pIntro->getPosition(), firstSound)) {
        pIntro->setStartPosition(beat);
        moved = true;
    }
    return moved;
}

void snapAutoCuesOnLoad(PlayerManager* pPlayerManager) {
    connectDecks(pPlayerManager);
    QObject::connect(pPlayerManager,
            &PlayerManager::numberOfDecksChanged,
            pPlayerManager,
            [pPlayerManager](int) { connectDecks(pPlayerManager); });
}

} // namespace android
} // namespace mixxx
