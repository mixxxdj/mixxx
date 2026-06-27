#include "qmlapplication.h"

#include <QAction>
#include <QCoreApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QQmlEngineExtensionPlugin>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTextDocument>
#include <utility>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "controllers/controllermanager.h"
#include "mixer/playermanager.h"
#include "moc_qmlapplication.cpp"
#include "preferences/configobject.h"
#include "qml/asyncimageprovider.h"
#include "qml/qmldlgpreferencesproxy.h"
#include "qml/qmlrecordingproxy.h"
#include "soundio/soundmanager.h"
#include "util/versionstore.h"
#include "waveform/guitick.h"
#include "waveform/visualsmanager.h"
#include "waveform/waveformwidgetfactory.h"
#if defined(Q_OS_ANDROID)
#include <android/api-level.h>
#include <android/log.h>
#include <android/performance_hint.h>
#endif

Q_IMPORT_QML_PLUGIN(MixxxPlugin)
Q_IMPORT_QML_PLUGIN(Mixxx_ControlsPlugin)

namespace {
const QString kMainQmlFileName = QStringLiteral("qml/main.qml");

// Converts a (capturing) lambda into a function pointer that can be passed to
// qmlRegisterSingletonType.
template<class F>
auto lambda_to_singleton_type_factory_ptr(F&& f) {
    static F fn = std::forward<F>(f);
    return [](QQmlEngine* pEngine, QJSEngine* pScriptEngine) -> QObject* {
        return fn(pEngine, pScriptEngine);
    };
}
} // namespace

namespace mixxx {
namespace qml {

QmlApplication::QmlApplication(
        QApplication* app,
        std::shared_ptr<CoreServices> pCoreServices,
        const QString& mainQmlFilePath)
        : m_pCoreServices(std::move(pCoreServices)),
          m_visualsManager(std::make_unique<VisualsManager>()),
          m_pGuiTick(std::make_unique<GuiTick>()),
          m_mainFilePath(mainQmlFilePath.isEmpty()
                          ? m_pCoreServices->getSettings()->getResourcePath() + kMainQmlFileName
                          : mainQmlFilePath),
          m_pAppEngine(nullptr),
          m_loadSucceeded(false),
#if defined(Q_OS_ANDROID)
          m_perfSession(nullptr),
#endif
          m_autoReload() {
    QQuickStyle::setStyle("Basic");

    m_pCoreServices->initialize(app);

    QString configVersion = m_pCoreServices->getSettings()->getValue(
            ConfigKey("[Config]", "Version"), "");

    // The risk check guards against Mixxx 3.0 potentially running different
    // database upgrade paths that could corrupt 2.x profiles.
    //
    // When a QML skin is auto-detected from preferences (--developer, no
    // --new-ui), the underlying binary and DB schema are identical to a
    // normal 2.x launch — there is no data corruption risk. Skip the gate.
    //
    // When explicitly launched with --new-ui, the full 3.0 application path
    // is taken and the gate remains in effect as designed.
    const bool viaNewUiFlag = CmdlineArgs::Instance().isQml();

    if (configVersion == VersionStore::FUTURE_UNSTABLE) {
        qDebug() << "Generating a new user profile for safe testing with unstable code";
    } else if (!viaNewUiFlag) {
        qDebug() << "QmlApplication: QML skin loaded via preferences, "
                    "skipping data-corruption risk check (2.x profile is safe)";
    } else if (CmdlineArgs::Instance().isAwareOfRisk()) {
        qCritical() << "Existing user profile detected from" << configVersion
                    << "but you said you wanted to play with fire!";
        m_pCoreServices->getSettings()->setValue(
                ConfigKey("[Config]", "did_run_with_unstable"), true);
    } else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(tr("Existing user profile detected"));
        msgBox.setText(
                tr("Trying to run Mixxx 3.0 with an existing %0 user profile! "
                   "<br><br>There is <b>serious risks</b> of data loss and "
                   "corruption.<br>We recommend using a test profile folder "
                   "with the '--settings-path' argument. <br><br>If you want "
                   "to continue at your own risk, run Mixxx with the argument "
                   "'--allow-dangerous-data-corruption-risk'.")
                        .arg(configVersion));

        QPushButton* continueButton =
                msgBox.addButton(tr("Ok"), QMessageBox::ActionRole);
        msgBox.exec();
        m_pCoreServices.reset();
        exit(-1);
    }

