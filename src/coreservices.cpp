#include "coreservices.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>

#ifdef __BROADCAST__
#include "broadcast/broadcastmanager.h"
#endif
#include "controllers/controllermanager.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "database/mixxxdb.h"
#include "effects/builtin/builtinbackend.h"
#include "effects/effectsmanager.h"
#ifdef __LILV__
#include "effects/lv2/lv2backend.h"
#endif
#include "engine/enginemaster.h"
#include "library/coverartcache.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_coreservices.cpp"
#include "preferences/settingsmanager.h"
#include "soundio/soundmanager.h"
#include "sources/soundsourceproxy.h"
#include "util/db/dbconnectionpooled.h"
#include "util/font.h"
#include "util/logger.h"
#include "util/screensaver.h"
#include "util/statsmanager.h"
#include "util/time.h"
#include "util/translations.h"
#include "util/version.h"
#include "vinylcontrol/vinylcontrolmanager.h"

#ifdef __APPLE__
#include "util/sandbox.h"
#endif

#if defined(Q_OS_LINUX)
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
// https://bugs.launchpad.net/mixxx/+bug/1805559
#if defined(Q_OS_LINUX)
typedef Bool (*WireToErrorType)(Display*, XErrorEvent*, xError*);

const int NUM_HANDLERS = 256;
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

CoreServices::CoreServices(const CmdlineArgs& args)
        : m_runtime_timer(QLatin1String("CoreServices::runtime")),
          m_cmdlineArgs(args) {
}

void CoreServices::initializeSettings() {
    QString settingsPath = m_cmdlineArgs.getSettingsPath();
#ifdef __APPLE__
    Sandbox::checkSandboxed();
    if (!m_cmdlineArgs.getSettingsPathSet()) {
        settingsPath = Sandbox::migrateOldSettings();
    }
#endif
    m_pSettingsManager = std::make_unique<SettingsManager>(settingsPath);
}

void CoreServices::initialize(QApplication* pApp) {
    m_runtime_timer.start();
    mixxx::Time::start();
    ScopedTimer t("CoreServices::initialize");

    mixxx::LogFlags logFlags = mixxx::LogFlag::LogToFile;
    if (m_cmdlineArgs.getDebugAssertBreak()) {
        logFlags.setFlag(mixxx::LogFlag::DebugAssertBreak);
    }
    mixxx::Logging::initialize(
            m_pSettingsManager->settings()->getSettingsPath(),
            m_cmdlineArgs.getLogLevel(),
            m_cmdlineArgs.getLogFlushLevel(),
            logFlags);

    VERIFY_OR_DEBUG_ASSERT(SoundSourceProxy::registerProviders()) {
        qCritical() << "Failed to register any SoundSource providers";
        return;
    }

    Version::logBuildDetails();

    // Only record stats in developer mode.
    if (m_cmdlineArgs.getDeveloper()) {
        StatsManager::createInstance();
    }

    initializeKeyboard();

    mixxx::Translations::initializeTranslations(
            m_pSettingsManager->settings(), pApp, m_cmdlineArgs.getLocale());

#if defined(Q_OS_LINUX)
    // XESetWireToError will segfault if running as a Wayland client
    if (pApp->platformName() == QLatin1String("xcb")) {
        for (auto i = 0; i < NUM_HANDLERS; ++i) {
            XESetWireToError(QX11Info::display(), i, &__xErrorHandler);
        }
    }
#endif

    UserSettingsPointer pConfig = m_pSettingsManager->settings();

    Sandbox::setPermissionsFilePath(QDir(pConfig->getSettingsPath()).filePath("sandbox.cfg"));

    QString resourcePath = pConfig->getResourcePath();

    emit initializationProgressUpdate(0, tr("fonts"));

    FontUtils::initializeFonts(resourcePath); // takes a long time

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = static_cast<mixxx::TooltipsPreference>(
            pConfig->getValue(ConfigKey("[Controls]", "Tooltips"),
                    static_cast<int>(mixxx::TooltipsPreference::TOOLTIPS_ON)));

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

    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();

    emit initializationProgressUpdate(20, tr("effects"));
    m_pEffectsManager = std::make_shared<EffectsManager>(this, pConfig, pChannelHandleFactory);

    m_pEngine = std::make_shared<EngineMaster>(
            pConfig,
            "[Master]",
            m_pEffectsManager.get(),
            pChannelHandleFactory,
            true);

    // Create effect backends. We do this after creating EngineMaster to allow
    // effect backends to refer to controls that are produced by the engine.
    BuiltInBackend* pBuiltInBackend = new BuiltInBackend(m_pEffectsManager.get());
    m_pEffectsManager->addEffectsBackend(pBuiltInBackend);
#ifdef __LILV__
    m_pLV2Backend = new LV2Backend(m_pEffectsManager.get());
    // EffectsManager takes ownership
    m_pEffectsManager->addEffectsBackend(m_pLV2Backend);
#else
    m_pLV2Backend = nullptr;
#endif

    m_pEffectsManager->setup();

    emit initializationProgressUpdate(30, tr("audio interface"));
    // Although m_pSoundManager is created here, m_pSoundManager->setupDevices()
    // needs to be called after m_pPlayerManager registers sound IO for each EngineChannel.
    m_pSoundManager = std::make_shared<SoundManager>(pConfig, m_pEngine.get());
    m_pEngine->registerNonEngineChannelSoundIO(m_pSoundManager.get());

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
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addPreviewDeck();

    m_pEffectsManager->loadEffectChains();

#ifdef __VINYLCONTROL__
    m_pVCManager->init();
#endif

    emit initializationProgressUpdate(50, tr("library"));
    CoverArtCache::createInstance();

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

    // Binding the PlayManager to the Library may already trigger
    // loading of tracks which requires that the GlobalTrackCache has
    // been created. Otherwise Mixxx might hang when accessing
    // the uninitialized singleton instance!
    m_pPlayerManager->bindToLibrary(m_pLibrary.get());

    bool hasChanged_MusicDir = false;

    QStringList dirs = m_pLibrary->getDirs();
    if (dirs.size() < 1) {
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
        if (!fd.isEmpty()) {
            // adds Folder to database.
            m_pLibrary->slotRequestAddDir(fd);
            hasChanged_MusicDir = true;
        }
    }

    emit initializationProgressUpdate(60, tr("controllers"));
    // Initialize controller sub-system,
    // but do not set up controllers until the end of the application startup
    // (long)
    qDebug() << "Creating ControllerManager";
    m_pControllerManager = std::make_shared<ControllerManager>(pConfig);

    // Inhibit the screensaver if the option is set. (Do it before creating the preferences dialog)
    int inhibit = pConfig->getValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), -1);
    if (inhibit == -1) {
        inhibit = static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON);
        pConfig->setValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), inhibit);
    }
    m_inhibitScreensaver = static_cast<mixxx::ScreenSaverPreference>(inhibit);
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    }

    // Wait until all other ControlObjects are set up before initializing
    // controllers
    m_pControllerManager->setUpDevices();

    // Scan the library for new files and directories
    bool rescan = pConfig->getValue<bool>(
            ConfigKey("[Library]", "RescanOnStartup"));
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

    const QList<QString> curr_plugins_list = SoundSourceProxy::getSupportedFileExtensions();
    QSet<QString> curr_plugins =
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QSet<QString>(curr_plugins_list.begin(), curr_plugins_list.end());
#else
            QSet<QString>::fromList(curr_plugins_list);
