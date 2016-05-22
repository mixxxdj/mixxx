// playermanager.cpp
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)
#include "mixer/playermanager.h"

#include <QMutexLocker>

#include "analyzer/analyzerqueue.h"
#include "control/controlobject.h"
#include "control/controlobject.h"
#include "effects/effectsmanager.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "mixer/auxiliary.h"
#include "mixer/deck.h"
#include "mixer/microphone.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "mixer/samplerbank.h"
#include "soundio/soundmanager.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/stat.h"
#include "util/sleepableqthread.h"

PlayerManager::PlayerManager(UserSettingsPointer pConfig,
                             SoundManager* pSoundManager,
                             EffectsManager* pEffectsManager,
                             EngineMaster* pEngine) :
        m_mutex(QMutex::Recursive),
        m_pConfig(pConfig),
        m_pSoundManager(pSoundManager),
        m_pEffectsManager(pEffectsManager),
        m_pEngine(pEngine),
        // NOTE(XXX) LegacySkinParser relies on these controls being Controls
        // and not ControlProxies.
        m_pAnalyzerQueue(NULL),
        m_pCONumDecks(new ControlObject(
            ConfigKey("[Master]", "num_decks"), true, true)),
        m_pCONumSamplers(new ControlObject(
            ConfigKey("[Master]", "num_samplers"), true, true)),
        m_pCONumPreviewDecks(new ControlObject(
            ConfigKey("[Master]", "num_preview_decks"), true, true)),
        m_pCONumMicrophones(new ControlObject(
            ConfigKey("[Master]", "num_microphones"), true, true)),
        m_pCONumAuxiliaries(new ControlObject(
            ConfigKey("[Master]", "num_auxiliaries"), true, true)){
    connect(m_pCONumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumDecks, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotNumDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumSamplers, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumSamplersControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumSamplers, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotNumSamplersControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumPreviewDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumPreviewDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumPreviewDecks, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotNumPreviewDecksControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumMicrophones, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumMicrophonesControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumMicrophones, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotNumMicrophonesControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumAuxiliaries, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumAuxiliariesControlChanged(double)),
            Qt::DirectConnection);
    connect(m_pCONumAuxiliaries, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotNumAuxiliariesControlChanged(double)),
            Qt::DirectConnection);

    // This is parented to the PlayerManager so does not need to be deleted
    SamplerBank* pSamplerBank = new SamplerBank(this);
    Q_UNUSED(pSamplerBank);

    // register the engine's outputs
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::MASTER, 0, 2),
                                    m_pEngine);
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::HEADPHONES, 0, 2),
                                    m_pEngine);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        m_pSoundManager->registerOutput(AudioOutput(AudioOutput::BUS, 0, 2, o),
                                        m_pEngine);
    }
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::SIDECHAIN, 0, 2),
                                    m_pEngine);
}

