#include "mixxx.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QGLFormat>
#include <QGuiApplication>
#include <QInputMethod>
#include <QLocale>
#include <QScreen>
#include <QStandardPaths>
#include <QUrl>
#include <QtDebug>

#include "dialog/dlgabout.h"
#include "dialog/dlgdevelopertools.h"
#include "effects/builtin/builtinbackend.h"
#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "preferences/constants.h"
#include "preferences/dialog/dlgprefeq.h"
#include "preferences/dialog/dlgpreferences.h"
#ifdef __LILV__
#include "effects/lv2/lv2backend.h"
#endif
#include "broadcast/broadcastmanager.h"
#include "control/controlpushbutton.h"
#include "controllers/controllermanager.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "database/mixxxdb.h"
#include "library/coverartcache.h"
#include "library/library.h"
#include "library/library_preferences.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "preferences/settingsmanager.h"
#include "recording/recordingmanager.h"
#include "skin/launchimage.h"
#include "skin/legacyskinparser.h"
#include "skin/skinloader.h"
#include "soundio/soundmanager.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/db/dbconnectionpooled.h"
#include "util/debug.h"
#include "util/experiment.h"
#include "util/font.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/sandbox.h"
#include "util/screensaver.h"
#include "util/statsmanager.h"
#include "util/time.h"
#include "util/timer.h"
#include "util/translations.h"
#include "util/version.h"
#include "util/widgethelper.h"
#include "waveform/guitick.h"
#include "waveform/sharedglcontext.h"
#include "waveform/visualsmanager.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wmainmenubar.h"

#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrolmanager.h"
#endif

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif

#if defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

#include <QtX11Extras/QX11Info>
// Xlibint.h predates C++ and defines macros which conflict
// with references to std::max and std::min
#undef max
#undef min
#endif