#endif

    rescan = rescan || (prev_plugins != curr_plugins);
    pConfig->set(ConfigKey("[Library]", "SupportedFileExtensions"), curr_plugins_list.join(","));

    // Scan the library directory. Do this after the skinloader has
    // loaded a skin, see Bug #1047435
    if (rescan || hasChanged_MusicDir || m_pSettingsManager->shouldRescanLibrary()) {
        m_pTrackCollectionManager->startLibraryScan();
    }

    // This has to be done before m_pSoundManager->setupDevices()
    // https://bugs.launchpad.net/mixxx/+bug/1758189
    m_pPlayerManager->loadSamplers();

    m_pTouchShift = std::make_unique<ControlPushButton>(ConfigKey("[Controls]", "touch_shift"));

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    const QList<QString>& musicFiles = m_cmdlineArgs.getMusicFiles();
    for (int i = 0; i < (int)m_pPlayerManager->numDecks() && i < musicFiles.count(); ++i) {
        if (SoundSourceProxy::isFileNameSupported(musicFiles.at(i))) {
            m_pPlayerManager->slotLoadToDeck(musicFiles.at(i), i + 1);
        }
    }
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

void CoreServices::shutdown() {
    Timer t("CoreServices::shutdown");
    t.start();

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

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

    // PlayerManager depends on Engine, SoundManager, VinylControlManager, and Config
    // The player manager has to be deleted before the library to ensure
    // that all modified track metadata of loaded tracks is saved.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting PlayerManager";
    CLEAR_AND_CHECK_DELETED(m_pPlayerManager);

    // Destroy PlayerInfo explicitly to release the track
    // pointers of tracks that were still loaded in decks
    // or samplers when PlayerManager was destroyed!
    PlayerInfo::destroy();

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

    // EngineMaster depends on Config and m_pEffectsManager.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting EngineMaster";
    CLEAR_AND_CHECK_DELETED(m_pEngine);

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

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pSettingsManager->save();

    m_pTouchShift.reset();

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

    Sandbox::shutdown();

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting SettingsManager";
    m_pSettingsManager.reset();

    CLEAR_AND_CHECK_DELETED(m_pKeyboardEventFilter);
    CLEAR_AND_CHECK_DELETED(m_pKbdConfig);
    CLEAR_AND_CHECK_DELETED(m_pKbdConfigEmpty);

    t.elapsed(true);
    // Report the total time we have been running.
    m_runtime_timer.elapsed(true);

    if (m_cmdlineArgs.getDeveloper()) {
        StatsManager::destroy();
    }
}

} // namespace mixxx
