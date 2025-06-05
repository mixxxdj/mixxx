#include "coreservices.h"

#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QtGlobal>
#include <gsl/pointers>

#ifdef __BROADCAST__
#include "broadcast/broadcastmanager.h"
#endif
#include "control/controlindicatortimer.h"
#include "controllers/controllermanager.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "database/mixxxdb.h"
#include "effects/effectsmanager.h"
#include "engine/enginemixer.h"
#ifdef __RUBBERBAND__
#include "engine/bufferscalers/rubberbandworkerpool.h"
#endif
#include "library/coverartcache.h"
#include "library/library.h"
#include "library/library_decl.h"
#include "library/library_prefs.h"
#include "library/overviewcache.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_coreservices.cpp"
#include "preferences/backup/backupsettings.h"
#include "preferences/dialog/dlgpreferences.h"
#include "preferences/settingsmanager.h"
#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif
#include "skin/skincontrols.h"
#ifdef MIXXX_USE_QML
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "controllers/scripting/controllerscriptenginebase.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmleffectsmanagerproxy.h"
#include "qml/qmllibraryproxy.h"
#include "qml/qmlplayermanagerproxy.h"
#endif
#include "soundio/soundmanager.h"
#include "sources/soundsourceproxy.h"
#include "util/clipboard.h"
#include "util/db/dbconnectionpooled.h"
#include "util/font.h"
#include "util/logger.h"
#include "util/screensavermanager.h"
#include "util/statsmanager.h"
#include "util/time.h"
#include "util/translations.h"
#include "util/versionstore.h"
#include "vinylcontrol/vinylcontrolmanager.h"

#ifdef __APPLE__
#include "util/sandbox.h"
#endif

#if defined(Q_OS_LINUX) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

#include <QtX11Extras/QX11Info>

#include "engine/channelhandle.h"
// Xlibint.h predates C++ and defines macros which conflict
// with references to std::max and std::min
#undef max
#undef min
#endif

namespace {
const mixxx::Logger kLogger("CoreServices");
constexpr int kMicrophoneCount = 4;
constexpr int kAuxiliaryCount = 4;
constexpr int kSamplerCount = 4;

#define CLEAR_AND_CHECK_DELETED(x) clearHelper(x, #x);

template<typename T>
void clearHelper(std::shared_ptr<T>& ref_ptr, const char* name) {
    std::weak_ptr<T> weak(ref_ptr);
    ref_ptr.reset();
    if (auto shared = weak.lock()) {
        qWarning() << name << "was leaked! Use count:" << shared.use_count();
        DEBUG_ASSERT(false);
    }
}

// hack around https://gitlab.freedesktop.org/xorg/lib/libx11/issues/25
// https://github.com/mixxxdj/mixxx/issues/9533
#if defined(Q_OS_LINUX) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
typedef Bool (*WireToErrorType)(Display*, XErrorEvent*, xError*);

constexpr int NUM_HANDLERS = 256;
WireToErrorType __oldHandlers[NUM_HANDLERS] = {nullptr};

Bool __xErrorHandler(Display* display, XErrorEvent* event, xError* error) {
    // Call any previous handler first in case it needs to do real work.
    auto code = static_cast<int>(event->error_code);
    if (__oldHandlers[code] != nullptr) {
        __oldHandlers[code](display, event, error);
    }

    // Always return false so the error does not get passed to the normal
    // application defined handler.
    return False;
}

#endif

inline QLocale inputLocale() {
    // Use the default config for local keyboard
    QInputMethod* pInputMethod = QGuiApplication::inputMethod();
    return pInputMethod ? pInputMethod->locale() : QLocale(QLocale::English);
}
} // anonymous namespace

