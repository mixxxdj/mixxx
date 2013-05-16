// playermanager.cpp
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)
#include <QMutexLocker>

#include "playermanager.h"

#include "controlobject.h"
#include "trackinfoobject.h"
#include "deck.h"
#include "sampler.h"
#include "previewdeck.h"
#include "analyserqueue.h"
#include "controlobject.h"
#include "samplerbank.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "engine/enginemaster.h"
#include "soundmanager.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "util/stat.h"
#include "engine/enginedeck.h"

PlayerManager::PlayerManager(ConfigObject<ConfigValue>* pConfig,
                             SoundManager* pSoundManager,
                             EngineMaster* pEngine,
                             VinylControlManager* pVCManager) :
        m_mutex(QMutex::Recursive),
        m_pConfig(pConfig),
        m_pSoundManager(pSoundManager),
        m_pEngine(pEngine),
        m_pVCManager(pVCManager),
        // NOTE(XXX) LegacySkinParser relies on these controls being COs and
        // not COTMs listening to a CO.
        m_pAnalyserQueue(NULL),
        m_pCONumDecks(new ControlObject(ConfigKey("[Master]", "num_decks"), true, true)),
        m_pCONumSamplers(new ControlObject(ConfigKey("[Master]", "num_samplers"), true, true)),
        m_pCONumPreviewDecks(new ControlObject(ConfigKey("[Master]", "num_preview_decks"), true, true)) {

    connect(m_pCOSkinNumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotSkinNumDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumSamplers, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumSamplersControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumPreviewDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumPreviewDecksControlChanged(double)),
            Qt::DirectConnection);

    // This is parented to the PlayerManager so does not need to be deleted
    SamplerBank* pSamplerBank = new SamplerBank(this);
    Q_UNUSED(pSamplerBank);

    // Redundant
    m_pCONumDecks->set(0);
    m_pCONumSamplers->set(0);
    m_pCONumPreviewDecks->set(0);

    // register the engine's outputs
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::MASTER),
            m_pEngine);
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::HEADPHONES),
            m_pEngine);
}

PlayerManager::~PlayerManager() {
    QMutexLocker locker(&m_mutex);
    // No need to delete anything because they are all parented to us and will
    // be destroyed when we are destroyed.
    m_players.clear();
    m_decks.clear();
    m_samplers.clear();

    delete m_pCOSkinNumDecks;
    delete m_pCONumSamplers;
    delete m_pCONumDecks;
    delete m_pCONumPreviewDecks;
    if (m_pAnalyserQueue) {
        delete m_pAnalyserQueue;
    }
}

void PlayerManager::bindToLibrary(Library* pLibrary) {
    QMutexLocker locker(&m_mutex);
    connect(pLibrary, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString, bool)));
    connect(pLibrary, SIGNAL(loadTrack(TrackPointer)),
            this, SLOT(slotLoadTrackIntoNextAvailableDeck(TrackPointer)));
    connect(this, SIGNAL(loadLocationToPlayer(QString, QString)),
            pLibrary, SLOT(slotLoadLocationToPlayer(QString, QString)));

    m_pAnalyserQueue = AnalyserQueue::createDefaultAnalyserQueue(m_pConfig,
            pLibrary->getTrackCollection());

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    foreach(Deck* pDeck, m_decks) {
        connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    foreach(Sampler* pSampler, m_samplers) {
        connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    foreach(PreviewDeck* pPreviewDeck, m_preview_decks) {
        connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }
}

// static
unsigned int PlayerManager::numDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    static ControlObject* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = ControlObject::getControl(
            ConfigKey("[Master]", "num_decks"));
    }
    return pNumCO ? pNumCO->get() : 0;
}

// static
bool PlayerManager::isDeckGroup(const QString& group, int* number) {
    if (!group.startsWith("[Channel")) {
        return false;
    }

    bool ok = false;
    int deckNum = group.mid(8,group.lastIndexOf("]")-8).toInt(&ok);
    if (!ok || deckNum <= 0) {
        return false;
    }
    if (number != NULL) {
        *number = deckNum;
    }
    return true;
}

// static
unsigned int PlayerManager::numSamplers() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    static ControlObject* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = ControlObject::getControl(
            ConfigKey("[Master]", "num_samplers"));
    }
    return pNumCO ? pNumCO->get() : 0;
}

// static
unsigned int PlayerManager::numPreviewDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    static ControlObject* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = ControlObject::getControl(
            ConfigKey("[Master]", "num_preview_decks"));
    }
    return pNumCO ? pNumCO->get() : 0;

}

