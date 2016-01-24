#include "widget/wmainwindow.h"

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QGLWidget>
#include <QUrl>
#include <QtDebug>

#include "dialog/dlgabout.h"
#include "preferences/dialog/dlgpreferences.h"
#include "preferences/constants.h"
#include "dialog/dlgdevelopertools.h"
#include "library/library_preferences.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "skin/legacyskinparser.h"
#include "skin/skinloader.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/sharedglcontext.h"
#include "util/debug.h"
#include "util/time.h"
#include "util/version.h"
#include "control/controlpushbutton.h"
#include "util/compatibility.h"
#include "util/sandbox.h"
#include "mixer/playerinfo.h"
#include "util/math.h"
#include "skin/launchimage.h"
#include "widget/wmainmenubar.h"
#include "util/screensaver.h"
#include "util/logger.h"
#include "coreservices.h"

#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrolmanager.h"
#endif

#ifdef __MODPLUG__
#include "preferences/dialog/dlgprefmodplug.h"
#endif

namespace {

const mixxx::Logger kLogger("WMainWindow");

} // anonymous namespace

WMainWindow::WMainWindow(QApplication* pApp,
                         std::shared_ptr<mixxx::CoreServices> pCoreServices)
        : m_pLaunchImage(nullptr),
          m_pCore(pCoreServices),
          m_skinLoader(pCoreServices->settingsManager()->settings()),
          m_pKeyboard(nullptr),
          m_pMenuBar(nullptr),
          m_pDeveloperToolsDlg(nullptr),
          m_pPrefDlg(nullptr),
          m_pKbdConfig(nullptr),
          m_pKbdConfigEmpty(nullptr),
          m_toolTipsCfg(mixxx::TooltipsPreference::TOOLTIPS_ON),
          m_pTouchShift(nullptr) {
    initializeKeyboard();

    createMenuBar();

    initializeWindow();

    // First load launch image to show a the user a quick responds
    LaunchImage* pLaunchImage = m_skinLoader.loadLaunchImage(this);
    connect(this, SIGNAL(launchProgress(int)), pLaunchImage, SLOT(progress(int)));
    setCentralWidget(pLaunchImage);

    show();
#if defined(Q_WS_X11)
    // In asynchronous X11, the window will be mapped to screen
    // some time after being asked to show itself on the screen.
    extern void qt_x11_wait_for_window_manager(QWidget *mainWin);
    qt_x11_wait_for_window_manager(this);
#endif
    pApp->processEvents();

    initialize(pApp);
}

WMainWindow::~WMainWindow() {
    ScopedTimer t("WMainWindow::~WMainWindow");

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    // Save the current window state (position, maximized, etc)
    UserSettingsPointer pSettings = m_pCore->settingsManager()->settings();
    pSettings->set(ConfigKey("[MainWindow]", "geometry"),
        QString(saveGeometry().toBase64()));
    pSettings->set(ConfigKey("[MainWindow]", "state"),
        QString(saveState().toBase64()));

    delete m_pPrefDlg;

    delete m_pTouchShift;

    WaveformWidgetFactory::destroy();

    delete m_pKeyboard;
    delete m_pKbdConfig;
    delete m_pKbdConfigEmpty;
}

