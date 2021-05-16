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

#include "defs_urls.h"
#include "dialog/dlgabout.h"
#include "dialog/dlgdevelopertools.h"
#include "dialog/dlgkeywheel.h"
#include "effects/builtin/builtinbackend.h"
#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "moc_mixxx.cpp"
#include "preferences/constants.h"
#include "preferences/dialog/dlgprefeq.h"
#include "preferences/dialog/dlgpreferences.h"
#ifdef __LILV__
#include "effects/lv2/lv2backend.h"
#endif
#ifdef __BROADCAST__
#include "broadcast/broadcastmanager.h"
#endif
#include "control/controlpushbutton.h"
#include "controllers/controllermanager.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "database/mixxxdb.h"
#include "library/coverartcache.h"
#include "library/library.h"
#include "library/library_preferences.h"
#ifdef __ENGINEPRIME__
#include "library/export/libraryexporter.h"
#endif
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
#include "util/versionstore.h"
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

MixxxMainWindow::MixxxMainWindow(
        QApplication* pApp, std::shared_ptr<mixxx::CoreServices> pCoreServices)
        : m_pCoreServices(pCoreServices),
          m_pCentralWidget(nullptr),
          m_pLaunchImage(nullptr),
          m_pGuiTick(nullptr),
          m_pDeveloperToolsDlg(nullptr),
          m_pPrefDlg(nullptr),
          m_pKeywheel(nullptr),
#ifdef __ENGINEPRIME__
          m_pLibraryExporter(nullptr),
#endif
          m_toolTipsCfg(mixxx::TooltipsPreference::TOOLTIPS_ON) {
    DEBUG_ASSERT(pApp);
    DEBUG_ASSERT(pCoreServices);
    m_pCoreServices->initializeSettings();
    m_pCoreServices->initializeKeyboard();
    // These depend on the settings
    createMenuBar();
    m_pMenuBar->hide();

    initializeWindow();

    // Show launch image immediately so the user knows Mixxx is starting
    m_pSkinLoader = std::make_unique<SkinLoader>(m_pCoreServices->getSettings());
    m_pLaunchImage = m_pSkinLoader->loadLaunchImage(this);
    m_pCentralWidget = (QWidget*)m_pLaunchImage;
    setCentralWidget(m_pCentralWidget);

    show();
    pApp->processEvents();

    m_pGuiTick = new GuiTick();
    m_pVisualsManager = new VisualsManager();

    connect(
            m_pCoreServices.get(),
            &mixxx::CoreServices::initializationProgressUpdate,
            this,
            &MixxxMainWindow::initializationProgressUpdate);

    // Inhibit the screensaver if the option is set. (Do it before creating the preferences dialog)
    UserSettingsPointer pConfig = m_pCoreServices->getSettings();
    int inhibit = pConfig->getValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), -1);
    if (inhibit == -1) {
        inhibit = static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON);
        pConfig->setValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), inhibit);
    }
    m_inhibitScreensaver = static_cast<mixxx::ScreenSaverPreference>(inhibit);
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    }

    m_pCoreServices->initialize(pApp);

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = static_cast<mixxx::TooltipsPreference>(
            pConfig->getValue(ConfigKey("[Controls]", "Tooltips"),
                    static_cast<int>(mixxx::TooltipsPreference::TOOLTIPS_ON)));

#ifdef __ENGINEPRIME__
    // Initialise library exporter
    // This has to be done before switching to fullscreen
    m_pLibraryExporter = m_pCoreServices->getLibrary()->makeLibraryExporter(this);
    connect(m_pCoreServices->getLibrary().get(),
            &Library::exportLibrary,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExport);
    connect(m_pCoreServices->getLibrary().get(),
            &Library::exportCrate,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExportWithInitialCrate);
