#include "coreservices.h"

#include <QList>
#include <QSet>
#include <QString>

#include "database/mixxxdb.h"
#include "effects/builtin/builtinbackend.h"
#ifdef __LILV__
#include "effects/lv2/lv2backend.h"
#endif
#include "mixer/playerinfo.h"
#include "sources/soundsourceproxy.h"
#include "track/bpm.h"
#include "track/replaygain.h"
#include "track/trackid.h"
#include "track/track.h"
#include "util/cmdlineargs.h"
#include "util/db/dbconnectionpooled.h"
#include "util/debug.h"
#include "util/font.h"
#include "util/logger.h"
#include "util/logging.h"
#include "util/memory.h"
#include "util/statsmanager.h"
#include "util/time.h"
#include "util/timer.h"
#include "util/translations.h"
#include "util/version.h"

namespace mixxx {
namespace {

const Logger kLogger("CoreServices");

} // anonymous namespace

void CoreServices::initializeStaticServices() {
    Version::logBuildDetails();
    const CmdlineArgs& args = CmdlineArgs::Instance();

    // Create the ErrorDialogHandler in the main thread, otherwise it will be
    // created in the thread of the first caller to instance(), which may not be
    // the main thread. Bug #1748636.
    ErrorDialogHandler::instance();

    Logging::initialize(args.getSettingsPath(),
                        args.getLogLevel(),
                        args.getLogFlushLevel(),
                        args.getDebugAssertBreak());

    SoundSourceProxy::registerSoundSourceProviders();

    // Start the global timer.
    Time::start();

    // Only record stats in developer mode.
    if (args.getDeveloper()) {
        StatsManager::createInstance();
    }
}

void CoreServices::shutdownStaticServices() {
    StatsManager::destroy();
    Logging::shutdown();
}

CoreServices::CoreServices(QObject* pParent, QCoreApplication* pApplication,
                           const QString& profilePath)
        : QObject(pParent),
          m_runtime_timer("CoreServices::runtime") {
    ScopedTimer t("CoreServices::CoreServices");
    const CmdlineArgs& args = CmdlineArgs::Instance();

    m_pSettingsManager = std::make_shared<SettingsManager>(this, profilePath);
    Sandbox::initialize(QDir(profilePath).filePath("sandbox.cfg"));

    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    QString resourcePath = pConfig->getResourcePath();

    Translations::initializeTranslations(
        pConfig, pApplication, args.getLocale());

    FontUtils::initializeFonts(resourcePath);

    m_pGuiTick = std::make_shared<GuiTick>();

    // TODO(rryan): Move this back into EngineMaster. The only reason it got
    // pulled out is due to some routing issues (EngineAux/Mic/etc. need access
    // to ChannelHandleFactory, so it was routed through EffectsManager, which
    // was the shortest path from A to B).
    m_pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();

    // Create the Effects subsystem.
    m_pEffectsManager = std::make_shared<EffectsManager>(this, pConfig, m_pChannelHandleFactory);

    // Starting the master (mixing of the channels and effects):
    m_pEngine = std::make_shared<EngineMaster>(
        pConfig, "[Master]", m_pEffectsManager, m_pChannelHandleFactory, true);

    // Create the SoundManager for managing sound I/O with the operating system.
    m_pSoundManager = std::make_shared<SoundManager>(pConfig, m_pEngine);

    m_pRecordingManager = std::make_shared<RecordingManager>(pConfig, m_pEngine);

#ifdef __BROADCAST__
    m_pBroadcastManager = std::make_shared<BroadcastManager>(m_pSettingsManager, m_pSoundManager);

#endif

#ifdef __VINYLCONTROL__
    m_pVinylControlManager = std::make_shared<VinylControlManager>(
        this, pConfig, m_pSoundManager);
#endif

    m_pPlayerManager = std::make_shared<PlayerManager>(pConfig, m_pSoundManager,
                                                       m_pEffectsManager,
                                                       m_pEngine);

    m_pDbConnectionPool = MixxxDb(pConfig).connectionPool();
    if (!m_pDbConnectionPool) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }
    // Create a connection for the main thread
    m_pDbConnectionPool->createThreadLocalConnection();
    if (!initializeDatabase()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }

    m_pLibrary = std::make_shared<Library>(this, pConfig, m_pDbConnectionPool,
                                           m_pPlayerManager, m_pRecordingManager);
    // Create the singular GlobalTrackCache instance immediately after
    // the Library has been created and BEFORE binding the
    // PlayerManager to it!
    GlobalTrackCache::createInstance(
            std::static_pointer_cast<GlobalTrackCacheSaver>(m_pLibrary));

    // TODO(rryan): de-singleton
    // TODO(rryan): Where does this belong?
    CoverArtCache::createInstance();

    // Initialize controller sub-system.
    m_pControllerManager = std::make_shared<ControllerManager>(pConfig);
}

#define CLEAR_AND_CHECK_DELETED(x) clearHelper(x, #x);

template <typename T>
void clearHelper(std::shared_ptr<T>& ref_ptr, const char* name) {
    std::weak_ptr<T> weak(ref_ptr);
    ref_ptr.reset();
    if (auto shared = weak.lock()) {
        qWarning() << name << "was leaked! Use count:" << shared.use_count();
        DEBUG_ASSERT(false);
    }
}

CoreServices::~CoreServices() {
    ScopedTimer t("CoreServices::~CoreServices");

    m_pSettingsManager->save();

    CLEAR_AND_CHECK_DELETED(m_pControllerManager);

#ifdef __VINYLCONTROL__
    // VinylControlManager depends on a CO the engine owns
    // (vinylcontrol_enabled in VinylControlControl) and SoundManager.
    CLEAR_AND_CHECK_DELETED(m_pVinylControlManager);
#endif

    // CoverArtCache is fairly independent of everything else.
    CoverArtCache::destroy();

    // Drop any references to TrackPointers for loaded tracks in PlayerInfo.
    PlayerInfo::destroy();

    // Evict all remaining tracks from the cache to trigger
    // updating of modified tracks. We assume that no other
    // components are accessing those files at this point.
    GlobalTrackCacheLocker().deactivateCache();

    // Delete the library after the view so there are no dangling pointers to
    // the data models.
    // Depends on RecordingManager and PlayerManager
    CLEAR_AND_CHECK_DELETED(m_pLibrary);

    // PlayerManager depends on Engine, SoundManager, VinylControlManager, and Config
    // TODO(rryan): Fix this bug:
    // The player manager has to be deleted before the library to ensure
    // that all modified track metadata of loaded tracks is saved.
    CLEAR_AND_CHECK_DELETED(m_pPlayerManager);

    // PlayerManager holds a reference to the DB connection pool (because it
    // creates an AnalyzerQueue).
    m_pDbConnectionPool->destroyThreadLocalConnection();
    CLEAR_AND_CHECK_DELETED(m_pDbConnectionPool);

    // RecordingManager depends on config, engine
    CLEAR_AND_CHECK_DELETED(m_pRecordingManager);

#ifdef __BROADCAST__
    // BroadcastManager depends on config, SoundManager, engine
    CLEAR_AND_CHECK_DELETED(m_pBroadcastManager);
#endif

    // SoundManager depends on engine and VinylControlProcessor.
    CLEAR_AND_CHECK_DELETED(m_pSoundManager);

    // EngineMaster depends on Config and m_pEffectsManager.
    CLEAR_AND_CHECK_DELETED(m_pEngine);

    // Must delete after EngineMaster and DlgPrefEq.
    CLEAR_AND_CHECK_DELETED(m_pEffectsManager);

    CLEAR_AND_CHECK_DELETED(m_pGuiTick);

    controlLeakCheck();

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pSettingsManager->save();
    CLEAR_AND_CHECK_DELETED(m_pSettingsManager);

    Sandbox::shutdown();
}

