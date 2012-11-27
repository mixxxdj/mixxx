// playermanager.cpp
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)

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

PlayerManager::PlayerManager(ConfigObject<ConfigValue> *pConfig,
                             SoundManager* pSoundManager,
                             EngineMaster* pEngine,
                             Library* pLibrary,
                             VinylControlManager* pVCManager)
        : m_pConfig(pConfig),
          m_pSoundManager(pSoundManager),
          m_pEngine(pEngine),
          m_pLibrary(pLibrary),
          m_pVCManager(pVCManager),
          // NOTE(XXX) LegacySkinParser relies on these controls being COs and
          // not COTMs listening to a CO.
          m_pCONumDecks(new ControlObject(ConfigKey("[Master]", "num_decks"))),
          m_pCONumSamplers(new ControlObject(ConfigKey("[Master]", "num_samplers"))),
          m_pCONumPreviewDecks(new ControlObject(ConfigKey("[Master]", "num_preview_decks"))) {


    connect(m_pCONumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksControlChanged(double)));
    connect(m_pCONumSamplers, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumSamplersControlChanged(double)));
    connect(m_pCONumPreviewDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumPreviewDecksControlChanged(double)));

    m_pAnalyserQueue = AnalyserQueue::createDefaultAnalyserQueue(m_pConfig);

    // This is parented to the PlayerManager so does not need to be deleted
    SamplerBank* pSamplerBank = new SamplerBank(this);
    Q_UNUSED(pSamplerBank);

    connect(m_pLibrary, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString)));
    connect(m_pLibrary, SIGNAL(loadTrack(TrackPointer)),
             this, SLOT(slotLoadTrackIntoNextAvailableDeck(TrackPointer)));

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
    // No need to delete anything because they are all parented to us and will
    // be destroyed when we are destroyed.
    m_players.clear();
    m_decks.clear();
    m_samplers.clear();

    delete m_pCONumSamplers;
    delete m_pCONumDecks;
    delete m_pCONumPreviewDecks;
    delete m_pAnalyserQueue;
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

void PlayerManager::slotNumDecksControlChanged(double v) {
    // First off, undo any changes to the control.
    m_pCONumDecks->set(m_decks.size());

    int num = v;
    if (num < m_decks.size()) {
        qDebug() << "Ignoring request to reduce the number of decks to" << num;
        return;
    }

    while (m_decks.size() < num) {
        addDeck();
    }
}

void PlayerManager::slotNumSamplersControlChanged(double v) {
    // First off, undo any changes to the control.
    m_pCONumSamplers->set(m_samplers.size());

    int num = v;
    if (num < m_samplers.size()) {
        qDebug() << "Ignoring request to reduce the number of samplers to" << num;
        return;
    }

    while (m_samplers.size() < num) {
        addSampler();
    }
}

void PlayerManager::slotNumPreviewDecksControlChanged(double v) {
    // First off, undo any changes to the control.
    m_pCONumPreviewDecks->set(m_preview_decks.size());

    int num = v;
    if (num < m_preview_decks.size()) {
        qDebug() << "Ignoring request to reduce the number of preview decks to" << num;
        return;
    }

    while (m_preview_decks.size() < num) {
        addPreviewDeck();
    }
}

Deck* PlayerManager::addDeck() {
    QString group = groupForDeck(numDecks());
    int number = numDecks() + 1;

    EngineChannel::ChannelOrientation orientation = EngineChannel::LEFT;
    if (number % 2 == 0)
        orientation = EngineChannel::RIGHT;

    Deck* pDeck = new Deck(this, m_pConfig, m_pEngine, orientation, m_pAnalyserQueue, group);

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
            m_pAnalyserQueue, SLOT(queueAnalyseTrack(TrackPointer)));

    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pDeck;
    m_decks.append(pDeck);
    m_pCONumDecks->add(1);

    // Register the deck output with SoundManager (deck is 0-indexed to SoundManager)
    m_pSoundManager->registerOutput(
        AudioOutput(AudioOutput::DECK, 0, number-1), m_pEngine);

    // If vinyl control manager exists, then register a VC input with
    // SoundManager.
    if (m_pVCManager) {
        m_pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, number-1), m_pVCManager);
    }

    return pDeck;
}

Sampler* PlayerManager::addSampler() {
    QString group = groupForSampler(numSamplers());

    // All samplers are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    Sampler* pSampler = new Sampler(this, m_pConfig, m_pEngine, orientation, group);

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
            m_pAnalyserQueue, SLOT(queueAnalyseTrack(TrackPointer)));

    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pSampler;
    m_samplers.append(pSampler);
    m_pCONumSamplers->add(1);

    return pSampler;
}

PreviewDeck* PlayerManager::addPreviewDeck() {
    QString group = groupForPreviewDeck(numPreviewDecks());

    // All preview decks are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    PreviewDeck* pPreviewDeck = new PreviewDeck(this, m_pConfig, m_pEngine, orientation, group);

    // Connect the player to the analyser queue so that loaded tracks are
    // analysed.
    connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
            m_pAnalyserQueue, SLOT(queueAnalyseTrack(TrackPointer)));
    Q_ASSERT(!m_players.contains(group));
    m_players[group] = pPreviewDeck;
    m_preview_decks.append(pPreviewDeck);
    m_pCONumPreviewDecks->add(1);
    return pPreviewDeck;
}