#endif

    // Turn on fullscreen mode
    // if we were told to start in fullscreen mode on the command-line
    // or if the user chose to always start in fullscreen mode.
    // Remember to refresh the Fullscreen menu item after connectMenuBar()
    bool fullscreenPref = m_pCoreServices->getSettings()->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if (CmdlineArgs::Instance().getStartInFullscreen() || fullscreenPref) {
        slotViewFullScreen(true);
    }

    initializationProgressUpdate(65, tr("skin"));

    installEventFilter(m_pCoreServices->getKeyboardEventFilter().get());

    DEBUG_ASSERT(m_pCoreServices->getPlayerManager());
    const QStringList visualGroups = m_pCoreServices->getPlayerManager()->getVisualPlayerGroups();
    for (const QString& group : visualGroups) {
        m_pVisualsManager->addDeck(group);
    }

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
    WaveformWidgetFactory::instance()->setConfig(m_pCoreServices->getSettings());
    WaveformWidgetFactory::instance()->startVSync(m_pGuiTick, m_pVisualsManager);

    connect(this,
            &MixxxMainWindow::skinLoaded,
            m_pCoreServices->getLibrary().get(),
            &Library::onSkinLoadFinished);

    connect(this,
            &MixxxMainWindow::skinLoaded,
            WaveformWidgetFactory::instance(),
            &WaveformWidgetFactory::slotSkinLoaded);

    // Initialize preference dialog
    m_pPrefDlg = new DlgPreferences(
            this,
            m_pSkinLoader,
            m_pCoreServices->getSoundManager(),
            m_pCoreServices->getPlayerManager(),
            m_pCoreServices->getControllerManager(),
            m_pCoreServices->getVinylControlManager(),
            m_pCoreServices->getLV2Backend(),
            m_pCoreServices->getEffectsManager(),
            m_pCoreServices->getSettingsManager(),
            m_pCoreServices->getLibrary());
    m_pPrefDlg->setWindowIcon(QIcon(":/images/mixxx_icon.svg"));
    m_pPrefDlg->setHidden(true);

    // Connect signals to the menubar. Should be done before emit newSkinLoaded.
    connectMenuBar();

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
    } else {
        m_pMenuBar->setStyleSheet(m_pCentralWidget->styleSheet());
    }

    // Check direct rendering and warn user if they don't have it
    if (!CmdlineArgs::Instance().getSafeMode()) {
        checkDirectRendering();
    }

    // Install an event filter to catch certain QT events, such as tooltips.
    // This allows us to turn off tooltips.
    pApp->installEventFilter(this); // The eventfilter is located in this
                                    // Mixxx class as a callback.

    // Try open player device If that fails, the preference panel is opened.
    bool retryClicked;
    do {
        retryClicked = false;
        SoundDeviceError result = m_pCoreServices->getSoundManager()->setupDevices();
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
    while (m_pCoreServices->getSoundManager()->getConfig().getOutputs().count() == 0) {
        // Exit when we press the Exit button in the noSoundDlg dialog
        // only call it if result != OK
        bool continueClicked = false;
        if (noOutputDlg(&continueClicked) != QDialog::Accepted) {
            exit(0);
        }
        if (continueClicked) {
            break;
        }
    }

    // this has to be after the OpenGL widgets are created or depending on a
    // million different variables the first waveform may be horribly
    // corrupted. See bug 521509 -- bkgood ?? -- vrince
    setCentralWidget(m_pCentralWidget);

    // Show the menubar after the launch image is replaced by the skin widget,
    // otherwise it would shift the launch image shortly before the skin is visible.
    m_pMenuBar->show();

    // The launch image widget is automatically disposed, but we still have a
    // pointer to it.
    m_pLaunchImage = nullptr;

    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noMicrophoneInputConfigured,
            this,
            &MixxxMainWindow::slotNoMicrophoneInputConfigured);
    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noAuxiliaryInputConfigured,
            this,
            &MixxxMainWindow::slotNoAuxiliaryInputConfigured);
    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noDeckPassthroughInputConfigured,
            this,
            &MixxxMainWindow::slotNoDeckPassthroughInputConfigured);
    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noVinylControlInputConfigured,
            this,
            &MixxxMainWindow::slotNoVinylControlInputConfigured);

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &MixxxMainWindow::slotUpdateWindowTitle);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingDeckChanged,
            this,
            &MixxxMainWindow::slotChangedPlayingDeck);
}

