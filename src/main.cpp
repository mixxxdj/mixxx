#include <QApplication>
#include <QDir>
#include <QPixmapCache>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QTextCodec>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>
#include <cstdio>
#include <memory>
#include <stdexcept>

#include "config.h"
#include "controllers/controllermanager.h"
#include "coreservices.h"
#include "errordialoghandler.h"
#include "mixxxapplication.h"
#ifdef MIXXX_USE_QML
#include "mixer/playermanager.h"
#include "qml/qmlapplication.h"
#include "waveform/guitick.h"
#include "waveform/visualsmanager.h"
#include "waveform/waveformwidgetfactory.h"
#endif
#include "mixxxmainwindow.h"
#if defined(__WINDOWS__)
#include "nativeeventhandlerwin.h"
#endif
#include "sources/soundsourceproxy.h"
#include "util/cmdlineargs.h"
#include "util/console.h"
#include "util/logging.h"
#include "util/sandbox.h"
#include "util/versionstore.h"

namespace {

// Exit codes
constexpr int kFatalErrorOnStartupExitCode = 1;
constexpr int kParseCmdlineArgsErrorExitCode = 2;

constexpr char kScaleFactorEnvVar[] = "QT_SCALE_FACTOR";
const QString kConfigGroup = QStringLiteral("[Config]");
const QString kScaleFactorKey = QStringLiteral("ScaleFactor");
const QString kNotifyMaxDbgTimeKey = QStringLiteral("notify_max_dbg_time");

// The default initial QPixmapCache limit is 10MB.
// But this is used for all CoverArts in all used sizes and
// as rendering cache for all SVG icons by Qt behind the scenes.
// Consequently coverArt cache will always have less than those
// 10MB available to store the pixmaps.
// Profiling at 100% HiDPI zoom on Windows, that with 20MByte,
// the SVG rendering happens sometimes during normal operation.
// An indicator that the QPixmapCache was too small.
constexpr int kPixmapCacheLimitAt100PercentZoom = 32 * 1024; // 32 MByte

int runMixxx(MixxxApplication* pApp, const CmdlineArgs& args) {
    CmdlineArgs::Instance().parseForUserFeedback();

    int exitCode;
#ifdef MIXXX_USE_QML
    if (args.isQml()) {
        mixxx::qml::QmlApplication qmlApplication(pApp, args);
        exitCode = pApp->exec();
    } else
#endif
    {
        auto pCoreServices = std::make_shared<mixxx::CoreServices>(args, pApp);

        // This scope ensures that `MixxxMainWindow` is destroyed *before*
        // CoreServices is shut down. Otherwise a debug assertion complaining about
        // leaked COs may be triggered.
        MixxxMainWindow mainWindow(pCoreServices);
        pApp->processEvents();
        pApp->installEventFilter(&mainWindow);

#if defined(__WINDOWS__)
        WindowsEventHandler winEventHandler;
        pApp->installNativeEventFilter(&winEventHandler);
#endif

        QObject::connect(pCoreServices.get(),
                &mixxx::CoreServices::initializationProgressUpdate,
                &mainWindow,
                &MixxxMainWindow::initializationProgressUpdate);

        // The size of cached pixmaps increases with the square of devicePixelRatio
        // (this covers both, operating system scaling and Mixxx preferences scaling)
        QPixmapCache::setCacheLimit(static_cast<int>(kPixmapCacheLimitAt100PercentZoom *
                pow(pApp->devicePixelRatio(), 2.0f)));

        pCoreServices->initialize(pApp);

#ifdef MIXXX_USE_QOPENGL
        // Will call initialize when the initial wglwidget's
        // qopenglwindow has been exposed
        mainWindow.initializeQOpenGL();
#else
        mainWindow.initialize();
#endif

        pCoreServices->getControllerManager()->setUpDevices();

        // If startup produced a fatal error, then don't even start the
        // Qt event loop.
        if (ErrorDialogHandler::instance()->checkError()) {
            exitCode = kFatalErrorOnStartupExitCode;
        } else {
            qDebug() << "Displaying main window";
            mainWindow.show();

            qDebug() << "Running Mixxx";
            exitCode = pApp->exec();
        }
    }
    return exitCode;
}

void adjustScaleFactor(CmdlineArgs* pArgs) {
    if (qEnvironmentVariableIsSet(kScaleFactorEnvVar)) {
        bool ok;
        const double f = qgetenv(kScaleFactorEnvVar).toDouble(&ok);
        if (ok && f > 0) {
            // The environment variable overrides the preferences option
            qDebug() << "Using" << kScaleFactorEnvVar << f;
            pArgs->setScaleFactor(f);
            return;
        }
    }
    // We cannot use SettingsManager, because it depends on MixxxApplication
    // but the scale factor is read during it's constructor.
    // QHighDpiScaling can not be used afterwards because it is private.
    // This means the following code may fail after down/upgrade ... a one time issue.

    // Read and parse the config file from the settings path
    auto config = ConfigObject<ConfigValue>(
            QDir(pArgs->getSettingsPath()).filePath(MIXXX_SETTINGS_FILE),
            QString(),
            QString());
    QString strScaleFactor = config.getValue(
            ConfigKey(kConfigGroup, kScaleFactorKey));
    double scaleFactor = strScaleFactor.toDouble();
    if (scaleFactor > 0) {
        qDebug() << "Using preferences ScaleFactor" << scaleFactor;
        qputenv(kScaleFactorEnvVar, strScaleFactor.toLocal8Bit());
        pArgs->setScaleFactor(scaleFactor);
    }
}

void applyStyleOverride(CmdlineArgs* pArgs) {
    if (!pArgs->getStyle().isEmpty()) {
        qDebug() << "Default style is overwritten by command line argument "
                    "-style"
                 << pArgs->getStyle();
        QApplication::setStyle(pArgs->getStyle());
        return;
    }
    if (qEnvironmentVariableIsSet("QT_STYLE_OVERRIDE")) {
        QString styleOverride = QString::fromLocal8Bit(qgetenv("QT_STYLE_OVERRIDE"));
        if (!styleOverride.isEmpty()) {
            // The environment variable overrides the command line option
            qDebug() << "Default style is overwritten by env variable "
                        "QT_STYLE_OVERRIDE"
                     << styleOverride;
            QApplication::setStyle(styleOverride);
        }
    }
}

} // anonymous namespace

