#include "mixer/playermanager.h"

#include <QMutexLocker>

#include "control/controlobject.h"
#include "effects/effectrack.h"
#include "effects/effectsmanager.h"
#include "engine/channels/enginedeck.h"
#include "engine/enginemaster.h"
#include "library/library.h"
#include "mixer/auxiliary.h"
#include "mixer/deck.h"
#include "mixer/microphone.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "mixer/samplerbank.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "soundio/soundmanager.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/logger.h"
#include "util/sleepableqthread.h"

namespace {

const mixxx::Logger kLogger("PlayerManager");

// Utilize half of the available cores for adhoc analysis of tracks
const int kNumberOfAnalyzerThreads = math_max(1, QThread::idealThreadCount() / 2);

} // anonymous namespace

//static
QAtomicPointer<ControlProxy> PlayerManager::m_pCOPNumDecks;
//static
QAtomicPointer<ControlProxy> PlayerManager::m_pCOPNumSamplers;
//static
QAtomicPointer<ControlProxy> PlayerManager::m_pCOPNumPreviewDecks;

PlayerManager::PlayerManager(UserSettingsPointer pConfig,
        SoundManager* pSoundManager,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineMaster* pEngine)
        : m_mutex(QMutex::Recursive),
          m_pConfig(pConfig),
          m_pSoundManager(pSoundManager),
          m_pEffectsManager(pEffectsManager),
          m_pVisualsManager(pVisualsManager),
          m_pEngine(pEngine),
          // NOTE(XXX) LegacySkinParser relies on these controls being Controls
          // and not ControlProxies.
          m_pCONumDecks(new ControlObject(
                  ConfigKey("[Master]", "num_decks"), true, true)),
          m_pCONumSamplers(new ControlObject(
                  ConfigKey("[Master]", "num_samplers"), true, true)),
          m_pCONumPreviewDecks(new ControlObject(
                  ConfigKey("[Master]", "num_preview_decks"), true, true)),
          m_pCONumMicrophones(new ControlObject(
                  ConfigKey("[Master]", "num_microphones"), true, true)),
          m_pCONumAuxiliaries(new ControlObject(
                  ConfigKey("[Master]", "num_auxiliaries"), true, true)),
          m_pTrackAnalysisScheduler(TrackAnalysisScheduler::NullPointer()) {
    m_pCONumDecks->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumDecks, Qt::DirectConnection);
    m_pCONumSamplers->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumSamplers, Qt::DirectConnection);
    m_pCONumPreviewDecks->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumPreviewDecks, Qt::DirectConnection);
    m_pCONumMicrophones->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumMicrophones, Qt::DirectConnection);
    m_pCONumAuxiliaries->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumAuxiliaries, Qt::DirectConnection);

    // This is parented to the PlayerManager so does not need to be deleted
    m_pSamplerBank = new SamplerBank(this);

    m_cloneTimer.start();
}

PlayerManager::~PlayerManager() {
    kLogger.debug() << "Destroying";

    QMutexLocker locker(&m_mutex);

    m_pSamplerBank->saveSamplerBankToPath(
        m_pConfig->getSettingsPath() + "/samplers.xml");
    // No need to delete anything because they are all parented to us and will
    // be destroyed when we are destroyed.
    m_players.clear();
    m_decks.clear();
    m_samplers.clear();
    m_microphones.clear();
    m_auxiliaries.clear();

    delete m_pCOPNumDecks.fetchAndStoreAcquire(nullptr);
    delete m_pCOPNumSamplers.fetchAndStoreAcquire(nullptr);
    delete m_pCOPNumPreviewDecks.fetchAndStoreAcquire(nullptr);

    delete m_pCONumSamplers;
    delete m_pCONumDecks;
    delete m_pCONumPreviewDecks;
    delete m_pCONumMicrophones;
    delete m_pCONumAuxiliaries;

    if (m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler->stop();
        m_pTrackAnalysisScheduler.reset();
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

    DEBUG_ASSERT(!m_pTrackAnalysisScheduler);
    m_pTrackAnalysisScheduler = TrackAnalysisScheduler::createInstance(
            pLibrary,
            kNumberOfAnalyzerThreads,
            m_pConfig,
            AnalyzerModeFlags::WithWaveform);

    connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::trackProgress,
            this, &PlayerManager::onTrackAnalysisProgress);
    connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::finished,
            this, &PlayerManager::onTrackAnalysisFinished);

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach(Deck* pDeck, m_decks) {
        connect(pDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotAnalyzeTrack(TrackPointer)));
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach(Sampler* pSampler, m_samplers) {
        connect(pSampler, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotAnalyzeTrack(TrackPointer)));
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach (PreviewDeck* pPreviewDeck, m_previewDecks) {
        connect(pPreviewDeck, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotAnalyzeTrack(TrackPointer)));
    }
}