void PlayerManager::slotSkinNumDecksControlChanged(double v) {
    m_skin_decks = static_cast<unsigned int>(v);
    m_pCONumDecks->set(m_skin_decks);
    remapDecks();
}

void PlayerManager::slotNumDecksControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_decks.size()) {
        // The request was invalid -- reset the value.
        m_pCONumDecks->set(m_decks.size());
        qDebug() << "Ignoring request to reduce the number of decks to" << num;
        return;
    }

    while (m_decks.size() < num) {
        addDeckInner(num);
    }
    
    remapDecks();
    
    m_pCONumDecks->set(m_decks.size());
}

void PlayerManager::remapDecks() {
	// Redistribute decks left and right based on new count.  If the number of decks has decreased,
	// Leave the extras where they are.
    for (int i = 1; i <= m_decks.size(); ++i) {
        if (i > m_skin_decks / 2) {
            ControlObject::getControl(ConfigKey(QString("[Channel%1]").arg(i), 
                                                "orientation"))->set(EngineChannel::RIGHT);
        } else {
            ControlObject::getControl(ConfigKey(QString("[Channel%1]").arg(i), 
                                                "orientation"))->set(EngineChannel::LEFT);
        }
    }
    // Make sure the count is up to date now.
}

void PlayerManager::slotNumSamplersControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_samplers.size()) {
        // The request was invalid -- reset the value.
        m_pCONumSamplers->set(m_samplers.size());
        qDebug() << "Ignoring request to reduce the number of samplers to" << num;
        return;
    }

    while (m_samplers.size() < num) {
        addSamplerInner();
    }
}

void PlayerManager::slotNumPreviewDecksControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_preview_decks.size()) {
        // The request was invalid -- reset the value.
        m_pCONumPreviewDecks->set(m_preview_decks.size());
        qDebug() << "Ignoring request to reduce the number of preview decks to" << num;
        return;
    }

    while (m_preview_decks.size() < num) {
        addPreviewDeckInner();
    }
}

void PlayerManager::addDeck(int total_decks) {
    QMutexLocker locker(&m_mutex);
    addDeckInner(total_decks);
    m_pCONumDecks->set((double)m_decks.count());
}

void PlayerManager::addDeckInner(int total_decks) {
    // Do not lock m_mutex here.
    QString group = groupForDeck(m_decks.count());
    int number = m_decks.count() + 1;

    EngineChannel::ChannelOrientation orientation = EngineChannel::LEFT;
    if (number > total_decks / 2) {
        orientation = EngineChannel::RIGHT;
    }

    Deck* pDeck = new Deck(this, m_pConfig, m_pEngine, orientation, group);
    if (m_pAnalyserQueue) {
        connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pDeck;
    m_decks.append(pDeck);

    // Register the deck output with SoundManager (deck is 0-indexed to SoundManager)
    m_pSoundManager->registerOutput(
        AudioOutput(AudioOutput::DECK, 0, number-1), m_pEngine);

    // If vinyl control manager exists, then register a VC input with
    // SoundManager.
    if (m_pVCManager) {
        m_pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, number-1), m_pVCManager);
    }

    // Also register vinyl input signal with deck for passthrough support.
    EngineDeck* pEngineDeck = pDeck->getEngineDeck();
    m_pSoundManager->registerInput(
        AudioInput(AudioInput::VINYLCONTROL, 0, number-1), pEngineDeck);
}

void PlayerManager::addSampler() {
    QMutexLocker locker(&m_mutex);
    addSamplerInner();
    m_pCONumSamplers->set(m_samplers.count());
}