namespace {

const mixxx::Logger kLogger("MixxxMainWindow");

// hack around https://gitlab.freedesktop.org/xorg/lib/libx11/issues/25
// https://bugs.launchpad.net/mixxx/+bug/1805559
#if defined(Q_OS_LINUX)
typedef Bool (*WireToErrorType)(Display*, XErrorEvent*, xError*);

const int NUM_HANDLERS = 256;
WireToErrorType __oldHandlers[NUM_HANDLERS] = {0};

Bool __xErrorHandler(Display* display, XErrorEvent* event, xError* error) {
    // Call any previous handler first in case it needs to do real work.
    auto code = static_cast<int>(event->error_code);
    if (__oldHandlers[code] != NULL) {
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
    return pInputMethod ? pInputMethod->locale() :
            QLocale(QLocale::English);
}

} // anonymous namespace

// static
const int MixxxMainWindow::kMicrophoneCount = 4;
// static
const int MixxxMainWindow::kAuxiliaryCount = 4;

MixxxMainWindow::MixxxMainWindow(QApplication* pApp, const CmdlineArgs& args)
        : m_pCentralWidget(nullptr),
          m_pLaunchImage(nullptr),
          m_pEffectsManager(nullptr),
          m_pEngine(nullptr),
          m_pSkinLoader(nullptr),
          m_pSoundManager(nullptr),
          m_pPlayerManager(nullptr),
          m_pRecordingManager(nullptr),
#ifdef __BROADCAST__
          m_pBroadcastManager(nullptr),
#endif
          m_pControllerManager(nullptr),
          m_pGuiTick(nullptr),
#ifdef __VINYLCONTROL__
          m_pVCManager(nullptr),
#endif
          m_pKeyboard(nullptr),
          m_pLibrary(nullptr),
          m_pDeveloperToolsDlg(nullptr),
          m_pPrefDlg(nullptr),
          m_pKbdConfig(nullptr),
          m_pKbdConfigEmpty(nullptr),
          m_toolTipsCfg(mixxx::TooltipsPreference::TOOLTIPS_ON),
          m_runtime_timer("MixxxMainWindow::runtime"),
          m_cmdLineArgs(args),
          m_pTouchShift(nullptr) {
    m_runtime_timer.start();
    mixxx::Time::start();

    Version::logBuildDetails();

    // Only record stats in developer mode.
    if (m_cmdLineArgs.getDeveloper()) {
        StatsManager::createInstance();
    }

    m_pSettingsManager = std::make_unique<SettingsManager>(args.getSettingsPath());

    initializeKeyboard();
    installEventFilter(m_pKeyboard);

    // Menubar depends on translations.
    mixxx::Translations::initializeTranslations(
        m_pSettingsManager->settings(), pApp, args.getLocale());

    createMenuBar();

    initializeWindow();

    // First load launch image to show a the user a quick responds
    m_pSkinLoader = new SkinLoader(m_pSettingsManager->settings());
    m_pLaunchImage = m_pSkinLoader->loadLaunchImage(this);
    m_pCentralWidget = (QWidget*)m_pLaunchImage;
    setCentralWidget(m_pCentralWidget);

    show();
    pApp->processEvents();

    initialize(pApp, args);
}

MixxxMainWindow::~MixxxMainWindow() {
    finalize();
    // SkinLoader depends on Config;
    delete m_pSkinLoader;
}

void MixxxMainWindow::initialize(QApplication* pApp, const CmdlineArgs& args) {
    ScopedTimer t("MixxxMainWindow::initialize");

#if defined(Q_OS_LINUX)
    // XESetWireToError will segfault if running as a Wayland client
    if (pApp->platformName() == QStringLiteral("xcb")) {
        for (auto i = 0; i < NUM_HANDLERS; ++i) {
            XESetWireToError(QX11Info::display(), i, &__xErrorHandler);
        }
    }
#endif

    UserSettingsPointer pConfig = m_pSettingsManager->settings();

    Sandbox::initialize(QDir(pConfig->getSettingsPath()).filePath("sandbox.cfg"));

    QString resourcePath = pConfig->getResourcePath();

    FontUtils::initializeFonts(resourcePath); // takes a long time

    launchProgress(2);

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = static_cast<mixxx::TooltipsPreference>(
        pConfig->getValue(ConfigKey("[Controls]", "Tooltips"),
                static_cast<int>(mixxx::TooltipsPreference::TOOLTIPS_ON)));

    m_pTouchShift = new ControlPushButton(ConfigKey("[Controls]", "touch_shift"));

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

    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();

    // Create the Effects subsystem.
    m_pEffectsManager = new EffectsManager(this, pConfig, pChannelHandleFactory);

    // Starting the master (mixing of the channels and effects):
    m_pEngine = new EngineMaster(
            pConfig,
            "[Master]",
            m_pEffectsManager,
            pChannelHandleFactory,
            true);

    // Create effect backends. We do this after creating EngineMaster to allow
    // effect backends to refer to controls that are produced by the engine.
    BuiltInBackend* pBuiltInBackend = new BuiltInBackend(m_pEffectsManager);
    m_pEffectsManager->addEffectsBackend(pBuiltInBackend);
#ifdef __LILV__
    LV2Backend* pLV2Backend = new LV2Backend(m_pEffectsManager);
    m_pEffectsManager->addEffectsBackend(pLV2Backend);
#else
    LV2Backend* pLV2Backend = nullptr;
#endif

    // Sets up the EffectChains and EffectRacks (long)
    m_pEffectsManager->setup();

    launchProgress(8);

    // Although m_pSoundManager is created here, m_pSoundManager->setupDevices()
    // needs to be called after m_pPlayerManager registers sound IO for each EngineChannel.
    m_pSoundManager = new SoundManager(pConfig, m_pEngine);
    m_pEngine->registerNonEngineChannelSoundIO(m_pSoundManager);

    m_pRecordingManager = new RecordingManager(pConfig, m_pEngine);

#ifdef __BROADCAST__
    m_pBroadcastManager = new BroadcastManager(
            m_pSettingsManager.get(),
            m_pSoundManager);
#endif

    launchProgress(11);

    // Needs to be created before CueControl (decks) and WTrackTableView.
    m_pGuiTick = new GuiTick();
    m_pVisualsManager = new VisualsManager();

#ifdef __VINYLCONTROL__
    m_pVCManager = new VinylControlManager(this, pConfig, m_pSoundManager);
#else
    m_pVCManager = NULL;
#endif

    // Create the player manager. (long)
    m_pPlayerManager = new PlayerManager(pConfig, m_pSoundManager,
            m_pEffectsManager, m_pVisualsManager, m_pEngine);
    connect(m_pPlayerManager,
            &PlayerManager::noMicrophoneInputConfigured,
            this,
            &MixxxMainWindow::slotNoMicrophoneInputConfigured);
    connect(m_pPlayerManager,
            &PlayerManager::noAuxiliaryInputConfigured,
            this,
            &MixxxMainWindow::slotNoAuxiliaryInputConfigured);
    connect(m_pPlayerManager,
            &PlayerManager::noDeckPassthroughInputConfigured,
            this,
            &MixxxMainWindow::slotNoDeckPassthroughInputConfigured);
    connect(m_pPlayerManager,
            &PlayerManager::noVinylControlInputConfigured,
            this,
            &MixxxMainWindow::slotNoVinylControlInputConfigured);
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

    launchProgress(30);

    m_pEffectsManager->loadEffectChains();

#ifdef __VINYLCONTROL__
    m_pVCManager->init();
#endif

#ifdef __MODPLUG__
    // restore the configuration for the modplug library before trying to load a module
    DlgPrefModplug* pModplugPrefs = new DlgPrefModplug(0, pConfig);
    pModplugPrefs->loadSettings();
    pModplugPrefs->applySettings();
    delete pModplugPrefs; // not needed anymore
#endif

    CoverArtCache::createInstance();

    launchProgress(30);

    m_pTrackCollectionManager = new TrackCollectionManager(
            this,
            pConfig,
            m_pDbConnectionPool);

    launchProgress(35);

    m_pLibrary = new Library(
            this,
            pConfig,
            m_pDbConnectionPool,
            m_pTrackCollectionManager,
            m_pPlayerManager,
            m_pRecordingManager);

    // Binding the PlayManager to the Library may already trigger
    // loading of tracks which requires that the GlobalTrackCache has
    // been created. Otherwise Mixxx might hang when accessing
    // the uninitialized singleton instance!
    m_pPlayerManager->bindToLibrary(m_pLibrary);

    launchProgress(40);

    // Get Music dir
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
        QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose music library directory"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
        if (!fd.isEmpty()) {
            // adds Folder to database.
            m_pLibrary->slotRequestAddDir(fd);
            hasChanged_MusicDir = true;
        }
    }

    // Call inits to invoke all other construction parts

    // Initialize controller sub-system,
    // but do not set up controllers until the end of the application startup
    // (long)
    qDebug() << "Creating ControllerManager";
    m_pControllerManager = new ControllerManager(pConfig);

    launchProgress(47);

    // Before creating the first skin we need to create a QGLWidget so that all
    // the QGLWidget's we create can use it as a shared QGLContext.
    if (!CmdlineArgs::Instance().getSafeMode() && QGLFormat::hasOpenGL()) {
        QGLFormat glFormat;
        glFormat.setDirectRendering(true);
        glFormat.setDoubleBuffer(true);
        glFormat.setDepth(false);
        // Disable waiting for vertical Sync
        // This can be enabled when using a single Threads for each QGLContext
        // Setting 1 causes QGLContext::swapBuffer to sleep until the next VSync
#if defined(__APPLE__)
        // On OS X, syncing to vsync has good performance FPS-wise and
        // eliminates tearing.
        glFormat.setSwapInterval(1);
#else
        // Otherwise, turn VSync off because it could cause horrible FPS on
        // Linux.
        // TODO(XXX): Make this configurable.
        // TODO(XXX): What should we do on Windows?
        glFormat.setSwapInterval(0);
#endif
        glFormat.setRgba(true);
        QGLFormat::setDefaultFormat(glFormat);

        QGLWidget* pContextWidget = new QGLWidget(this);
        pContextWidget->setGeometry(QRect(0, 0, 3, 3));
        pContextWidget->hide();
        SharedGLContext::setWidget(pContextWidget);
    }

