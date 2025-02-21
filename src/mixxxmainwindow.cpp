#include "mixxxmainwindow.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QOpenGLContext>
#include <QUrl>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLFormat>
#endif

#ifdef __LINUX__
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#ifdef MIXXX_USE_QOPENGL
#include "widget/tooltipqopengl.h"
#include "widget/winitialglwidget.h"
#endif

#include "controllers/keyboard/keyboardeventfilter.h"
#include "coreservices.h"
#include "defs_urls.h"
#include "dialog/dlgabout.h"
#include "dialog/dlgdevelopertools.h"
#include "dialog/dlgkeywheel.h"
#include "moc_mixxxmainwindow.cpp"
#include "preferences/dialog/dlgpreferences.h"
#ifdef __BROADCAST__
#include "broadcast/broadcastmanager.h"
#endif
#include "control/controlindicatortimer.h"
#include "library/library.h"
#include "library/library_prefs.h"
#ifdef __ENGINEPRIME__
#include "library/export/libraryexporter.h"
#endif
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "recording/recordingmanager.h"
#include "skin/legacy/launchimage.h"
#include "skin/skinloader.h"
#include "soundio/soundmanager.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/debug.h"
#include "util/desktophelper.h"
#include "util/sandbox.h"
#include "util/scopedoverridecursor.h"
#include "util/timer.h"
#include "util/versionstore.h"
#include "waveform/guitick.h"
#include "waveform/sharedglcontext.h"
#include "waveform/visualsmanager.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wglwidget.h"
#include "widget/wmainmenubar.h"

#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrolmanager.h"
#endif

namespace {
#ifdef __LINUX__
// Detect if the desktop supports a global menu to decide whether we need to rebuild
// and reconnect the menu bar when switching to/from fullscreen mode.
// Compared to QMenuBar::isNativeMenuBar() (requires a set menu bar) and
// Qt::AA_DontUseNativeMenuBar, which may both change, this is way more reliable
// since it's rather unlikely that the Appmenu.Registrar service is unloaded/stopped
// while Mixxx is running.
// This is a reimplementation of QGenericUnixTheme > checkDBusGlobalMenuAvailable()
inline bool supportsGlobalMenu() {
#ifndef QT_NO_DBUS
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (const auto* pIface = conn.interface()) {
        return pIface->isServiceRegistered("com.canonical.AppMenu.Registrar");
    }
#endif
    return false;
}
#endif

const ConfigKey kHideMenuBarConfigKey = ConfigKey("[Config]", "hide_menubar");
const ConfigKey kMenuBarHintConfigKey = ConfigKey("[Config]", "show_menubar_hint");
} // namespace

MixxxMainWindow::MixxxMainWindow(std::shared_ptr<mixxx::CoreServices> pCoreServices)
        : m_pCoreServices(pCoreServices),
          m_pCentralWidget(nullptr),
          m_pLaunchImage(nullptr),
#ifndef __APPLE__
          m_prevState(Qt::WindowNoState),
#endif
          m_pGuiTick(nullptr),
#ifdef __LINUX__
          m_supportsGlobalMenuBar(supportsGlobalMenu()),
#endif
          m_inRebootMixxxView(false),
          m_pDeveloperToolsDlg(nullptr),
          m_pPrefDlg(nullptr),
          m_toolTipsCfg(mixxx::preferences::Tooltips::On) {
    DEBUG_ASSERT(pCoreServices);
    // These depend on the settings
#ifdef __LINUX__
    // If the desktop features a global menubar and we'll go fullscreen during
    // startup, set Qt::AA_DontUseNativeMenuBar so the menubar is placed in the
    // window like it's done in slotViewFullScreen(). On other desktops this
    // attribute has no effect. This is a safe alternative to setNativeMenuBar()
    // which can cause a crash when using menu shortcuts like Alt+F after resetting
    // the menubar. See https://github.com/mixxxdj/mixxx/issues/11320
    if (m_supportsGlobalMenuBar) {
        bool fullscreenPref = m_pCoreServices->getSettings()->getValue<bool>(
                ConfigKey("[Config]", "StartInFullscreen"));
        QApplication::setAttribute(
                Qt::AA_DontUseNativeMenuBar,
                CmdlineArgs::Instance().getStartInFullscreen() || fullscreenPref);
    }
#endif // __LINUX__
    createMenuBar();
    m_pMenuBar->hide();

    initializeWindow();

    // Show launch image immediately so the user knows Mixxx is starting
    m_pSkinLoader = std::make_unique<mixxx::skin::SkinLoader>(m_pCoreServices->getSettings());
    m_pLaunchImage = m_pSkinLoader->loadLaunchImage(this);
    m_pCentralWidget = (QWidget*)m_pLaunchImage;
    setCentralWidget(m_pCentralWidget);

    show();

    m_pGuiTick = new GuiTick();
    m_pVisualsManager = new VisualsManager();
}

