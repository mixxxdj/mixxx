/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QThread>
#include <QDir>
#include <QtDebug>
#include <QApplication>
#include <QStringList>
#include <QString>
#include <QTextCodec>

#include "mixxx.h"
#include "mixxxapplication.h"
#include "sources/soundsourceproxy.h"
#include "errordialoghandler.h"
#include "util/cmdlineargs.h"
#include "util/console.h"
#include "util/logging.h"
#include "util/version.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

namespace {

int runMixxx(MixxxApplication* app, const CmdlineArgs& args) {
    int result = -1;
    MixxxMainWindow mainWindow(app, args);
    // If startup produced a fatal error, then don't even start the
    // Qt event loop.
    if (ErrorDialogHandler::instance()->checkError()) {
        mainWindow.finalize();
    } else {
        qDebug() << "Displaying main window";
        mainWindow.show();

        qDebug() << "Running Mixxx";
        result = app->exec();
    }
    return result;
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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
        return 0;
    }

    // If you change this here, you also need to change it in
    // ErrorDialogHandler::errorDialog(). TODO(XXX): Remove this hack.
    QThread::currentThread()->setObjectName("Main");

    // Create the ErrorDialogHandler in the main thread, otherwise it will be
    // created in the thread of the first caller to instance(), which may not be
    // the main thread. Bug #1748636.
    ErrorDialogHandler::instance();

    mixxx::Logging::initialize(args.getSettingsPath(),
                               args.getLogLevel(),
                               args.getLogFlushLevel(),
                               args.getDebugAssertBreak());

    MixxxApplication app(argc, argv);

    // Support utf-8 for all translation strings. Not supported in Qt 5.
    // TODO(rryan): Is this needed when we switch to qt5? Some sources claim it
    // isn't.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    SoundSourceProxy::registerSoundSourceProviders();

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
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    int result = runMixxx(&app, args);

    qDebug() << "Mixxx shutdown complete with code" << result;

    mixxx::Logging::shutdown();

    return result;
}