MixxxMainWindow::~MixxxMainWindow() {
    Timer t("~MixxxMainWindow");
    t.start();

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    // Save the current window state (position, maximized, etc)
    // Note(ronso0): Unfortunately saveGeometry() also stores the fullscreen state.
    // On next start restoreGeometry would enable fullscreen mode even though that
    // might not be requested (no '--fullscreen' command line arg and
    // [Config],StartInFullscreen is '0'.
    // https://bugs.launchpad.net/mixxx/+bug/1882474
    // https://bugs.launchpad.net/mixxx/+bug/1909485
    // So let's quit fullscreen if StartInFullscreen is not checked in Preferences.
    bool fullscreenPref = m_pCoreServices->getSettings()->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if (isFullScreen() && !fullscreenPref) {
        slotViewFullScreen(false);
        // After returning from fullscreen the main window incl. window decoration
        // may be too large for the screen.
        // Maximize the window so we can store a geometry that fits the screen.
        showMaximized();
    }
    m_pCoreServices->getSettings()->set(ConfigKey("[MainWindow]", "geometry"),
            QString(saveGeometry().toBase64()));
    m_pCoreServices->getSettings()->set(ConfigKey("[MainWindow]", "state"),
            QString(saveState().toBase64()));

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

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting DeveloperToolsDlg";
    if (m_pDeveloperToolsDlg) {
        delete m_pDeveloperToolsDlg;
    }

#ifdef __ENGINEPRIME__
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting LibraryExporter";
    m_pLibraryExporter.reset();
#endif

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting DlgPreferences";
    delete m_pPrefDlg;

    WaveformWidgetFactory::destroy();

    delete m_pGuiTick;
    delete m_pVisualsManager;

    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    m_pCoreServices->shutdown();
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
    restoreGeometry(QByteArray::fromBase64(
            m_pCoreServices->getSettings()
                    ->getValueString(ConfigKey("[MainWindow]", "geometry"))
                    .toUtf8()));
    restoreState(QByteArray::fromBase64(
            m_pCoreServices->getSettings()
                    ->getValueString(ConfigKey("[MainWindow]", "state"))
                    .toUtf8()));

    setWindowIcon(QIcon(":/images/mixxx_icon.svg"));
    slotUpdateWindowTitle(TrackPointer());
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
            m_pCoreServices->getSoundManager()->clearAndQueryDevices();
            *retryClicked = true;
            return QDialog::Accepted;
        } else if (msgBox.clickedButton() == wikiButton) {
            QDesktopServices::openUrl(QUrl(MIXXX_WIKI_TROUBLESHOOTING_SOUND_URL));
            wikiButton->setEnabled(false);
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();

            m_pCoreServices->getSoundManager()->clearAndQueryDevices();
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
                    m_pCoreServices->getSoundManager()->getErrorDeviceName() %
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
                    "</ul></html>");
    return soundDeviceErrorDlg(title, text, retryClicked);
}


QDialog::DialogCode MixxxMainWindow::soundDeviceErrorMsgDlg(
        SoundDeviceError err, bool* retryClicked) {
    QString title(tr("Sound Device Error"));
    QString text("<html> <p>" %
                    tr("Mixxx was unable to open all the configured sound "
                       "devices.") +
            "</p> <p>" %
                    m_pCoreServices->getSoundManager()
                            ->getLastErrorMessage(err)
                            .replace("\n", "<br/>") %
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
                    "</ul></html>");
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
    QString appTitle = VersionStore::applicationName();

    // If we have a track, use getInfo() to format a summary string and prepend
    // it to the title.
    // TODO(rryan): Does this violate Mac App Store policies?
    if (pTrack) {
        QString trackInfo = pTrack->getInfo();
        if (!trackInfo.isEmpty()) {
            appTitle = QString("%1 | %2").arg(trackInfo, appTitle);
        }
    }
    this->setWindowTitle(appTitle);
}

void MixxxMainWindow::createMenuBar() {
    ScopedTimer t("MixxxMainWindow::createMenuBar");
    DEBUG_ASSERT(m_pCoreServices->getKeyboardConfig());
    m_pMenuBar = make_parented<WMainMenuBar>(
            this, m_pCoreServices->getSettings(), m_pCoreServices->getKeyboardConfig().get());
    if (m_pCentralWidget) {
        m_pMenuBar->setStyleSheet(m_pCentralWidget->styleSheet());
    }
    setMenuBar(m_pMenuBar);
}

