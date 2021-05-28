#include "engine/sync/enginesync.h"

#include <QMetaType>
#include <QStringList>

#include "engine/channels/enginechannel.h"
#include "engine/enginebuffer.h"
#include "engine/sync/internalclock.h"
#include "util/assert.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("EngineSync");
const QString kInternalClockGroup = QStringLiteral("[InternalClock]");
} // anonymous namespace

EngineSync::EngineSync(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pInternalClock(new InternalClock(kInternalClockGroup, this)),
          m_pLeaderSyncable(nullptr) {
    qRegisterMetaType<SyncMode>("SyncMode");
    m_pInternalClock->updateLeaderBpm(124.0);
}

EngineSync::~EngineSync() {
    // We use the slider value because that is never set to 0.0.
    m_pConfig->set(ConfigKey(kInternalClockGroup, "bpm"), ConfigValue(m_pInternalClock->getBpm()));
    delete m_pInternalClock;
}

void EngineSync::requestSyncMode(Syncable* pSyncable, SyncMode mode) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestSyncMode" << pSyncable->getGroup() << mode;
    }
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        return;
    }

    // There are two stages to setting the mode: first, figuring out
    // the pSyncable's new mode (it may not be the one they requested),
    // and activating the appropriate modes in it as well as possibly other
    // decks that need to change as a result.
    Syncable* oldLeader = m_pLeaderSyncable;
    switch (mode) {
    case SYNC_LEADER_EXPLICIT:
    case SYNC_LEADER_SOFT: {
        if (pSyncable->getBaseBpm() > 0) {
            activateLeader(pSyncable, mode);
        } else {
            // Because we don't have a valid bpm, we can't be the leader
            // (or else everyone would try to be syncing to zero bpm).
            // Override and make us a follower instead.
            activateFollower(pSyncable);
        }
        break;
    }
    case SYNC_FOLLOWER: {
        // A request for follower mode may be converted into an enabling of soft
        // leader mode.
        activateFollower(pSyncable);
        Syncable* newLeader = pickLeader(pSyncable);
        if (newLeader && newLeader != m_pLeaderSyncable) {
            // if the leader has changed, activate it (this updates m_pLeaderSyncable)
            activateLeader(newLeader, SYNC_LEADER_SOFT);
        }
        break;
    }
    case SYNC_NONE: {
        if (pSyncable != m_pInternalClock) {
            deactivateSync(pSyncable);
        }
        break;
    }
    default:;
    }

    Syncable* pOnlyPlayer = getUniquePlayingSyncedDeck();
    if (pOnlyPlayer) {
        // This resets the user offset, so that if this deck gets used as the params syncable
        // it will have that offset removed.
        pOnlyPlayer->notifyUniquePlaying();
    }

    // Second, figure out what Syncable should be used to initialize the leader
    // parameters, if any. Usually this is the new leader. (Note, that pointer might be null!)
    Syncable* pParamsSyncable = m_pLeaderSyncable;
    // But If we are newly soft leader, we need to match to some other deck.
    if (pSyncable == m_pLeaderSyncable && pSyncable != oldLeader && mode != SYNC_LEADER_EXPLICIT) {
        pParamsSyncable = findBpmMatchTarget(pSyncable);
        if (!pParamsSyncable) {
            // We weren't able to find anything to match to, so set ourselves as the
            // target.  That way we'll use our own params when we updateLeaderParams below.
            pParamsSyncable = pSyncable;
        }
    }
    // Now that all of the decks have their assignments, reinit leader params if needed.
    if (pParamsSyncable) {
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "EngineSync::requestSyncMode setting leader params from "
                    << pParamsSyncable->getGroup();
        }
        reinitLeaderParams(pParamsSyncable);
        pSyncable->updateInstantaneousBpm(pParamsSyncable->getBpm());
        if (pParamsSyncable != pSyncable) {
            pSyncable->requestSync();
        }
    }

}

void EngineSync::activateFollower(Syncable* pSyncable) {
    if (pSyncable == nullptr) {
        qWarning() << "WARNING: Logic Error: Called activateFollower on a nullptr Syncable.";
        return;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateFollower: "
                        << pSyncable->getGroup();
    }

    if (m_pLeaderSyncable == pSyncable) {
        m_pLeaderSyncable = nullptr;
    }

    pSyncable->setSyncMode(SYNC_FOLLOWER);
}

