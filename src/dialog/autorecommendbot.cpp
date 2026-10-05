#include "dialog/autorecommendbot.h"

#include <algorithm>

#include <moc_autorecommendbot.cpp>

#include "control/controlobject.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/assert.h"

namespace {

// Configuration keys for persisting the bot state and settings.
const ConfigKey kConfigEnabled("[AutoRecommend]", "Enabled");
const ConfigKey kConfigDeckPreloading("[AutoRecommend]", "DeckPreloading");
const ConfigKey kConfigTransitionAware("[AutoRecommend]", "TransitionAware");
const ConfigKey kConfigQueueSize("[AutoRecommend]", "QueueSize");
const ConfigKey kConfigWeightTempo("[AutoRecommend]", "WeightTempo");
const ConfigKey kConfigWeightEnergy("[AutoRecommend]", "WeightEnergy");
const ConfigKey kConfigWeightKey("[AutoRecommend]", "WeightKey");
const ConfigKey kConfigWeightFame("[AutoRecommend]", "WeightFame");

constexpr int kMaxCandidates = 2000;

double loadWeight(
        UserSettingsPointer pConfig,
        const ConfigKey& key,
        double defaultValue) {
    const double value = pConfig->getValue<double>(key, defaultValue);
    return value >= 0.0 ? value : defaultValue;
}

bool isAutoDjEnabled() {
    PollingControlProxy autoDjEnabled(
            "[AutoDJ]", "enabled", ControlFlag::AllowMissingOrInvalid);
    return autoDjEnabled.valid() && autoDjEnabled.toBool();
}

} // anonymous namespace

AutoRecommendBot::AutoRecommendBot(
        TrackCollectionManager* pTrackCollectionManager,
        PlayerManagerInterface* pPlayerManager,
        UserSettingsPointer pConfig,
        QObject* pParent)
        : QObject(pParent),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_pPlayerManager(pPlayerManager),
          m_pConfig(pConfig),
          m_enabled(m_pConfig->getValue<bool>(kConfigEnabled, false)),
          m_deckPreloadingEnabled(
                  m_pConfig->getValue<bool>(kConfigDeckPreloading, false)),
          m_transitionAware(
                  m_pConfig->getValue<bool>(kConfigTransitionAware, true)),
          m_weights(),
          m_queueSize(std::clamp(
                  m_pConfig->getValue<int>(kConfigQueueSize, kDefaultQueueSize),
                  1,
                  kMaxQueueSize)) {
    m_weights.tempo = loadWeight(m_pConfig, kConfigWeightTempo, 1.0);
    m_weights.energy = loadWeight(m_pConfig, kConfigWeightEnergy, 1.0);
    m_weights.key = loadWeight(m_pConfig, kConfigWeightKey, 1.0);
    m_weights.fame = loadWeight(m_pConfig, kConfigWeightFame, 0.5);

    DEBUG_ASSERT(m_pPlayerManager);
    if (m_pPlayerManager) {
        // Rebind the deck play watchers whenever the number of decks
        // changes, so newly added decks are watched and preloaded
        // too. The signal is emitted after the new decks exist, so
        // their play controls are available for binding.
        connect(m_pPlayerManager,
                &PlayerManagerInterface::numberOfDecksChanged,
                this,
                &AutoRecommendBot::slotNumDecksChanged);
    }
}

mixxx::AutoRecommendationWeights AutoRecommendBot::weights() const {
    return m_weights;
}

int AutoRecommendBot::queueSize() const {
    return m_queueSize;
}

void AutoRecommendBot::setEnabled(bool enabled) {
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;
    m_pConfig->setValue(kConfigEnabled, m_enabled);
    emit enabledChanged(m_enabled);
    DEBUG_ASSERT(m_pTrackCollectionManager);
    if (m_enabled && m_pTrackCollectionManager) {
        // Start with the track that is currently playing.
        slotCurrentPlayingTrackChanged(
                PlayerInfo::instance().getCurrentPlayingTrack());
    }
}

void AutoRecommendBot::setDeckPreloadingEnabled(bool enabled) {
    if (m_deckPreloadingEnabled == enabled) {
        return;
    }
    m_deckPreloadingEnabled = enabled;
    m_pConfig->setValue(kConfigDeckPreloading, m_deckPreloadingEnabled);
    emit deckPreloadingEnabledChanged(m_deckPreloadingEnabled);
}

void AutoRecommendBot::setTransitionAware(bool transitionAware) {
    if (m_transitionAware == transitionAware) {
        return;
    }
    m_transitionAware = transitionAware;
    m_pConfig->setValue(kConfigTransitionAware, m_transitionAware);
    emit transitionAwareChanged(m_transitionAware);
}