namespace mixxx {

CoreServices::CoreServices(const CmdlineArgs& args, QApplication* pApp)
        : m_runtime_timer(QLatin1String("CoreServices::runtime")),
          m_cmdlineArgs(args),
          m_isInitialized(false) {
    m_runtime_timer.start();
    mixxx::Time::start();
    ScopedTimer t(QStringLiteral("CoreServices::CoreServices"));
    // All this here is running without without start up screen
    // Defer long initializations to CoreServices::initialize() which is
    // called after the GUI is initialized
    initializeSettings();
    initializeLogging();

    BackUpSettings* backUp = new BackUpSettings(m_pSettingsManager->settings());
    backUp->createSettingsBackUp();

    // createSettingsBackUp(m_pSettingsManager->settings())
    // createSettingsBackUp(m_pSettingsManager->settings());
    //  Only record stats in developer mode.
    if (m_cmdlineArgs.getDeveloper()) {
        StatsManager::createInstance();
    }
    mixxx::Translations::initializeTranslations(
            m_pSettingsManager->settings(), pApp, m_cmdlineArgs.getLocale());
    initializeKeyboard();
}

CoreServices::~CoreServices() {
    if (m_isInitialized) {
        finalize();
    }

    // Tear down remaining stuff that was initialized in the constructor.
    CLEAR_AND_CHECK_DELETED(m_pKeyboardEventFilter);
    CLEAR_AND_CHECK_DELETED(m_pKbdConfig);
    CLEAR_AND_CHECK_DELETED(m_pKbdConfigEmpty);

    if (m_cmdlineArgs.getDeveloper()) {
        StatsManager::destroy();
    }

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pSettingsManager->save();
    m_pSettingsManager.reset();

    Sandbox::shutdown();

    // Check for leaked ControlObjects and give warnings.
    {
        const QList<QSharedPointer<ControlDoublePrivate>> leakedControls =
                ControlDoublePrivate::takeAllInstances();
        if (!leakedControls.isEmpty()) {
            qWarning()
                    << "The following"
                    << leakedControls.size()
                    << "controls were leaked:";
            for (auto pCDP : leakedControls) {
                ConfigKey key = pCDP->getKey();
                qWarning() << key.group << key.item << pCDP->getCreatorCO();
                // Deleting leaked objects helps to satisfy valgrind.
                // These delete calls could cause crashes if a destructor for a control
                // we thought was leaked is triggered after this one exits.
                // So, only delete so if developer mode is on.
                if (CmdlineArgs::Instance().getDeveloper()) {
                    pCDP->deleteCreatorCO();
                }
            }
            DEBUG_ASSERT(!"Controls were leaked!");
        }
        // Finally drop all shared pointers by exiting this scope
    }

    // Report the total time we have been running.
    m_runtime_timer.elapsed(true);
}

void CoreServices::initializeSettings() {
#ifdef Q_OS_MACOS
    // TODO: At this point it is too late to provide the same settings path to all components
    // and too early to log errors and give users advises in their system language.
    // Calling this from main.cpp before the QApplication is initialized may cause a crash
    // due to potential QMessageBox invocations within migrateOldSettings().
    // Solution: Start Mixxx with default settings, migrate the preferences, and then restart
    // immediately.
    if (!m_cmdlineArgs.getSettingsPathSet()) {
        CmdlineArgs::Instance().setSettingsPath(Sandbox::migrateOldSettings());
    }
#endif
    QString settingsPath = m_cmdlineArgs.getSettingsPath();
    m_pSettingsManager = std::make_unique<SettingsManager>(settingsPath);
}

void CoreServices::initializeLogging() {
    mixxx::LogFlags logFlags = mixxx::LogFlag::LogToFile;
    if (m_cmdlineArgs.getDebugAssertBreak()) {
        logFlags.setFlag(mixxx::LogFlag::DebugAssertBreak);
    }
    mixxx::Logging::initialize(
            m_pSettingsManager->settings()->getSettingsPath(),
            m_cmdlineArgs.getLogLevel(),
            m_cmdlineArgs.getLogFlushLevel(),
            logFlags);
}

void CoreServices::initialize(QApplication* pApp) {
    VERIFY_OR_DEBUG_ASSERT(!m_isInitialized) {
        return;
    }

    ScopedTimer t(QStringLiteral("CoreServices::initialize"));

    VERIFY_OR_DEBUG_ASSERT(SoundSourceProxy::registerProviders()) {
        qCritical() << "Failed to register any SoundSource providers";
        return;
    }

    VersionStore::logBuildDetails();

#if defined(Q_OS_LINUX) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // XESetWireToError will segfault if running as a Wayland client
    if (pApp->platformName() == QLatin1String("xcb")) {
        for (auto i = 0; i < NUM_HANDLERS; ++i) {
            XESetWireToError(QX11Info::display(), i, &__xErrorHandler);
        }
    }
#else
    Q_UNUSED(pApp);
#endif

    UserSettingsPointer pConfig = m_pSettingsManager->settings();

    Sandbox::setPermissionsFilePath(QDir(pConfig->getSettingsPath()).filePath("sandbox.cfg"));

    QString resourcePath = pConfig->getResourcePath();

    emit initializationProgressUpdate(0, tr("fonts"));

    FontUtils::initializeFonts(resourcePath); // takes a long time

    emit initializationProgressUpdate(10, tr("database"));
    m_pDbConnectionPool = MixxxDb(pConfig).connectionPool();
    if (!m_pDbConnectionPool) {
        exit(-1);
    }
    // Create a connection for the main thread
    m_pDbConnectionPool->createThreadLocalConnection();
    if (!initializeDatabase()) {
        exit(-1);
    }

    m_pControlIndicatorTimer = std::make_shared<mixxx::ControlIndicatorTimer>(this);

    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();

    emit initializationProgressUpdate(20, tr("effects"));
    m_pEffectsManager = std::make_shared<EffectsManager>(pConfig, pChannelHandleFactory);

    m_pEngine = std::make_shared<EngineMixer>(
            pConfig,
            "[Master]",
            m_pEffectsManager.get(),
            pChannelHandleFactory,
            true);
#ifdef __RUBBERBAND__
    RubberBandWorkerPool::createInstance(pConfig);
#endif

    emit initializationProgressUpdate(30, tr("audio interface"));
    // Although m_pSoundManager is created here, m_pSoundManager->setupDevices()
    // needs to be called after m_pPlayerManager registers sound IO for each EngineChannel.
    m_pSoundManager = std::make_shared<SoundManager>(pConfig, m_pEngine.get());
    m_pEngine->registerNonEngineChannelSoundIO(gsl::make_not_null(m_pSoundManager.get()));

    m_pRecordingManager = std::make_shared<RecordingManager>(pConfig, m_pEngine.get());

#ifdef __BROADCAST__
    m_pBroadcastManager = std::make_shared<BroadcastManager>(
            m_pSettingsManager.get(),
            m_pSoundManager.get());
#endif

#ifdef __VINYLCONTROL__
    m_pVCManager = std::make_shared<VinylControlManager>(this, pConfig, m_pSoundManager.get());
#else
    m_pVCManager = nullptr;
#endif

    emit initializationProgressUpdate(40, tr("decks"));
    // Create the player manager. (long)
    m_pPlayerManager = std::make_shared<PlayerManager>(
            pConfig,
            m_pSoundManager.get(),
            m_pEffectsManager.get(),
            m_pEngine.get());
    // TODO: connect input not configured error dialog slots
    PlayerInfo::create();

    for (int i = 0; i < kMicrophoneCount; ++i) {
        m_pPlayerManager->addMicrophone();
    }

    for (int i = 0; i < kAuxiliaryCount; ++i) {
        m_pPlayerManager->addAuxiliary();
    }

    m_pPlayerManager->addConfiguredDecks();

    for (int i = 0; i < kSamplerCount; ++i) {
        m_pPlayerManager->addSampler();
    }

    m_pPlayerManager->addPreviewDeck();

    m_pEffectsManager->setup();

#ifdef __VINYLCONTROL__
    m_pVCManager->init();
#endif

#ifdef __MODPLUG__
    // Restore the configuration for the modplug library before trying to load a module.
    DlgPrefModplug modplugPrefs{nullptr, pConfig};
    modplugPrefs.loadSettings();
    modplugPrefs.applySettings();
#endif

    // Inhibit Screensaver
    m_pScreensaverManager = std::make_shared<ScreensaverManager>(pConfig);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingDeckChanged,
            m_pScreensaverManager.get(),
            &ScreensaverManager::slotCurrentPlayingDeckChanged);