BaseTrackPlayer* PlayerManager::getPlayer(QString group) const {
    if (m_players.contains(group)) {
        return m_players[group];
    }
    return NULL;
}


Deck* PlayerManager::getDeck(unsigned int deck) const {
    if (deck < 1 || deck > numDecks()) {
        qWarning() << "Warning PlayerManager::getDeck() called with invalid index: "
                   << deck;
        return NULL;
    }
    return m_decks[deck - 1];
}

PreviewDeck* PlayerManager::getPreviewDeck(unsigned int libPreviewPlayer) const {
    if (libPreviewPlayer < 1 || libPreviewPlayer > numPreviewDecks()) {
        qWarning() << "Warning PlayerManager::getPreviewDeck() called with invalid index: "
                   << libPreviewPlayer;
        return NULL;
    }
    return m_preview_decks[libPreviewPlayer - 1];
}

Sampler* PlayerManager::getSampler(unsigned int sampler) const {
    if (sampler < 1 || sampler > numSamplers()) {
        qWarning() << "Warning PlayerManager::getSampler() called with invalid index: "
                   << sampler;
        return NULL;
    }
    return m_samplers[sampler - 1];
}

void PlayerManager::slotLoadTrackToPlayer(TrackPointer pTrack, QString group) {
    BaseTrackPlayer* pPlayer = getPlayer(group);

    if (pPlayer == NULL) {
        qWarning() << "Invalid group argument " << group << " to slotLoadTrackToPlayer.";
        return;
    }

    pPlayer->slotLoadTrack(pTrack);
}

void PlayerManager::slotLoadToPlayer(QString location, QString group) {
    BaseTrackPlayer* pPlayer = getPlayer(group);

    if (pPlayer == NULL) {
        qWarning() << "Invalid group argument " << group << " to slotLoadToPlayer.";
        return;
    }

    TrackPointer pTrack = lookupTrack(location);

    //Load the track into the Player.
    pPlayer->slotLoadTrack(pTrack);
}

void PlayerManager::slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack)
{
    QList<Deck*>::iterator it = m_decks.begin();
    while (it != m_decks.end()) {
        Deck* pDeck = *it;
        ControlObject* playControl =
                ControlObject::getControl(ConfigKey(pDeck->getGroup(), "play"));
        if (playControl && playControl->get() != 1.) {
            pDeck->slotLoadTrack(pTrack, false);
            break;
        }
        it++;
    }
}

void PlayerManager::slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack)
{
    QList<Sampler*>::iterator it = m_samplers.begin();
    while (it != m_samplers.end()) {
        Sampler* pSampler = *it;
        ControlObject* playControl =
                ControlObject::getControl(ConfigKey(pSampler->getGroup(), "play"));
        if (playControl && playControl->get() != 1.) {
            pSampler->slotLoadTrack(pTrack, false);
            break;
        }
        it++;
    }
}

TrackPointer PlayerManager::lookupTrack(QString location) {
    // Try to get TrackPointer from library, identified by location.
    TrackDAO& trackDao = m_pLibrary->getTrackCollection()->getTrackDAO();
    TrackPointer pTrack = trackDao.getTrack(trackDao.getTrackId(location));
    // If not, create a new TrackPointer
    if (pTrack == NULL)
    {
        pTrack = TrackPointer(new TrackInfoObject(location));
    }
    return pTrack;
}

void PlayerManager::slotLoadToDeck(QString location, int deck) {
    Deck* pDeck = getDeck(deck);

    if (pDeck == NULL) {
        qWarning() << "Invalid deck argument " << deck << " to slotLoadToDeck.";
        return;
    }

    TrackPointer pTrack = lookupTrack(location);

    //Load the track into the Deck.
    pDeck->slotLoadTrack(pTrack);
}

void PlayerManager::slotLoadToPreviewDeck(QString location, int libPreviewPlayer) {
    PreviewDeck* pPreviewDeck = getPreviewDeck(libPreviewPlayer);
    if (pPreviewDeck == NULL) {
        qWarning() << "Invalid PreviewDeck argument " << libPreviewPlayer << " to slotLoadToPreviewDeck.";
        return;
    }

    TrackPointer pTrack = lookupTrack(location);

    //Load the track into the Deck.
    pPreviewDeck->slotLoadTrack(pTrack);
}

void PlayerManager::slotLoadToSampler(QString location, int sampler) {
    Sampler* pSampler = getSampler(sampler);

    if (pSampler == NULL) {
        qWarning() << "Invalid sampler argument " << sampler << " to slotLoadToSampler.";
        return;
    }

    TrackPointer pTrack = lookupTrack(location);

    //Load the track into the Sampler.
    pSampler->slotLoadTrack(pTrack);
}