void AutoRecommendBot::setWeights(
        const mixxx::AutoRecommendationWeights& weights) {
    if (m_weights.tempo == weights.tempo && m_weights.energy == weights.energy &&
            m_weights.key == weights.key && m_weights.fame == weights.fame) {
        return;
    }
    m_weights = weights;
    m_pConfig->setValue(kConfigWeightTempo, weights.tempo);
    m_pConfig->setValue(kConfigWeightEnergy, weights.energy);
    m_pConfig->setValue(kConfigWeightKey, weights.key);
    m_pConfig->setValue(kConfigWeightFame, weights.fame);
    emit weightsChanged(m_weights);
}

void AutoRecommendBot::setQueueSize(int queueSize) {
    queueSize = std::clamp(queueSize, 1, kMaxQueueSize);
    if (m_queueSize == queueSize) {
        return;
    }
    m_queueSize = queueSize;
    m_pConfig->setValue(kConfigQueueSize, m_queueSize);
    emit queueSizeChanged(m_queueSize);
}

void AutoRecommendBot::bindDeckPlayControls(int numDecks) {
    numDecks = std::max(numDecks, 0);
    // Replace the watchers of the previous deck count. They are
    // owned by the Qt object tree; deleting a proxy also drops its
    // value-changed connection. This is never called from within a
    // play-control callback, so immediate deletion is safe.
    for (ControlProxy* pPlayProxy : m_deckPlayProxies) {
        delete pPlayProxy;
    }
    m_deckPlayProxies.clear();

    m_deckWasPlaying.clear();
    m_deckWasPlaying.reserve(numDecks);
    for (int deckIndex = 0; deckIndex < numDecks; ++deckIndex) {
        const QString group = PlayerManager::groupForDeck(deckIndex);
        // The proxies are parented to this bot and cleaned up by the
        // Qt object tree; the raw pointers are only tracked to allow
        // rebinding on deck-count changes.
        auto pPlayProxy = make_parented<ControlProxy>(group, "play", this);
        // Capture the deck index, because the valueChanged signal does
        // not carry the group of the changed control.
        pPlayProxy->connectValueChanged(
                this,
                [this, deckIndex](double value) {
                    slotDeckPlayChanged(deckIndex, value);
                });
        m_deckWasPlaying.append(pPlayProxy->toBool());
        m_deckPlayProxies.append(pPlayProxy.get());
    }
}

void AutoRecommendBot::slotNumDecksChanged(int numDecks) {
    if (numDecks < 0 || numDecks == m_deckWasPlaying.size()) {
        // Nothing changed: keep the current watchers.
        return;
    }
    bindDeckPlayControls(numDecks);
}

QSet<TrackId> AutoRecommendBot::queuedTrackIds() const {
    DEBUG_ASSERT(m_pTrackCollectionManager);
    if (!m_pTrackCollectionManager) {
        return {};
    }
    const QList<TrackId> queuedTrackIds = m_pTrackCollectionManager
                                                  ->internalCollection()
                                                  ->getPlaylistDAO()
                                                  .getAutoDJTrackIds();
    return QSet<TrackId>(queuedTrackIds.cbegin(), queuedTrackIds.cend());
}

QSet<TrackId> AutoRecommendBot::loadedTrackIds() const {
    QSet<TrackId> loadedTrackIds;
    const QMap<QString, TrackPointer> loadedTracks =
            PlayerInfo::instance().getLoadedTracks();
    loadedTrackIds.reserve(loadedTracks.size());
    for (auto it = loadedTracks.cbegin(); it != loadedTracks.cend(); ++it) {
        if (it.value()) {
            const TrackId trackId = it.value()->getId();
            if (trackId.isValid()) {
                loadedTrackIds.insert(trackId);
            }
        }
    }
    return loadedTrackIds;
}

TrackPointer AutoRecommendBot::bestRecommendation(
        const TrackPointer& pReferenceTrack) {
    DEBUG_ASSERT(m_pTrackCollectionManager);
    if (!m_pTrackCollectionManager) {
        return {};
    }

    TrackCollection* pCollection = m_pTrackCollectionManager->internalCollection();
    const QSet<QString> locations = pCollection->getTrackDAO().getAllTrackLocations();

    const QSet<TrackId> excludeTrackIds = queuedTrackIds() + loadedTrackIds();

    QList<TrackPointer> candidates;
    candidates.reserve(locations.size());
    for (const QString& location : locations) {
        if (candidates.size() >= kMaxCandidates) {
            break;
        }
        const TrackPointer pTrack = m_pTrackCollectionManager->getTrackByRef(
                TrackRef::fromFilePath(location));
        if (pTrack) {
            candidates.append(pTrack);
        }
    }

    const QList<TrackId> selectedTrackIds = mixxx::autoRecommendationSelect(
            candidates,
            pReferenceTrack,
            excludeTrackIds,
            m_weights,
            1,
            m_transitionAware);
    if (selectedTrackIds.isEmpty()) {
        return {};
    }
    return m_pTrackCollectionManager->getTrackById(selectedTrackIds.first());
}