    emit initializationProgressUpdate(50, tr("library"));
    CoverArtCache::createInstance();
    Clipboard::createInstance();

    m_pTrackCollectionManager = std::make_shared<TrackCollectionManager>(
            this,
            pConfig,
            m_pDbConnectionPool);

    m_pLibrary = std::make_shared<Library>(
            this,
            pConfig,
            m_pDbConnectionPool,
            m_pTrackCollectionManager.get(),
            m_pPlayerManager.get(),
            m_pRecordingManager.get());

    OverviewCache* pOverviewCache = OverviewCache::createInstance(pConfig, m_pDbConnectionPool);
    connect(&(m_pTrackCollectionManager->internalCollection()->getTrackDAO()),
            &TrackDAO::waveformSummaryUpdated,
            pOverviewCache,
            &OverviewCache::onTrackSummaryChanged);

    // Binding the PlayManager to the Library may already trigger
    // loading of tracks which requires that the GlobalTrackCache has
    // been created. Otherwise Mixxx might hang when accessing
    // the uninitialized singleton instance!
    m_pPlayerManager->bindToLibrary(m_pLibrary.get());

    bool musicDirAdded = false;

    if (m_pTrackCollectionManager->internalCollection()->loadRootDirs().isEmpty()) {
#if defined(Q_OS_IOS) || defined(Q_OS_WASM)
        // On the web and iOS, we are running in a sandbox (a virtual file
        // system on the web). Since we are generally limited to paths within
        // the sandbox, there is not much point in asking the user about a
        // custom directory, so we just default to Qt's standard music directory
        // (~/Documents/Music on iOS and ~/Music on Wasm). Since the sandbox is
        // initially empty, we create that directory automatically.
        // Advanced users can still customize this directory in the settings.
        QString fd = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        QDir dir = fd;
        if (!dir.exists()) {
            dir.mkpath(".");
        }
#else
        // TODO(XXX) this needs to be smarter, we can't distinguish between an empty
        // path return value (not sure if this is normally possible, but it is
        // possible with the Windows 7 "Music" library, which is what
        // QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
        // resolves to) and a user hitting 'cancel'. If we get a blank return
        // but the user didn't hit cancel, we need to know this and let the
        // user take some course of action -- bkgood
        QString fd = QFileDialog::getExistingDirectory(nullptr,
                tr("Choose music library directory"),
                QStandardPaths::writableLocation(
                        QStandardPaths::MusicLocation));
#endif
        // request to add directory to database.
        if (!fd.isEmpty() && m_pLibrary->requestAddDir(fd)) {
            musicDirAdded = true;
        }
    }