void MixxxMainWindow::connectMenuBar() {
    // This function might be invoked multiple times on startup
    // so all connections must be unique!

    ScopedTimer t("MixxxMainWindow::connectMenuBar");
    connect(this,
            &MixxxMainWindow::skinLoaded,
            m_pMenuBar,
            &WMainMenuBar::onNewSkinLoaded,
            Qt::UniqueConnection);

    // Misc
    connect(m_pMenuBar,
            &WMainMenuBar::quit,
            this,
            &MixxxMainWindow::close,
            Qt::UniqueConnection);
    connect(m_pMenuBar,
            &WMainMenuBar::showPreferences,
            this,
            &MixxxMainWindow::slotOptionsPreferences,
            Qt::UniqueConnection);
    connect(m_pMenuBar,
            &WMainMenuBar::loadTrackToDeck,
            this,
            &MixxxMainWindow::slotFileLoadSongPlayer,
            Qt::UniqueConnection);

    connect(m_pMenuBar,
            &WMainMenuBar::showKeywheel,
            this,
            &MixxxMainWindow::slotShowKeywheel);

    // Fullscreen
    connect(m_pMenuBar,
            &WMainMenuBar::toggleFullScreen,
            this,
            &MixxxMainWindow::slotViewFullScreen,
            Qt::UniqueConnection);
    connect(this,
            &MixxxMainWindow::fullScreenChanged,
            m_pMenuBar,
            &WMainMenuBar::onFullScreenStateChange,
            Qt::UniqueConnection);
    // Refresh the Fullscreen checkbox for the case we went fullscreen earlier
    m_pMenuBar->onFullScreenStateChange(isFullScreen());

    // Keyboard shortcuts
    connect(m_pMenuBar,
            &WMainMenuBar::toggleKeyboardShortcuts,
            m_pCoreServices.get(),
            &mixxx::CoreServices::slotOptionsKeyboard,
            Qt::UniqueConnection);

    // Help
    connect(m_pMenuBar,
            &WMainMenuBar::showAbout,
            this,
            &MixxxMainWindow::slotHelpAbout,
            Qt::UniqueConnection);

    // Developer
    connect(m_pMenuBar,
            &WMainMenuBar::reloadSkin,
            this,
            &MixxxMainWindow::rebootMixxxView,
            Qt::UniqueConnection);
    connect(m_pMenuBar,
            &WMainMenuBar::toggleDeveloperTools,
            this,
            &MixxxMainWindow::slotDeveloperTools,
            Qt::UniqueConnection);

    if (m_pCoreServices->getRecordingManager()) {
        connect(m_pCoreServices->getRecordingManager().get(),
                &RecordingManager::isRecording,
                m_pMenuBar,
                &WMainMenuBar::onRecordingStateChange,
                Qt::UniqueConnection);
        connect(m_pMenuBar,
                &WMainMenuBar::toggleRecording,
                m_pCoreServices->getRecordingManager().get(),
                &RecordingManager::slotSetRecording,
                Qt::UniqueConnection);
        m_pMenuBar->onRecordingStateChange(
                m_pCoreServices->getRecordingManager()->isRecordingActive());
    }

#ifdef __BROADCAST__
    if (m_pCoreServices->getBroadcastManager()) {
        connect(m_pCoreServices->getBroadcastManager().get(),
                &BroadcastManager::broadcastEnabled,
                m_pMenuBar,
                &WMainMenuBar::onBroadcastingStateChange,
                Qt::UniqueConnection);
        connect(m_pMenuBar,
                &WMainMenuBar::toggleBroadcasting,
                m_pCoreServices->getBroadcastManager().get(),
                &BroadcastManager::setEnabled,
                Qt::UniqueConnection);
        m_pMenuBar->onBroadcastingStateChange(m_pCoreServices->getBroadcastManager()->isEnabled());
    }
#endif

#ifdef __VINYLCONTROL__
    if (m_pCoreServices->getVinylControlManager()) {
        connect(m_pMenuBar,
                &WMainMenuBar::toggleVinylControl,
                m_pCoreServices->getVinylControlManager().get(),
                &VinylControlManager::toggleVinylControl,
                Qt::UniqueConnection);
        connect(m_pCoreServices->getVinylControlManager().get(),
                &VinylControlManager::vinylControlDeckEnabled,
                m_pMenuBar,
                &WMainMenuBar::onVinylControlDeckEnabledStateChange,
                Qt::UniqueConnection);
    }
#endif

    if (m_pCoreServices->getPlayerManager()) {
        connect(m_pCoreServices->getPlayerManager().get(),
                &PlayerManager::numberOfDecksChanged,
                m_pMenuBar,
                &WMainMenuBar::onNumberOfDecksChanged,
                Qt::UniqueConnection);
        m_pMenuBar->onNumberOfDecksChanged(m_pCoreServices->getPlayerManager()->numberOfDecks());
    }

    if (m_pCoreServices->getTrackCollectionManager()) {
        connect(m_pMenuBar,
                &WMainMenuBar::rescanLibrary,
                m_pCoreServices->getTrackCollectionManager().get(),
                &TrackCollectionManager::startLibraryScan,
                Qt::UniqueConnection);
        connect(m_pCoreServices->getTrackCollectionManager().get(),
                &TrackCollectionManager::libraryScanStarted,
                m_pMenuBar,
                &WMainMenuBar::onLibraryScanStarted,
                Qt::UniqueConnection);
        connect(m_pCoreServices->getTrackCollectionManager().get(),
                &TrackCollectionManager::libraryScanFinished,
                m_pMenuBar,
                &WMainMenuBar::onLibraryScanFinished,
                Qt::UniqueConnection);
    }

    if (m_pCoreServices->getLibrary()) {
        connect(m_pMenuBar,
                &WMainMenuBar::createCrate,
                m_pCoreServices->getLibrary().get(),
                &Library::slotCreateCrate,
                Qt::UniqueConnection);
        connect(m_pMenuBar,
                &WMainMenuBar::createPlaylist,
                m_pCoreServices->getLibrary().get(),
                &Library::slotCreatePlaylist,
                Qt::UniqueConnection);
    }

#ifdef __ENGINEPRIME__
    DEBUG_ASSERT(m_pLibraryExporter);
    connect(m_pMenuBar,
            &WMainMenuBar::exportLibrary,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExport,
            Qt::UniqueConnection);
#endif
}

