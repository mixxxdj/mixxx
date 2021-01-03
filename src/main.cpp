#include <QApplication>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QThread>
#include <QtDebug>

#include "coreservices.h"
#include "errordialoghandler.h"
#include "mixxx.h"
#include "mixxxapplication.h"
#include "sources/soundsourceproxy.h"
#include "util/cmdlineargs.h"
#include "util/console.h"
#include "util/logging.h"
#include "util/version.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

namespace {

// Exit codes
constexpr int kFatalErrorOnStartupExitCode = 1;
constexpr int kParseCmdlineArgsErrorExitCode = 2;

int runMixxx(MixxxApplication* app, const CmdlineArgs& args) {
    auto coreServices = std::make_shared<mixxx::CoreServices>(args);
    MixxxMainWindow mainWindow(app, coreServices);
    // If startup produced a fatal error, then don't even start the
    // Qt event loop.
    if (ErrorDialogHandler::instance()->checkError()) {
        return kFatalErrorOnStartupExitCode;
    } else {
        qDebug() << "Displaying main window";
        mainWindow.show();

        qDebug() << "Running Mixxx";
        return app->exec();
    }
}

} // anonymous namespace

int main(int argc, char * argv[]) {
    Console console;

#ifdef Q_OS_LINUX
    XInitThreads();
#endif

    // These need to be set early on (not sure how early) in order to trigger
    // logic in the OS X appstore support patch from QTBUG-16549.
    QCoreApplication::setOrganizationDomain("mixxx.org");

    // This needs to be set before initializing the QApplication.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // workaround for https://bugreports.qt.io/browse/QTBUG-84363
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && QT_VERSION < QT_VERSION_CHECK(5, 15, 1)
    qputenv("QV4_FORCE_INTERPRETER", QByteArrayLiteral("1"));
#endif

    // Setting the organization name results in a QDesktopStorage::DataLocation
    // of "$HOME/Library/Application Support/Mixxx/Mixxx" on OS X. Leave the
    // organization name blank.
    //QCoreApplication::setOrganizationName("Mixxx");

    QCoreApplication::setApplicationName(Version::applicationName());
    QCoreApplication::setApplicationVersion(Version::version());

    // Construct a list of strings based on the command line arguments
    CmdlineArgs& args = CmdlineArgs::Instance();
    if (!args.Parse(argc, argv)) {
        args.printUsage();
        return kParseCmdlineArgsErrorExitCode;
    }

    // If you change this here, you also need to change it in
    // ErrorDialogHandler::errorDialog(). TODO(XXX): Remove this hack.
    QThread::currentThread()->setObjectName("Main");

    // Create the ErrorDialogHandler in the main thread, otherwise it will be
    // created in the thread of the first caller to instance(), which may not be
    // the main thread. Bug #1748636.
    ErrorDialogHandler::instance();

    MixxxApplication app(argc, argv);


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