    emit initializationProgressUpdate(60, tr("controllers"));
    // Initialize controller sub-system,
    // but do not set up controllers until the end of the application startup
    // (long)
    qDebug() << "Creating ControllerManager";
    m_pControllerManager = std::make_shared<ControllerManager>(pConfig);

    // Scan the library for new files and directories
    bool rescan = m_cmdlineArgs.getRescanLibrary() ||
            pConfig->getValue<bool>(library::prefs::kRescanOnStartupConfigKey);
    // rescan the library if we get a new plugin
    QList<QString> prev_plugins_list =
            pConfig->getValueString(
                           ConfigKey("[Library]", "SupportedFileExtensions"))
                    .split(',',
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                            Qt::SkipEmptyParts);
#else
                            QString::SkipEmptyParts);
#endif

    // TODO: QSet<T>::fromList(const QList<T>&) is deprecated and should be
    // replaced with QSet<T>(list.begin(), list.end()).
    // However, the proposed alternative has just been introduced in Qt
    // 5.14. Until the minimum required Qt version of Mixxx is increased,
    // we need a version check here
    QSet<QString> prev_plugins =
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QSet<QString>(prev_plugins_list.begin(), prev_plugins_list.end());
#else
            QSet<QString>::fromList(prev_plugins_list);
#endif

    const QList<QString> supportedFileSuffixes = SoundSourceProxy::getSupportedFileSuffixes();
    auto curr_plugins =
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QSet<QString>(supportedFileSuffixes.begin(), supportedFileSuffixes.end());
#else
            QSet<QString>::fromList(supportedFileSuffixes);
#endif

    rescan = rescan || (prev_plugins != curr_plugins);
    pConfig->set(ConfigKey("[Library]", "SupportedFileExtensions"),
            supportedFileSuffixes.join(","));

    // Forward the scanner signal so MixxxMainWindow can display a summary popup.
    connect(m_pTrackCollectionManager.get(),
            &TrackCollectionManager::libraryScanSummary,
            this,
            &CoreServices::libraryScanSummary,
            Qt::UniqueConnection);
    // Scan the library directory. Do this after the skinloader has
    // loaded a skin, see issue #6625
    if (rescan || musicDirAdded || m_pSettingsManager->shouldRescanLibrary()) {
        m_pTrackCollectionManager->startLibraryAutoScan();
    }

    // This has to be done before m_pSoundManager->setupDevices()
    // https://github.com/mixxxdj/mixxx/issues/9188
    m_pPlayerManager->loadSamplers();

    m_pTouchShift = std::make_unique<ControlPushButton>(ConfigKey("[Controls]", "touch_shift"));

    // The UI controls must be created here so that controllers can bind to
    // them on startup.
    m_pSkinControls = std::make_unique<SkinControls>();

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    const QList<QString>& musicFiles = m_cmdlineArgs.getMusicFiles();
    for (int i = 0; i < (int)m_pPlayerManager->numDecks() && i < musicFiles.count(); ++i) {
        if (SoundSourceProxy::isFileNameSupported(musicFiles.at(i))) {
            m_pPlayerManager->slotLoadToDeck(musicFiles.at(i), i + 1);
        }
    }

    m_isInitialized = true;