    WaveformWidgetFactory::createInstance(); // takes a long time
    WaveformWidgetFactory::instance()->setConfig(pConfig);
    WaveformWidgetFactory::instance()->startVSync(m_pGuiTick, m_pVisualsManager);

    launchProgress(52);

    connect(this,
            &MixxxMainWindow::skinLoaded,
            m_pLibrary,
            &Library::onSkinLoadFinished);

    connect(this,
            &MixxxMainWindow::skinLoaded,
            WaveformWidgetFactory::instance(),
            &WaveformWidgetFactory::slotSkinLoaded);

    // Inhibit the screensaver if the option is set. (Do it before creating the preferences dialog)
    int inhibit = pConfig->getValue<int>(ConfigKey("[Config]","InhibitScreensaver"),-1);
    if (inhibit == -1) {
        inhibit = static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON);
        pConfig->setValue<int>(ConfigKey("[Config]","InhibitScreensaver"), inhibit);
    }
    m_inhibitScreensaver = static_cast<mixxx::ScreenSaverPreference>(inhibit);
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    }

    // Initialize preference dialog
    m_pPrefDlg = new DlgPreferences(
            this,
            m_pSkinLoader,
            m_pSoundManager,
            m_pPlayerManager,
            m_pControllerManager,
            m_pVCManager,
            pLV2Backend,
            m_pEffectsManager,
            m_pSettingsManager.get(),
            m_pLibrary);
    m_pPrefDlg->setWindowIcon(QIcon(":/images/mixxx_icon.svg"));
    m_pPrefDlg->setHidden(true);

    launchProgress(60);

    // Connect signals to the menubar. Should be done before we go fullscreen
    // and emit newSkinLoaded.
    connectMenuBar();

    launchProgress(63);

    QWidget* oldWidget = m_pCentralWidget;

    // Load default styles that can be overridden by skins
    QFile file(":/skins/default.qss");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileBytes = file.readAll();
        QString style = QString::fromLocal8Bit(fileBytes.constData(),
                                               fileBytes.length());
        setStyleSheet(style);
    } else {
        qWarning() << "Failed to load default skin styles!";
    }

    if (!loadConfiguredSkin()) {
        reportCriticalErrorAndQuit(
                "default skin cannot be loaded - see <b>mixxx</b> trace for more information");
        m_pCentralWidget = oldWidget;
        //TODO (XXX) add dialog to warn user and launch skin choice page
    }

    // Fake a 100 % progress here.
    // At a later place it will newer shown up, since it is
    // immediately replaced by the real widget.
    launchProgress(100);

    // Check direct rendering and warn user if they don't have it
    if (!CmdlineArgs::Instance().getSafeMode()) {
        checkDirectRendering();
    }

    // Install an event filter to catch certain QT events, such as tooltips.
    // This allows us to turn off tooltips.
    pApp->installEventFilter(this); // The eventfilter is located in this
                                    // Mixxx class as a callback.

    // If we were told to start in fullscreen mode on the command-line or if
    // user chose always starts in fullscreen mode, then turn on fullscreen
    // mode.
    bool fullscreenPref = pConfig->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if (args.getStartInFullscreen() || fullscreenPref) {
        slotViewFullScreen(true);
    }
    emit skinLoaded();


    // Wait until all other ControlObjects are set up before initializing
    // controllers
    m_pControllerManager->setUpDevices();

    // Scan the library for new files and directories
    bool rescan = pConfig->getValue<bool>(
            ConfigKey("[Library]","RescanOnStartup"));
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
    // 5.14. Until the minimum required Qt version of Mixx is increased,
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

    // Try open player device If that fails, the preference panel is opened.
    bool retryClicked;
    do {
        retryClicked = false;
        SoundDeviceError result = m_pSoundManager->setupDevices();
        if (result == SOUNDDEVICE_ERROR_DEVICE_COUNT ||
                result == SOUNDDEVICE_ERROR_EXCESSIVE_OUTPUT_CHANNEL) {
            if (soundDeviceBusyDlg(&retryClicked) != QDialog::Accepted) {
                exit(0);
            }
        } else if (result != SOUNDDEVICE_ERROR_OK) {
            if (soundDeviceErrorMsgDlg(result, &retryClicked) !=
                    QDialog::Accepted) {
                exit(0);
            }
        }
    } while (retryClicked);

    // test for at least one out device, if none, display another dlg that
    // says "mixxx will barely work with no outs"
    // In case persisting errors, the user has already received a message
    // box from the preferences dialog above. So we can watch here just the
    // output count.
    while (m_pSoundManager->getConfig().getOutputs().count() == 0) {
        // Exit when we press the Exit button in the noSoundDlg dialog
        // only call it if result != OK
        bool continueClicked = false;
        if (noOutputDlg(&continueClicked) != QDialog::Accepted) {
            exit(0);
        }
        if (continueClicked) break;
   }

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    const QList<QString>& musicFiles = args.getMusicFiles();
    for (int i = 0; i < (int)m_pPlayerManager->numDecks()
            && i < musicFiles.count(); ++i) {
        if (SoundSourceProxy::isFileNameSupported(musicFiles.at(i))) {
            m_pPlayerManager->slotLoadToDeck(musicFiles.at(i), i+1);
        }
    }

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &MixxxMainWindow::slotUpdateWindowTitle);

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingDeckChanged,
            this,
            &MixxxMainWindow::slotChangedPlayingDeck);

    // this has to be after the OpenGL widgets are created or depending on a
    // million different variables the first waveform may be horribly
    // corrupted. See bug 521509 -- bkgood ?? -- vrince
    setCentralWidget(m_pCentralWidget);
    // The launch image widget is automatically disposed, but we still have a
    // pointer to it.
    m_pLaunchImage = nullptr;
}

