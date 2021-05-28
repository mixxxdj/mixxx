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
          m_pMasterSyncable(nullptr) {
    qRegisterMetaType<SyncMode>("SyncMode");
    m_pInternalClock->setMasterBpm(124.0);
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
    Syncable* oldMaster = m_pMasterSyncable;
    switch (mode) {
    case SYNC_MASTER_EXPLICIT:
    case SYNC_MASTER_SOFT: {
        if (pSyncable->getBaseBpm() > 0) {
            activateMaster(pSyncable, mode);
        } else {
            // Because we don't have a valid bpm, we can't be the master
            // (or else everyone would try to be syncing to zero bpm).
            // Override and make us a follower instead.
            activateFollower(pSyncable);
        }
        break;
    }
    case SYNC_FOLLOWER: {
        // A request for follower mode may be converted into an enabling of soft
        // master mode.
        activateFollower(pSyncable);
        Syncable* newMaster = pickMaster(pSyncable);
        if (newMaster && newMaster != m_pMasterSyncable) {
            // if the master has changed, activate it (this updates m_pMasterSyncable)
            activateMaster(newMaster, SYNC_MASTER_SOFT);
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

    // Second, figure out what Syncable should be used to initialize the master
    // parameters, if any. Usually this is the new master. (Note, that pointer might be null!)
    Syncable* pParamsSyncable = m_pMasterSyncable;
    // But If we are newly soft master, we need to match to some other deck.
    if (pSyncable == m_pMasterSyncable && pSyncable != oldMaster && mode != SYNC_MASTER_EXPLICIT) {
        pParamsSyncable = findBpmMatchTarget(pSyncable);
        if (!pParamsSyncable) {
            // We weren't able to find anything to match to, so set ourselves as the
            // target.  That way we'll use our own params when we setMasterParams below.
            pParamsSyncable = pSyncable;
        }
    }
    // Now that all of the decks have their assignments, reinit master params if needed.
    if (pParamsSyncable) {
        setMasterParams(pParamsSyncable);
        pSyncable->setInstantaneousBpm(pParamsSyncable->getBpm());
        if (pParamsSyncable != pSyncable) {
            pSyncable->requestSync();
        }
    }

    if (auto onlyPlayer = getOnlyPlayingSyncable(); onlyPlayer != nullptr) {
        onlyPlayer->notifyOnlyPlayingSyncable();
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

    if (m_pMasterSyncable == pSyncable) {
        m_pMasterSyncable = nullptr;
    }

    pSyncable->setSyncMode(SYNC_FOLLOWER);
}

void EngineSync::activateMaster(Syncable* pSyncable, SyncMode masterType) {
    VERIFY_OR_DEBUG_ASSERT(pSyncable) {
        qWarning() << "WARNING: Logic Error: Called activateMaster on a nullptr Syncable.";
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(masterType == SYNC_MASTER_SOFT || masterType == SYNC_MASTER_EXPLICIT) {
        qWarning() << "WARNING: Logic Error: Called activateMaster with non-master mode";
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::activateMaster: "
                        << pSyncable->getGroup() << "type: "
                        << masterType;
    }

    if (m_pMasterSyncable == pSyncable) {
        // Already master, update the master type.
        if (m_pMasterSyncable->getSyncMode() != masterType) {
            m_pMasterSyncable->setSyncMode(masterType);
        }
        // nothing else to do
        return;
    }

    // If a different channel is already master, disable it.
    Syncable* pOldChannelMaster = m_pMasterSyncable;
    m_pMasterSyncable = nullptr;
    if (pOldChannelMaster) {
        pOldChannelMaster->setSyncMode(SYNC_FOLLOWER);
    }

    m_pMasterSyncable = pSyncable;
    pSyncable->setSyncMode(masterType);

    if (m_pMasterSyncable != m_pInternalClock) {
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
    bool wasMaster = isSyncMaster(pSyncable);
    if (wasMaster) {
        m_pMasterSyncable = nullptr;
    }

    // Notifications happen after-the-fact.
    pSyncable->setSyncMode(SYNC_NONE);

    bool bSyncDeckExists = syncDeckExists();
    if (pSyncable != m_pInternalClock && !bSyncDeckExists) {
        // Deactivate the internal clock if there are no more sync decks left.
        m_pMasterSyncable = nullptr;
        m_pInternalClock->setSyncMode(SYNC_NONE);
        return;
    }

    if (wasMaster) {
        Syncable* newMaster = pickMaster(nullptr);
        if (newMaster != nullptr) {
            activateMaster(newMaster, SYNC_MASTER_SOFT);
        }
    }
}

Syncable* EngineSync::pickMaster(Syncable* enabling_syncable) {
    if (m_pMasterSyncable &&
            m_pMasterSyncable->getSyncMode() == SYNC_MASTER_EXPLICIT &&
            m_pMasterSyncable->getBaseBpm() != 0.0) {
        return m_pMasterSyncable;
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
                    PREFER_IMPLICIT_MASTER));
    switch (picker) {
    case PREFER_IMPLICIT_MASTER:
        // Always pick a deck for a new master.
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
        // Skip non-master decks, like preview decks.
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

    // similar to enablesync -- we pick a new master and maybe reinit.
    Syncable* newMaster = pickMaster(pSyncable);

    if (newMaster != nullptr && newMaster != m_pMasterSyncable) {
        activateMaster(newMaster, SYNC_MASTER_SOFT);
        setMasterParams(newMaster);
    } else if (auto onlyPlayer = getOnlyPlayingSyncable(); onlyPlayer != nullptr) {
        // Even if we didn't change master, if there is only one player (us), then we should
        // reinit the beat distance.
        onlyPlayer->notifyOnlyPlayingSyncable();
        setMasterBeatDistance(onlyPlayer, onlyPlayer->getBeatDistance());
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

    if (isSyncMaster(pSyncable)) {
        setMasterBpm(pSyncable, bpm);
    }
}

void EngineSync::notifyRateChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyRateChanged" << pSyncable->getGroup() << bpm;
    }

    setMasterBpm(pSyncable, bpm);
}

void EngineSync::requestBpmUpdate(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::requestBpmUpdate" << pSyncable->getGroup() << bpm;
    }

    double mbaseBpm = 0.0;
    double mbpm = 0.0;
    double beatDistance = 0.0;
    if (m_pMasterSyncable) {
        mbaseBpm = m_pMasterSyncable->getBaseBpm();
        mbpm = m_pMasterSyncable->getBpm();
        beatDistance = m_pMasterSyncable->getBeatDistance();
    }

    if (mbaseBpm != 0.0) {
        // resync to current master
        pSyncable->setMasterParams(beatDistance, mbaseBpm, mbpm);
    } else {
        // There is no other master, adopt this bpm as master
        pSyncable->setMasterParams(0.0, 0.0, bpm);
    }
}

void EngineSync::notifyInstantaneousBpmChanged(Syncable* pSyncable, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyInstantaneousBpmChanged" << pSyncable->getGroup() << bpm;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    // Do not update the master rate slider because instantaneous changes are
    // not user visible.
    setMasterInstantaneousBpm(pSyncable, bpm);
}

void EngineSync::notifyBeatDistanceChanged(Syncable* pSyncable, double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::notifyBeatDistanceChanged"
                        << pSyncable->getGroup() << beatDistance;
    }
    if (pSyncable != m_pInternalClock) {
        return;
    }

    setMasterBeatDistance(pSyncable, beatDistance);
}

Syncable* EngineSync::pickNonSyncSyncTarget(EngineChannel* pDontPick) const {
    // First choice: the sync master, if it's a deck
    if (m_pMasterSyncable &&
            m_pMasterSyncable->getChannel() &&
            m_pMasterSyncable->getChannel() != pDontPick) {
        return m_pMasterSyncable;
    }

    Syncable* pFirstPlayingDeck = nullptr;
    Syncable* pFirstNonplayingDeck = nullptr;
    foreach (Syncable* pSyncable, m_syncables) {
        EngineChannel* pChannel = pSyncable->getChannel();
        // Exclude non-decks
        if (pChannel == nullptr || pChannel == pDontPick) {
            continue;
        }

        // Only consider channels that have a track loaded, are in the master
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

EngineChannel* EngineSync::getMaster() const {
    return m_pMasterSyncable ? m_pMasterSyncable->getChannel() : nullptr;
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

double EngineSync::masterBpm() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBpm();
    }
    return m_pInternalClock->getBpm();
}

double EngineSync::masterBeatDistance() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBeatDistance();
    }
    return m_pInternalClock->getBeatDistance();
}

double EngineSync::masterBaseBpm() const {
    if (m_pMasterSyncable) {
        return m_pMasterSyncable->getBaseBpm();
    }
    return m_pInternalClock->getBaseBpm();
}

void EngineSync::setMasterBpm(Syncable* pSource, double bpm) {
    //qDebug() << "EngineSync::setMasterBpm" << pSource << bpm;
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBpm(bpm);
    }
}

void EngineSync::setMasterInstantaneousBpm(Syncable* pSource, double bpm) {
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setInstantaneousBpm(bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setInstantaneousBpm(bpm);
    }
}

void EngineSync::setMasterBeatDistance(Syncable* pSource, double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "EngineSync::setMasterBeatDistance"
                        << (pSource ? pSource->getGroup() : "null")
                        << beatDistance;
    }
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterBeatDistance(beatDistance);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (pSyncable == pSource ||
                !pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterBeatDistance(beatDistance);
    }
}

void EngineSync::setMasterParams(Syncable* pSource) {
    // Important note! Because of the way sync works, the new master is usually not the same
    // as the Syncable setting the master parameters (here, pSource). Notify the proper Syncable
    // so it can prepare itself.  (This is a hack to undo half/double math so that we initialize
    // based on un-multiplied bpm values).
    pSource->notifyMasterParamSource();

    double beatDistance = pSource->getBeatDistance();
    if (!pSource->isPlaying()) {
        // If the params source is not playing, but other syncables are, then we are a stopped
        // explicit Master and we should not initialize the beat distance.  Take it from the
        // internal clock instead.
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
        kLogger.trace() << "BaseSyncableListener::setMasterParams, source is"
                        << pSource->getGroup() << beatDistance << baseBpm << bpm;
    }
    if (pSource != m_pInternalClock) {
        m_pInternalClock->setMasterParams(beatDistance, baseBpm, bpm);
    }
    foreach (Syncable* pSyncable, m_syncables) {
        if (!pSyncable->isSynchronized()) {
            continue;
        }
        pSyncable->setMasterParams(beatDistance, baseBpm, bpm);
    }
}

Syncable* EngineSync::getOnlyPlayingSyncable() const {
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