#ifdef MIXXX_USE_QML
    initializeQMLSingletons();
}

void CoreServices::initializeQMLSingletons() {
    // Any uncreateable non-singleton types registered here require
    // arguments that we don't want to expose to QML directly. Instead, they
    // can be retrieved by member properties or methods from the singleton
    // types.
    //
    // The alternative would be to register their *arguments* in the QML
    // system, which would improve nothing, or we had to expose them as
    // singletons to that they can be accessed by components instantiated by
    // QML, which would also be suboptimal.
    mixxx::qml::QmlEffectsManagerProxy::registerEffectsManager(getEffectsManager());
    mixxx::qml::QmlPlayerManagerProxy::registerPlayerManager(getPlayerManager());
    mixxx::qml::QmlConfigProxy::registerUserSettings(getSettings());
    mixxx::qml::QmlLibraryProxy::registerLibrary(getLibrary());

    ControllerScriptEngineBase::registerTrackCollectionManager(getTrackCollectionManager());

    // Currently, it is required to enforce QQuickWindow RHI backend to use
    // OpenGL on all platforms to allow offscreen rendering to function as
    // expected
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif
}

void CoreServices::initializeKeyboard() {
    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    QString resourcePath = pConfig->getResourcePath();

    // Set the default value in settings file
    if (pConfig->getValueString(ConfigKey("[Keyboard]", "Enabled")).length() == 0) {
        pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(1));
    }

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard = QDir(pConfig->getSettingsPath()).filePath("Custom.kbd.cfg");

    // Empty keyboard configuration
    m_pKbdConfigEmpty = std::make_shared<ConfigObject<ConfigValueKbd>>(QString());

    if (QFile::exists(userKeyboard)) {
        qDebug() << "Found and will use custom keyboard mapping" << userKeyboard;
        m_pKbdConfig = std::make_shared<ConfigObject<ConfigValueKbd>>(userKeyboard);
    } else {
        // Default to the locale for the main input method (e.g. keyboard).
        QLocale locale = inputLocale();

        // check if a default keyboard exists
        QString defaultKeyboard = QString(resourcePath).append("keyboard/");
        defaultKeyboard += locale.name();
        defaultKeyboard += ".kbd.cfg";
        qDebug() << "Found and will use default keyboard mapping" << defaultKeyboard;

        if (!QFile::exists(defaultKeyboard)) {
            qDebug() << defaultKeyboard << " not found, using en_US.kbd.cfg";
            defaultKeyboard = QString(resourcePath).append("keyboard/").append("en_US.kbd.cfg");
            if (!QFile::exists(defaultKeyboard)) {
                qDebug() << defaultKeyboard << " not found, starting without shortcuts";
                defaultKeyboard = "";
            }
        }
        m_pKbdConfig = std::make_shared<ConfigObject<ConfigValueKbd>>(defaultKeyboard);
    }

    // TODO(XXX) leak pKbdConfig, KeyboardEventFilter owns it? Maybe roll all keyboard
    // initialization into KeyboardEventFilter
    // Workaround for today: KeyboardEventFilter calls delete
    bool keyboardShortcutsEnabled = pConfig->getValue<bool>(
            ConfigKey("[Keyboard]", "Enabled"));
    m_pKeyboardEventFilter = std::make_shared<KeyboardEventFilter>(
            keyboardShortcutsEnabled ? m_pKbdConfig.get() : m_pKbdConfigEmpty.get());
}

void CoreServices::slotOptionsKeyboard(bool toggle) {
    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    if (toggle) {
        //qDebug() << "Enable keyboard shortcuts/mappings";
        m_pKeyboardEventFilter->setKeyboardConfig(m_pKbdConfig.get());
        pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(1));
    } else {
        //qDebug() << "Disable keyboard shortcuts/mappings";
        m_pKeyboardEventFilter->setKeyboardConfig(m_pKbdConfigEmpty.get());
        pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(0));
    }
}

bool CoreServices::initializeDatabase() {
    kLogger.info() << "Connecting to database";
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
    if (!dbConnection.isOpen()) {
        QMessageBox::critical(nullptr,
                tr("Cannot open database"),
                tr("Unable to establish a database connection.\n"
                   "Mixxx requires QT with SQLite support. Please read "
                   "the Qt SQL driver documentation for information on how "
                   "to build it.\n\n"
                   "Click OK to exit."),
                QMessageBox::Ok);
        return false;
    }

    kLogger.info() << "Initializing or upgrading database schema";
    return MixxxDb::initDatabaseSchema(dbConnection);
}