void MixxxMainWindow::finalize() {
    Timer t("MixxxMainWindow::~finalize");
    t.start();

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    // Stop all pending library operations
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "stopping pending Library tasks";
    m_pTrackCollectionManager->stopLibraryScan();
    m_pLibrary->stopPendingTasks();

   // Save the current window state (position, maximized, etc)
    m_pSettingsManager->settings()->set(ConfigKey("[MainWindow]", "geometry"),
        QString(saveGeometry().toBase64()));
    m_pSettingsManager->settings()->set(ConfigKey("[MainWindow]", "state"),
        QString(saveState().toBase64()));

    qDebug() << "Destroying MixxxMainWindow";

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "saving configuration";
    m_pSettingsManager->save();

    // GUI depends on KeyboardEventFilter, PlayerManager, Library
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting skin";
    m_pCentralWidget = nullptr;
    QPointer<QWidget> pSkin(centralWidget());
    setCentralWidget(nullptr);
    if (!pSkin.isNull()) {
        QCoreApplication::sendPostedEvents(pSkin, QEvent::DeferredDelete);
    }
    // Our central widget is now deleted.
    VERIFY_OR_DEBUG_ASSERT(pSkin.isNull()) {
        qWarning() << "Central widget was not deleted by our sendPostedEvents trick.";
    }

    // Delete Controls created by skins
    qDeleteAll(m_skinCreatedControls);
    m_skinCreatedControls.clear();

    // TODO() Verify if this comment still applies:
    // WMainMenuBar holds references to controls so we need to delete it
    // before MixxxMainWindow is destroyed. QMainWindow calls deleteLater() in
    // setMenuBar() but we need to delete it now so we can ask for
    // DeferredDelete events to be processed for it. Once Mixxx shutdown lives
    // outside of MixxxMainWindow the parent relationship will directly destroy
    // the WMainMenuBar and this will no longer be a problem.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting menubar";

    QPointer<WMainMenuBar> pMenuBar = m_pMenuBar.toWeakRef();
    DEBUG_ASSERT(menuBar() == m_pMenuBar.get());
    // We need to reset the parented pointer here that it does not become a
    // dangling pinter after the object has been deleted.
    m_pMenuBar = nullptr;
    setMenuBar(nullptr);
    if (!pMenuBar.isNull()) {
        QCoreApplication::sendPostedEvents(pMenuBar, QEvent::DeferredDelete);
    }
    // Our main menu is now deleted.
    VERIFY_OR_DEBUG_ASSERT(pMenuBar.isNull()) {
        qWarning() << "WMainMenuBar was not deleted by our sendPostedEvents trick.";
    }

    // SoundManager depend on Engine and Config
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting SoundManager";
    delete m_pSoundManager;

    // ControllerManager depends on Config
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting ControllerManager";
    delete m_pControllerManager;

#ifdef __VINYLCONTROL__
    // VinylControlManager depends on a CO the engine owns
    // (vinylcontrol_enabled in VinylControlControl)
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting VinylControlManager";
    delete m_pVCManager;
#endif

    // CoverArtCache is fairly independent of everything else.
    CoverArtCache::destroy();

    // PlayerManager depends on Engine, SoundManager, VinylControlManager, and Config
    // The player manager has to be deleted before the library to ensure
    // that all modified track metadata of loaded tracks is saved.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting PlayerManager";
    delete m_pPlayerManager;

    // Destroy PlayerInfo explicitly to release the track
    // pointers of tracks that were still loaded in decks
    // or samplers when PlayerManager was destroyed!
    PlayerInfo::destroy();

    // Delete the library after the view so there are no dangling pointers to
    // the data models.
    // Depends on RecordingManager and PlayerManager
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting Library";
    delete m_pLibrary;

    // RecordingManager depends on config, engine
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting RecordingManager";
    delete m_pRecordingManager;

#ifdef __BROADCAST__
    // BroadcastManager depends on config, engine
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting BroadcastManager";
    delete m_pBroadcastManager;
#endif

    // EngineMaster depends on Config and m_pEffectsManager.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting EngineMaster";
    delete m_pEngine;

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting DlgPreferences";
    delete m_pPrefDlg;

    // Must delete after EngineMaster and DlgPrefEq.
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting EffectsManager";
    delete m_pEffectsManager;

    delete m_pTouchShift;

    WaveformWidgetFactory::destroy();

    delete m_pGuiTick;
    delete m_pVisualsManager;

    // Delete the track collections after all internal track pointers
    // in other components have been released by deleting those components
    // beforehand!
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "detaching all track collections";
    delete m_pTrackCollectionManager;

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "closing database connection(s)";
    m_pDbConnectionPool->destroyThreadLocalConnection();
    m_pDbConnectionPool.reset(); // should drop the last reference

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pSettingsManager->save();

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

    delete m_pKeyboard;
    delete m_pKbdConfig;
    delete m_pKbdConfigEmpty;

    t.elapsed(true);
    // Report the total time we have been running.
    m_runtime_timer.elapsed(true);

    if (m_cmdLineArgs.getDeveloper()) {
        StatsManager::destroy();
    }
}

