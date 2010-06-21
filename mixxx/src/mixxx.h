/***************************************************************************
                          mixxx.h  -  description
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

#ifndef MIXXX_H
#define MIXXX_H

// include files for QT
#include <qaction.h>
#include <qmenubar.h>
#include <qtoolbutton.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QFrame>
#include <qstringlist.h>

#ifdef QT3_SUPPORT
#include <Q3Action>
#include <q3mainwindow.h>
#include <q3popupmenu.h>
#include <q3whatsthis.h>
#include <q3filedialog.h>
#else
#include <q3mainwindow.h>
#include <q3popupmenu.h>
#include <q3whatsthis.h>
#include <q3filedialog.h>
#endif
// application specific includes
#include "defs.h"
#include "mixxxview.h"
#include "controlobject.h"
#include "dlgpreferences.h"
//#include "trackplaylist.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrol.h"
#endif

#ifdef __SCRIPT__
#include "script/scriptengine.h"
#endif

class EngineMaster;
class PlayerManager;
class SamplerManager;
class TrackInfoObject;
class PlayerProxy;
class BpmDetector;
class QSplashScreen;
class ScriptEngine;
class Player;
class LibraryScanner;
class AnalyserQueue;
class Library;
class MidiDeviceManager;

/**
  * This Class is the base class for Mixxx. It sets up the main
  * window and providing a menubar.
  * For the main view, an instance of class MixxxView is
  * created which creates your view.
  */
class MixxxApp : public QMainWindow
{
  Q_OBJECT

  public:
    /** Construtor. files is a list of command line arguments */
    MixxxApp(QApplication *app, struct CmdlineArgs args);
    /** destructor */
    ~MixxxApp();
    /** initializes all QActions of the application */
    void initActions();
    /** initMenuBar creates the menu_bar and inserts the menuitems */
    void initMenuBar();
    /** overloaded for Message box on last window exit */
    bool queryExit();

    void rebootMixxxView();

  public slots:

    //void slotQuitFullScreen();
    /** Opens a file in player 1 */
    void slotFileLoadSongPlayer1();
    /** Opens a file in player 2 */
    void slotFileLoadSongPlayer2();
    /** exits the application */
    void slotFileQuit();

    /** toogle ipod active - Don't #ifdef this because MOC is dumb**/
    void slotiPodToggle(bool toggle);

    /** toggle audio beat marks */
    void slotOptionsBeatMark(bool toggle);
    /** toggle vinyl control - Don't #ifdef this because MOC is dumb**/
    void slotOptionsVinylControl(bool toggle);
    /** toggle recording - Don't #ifdef this because MOC is dumb**/
    void slotOptionsRecord(bool toggle);
    /** toogle full screen mode */
    void slotOptionsFullScreen(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** shows an about dlg*/
    void slotHelpAbout();
    /** visits support section of website*/
    void slotHelpSupport();
    /** Change of file to play */
    //void slotChangePlay(int,int,int, const QPoint &);
	QString getSkinPath();

    void slotlibraryMenuAboutToShow();

	/** Scan or rescan the music library directory */
	void slotScanLibrary();
	/** Enables the "Rescan Library" menu item. This gets disabled when a scan is running.*/
	void slotEnableRescanLibraryAction();

  protected:
    /** Event filter to block certain events (eg. tooltips if tooltips are disabled) */
    bool eventFilter(QObject *obj, QEvent *event);

  private:
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    MixxxView *view;
    QFrame *frame;

    QApplication *app;
    // The mixing engine.
    EngineMaster *m_pEngine;

    // The sound manager
    SoundManager *soundmanager;

    PlayerManager* m_pPlayerManager;
    SamplerManager* m_pSamplerManager;

    MidiDeviceManager *m_pMidiDeviceManager;
    ControlObject *control;
    ConfigObject<ConfigValue> *config;
    /** Pointer to active keyboard configuration */
    ConfigObject<ConfigValueKbd> *kbdconfig;
    /** Library scanner object */
    LibraryScanner* m_pLibraryScanner;
    // The library management object
    Library* m_pLibrary;

    /** file_menu contains all items of the menubar entry "File" */
    QMenu *fileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    QMenu *editMenu;
    /** library menu */
    QMenu *libraryMenu;
    /** options_menu contains all items of the menubar entry "Options" */
    QMenu *optionsMenu;
    /** view_menu contains all items of the menubar entry "View" */
    QMenu *viewMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    QMenu *helpMenu;

#ifdef __SCRIPT__
    QMenu *macroMenu;
#endif

    /** actions for the application initialized in initActions() and used to en/disable them
      * according to your needs during the program */
    QAction *fileNew;
    QAction *fileLoadSongPlayer1;
    QAction *fileLoadSongPlayer2;
    QAction *fileSave;
    QAction *fileSaveAs;
    QAction *fileClose;
    QAction *filePrint;
    QAction *fileQuit;

    QAction *editCut;
    QAction *editCopy;
    QAction *editPaste;

    QAction *playlistsNew;
    QAction *playlistsImport;
    QAction **playlistsList;

    QAction *iPodToggle;

    QAction *batchBpmDetect;

    QAction *libraryRescan;

    QAction *optionsBeatMark;
#ifdef __VINYLCONTROL__
    QAction *optionsVinylControl;
#endif
    QAction *optionsRecord;
    QAction *optionsFullScreen;
    QAction *optionsPreferences;

    QAction *helpAboutApp;
    QAction *helpSupport;
#ifdef __SCRIPT__
    QAction *macroStudio;
#endif
    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences *prefDlg;

#ifdef __SCRIPT__
    ScriptEngine *scriptEng;
#endif

    int noSoundDlg(void);
    // Fullscreen patch
    QPoint winpos;
};

//A structure to store the parsed command-line arguments
struct CmdlineArgs
{
    QList<QString> qlMusicFiles;    /* List of files to load into players at startup */
    bool bStartInFullscreen;        /* Start in fullscreen mode */
};


#endif