void EngineSync::activateLeader(Syncable* pSyncable, SyncMode leaderType) {
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        qWarning() << "WARNING: Logic Error: Called activateLeader on a nullptr Syncable.";
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(leaderType == SYNC_LEADER_SOFT || leaderType == SYNC_LEADER_EXPLICIT) {
        qWarning() << "WARNING: Logic Error: Called activateLeader with non-leader mode";
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateLeader: "
                        << pSyncable->getGroup() << "type: "
                        << leaderType;
    }

    if (m_pLeaderSyncable == pSyncable) {
        // Already leader, update the leader type.
        if (m_pLeaderSyncable->getSyncMode() != leaderType) {
            m_pLeaderSyncable->setSyncMode(leaderType);
        }
        // nothing else to do
        return;
    }

    // If a different channel is already leader, disable it.
    Syncable* pOldChannelLeader = m_pLeaderSyncable;
    m_pLeaderSyncable = nullptr;
    if (pOldChannelLeader) {
        pOldChannelLeader->setSyncMode(SYNC_FOLLOWER);
    }

    m_pLeaderSyncable = pSyncable;
    pSyncable->setSyncMode(leaderType);

    if (m_pLeaderSyncable != m_pInternalClock) {
        // the internal clock gets activated and its values are overwritten with this
        // new deck.
        activateFollower(m_pInternalClock);
    }

    // It is up to callers of this function to initialize bpm and beat_distance
    // if necessary.
}

void EngineSync::deactivateSync(Syncable* pSyncable) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::deactivateSync" << pSyncable->getGroup();
    }
    bool wasLeader = isSyncLeader(pSyncable);
    if (wasLeader) {
        m_pLeaderSyncable = nullptr;
    }

    // Notifications happen after-the-fact.
    pSyncable->setSyncMode(SYNC_NONE);

    bool bSyncDeckExists = syncDeckExists();
    if (pSyncable != m_pInternalClock && !bSyncDeckExists) {
        // Deactivate the internal clock if there are no more sync decks left.
        m_pLeaderSyncable = nullptr;
        m_pInternalClock->setSyncMode(SYNC_NONE);
        return;
    }

    if (wasLeader) {
        Syncable* newLeader = pickLeader(nullptr);
        if (newLeader != nullptr) {
            activateLeader(newLeader, SYNC_LEADER_SOFT);
        }
    }
}

Syncable* EngineSync::pickLeader(Syncable* enabling_syncable) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::pickLeader";
    }
    if (m_pLeaderSyncable &&
            m_pLeaderSyncable->getSyncMode() == SYNC_LEADER_EXPLICIT &&
            m_pLeaderSyncable->getBaseBpm() != 0.0) {
        return m_pLeaderSyncable;
    }

    // First preference: some other sync deck that is not playing.
    // Note, if we are using PREFER_LOCK_BPM we don't use this option.
    Syncable* first_other_playing_deck = nullptr;
    // Second preference: whatever the first playing sync deck is, even if it's us.
    Syncable* first_playing_deck = nullptr;
    // Third preference: the first stopped sync deck.
    Syncable* first_stopped_deck = nullptr;
    // Last resorts: Internal Clock or nullptr.

    int stopped_deck_count = 0;
    int playing_deck_count = 0;

    for (const auto& pSyncable : qAsConst(m_syncables)) {
        if (pSyncable->getBaseBpm() <= 0.0) {
            continue;
        }

        if (pSyncable != enabling_syncable) {
            if (!pSyncable->getChannel()->isPrimaryDeck()) {
                continue;
            }
            if (!pSyncable->isSynchronized()) {
                continue;
            }
        }

        if (pSyncable->isPlaying() && pSyncable->isAudible()) {
            if (playing_deck_count == 0) {
                first_playing_deck = pSyncable;
            }
            if (!first_other_playing_deck && pSyncable != enabling_syncable) {
                first_other_playing_deck = pSyncable;
            }
            playing_deck_count++;
        } else {
            if (stopped_deck_count == 0) {
                first_stopped_deck = pSyncable;
            }
            stopped_deck_count++;
        }
    }

    const SyncLockAlgorithm picker = static_cast<SyncLockAlgorithm>(
            m_pConfig->getValue<int>(ConfigKey("[BPM]", "sync_lock_algorithm"),
                    PREFER_IMPLICIT_LEADER));
    switch (picker) {
    case PREFER_IMPLICIT_LEADER:
        // Always pick a deck for a new leader.
        if (playing_deck_count == 1) {
            return first_playing_deck;
        } else if (playing_deck_count > 1) {
            return first_other_playing_deck;
        }

        if (stopped_deck_count >= 1) {
            return first_stopped_deck;
        }
        break;
    case PREFER_LOCK_BPM:
        // Old 2.3 behavior:
        // Lock the bpm if there is more than one playing sync deck
        if (playing_deck_count == 1) {
            return first_playing_deck;
        } else if (playing_deck_count > 1) {
            return m_pInternalClock;
        }

        if (stopped_deck_count >= 1) {
            return first_stopped_deck;
        }
        break;
    }

    return nullptr;
}