bool MixxxMainWindow::initializeDatabase() {
    kLogger.info() << "Connecting to database";
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
    if (!dbConnection.isOpen()) {
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

void MixxxMainWindow::initializeWindow() {
    // be sure createMenuBar() is called first
    DEBUG_ASSERT(m_pMenuBar);

    QPalette Pal(palette());
    // safe default QMenuBar background
    QColor MenuBarBackground(m_pMenuBar->palette().color(QPalette::Window));
    Pal.setColor(QPalette::Window, QColor(0x202020));
    setAutoFillBackground(true);
    setPalette(Pal);
    // restore default QMenuBar background
    Pal.setColor(QPalette::Window, MenuBarBackground);
    m_pMenuBar->setPalette(Pal);

    // Restore the current window state (position, maximized, etc)
    restoreGeometry(QByteArray::fromBase64(m_pSettingsManager->settings()->getValueString(
        ConfigKey("[MainWindow]", "geometry")).toUtf8()));
    restoreState(QByteArray::fromBase64(m_pSettingsManager->settings()->getValueString(
        ConfigKey("[MainWindow]", "state")).toUtf8()));

    setWindowIcon(QIcon(":/images/mixxx_icon.svg"));
    slotUpdateWindowTitle(TrackPointer());
}

void MixxxMainWindow::initializeKeyboard() {
    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    QString resourcePath = pConfig->getResourcePath();

    // Set the default value in settings file
    if (pConfig->getValueString(ConfigKey("[Keyboard]","Enabled")).length() == 0)
        pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(1));

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard = QDir(pConfig->getSettingsPath()).filePath("Custom.kbd.cfg");

    // Empty keyboard configuration
    m_pKbdConfigEmpty = new ConfigObject<ConfigValueKbd>(QString());

    if (QFile::exists(userKeyboard)) {
        qDebug() << "Found and will use custom keyboard preset" << userKeyboard;
        m_pKbdConfig = new ConfigObject<ConfigValueKbd>(userKeyboard);
    } else {
        // Default to the locale for the main input method (e.g. keyboard).
        QLocale locale = inputLocale();

        // check if a default keyboard exists
        QString defaultKeyboard = QString(resourcePath).append("keyboard/");
        defaultKeyboard += locale.name();
        defaultKeyboard += ".kbd.cfg";
        qDebug() << "Found and will use default keyboard preset" << defaultKeyboard;

        if (!QFile::exists(defaultKeyboard)) {
            qDebug() << defaultKeyboard << " not found, using en_US.kbd.cfg";
            defaultKeyboard = QString(resourcePath).append("keyboard/").append("en_US.kbd.cfg");
            if (!QFile::exists(defaultKeyboard)) {
                qDebug() << defaultKeyboard << " not found, starting without shortcuts";
                defaultKeyboard = "";
            }
        }
        m_pKbdConfig = new ConfigObject<ConfigValueKbd>(defaultKeyboard);
    }

    // TODO(XXX) leak pKbdConfig, KeyboardEventFilter owns it? Maybe roll all keyboard
    // initialization into KeyboardEventFilter
    // Workaround for today: KeyboardEventFilter calls delete
    bool keyboardShortcutsEnabled = pConfig->getValue<bool>(
            ConfigKey("[Keyboard]", "Enabled"));
    m_pKeyboard = new KeyboardEventFilter(keyboardShortcutsEnabled ? m_pKbdConfig : m_pKbdConfigEmpty);
}

QDialog::DialogCode MixxxMainWindow::soundDeviceErrorDlg(
        const QString &title, const QString &text, bool* retryClicked) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);

    QPushButton* retryButton =
            msgBox.addButton(tr("Retry"), QMessageBox::ActionRole);
    QPushButton* reconfigureButton =
            msgBox.addButton(tr("Reconfigure"), QMessageBox::ActionRole);
    QPushButton* wikiButton =
            msgBox.addButton(tr("Help"), QMessageBox::ActionRole);
    QPushButton* exitButton =
            msgBox.addButton(tr("Exit"), QMessageBox::ActionRole);

    while (true)
    {
        msgBox.exec();

        if (msgBox.clickedButton() == retryButton) {
            m_pSoundManager->clearAndQueryDevices();
            *retryClicked = true;
            return QDialog::Accepted;
        } else if (msgBox.clickedButton() == wikiButton) {
            QDesktopServices::openUrl(QUrl(
                "http://mixxx.org/wiki/doku.php/troubleshooting"
                "#i_can_t_select_my_sound_card_in_the_sound_hardware_preferences"));
            wikiButton->setEnabled(false);
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();

            m_pSoundManager->clearAndQueryDevices();
            // This way of opening the dialog allows us to use it synchronously
            m_pPrefDlg->setWindowModality(Qt::ApplicationModal);
            m_pPrefDlg->exec();
            if (m_pPrefDlg->result() == QDialog::Accepted) {
                return QDialog::Accepted;
            }

            msgBox.show();
        } else if (msgBox.clickedButton() == exitButton) {
            // Will finally quit Mixxx
            return QDialog::Rejected;
        }
    }
}

QDialog::DialogCode MixxxMainWindow::soundDeviceBusyDlg(bool* retryClicked) {
    QString title(tr("Sound Device Busy"));
    QString text(
            "<html> <p>" %
            tr("Mixxx was unable to open all the configured sound devices.") +
            "</p> <p>" %
            m_pSoundManager->getErrorDeviceName() %
            " is used by another application or not plugged in."
            "</p><ul>"
                "<li>" %
                    tr("<b>Retry</b> after closing the other application "
                    "or reconnecting a sound device") %
                "</li>"
                "<li>" %
                    tr("<b>Reconfigure</b> Mixxx's sound device settings.") %
                "</li>"
                "<li>" %
                    tr("Get <b>Help</b> from the Mixxx Wiki.") %
                "</li>"
                "<li>" %
                    tr("<b>Exit</b> Mixxx.") %
                "</li>"
            "</ul></html>"
    );
    return soundDeviceErrorDlg(title, text, retryClicked);
}


QDialog::DialogCode MixxxMainWindow::soundDeviceErrorMsgDlg(
        SoundDeviceError err, bool* retryClicked) {
    QString title(tr("Sound Device Error"));
    QString text(
            "<html> <p>" %
            tr("Mixxx was unable to open all the configured sound devices.") +
            "</p> <p>" %
            m_pSoundManager->getLastErrorMessage(err).replace("\n", "<br/>") %
            "</p><ul>"
                "<li>" %
                    tr("<b>Retry</b> after fixing an issue") %
                "</li>"
                "<li>" %
                    tr("<b>Reconfigure</b> Mixxx's sound device settings.") %
                "</li>"
                "<li>" %
                    tr("Get <b>Help</b> from the Mixxx Wiki.") %
                "</li>"
                "<li>" %
                    tr("<b>Exit</b> Mixxx.") %
                "</li>"
            "</ul></html>"
    );
    return soundDeviceErrorDlg(title, text, retryClicked);
}