// static
bool PlayerManager::isDeckGroup(const QString& group, int* number) {
    if (!group.startsWith("[Channel")) {
        return false;
    }

    bool ok = false;
    int deckNum = group.midRef(8,group.lastIndexOf("]")-8).toInt(&ok);
    if (!ok || deckNum <= 0) {
        return false;
    }
    if (number != NULL) {
        *number = deckNum;
    }
    return true;
}

// static
bool PlayerManager::isSamplerGroup(const QString& group, int* number) {
    if (!group.startsWith("[Sampler")) {
        return false;
    }

    bool ok = false;
    int deckNum = group.midRef(8,group.lastIndexOf("]")-8).toInt(&ok);
    if (!ok || deckNum <= 0) {
        return false;
    }
    if (number != nullptr) {
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
    int deckNum = group.midRef(12,group.lastIndexOf("]")-12).toInt(&ok);
    if (!ok || deckNum <= 0) {
        return false;
    }
    if (number != NULL) {
        *number = deckNum;
    }
    return true;
}

// static
unsigned int PlayerManager::numDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    ControlProxy* pCOPNumDecks = atomicLoadRelaxed(m_pCOPNumDecks);
    if (pCOPNumDecks == nullptr) {
        pCOPNumDecks = new ControlProxy(ConfigKey("[Master]", "num_decks"));
        if (!pCOPNumDecks->valid()) {
            delete pCOPNumDecks;
            pCOPNumDecks = nullptr;
        } else {
            m_pCOPNumDecks = pCOPNumDecks;
        }
    }
    // m_pCOPNumDecks->get() fails on MacOs
    return pCOPNumDecks ? static_cast<int>(pCOPNumDecks->get()) : 0;
}

// static
unsigned int PlayerManager::numSamplers() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    ControlProxy* pCOPNumSamplers = atomicLoadRelaxed(m_pCOPNumSamplers);
    if (pCOPNumSamplers == nullptr) {
        pCOPNumSamplers = new ControlProxy(ConfigKey("[Master]", "num_samplers"));
        if (!pCOPNumSamplers->valid()) {
            delete pCOPNumSamplers;
            pCOPNumSamplers = nullptr;
        } else {
            m_pCOPNumSamplers = pCOPNumSamplers;
        }
    }
    // m_pCOPNumSamplers->get() fails on MacOs
    return pCOPNumSamplers ? static_cast<int>(pCOPNumSamplers->get()) : 0;
}

// static
unsigned int PlayerManager::numPreviewDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    ControlProxy* pCOPNumPreviewDecks = atomicLoadRelaxed(m_pCOPNumPreviewDecks);
    if (pCOPNumPreviewDecks == nullptr) {
        pCOPNumPreviewDecks = new ControlProxy(
                ConfigKey("[Master]", "num_preview_decks"));
        if (!pCOPNumPreviewDecks->valid()) {
            delete pCOPNumPreviewDecks;
            pCOPNumPreviewDecks = nullptr;
        } else {
            m_pCOPNumPreviewDecks = pCOPNumPreviewDecks;
        }
    }
    // m_pCOPNumPreviewDecks->get() fails on MacOs
    return pCOPNumPreviewDecks ? static_cast<int>(pCOPNumPreviewDecks->get()) : 0;
}

void PlayerManager::slotChangeNumDecks(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;

    VERIFY_OR_DEBUG_ASSERT(num <= kMaxNumberOfDecks) {
        qWarning() << "Number of decks exceeds the maximum we expect."
                   << num << "vs" << kMaxNumberOfDecks
                   << " Refusing to add another deck. Please update util/defs.h";
        return;
    }

    // Update the soundmanager config even if the number of decks has been
    // reduced.
    m_pSoundManager->setConfiguredDeckCount(num);

    if (num < m_decks.size()) {
        // The request was invalid -- reset the value.
        kLogger.debug() << "Ignoring request to reduce the number of decks to" << num;
        return;
    }

    if (m_decks.size() < num) {
        do {
            addDeckInner();
        } while (m_decks.size() < num);
        m_pCONumDecks->setAndConfirm(m_decks.size());
        emit numberOfDecksChanged(m_decks.count());
    }
}