void WMainWindow::initialize(QApplication* pApp) {
    ScopedTimer t("WMainWindow::initialize");

    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();

    emit(launchProgress(2));

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = static_cast<mixxx::TooltipsPreference>(
        pConfig->getValue(ConfigKey("[Controls]", "Tooltips"),
                static_cast<int>(mixxx::TooltipsPreference::TOOLTIPS_ON)));

    setAttribute(Qt::WA_AcceptTouchEvents);
    m_pTouchShift = new ControlPushButton(ConfigKey("[Controls]", "touch_shift"));

    emit(launchProgress(11));

    auto pPlayerManager = m_pCore->playerManager();
    connect(pPlayerManager.get(), SIGNAL(noMicrophoneInputConfigured()),
            this, SLOT(slotNoMicrophoneInputConfigured()));
    connect(pPlayerManager.get(), SIGNAL(noDeckPassthroughInputConfigured()),
            this, SLOT(slotNoDeckPassthroughInputConfigured()));
    connect(pPlayerManager.get(), SIGNAL(noVinylControlInputConfigured()),
            this, SLOT(slotNoVinylControlInputConfigured()));

    emit(launchProgress(30));

#ifdef __MODPLUG__
    // restore the configuration for the modplug library before trying to load a module
    DlgPrefModplug* pModplugPrefs = new DlgPrefModplug(0, pConfig);
    pModplugPrefs->loadSettings();
    pModplugPrefs->applySettings();
    delete pModplugPrefs; // not needed anymore
#endif

    auto pLibraryManager = m_pCore->libraryManager();
    connect(this, SIGNAL(newSkinLoaded()),
            pLibraryManager.get(), SLOT(onSkinLoadFinished()));

    emit(launchProgress(35));

    // Get Music dir
    bool hasChanged_MusicDir = false;

    QStringList dirs = pLibraryManager->getDirs();
    if (dirs.size() < 1) {
        // TODO(XXX) this needs to be smarter, we can't distinguish between an empty
        // path return value (not sure if this is normally possible, but it is
        // possible with the Windows 7 "Music" library, which is what
        // QDesktopServices::storageLocation(QDesktopServices::MusicLocation)
        // resolves to) and a user hitting 'cancel'. If we get a blank return
        // but the user didn't hit cancel, we need to know this and let the
        // user take some course of action -- bkgood
        QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose music library directory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
        if (!fd.isEmpty()) {
            // adds Folder to database.
            pLibraryManager->slotRequestAddDir(fd);
            hasChanged_MusicDir = true;
        }
    }

    WaveformWidgetFactory::createInstance(); // takes a long time
    WaveformWidgetFactory::instance()->startVSync(m_pCore->getGuiTick());
    WaveformWidgetFactory::instance()->setConfig(pConfig);

    emit(launchProgress(52));

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
    m_pPrefDlg = new DlgPreferences(this, &m_skinLoader, m_pCore);
    m_pPrefDlg->setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));
    m_pPrefDlg->setHidden(true);

    emit(launchProgress(60));

    // Connect signals to the menubar. Should be done before we go fullscreen
    // and emit newSkinLoaded.
    connectMenuBar();

    // Before creating the first skin we need to create a QGLWidget so that all
    // the QGLWidget's we create can use it as a shared QGLContext.
    QGLWidget* pContextWidget = new QGLWidget(this);
    pContextWidget->hide();
    SharedGLContext::setWidget(pContextWidget);

    emit(launchProgress(63));

    // Load skin to a QWidget that we set as the central widget. Assignment
    // intentional in next line.
    QWidget* pSkinWidget = nullptr;
    if (!(pSkinWidget = m_skinLoader.loadConfiguredSkin(this, m_pKeyboard,
                                                        m_pCore->playerManager(),
                                                        m_pCore->controllerManager(),
                                                        m_pCore->libraryManager(),
                                                        m_pCore->vinylControlManager(),
                                                        m_pCore->effectsManager(),
                                                        m_pCore->recordingManager()))) {
        reportCriticalErrorAndQuit(
                "default skin cannot be loaded see <b>mixxx</b> trace for more information.");
        //TODO (XXX) add dialog to warn user and launch skin choice page
    }

    // Fake a 100 % progress here.
    // At a later place it will newer shown up, since it is
    // immediately replaced by the real widget.
    emit(launchProgress(100));

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
    const bool fullscreenPref = pConfig->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if (CmdlineArgs::Instance().getStartInFullscreen() || fullscreenPref) {
        slotViewFullScreen(true);
    }

    emit(newSkinLoaded());

    // Scan the library for new files and directories
    bool rescan = pConfig->getValue<bool>(
            ConfigKey("[Library]","RescanOnStartup"));
    // rescan the library if we get a new plugin
    QSet<QString> prev_plugins = QSet<QString>::fromList(
            pConfig->getValueString(
                    ConfigKey("[Library]", "SupportedFileExtensions")).split(
                    ",", QString::SkipEmptyParts));
    QSet<QString> curr_plugins = QSet<QString>::fromList(
        SoundSourceProxy::getSupportedFileExtensions());
    rescan = rescan || (prev_plugins != curr_plugins);
    pConfig->set(ConfigKey("[Library]", "SupportedFileExtensions"),
            QStringList(SoundSourceProxy::getSupportedFileExtensions()).join(","));

    // Scan the library directory. Do this after the skinloader has
    // loaded a skin, see Bug #1047435
    if (rescan || hasChanged_MusicDir || m_pCore->settingsManager()->shouldRescanLibrary()) {
        pLibraryManager->scan();
    }

    // Try open player device If that fails, the preference panel is opened.
    auto pSoundManager = m_pCore->soundManager();
    bool retryClicked;
    do {
        retryClicked = false;
        SoundDeviceError result = pSoundManager->setupDevices();
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
    while (pSoundManager->getConfig().getOutputs().count() == 0) {
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
    const QList<QString>& musicFiles = CmdlineArgs::Instance().getMusicFiles();
    for (int i = 0; i < (int)pPlayerManager->numDecks()
            && i < musicFiles.count(); ++i) {
        if (SoundSourceProxy::isFileNameSupported(musicFiles.at(i))) {
            pPlayerManager->slotLoadToDeck(musicFiles.at(i), i+1);
        }
    }

    connect(&PlayerInfo::instance(),
            SIGNAL(currentPlayingTrackChanged(TrackPointer)),
            this, SLOT(slotUpdateWindowTitle(TrackPointer)));

    connect(&PlayerInfo::instance(),
            SIGNAL(currentPlayingDeckChanged(int)),
            this, SLOT(slotChangedPlayingDeck(int)));

    // this has to be after the OpenGL widgets are created or depending on a
    // million different variables the first waveform may be horribly
    // corrupted. See bug 521509 -- bkgood ?? -- vrince
    setCentralWidget(pSkinWidget);
}

void WMainWindow::initializeWindow() {
    // be sure createMenuBar() is called first
    DEBUG_ASSERT(m_pMenuBar != nullptr);

    QPalette Pal(palette());
    // safe default QMenuBar background
    QColor MenuBarBackground(m_pMenuBar->palette().color(QPalette::Background));
    Pal.setColor(QPalette::Background, QColor(0x202020));
    setAutoFillBackground(true);
    setPalette(Pal);
    // restore default QMenuBar background
    Pal.setColor(QPalette::Background, MenuBarBackground);
    m_pMenuBar->setPalette(Pal);

    // Restore the current window state (position, maximized, etc)
    UserSettingsPointer pSettings = m_pCore->settingsManager()->settings();
    restoreGeometry(QByteArray::fromBase64(pSettings->getValueString(
        ConfigKey("[MainWindow]", "geometry")).toUtf8()));
    restoreState(QByteArray::fromBase64(pSettings->getValueString(
        ConfigKey("[MainWindow]", "state")).toUtf8()));

    setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));
    slotUpdateWindowTitle(TrackPointer());
}