QDialog::DialogCode MixxxMainWindow::noOutputDlg(bool* continueClicked) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("No Output Devices"));
    msgBox.setText(
            "<html>" + tr("Mixxx was configured without any output sound devices. "
            "Audio processing will be disabled without a configured output device.") +
            "<ul>"
                "<li>" +
                    tr("<b>Continue</b> without any outputs.") +
                "</li>"
                "<li>" +
                    tr("<b>Reconfigure</b> Mixxx's sound device settings.") +
                "</li>"
                "<li>" +
                    tr("<b>Exit</b> Mixxx.") +
                "</li>"
            "</ul></html>"
    );

    QPushButton* continueButton =
            msgBox.addButton(tr("Continue"), QMessageBox::ActionRole);
    QPushButton* reconfigureButton =
            msgBox.addButton(tr("Reconfigure"), QMessageBox::ActionRole);
    QPushButton* exitButton =
            msgBox.addButton(tr("Exit"), QMessageBox::ActionRole);

    while (true)
    {
        msgBox.exec();

        if (msgBox.clickedButton() == continueButton) {
            *continueClicked = true;
            return QDialog::Accepted;
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();

            // This way of opening the dialog allows us to use it synchronously
            m_pPrefDlg->setWindowModality(Qt::ApplicationModal);
            m_pPrefDlg->exec();
            if (m_pPrefDlg->result() == QDialog::Accepted) {
                return QDialog::Accepted;
            }

            msgBox.show();

        } else if (msgBox.clickedButton() == exitButton) {
            // Will finally quit Mixxx
            return QDialog::Rejected;
        }
    }
}

void MixxxMainWindow::slotUpdateWindowTitle(TrackPointer pTrack) {
    QString appTitle = Version::applicationTitle();

    // If we have a track, use getInfo() to format a summary string and prepend
    // it to the title.
    // TODO(rryan): Does this violate Mac App Store policies?
    if (pTrack) {
        QString trackInfo = pTrack->getInfo();
        if (!trackInfo.isEmpty()) {
            appTitle = QString("%1 | %2")
                    .arg(trackInfo)
                    .arg(appTitle);
        }
    }
    this->setWindowTitle(appTitle);
}

void MixxxMainWindow::createMenuBar() {
    ScopedTimer t("MixxxMainWindow::createMenuBar");
    DEBUG_ASSERT(m_pKbdConfig != nullptr);
    m_pMenuBar = make_parented<WMainMenuBar>(this, m_pSettingsManager->settings(), m_pKbdConfig);
    setMenuBar(m_pMenuBar);
}

void MixxxMainWindow::connectMenuBar() {
    ScopedTimer t("MixxxMainWindow::connectMenuBar");
    connect(this,
            &MixxxMainWindow::skinLoaded,
            m_pMenuBar,
            &WMainMenuBar::onNewSkinLoaded);

    // Misc
    connect(m_pMenuBar, &WMainMenuBar::quit, this, &MixxxMainWindow::close);
    connect(m_pMenuBar,
            &WMainMenuBar::showPreferences,
            this,
            &MixxxMainWindow::slotOptionsPreferences);
    connect(m_pMenuBar,
            &WMainMenuBar::loadTrackToDeck,
            this,
            &MixxxMainWindow::slotFileLoadSongPlayer);

    // Fullscreen
    connect(m_pMenuBar,
            &WMainMenuBar::toggleFullScreen,
            this,
            &MixxxMainWindow::slotViewFullScreen);
    connect(this,
            &MixxxMainWindow::fullScreenChanged,
            m_pMenuBar,
            &WMainMenuBar::onFullScreenStateChange);

    // Keyboard shortcuts
    connect(m_pMenuBar,
            &WMainMenuBar::toggleKeyboardShortcuts,
            this,
            &MixxxMainWindow::slotOptionsKeyboard);

    // Help
    connect(m_pMenuBar,
            &WMainMenuBar::showAbout,
            this,
            &MixxxMainWindow::slotHelpAbout);

    // Developer
    connect(m_pMenuBar,
            &WMainMenuBar::reloadSkin,
            this,
            &MixxxMainWindow::rebootMixxxView);
    connect(m_pMenuBar,
            &WMainMenuBar::toggleDeveloperTools,
            this,
            &MixxxMainWindow::slotDeveloperTools);

    if (m_pRecordingManager) {
        connect(m_pRecordingManager,
                &RecordingManager::isRecording,
                m_pMenuBar,
                &WMainMenuBar::onRecordingStateChange);
        connect(m_pMenuBar,
                &WMainMenuBar::toggleRecording,
                m_pRecordingManager,
                &RecordingManager::slotSetRecording);
        m_pMenuBar->onRecordingStateChange(m_pRecordingManager->isRecordingActive());
    }

#ifdef __BROADCAST__
    if (m_pBroadcastManager) {
        connect(m_pBroadcastManager,
                &BroadcastManager::broadcastEnabled,
                m_pMenuBar,
                &WMainMenuBar::onBroadcastingStateChange);
        connect(m_pMenuBar,
                &WMainMenuBar::toggleBroadcasting,
                m_pBroadcastManager,
                &BroadcastManager::setEnabled);
        m_pMenuBar->onBroadcastingStateChange(m_pBroadcastManager->isEnabled());
    }
#endif

#ifdef __VINYLCONTROL__
    if (m_pVCManager) {
        connect(m_pMenuBar,
                &WMainMenuBar::toggleVinylControl,
                m_pVCManager,
                &VinylControlManager::toggleVinylControl);
        connect(m_pVCManager,
                &VinylControlManager::vinylControlDeckEnabled,
                m_pMenuBar,
                &WMainMenuBar::onVinylControlDeckEnabledStateChange);
    }
#endif

    if (m_pPlayerManager) {
        connect(m_pPlayerManager,
                &PlayerManager::numberOfDecksChanged,
                m_pMenuBar,
                &WMainMenuBar::onNumberOfDecksChanged);
        m_pMenuBar->onNumberOfDecksChanged(m_pPlayerManager->numberOfDecks());
    }

    if (m_pTrackCollectionManager) {
        connect(m_pMenuBar,
                &WMainMenuBar::rescanLibrary,
                m_pTrackCollectionManager,
                &TrackCollectionManager::startLibraryScan);
        connect(m_pTrackCollectionManager,
                &TrackCollectionManager::libraryScanStarted,
                m_pMenuBar,
                &WMainMenuBar::onLibraryScanStarted);
        connect(m_pTrackCollectionManager,
                &TrackCollectionManager::libraryScanFinished,
                m_pMenuBar,
                &WMainMenuBar::onLibraryScanFinished);
    }

    if (m_pLibrary) {
        connect(m_pMenuBar,
                &WMainMenuBar::createCrate,
                m_pLibrary,
                &Library::slotCreateCrate);
        connect(m_pMenuBar,
                &WMainMenuBar::createPlaylist,
                m_pLibrary,
                &Library::slotCreatePlaylist);
    }
}