PlayerManager::~PlayerManager() {
    QMutexLocker locker(&m_mutex);
    // No need to delete anything because they are all parented to us and will
    // be destroyed when we are destroyed.
    m_players.clear();
    m_decks.clear();
    m_samplers.clear();
    m_microphones.clear();
    m_auxiliaries.clear();

    delete m_pCONumSamplers;
    delete m_pCONumDecks;
    delete m_pCONumPreviewDecks;
    delete m_pCONumMicrophones;
    delete m_pCONumAuxiliaries;
    if (m_pAnalyzerQueue) {
        delete m_pAnalyzerQueue;
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

    m_pAnalyzerQueue = AnalyzerQueue::createDefaultAnalyzerQueue(m_pConfig,
            pLibrary->getTrackCollection());

    // Connect the player to the analyzer queue so that loaded tracks are
    // analysed.
    foreach(Deck* pDeck, m_decks) {
        connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analysed.
    foreach(Sampler* pSampler, m_samplers) {
        connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analysed.
    foreach(PreviewDeck* pPreviewDeck, m_preview_decks) {
        connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }
}

// static
unsigned int PlayerManager::numDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    static ControlProxy* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = new ControlProxy(ConfigKey("[Master]", "num_decks"));
        if (!pNumCO->valid()) {
            delete pNumCO;
            pNumCO = NULL;
        }
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
bool PlayerManager::isPreviewDeckGroup(const QString& group, int* number) {
    if (!group.startsWith("[PreviewDeck")) {
        return false;
    }

    bool ok = false;
    int deckNum = group.mid(12,group.lastIndexOf("]")-12).toInt(&ok);
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
    static ControlProxy* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = new ControlProxy(ConfigKey("[Master]", "num_samplers"));
        if (!pNumCO->valid()) {
            delete pNumCO;
            pNumCO = NULL;
        }
    }
    return pNumCO ? pNumCO->get() : 0;
}

// static
unsigned int PlayerManager::numPreviewDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    static ControlProxy* pNumCO = NULL;
    if (pNumCO == NULL) {
        pNumCO = new ControlProxy(
                ConfigKey("[Master]", "num_preview_decks"));
        if (!pNumCO->valid()) {
            delete pNumCO;
            pNumCO = NULL;
        }
    }
    return pNumCO ? pNumCO->get() : 0;
}

void PlayerManager::slotNumDecksControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;

    // Update the soundmanager config even if the number of decks has been
    // reduced.
    m_pSoundManager->setConfiguredDeckCount(num);

    if (num < m_decks.size()) {
        // The request was invalid -- reset the value.
        m_pCONumDecks->set(m_decks.size());
        qDebug() << "Ignoring request to reduce the number of decks to" << num;
        return;
    }

    while (m_decks.size() < num) {
        addDeckInner();
    }
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

void PlayerManager::slotNumMicrophonesControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_microphones.size()) {
        // The request was invalid -- reset the value.
        m_pCONumMicrophones->set(m_microphones.size());
        qDebug() << "Ignoring request to reduce the number of microphones to" << num;
        return;
    }

    while (m_microphones.size() < num) {
        addMicrophoneInner();
    }
}

void PlayerManager::slotNumAuxiliariesControlChanged(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_auxiliaries.size()) {
        // The request was invalid -- reset the value.
        m_pCONumAuxiliaries->set(m_auxiliaries.size());
        qDebug() << "Ignoring request to reduce the number of auxiliaries to" << num;
        return;
    }

    while (m_auxiliaries.size() < num) {
        addAuxiliaryInner();
    }
}

void PlayerManager::addDeck() {
    QMutexLocker locker(&m_mutex);
    addDeckInner();
    m_pCONumDecks->set((double)m_decks.count());
    emit(numberOfDecksChanged(m_decks.count()));
}

void PlayerManager::addConfiguredDecks() {
    // Cache this value in case it changes out from under us.
    int deck_count = m_pSoundManager->getConfiguredDeckCount();
    for (int i = 0; i < deck_count; ++i) {
        addDeck();
    }
}

void PlayerManager::addDeckInner() {
    // Do not lock m_mutex here.
    QString group = groupForDeck(m_decks.count());
    DEBUG_ASSERT_AND_HANDLE(!m_players.contains(group)) {
        return;
    }

    int number = m_decks.count() + 1;

    EngineChannel::ChannelOrientation orientation = EngineChannel::LEFT;
    if (number % 2 == 0) {
        orientation = EngineChannel::RIGHT;
    }

    Deck* pDeck = new Deck(this, m_pConfig, m_pEngine, m_pEffectsManager,
                           orientation, group);
    connect(pDeck, SIGNAL(noPassthroughInputConfigured()),
            this, SIGNAL(noDeckPassthroughInputConfigured()));
    connect(pDeck, SIGNAL(noVinylControlInputConfigured()),
            this, SIGNAL(noVinylControlInputConfigured()));

    if (m_pAnalyzerQueue) {
        connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    m_players[group] = pDeck;
    m_decks.append(pDeck);

    // Register the deck output with SoundManager (deck is 0-indexed to SoundManager)
    m_pSoundManager->registerOutput(
            AudioOutput(AudioOutput::DECK, 0, 2, number - 1), m_pEngine);

    // Register vinyl input signal with deck for passthrough support.
    EngineDeck* pEngineDeck = pDeck->getEngineDeck();
    m_pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, 2, number - 1), pEngineDeck);

    // Setup equalizer rack for this deck.
    EqualizerRackPointer pEqRack = m_pEffectsManager->getEqualizerRack(0);
    if (pEqRack) {
        pEqRack->addEffectChainSlotForGroup(group);
    }

    // BaseTrackPlayer needs to delay until we have setup the equalizer rack for
    // this deck to fetch the legacy EQ controls.
    // TODO(rryan): Find a way to remove this cruft.
    pDeck->setupEqControls();

    // Setup quick effect rack for this deck.
    QuickEffectRackPointer pQuickEffectRack = m_pEffectsManager->getQuickEffectRack(0);
    if (pQuickEffectRack) {
        pQuickEffectRack->addEffectChainSlotForGroup(group);
    }
}