    SoundDeviceStatus result = m_pCoreServices->getSoundManager()->setupDevices();
    if (result != SoundDeviceStatus::Ok) {
        const int reInt = static_cast<int>(result);
        qCritical() << "Error setting up sound devices:" << reInt;
#ifndef Q_OS_ANDROID
        exit(reInt);
#endif
    }

    setupSpinnyCoverControls();

    // FIXME: DlgPreferences has some initialization logic that must be executed
    // before the GUI is shown, at least for the effects system.
    std::shared_ptr<QDialog> pDlgPreferences = m_pCoreServices->makeDlgPreferences();
    // Without this, QApplication will quit when the last QWidget QWindow is
    // closed because it does not take into account the window created by
    // the QQmlApplicationEngine.
    pDlgPreferences->setAttribute(Qt::WA_QuitOnClose, false);

    auto showNoInputConfiguredWarning = [pDlgPreferences](
                                                const QString& message) {
        QMessageBox msgBox(QMessageBox::Warning,
                VersionStore::applicationName(),
                message,
                QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setOption(QMessageBox::Option::DontUseNativeDialog);
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.exec();
        if (msgBox.clickedButton() == msgBox.button(QMessageBox::Ok)) {
            pDlgPreferences->show();
            pDlgPreferences->raise();
            pDlgPreferences->activateWindow();
        }
    };

    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noDeckPassthroughInputConfigured,
            this,
            [showNoInputConfiguredWarning]() {
                showNoInputConfiguredWarning(
                        tr("There is no input device selected for this "
                           "passthrough control.\n"
                           "Please select an input device in the sound "
                           "hardware preferences first."));
            });
    connect(m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::noVinylControlInputConfigured,
            this,
            [showNoInputConfiguredWarning]() {
                showNoInputConfiguredWarning(
                        tr("There is no input device selected for this vinyl "
                           "control.\n"
                           "Please select an input device in the sound "
                           "hardware preferences first."));
            });

    // Since DlgPreferences is only meant to be used in the main QML engine, it
    // follows a strict singleton pattern design
    QmlDlgPreferencesProxy::s_pInstance =
            std::make_unique<QmlDlgPreferencesProxy>(pDlgPreferences, this);
    QmlRecordingProxy::s_pRecordingManager = m_pCoreServices->getRecordingManager();

    m_pMenuBar = std::make_unique<QMenuBar>();
    QMenu* pApplicationMenu = m_pMenuBar->addMenu(QCoreApplication::applicationName());
    QAction* pPreferencesAction = pApplicationMenu->addAction(tr("&Preferences"));
    pPreferencesAction->setMenuRole(QAction::PreferencesRole);
    pPreferencesAction->setShortcut(QKeySequence::Preferences);
    connect(pPreferencesAction, &QAction::triggered, this, [pDlgPreferences]() {
        pDlgPreferences->show();
        pDlgPreferences->raise();
        pDlgPreferences->activateWindow();
    });

    const QStringList visualGroups =
            m_pCoreServices->getPlayerManager()->getVisualPlayerGroups();
    for (const QString& group : visualGroups) {
        m_visualsManager->addDeck(group);
    }

    m_pCoreServices->getPlayerManager()->connect(
            m_pCoreServices->getPlayerManager().get(),
            &PlayerManager::numberOfDecksChanged,
            this,
            [this](int decks) {
                for (int i = 0; i < decks; ++i) {
                    QString group = PlayerManager::groupForDeck(i);
                    m_visualsManager->addDeckIfNotExist(group);
                }
            });

    connect(&m_guiTickTimer, &QTimer::timeout, this, [this]() {
        m_pGuiTick->process();
    });
    m_guiTickTimer.start(std::chrono::milliseconds(16));

    m_loadSucceeded = loadQml(m_mainFilePath);
    if (!m_loadSucceeded) {
        return;
    }

    m_pCoreServices->getControllerManager()->setUpDevices();