void PlayerManager::addSamplerInner() {
    // Do not lock m_mutex here.
    QString group = groupForSampler(m_samplers.count());

    // All samplers are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    Sampler* pSampler = new Sampler(this, m_pConfig, m_pEngine, orientation, group);
    if (m_pAnalyserQueue) {
        connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pSampler;
    m_samplers.append(pSampler);
}

void PlayerManager::addPreviewDeck() {
    QMutexLocker locker(&m_mutex);
    addPreviewDeckInner();
    m_pCONumPreviewDecks->set(m_preview_decks.count());
}

void PlayerManager::addPreviewDeckInner() {
    // Do not lock m_mutex here.
    QString group = groupForPreviewDeck(m_preview_decks.count());

    // All preview decks are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    PreviewDeck* pPreviewDeck = new PreviewDeck(this, m_pConfig, m_pEngine, orientation, group);
    if (m_pAnalyserQueue) {
        connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyserQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pPreviewDeck;
    m_preview_decks.append(pPreviewDeck);
}

BaseTrackPlayer* PlayerManager::getPlayer(QString group) const {
    QMutexLocker locker(&m_mutex);
    if (m_players.contains(group)) {
        return m_players[group];
    }
    return NULL;
}


Deck* PlayerManager::getDeck(unsigned int deck) const {
    QMutexLocker locker(&m_mutex);
    if (deck < 1 || deck > numDecks()) {
        qWarning() << "Warning PlayerManager::getDeck() called with invalid index: "
                   << deck;
        return NULL;
    }
    return m_decks[deck - 1];
}

PreviewDeck* PlayerManager::getPreviewDeck(unsigned int libPreviewPlayer) const {
    QMutexLocker locker(&m_mutex);
    if (libPreviewPlayer < 1 || libPreviewPlayer > numPreviewDecks()) {
        qWarning() << "Warning PlayerManager::getPreviewDeck() called with invalid index: "
                   << libPreviewPlayer;
        return NULL;
    }
    return m_preview_decks[libPreviewPlayer - 1];
}

Sampler* PlayerManager::getSampler(unsigned int sampler) const {
    QMutexLocker locker(&m_mutex);
    if (sampler < 1 || sampler > numSamplers()) {
        qWarning() << "Warning PlayerManager::getSampler() called with invalid index: "
                   << sampler;
        return NULL;
    }
    return m_samplers[sampler - 1];
}

void PlayerManager::slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play) {
    // Do not lock mutex in this method unless it is changed to access
    // PlayerManager state.
    BaseTrackPlayer* pPlayer = getPlayer(group);

    if (pPlayer == NULL) {
        qWarning() << "Invalid group argument " << group << " to slotLoadTrackToPlayer.";
        return;
    }

    pPlayer->slotLoadTrack(pTrack, play);
}

void PlayerManager::slotLoadToPlayer(QString location, QString group) {
    // The library will get the track and then signal back to us to load the
    // track via slotLoadTrackToPlayer.
    emit(loadLocationToPlayer(location, group));
}

void PlayerManager::slotLoadToDeck(QString location, int deck) {
    slotLoadToPlayer(location, groupForDeck(deck-1));
}

void PlayerManager::slotLoadToPreviewDeck(QString location, int previewDeck) {
    slotLoadToPlayer(location, groupForPreviewDeck(previewDeck-1));
}

void PlayerManager::slotLoadToSampler(QString location, int sampler) {
    slotLoadToPlayer(location, groupForSampler(sampler-1));
}

void PlayerManager::slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack)
{
    int num_decks = m_pCOSkinNumDecks->get();
    if (num_decks == 0) {
        num_decks = m_decks.size();
    }
    // Start with the inner two decks and work our way out.
    int deck_l = (num_decks + 1) / 2 - 1;
    int deck_r = deck_l + 1;
    bool try_l = true;
    // 
    while (deck_l >= 0 || deck_r < num_decks) {
        Deck* pDeck;
        if (try_l && deck_l >= 0) {
            pDeck = m_decks[deck_l];
            --deck_l;
        } else {
            pDeck = m_decks[deck_r];
            ++deck_r;
        }
        try_l = !try_l;
        
        ControlObject* vinylControlEnabled =
                ControlObject::getControl(ConfigKey(pDeck->getGroup(), 
                                          "vinylcontrol_enabled"));
                                          
        if (vinylControlEnabled && vinylControlEnabled->get())
        {
            // With vinyl, we can't rely solely on play-button status.  Load if
            // either no track is loaded, or if the scratch2 rate is very low.
            TrackPointer tp = pDeck->getLoadedTrack();
            ControlObject* vinylRate =
                    ControlObject::getControl(ConfigKey(pDeck->getGroup(), 
                                              "scratch2"));
            if (vinylRate && (tp == NULL || fabs(vinylRate->get()) < 0.1 )) {
                pDeck->slotLoadTrack(pTrack, false);
                break;
            }
        }
        else
        {
            ControlObject* playControl =
                    ControlObject::getControl(ConfigKey(pDeck->getGroup(), "play"));
            if (playControl && playControl->get() != 1. ) {
                pDeck->slotLoadTrack(pTrack, false);
                break;
            }
        }
    }
}

void PlayerManager::slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack) {
    QMutexLocker locker(&m_mutex);
    QList<Sampler*>::iterator it = m_samplers.begin();
    while (it != m_samplers.end()) {
        Sampler* pSampler = *it;
        ControlObject* playControl =
                ControlObject::getControl(ConfigKey(pSampler->getGroup(), "play"));
        if (playControl && playControl->get() != 1.) {
            locker.unlock();
            pSampler->slotLoadTrack(pTrack, false);
            return;
        }
        it++;
    }
}