void MixxxMainWindow::slotFileLoadSongPlayer(int deck) {
    QString group = m_pPlayerManager->groupForDeck(deck-1);

    QString loadTrackText = tr("Load track to Deck %1").arg(QString::number(deck));
    QString deckWarningMessage = tr("Deck %1 is currently playing a track.")
            .arg(QString::number(deck));
    QString areYouSure = tr("Are you sure you want to load a new track?");

    if (ControlObject::get(ConfigKey(group, "play")) > 0.0) {
        int ret = QMessageBox::warning(this, Version::applicationName(),
            deckWarningMessage + "\n" + areYouSure,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return;
    }

    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    QString trackPath =
        QFileDialog::getOpenFileName(
            this,
            loadTrackText,
            pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR),
            QString("Audio (%1)")
                .arg(SoundSourceProxy::getSupportedFileNamePatterns().join(" ")));


    if (!trackPath.isNull()) {
        // The user has picked a file via a file dialog. This means the system
        // sandboxer (if we are sandboxed) has granted us permission to this
        // folder. Create a security bookmark while we have permission so that
        // we can access the folder on future runs. We need to canonicalize the
        // path so we first wrap the directory string with a QDir.
        QFileInfo trackInfo(trackPath);
        Sandbox::createSecurityToken(trackInfo);

        m_pPlayerManager->slotLoadToDeck(trackPath, deck);
    }
}


void MixxxMainWindow::slotOptionsKeyboard(bool toggle) {
    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    if (toggle) {
        //qDebug() << "Enable keyboard shortcuts/mappings";
        m_pKeyboard->setKeyboardConfig(m_pKbdConfig);
        pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(1));
    } else {
        //qDebug() << "Disable keyboard shortcuts/mappings";
        m_pKeyboard->setKeyboardConfig(m_pKbdConfigEmpty);
        pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(0));
    }
}

void MixxxMainWindow::slotDeveloperTools(bool visible) {
    if (visible) {
        if (m_pDeveloperToolsDlg == nullptr) {
            UserSettingsPointer pConfig = m_pSettingsManager->settings();
            m_pDeveloperToolsDlg = new DlgDeveloperTools(this, pConfig);
            connect(m_pDeveloperToolsDlg,
                    &DlgDeveloperTools::destroyed,
                    this,
                    &MixxxMainWindow::slotDeveloperToolsClosed);
            connect(this,
                    &MixxxMainWindow::closeDeveloperToolsDlgChecked,
                    m_pDeveloperToolsDlg,
                    &DlgDeveloperTools::done);
            connect(m_pDeveloperToolsDlg,
                    &DlgDeveloperTools::destroyed,
                    m_pMenuBar,
                    &WMainMenuBar::onDeveloperToolsHidden);
        }
        m_pMenuBar->onDeveloperToolsShown();
        m_pDeveloperToolsDlg->show();
        m_pDeveloperToolsDlg->activateWindow();
    } else {
        emit closeDeveloperToolsDlgChecked(0);
    }
}

void MixxxMainWindow::slotDeveloperToolsClosed() {
    m_pDeveloperToolsDlg = NULL;
}

void MixxxMainWindow::slotViewFullScreen(bool toggle) {
    if (isFullScreen() == toggle) {
        return;
    }

    if (toggle) {
        showFullScreen();
#ifdef __LINUX__
        // Fix for "No menu bar with ubuntu unity in full screen mode" Bug
        // #885890 and Bug #1076789. Before touching anything here, please read
        // those bugs.
        createMenuBar();
        connectMenuBar();
        if (m_pMenuBar->isNativeMenuBar()) {
            m_pMenuBar->setNativeMenuBar(false);
        }
#endif
    } else {
#ifdef __LINUX__
        createMenuBar();
        connectMenuBar();
#endif
        showNormal();
    }
    emit fullScreenChanged(toggle);
}

void MixxxMainWindow::slotOptionsPreferences() {
    m_pPrefDlg->show();
    m_pPrefDlg->raise();
    m_pPrefDlg->activateWindow();
}

void MixxxMainWindow::slotNoVinylControlInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::warning(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this vinyl control.\n"
           "Please select an input device in the sound hardware preferences first."),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoDeckPassthroughInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::warning(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this passthrough control.\n"
           "Please select an input device in the sound hardware preferences first."),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoMicrophoneInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::question(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this microphone.\n"
           "Do you want to select an input device?"),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoAuxiliaryInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::question(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this auxiliary.\n"
           "Do you want to select an input device?"),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotChangedPlayingDeck(int deck) {
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON_PLAY) {
        if (deck==-1) {
            // If no deck is playing, allow the screensaver to run.
            mixxx::ScreenSaverHelper::uninhibit();
        } else {
            mixxx::ScreenSaverHelper::inhibit();
        }
    }
}

void MixxxMainWindow::slotHelpAbout() {
    DlgAbout* about = new DlgAbout(this);
    about->show();
}

void MixxxMainWindow::setToolTipsCfg(mixxx::TooltipsPreference tt) {
    UserSettingsPointer pConfig = m_pSettingsManager->settings();
    pConfig->set(ConfigKey("[Controls]","Tooltips"),
                 ConfigValue(static_cast<int>(tt)));
    m_toolTipsCfg = tt;
}