void CoreServices::initialize() {
    ScopedTimer t("CoreServices::initialize");

    // Create effect backends. We do this after creating EngineMaster to allow
    // effect backends to refer to controls that are produced by the engine.
    BuiltInBackend* pBuiltInBackend = new BuiltInBackend(m_pEffectsManager.get());
    m_pEffectsManager->addEffectsBackend(pBuiltInBackend);
#ifdef __LILV__
    LV2Backend* pLV2Backend = new LV2Backend(m_pEffectsManager.get());
    m_pEffectsManager->addEffectsBackend(pLV2Backend);
#endif

    // Sets up the effect backends, default EffectChains and EffectRacks (long).
    m_pEffectsManager->setup();

    const int kMicrophoneCount = 4;
    const int kAuxiliaryCount = 4;

    for (int i = 0; i < kMicrophoneCount; ++i) {
        m_pPlayerManager->addMicrophone();
    }

    for (int i = 0; i < kAuxiliaryCount; ++i) {
        m_pPlayerManager->addAuxiliary();
    }

    m_pPlayerManager->addConfiguredDecks();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addPreviewDeck();

    // TODO(rryan): Why is this here?
    m_pEffectsManager->loadEffectChains();

#ifdef __VINYLCONTROL__
    m_pVinylControlManager->init();
#endif

    // Binding the PlayManager to the Library may already trigger
    // loading of tracks which requires that the GlobalTrackCache has
    // been created. Otherwise Mixxx might hang when accessing
    // the uninitialized singleton instance!
    m_pPlayerManager->bindToLibrary(m_pLibrary);

    // Wait until all other ControlObjects are set up before initializing
    // controllers
    m_pControllerManager->setUpDevices();

    // This has to be done before m_pSoundManager->setupDevices()
    // https://bugs.launchpad.net/mixxx/+bug/1758189
    m_pPlayerManager->loadSamplers();
}

void CoreServices::finalize() {
    ScopedTimer t("CoreServices::finalize");
}

void CoreServices::controlLeakCheck() const {
    // Check for leaked ControlObjects and give warnings.
    QList<QSharedPointer<ControlDoublePrivate> > leakedControls;
    QList<ConfigKey> leakedConfigKeys;

    ControlDoublePrivate::getControls(&leakedControls);

    if (leakedControls.empty()) {
        return;
    }
    qDebug() << "WARNING: The following" << leakedControls.size()
             << "controls were leaked:";
    for (QSharedPointer<ControlDoublePrivate> pCDP : leakedControls) {
        if (pCDP.isNull()) {
            continue;
        }
        ConfigKey key = pCDP->getKey();
        qDebug() << key.group << key.item << pCDP->getCreatorCO();
        leakedConfigKeys.append(key);
    }

    // Deleting leaked objects helps to satisfy valgrind.
    // These delete calls could cause crashes if a destructor for a control
    // we thought was leaked is triggered after this one exits.
    // So, only delete so if developer mode is on.
    if (CmdlineArgs::Instance().getDeveloper()) {
        foreach (ConfigKey key, leakedConfigKeys) {
            // A deletion early in the list may trigger a destructor
            // for a control later in the list, so we check for a null
            // pointer each time.
            ControlObject* pCo = ControlObject::getControl(key, false);
            if (pCo) {
                delete pCo;
            }
        }
    }
}

bool CoreServices::initializeDatabase() {
    kLogger.info() << "Connecting to database";
    QSqlDatabase dbConnection = DbConnectionPooled(m_pDbConnectionPool);
    if (!dbConnection.isOpen()) {
        // TODO(rryan): No message box in non-GUI code.
        QMessageBox::critical(0, tr("Cannot open database"),
                            tr("Unable to establish a database connection.\n"
                                "Mixxx requires QT with SQLite support. Please read "
                                "the Qt SQL driver documentation for information on how "
                                "to build it.\n\n"
                                "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

    kLogger.info() << "Initializing or upgrading database schema";
    return MixxxDb::initDatabaseSchema(dbConnection);
}

}  // namespace mixxx