#ifdef MIXXX_USE_QOPENGL
void MixxxMainWindow::initializeQOpenGL() {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 6 will nno longer crash if no GL is available and
    // QGLFormat::hasOpenGL() has been removed.
    if (!CmdlineArgs::Instance().getSafeMode() && QGLFormat::hasOpenGL()) {
#else
    if (!CmdlineArgs::Instance().getSafeMode()) {
#endif
        QOpenGLContext context;
        context.setFormat(WaveformWidgetFactory::getSurfaceFormat(m_pCoreServices->getSettings()));
        if (context.create()) {
            // This widget and its QOpenGLWindow will be used to query QOpenGL
            // information (version, driver, etc) in WaveformWidgetFactory.
            // The "SharedGLContext" terminology here doesn't really apply,
            // but allows us to take advantage of the existing classes.
            WInitialGLWidget* widget = new WInitialGLWidget(this);
            widget->setGeometry(QRect(0, 0, 3, 3));
            SharedGLContext::setWidget(widget);
            // When the widget's QOpenGLWindow has been initialized, we continue
            // with the actual initialization
            connect(widget, &WInitialGLWidget::onInitialized, this, &MixxxMainWindow::initialize);
            widget->show();
            return;
        }
        qDebug() << "QOpenGLContext::create() failed";
    }
    qInfo() << "Initializing without OpenGL";
    initialize();
}
#endif

void MixxxMainWindow::initialize() {
    m_pCoreServices->getControlIndicatorTimer()->setLegacyVsyncEnabled(true);

    UserSettingsPointer pConfig = m_pCoreServices->getSettings();

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = pConfig->getValue(
            ConfigKey("[Controls]", "Tooltips"),
            mixxx::preferences::Tooltips::On);
#ifdef MIXXX_USE_QOPENGL
    ToolTipQOpenGL::singleton().setActive(
            m_toolTipsCfg == mixxx::preferences::Tooltips::On);
#endif

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
    // The Fullscreen menu item is refreshed in connectMenuBar()
    bool fullscreenPref = m_pCoreServices->getSettings()->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if ((CmdlineArgs::Instance().getStartInFullscreen() || fullscreenPref) &&
            // could be we're fullscreen already after setGeomtery(previousGeometry)
            !isFullScreen()) {
        showFullScreen();
    }

    initializationProgressUpdate(65, tr("skin"));

    // Install an event filter to catch certain QT events, such as tooltips.
    // This allows us to turn off tooltips.
    installEventFilter(m_pCoreServices->getKeyboardEventFilter().get());

    auto pPlayerManager = m_pCoreServices->getPlayerManager();
    DEBUG_ASSERT(pPlayerManager);
    const QStringList visualGroups = pPlayerManager->getVisualPlayerGroups();
    for (const QString& group : visualGroups) {
        m_pVisualsManager->addDeck(group);
    }
    connect(pPlayerManager.get(),
            &PlayerManager::numberOfDecksChanged,
            this,
            [this](int decks) {
                for (int i = 0; i < decks; ++i) {
                    QString group = PlayerManager::groupForDeck(i);
                    m_pVisualsManager->addDeckIfNotExist(group);
                }
            });
    connect(pPlayerManager.get(),
            &PlayerManager::numberOfSamplersChanged,
            this,
            [this](int decks) {
                for (int i = 0; i < decks; ++i) {
                    QString group = PlayerManager::groupForSampler(i);
                    m_pVisualsManager->addDeckIfNotExist(group);
                }
            });

#ifndef MIXXX_USE_QOPENGL
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

        WGLWidget* pContextWidget = new WGLWidget(this);
        pContextWidget->setGeometry(QRect(0, 0, 3, 3));
        pContextWidget->hide();
        SharedGLContext::setWidget(pContextWidget);
    }
#endif

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
            m_pCoreServices->getScreensaverManager(),
            m_pSkinLoader,
            m_pCoreServices->getSoundManager(),
            m_pCoreServices->getControllerManager(),
            m_pCoreServices->getVinylControlManager(),
            m_pCoreServices->getEffectsManager(),
            m_pCoreServices->getSettingsManager(),
            m_pCoreServices->getLibrary());
    m_pPrefDlg->setWindowIcon(QIcon(MIXXX_ICON_PATH));
    m_pPrefDlg->setHidden(true);
    connect(m_pPrefDlg,
            &DlgPreferences::tooltipModeChanged,
            this,
            &MixxxMainWindow::slotTooltipModeChanged);
    connect(m_pPrefDlg,
            &DlgPreferences::reloadUserInterface,
            this,
            &MixxxMainWindow::rebootMixxxView,
            Qt::DirectConnection);
