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
#include <QIODevice>
#include <QFile>

#include <stdio.h>
#include <iostream>

#include "mixxx.h"
#include "mixxxapplication.h"
#include "soundsourceproxy.h"
#include "errordialoghandler.h"
#include "util/cmdlineargs.h"
#include "util/version.h"
#include "util/console.h"

#include <QFile>
#include <QFileInfo>
#ifdef __FFMPEGFILE__
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#endif

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

QStringList plugin_paths; //yes this is global. sometimes global is good.

//void qInitImages_mixxx();

QFile Logfile; // global logfile variable
QMutex mutexLogfile;

/* Debug message handler which outputs to both a logfile and a
 * and prepends the thread the message came from too.
 */
void MessageHandler(QtMsgType type,
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                    const char* input) {
#else
                    const QMessageLogContext&, const QString& input) {
#endif

    // It's possible to deadlock if any method in this function can
    // qDebug/qWarning/etc. Writing to a closed QFile, for example, produces a
    // qWarning which causes a deadlock. That's why every use of Logfile is
    // wrapped with isOpen() checks.
    QMutexLocker locker(&mutexLogfile);
    QByteArray ba;
    QThread* thread = QThread::currentThread();
    if (thread) {
        ba = "[" + QThread::currentThread()->objectName().toLocal8Bit() + "]: ";
    } else {
        ba = "[?]: ";
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    ba += input;
#else
    ba += input.toLocal8Bit();
#endif
    ba += "\n";

    if (!Logfile.isOpen()) {
        // This Must be done in the Message Handler itself, to guarantee that the
        // QApplication is initialized
        QString logLocation = CmdlineArgs::Instance().getSettingsPath();
        QString logFileName;

        // Rotate old logfiles
        //FIXME: cerr << doesn't get printed until after mixxx quits (???)
        for (int i = 9; i >= 0; --i) {
            if (i == 0) {
                logFileName = QDir(logLocation).filePath("mixxx.log");
            } else {
                logFileName = QDir(logLocation).filePath(QString("mixxx.log.%1").arg(i));
            }
            QFileInfo logbackup(logFileName);
            if (logbackup.exists()) {
                QString olderlogname =
                        QDir(logLocation).filePath(QString("mixxx.log.%1").arg(i + 1));
                // This should only happen with number 10
                if (QFileInfo(olderlogname).exists()) {
                    QFile::remove(olderlogname);
                }
                if (!QFile::rename(logFileName, olderlogname)) {
                    std::cerr << "Error rolling over logfile " << logFileName.toStdString();
                }
            }
        }

        // WARNING(XXX) getSettingsPath() may not be ready yet. This causes
        // Logfile writes below to print qWarnings which in turn recurse into
        // MessageHandler -- potentially deadlocking.
        // XXX will there ever be a case that we can't write to our current
        // working directory?
        Logfile.setFileName(logFileName);
        Logfile.open(QIODevice::WriteOnly | QIODevice::Text);
    }

    switch (type) {
    case QtDebugMsg:
#ifdef __WINDOWS__  //wtf? -kousu 2/2009
        if (strstr(input, "doneCurrent")) {
            break;
        }
#endif
        fprintf(stderr, "Debug %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Debug ");
            Logfile.write(ba);
        }
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Warning ");
            Logfile.write(ba);
        }
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Critical ");
            Logfile.write(ba);
        }
        break; //NOTREACHED
    case QtFatalMsg:
        fprintf(stderr, "Fatal %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Fatal ");
            Logfile.write(ba);
        }
        break; //NOTREACHED
    default:
        fprintf(stderr, "Unknown %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Unknown ");
            Logfile.write(ba);
        }
        break;
    }
    if (Logfile.isOpen()) {
        Logfile.flush();
    }
}