int AutoRecommendBot::topUpQueue(const TrackPointer& pReferenceTrack) {
    if (!pReferenceTrack) {
        return 0;
    }
    DEBUG_ASSERT(m_pTrackCollectionManager);
    if (!m_pTrackCollectionManager) {
        return 0;
    }

    TrackCollection* pCollection = m_pTrackCollectionManager->internalCollection();
    const QSet<QString> locations = pCollection->getTrackDAO().getAllTrackLocations();

    QList<TrackPointer> candidates;
    candidates.reserve(locations.size());
    for (const QString& location : locations) {
        const TrackPointer pTrack = m_pTrackCollectionManager->getTrackByRef(
                TrackRef::fromFilePath(location));
        if (pTrack && candidates.size() < kMaxCandidates) {
            candidates.append(pTrack);
        }
    }

    const QList<TrackId> selectedTrackIds = mixxx::autoRecommendationSelect(
            candidates,
            pReferenceTrack,
            queuedTrackIds(),
            m_weights,
            m_queueSize,
            m_transitionAware);

    int appended = 0;
    if (!selectedTrackIds.isEmpty()) {
        PlaylistDAO& playlistDao = pCollection->getPlaylistDAO();
        playlistDao.addTracksToAutoDJQueue(
                selectedTrackIds, PlaylistDAO::AutoDJSendLoc::BOTTOM);
        appended = selectedTrackIds.size();
    }

    if (appended > 0) {
        emit recommended(appended);
    }
    return appended;
}

void AutoRecommendBot::slotCurrentPlayingTrackChanged(TrackPointer pTrack) {
    if (!m_enabled || !pTrack) {
        return;
    }
    topUpQueue(pTrack);
}

void AutoRecommendBot::slotDeckPlayChanged(int deckIndex, double value) {
    if (!m_enabled || !m_deckPreloadingEnabled) {
        return;
    }
    if (deckIndex < 0 || deckIndex >= m_deckWasPlaying.size()) {
        return;
    }

    const bool isPlaying = value > 0.0;
    const bool wasPlaying = m_deckWasPlaying.at(deckIndex);
    m_deckWasPlaying[deckIndex] = isPlaying;

    // React only to playing -> stopped transitions. Never preload
    // while AutoDJ is enabled, so both features never fight over the
    // decks.
    if (!wasPlaying || isPlaying || isAutoDjEnabled()) {
        return;
    }

    DEBUG_ASSERT(m_pPlayerManager);
    if (!m_pPlayerManager) {
        return;
    }
    BaseTrackPlayer* pStoppedDeck = m_pPlayerManager->getDeckBase(deckIndex);
    if (!pStoppedDeck) {
        return;
    }
    preloadIntoIdleDeck(pStoppedDeck->getLoadedTrack(), deckIndex);
}

void AutoRecommendBot::preloadIntoIdleDeck(
        const TrackPointer& pReferenceTrack,
        int fallbackDeckIndex) {
    if (!pReferenceTrack) {
        return;
    }
    DEBUG_ASSERT(m_pPlayerManager);
    if (!m_pPlayerManager) {
        return;
    }

    // Prefer the first empty, stopped deck; fall back to the deck
    // that just stopped. Never touch decks that are playing or hold
    // manually loaded tracks.
    BaseTrackPlayer* pTargetDeck = nullptr;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        BaseTrackPlayer* pDeck = m_pPlayerManager->getDeckBase(i);
        if (!pDeck || pDeck->getLoadedTrack()) {
            continue;
        }
        ControlObject* pPlayControl = ControlObject::getControl(
                ConfigKey(pDeck->getGroup(), "play"));
        if (pPlayControl && !pPlayControl->toBool()) {
            pTargetDeck = pDeck;
            break;
        }
    }
    if (!pTargetDeck && fallbackDeckIndex >= 0) {
        // Fall back to the deck that just stopped: its track was
        // played, so replacing it is the natural "next track" flow.
        pTargetDeck = m_pPlayerManager->getDeckBase(fallbackDeckIndex);
    }
    if (!pTargetDeck) {
        return;
    }

    const TrackPointer pTrack = bestRecommendation(pReferenceTrack);
    if (!pTrack) {
        return;
    }
#ifdef __STEM__
    pTargetDeck->slotLoadTrack(pTrack, mixxx::StemChannelSelection(), false);
#else
    pTargetDeck->slotLoadTrack(pTrack, false);
#endif
    emit trackPreloaded(pTargetDeck->getGroup(), pTrack->getTitle());
}
