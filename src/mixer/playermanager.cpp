#include "mixer/playermanager.h"

#include <QRegularExpression>

#include "audio/types.h"
#include "control/controlobject.h"
#include "effects/effectsmanager.h"
#include "engine/channels/enginedeck.h"
#include "engine/enginemixer.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "mixer/auxiliary.h"
#include "mixer/deck.h"
#include "mixer/microphone.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "mixer/samplerbank.h"
#include "moc_playermanager.cpp"
#include "preferences/dialog/dlgprefdeck.h"
#include "soundio/soundmanager.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/compatibility/qatomic.h"
#include "util/defs.h"
#include "util/logger.h"

namespace {

#ifdef __STEM__
constexpr int kMaxSupportedStems = 5;
#endif

const mixxx::Logger kLogger("PlayerManager");
const QString kAppGroup = QStringLiteral("[App]");
const QString kLegacyGroup = QStringLiteral("[Master]");

// Utilize half of the available cores for adhoc analysis of tracks
const int kNumberOfAnalyzerThreads = math_max(1, QThread::idealThreadCount() / 2);

const QRegularExpression kDeckRegex(QStringLiteral("^\\[Channel(\\d+)\\]$"));
const QRegularExpression kSamplerRegex(QStringLiteral("^\\[Sampler(\\d+)\\]$"));
const QRegularExpression kPreviewDeckRegex(QStringLiteral("^\\[PreviewDeck(\\d+)\\]$"));

bool extractIntFromRegex(const QRegularExpression& regex, const QString& group, int* number) {
    const QRegularExpressionMatch match = regex.match(group);
    DEBUG_ASSERT(match.isValid());
    if (!match.hasMatch()) {
        return false;
    }
    // The regex is expected to contain a single capture group with the number
    constexpr int capturedNumberIndex = 1;
    DEBUG_ASSERT(match.lastCapturedIndex() <= capturedNumberIndex);
    if (match.lastCapturedIndex() < capturedNumberIndex) {
        qWarning() << "No number found in group" << group;
        return false;
    }
    if (number) {
        const QString capturedNumber = match.captured(capturedNumberIndex);
        DEBUG_ASSERT(!capturedNumber.isNull());
        bool okay = false;
        const int numberFromMatch = capturedNumber.toInt(&okay);
        VERIFY_OR_DEBUG_ASSERT(okay) {
            return false;
        }
        *number = numberFromMatch;
    }
    return true;
}

/// Returns the first object from a list of `BaseTrackPlayer` instances where
/// the corresponding `play` CO is set to 0.
template<class T>
T* findFirstStoppedPlayerInList(const QList<T*>& players) {
    for (T* pPlayer : players) {
        VERIFY_OR_DEBUG_ASSERT(pPlayer != nullptr) {
            continue;
        }

        ControlObject* pPlayControl = ControlObject::getControl(
                ConfigKey(pPlayer->getGroup(), "play"));
        VERIFY_OR_DEBUG_ASSERT(pPlayControl != nullptr) {
            continue;
        }

        if (!pPlayControl->toBool()) {
            return pPlayer;
        }
    }

    // There is no stopped player in the list.
    return nullptr;
}

inline QString getDefaultSamplerPath(UserSettingsPointer pConfig) {
    return pConfig->getSettingsPath() + QStringLiteral("/samplers.xml");
}

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
        EngineMixer* pEngine)
        : m_mutex(QT_RECURSIVE_MUTEX_INIT),
          m_pConfig(pConfig),
          m_pLibrary(nullptr),
          m_pSoundManager(pSoundManager),
          m_pEffectsManager(pEffectsManager),
          m_pEngine(pEngine),
          // NOTE(XXX) LegacySkinParser relies on these controls being Controls
          // and not ControlProxies.
          m_pCONumDecks(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("num_decks")), true, true)),
          m_pCONumSamplers(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("num_samplers")), true, true)),
          m_pCONumPreviewDecks(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("num_preview_decks")), true, true)),
          m_pCONumMicrophones(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("num_microphones")), true, true)),
          m_pCONumAuxiliaries(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("num_auxiliaries")), true, true)),
          m_pTrackAnalysisScheduler(TrackAnalysisScheduler::NullPointer()) {
    m_pCONumDecks->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("num_decks")));
    m_pCONumDecks->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumDecks, Qt::DirectConnection);
    m_pCONumSamplers->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("num_samplers")));
    m_pCONumSamplers->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumSamplers, Qt::DirectConnection);
    m_pCONumPreviewDecks->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("num_preview_decks")));
    m_pCONumPreviewDecks->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumPreviewDecks, Qt::DirectConnection);
    m_pCONumMicrophones->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("num_microphones")));
    m_pCONumMicrophones->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumMicrophones, Qt::DirectConnection);
    m_pCONumAuxiliaries->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("num_auxiliaries")));
    m_pCONumAuxiliaries->connectValueChangeRequest(this,
            &PlayerManager::slotChangeNumAuxiliaries, Qt::DirectConnection);

    // This is parented to the PlayerManager so does not need to be deleted
    m_pSamplerBank = new SamplerBank(m_pConfig, this);

    m_cloneTimer.start();
}

