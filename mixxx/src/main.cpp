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
#include <qapplication.h>
#include <qfont.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qmessagebox.h>
#include <qiodevice.h>
#include <qfile.h>
#include <qstringlist.h>
#include <stdio.h>
#include <math.h>
#include "mixxx.h"
#include "soundsourceproxy.h"
#include "qpixmap.h"
#include "qsplashscreen.h"
#include "errordialoghandler.h"
#include "defs_version.h"

#ifdef __LADSPA__
#include <ladspa/ladspaloader.h>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef __WINDOWS__
#ifdef DEBUGCONSOLE
#include <io.h> // Debug Console
#include <windows.h>

void InitDebugConsole() { // Open a Debug Console so we can printf
    int fd;
    FILE *fp;

    FreeConsole();
    if (AllocConsole()) {
        SetConsoleTitleA("Mixxx Debug Messages");

        fd = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), 0);
        fp = _fdopen(fd, "w");

        *stdout = *fp;
        setvbuf(stdout, NULL, _IONBF, 0);

        fd = _open_osfhandle((long) GetStdHandle(STD_ERROR_HANDLE), 0);
        fp = _fdopen(fd, "w");

        *stderr = *fp;
        setvbuf(stderr, NULL, _IONBF, 0);
    }
}
#endif // DEBUGCONSOLE
#endif // __WINDOWS__

QApplication *a;

QStringList plugin_paths; //yes this is global. sometimes global is good.

//void qInitImages_mixxx();

QFile Logfile; // global logfile variable
QMutex mutexLogfile;

/* Debug message handler which outputs to both a logfile and a
 * and prepends the thread the message came from too.
 */
void MessageHandler(QtMsgType type, const char *input)
{
    QMutexLocker locker(&mutexLogfile);
    QByteArray ba;
    QThread* thread = QThread::currentThread();
    if (thread) {
        ba = "[" + QThread::currentThread()->objectName().toLocal8Bit() + "]: ";
    } else {
        ba = "[?]: ";
    }
    ba += input;
    ba += "\n";

    if(!Logfile.isOpen())
    {
        // This Must be done in the Message Handler itself, to guarantee that the
        // QApplication is initialized
        QString logFileName = CmdlineArgs::Instance().getSettingsPath() + "/mixxx.log";

        // XXX will there ever be a case that we can't write to our current
        // working directory?
        Logfile.setFileName(logFileName);
        Logfile.open(QIODevice::WriteOnly | QIODevice::Text);
    }

    ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();

    switch (type) {
    case QtDebugMsg:
#ifdef __WINDOWS__  //wtf? -kousu 2/2009
        if (strstr(input, "doneCurrent")) {
            break;
        }
#endif
        fprintf(stderr, "Debug %s", ba.constData());
        Logfile.write("Debug ");
        Logfile.write(ba);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning %s", ba.constData());
        Logfile.write("Warning ");
        Logfile.write(ba);
        // Don't use qWarning for reporting user-facing errors.
        //dialogHandler->requestErrorDialog(DLG_WARNING,input);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical %s", ba.constData());
        Logfile.write("Critical ");
        Logfile.write(ba);
        Logfile.flush();    // Ensure the error is written to the log before exiting
        dialogHandler->requestErrorDialog(DLG_CRITICAL,input);
//         exit(-1);    // Done in ErrorDialogHandler
        break; //NOTREACHED
    case QtFatalMsg:
        fprintf(stderr, "Fatal %s", ba.constData());
        Logfile.write("Fatal ");
        Logfile.write(ba);
        Logfile.flush();    // Ensure the error is written to the log before aborting
        dialogHandler->requestErrorDialog(DLG_FATAL,input);
//         abort();    // Done in ErrorDialogHandler
        break; //NOTREACHED
    }
    Logfile.flush();
}

