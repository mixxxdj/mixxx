#include <QApplication>
#include <QDir>
#include <QPixmapCache>
#include <QScreen>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QTextCodec>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
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
#include "preferences/configobject.h"
#if defined(__WINDOWS__)
#include "nativeeventhandlerwin.h"
#endif
#include "skin/skin.h"
#include "skin/skinloader.h"
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
#ifdef Q_OS_ANDROID
// Flag to distinguish our programmatic QT_SCALE_FACTOR (safe default) from a
// user-provided override. When we set it ourselves as a boot-up default,
// maybeAutoDetectScaleFactor() must still run to adapt to the actual display.
static bool s_androidScaleSetInternally = false;
#endif
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

int runMixxx(QGuiApplication* pApp, const CmdlineArgs& args) {
    CmdlineArgs::Instance().parseForUserFeedback();

    int exitCode;
    auto pCoreServices = std::make_shared<mixxx::CoreServices>(args, pApp);
#ifdef MIXXX_USE_QML
    // The user can opt in to the experimental QML UI on any platform with
    // `--qml`. On Android we used to force this on (no Qt-Widgets support
    // assumption was wrong — Qt 6 supports Widgets on Android, and on Samsung
    // DeX / large tablets the desktop skin is actually preferred), so the
    // override is now opt-in everywhere. The skin engine + LateNight skin run
    // on Android via `MixxxApplication` (QApplication) below.
    QString mainQmlFilePath;
    bool loadQml = args.isQml();
    if (!loadQml && args.getDeveloper()) {
        mixxx::skin::SkinLoader skinLoader(pCoreServices->getSettings());
        const mixxx::skin::SkinPointer pSkin = skinLoader.getConfiguredSkin();
        if (pSkin && pSkin->type() == mixxx::skin::SkinType::QML) {
            loadQml = true;
            mainQmlFilePath = pSkin->mainQmlFilePath();
        }
    }
    if (loadQml) {
        // This is a workaround to support Qt 6.4.2, currently shipped on
        // Ubuntu 24.04 See
        // https://github.com/mixxxdj/mixxx/pull/14514#issuecomment-2770811094
        // for further details
        qputenv("QT_QUICK_TABLEVIEW_COMPAT_VERSION", "6.4");
        mixxx::qml::QmlApplication qmlApplication(
                qobject_cast<QApplication*>(pApp), pCoreServices, mainQmlFilePath);
        if (!qmlApplication.isReady()) {
            exitCode = kFatalErrorOnStartupExitCode;
        } else {
            exitCode = pApp->exec();
        }
    } else
#endif
    {
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

        if (pCoreServices->getSettings()->getValue(
                    ConfigKey("[Config]", "did_run_with_unstable"), false)) {
            qInfo() << "User previously ran the unstable version on this profile";
        }

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
#ifdef Q_OS_ANDROID
            mainWindow.showMaximized();
#else
            mainWindow.show();
#endif

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
#ifdef Q_OS_ANDROID
    // Android phone/tablet builds run the fixed-pixel LateNight desktop skin.
    // Qt must know the scale factor before QApplication is constructed or
    // widget metrics, dialogs, menus, and fonts remain far too large for the
    // display. We cannot inspect QScreen before QApplication exists, so:
    //
    //   1. Use the persisted [Config]/ScaleFactor if it exists — on the
    //      second launch with the same display this gives Qt the correct
    //      widget scale from the start (no tiny menus on DeX).
    //   2. Fall back to the minimum supported skin scale (0.45) as a safe
    //      one-size-fits-phones default for first launch.
    //
    // maybeAutoDetectScaleFactor() still re-runs post-QApplication so the
    // skin parser pixmaps are correct even on first launch. Persisted value
    // matching the current display closes the Qt-widget-vs-skin scale gap.
    auto config = ConfigObject<ConfigValue>(
            QDir(pArgs->getSettingsPath()).filePath(MIXXX_SETTINGS_FILE),
            QString(),
            QString());
    const QString strScaleFactor = config.getValue(
            ConfigKey(kConfigGroup, kScaleFactorKey));
    bool ok = false;
    const double persistedScale = strScaleFactor.toDouble(&ok);
    constexpr double kAndroidDefaultScaleFactor = 0.45;
    const double scale = (ok && persistedScale > 0) ? persistedScale
                                                    : kAndroidDefaultScaleFactor;
    const QByteArray scaleFactorBytes =
            QByteArray::number(scale, 'f', 2);
    qDebug() << "Using Android ScaleFactor" << scaleFactorBytes;
    qputenv(kScaleFactorEnvVar, scaleFactorBytes);
    pArgs->setScaleFactor(scale);
    s_androidScaleSetInternally = true;
    return;
#endif
    // We cannot use SettingsManager, because it depends on MixxxApplication
    // but the scale factor is read during it's constructor.
    // QHighDpiScaling can not be used afterwards because it is private.
    // This means the following code may fail after down/upgrade ... a one time issue.

// Android can launch on very different displays with the same profile: a phone
// screen, tablet screen, or Samsung DeX/external monitor. A persisted
// [Config]/ScaleFactor from the previous display must not be exported to
// QT_SCALE_FACTOR before QApplication exists, because that freezes Qt's widget
// scaling before maybeAutoDetectScaleFactor() can inspect the current QScreen.
#ifndef Q_OS_ANDROID
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
#endif
}

// LateNight (the default skin) declares <MinimumSize>1280,668</MinimumSize>.
// On phones this overflows the screen, clipping the right deck (the FX1-4
// row visible in the original bug report). On Samsung DeX with a 1080p
// external display the same skin underuses the screen. We can't make the
// fixed-pixel skin truly responsive without a rewrite, but we can size the
// scale factor to match the available display so the skin always fits the
// current screen exactly.
//
// Re-runs on every launch (not just the first) so that opening Mixxx on a
// different display — phone screen vs. DeX, laptop vs. external monitor,
// portable settings on a new machine — picks up the right factor without
// the user having to fiddle with preferences. We persist the new value to
// [Config]/ScaleFactor so on the *next* launch with the same display it
// also takes effect for Qt's own widget scaling (QT_SCALE_FACTOR is read
// at QApplication construction time, before this function gets a QScreen
// to query, so the very first launch on a new display only scales the
// LegacySkinParser pixmaps; restarting closes the gap).
//
// Must run AFTER QApplication construction (we need QScreen).
void maybeAutoDetectScaleFactor(CmdlineArgs* pArgs) {
#ifndef Q_OS_ANDROID
    // Respect explicit user choice from the command line / env. If the user
    // passed --scale-factor or set QT_SCALE_FACTOR themselves, never override.
    if (qEnvironmentVariableIsSet(kScaleFactorEnvVar)) {
        return;
    }
#else
    // On Android only a user-provided environment variable is treated as
    // explicit. The value persisted from a previous Android launch is
    // intentionally ignored here so phone ↔ DeX/external-screen switches
    // recalculate every time the app opens. If we set the env var ourselves
    // as a safe pre-QApplication default, always proceed with detection.
    if (!s_androidScaleSetInternally &&
            qEnvironmentVariableIsSet(kScaleFactorEnvVar) &&
            !qgetenv(kScaleFactorEnvVar).isEmpty()) {
        return;
    }
#endif

    QScreen* pScreen = QGuiApplication::primaryScreen();
    if (!pScreen) {
        return;
    }
    QSize screenSize = pScreen->availableSize();
#ifdef Q_OS_ANDROID
    // On Android, QScreen::availableSize() returns logical pixels DIVIDED by
    // our QT_SCALE_FACTOR (which we set to 0.45 pre-QApplication). To compute
    // the true density-independent pixel (DIP) count we must undo the effect
    // of our own scale factor. The devicePixelRatio already accounts for the
    // display's native density; we should NOT divide by it again because Qt
    // already incorporated it into the logical coordinate system.
    //
    // Example: Samsung DeX on 1920x1080 monitor, DPR=1.5, our scale=0.45
    //   Qt reports availableSize = 1920 / (0.45 * 1.5) ≈ 2844 logical px
    //   We want physical DIPs = 1920 / 1.5 = 1280
    //   So: screenSize * ourScale * DPR / DPR = screenSize * ourScale
    //
    // Actually, Qt6 with PassThrough applies: physicalPx = logicalPx * scaleFactor * DPR
    //   logicalPx = physicalPx / (scaleFactor * DPR)
    //   physicalDIPs = physicalPx / DPR = logicalPx * scaleFactor
    //
    // We need physicalDIPs (what the user actually sees) to calculate the
    // proper scale for fitting the 1280x668 skin to the real display.
    const qreal currentScaleFactor = qgetenv(kScaleFactorEnvVar).toDouble();
    const qreal devicePixelRatio = pScreen->devicePixelRatio();
    if (currentScaleFactor > 0 && devicePixelRatio > 0) {
        // Recover physical DIPs: logical * ourScale gives us the true
        // device-independent size of the display.
        const qreal effectiveFactor = currentScaleFactor;
        screenSize = QSize(
                qRound(screenSize.width() * effectiveFactor),
                qRound(screenSize.height() * effectiveFactor));
    }
    qDebug() << "Android screen detection: availableSize=" << pScreen->availableSize()
             << "DPR=" << devicePixelRatio
             << "QT_SCALE_FACTOR=" << currentScaleFactor
             << "computed DIPs=" << screenSize;
#endif
    if (screenSize.isEmpty()) {
        return;
    }
    // LateNight nominal layout — see res/skins/LateNight/skin.xml MinimumSize.
    constexpr int kSkinNominalWidth = 1280;
    constexpr int kSkinNominalHeight = 668;
    // Reserve a little headroom so the window chrome (title bar / Android
    // system bars) doesn't clip the skin at exactly the minimum size.
    constexpr double kHeadroom = 0.95;
    const double byWidth = (screenSize.width() * kHeadroom) / kSkinNominalWidth;
    const double byHeight = (screenSize.height() * kHeadroom) / kSkinNominalHeight;
    double scale = std::min(byWidth, byHeight);
    // Clamp to a sensible range. Below 0.45 the skin becomes unreadable;
    // above 2.0 it's larger than any reasonable display benefits from.
    constexpr double kMinScale = 0.45;
    constexpr double kMaxScale = 2.0;
    scale = std::clamp(scale, kMinScale, kMaxScale);
    // Round to two decimals to keep the config human-readable and avoid
    // regenerating a slightly different float on every launch.
    scale = std::round(scale * 100.0) / 100.0;

    auto config = ConfigObject<ConfigValue>(
            QDir(pArgs->getSettingsPath()).filePath(MIXXX_SETTINGS_FILE),
            QString(),
            QString());

    const QString existingStr =
            config.getValue(ConfigKey(kConfigGroup, kScaleFactorKey));
    bool ok = false;
    const double existing = existingStr.toDouble(&ok);
    const bool sameAsConfig =
            ok && existing > 0 && std::fabs(existing - scale) < 0.005;

    qDebug() << "Auto-detected ScaleFactor" << scale << "for screen" << screenSize
             << "(LateNight nominal" << kSkinNominalWidth << "x"
             << kSkinNominalHeight << ", previous=" << existingStr << ")";

    // Apply to this run so LegacySkinParser pixmap rendering is sized
    // correctly even on the very first launch on a new display.
    pArgs->setScaleFactor(scale);
#ifdef Q_OS_ANDROID
    // Update QT_SCALE_FACTOR so that any Qt internal code that re-reads it
    // (e.g. dialog font metrics) uses the auto-detected value instead of the
    // conservative 0.45 boot default.
    qputenv(kScaleFactorEnvVar, QByteArray::number(scale, 'f', 2));
#endif

    // Persist only when it actually changed — avoids needless writes on the
    // common case where the user always opens Mixxx on the same display.
    if (!sameAsConfig) {
        config.set(ConfigKey(kConfigGroup, kScaleFactorKey),
                ConfigValue(QString::number(scale)));
        config.save();
    }
}

#ifndef Q_OS_ANDROID
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
#endif

} // anonymous namespace