void WMainWindow::initializeKeyboard() {
    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();
    QString resourcePath = pConfig->getResourcePath();

    // Set the default value in settings file
    if (pConfig->getValueString(ConfigKey("[Keyboard]","Enabled")).length() == 0)
        pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(1));

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard = QDir(CmdlineArgs::Instance().getSettingsPath()).filePath("Custom.kbd.cfg");

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

QDialog::DialogCode WMainWindow::soundDeviceErrorDlg(
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

    auto pSoundManager = m_pCore->soundManager();

    while (true)
    {
        msgBox.exec();

        if (msgBox.clickedButton() == retryButton) {
            pSoundManager->clearAndQueryDevices();
            *retryClicked = true;
            return QDialog::Accepted;
        } else if (msgBox.clickedButton() == wikiButton) {
            QDesktopServices::openUrl(QUrl(
                "http://mixxx.org/wiki/doku.php/troubleshooting"
                "#i_can_t_select_my_sound_card_in_the_sound_hardware_preferences"));
            wikiButton->setEnabled(false);
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();

            pSoundManager->clearAndQueryDevices();
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

QDialog::DialogCode WMainWindow::soundDeviceBusyDlg(bool* retryClicked) {
    auto pSoundManager = m_pCore->soundManager();
    QString title(tr("Sound Device Busy"));
    QString text(
            "<html> <p>" %
            tr("Mixxx was unable to open all the configured sound devices.") +
            "</p> <p>" %
            pSoundManager->getErrorDeviceName() %
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


QDialog::DialogCode WMainWindow::soundDeviceErrorMsgDlg(
        SoundDeviceError err, bool* retryClicked) {
    auto pSoundManager = m_pCore->soundManager();
    QString title(tr("Sound Device Error"));
    QString text(
            "<html> <p>" %
            tr("Mixxx was unable to open all the configured sound devices.") +
            "</p> <p>" %
            pSoundManager->getLastErrorMessage(err).replace("\n", "<br/>") %
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

QDialog::DialogCode WMainWindow::noOutputDlg(bool* continueClicked) {
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

void WMainWindow::slotUpdateWindowTitle(TrackPointer pTrack) {
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

void WMainWindow::createMenuBar() {
    ScopedTimer t("WMainWindow::createMenuBar");
    DEBUG_ASSERT(m_pKbdConfig != nullptr);
    m_pMenuBar = new WMainMenuBar(this, m_pCore->settingsManager()->settings(),
                                  m_pKbdConfig);
    setMenuBar(m_pMenuBar);
}

void WMainWindow::connectMenuBar() {
    ScopedTimer t("WMainWindow::connectMenuBar");
    connect(this, SIGNAL(newSkinLoaded()),
            m_pMenuBar, SLOT(onNewSkinLoaded()));

    // Misc
    connect(m_pMenuBar, SIGNAL(quit()), this, SLOT(close()));
    connect(m_pMenuBar, SIGNAL(showPreferences()),
            this, SLOT(slotOptionsPreferences()));
    connect(m_pMenuBar, SIGNAL(loadTrackToDeck(int)),
            this, SLOT(slotFileLoadSongPlayer(int)));

    // Fullscreen
    connect(m_pMenuBar, SIGNAL(toggleFullScreen(bool)),
            this, SLOT(slotViewFullScreen(bool)));
    connect(this, SIGNAL(fullScreenChanged(bool)),
            m_pMenuBar, SLOT(onFullScreenStateChange(bool)));

    // Keyboard shortcuts
    connect(m_pMenuBar, SIGNAL(toggleKeyboardShortcuts(bool)),
            this, SLOT(slotOptionsKeyboard(bool)));

    // Help
    connect(m_pMenuBar, SIGNAL(showAbout()),
            this, SLOT(slotHelpAbout()));

    // Developer
    connect(m_pMenuBar, SIGNAL(reloadSkin()),
            this, SLOT(rebootMixxxView()));
    connect(m_pMenuBar, SIGNAL(toggleDeveloperTools(bool)),
            this, SLOT(slotDeveloperTools(bool)));

    auto pRecordingManager = m_pCore->recordingManager();
    if (pRecordingManager) {
        connect(pRecordingManager.get(), SIGNAL(isRecording(bool)),
                m_pMenuBar, SLOT(onRecordingStateChange(bool)));
        connect(m_pMenuBar, SIGNAL(toggleRecording(bool)),
                pRecordingManager.get(), SLOT(slotSetRecording(bool)));
        m_pMenuBar->onRecordingStateChange(pRecordingManager->isRecordingActive());
    }

#ifdef __BROADCAST__
    auto pBroadcastManager = m_pCore->broadcastManager();
    if (pBroadcastManager) {
        connect(pBroadcastManager.get(), SIGNAL(broadcastEnabled(bool)),
                m_pMenuBar, SLOT(onBroadcastingStateChange(bool)));
        connect(m_pMenuBar, SIGNAL(toggleBroadcasting(bool)),
                pBroadcastManager.get(), SLOT(setEnabled(bool)));
        m_pMenuBar->onBroadcastingStateChange(pBroadcastManager->isEnabled());
    }
#endif

#ifdef __VINYLCONTROL__
    auto pVinylControlManager = m_pCore->vinylControlManager();
    if (pVinylControlManager) {
        connect(m_pMenuBar, SIGNAL(toggleVinylControl(int)),
                pVinylControlManager.get(), SLOT(toggleVinylControl(int)));
        connect(pVinylControlManager.get(), SIGNAL(vinylControlDeckEnabled(int, bool)),
                m_pMenuBar, SLOT(onVinylControlDeckEnabledStateChange(int, bool)));
    }
#endif

    auto pPlayerManager = m_pCore->playerManager();
    if (pPlayerManager) {
        connect(pPlayerManager.get(), SIGNAL(numberOfDecksChanged(int)),
                m_pMenuBar, SLOT(onNumberOfDecksChanged(int)));
        m_pMenuBar->onNumberOfDecksChanged(pPlayerManager->numberOfDecks());
    }

    auto pLibraryManager = m_pCore->libraryManager();
    if (pLibraryManager) {
        connect(m_pMenuBar, SIGNAL(createCrate()),
                pLibraryManager.get(), SLOT(slotCreateCrate()));
        connect(m_pMenuBar, SIGNAL(createPlaylist()),
                pLibraryManager.get(), SLOT(slotCreatePlaylist()));
        connect(pLibraryManager.get(), SIGNAL(scanStarted()),
                m_pMenuBar, SLOT(onLibraryScanStarted()));
        connect(pLibraryManager.get(), SIGNAL(scanFinished()),
                m_pMenuBar, SLOT(onLibraryScanFinished()));
        connect(m_pMenuBar, SIGNAL(rescanLibrary()),
                pLibraryManager.get(), SLOT(scan()));
    }

}

void WMainWindow::slotFileLoadSongPlayer(int deck) {
    auto pPlayerManager = m_pCore->playerManager();
    QString group = pPlayerManager->groupForDeck(deck-1);

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

    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();
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

        pPlayerManager->slotLoadToDeck(trackPath, deck);
    }
}

void WMainWindow::slotOptionsKeyboard(bool toggle) {
    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();
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

void WMainWindow::slotDeveloperTools(bool visible) {
    if (visible) {
        if (m_pDeveloperToolsDlg == nullptr) {
            UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();
            m_pDeveloperToolsDlg = new DlgDeveloperTools(this, pConfig);
            connect(m_pDeveloperToolsDlg, SIGNAL(destroyed()),
                    this, SLOT(slotDeveloperToolsClosed()));
            connect(this, SIGNAL(closeDeveloperToolsDlgChecked(int)),
                    m_pDeveloperToolsDlg, SLOT(done(int)));
            connect(m_pDeveloperToolsDlg, SIGNAL(destroyed()),
                    m_pMenuBar, SLOT(onDeveloperToolsHidden()));
        }
        m_pMenuBar->onDeveloperToolsShown();
        m_pDeveloperToolsDlg->show();
        m_pDeveloperToolsDlg->activateWindow();
    } else {
        emit(closeDeveloperToolsDlgChecked(0));
    }
}

void WMainWindow::slotDeveloperToolsClosed() {
    m_pDeveloperToolsDlg = NULL;
}

void WMainWindow::slotViewFullScreen(bool toggle) {
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
    emit(fullScreenChanged(toggle));
}

void WMainWindow::slotOptionsPreferences() {
    m_pPrefDlg->show();
    m_pPrefDlg->raise();
    m_pPrefDlg->activateWindow();
}

void WMainWindow::slotNoVinylControlInputConfigured() {
    QMessageBox::warning(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this vinyl control.\n"
           "Please select an input device in the sound hardware preferences first."),
        QMessageBox::Ok, QMessageBox::Ok);
    m_pPrefDlg->show();
    m_pPrefDlg->showSoundHardwarePage();
}

void WMainWindow::slotNoDeckPassthroughInputConfigured() {
    QMessageBox::warning(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this passthrough control.\n"
           "Please select an input device in the sound hardware preferences first."),
        QMessageBox::Ok, QMessageBox::Ok);
    m_pPrefDlg->show();
    m_pPrefDlg->showSoundHardwarePage();
}

void WMainWindow::slotNoMicrophoneInputConfigured() {
    QMessageBox::warning(
        this,
        Version::applicationName(),
        tr("There is no input device selected for this microphone.\n"
           "Please select an input device in the sound hardware preferences first."),
        QMessageBox::Ok, QMessageBox::Ok);
    m_pPrefDlg->show();
    m_pPrefDlg->showSoundHardwarePage();
}

void WMainWindow::slotChangedPlayingDeck(int deck) {
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON_PLAY) {
        if (deck==-1) {
            // If no deck is playing, allow the screensaver to run.
            mixxx::ScreenSaverHelper::uninhibit();
        } else {
            mixxx::ScreenSaverHelper::inhibit();
        }
    }
}

void WMainWindow::slotHelpAbout() {
    DlgAbout* about = new DlgAbout(this);
    about->show();
}

void WMainWindow::setToolTipsCfg(mixxx::TooltipsPreference tt) {
    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();
    pConfig->set(ConfigKey("[Controls]","Tooltips"),
                 ConfigValue(static_cast<int>(tt)));
    m_toolTipsCfg = tt;
}

void WMainWindow::rebootMixxxView() {
    qDebug() << "Now in rebootMixxxView...";

    QPoint initPosition = pos();
    // frameSize()  : Window size including all borders and only if the window manager works.
    // size() : Window without the borders nor title, but including the Menu!
    // centralWidget()->size() : Size of the internal window Widget.
    QSize initSize;
    QWidget* pWidget = centralWidget(); // can be null if previous skin loading fails
    if (pWidget) {
        initSize = centralWidget()->size();
    }

    // We need to tell the menu bar that we are about to delete the old skin and
    // create a new one. It holds "visibility" controls (e.g. "Show Samplers")
    // that need to be deleted -- otherwise we can't tell what features the skin
    // supports since the controls from the previous skin will be left over.
    m_pMenuBar->onNewSkinAboutToLoad();


    if (QWidget* pOldSkinWidget = centralWidget()) {
        pOldSkinWidget->hide();
        WaveformWidgetFactory::instance()->destroyWidgets();
        delete pOldSkinWidget;
    }

    // Workaround for changing skins while fullscreen, just go out of fullscreen
    // mode. If you change skins while in fullscreen (on Linux, at least) the
    // window returns to 0,0 but and the backdrop disappears so it looks as if
    // it is not fullscreen, but acts as if it is.
    bool wasFullScreen = isFullScreen();
    slotViewFullScreen(false);

    // Load skin to a QWidget that we set as the central widget. Assignment
    // intentional in next line.
    QWidget* pSkinWidget = nullptr;
    if (!(pSkinWidget = m_skinLoader.loadConfiguredSkin(this,
                                                        m_pKeyboard,
                                                        m_pCore->playerManager(),
                                                        m_pCore->controllerManager(),
                                                        m_pCore->libraryManager(),
                                                        m_pCore->vinylControlManager(),
                                                        m_pCore->effectsManager(),
                                                        m_pCore->recordingManager()))) {

        QMessageBox::critical(this,
                              tr("Error in skin file"),
                              tr("The selected skin cannot be loaded."));
        return;
    }

    setCentralWidget(pSkinWidget);
    adjustSize();

    if (wasFullScreen) {
        slotViewFullScreen(true);
    } else if (!initSize.isEmpty()) {
        // Not all OSs and/or window managers keep the window inside of the screen, so force it.
        int newX = initPosition.x() + (initSize.width() - pSkinWidget->width()) / 2;
        int newY = initPosition.y() + (initSize.height() - pSkinWidget->height()) / 2;
        newX = std::max(0, std::min(newX, QApplication::desktop()->screenGeometry().width() - pSkinWidget->width()));
        newY = std::max(0, std::min(newY, QApplication::desktop()->screenGeometry().height() - pSkinWidget->height()));
        move(newX,newY);
    }

    qDebug() << "rebootMixxxView DONE";
    emit(newSkinLoaded());
}

bool WMainWindow::eventFilter(QObject* obj, QEvent* event) {
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

bool WMainWindow::event(QEvent* e) {
    switch(e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        // If the touch event falls through to the main widget, no touch widget
        // was touched, so we resend it as a mouse event.
        // We have to accept it here, so QApplication will continue to deliver
        // the following events of this touch point as well.
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(e);
        touchEvent->accept();
        return true;
    }
    default:
        break;
    }
    return QWidget::event(e);
}

void WMainWindow::closeEvent(QCloseEvent *event) {
    // WARNING: We can receive a CloseEvent while only partially
    // initialized. This is because we call QApplication::processEvents to
    // render LaunchImage progress in the constructor.
    if (!confirmExit()) {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}

void WMainWindow::checkDirectRendering() {
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

    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();

    if (!factory->isOpenGLAvailable() &&
        pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes")) {
        QMessageBox::warning(
            0, tr("OpenGL Direct Rendering"),
            tr("Direct rendering is not enabled on your machine.<br><br>"
               "This means that the waveform displays will be very<br>"
               "<b>slow and may tax your CPU heavily</b>. Either update your<br>"
               "configuration to enable direct rendering, or disable<br>"
               "the waveform displays in the Mixxx preferences by selecting<br>"
               "\"Empty\" as the waveform display in the 'Interface' section.<br><br>"
               "NOTE: If you use NVIDIA hardware,<br>"
               "direct rendering may not be present, but you should<br>"
               "not experience degraded performance."));
        pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), QString("yes"));
    }
}

bool WMainWindow::confirmExit() {
    bool playing(false);
    bool playingSampler(false);

    auto pPlayerManager = m_pCore->playerManager();
    unsigned int deckCount = pPlayerManager->numDecks();
    unsigned int samplerCount = pPlayerManager->numSamplers();
    for (unsigned int i = 0; i < deckCount; ++i) {
        if (ControlObject::get(
                ConfigKey(PlayerManager::groupForDeck(i), "play"))) {
            playing = true;
            break;
        }
    }
    for (unsigned int i = 0; i < samplerCount; ++i) {
        if (ControlObject::get(
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

void WMainWindow::setInhibitScreensaver(mixxx::ScreenSaverPreference newInhibit)
{
    UserSettingsPointer pConfig = m_pCore->settingsManager()->settings();

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

mixxx::ScreenSaverPreference WMainWindow::getInhibitScreensaver()
{
    return m_inhibitScreensaver;
}