void PlayerManager::slotChangeNumSamplers(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_samplers.size()) {
        // The request was invalid -- don't set the value.
        kLogger.debug() << "Ignoring request to reduce the number of samplers to" << num;
        return;
    }

    while (m_samplers.size() < num) {
        addSamplerInner();
    }
    m_pCONumSamplers->setAndConfirm(m_samplers.size());
}

void PlayerManager::slotChangeNumPreviewDecks(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_previewDecks.size()) {
        // The request was invalid -- don't set the value.
        kLogger.debug() << "Ignoring request to reduce the number of preview decks to" << num;
        return;
    }
    while (m_previewDecks.size() < num) {
        addPreviewDeckInner();
    }
    m_pCONumPreviewDecks->setAndConfirm(m_previewDecks.size());
}

void PlayerManager::slotChangeNumMicrophones(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_microphones.size()) {
        // The request was invalid -- don't set the value.
        kLogger.debug() << "Ignoring request to reduce the number of microphones to" << num;
        return;
    }
    while (m_microphones.size() < num) {
        addMicrophoneInner();
    }
    m_pCONumMicrophones->setAndConfirm(m_microphones.size());
}

void PlayerManager::slotChangeNumAuxiliaries(double v) {
    QMutexLocker locker(&m_mutex);
    int num = (int)v;
    if (num < m_auxiliaries.size()) {
        // The request was invalid -- don't set the value.
        kLogger.debug() << "Ignoring request to reduce the number of auxiliaries to" << num;
        return;
    }
    while (m_auxiliaries.size() < num) {
        addAuxiliaryInner();
    }
    m_pCONumAuxiliaries->setAndConfirm(m_auxiliaries.size());
}

void PlayerManager::addDeck() {
    QMutexLocker locker(&m_mutex);
    double count = m_pCONumDecks->get() + 1;
    slotChangeNumDecks(count);
}

void PlayerManager::addConfiguredDecks() {
    slotChangeNumDecks(m_pSoundManager->getConfiguredDeckCount());
}

void PlayerManager::addDeckInner() {
    // Do not lock m_mutex here.
    ChannelHandleAndGroup handleGroup =
            m_pEngine->registerChannelGroup(groupForDeck(m_decks.count()));
    VERIFY_OR_DEBUG_ASSERT(!m_players.contains(handleGroup.handle())) {
        return;
    }

    int deckIndex = m_decks.count();

    Deck* pDeck = new Deck(this,
            m_pConfig,
            m_pEngine,
            m_pEffectsManager,
            m_pVisualsManager,
            deckIndex % 2 == 1 ? EngineChannel::RIGHT : EngineChannel::LEFT,
            handleGroup);
    connect(pDeck->getEngineDeck(),
            &EngineDeck::noPassthroughInputConfigured,
            this,
            &PlayerManager::noDeckPassthroughInputConfigured);
    connect(pDeck,
            &Deck::noVinylControlInputConfigured,
            this,
            &PlayerManager::noVinylControlInputConfigured);

    if (m_pTrackAnalysisScheduler) {
        connect(pDeck,
                &Deck::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }

    m_players[handleGroup.handle()] = pDeck;
    m_decks.append(pDeck);

    // Register the deck output with SoundManager.
    m_pSoundManager->registerOutput(
            AudioOutput(AudioOutput::DECK, 0, 2, deckIndex), m_pEngine);

    // Register vinyl input signal with deck for passthrough support.
    EngineDeck* pEngineDeck = pDeck->getEngineDeck();
    m_pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, 2, deckIndex), pEngineDeck);

    // Setup equalizer rack for this deck.
    EqualizerRackPointer pEqRack = m_pEffectsManager->getEqualizerRack(0);
    VERIFY_OR_DEBUG_ASSERT(pEqRack) {
        return;
    }
    pEqRack->setupForGroup(handleGroup.name());

    // BaseTrackPlayer needs to delay until we have setup the equalizer rack for
    // this deck to fetch the legacy EQ controls.
    // TODO(rryan): Find a way to remove this cruft.
    pDeck->setupEqControls();

    // Setup quick effect rack for this deck.
    QuickEffectRackPointer pQuickEffectRack =
            m_pEffectsManager->getQuickEffectRack(0);
    VERIFY_OR_DEBUG_ASSERT(pQuickEffectRack) {
        return;
    }
    pQuickEffectRack->setupForGroup(handleGroup.name());
}

void PlayerManager::loadSamplers() {
    m_pSamplerBank->loadSamplerBankFromPath(
            m_pConfig->getSettingsPath() + "/samplers.xml");
}