Syncable* EngineSync::findBpmMatchTarget(Syncable* requester) {
    // Iterates through all decks *except* the requester, and picks:
    // First preference: playing synced deck
    // Second preferene: stopped synced deck
    // Third preference: playing nonsync deck
    // Fourth preference: stopped nonsync deck
    // This could probably be rewritten with a nicer algorithm.

    Syncable* pStoppedSyncTarget = nullptr;
    Syncable* pPlayingNonSyncTarget = nullptr;
    Syncable* pStoppedNonSyncTarget = nullptr;

    for (const auto& pOtherSyncable : qAsConst(m_syncables)) {
        if (pOtherSyncable == requester) {
            continue;
        }
        // Skip non-leader decks, like preview decks.
        if (!pOtherSyncable->getChannel()->isMasterEnabled()) {
            continue;
        }
        if (!pOtherSyncable->getChannel()->isPrimaryDeck()) {
            continue;
        }
        if (pOtherSyncable->getBaseBpm() == 0.0) {
            continue;
        }

        // If the other deck is playing we stop looking immediately. Otherwise continue looking
        // for a playing deck with bpm > 0.0.
        if (pOtherSyncable->isPlaying() && pOtherSyncable->isAudible()) {
            if (pOtherSyncable->isSynchronized()) {
                return pOtherSyncable;
            }
            if (!pPlayingNonSyncTarget) {
                pPlayingNonSyncTarget = pOtherSyncable;
            }
        }

        // The target is not playing. If this is the first one we have seen,
        // record it. If we never find a playing target, we'll return
        // this one as a fallback.
        // Exception: if the requester is playing, we don't want to match it
        // against a stopped deck.
        if (!(requester->isPlaying() && requester->isAudible())) {
            if (!pStoppedSyncTarget && pOtherSyncable->isSynchronized()) {
                pStoppedSyncTarget = pOtherSyncable;
            } else if (!pStoppedNonSyncTarget) {
                pStoppedNonSyncTarget = pOtherSyncable;
            }
        }
    }

    if (pStoppedSyncTarget) {
        return pStoppedSyncTarget;
    }

    if (pPlayingNonSyncTarget) {
        return pPlayingNonSyncTarget;
    }

    return pStoppedNonSyncTarget;
}

void EngineSync::notifyPlayingAudible(Syncable* pSyncable, bool playingAudible) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyPlayingAudible"
                        << pSyncable->getGroup() << playingAudible;
    }
    // For now we don't care if the deck is now playing or stopping.
    if (!pSyncable->isSynchronized()) {
        return;
    }

    // similar to enablesync -- we pick a new leader and maybe reinit.
    Syncable* newLeader = pickLeader(pSyncable);

    if (newLeader != nullptr && newLeader != m_pLeaderSyncable) {
        activateLeader(newLeader, SYNC_LEADER_SOFT);
        reinitLeaderParams(newLeader);
    } else {
        Syncable* pOnlyPlayer = getUniquePlayingSyncedDeck();
        if (pOnlyPlayer) {
            // Even if we didn't change leader, if there is only one player (us), then we should
            // update the beat distance.
            pOnlyPlayer->notifyUniquePlaying();
            updateLeaderBeatDistance(pOnlyPlayer, pOnlyPlayer->getBeatDistance());
        }
    }

    pSyncable->requestSync();
}

void EngineSync::notifyScratching(Syncable* pSyncable, bool scratching) {
    // No special behavior for now.
    Q_UNUSED(pSyncable);
    Q_UNUSED(scratching);
}