PlayerManager::~PlayerManager() {
    kLogger.debug() << "Destroying";

    const auto locker = lockMutex(&m_mutex);

    m_pSamplerBank->saveSamplerBankToPath(getDefaultSamplerPath(m_pConfig));
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

    if (m_pTrackAnalysisScheduler) {
        m_pTrackAnalysisScheduler->stop();
        m_pTrackAnalysisScheduler.reset();
    }
}

void PlayerManager::bindToLibrary(Library* pLibrary) {
    m_pLibrary = pLibrary;
    const auto locker = lockMutex(&m_mutex);
    connect(pLibrary, &Library::loadTrackToPlayer, this, &PlayerManager::slotLoadTrackToPlayer);
    connect(pLibrary,
            &Library::loadTrack,
            this,
            &PlayerManager::slotLoadTrackIntoNextAvailableDeck);
    connect(this,
            &PlayerManager::loadLocationToPlayer,
            pLibrary,
            &Library::slotLoadLocationToPlayer);

    DEBUG_ASSERT(!m_pTrackAnalysisScheduler);
    m_pTrackAnalysisScheduler = pLibrary->createTrackAnalysisScheduler(
            kNumberOfAnalyzerThreads,
            AnalyzerModeFlags::WithWaveform);

    connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::trackProgress,
            this, &PlayerManager::onTrackAnalysisProgress);
    connect(m_pTrackAnalysisScheduler.get(), &TrackAnalysisScheduler::finished,
            this, &PlayerManager::onTrackAnalysisFinished);

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach(Deck* pDeck, m_decks) {
        connect(pDeck, &BaseTrackPlayer::newTrackLoaded, this, &PlayerManager::slotAnalyzeTrack);
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach(Sampler* pSampler, m_samplers) {
        connect(pSampler, &BaseTrackPlayer::newTrackLoaded, this, &PlayerManager::slotAnalyzeTrack);
    }

    // Connect the player to the analyzer queue so that loaded tracks are
    // analyzed.
    foreach (PreviewDeck* pPreviewDeck, m_previewDecks) {
        connect(pPreviewDeck,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }
}

QStringList PlayerManager::getVisualPlayerGroups() {
    QStringList groups;
    for (const auto& pDeck : std::as_const(m_decks)) {
        groups.append(pDeck->getGroup());
    }
    for (const auto& pPreview : std::as_const(m_previewDecks)) {
        groups.append(pPreview->getGroup());
    }
    for (const auto& pSampler : std::as_const(m_samplers)) {
        groups.append(pSampler->getGroup());
    }
    return groups;
}

// static
bool PlayerManager::isDeckGroup(const QString& group, int* number) {
    return extractIntFromRegex(kDeckRegex, group, number);
}

// static
bool PlayerManager::isSamplerGroup(const QString& group, int* number) {
    return extractIntFromRegex(kSamplerRegex, group, number);
}

// static
bool PlayerManager::isPreviewDeckGroup(const QString& group, int* number) {
    return extractIntFromRegex(kPreviewDeckRegex, group, number);
}

// static
unsigned int PlayerManager::numDecks() {
    // We do this to cache the control once it is created so callers don't incur
    // a hashtable lookup every time they call this.
    ControlProxy* pCOPNumDecks = atomicLoadRelaxed(m_pCOPNumDecks);
    if (pCOPNumDecks == nullptr) {
        pCOPNumDecks = new ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_decks")));
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
        pCOPNumSamplers = new ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_samplers")));
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
                ConfigKey(kAppGroup, QStringLiteral("num_preview_decks")));
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
    const auto locker = lockMutex(&m_mutex);
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
    const auto locker = lockMutex(&m_mutex);
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
    emit numberOfSamplersChanged(m_samplers.count());
}

void PlayerManager::slotChangeNumPreviewDecks(double v) {
    const auto locker = lockMutex(&m_mutex);
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
    const auto locker = lockMutex(&m_mutex);
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
    const auto locker = lockMutex(&m_mutex);
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
    const auto locker = lockMutex(&m_mutex);
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
            deckIndex % 2 == 1 ? EngineChannel::RIGHT : EngineChannel::LEFT,
            handleGroup);
    connect(pDeck->getEngineDeck(),
            &EngineDeck::noPassthroughInputConfigured,
            this,
            &PlayerManager::noDeckPassthroughInputConfigured);
    connect(pDeck,
            &BaseTrackPlayer::noVinylControlInputConfigured,
            this,
            &PlayerManager::noVinylControlInputConfigured);
    connect(pDeck,
            &BaseTrackPlayer::trackUnloaded,
            this,
            &PlayerManager::slotSaveEjectedTrack);

    if (m_pTrackAnalysisScheduler) {
        connect(pDeck,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }

    m_players[handleGroup.handle()] = pDeck;
    m_decks.append(pDeck);

    // Register the deck output with SoundManager.
    m_pSoundManager->registerOutput(
            AudioOutput(AudioPathType::Deck,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    deckIndex),
            m_pEngine);

    // Register vinyl input signal with deck for passthrough support.
    EngineDeck* pEngineDeck = pDeck->getEngineDeck();
    m_pSoundManager->registerInput(AudioInput(AudioPathType::VinylControl,
                                           0,
                                           mixxx::audio::ChannelCount::stereo(),
                                           deckIndex),
            pEngineDeck);

    // Setup equalizer and QuickEffect chain for this deck.
    m_pEffectsManager->addDeck(handleGroup);

#ifdef __STEM__
    // Setup stem QuickEffect chain for this deck
    for (int i = 0; i < kMaxSupportedStems; i++) {
        ChannelHandleAndGroup stemHandleGroup =
                m_pEngine->registerChannelGroup(groupForDeckStem(deckIndex, i));
        pDeck->getEngineDeck()->addStemHandle(stemHandleGroup);
        m_pEffectsManager->addStem(stemHandleGroup);
    }
#endif

    // Setup EQ ControlProxies used for resetting EQs on track load
    pDeck->setupEqControls();
}

void PlayerManager::loadSamplers() {
    m_pSamplerBank->loadSamplerBankFromPath(getDefaultSamplerPath(m_pConfig));
}

void PlayerManager::addSampler() {
    const auto locker = lockMutex(&m_mutex);
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
            orientation,
            handleGroup);
    if (m_pTrackAnalysisScheduler) {
        connect(pSampler,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }
    connect(pSampler,
            &BaseTrackPlayer::trackUnloaded,
            this,
            &PlayerManager::slotSaveEjectedTrack);

    m_players[handleGroup.handle()] = pSampler;
    m_samplers.append(pSampler);
}

void PlayerManager::addPreviewDeck() {
    const auto locker = lockMutex(&m_mutex);
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
            orientation,
            handleGroup);
    if (m_pTrackAnalysisScheduler) {
        connect(pPreviewDeck,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &PlayerManager::slotAnalyzeTrack);
    }

    m_players[handleGroup.handle()] = pPreviewDeck;
    m_previewDecks.append(pPreviewDeck);
}