int main(int argc, char * argv[])
{

#ifdef Q_WS_X11
    XInitThreads();
#endif

    // Check if an instance of Mixxx is already running
    // See http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtsingleapplication

    // These need to be set early on (not sure how early) in order to trigger
    // logic in the OS X appstore support patch from QTBUG-16549.
    QCoreApplication::setOrganizationDomain("mixxx.org");
    QCoreApplication::setOrganizationName("Mixxx");

    // Construct a list of strings based on the command line arguments
    CmdlineArgs& args = CmdlineArgs::Instance();
    if (!args.Parse(argc, argv)) {
        fputs("Mixxx digital DJ software v", stdout);
        fputs(VERSION, stdout);
        fputs(" - Command line options", stdout);
        fputs(
                   "\n(These are case-sensitive.)\n\n\
    [FILE]                  Load the specified music file(s) at start-up.\n\
                            Each must be one of the following file types:\n\
                            ", stdout);

        QString fileExtensions = SoundSourceProxy::supportedFileExtensionsString();
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
                            %s\n", args.getSettingsPath().toLocal8Bit().data());
        fputs("\
\n\
    --controllerDebug       Causes Mixxx to display/log all of the controller\n\
                            data it receives and script functions it loads\n\
\n\
    --developer             Enables developer-mode. Includes extra log info,\n\
                            stats on performance, and a Developer tools menu.\n\
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

    //it seems like this code should be inline in MessageHandler() but for some reason having it there corrupts the messages sometimes -kousu 2/2009

#ifdef __WINDOWS__
  #ifdef DEBUGCONSOLE
    InitDebugConsole();
  #endif
#endif
    qInstallMsgHandler(MessageHandler);

    // Other things depend on this name to enforce thread exclusivity,
    //  so if you change it here, change it also in:
    //      * ErrorDialogHandler::errorDialog()
    QThread::currentThread()->setObjectName("Main");
    QApplication a(argc, argv);

    //Enumerate and load SoundSource plugins
    SoundSourceProxy::loadPlugins();
#ifdef __LADSPA__
    //LADSPALoader ladspaloader;
#endif

    // Check if one of the command line arguments is "--no-visuals"
//    bool bVisuals = true;
//    for (int i=0; i<argc; ++i)
//        if(QString("--no-visuals")==argv[i])
//            bVisuals = false;


    // set up the plugin paths...
    /*
    qDebug() << "Setting up plugin paths...";
    plugin_paths = QStringList();
    QString ladspaPath = QString(getenv("LADSPA_PATH"));

    if (!ladspaPath.isEmpty())
    {
        // get the list of directories containing LADSPA plugins
#ifdef __WINDOWS__
        //paths.ladspaPath.split(';');
#else  //this doesn't work, I think we need to iterate over the splitting to do it properly
        //paths = ladspaPath.split(':');
#endif
    }
    else
    {
        // add default path if LADSPA_PATH is not set
#ifdef __LINUX__
        plugin_paths.push_back ("/usr/lib/ladspa/");
        plugin_paths.push_back ("/usr/lib64/ladspa/");
#elif __APPLE__
      QDir dir(a.applicationDirPath());
     dir.cdUp();
     dir.cd("PlugIns");
         plugin_paths.push_back ("/Library/Audio/Plug-ins/LADSPA");
         plugin_paths.push_back (dir.absolutePath()); //ladspa_plugins directory in Mixxx.app bundle //XXX work in QApplication::appdir()
#elif __WINDOWS__
        // not tested yet but should work:
        QString programFiles = QString(getenv("ProgramFiles"));
         plugin_paths.push_back (programFiles+"\\LADSPA Plugins");
         plugin_paths.push_back (programFiles+"\\Audacity\\Plug-Ins");
#endif
    }
    qDebug() << "...done.";
    */


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

    MixxxApp* mixxx = new MixxxApp(&a, args);

    //a.setMainWidget(mixxx);
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    int result = -1;

    if (!(ErrorDialogHandler::instance()->checkError())) {
        qDebug() << "Displaying mixxx";
        mixxx->show();

        qDebug() << "Running Mixxx";
        result = a.exec();
    }

    delete mixxx;

    qDebug() << "Mixxx shutdown complete with code" << result;

    qInstallMsgHandler(0); //Reset to default.

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