void EngineSync::notifyBaseBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBaseBpmChanged" << pSyncable->getGroup() << bpm;
    }

    if (isSyncLeader(pSyncable)) {
        updateLeaderBpm(pSyncable, bpm);
    }
}

void EngineSync::notifyRateChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyRateChanged" << pSyncable->getGroup() << bpm;
    }

    updateLeaderBpm(pSyncable, bpm);
}

void EngineSync::requestBpmUpdate(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestBpmUpdate" << pSyncable->getGroup() << bpm;
    }

    double mbaseBpm = 0.0;
    double mbpm = 0.0;
    double beatDistance = 0.0;
    if (m_pLeaderSyncable) {
        mbaseBpm = m_pLeaderSyncable->getBaseBpm();
        mbpm = m_pLeaderSyncable->getBpm();
        beatDistance = m_pLeaderSyncable->getBeatDistance();
    }

    if (mbaseBpm != 0.0) {
        // update from current leader
        pSyncable->updateLeaderBeatDistance(beatDistance);
        pSyncable->updateLeaderBpm(mbpm);
    } else {
        // There is no leader, adopt this bpm as leader value
        pSyncable->updateLeaderBeatDistance(0.0);
        pSyncable->updateLeaderBpm(bpm);
    }
}

void EngineSync::notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyInstantaneousBpmChanged" << pSyncable->getGroup() << bpm;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    // Do not update the leader rate slider because instantaneous changes are
    // not user visible.
    updateLeaderInstantaneousBpm(pSyncable, bpm);
}

void EngineSync::notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBeatDistanceChanged"
                        << pSyncable->getGroup() << beatDistance;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    updateLeaderBeatDistance(pSyncable, beatDistance);
}

Syncable* EngineSync::pickNonSyncSyncTarget(EngineChannel* pDontPick) const {
    // First choice: the sync leader, if it's a deck
    if (m_pLeaderSyncable &&
            m_pLeaderSyncable->getChannel() &&
            m_pLeaderSyncable->getChannel() != pDontPick) {
        return m_pLeaderSyncable;
    }

    Syncable* pFirstPlayingDeck = nullptr;
    Syncable* pFirstNonplayingDeck = nullptr;
    foreach (Syncable* pSyncable, m_syncables) {
        EngineChannel* pChannel = pSyncable->getChannel();
        // Exclude non-decks
        if (pChannel == nullptr || pChannel == pDontPick) {
            continue;
        }

        // Only consider channels that have a track loaded, are in the leader
        // mix, and are primary decks.
        if (pChannel->isActive() && pChannel->isMasterEnabled() && pChannel->isPrimaryDeck()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                if (pBuffer->getSpeed() != 0.0) {
                    if (pSyncable->getSyncMode() != SYNC_NONE) {
                        // Second choice: first playing sync deck
                        return pSyncable;
                    }
                    if (pFirstPlayingDeck == nullptr) {
                        pFirstPlayingDeck = pSyncable;
                    }
                } else if (pFirstNonplayingDeck == nullptr) {
                    pFirstNonplayingDeck = pSyncable;
                }
            }
        }
    }
    if (pFirstPlayingDeck) {
        // Third choice: first playing non-sync deck
        return pFirstPlayingDeck;
    }

    // No playing decks have a BPM. Go with the first deck that was stopped but
    // had a BPM.
    return pFirstNonplayingDeck;
}

bool EngineSync::otherSyncedPlaying(const QString& group) {
    bool othersInSync = false;
    for (Syncable* theSyncable : qAsConst(m_syncables)) {
        bool isSynchonized = theSyncable->isSynchronized();
        if (theSyncable->getGroup() == group) {
            if (!isSynchonized) {
                return false;
            }
            continue;
        }
        // We normally check for isAudible, but in this case we want to be less
        // strict.
        if (theSyncable->isPlaying() && isSynchonized) {
            othersInSync = true;
        }
    }
    return othersInSync;
}

void EngineSync::addSyncableDeck(Syncable* pSyncable) {
    if (m_syncables.contains(pSyncable)) {
        qDebug() << "EngineSync: already has" << pSyncable;
        return;
    }
    m_syncables.append(pSyncable);
}

void EngineSync::onCallbackStart(int sampleRate, int bufferSize) {
    m_pInternalClock->onCallbackStart(sampleRate, bufferSize);
}