int main(int argc, char * argv[])
{
    Console console;

#ifdef Q_OS_LINUX
    XInitThreads();
#endif

    // Check if an instance of Mixxx is already running
    // See http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtsingleapplication

    // These need to be set early on (not sure how early) in order to trigger
    // logic in the OS X appstore support patch from QTBUG-16549.
    QCoreApplication::setOrganizationDomain("mixxx.org");

    // Setting the organization name results in a QDesktopStorage::DataLocation
    // of "$HOME/Library/Application Support/Mixxx/Mixxx" on OS X. Leave the
    // organization name blank.
    //QCoreApplication::setOrganizationName("Mixxx");

    QCoreApplication::setApplicationName("Mixxx");
    QString mixxxVersion = Version::version();
    QByteArray mixxxVersionBA = mixxxVersion.toLocal8Bit();
    QCoreApplication::setApplicationVersion(mixxxVersion);

    // Construct a list of strings based on the command line arguments
    CmdlineArgs& args = CmdlineArgs::Instance();
    if (!args.Parse(argc, argv)) {
        fputs("Mixxx DJ Software v", stdout);
        fputs(mixxxVersionBA.constData(), stdout);
        fputs(" - Command line options", stdout);
        fputs(
                   "\n(These are case-sensitive.)\n\n\
    [FILE]                  Load the specified music file(s) at start-up.\n\
                            Each must be one of the following file types:\n\
                            ", stdout);

        QString fileExtensions(SoundSourceProxy::getSupportedFileNamePatterns().join(" "));
        QByteArray fileExtensionsBA = QString(fileExtensions).toUtf8();
        fputs(fileExtensionsBA.constData(), stdout);
        fputs("\n\n", stdout);
        fputs("\
                            Each file you specify will be loaded into the\n\
                            next virtual deck.\n\
\n\
    --resourcePath PATH     Top-level directory where Mixxx should look\n\
                            for its resource files such as MIDI mappings,\n\
                            overriding the default installation location.\n\
\n\
    --pluginPath PATH       Top-level directory where Mixxx should look\n\
                            for sound source plugins in addition to default\n\
                            locations.\n\
\n\
    --settingsPath PATH     Top-level directory where Mixxx should look\n\
                            for settings. Default is:\n", stdout);
        fprintf(stdout, "\
                            %s\n", args.getSettingsPath().toLocal8Bit().constData());
        fputs("\
\n\
    --controllerDebug       Causes Mixxx to display/log all of the controller\n\
                            data it receives and script functions it loads\n\
\n\
    --developer             Enables developer-mode. Includes extra log info,\n\
                            stats on performance, and a Developer tools menu.\n\
\n\
    --safeMode              Enables safe-mode. Disables OpenGL waveforms,\n\
                            and spinning vinyl widgets. Try this option if\n\
                            Mixxx is crashing on startup.\n\
\n\
    --locale LOCALE         Use a custom locale for loading translations\n\
                            (e.g 'fr')\n\
\n\
    -f, --fullScreen        Starts Mixxx in full-screen mode\n\
\n\
    -h, --help              Display this help message and exit", stdout);

        fputs("\n\n(For more information, see http://mixxx.org/wiki/doku.php/command_line_options)\n",stdout);
        return(0);
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(MessageHandler);
#else
    qInstallMessageHandler(MessageHandler);
#endif

    // Other things depend on this name to enforce thread exclusivity,
    //  so if you change it here, change it also in:
    //      * ErrorDialogHandler::errorDialog()
    QThread::currentThread()->setObjectName("Main");
    MixxxApplication a(argc, argv);

    // Support utf-8 for all translation strings. Not supported in Qt 5.
    // TODO(rryan): Is this needed when we switch to qt5? Some sources claim it
    // isn't.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

#ifdef __FFMPEGFILE__
     av_register_all();
     avcodec_register_all();
#endif

     //Enumerate and load SoundSource plugins
     SoundSourceProxy::loadPlugins();

    // Check if one of the command line arguments is "--no-visuals"
//    bool bVisuals = true;
//    for (int i=0; i<argc; ++i)
//        if(QString("--no-visuals")==argv[i])
//            bVisuals = false;


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

    MixxxMainWindow* mixxx = new MixxxMainWindow(&a, args);

    //a.setMainWidget(mixxx);
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    int result = -1;

    if (!(ErrorDialogHandler::instance()->checkError())) {
        qDebug() << "Displaying mixxx";
        mixxx->show();

        qDebug() << "Running Mixxx";
        result = a.exec();
    } else {
        mixxx->finalize();
    }

    delete mixxx;

    qDebug() << "Mixxx shutdown complete with code" << result;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(NULL);  // Reset to default.
#else
    qInstallMessageHandler(NULL);  // Reset to default.
#endif

    // Don't make any more output after this
    //    or mixxx.log will get clobbered!
    { // scope
        QMutexLocker locker(&mutexLogfile);
        if(Logfile.isOpen()) {
            Logfile.close();
        }
    }

    //delete plugin_paths;
    return result;
}