std::shared_ptr<QDialog> CoreServices::makeDlgPreferences() const {
    // Note: We return here the base class pointer to make the coreservices.h usable
    // in test classes where header included from dlgpreferences.h are not accessible.
    std::shared_ptr<DlgPreferences> pDlgPreferences = std::make_shared<DlgPreferences>(
            getScreensaverManager(),
            nullptr,
            getSoundManager(),
            getControllerManager(),
            getVinylControlManager(),
            getEffectsManager(),
            getSettingsManager(),
            getLibrary());
    return pDlgPreferences;
}

void CoreServices::finalize() {
    VERIFY_OR_DEBUG_ASSERT(m_isInitialized) {
        qDebug() << "Skipping CoreServices finalization because it was never initialized.";
        return;
    }

    Timer t("CoreServices::~CoreServices");
    t.start();

#ifdef MIXXX_USE_QML
    // Delete all the QML singletons in order to prevent controller leaks
    mixxx::qml::QmlEffectsManagerProxy::registerEffectsManager(nullptr);
    mixxx::qml::QmlPlayerManagerProxy::registerPlayerManager(nullptr);
    mixxx::qml::QmlConfigProxy::registerUserSettings(nullptr);
    mixxx::qml::QmlLibraryProxy::registerLibrary(nullptr);

    ControllerScriptEngineBase::registerTrackCollectionManager(nullptr);
#endif

    // Stop all pending library operations
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "stopping pending Library tasks";
    m_pTrackCollectionManager->stopLibraryScan();
    m_pLibrary->stopPendingTasks();

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "saving configuration";
    m_pSettingsManager->save();

    // SoundManager depend on Engine and Config
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting SoundManager";
    CLEAR_AND_CHECK_DELETED(m_pSoundManager);

    // ControllerManager depends on Config
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting ControllerManager";
    CLEAR_AND_CHECK_DELETED(m_pControllerManager);

#ifdef __VINYLCONTROL__
    // VinylControlManager depends on a CO the engine owns
    // (vinylcontrol_enabled in VinylControlControl)
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting VinylControlManager";
    CLEAR_AND_CHECK_DELETED(m_pVCManager);
#endif

    // CoverArtCache is fairly independent of everything else.
    CoverArtCache::destroy();

    Clipboard::destroy();

    // PlayerManager depends on Engine, SoundManager, VinylControlManager, and Config
    // The player manager has to be deleted before the library to ensure
    // that all modified track metadata of loaded tracks is saved.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting PlayerManager";
    CLEAR_AND_CHECK_DELETED(m_pPlayerManager);

    // Delete the library after the view so there are no dangling pointers to
    // the data models.
    // Depends on RecordingManager and PlayerManager
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting Library";
    CLEAR_AND_CHECK_DELETED(m_pLibrary);

    // RecordingManager depends on config, engine
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting RecordingManager";
    CLEAR_AND_CHECK_DELETED(m_pRecordingManager);

#ifdef __BROADCAST__
    // BroadcastManager depends on config, engine
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting BroadcastManager";
    CLEAR_AND_CHECK_DELETED(m_pBroadcastManager);
#endif

    // EngineMixer depends on Config and m_pEffectsManager.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting EngineMixer";
    CLEAR_AND_CHECK_DELETED(m_pEngine);
#ifdef __RUBBERBAND__
    RubberBandWorkerPool::destroy();
#endif

    // Destroy PlayerInfo explicitly to release the track
    // pointers of tracks that were still loaded in decks
    // or samplers when PlayerManager was destroyed!
    // Do this after deleting EngineMixer which makes use of
    // PlayerInfo in EngineRecord.
    PlayerInfo::destroy();

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting EffectsManager";
    CLEAR_AND_CHECK_DELETED(m_pEffectsManager);

    // Delete the track collections after all internal track pointers
    // in other components have been released by deleting those components
    // beforehand!
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "detaching all track collections";
    CLEAR_AND_CHECK_DELETED(m_pTrackCollectionManager);

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "closing database connection(s)";
    m_pDbConnectionPool->destroyThreadLocalConnection();
    m_pDbConnectionPool.reset(); // should drop the last reference

    m_pTouchShift.reset();

    m_pSkinControls.reset();

    m_pControlIndicatorTimer.reset();

    t.elapsed(true);
}

} // namespace mixxx