void EngineSync::onCallbackEnd(int sampleRate, int bufferSize) {
    m_pInternalClock->onCallbackEnd(sampleRate, bufferSize);
}

EngineChannel* EngineSync::getLeader() const {
    return m_pLeaderSyncable ? m_pLeaderSyncable->getChannel() : nullptr;
}

Syncable* EngineSync::getSyncableForGroup(const QString& group) {
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable->getGroup() == group) {
            return pSyncable;
        }
    }
    return nullptr;
}

bool EngineSync::syncDeckExists() const {
    for (const auto& pSyncable : qAsConst(m_syncables)) {
        if (pSyncable->isSynchronized() && pSyncable->getBaseBpm() > 0) {
            return true;
        }
    }
    return false;
}

double EngineSync::leaderBpm() const {
    if (m_pLeaderSyncable) {
        return m_pLeaderSyncable->getBpm();
    }
    return m_pInternalClock->getBpm();
}

double EngineSync::leaderBeatDistance() const {
    if (m_pLeaderSyncable) {
        return m_pLeaderSyncable->getBeatDistance();
    }
    return m_pInternalClock->getBeatDistance();
}

double EngineSync::leaderBaseBpm() const {
    if (m_pLeaderSyncable) {
        return m_pLeaderSyncable->getBaseBpm();
    }
    return m_pInternalClock->getBaseBpm();
}

void EngineSync::updateLeaderBpm(Syncable* pSource, double bpm) {
    //qDebug() << "EngineSync::updateLeaderBpm" << pSource << bpm;
    if (pSource != m_pInternalClock) {
        m_pInternalClock->updateLeaderBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->updateLeaderBpm(bpm);
    }
}

void EngineSync::updateLeaderInstantaneousBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->updateInstantaneousBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->updateInstantaneousBpm(bpm);
    }
}

void EngineSync::updateLeaderBeatDistance(Syncable* pSource, double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::updateLeaderBeatDistance"
                        << (pSource ? pSource->getGroup() : "null")
                        << beatDistance;
    }
    if (pSource != m_pInternalClock) {
        m_pInternalClock->updateLeaderBeatDistance(beatDistance);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->updateLeaderBeatDistance(beatDistance);
    }
}

void EngineSync::reinitLeaderParams(Syncable* pSource) {
    // Important note! Because of the way sync works, the new leader is usually not the same
    // as the Syncable setting the leader parameters (here, pSource). Notify the proper Syncable
    // so it can prepare itself.  (This is a hack to undo half/double math so that we initialize
    // based on un-multiplied bpm values).
    pSource->notifyLeaderParamSource();

    double beatDistance = pSource->getBeatDistance();
    if (!pSource->isPlaying()) {
        // If the params source is not playing, but other syncables are, then we are a stopped
        // explicit Leader and we should not initialize the beat distance.  Take it from the
        // internal clock instead, because that will be up to date with the playing deck(s).
        bool playingSyncables = false;
        for (Syncable* pSyncable : qAsConst(m_syncables)) {
            if (pSyncable == pSource) {
                continue;
            }
            if (!pSyncable->isSynchronized()) {
                continue;
            }
            if (pSyncable->isPlaying()) {
                playingSyncables = true;
                break;
            }
        }
        if (playingSyncables) {
            beatDistance = m_pInternalClock->getBeatDistance();
        }
    }
    const double baseBpm = pSource->getBaseBpm();
    double bpm = pSource->getBpm();
    if (bpm <= 0) {
        bpm = baseBpm;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "BaseSyncableListener::reinitLeaderParams, source is"
                        << pSource->getGroup() << beatDistance << baseBpm << bpm;
    }
    if (pSource != m_pInternalClock) {
        m_pInternalClock->reinitLeaderParams(beatDistance, baseBpm, bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (!pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->reinitLeaderParams(beatDistance, baseBpm, bpm);
    }
}

Syncable* EngineSync::getUniquePlayingSyncedDeck() const {
    Syncable* onlyPlaying = nullptr;
    for (Syncable* pSyncable : m_syncables) {
        if (!pSyncable->isSynchronized()) {
            continue;
        }

        if (pSyncable->isPlaying()) {
            if (!onlyPlaying) {
                onlyPlaying = pSyncable;
            } else {
                return nullptr;
            }
        }
    }
    return onlyPlaying;
}