#ifndef __APPLE__
    connect(m_pPrefDlg,
            &DlgPreferences::menuBarAutoHideChanged,
            this,
            &MixxxMainWindow::slotUpdateMenuBarAltKeyConnection,
            Qt::DirectConnection);
#endif

    // Connect signals to the menubar. Should be done before emit skinLoaded.
    connectMenuBar();

    QWidget* oldWidget = m_pCentralWidget;

    tryParseAndSetDefaultStyleSheet();

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

    // Sound hardware setup
    // Try to open configured devices. If that fails, display dialogs
    // that allow to either retry, reconfigure devices or exit.
    bool retryClicked;
    do {
        retryClicked = false;
        SoundDeviceStatus result = m_pCoreServices->getSoundManager()->setupDevices();
        if (result == SoundDeviceStatus::ErrorDeviceCount ||
                result == SoundDeviceStatus::ErrorExcessiveOutputChannel) {
            if (soundDeviceBusyDlg(&retryClicked) != QDialog::Accepted) {
                exit(0);
            }
        } else if (result != SoundDeviceStatus::Ok) {
            if (soundDeviceErrorMsgDlg(result, &retryClicked) !=
                    QDialog::Accepted) {
                exit(0);
            }
        }
    } while (retryClicked);

    // Test for at least one output device. If none, display another dialog
    // that says "mixxx will barely work with no outs".
    // In case of persisting errors, the user has already received a message
    // above. So we can just check the output count here.
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

    // The user has either reconfigured devices or accepted no outputs,
    // so it's now safe to write the new config to disk.
    m_pCoreServices->getSoundManager()->getConfig().writeToDisk();

    // this has to be after the OpenGL widgets are created or depending on a
    // million different variables the first waveform may be horribly
    // corrupted. See bug 521509 -- bkgood ?? -- vrince
    setCentralWidget(m_pCentralWidget);

#ifndef __APPLE__
    // Ask for permission to auto-hide the menu bar if applicable.
#ifdef __LINUX__
    // This makes no sense when starting in windowed mode with a global menu,
    // we'll ask when going fullscreen.
    if (!m_supportsGlobalMenuBar || isFullScreen()) {
        alwaysHideMenuBarDlg();
        slotUpdateMenuBarAltKeyConnection();
    }
#else
    alwaysHideMenuBarDlg();
    slotUpdateMenuBarAltKeyConnection();