int main(int argc, char * argv[]) {
    Console console;

    // These need to be set early on (not sure how early) in order to trigger
    // logic in the OS X appstore support patch from QTBUG-16549.
    QCoreApplication::setOrganizationDomain("mixxx.org");

    // High DPI scaling is always enabled in Qt6.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // This needs to be set before initializing the QApplication.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#ifdef MIXXX_USE_QOPENGL
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif

    // workaround for https://bugreports.qt.io/browse/QTBUG-84363
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && QT_VERSION < QT_VERSION_CHECK(5, 15, 1)
    qputenv("QV4_FORCE_INTERPRETER", QByteArrayLiteral("1"));
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Follow whatever factor the user has selected in the system settings
    // By default the value is always rounded to the nearest int.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#ifdef __LINUX__
    // Needed by Wayland compositors to set proper app_id and window icon
    QGuiApplication::setDesktopFileName(QStringLiteral("org.mixxx.Mixxx"));
#endif

    // Setting the organization name results in a QDesktopStorage::DataLocation
    // of "$HOME/Library/Application Support/Mixxx/Mixxx" on OS X. Leave the
    // organization name blank.
    //QCoreApplication::setOrganizationName("Mixxx");

    QCoreApplication::setApplicationName(VersionStore::applicationName());
    QCoreApplication::setApplicationVersion(VersionStore::version());

    // Construct a list of strings based on the command line arguments
    CmdlineArgs& args = CmdlineArgs::Instance();
    if (!args.parse(argc, argv)) {
        return kParseCmdlineArgsErrorExitCode;
    }

    // If you change this here, you also need to change it in
    // ErrorDialogHandler::errorDialog(). TODO(XXX): Remove this hack.
    QThread::currentThread()->setObjectName("Main");

    // Create the ErrorDialogHandler in the main thread, otherwise it will be
    // created in the thread of the first caller to instance(), which may not be
    // the main thread. Issue #9130.
    ErrorDialogHandler::instance();

#ifdef __APPLE__
    Sandbox::checkSandboxed();
#endif

    adjustScaleFactor(&args);

    MixxxApplication app(argc, argv);

#if defined(Q_OS_WIN)
    // The Mixxx style is based on Qt's WindowsVista style
    QApplication::setStyle("windowsvista");
#endif

    applyStyleOverride(&args);

    qInfo() << "Selected Qt style:" << QApplication::style()->objectName();

#if defined(Q_OS_WIN)
    if (QApplication::style()->objectName() != "windowsvista") {
        qWarning() << "Qt style for Windows is not set to 'windowsvista'. GUI might look broken!";
    }
#endif

    auto config = ConfigObject<ConfigValue>(
            QDir(args.getSettingsPath()).filePath(MIXXX_SETTINGS_FILE),
            QString(),
            QString());
    int notifywarningThreshold = config.getValue<int>(
            ConfigKey(kConfigGroup, kNotifyMaxDbgTimeKey), 10);
    app.setNotifyWarningThreshold(notifywarningThreshold);

#ifdef Q_OS_MACOS
    // TODO: At this point it is too late to provide the same settings path to all components
    // and too early to log errors and give users advises in their system language.
    // Calling this from main.cpp before the QApplication is initialized may cause a crash
    // due to potential QMessageBox invocations within migrateOldSettings().
    // Solution: Start Mixxx with default settings, migrate the preferences, and then restart
    // immediately.
    if (!args.getSettingsPathSet()) {
        CmdlineArgs::Instance().setSettingsPath(Sandbox::migrateOldSettings());
    }
#endif

#ifdef __APPLE__
    QDir dir(QApplication::applicationDirPath());
    // Set the search path for Qt plugins to be in the bundle's PlugIns
    // directory, but only if we think the mixxx binary is in a bundle.
    if (dir.path().contains(".app/")) {
        // If in a bundle, applicationDirPath() returns something formatted
        // like: .../Mixxx.app/Contents/MacOS
        dir.cdUp();
        dir.cd("PlugIns");
        qDebug() << "Setting Qt plugin search path to:" << dir.absolutePath();
        // asantoni: For some reason we need to do setLibraryPaths() and not
        // addLibraryPath(). The latter causes weird problems once the binary
        // is bundled (happened with 1.7.2 when Brian packaged it up).
        QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
    }
#endif

    // When the last window is closed, terminate the Qt event loop.
    QObject::connect(&app, &MixxxApplication::lastWindowClosed, &app, &MixxxApplication::quit);

    int exitCode = runMixxx(&app, args);

    qDebug() << "Mixxx shutdown complete with code" << exitCode;

    mixxx::Logging::shutdown();

    return exitCode;
}