void MixxxMainWindow::slotFileLoadSongPlayer(int deck) {
    QString group = m_pCoreServices->getPlayerManager()->groupForDeck(deck - 1);

    QString loadTrackText = tr("Load track to Deck %1").arg(QString::number(deck));
    QString deckWarningMessage = tr("Deck %1 is currently playing a track.")
            .arg(QString::number(deck));
    QString areYouSure = tr("Are you sure you want to load a new track?");

    if (ControlObject::get(ConfigKey(group, "play")) > 0.0) {
        int ret = QMessageBox::warning(this,
                VersionStore::applicationName(),
                deckWarningMessage + "\n" + areYouSure,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    UserSettingsPointer pConfig = m_pCoreServices->getSettings();
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
        mixxx::FileInfo fileInfo(trackPath);
        Sandbox::createSecurityToken(&fileInfo);

        m_pCoreServices->getPlayerManager()->slotLoadToDeck(trackPath, deck);
    }
}

void MixxxMainWindow::slotDeveloperTools(bool visible) {
    if (visible) {
        if (m_pDeveloperToolsDlg == nullptr) {
            UserSettingsPointer pConfig = m_pCoreServices->getSettings();
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
    m_pDeveloperToolsDlg = nullptr;
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
            VersionStore::applicationName(),
            tr("There is no input device selected for this vinyl control.\n"
               "Please select an input device in the sound hardware preferences first."),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoDeckPassthroughInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::warning(
            this,
            VersionStore::applicationName(),
            tr("There is no input device selected for this passthrough control.\n"
               "Please select an input device in the sound hardware preferences first."),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoMicrophoneInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::question(
            this,
            VersionStore::applicationName(),
            tr("There is no input device selected for this microphone.\n"
               "Do you want to select an input device?"),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
    if (btn == QMessageBox::Ok) {
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotNoAuxiliaryInputConfigured() {
    QMessageBox::StandardButton btn = QMessageBox::question(
            this,
            VersionStore::applicationName(),
            tr("There is no input device selected for this auxiliary.\n"
               "Do you want to select an input device?"),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
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

void MixxxMainWindow::slotShowKeywheel(bool toggle) {
    if (!m_pKeywheel) {
        m_pKeywheel = make_parented<DlgKeywheel>(this, m_pCoreServices->getSettings());
        // uncheck the menu item on window close
        connect(m_pKeywheel.get(),
                &DlgKeywheel::finished,
                m_pMenuBar,
                &WMainMenuBar::onKeywheelChange);
    }
    if (toggle) {
        m_pKeywheel->show();
        m_pKeywheel->raise();
    } else {
        m_pKeywheel->hide();
    }
}

void MixxxMainWindow::setToolTipsCfg(mixxx::TooltipsPreference tt) {
    UserSettingsPointer pConfig = m_pCoreServices->getSettings();
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
    m_pMenuBar->setStyleSheet(m_pCentralWidget->styleSheet());

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
}

bool MixxxMainWindow::loadConfiguredSkin() {
    // TODO: use std::shared_ptr throughout skin widgets instead of these hacky get() calls
    m_pCentralWidget = m_pSkinLoader->loadConfiguredSkin(this,
            &m_skinCreatedControls,
            m_pCoreServices->getKeyboardEventFilter().get(),
            m_pCoreServices->getPlayerManager().get(),
            m_pCoreServices->getControllerManager().get(),
            m_pCoreServices->getLibrary().get(),
            m_pCoreServices->getVinylControlManager().get(),
            m_pCoreServices->getEffectsManager().get(),
            m_pCoreServices->getRecordingManager().get());
    if (centralWidget() == m_pLaunchImage) {
        initializationProgressUpdate(100, "");
    }
    emit skinLoaded();
    return m_pCentralWidget != nullptr;
}

bool MixxxMainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        // return true for no tool tips
        switch (m_toolTipsCfg) {
            case mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY:
                if (dynamic_cast<WBaseWidget*>(obj) != nullptr) {
                    return true;
                }
                break;
            case mixxx::TooltipsPreference::TOOLTIPS_ON:
                break;
            case mixxx::TooltipsPreference::TOOLTIPS_OFF:
                return true;
            default:
                DEBUG_ASSERT(!"m_toolTipsCfg value unknown");
                return true;
        }
    }
    // standard event processing
    return QMainWindow::eventFilter(obj, event);
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
    if (!factory) {
        return;
    }

    UserSettingsPointer pConfig = m_pCoreServices->getSettings();

    if (!factory->isOpenGlAvailable() && !factory->isOpenGlesAvailable() &&
        pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes")) {
        QMessageBox::warning(nullptr,
                tr("OpenGL Direct Rendering"),
                tr("Direct rendering is not enabled on your machine.<br><br>"
                   "This means that the waveform displays will be very<br>"
                   "<b>slow and may tax your CPU heavily</b>. Either update "
                   "your<br>"
                   "configuration to enable direct rendering, or disable<br>"
                   "the waveform displays in the Mixxx preferences by "
                   "selecting<br>"
                   "\"Empty\" as the waveform display in the 'Interface' "
                   "section."));
        pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), QString("yes"));
    }
}

bool MixxxMainWindow::confirmExit() {
    bool playing(false);
    bool playingSampler(false);
    unsigned int deckCount = m_pCoreServices->getPlayerManager()->numDecks();
    unsigned int samplerCount = m_pCoreServices->getPlayerManager()->numSamplers();
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
    UserSettingsPointer pConfig = m_pCoreServices->getSettings();

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

void MixxxMainWindow::initializationProgressUpdate(int progress, const QString& serviceName) {
    if (m_pLaunchImage) {
        m_pLaunchImage->progress(progress, serviceName);
    }
    qApp->processEvents();
}