#endif
#endif

    // Show the menubar after the launch image is replaced by the skin widget,
    // otherwise it would shift the launch image shortly before the skin is visible.
    m_pMenuBar->show();

    // The launch image widget is automatically disposed, but we still have a
    // pointer to it.
    m_pLaunchImage = nullptr;

    connect(pPlayerManager.get(),
            &PlayerManager::noMicrophoneInputConfigured,
            this,
            &MixxxMainWindow::slotNoMicrophoneInputConfigured);
    connect(pPlayerManager.get(),
            &PlayerManager::noAuxiliaryInputConfigured,
            this,
            &MixxxMainWindow::slotNoAuxiliaryInputConfigured);
    connect(pPlayerManager.get(),
            &PlayerManager::noDeckPassthroughInputConfigured,
            this,
            &MixxxMainWindow::slotNoDeckPassthroughInputConfigured);
    connect(pPlayerManager.get(),
            &PlayerManager::noVinylControlInputConfigured,
            this,
            &MixxxMainWindow::slotNoVinylControlInputConfigured);

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &MixxxMainWindow::slotUpdateWindowTitle);

    // Start Auto DJ if the cmdline arg is passed.
    if (CmdlineArgs::Instance().getStartAutoDJ()) {
        qDebug("Enabling Auto DJ from CLI flag.");
        ControlObject::set(ConfigKey("[AutoDJ]", "enabled"), 1.0);
    }
}