#ifdef Q_OS_ANDROID
  // Currently, accessibility properties are not set, leading to a warning spam in
  // the android logcat. Furthermore, there seems to be a few issues with the default setup,
  // which leads to huge performance loss and occasional random crash.
extern "C" {
JNIEXPORT bool JNICALL
Java_org_qtproject_qt_android_QtNativeAccessibility_accessibilitySupported(JNIEnv*, jobject) {
    return false;
}
}
#endif

int main(int argc, char* argv[]) {
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
    // QCoreApplication::setOrganizationName("Mixxx");

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

    // On Android we now use the same MixxxApplication (QApplication-based)
    // as desktop builds, so the legacy skin engine works on Samsung DeX /
    // large tablets. QApplication still satisfies QGuiApplication, so the
    // optional --qml UI keeps working on top of it.
    MixxxApplication app(argc, argv);

    // Auto-fit the LateNight skin to the actual screen on first launch.
    // No-op if the user already has an explicit ScaleFactor in config or env.
    maybeAutoDetectScaleFactor(&args);

    // Re-run auto-detection when the screen geometry or primary screen
    // changes (Samsung DeX connect/disconnect, external monitor
    // plug/unplug, phone rotation). Critical on Android where DeX can
    // switch the effective display from phone-size to desktop-size at
    // runtime without restarting the app.
    auto* pScreen = QGuiApplication::primaryScreen();
    QMetaObject::Connection geometryConn;
    if (pScreen) {
        geometryConn = QObject::connect(pScreen,
                &QScreen::availableGeometryChanged,
                &app,
                [&args, &app]() {
                    qDebug() << "Screen geometry changed — re-running scale detection";
                    maybeAutoDetectScaleFactor(&args);
                });
    }
    QObject::connect(&app,
            &QGuiApplication::primaryScreenChanged,
            &app,
            [&args, &geometryConn](QScreen* pNewScreen) {
                if (!pNewScreen) {
                    return;
                }
                qDebug() << "Primary screen changed — re-running scale detection";
                maybeAutoDetectScaleFactor(&args);
                // Re-wire the geometry change listener to the new screen,
                // disconnecting the old one first to avoid duplicate listeners.
                if (geometryConn) {
                    QObject::disconnect(geometryConn);
                }
                geometryConn = QObject::connect(pNewScreen,
                        &QScreen::availableGeometryChanged,
                        &app,
                        [&args, &app]() {
                            qDebug()
                                    << "Screen geometry changed — re-running scale detection";
                            maybeAutoDetectScaleFactor(&args);
                        });
            });

#if defined(Q_OS_WIN)
    // The Mixxx style is based on Qt's WindowsVista style
    QApplication::setStyle("windowsvista");
#endif

#ifndef Q_OS_ANDROID
    applyStyleOverride(&args);

    qInfo() << "Selected Qt style:" << QApplication::style()->objectName();

#if defined(Q_OS_WIN)
    if (QApplication::style()->objectName() != "windowsvista") {
        qWarning() << "Qt style for Windows is not set to 'windowsvista'. GUI might look broken!";
    }
#endif
#endif

    auto config = ConfigObject<ConfigValue>(
            QDir(args.getSettingsPath()).filePath(MIXXX_SETTINGS_FILE),
            QString(),
            QString());
#ifndef Q_OS_ANDROID
    int notifywarningThreshold = config.getValue<int>(
            ConfigKey(kConfigGroup, kNotifyMaxDbgTimeKey), 10);
    app.setNotifyWarningThreshold(notifywarningThreshold);
#else
    Q_UNUSED(config);
#endif

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
    QObject::connect(&app, &QGuiApplication::lastWindowClosed, &app, &QCoreApplication::quit);

    int exitCode = runMixxx(&app, args);

    qDebug() << "Mixxx shutdown complete with code" << exitCode;

    mixxx::Logging::shutdown();

    return exitCode;
}
