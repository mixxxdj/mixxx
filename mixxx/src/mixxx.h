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
#include <qdom.h>
#include <qmenubar.h>
#include <qtoolbutton.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qapplication.h>
#include <QList>
//Added by qt3to4:
#include <QFrame>
#include <qstringlist.h>

// application specific includes
#include "defs.h"
#include "trackinfoobject.h"
#include "engine/enginemaster.h"
#include "controlobject.h"
#include "dlgpreferences.h"
//#include "trackplaylist.h"
#include "recording/recordingmanager.h"

#ifdef __SCRIPT__
#include "script/scriptengine.h"
#endif

class EngineMaster;
class PlayerManager;
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
class MixxxKeyboard;
class SkinLoader;

class VinylControlManager;

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
    virtual ~MixxxApp();
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

    /** toggle audio beat marks */
    void slotOptionsBeatMark(bool toggle);
    /** toggle vinyl control - Don't #ifdef this because MOC is dumb**/
    void slotControlVinylControl(double toggle);
    void slotCheckboxVinylControl(bool toggle);
    void slotControlVinylControl2(double toggle);
    void slotCheckboxVinylControl2(bool toggle);
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
    // Visits a feedback form
    void slotHelpFeedback();
    /** Change of file to play */
    //void slotChangePlay(int,int,int, const QPoint &);

    void slotlibraryMenuAboutToShow();
    /** Scan or rescan the music library directory */
    void slotScanLibrary();
    /** Enables the "Rescan Library" menu item. This gets disabled when a scan is running.*/
    void slotEnableRescanLibraryAction();
    /**Updates the checkboxes for Recording and Livebroadcasting when connection drops, or lame is not available **/
    void slotOptionsMenuShow();
    /** toggles Livebroadcasting **/
    void slotOptionsShoutcast(bool value);



  protected:
    /** Event filter to block certain events (eg. tooltips if tooltips are disabled) */
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);



  private:
    void checkDirectRendering();
    bool confirmExit();

    // Pointer to the root GUI widget
    QWidget* m_pView;
    QWidget* m_pWidgetParent;

    QApplication *m_pApp;

    // The mixing engine.
    EngineMaster *m_pEngine;

    // The skin loader
    SkinLoader* m_pSkinLoader;

    // The sound manager
    SoundManager *m_pSoundManager;

    // Keeps track of players
    PlayerManager* m_pPlayerManager;
    // RecordingManager
    RecordingManager* m_pRecordingManager;

    MidiDeviceManager *m_pMidiDeviceManager;

    ConfigObject<ConfigValue> *m_pConfig;

    VinylControlManager *m_pVCManager;

    MixxxKeyboard* m_pKeyboard;
    /** Library scanner object */
    LibraryScanner* m_pLibraryScanner;
    // The library management object
    Library* m_pLibrary;

    /** file_menu contains all items of the menubar entry "File" */
    QMenu *m_pFileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    QMenu *m_pEditMenu;
    /** library menu */
    QMenu *m_pLibraryMenu;
    /** options_menu contains all items of the menubar entry "Options" */
    QMenu *m_pOptionsMenu;
    /** view_menu contains all items of the menubar entry "View" */
    QMenu *m_pViewMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    QMenu *m_pHelpMenu;

#ifdef __SCRIPT__
    QMenu *macroMenu;
#endif

    QAction *m_pFileLoadSongPlayer1;
    QAction *m_pFileLoadSongPlayer2;
    QAction *m_pFileQuit;

    QAction *m_pPlaylistsNew;
    QAction *m_pCratesNew;
    QAction *m_pPlaylistsImport;
    QAction **m_pPlaylistsList;

    QAction *m_pBatchBpmDetect;

    QAction *m_pLibraryRescan;

    QAction *m_pOptionsBeatMark;

#ifdef __VINYLCONTROL__
    QMenu *m_pVinylControlMenu;
    QAction *m_pOptionsVinylControl;
    QAction *m_pOptionsVinylControl2;
#endif
    QAction *m_pOptionsRecord;
    QAction *m_pOptionsFullScreen;
    QAction *m_pOptionsPreferences;
#ifdef __SHOUTCAST__
    QAction *m_pOptionsShoutcast;
#endif

    QAction *m_pHelpAboutApp;
    QAction *m_pHelpSupport;
    QAction *m_pHelpFeedback;
#ifdef __SCRIPT__
    QAction *macroStudio;
#endif
    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences *m_pPrefDlg;

#ifdef __SCRIPT__
    ScriptEngine *scriptEng;
#endif

    int noSoundDlg(void);
    int noOutputDlg(bool *continueClicked);
    // Fullscreen patch
    QPoint m_winpos;
};

//A structure to store the parsed command-line arguments
struct CmdlineArgs
{
    QList<QString> qlMusicFiles;    /* List of files to load into players at startup */
    bool bStartInFullscreen;        /* Start in fullscreen mode */
    QString locale;
};


#endif