void MixxxMainWindow::rebootMixxxView() {
    qDebug() << "Now in rebootMixxxView...";

    // safe geometry for later restoration
    const QRect initGeometry = geometry();

    // We need to tell the menu bar that we are about to delete the old skin and
    // create a new one. It holds "visibility" controls (e.g. "Show Samplers")
    // that need to be deleted -- otherwise we can't tell what features the skin
    // supports since the controls from the previous skin will be left over.
    m_pMenuBar->onNewSkinAboutToLoad();

    if (m_pCentralWidget) {
        m_pCentralWidget->hide();
        WaveformWidgetFactory::instance()->destroyWidgets();
        delete m_pCentralWidget;
        m_pCentralWidget = nullptr;
    }

    // Workaround for changing skins while fullscreen, just go out of fullscreen
    // mode. If you change skins while in fullscreen (on Linux, at least) the
    // window returns to 0,0 but and the backdrop disappears so it looks as if
    // it is not fullscreen, but acts as if it is.
    bool wasFullScreen = isFullScreen();
    slotViewFullScreen(false);

    if (!loadConfiguredSkin()) {
        QMessageBox::critical(this,
                              tr("Error in skin file"),
                              tr("The selected skin cannot be loaded."));
        // m_pWidgetParent is NULL, we can't continue.
        return;
    }

    setCentralWidget(m_pCentralWidget);
#ifdef __LINUX__
    // don't adjustSize() on Linux as this wouldn't use the entire available area
    // to paint the new skin with X11
    // https://bugs.launchpad.net/mixxx/+bug/1773587
#else
    adjustSize();
#endif

    if (wasFullScreen) {
        slotViewFullScreen(true);
    } else {
        // Programatic placement at this point is very problematic.
        // The screen() method returns stale data (primary screen)
        // until the user interacts with mixxx again. Keyboard shortcuts
        // do not count, moving window, opening menu etc does
        // Therefore the placement logic was removed by a simple geometry restore.
        // If the minimum size of the new skin is larger then the restored
        // geometry, the window will be enlarged right & bottom which is
        // safe as the menu is still reachable.
        setGeometry(initGeometry);
    }

    qDebug() << "rebootMixxxView DONE";
    emit skinLoaded();
}

bool MixxxMainWindow::loadConfiguredSkin() {
    m_pCentralWidget = m_pSkinLoader->loadConfiguredSkin(this,
            &m_skinCreatedControls,
            m_pKeyboard,
            m_pPlayerManager,
            m_pControllerManager,
            m_pLibrary,
            m_pVCManager,
            m_pEffectsManager,
            m_pRecordingManager);
    return m_pCentralWidget != nullptr;
}

bool MixxxMainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        // return true for no tool tips
        switch (m_toolTipsCfg) {
            case mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY:
                return dynamic_cast<WBaseWidget*>(obj) != nullptr;
            case mixxx::TooltipsPreference::TOOLTIPS_ON:
                return false;
            case mixxx::TooltipsPreference::TOOLTIPS_OFF:
                return true;
            default:
                DEBUG_ASSERT(!"m_toolTipsCfg value unknown");
                return true;
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void MixxxMainWindow::closeEvent(QCloseEvent *event) {
    // WARNING: We can receive a CloseEvent while only partially
    // initialized. This is because we call QApplication::processEvents to
    // render LaunchImage progress in the constructor.
    if (!confirmExit()) {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}


void MixxxMainWindow::checkDirectRendering() {
    // IF
    //  * A waveform viewer exists
    // AND
    //  * The waveform viewer is an OpenGL waveform viewer
    // AND
    //  * The waveform viewer does not have direct rendering enabled.
    // THEN
    //  * Warn user

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    if (!factory)
        return;

    UserSettingsPointer pConfig = m_pSettingsManager->settings();

    if (!factory->isOpenGlAvailable() && !factory->isOpenGlesAvailable() &&
        pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes")) {
        QMessageBox::warning(
            0, tr("OpenGL Direct Rendering"),
            tr("Direct rendering is not enabled on your machine.<br><br>"
               "This means that the waveform displays will be very<br>"
               "<b>slow and may tax your CPU heavily</b>. Either update your<br>"
               "configuration to enable direct rendering, or disable<br>"
               "the waveform displays in the Mixxx preferences by selecting<br>"
               "\"Empty\" as the waveform display in the 'Interface' section."));
        pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), QString("yes"));
    }
}

bool MixxxMainWindow::confirmExit() {
    bool playing(false);
    bool playingSampler(false);
    unsigned int deckCount = m_pPlayerManager->numDecks();
    unsigned int samplerCount = m_pPlayerManager->numSamplers();
    for (unsigned int i = 0; i < deckCount; ++i) {
        if (ControlObject::toBool(
                    ConfigKey(PlayerManager::groupForDeck(i), "play"))) {
            playing = true;
            break;
        }
    }
    for (unsigned int i = 0; i < samplerCount; ++i) {
        if (ControlObject::toBool(
                    ConfigKey(PlayerManager::groupForSampler(i), "play"))) {
            playingSampler = true;
            break;
        }
    }
    if (playing) {
        QMessageBox::StandardButton btn = QMessageBox::question(this,
            tr("Confirm Exit"),
            tr("A deck is currently playing. Exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
    } else if (playingSampler) {
        QMessageBox::StandardButton btn = QMessageBox::question(this,
            tr("Confirm Exit"),
            tr("A sampler is currently playing. Exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
    }
    if (m_pPrefDlg && m_pPrefDlg->isVisible()) {
        QMessageBox::StandardButton btn = QMessageBox::question(
            this, tr("Confirm Exit"),
            tr("The preferences window is still open.") + "<br>" +
            tr("Discard any changes and exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
        else {
            m_pPrefDlg->close();
        }
    }

    return true;
}

void MixxxMainWindow::setInhibitScreensaver(mixxx::ScreenSaverPreference newInhibit)
{
    UserSettingsPointer pConfig = m_pSettingsManager->settings();

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    if (newInhibit == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    } else if (newInhibit == mixxx::ScreenSaverPreference::PREVENT_ON_PLAY
            && PlayerInfo::instance().getCurrentPlayingDeck()!=-1) {
        mixxx::ScreenSaverHelper::inhibit();
    }
    int inhibit_int = static_cast<int>(newInhibit);
    pConfig->setValue<int>(ConfigKey("[Config]","InhibitScreensaver"), inhibit_int);
    m_inhibitScreensaver = newInhibit;
}

mixxx::ScreenSaverPreference MixxxMainWindow::getInhibitScreensaver()
{
    return m_inhibitScreensaver;
}

void MixxxMainWindow::launchProgress(int progress) {
    if (m_pLaunchImage) {
        m_pLaunchImage->progress(progress);
    }
    qApp->processEvents();
}
