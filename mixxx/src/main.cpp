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

#include <qapplication.h>
#include <qfont.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qmessagebox.h>
#include <qiodevice.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qstringlist.h>
#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#include "mixxx.h"
#include "qpixmap.h"
#include "qsplashscreen.h"
#include "log.h"

#ifdef __LADSPA__
#include <ladspa/ladspaloader.h>
#endif

#ifdef Q_WS_WIN
#ifdef DEBUGCONSOLE
#include <io.h> // Debug Console
#include <windows.h>

void InitDebugConsole() { // Open a Debug Console so we can printf
    int fd;
    FILE * fp;

    FreeConsole();
    if (AllocConsole()) {
        SetConsoleTitleA("Mixxx Debug Messages");

        fd = _open_osfhandle( (long)GetStdHandle( STD_OUTPUT_HANDLE ), 0);
        fp = _fdopen( fd, "w" );

        *stdout = *fp;
        setvbuf( stdout, NULL, _IONBF, 0 );

        fd = _open_osfhandle( (long)GetStdHandle( STD_ERROR_HANDLE ), 0);
        fp = _fdopen( fd, "w" );

        *stderr = *fp;
        setvbuf( stderr, NULL, _IONBF, 0 );
    }
}
#endif // DEBUGCONSOLE
#endif // Q_WS_WIN

QApplication * a;

void qInitImages_mixxx();

void MessageOutput( QtMsgType type, const char * msg )
{
    switch ( type ) {
    case QtDebugMsg:
#ifdef Q_WS_WIN
        if (strstr(msg, "doneCurrent")) {
            break;
        }
#endif
        fprintf( stderr, "Debug: %s\n", msg );
        break;
    case QtWarningMsg:
        fprintf( stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf( stderr, "Critical: %s\n", msg );
        QMessageBox::warning(0, "Mixxx", msg);
        exit(-1);
        break;
    case QtFatalMsg:
        fprintf( stderr, "Fatal: %s\n", msg );
        QMessageBox::warning(0, "Mixxx", msg);
        abort();
    }
}

QFile Logfile; // global logfile variable

void MessageToLogfile( QtMsgType type, const char * msg )
{
    Q3TextStream Log( &Logfile );
    switch ( type ) {
    case QtDebugMsg:
        Log << "Debug: " << msg << "\n";
        break;
    case QtWarningMsg:
        Log << "Warning: " << msg << "\n";
        a->lock();
        QMessageBox::warning(0, "Mixxx", msg);
        a->unlock();
        break;
    case QtCriticalMsg:
        fprintf( stderr, "Critical: %s\n", msg );
        QMessageBox::warning(0, "Mixxx", msg);
        exit(-1);
        break;
    case QtFatalMsg:
        fprintf( stderr, "Fatal: %s\n", msg );
        QMessageBox::warning(0, "Mixxx", msg);
        abort();
    }
    Logfile.flush();
}


int main(int argc, char * argv[])
{
    // Check if an instance of Mixxx is already running


#ifdef Q_WS_WIN
    // For windows write all debug messages to a logfile:
    Logfile.setName( "mixxx.log" );
  #ifndef QT3_SUPPORT
    Logfile.open(IO_WriteOnly | IO_Translate);
  #else
    Logfile.open(QIODevice::WriteOnly | QIODevice::Text);
  #endif
  #ifdef DEBUGCONSOLE
    InitDebugConsole();
    qInstallMsgHandler( MessageOutput );
  #else
    qInstallMsgHandler( MessageToLogfile );
  #endif
#else
    // For others, write to the console:
    qInstallMsgHandler( MessageOutput );
#endif

    a = new QApplication(argc, argv);

//Hack to make QT4 find the plugins when packaging in the Mixxx.app bundle on OS X.
//See http://doc.trolltech.com/4.3/deployment-mac.html for details.
#ifdef __MACX__
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif


#ifdef __LADSPA__
    //LADSPALoader ladspaloader;
#endif

    // Show splash
    QSplashScreen * pSplash = 0;
    /*
       QPixmap pixmap("splash.png");
       pSplash = new QSplashScreen(pixmap);
       pSplash->show();
       pSplash->message("Loading...",Qt::AlignLeft|Qt::AlignBottom);
     */

    QTranslator tor( 0 );
    // set the location where your .qm files are in load() below as the last parameter instead of "."
    // for development, use "/" to use the english original as
    // .qm files are stored in the base project directory.
    tor.load( QString("mixxx.") + QTextCodec::locale(), "." );
    a->installTranslator( &tor );

    // Check if one of the command line arguments is "--no-visuals"
//    bool bVisuals = true;
//    for (int i=0; i<argc; ++i)
//        if(QString("--no-visuals")==argv[i])
//            bVisuals = false;

    // Construct a list of strings based on the command line arguments
    struct CmdlineArgs args;
    args.bStartInFullscreen = false; //Initialize vars

    QString qLogFileName = "";
    for (int i=0; i<argc; ++i)
    {
        if (argv[i]==QString("--log"))
        {
            args.qLogFileName = argv[i+1];
            i++;
        }
        if (argv[i]==QString("-f") || argv[i]==QString("--f"))
        {
            args.bStartInFullscreen = true;
        }
        else
            args.qlMusicFiles += argv[i];
    }

    MixxxApp * mixxx=new MixxxApp(a, args, pSplash);
    
    a->setMainWidget(mixxx);

    mixxx->show();

    if (pSplash)
    {
        pSplash->finish(mixxx);
        delete pSplash;
    }

    int result = a->exec();
    delete mixxx;
    return result;
}