    connect(&m_autoReload,
            &QmlAutoReload::triggered,
            this,
            [this]() {
                if (!loadQml(m_mainFilePath)) {
                    qWarning() << "Auto-reload failed to load QML. Exiting.";
                    QCoreApplication::exit(-1);
                }
            });

#if defined(Q_OS_ANDROID)
    APerformanceHintManager* manager = APerformanceHint_getManager();
    VERIFY_OR_DEBUG_ASSERT(manager) {
        return;
    }
    int32_t thread32 = gettid();
    m_perfSession = APerformanceHint_createSession(manager, &thread32, 1, 1e9 / 60);
    VERIFY_OR_DEBUG_ASSERT(m_perfSession) {
        __android_log_print(ANDROID_LOG_WARN, "mixxx", "unable to create a ADPF session!");
    }
    else {
        APerformanceHint_setPreferPowerEfficiency(m_perfSession, false);
        __android_log_print(ANDROID_LOG_VERBOSE, "mixxx", "ADPF session ready");
    }
}

void QmlApplication::slotWindowChanged(QQuickWindow* window) {
    if (window) {
        connect(window, &QQuickWindow::afterFrameEnd, this, &QmlApplication::slotFrameSwapped);
    }
    m_frameTimer.restart();
}

void QmlApplication::slotFrameSwapped() {
    VERIFY_OR_DEBUG_ASSERT(m_perfSession) {
        return;
    }
    auto lastFrameDurationNs = m_frameTimer.elapsed().toIntegerNanos();
    auto t = std::chrono::steady_clock::now() - std::chrono::steady_clock::time_point{};
    APerformanceHint_reportActualWorkDuration(m_perfSession,
            lastFrameDurationNs);
    m_frameTimer.restart();
#endif
}

QmlApplication::~QmlApplication() {
    // Delete all the QML singletons in order to prevent leak detection in CoreService
    QmlRecordingProxy::s_pRecordingManager.reset();
    QmlDlgPreferencesProxy::s_pInstance.reset();
    m_visualsManager.reset();
    m_pAppEngine.reset();
    m_pCoreServices.reset();
}

void QmlApplication::setupSpinnyCoverControls() {
    m_pShowSpinny = make_parented<ControlProxy>("[Skin]", "show_spinnies", this);
    m_pShowCover = make_parented<ControlProxy>("[Skin]", "show_coverart", this);
    m_pSelectBigSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "select_big_spinny_or_cover"), true);
    m_pSelectBigSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pShowSpinnyAndOrCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_spinny_or_cover"));
    m_pShowSpinnyAndOrCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowSpinnyAndOrCover->setReadOnly();

    m_pShowSmallSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_small_spinny_or_cover"));
    m_pShowSmallSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowSmallSpinnyCover->setReadOnly();

    m_pShowBigSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_big_spinny_or_cover"));
    m_pShowBigSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowBigSpinnyCover->setReadOnly();

    m_pShowSpinny->connectValueChanged(this, &QmlApplication::updateSpinnyCoverControls);
    m_pShowCover->connectValueChanged(this, &QmlApplication::updateSpinnyCoverControls);
    connect(m_pSelectBigSpinnyCover.get(),
            &ControlObject::valueChanged,
            this,
            &QmlApplication::updateSpinnyCoverControls);

    updateSpinnyCoverControls();
}

void QmlApplication::updateSpinnyCoverControls() {
    m_pShowSpinnyAndOrCover->setAndConfirm(
            (m_pShowSpinny->toBool() || m_pShowCover->toBool()) ? 1.0 : 0.0);
    m_pShowSmallSpinnyCover->setAndConfirm(
            (m_pShowSpinnyAndOrCover->toBool() && !m_pSelectBigSpinnyCover->toBool())
                    ? 1.0
                    : 0.0);
    m_pShowBigSpinnyCover->setAndConfirm(
            (m_pShowSpinnyAndOrCover->toBool() && m_pSelectBigSpinnyCover->toBool())
                    ? 1.0
                    : 0.0);
}

bool QmlApplication::loadQml(const QString& path) {
    // QQmlApplicationEngine::load creates a new window but also leaves the old one,
    // so it is necessary to destroy the old QQmlApplicationEngine and create a new one.
    m_pAppEngine = std::make_unique<QQmlApplicationEngine>();

    m_autoReload.clear();
    m_pAppEngine->addUrlInterceptor(&m_autoReload);
    m_pAppEngine->addImportPath(QStringLiteral(":/mixxx.org/imports"));

    // No memory leak here, the QQmlEngine takes ownership of the provider
    QQuickAsyncImageProvider* pImageProvider = new AsyncImageProvider(
            m_pCoreServices->getTrackCollectionManager());
    m_pAppEngine->addImageProvider(AsyncImageProvider::kProviderName, pImageProvider);

    m_pAppEngine->load(path);
    if (m_pAppEngine->rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML file" << path;
        m_pAppEngine.reset();
        return false;
    }

#if defined(Q_OS_ANDROID)
    for (auto* item : m_pAppEngine->rootObjects()) {
        auto* pWindow = qobject_cast<QQuickWindow*>(item);
        if (!pWindow) {
            continue;
        }
        slotWindowChanged(pWindow);
        break;
    }
#endif
    return true;
}

} // namespace qml
} // namespace mixxx