void PlayerManager::addSampler() {
    QMutexLocker locker(&m_mutex);
    addSamplerInner();
    m_pCONumSamplers->set(m_samplers.count());
}

void PlayerManager::addSamplerInner() {
    // Do not lock m_mutex here.
    QString group = groupForSampler(m_samplers.count());

    DEBUG_ASSERT_AND_HANDLE(!m_players.contains(group)) {
        return;
    }

    // All samplers are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    Sampler* pSampler = new Sampler(this, m_pConfig, m_pEngine,
                                    m_pEffectsManager, orientation, group);
    if (m_pAnalyzerQueue) {
        connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

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
    DEBUG_ASSERT_AND_HANDLE(!m_players.contains(group)) {
        return;
    }

    // All preview decks are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    PreviewDeck* pPreviewDeck = new PreviewDeck(this, m_pConfig, m_pEngine,
                                                m_pEffectsManager, orientation,
                                                group);
    if (m_pAnalyzerQueue) {
        connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                m_pAnalyzerQueue, SLOT(slotAnalyseTrack(TrackPointer)));
    }

    m_players[group] = pPreviewDeck;
    m_preview_decks.append(pPreviewDeck);
}

void PlayerManager::addMicrophone() {
    QMutexLocker locker(&m_mutex);
    addMicrophoneInner();
    m_pCONumMicrophones->set(m_microphones.count());
}

void PlayerManager::addMicrophoneInner() {
    // Do not lock m_mutex here.
    int index = m_microphones.count();
    QString group = groupForMicrophone(index);
    Microphone* pMicrophone = new Microphone(this, group, index, m_pSoundManager,
                                             m_pEngine, m_pEffectsManager);
    connect(pMicrophone, SIGNAL(noMicrophoneInputConfigured()),
            this, SIGNAL(noMicrophoneInputConfigured()));
    m_microphones.append(pMicrophone);
}

void PlayerManager::addAuxiliary() {
    QMutexLocker locker(&m_mutex);
    addAuxiliaryInner();
    m_pCONumAuxiliaries->set(m_auxiliaries.count());
}

void PlayerManager::addAuxiliaryInner() {
    // Do not lock m_mutex here.
    int index = m_auxiliaries.count();
    QString group = groupForAuxiliary(index);

    Auxiliary* pAuxiliary = new Auxiliary(this, group, index, m_pSoundManager,
                                          m_pEngine, m_pEffectsManager);
    m_auxiliaries.append(pAuxiliary);
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

Microphone* PlayerManager::getMicrophone(unsigned int microphone) const {
    QMutexLocker locker(&m_mutex);
    if (microphone < 1 || microphone >= static_cast<unsigned int>(m_microphones.size())) {
        qWarning() << "Warning PlayerManager::getMicrophone() called with invalid index: "
                   << microphone;
        return NULL;
    }
    return m_microphones[microphone - 1];
}

Auxiliary* PlayerManager::getAuxiliary(unsigned int auxiliary) const {
    QMutexLocker locker(&m_mutex);
    if (auxiliary < 1 || auxiliary > static_cast<unsigned int>(m_auxiliaries.size())) {
        qWarning() << "Warning PlayerManager::getAuxiliary() called with invalid index: "
                   << auxiliary;
        return NULL;
    }
    return m_auxiliaries[auxiliary - 1];
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

void PlayerManager::slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack) {
    QMutexLocker locker(&m_mutex);
    QList<Deck*>::iterator it = m_decks.begin();
    while (it != m_decks.end()) {
        Deck* pDeck = *it;
        ControlObject* playControl =
                ControlObject::getControl(ConfigKey(pDeck->getGroup(), "play"));
        if (playControl && playControl->get() != 1.) {
            locker.unlock();
            pDeck->slotLoadTrack(pTrack, false);
            // Test for a fixed race condition with fast loads
            //SleepableQThread::sleep(1);
            //pDeck->slotLoadTrack(TrackPointer(), false);
            return;
        }
        ++it;
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
        ++it;
    }
}