void PlayerManager::addMicrophone() {
    const auto locker = lockMutex(&m_mutex);
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
    const auto locker = lockMutex(&m_mutex);
    slotChangeNumAuxiliaries(m_pCONumAuxiliaries->get() + 1);
}

void PlayerManager::addAuxiliaryInner() {
    // Do not lock m_mutex here.
    int index = m_auxiliaries.count();
    QString group = groupForAuxiliary(index);

    auto* pAuxiliary = new Auxiliary(
            this, group, index, m_pSoundManager, m_pEngine, m_pEffectsManager);
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
    const auto locker = lockMutex(&m_mutex);

    if (m_players.contains(handle)) {
        return m_players[handle];
    }
    return nullptr;
}

Deck* PlayerManager::getDeck(unsigned int deck) const {
    const auto locker = lockMutex(&m_mutex);
    VERIFY_OR_DEBUG_ASSERT(deck > 0 && deck <= numDecks()) {
        qWarning() << "getDeck() called with invalid number:" << deck;
        return nullptr;
    }
    return m_decks[deck - 1];
}

PreviewDeck* PlayerManager::getPreviewDeck(unsigned int libPreviewPlayer) const {
    const auto locker = lockMutex(&m_mutex);
    if (libPreviewPlayer < 1 || libPreviewPlayer > numPreviewDecks()) {
        kLogger.warning() << "Warning getPreviewDeck() called with invalid index: "
                   << libPreviewPlayer;
        return nullptr;
    }
    return m_previewDecks[libPreviewPlayer - 1];
}