MixxxMainWindow::~MixxxMainWindow() {
    Timer t("~MixxxMainWindow");
    t.start();

    // Save the current window state (position, maximized, etc)
    // Note(ronso0): Unfortunately saveGeometry() also stores the fullscreen state.
    // On next start restoreGeometry would enable fullscreen mode even though that
    // might not be requested (no '--fullscreen' command line arg and
    // [Config],StartInFullscreen is '0'.
    // https://github.com/mixxxdj/mixxx/issues/10005
    // So let's quit fullscreen if StartInFullscreen is not checked in Preferences.
    bool fullscreenPref = m_pCoreServices->getSettings()->getValue<bool>(
            ConfigKey("[Config]", "StartInFullscreen"));
    if (isFullScreen() && !fullscreenPref) {
        // Simply maximize the window so we can store a geometry that fits the screen.
        // Don't call slotViewFullScreen(false) (calls showNormal()) because that
        // can make the main window incl. window decoration too large for the screen.
#ifndef __APPLE__
        // Before, store the expected window state so eventFilter() will ignore
        // the following QWindowChangeEvent and not recreate & re-sync the menu bar.
        m_prevState = Qt::WindowMaximized;
#endif
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
    // dangling pointer after the object has been deleted.
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
    delete m_pDeveloperToolsDlg;

#ifdef __ENGINEPRIME__
    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting LibraryExporter";
    m_pLibraryExporter.reset();
#endif

    qDebug() << t.elapsed(false).debugMillisWithUnit() << "deleting DlgPreferences";
    delete m_pPrefDlg;

    m_pCoreServices->getControlIndicatorTimer()->setLegacyVsyncEnabled(false);

    WaveformWidgetFactory::destroy();

    delete m_pGuiTick;
    delete m_pVisualsManager;
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

    // Restore the current window state (position, maximized, etc).
    // This will also restore fullscreen and thereby create a seamless
    // start if we did shut down while in fullscreen mode and with
    // [Config],StartInFullscreen = 1
    // (slotViewFullScreen(true) in  initialize() is a no-op then)
    restoreGeometry(QByteArray::fromBase64(
            m_pCoreServices->getSettings()
                    ->getValueString(ConfigKey("[MainWindow]", "geometry"))
                    .toUtf8()));
    restoreState(QByteArray::fromBase64(
            m_pCoreServices->getSettings()
                    ->getValueString(ConfigKey("[MainWindow]", "state"))
                    .toUtf8()));

    setWindowIcon(QIcon(MIXXX_ICON_PATH));
    slotUpdateWindowTitle(TrackPointer());
}

#ifndef __APPLE__
void MixxxMainWindow::alwaysHideMenuBarDlg() {
    // Don't show the dialog if the user unchecked "Ask me again"
    if (!m_pCoreServices->getSettings()->getValue<bool>(
                kMenuBarHintConfigKey, true)) {
        return;
    }
    QString title = tr("Allow Mixxx to hide the menu bar?");
    //: Always show the menu bar?
    QString hideBtnLabel = tr("Hide");
    QString showBtnLabel = tr("Always show");
    //: Keep formatting tags <b> (bold text) and <br> (linebreak).
    //: %1 is the placeholder for the 'Always show' button label
    QString desc = tr(
            "The Mixxx menu bar is hidden and can be toggled with a single press "
            "of the <b>Alt</b> key.<br><br>"
            "Click <b>%1</b> to agree.<br><br>"
            "Click <b>%2</b> to disable that, for example if you don't use Mixxx "
            "with a keyboard.<br><br>"
            "You can change this setting any time in Preferences -> Interface."
            "<br>") // line break for some extra margin to the checkbox
                           .arg(hideBtnLabel, showBtnLabel);

    QMessageBox msg;
    msg.setIcon(QMessageBox::Question);
    msg.setWindowTitle(title);
    msg.setText(desc);
    QCheckBox askAgainCheckBox;
    askAgainCheckBox.setText(tr("Ask me again"));
    askAgainCheckBox.setCheckState(Qt::Checked);
    msg.setCheckBox(&askAgainCheckBox);
    QPushButton* pHideBtn = msg.addButton(hideBtnLabel, QMessageBox::AcceptRole);
    QPushButton* pShowBtn = msg.addButton(showBtnLabel, QMessageBox::RejectRole);
    msg.setDefaultButton(pShowBtn);
    msg.exec();

    m_pCoreServices->getSettings()->setValue(
            kMenuBarHintConfigKey,
            askAgainCheckBox.checkState() == Qt::Checked ? 1 : 0);

    m_pCoreServices->getSettings()->setValue(
            kHideMenuBarConfigKey,
            msg.clickedButton() == pHideBtn ? 1 : 0);
}
#endif

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
            mixxx::DesktopHelper::openUrl(QUrl(MIXXX_WIKI_TROUBLESHOOTING_SOUND_URL));
            wikiButton->setEnabled(false);
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();

            m_pCoreServices->getSoundManager()->clearAndQueryDevices();
            // This way of opening the dialog allows us to use it synchronously
            m_pPrefDlg->setWindowModality(Qt::ApplicationModal);
            // Open preferences, sound hardware page is selected (default on first call)
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
        SoundDeviceStatus status, bool* retryClicked) {
    QString title(tr("Sound Device Error"));
    QString text("<html> <p>" %
                    tr("Mixxx was unable to open all the configured sound "
                       "devices.") +
            "</p> <p>" %
                    m_pCoreServices->getSoundManager()
                            ->getLastErrorMessage(status)
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
    QString filePath;

    // If we have a track, use getInfo() to format a summary string and prepend
    // it to the title.
    // TODO(rryan): Does this violate Mac App Store policies?
    if (pTrack) {
        QString trackInfo = pTrack->getInfo();
        if (!trackInfo.isEmpty()) {
            appTitle = QString("%1 | %2").arg(trackInfo, appTitle);
        }
        filePath = pTrack->getLocation();
    }
    setWindowTitle(appTitle);

    // Display a draggable proxy icon for the track in the title bar on
    // platforms that support it, e.g. macOS
    setWindowFilePath(filePath);
}

void MixxxMainWindow::createMenuBar() {
    ScopedTimer t(QStringLiteral("MixxxMainWindow::createMenuBar"));
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

    ScopedTimer t(QStringLiteral("MixxxMainWindow::connectMenuBar"));
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
            &MixxxMainWindow::slotShowKeywheel,
            Qt::UniqueConnection);

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

    auto pPlayerManager = m_pCoreServices->getPlayerManager();
    if (pPlayerManager) {
        connect(pPlayerManager.get(),
                &PlayerManager::numberOfDecksChanged,
                m_pMenuBar,
                &WMainMenuBar::onNumberOfDecksChanged,
                Qt::UniqueConnection);
        m_pMenuBar->onNumberOfDecksChanged(pPlayerManager->numberOfDecks());
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

/// Enable/disable listening to Alt key press for toggling the menubar.
#ifndef __APPLE__
void MixxxMainWindow::slotUpdateMenuBarAltKeyConnection() {
    if (!m_pCoreServices->getKeyboardEventFilter() || !m_pMenuBar) {
        return;
    }

    if (m_pCoreServices->getSettings()->getValue<bool>(kHideMenuBarConfigKey, false)) {
        // with Qt::UniqueConnection we don't need to check whether we're already connected
        connect(m_pCoreServices->getKeyboardEventFilter().get(),
                &KeyboardEventFilter::altPressedWithoutKeys,
                m_pMenuBar,
                &WMainMenuBar::slotToggleMenuBar,
                Qt::UniqueConnection);
        m_pMenuBar->hideMenuBar();
    } else {
        disconnect(m_pCoreServices->getKeyboardEventFilter().get(),
                &KeyboardEventFilter::altPressedWithoutKeys,
                m_pMenuBar,
                &WMainMenuBar::slotToggleMenuBar);
        m_pMenuBar->showMenuBar();
    }
}
#endif

void MixxxMainWindow::slotFileLoadSongPlayer(int deck) {
    QString group = PlayerManager::groupForDeck(deck - 1);

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
                    pConfig->getValueString(mixxx::library::prefs::kLegacyDirectoryConfigKey),
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

    // Just switch the window state here. eventFilter() will catch the
    // QWindowStateChangeEvent and inform the menu bar that fullscreen changed.
    if (toggle) {
        showFullScreen();
    } else {
        showNormal();
    }
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

void MixxxMainWindow::slotHelpAbout() {
    DlgAbout* about = new DlgAbout;
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

void MixxxMainWindow::slotTooltipModeChanged(mixxx::preferences::Tooltips tt) {
    m_toolTipsCfg = tt;
#ifdef MIXXX_USE_QOPENGL
    ToolTipQOpenGL::singleton().setActive(
            m_toolTipsCfg == mixxx::preferences::Tooltips::On);
#endif
}

void MixxxMainWindow::rebootMixxxView() {
    qDebug() << "Now in rebootMixxxView...";
    m_inRebootMixxxView = true;

    ScopedWaitCursor cursor;
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
    if (wasFullScreen) {
        showMaximized();
    }

    tryParseAndSetDefaultStyleSheet();

    if (!loadConfiguredSkin()) {
        QMessageBox::critical(this,
                              tr("Error in skin file"),
                              tr("The selected skin cannot be loaded."));
        m_inRebootMixxxView = false;
        // m_pWidgetParent is NULL, we can't continue.
        return;
    }
    m_pMenuBar->setStyleSheet(m_pCentralWidget->styleSheet());

    setCentralWidget(m_pCentralWidget);
#ifdef __LINUX__
    // don't adjustSize() on Linux as this wouldn't use the entire available area
    // to paint the new skin with X11
    // https://github.com/mixxxdj/mixxx/issues/9309
#else
    adjustSize();
#endif

    if (wasFullScreen) {
        showFullScreen();
    } else {
        // Programmatic placement at this point is very problematic.
        // The screen() method returns stale data (primary screen)
        // until the user interacts with mixxx again. Keyboard shortcuts
        // do not count, moving window, opening menu etc does
        // Therefore the placement logic was removed by a simple geometry restore.
        // If the minimum size of the new skin is larger then the restored
        // geometry, the window will be enlarged right & bottom which is
        // safe as the menu is still reachable.
        setGeometry(initGeometry);
    }

    m_inRebootMixxxView = false;
    qDebug() << "rebootMixxxView DONE";
}

bool MixxxMainWindow::loadConfiguredSkin() {
    // TODO: use std::shared_ptr throughout skin widgets instead of these hacky get() calls
    m_pCentralWidget = m_pSkinLoader->loadConfiguredSkin(this,
            &m_skinCreatedControls,
            m_pCoreServices.get());
    if (centralWidget() == m_pLaunchImage) {
        initializationProgressUpdate(100, "");
    }
    emit skinLoaded();
    return m_pCentralWidget != nullptr;
}

/// Try to load default styles that can be overridden by skins
void MixxxMainWindow::tryParseAndSetDefaultStyleSheet() {
    const QString resPath = m_pCoreServices->getSettings()->getResourcePath();
    QFile file(resPath + "/skins/default.qss");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileBytes = file.readAll();
        QString style = QString::fromUtf8(fileBytes);
        setStyleSheet(style);
    } else {
        qWarning() << "Failed to load default skin styles /skins/default.qss!";
    }
}

/// Catch ToolTip and WindowStateChange events
bool MixxxMainWindow::eventFilter(QObject* obj, QEvent* event) {
    // Always show tooltips if Ctrl is held down
    if (event->type() == QEvent::ToolTip &&
            !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        QWidget* activeWindow = QApplication::activeWindow();
        if (activeWindow &&
                QLatin1String(activeWindow->metaObject()->className()) !=
                        "DlgPreferences") {
            // return true for no tool tips
            switch (m_toolTipsCfg) {
            case mixxx::preferences::Tooltips::OnlyInLibrary:
                if (dynamic_cast<WBaseWidget*>(obj) != nullptr) {
                    return true;
                }
                break;
            case mixxx::preferences::Tooltips::On:
                break;
            case mixxx::preferences::Tooltips::Off:
                return true;
            default:
                DEBUG_ASSERT(!"m_toolTipsCfg value unknown");
                return true;
            }
        }
    } else if (event->type() == QEvent::WindowStateChange) {
#ifndef __APPLE__
        if (windowState() == m_prevState) {
            // Ignore no-op. This happens if another window is raised above
            // MixxxMianWindow,  e.g. DlgPeferences. In such a case event->oldState()
            // will be Qt::WindowNoState which is wrong anyway, so there is nothing
            // to do internally.
            return QMainWindow::eventFilter(obj, event);
        }
        m_prevState = windowState();
#endif
        // Detect if we entered or quit fullscreen mode.
        QWindowStateChangeEvent* changeEvent =
                static_cast<QWindowStateChangeEvent*>(event);
        const bool wasFullScreen = changeEvent->oldState() & Qt::WindowFullScreen;
        const bool isFullScreenNow = windowState() & Qt::WindowFullScreen;
        if ((isFullScreenNow && !wasFullScreen) ||
                (!isFullScreenNow && wasFullScreen)) {
#ifdef __LINUX__
            // Fix for "No menu bar with ubuntu unity in full screen mode"
            // (issues #6072 and #6689). Before touching anything here, please
            // read those bugs.
            // Set this attribute instead of calling setNativeMenuBar(false),
            // see https://github.com/mixxxdj/mixxx/issues/11320
            if (m_supportsGlobalMenuBar) {
                QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, isFullScreenNow);
                createMenuBar();
                connectMenuBar();
            }
#endif

#ifndef __APPLE__
#ifdef __LINUX__
            // Only show the dialog if we are able to have the menubar in the
            // main window, only then we're able to hide it.
            if (!m_supportsGlobalMenuBar || isFullScreenNow)
#endif
            {
                if (!m_inRebootMixxxView) {
                    alwaysHideMenuBarDlg();
                }
                slotUpdateMenuBarAltKeyConnection();
            }
#endif

            // This will toggle the Fullscreen checkbox and hide the menubar if
            // we go fullscreen.
            // Skip this during startup or the launchimage will be shifted
            // up & down when the menu is shown menu and 'hidden'. The menu
            // will be updated when the skin finished loading.
            if (centralWidget() != m_pLaunchImage) {
                emit fullScreenChanged(isFullScreen());
            }
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
    auto pPlayerManager = m_pCoreServices->getPlayerManager();
    unsigned int deckCount = pPlayerManager->numDecks();
    unsigned int samplerCount = pPlayerManager->numSamplers();
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

void MixxxMainWindow::initializationProgressUpdate(int progress, const QString& serviceName) {
    if (m_pLaunchImage) {
        m_pLaunchImage->progress(progress, serviceName);
    }
    qApp->processEvents();
}