void PlayerManager::addSampler() {
    QMutexLocker locker(&m_mutex);
    double count = m_pCONumSamplers->get() + 1;
    slotChangeNumSamplers(count);
}

void PlayerManager::addSamplerInner() {
    // Do not lock m_mutex here.
    ChannelHandleAndGroup handleGroup =
            m_pEngine->registerChannelGroup(groupForSampler(m_samplers.count()));
    VERIFY_OR_DEBUG_ASSERT(!m_players.contains(handleGroup.handle())) {
        return;
    }

    // All samplers are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    Sampler* pSampler = new Sampler(this,
            m_pConfig,
            m_pEngine,
            m_pEffectsManager,
            m_pVisualsManager,
            orientation,
            handleGroup);
    if (m_pTrackAnalysisScheduler) {
        connect(pSampler,
                &Sampler::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }

    m_players[handleGroup.handle()] = pSampler;
    m_samplers.append(pSampler);
}

void PlayerManager::addPreviewDeck() {
    QMutexLocker locker(&m_mutex);
    slotChangeNumPreviewDecks(m_pCONumPreviewDecks->get() + 1);
}

void PlayerManager::addPreviewDeckInner() {
    // Do not lock m_mutex here.
    ChannelHandleAndGroup handleGroup = m_pEngine->registerChannelGroup(
            groupForPreviewDeck(m_previewDecks.count()));
    VERIFY_OR_DEBUG_ASSERT(!m_players.contains(handleGroup.handle())) {
        return;
    }

    // All preview decks are in the center
    EngineChannel::ChannelOrientation orientation = EngineChannel::CENTER;

    PreviewDeck* pPreviewDeck = new PreviewDeck(this,
            m_pConfig,
            m_pEngine,
            m_pEffectsManager,
            m_pVisualsManager,
            orientation,
            handleGroup);
    if (m_pTrackAnalysisScheduler) {
        connect(pPreviewDeck,
                &PreviewDeck::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }

    m_players[handleGroup.handle()] = pPreviewDeck;
    m_previewDecks.append(pPreviewDeck);
}

void PlayerManager::addMicrophone() {
    QMutexLocker locker(&m_mutex);
    slotChangeNumMicrophones(m_pCONumMicrophones->get() + 1);
}

void PlayerManager::addMicrophoneInner() {
    // Do not lock m_mutex here.
    int index = m_microphones.count();
    QString group = groupForMicrophone(index);
    Microphone* pMicrophone = new Microphone(this,
            group,
            index,
            m_pSoundManager,
            m_pEngine,
            m_pEffectsManager);
    connect(pMicrophone,
            &Microphone::noMicrophoneInputConfigured,
            this,
            &PlayerManager::noMicrophoneInputConfigured);
    m_microphones.append(pMicrophone);
}

void PlayerManager::addAuxiliary() {
    QMutexLocker locker(&m_mutex);
    slotChangeNumAuxiliaries(m_pCONumAuxiliaries->get() + 1);
}

void PlayerManager::addAuxiliaryInner() {
    // Do not lock m_mutex here.
    int index = m_auxiliaries.count();
    QString group = groupForAuxiliary(index);

    auto pAuxiliary = new Auxiliary(this, group, index, m_pSoundManager, m_pEngine, m_pEffectsManager);
    connect(pAuxiliary,
            &Auxiliary::noAuxiliaryInputConfigured,
            this,
            &PlayerManager::noAuxiliaryInputConfigured);
    m_auxiliaries.append(pAuxiliary);
}

BaseTrackPlayer* PlayerManager::getPlayer(const QString& group) const {
    return getPlayer(m_pEngine->registerChannelGroup(group).handle());
}

BaseTrackPlayer* PlayerManager::getPlayer(const ChannelHandle& handle) const {
    QMutexLocker locker(&m_mutex);

    if (m_players.contains(handle)) {
        return m_players[handle];
    }
    return nullptr;
}

Deck* PlayerManager::getDeck(unsigned int deck) const {
    QMutexLocker locker(&m_mutex);
    VERIFY_OR_DEBUG_ASSERT(deck > 0 && deck <= numDecks()) {
        qWarning() << "getDeck() called with invalid number:" << deck;
        return nullptr;
    }
    return m_decks[deck - 1];
}

PreviewDeck* PlayerManager::getPreviewDeck(unsigned int libPreviewPlayer) const {
    QMutexLocker locker(&m_mutex);
    if (libPreviewPlayer < 1 || libPreviewPlayer > numPreviewDecks()) {
        kLogger.warning() << "Warning getPreviewDeck() called with invalid index: "
                   << libPreviewPlayer;
        return NULL;
    }
    return m_previewDecks[libPreviewPlayer - 1];
}

Sampler* PlayerManager::getSampler(unsigned int sampler) const {
    QMutexLocker locker(&m_mutex);
    if (sampler < 1 || sampler > numSamplers()) {
        kLogger.warning() << "Warning getSampler() called with invalid index: "
                   << sampler;
        return NULL;
    }
    return m_samplers[sampler - 1];
}

Microphone* PlayerManager::getMicrophone(unsigned int microphone) const {
    QMutexLocker locker(&m_mutex);
    if (microphone < 1 || microphone >= static_cast<unsigned int>(m_microphones.size())) {
        kLogger.warning() << "Warning getMicrophone() called with invalid index: "
                   << microphone;
        return NULL;
    }
    return m_microphones[microphone - 1];
}

Auxiliary* PlayerManager::getAuxiliary(unsigned int auxiliary) const {
    QMutexLocker locker(&m_mutex);
    if (auxiliary < 1 || auxiliary > static_cast<unsigned int>(m_auxiliaries.size())) {
        kLogger.warning() << "Warning getAuxiliary() called with invalid index: "
                   << auxiliary;
        return NULL;
    }
    return m_auxiliaries[auxiliary - 1];
}

void PlayerManager::slotCloneDeck(QString source_group, QString target_group) {
    BaseTrackPlayer* pPlayer = getPlayer(target_group);

    if (pPlayer == nullptr) {
        qWarning() << "Invalid group argument " << target_group << " to slotCloneDeck.";
        return;
    }

    pPlayer->slotCloneFromGroup(source_group);
}

void PlayerManager::slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play) {
    // Do not lock mutex in this method unless it is changed to access
    // PlayerManager state.
    BaseTrackPlayer* pPlayer = getPlayer(group);

    if (pPlayer == NULL) {
        kLogger.warning() << "Invalid group argument " << group << " to slotLoadTrackToPlayer.";
        return;
    }

    mixxx::Duration elapsed = m_cloneTimer.restart();
    // If not present in the config, use & set the default value
    bool cloneOnDoubleTap = m_pConfig->getValue(
            ConfigKey("[Controls]", "CloneDeckOnLoadDoubleTap"), kDefaultCloneDeckOnLoad);

    // If AutoDJ is enabled, prevent it from cloning decks if the same track
    // is in the AutoDJ queue twice in a row. This can happen when the option to
    // repeat the AutoDJ queue is enabled and the user presses the "Skip now"
    // button repeatedly.
    // AutoDJProcessor is initialized after PlayerManager, so check that the
    // ControlProxy is pointing to the real ControlObject.
    if (!m_pAutoDjEnabled) {
        m_pAutoDjEnabled = make_parented<ControlProxy>("[AutoDJ]", "enabled", this);
    }
    bool autoDjSkipClone = m_pAutoDjEnabled->toBool() &&
            (pPlayer == m_decks.at(0) || pPlayer == m_decks.at(1));

    if (cloneOnDoubleTap && m_lastLoadedPlayer == group
        && elapsed < mixxx::Duration::fromSeconds(0.5)
        && !autoDjSkipClone) {
        // load was pressed twice quickly while [Controls],CloneDeckOnLoadDoubleTap is TRUE,
        // so clone another playing deck instead of loading the selected track
        pPlayer->slotCloneDeck();
    } else {
        pPlayer->slotLoadTrack(pTrack, play);
    }

    m_lastLoadedPlayer = group;
}

void PlayerManager::slotLoadToPlayer(QString location, QString group) {
    // The library will get the track and then signal back to us to load the
    // track via slotLoadTrackToPlayer.
    emit loadLocationToPlayer(location, group);
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

void PlayerManager::slotAnalyzeTrack(TrackPointer track) {
    VERIFY_OR_DEBUG_ASSERT(track) {
        return;
    }
    if (m_pTrackAnalysisScheduler) {
        if (m_pTrackAnalysisScheduler->scheduleTrackById(track->getId())) {
            m_pTrackAnalysisScheduler->resume();
        }
        // The first progress signal will suspend a running batch analysis
        // until all loaded tracks have been analyzed. Emit it once just now
        // before any signals from the analyzer queue arrive.
        emit trackAnalyzerProgress(track->getId(), kAnalyzerProgressUnknown);
    }
}

void PlayerManager::onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    emit trackAnalyzerProgress(trackId, analyzerProgress);
}

void PlayerManager::onTrackAnalysisFinished() {
    emit trackAnalyzerIdle();
}