Sampler* PlayerManager::getSampler(unsigned int sampler) const {
    const auto locker = lockMutex(&m_mutex);
    if (sampler < 1 || sampler > numSamplers()) {
        kLogger.warning() << "Warning getSampler() called with invalid index: "
                   << sampler;
        return nullptr;
    }
    return m_samplers[sampler - 1];
}

TrackPointer PlayerManager::getLastEjectedTrack() const {
    VERIFY_OR_DEBUG_ASSERT(m_pLibrary != nullptr) {
        return nullptr;
    }
    return m_pLibrary->trackCollectionManager()->getTrackById(m_lastEjectedTrackId);
}

TrackPointer PlayerManager::getSecondLastEjectedTrack() const {
    VERIFY_OR_DEBUG_ASSERT(m_pLibrary != nullptr) {
        return nullptr;
    }
    return m_pLibrary->trackCollectionManager()->getTrackById(m_secondLastEjectedTrackId);
}

Microphone* PlayerManager::getMicrophone(unsigned int microphone) const {
    const auto locker = lockMutex(&m_mutex);
    if (microphone < 1 || microphone >= static_cast<unsigned int>(m_microphones.size())) {
        kLogger.warning() << "Warning getMicrophone() called with invalid index: "
                   << microphone;
        return nullptr;
    }
    return m_microphones[microphone - 1];
}

Auxiliary* PlayerManager::getAuxiliary(unsigned int auxiliary) const {
    const auto locker = lockMutex(&m_mutex);
    if (auxiliary < 1 || auxiliary > static_cast<unsigned int>(m_auxiliaries.size())) {
        kLogger.warning() << "Warning getAuxiliary() called with invalid index: "
                   << auxiliary;
        return nullptr;
    }
    return m_auxiliaries[auxiliary - 1];
}

void PlayerManager::slotCloneDeck(const QString& source_group, const QString& target_group) {
    BaseTrackPlayer* pPlayer = getPlayer(target_group);

    if (pPlayer == nullptr) {
        qWarning() << "Invalid group argument " << target_group << " to slotCloneDeck.";
        return;
    }

    pPlayer->slotCloneFromGroup(source_group);
}

#ifdef __STEM__
void PlayerManager::slotLoadTrackToPlayer(TrackPointer pTrack,
        const QString& group,
        mixxx::StemChannelSelection stemMask,
        bool play) {
#else
void PlayerManager::slotLoadTrackToPlayer(
        TrackPointer pTrack, const QString& group, bool play) {
#endif
    // Do not lock mutex in this method unless it is changed to access
    // PlayerManager state.
    BaseTrackPlayer* pPlayer = getPlayer(group);

    if (pPlayer == nullptr) {
        kLogger.warning() << "Invalid group argument " << group << " to slotLoadTrackToPlayer.";
        return;
    }

    bool clone = false;
    if (isDeckGroup(group)) {
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

        if (cloneOnDoubleTap && m_lastLoadedPlayer == group &&
                elapsed < mixxx::Duration::fromSeconds(0.5) &&
                !autoDjSkipClone) {
            // load was pressed twice quickly while [Controls],CloneDeckOnLoadDoubleTap is TRUE,
            // so clone another playing deck instead of loading the selected track
            clone = true;
        }
    } else if (isPreviewDeckGroup(group) && play) {
        // This extends/overrides the behaviour of [PreviewDeckN],LoadSelectedTrackAndPlay:
        // if the track is already loaded, toggle play/pause.
        if (pTrack == pPlayer->getLoadedTrack()) {
            auto* pPlay =
                    ControlObject::getControl(ConfigKey(group, QStringLiteral("play")));
            double newPlay = pPlay->toBool() ? 0.0 : 1.0;
            pPlay->set(newPlay);
            return;
        }
    }

    if (clone) {
        pPlayer->slotCloneDeck();
    } else {
#ifdef __STEM__
        pPlayer->slotLoadTrack(pTrack, stemMask, play);
#else
        pPlayer->slotLoadTrack(pTrack, play);
#endif
    }

    m_lastLoadedPlayer = group;
}

void PlayerManager::slotLoadLocationToPlayer(
        const QString& location, const QString& group, bool play) {
    // The library will get the track and then signal back to us to load the
    // track via slotLoadTrackToPlayer.
    emit loadLocationToPlayer(location, group, play);
}

void PlayerManager::slotLoadLocationToPlayerMaybePlay(
        const QString& location, const QString& group) {
    bool play = false;
    LoadWhenDeckPlaying loadWhenDeckPlaying = m_pConfig->getValue(
            kConfigKeyLoadWhenDeckPlaying, kDefaultLoadWhenDeckPlaying);
    switch (loadWhenDeckPlaying) {
    case LoadWhenDeckPlaying::AllowButStopDeck:
    case LoadWhenDeckPlaying::Reject:
        break;
    case LoadWhenDeckPlaying::Allow:
        if (ControlObject::get(ConfigKey(group, "play")) > 0.0) {
            // deck is currently playing, so immediately play new track
            play = true;
        }
        break;
    }
    slotLoadLocationToPlayer(location, group, play);
}

void PlayerManager::slotLoadToDeck(const QString& location, int deck) {
    slotLoadLocationToPlayer(location, groupForDeck(deck - 1), false);
}

void PlayerManager::slotLoadToPreviewDeck(const QString& location, int previewDeck) {
    slotLoadLocationToPlayer(location, groupForPreviewDeck(previewDeck - 1), false);
}

void PlayerManager::slotLoadToSampler(const QString& location, int sampler) {
    slotLoadLocationToPlayer(location, groupForSampler(sampler - 1), false);
}

void PlayerManager::slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack) {
    auto locker = lockMutex(&m_mutex);
    BaseTrackPlayer* pDeck = findFirstStoppedPlayerInList(m_decks);
    if (pDeck == nullptr) {
        qDebug() << "PlayerManager: No stopped deck found, not loading track!";
        return;
    }

    pDeck->slotLoadTrack(pTrack,
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            false);
}

void PlayerManager::slotLoadLocationIntoNextAvailableDeck(const QString& location, bool play) {
    auto locker = lockMutex(&m_mutex);
    BaseTrackPlayer* pDeck = findFirstStoppedPlayerInList(m_decks);
    if (pDeck == nullptr) {
        qDebug() << "PlayerManager: No stopped deck found, not loading track!";
        return;
    }

    slotLoadLocationToPlayer(location, pDeck->getGroup(), play);
}

void PlayerManager::slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack) {
    auto locker = lockMutex(&m_mutex);
    BaseTrackPlayer* pSampler = findFirstStoppedPlayerInList(m_samplers);
    if (pSampler == nullptr) {
        qDebug() << "PlayerManager: No stopped sampler found, not loading track!";
        return;
    }
    locker.unlock();

#ifdef __STEM__
    pSampler->slotLoadTrack(pTrack, mixxx::StemChannelSelection(), false);
#else
    pSampler->slotLoadTrack(pTrack, false);
#endif
}

void PlayerManager::slotAnalyzeTrack(TrackPointer track) {
    VERIFY_OR_DEBUG_ASSERT(track) {
        return;
    }
    if (m_pTrackAnalysisScheduler) {
        if (m_pTrackAnalysisScheduler->scheduleTrack(track->getId())) {
            m_pTrackAnalysisScheduler->resume();
        }
        // The first progress signal will suspend a running batch analysis
        // until all loaded tracks have been analyzed. Emit it once just now
        // before any signals from the analyzer queue arrive.
        emit trackAnalyzerProgress(track->getId(), kAnalyzerProgressUnknown);
    }
}

void PlayerManager::slotSaveEjectedTrack(TrackPointer track) {
    VERIFY_OR_DEBUG_ASSERT(track) {
        return;
    }
    const TrackId id = track->getId();
    if (id == m_lastEjectedTrackId) {
        return;
    }
    m_secondLastEjectedTrackId = m_lastEjectedTrackId;
    m_lastEjectedTrackId = id;
}

void PlayerManager::onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    emit trackAnalyzerProgress(trackId, analyzerProgress);
}

void PlayerManager::onTrackAnalysisFinished() {
    emit trackAnalyzerIdle();
}
