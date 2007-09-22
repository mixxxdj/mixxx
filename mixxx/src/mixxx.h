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
#include <vector>
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
#include "enginebuffer.h"
#include "enginechannel.h"
#include "enginemaster.h"
#include "controlobject.h"
#include "dlgpreferences.h"
#include "trackplaylist.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrol.h"
#endif

#ifdef __SCRIPT__
#include "script/scriptengine.h"
#endif

#ifdef __LADSPA__
#include "dlgladspa.h"
#endif

class WVisual;
class Track;
class TrackInfoObject;
class PlayerProxy;
class WaveSummary;
class BpmDetector;
class QSplashScreen;
class ScriptEngine;
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
    MixxxApp(QApplication *app, QStringList files, QSplashScreen *pSplash, QString qLogFileName);
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
    void slotFileOpen();
    /** exits the application */
    void slotFileQuit();
    /** toggle audio beat marks */
    void slotOptionsBeatMark(bool toggle);
    /** toggle vinyl control - Don't #ifdef this because MOC is dumb**/
    void slotOptionsVinylControl(bool toggle);
    /** toogle full screen mode */
    void slotOptionsFullScreen(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** shows an about dlg*/
    void slotHelpAbout();
    /** Change of file to play */
    //void slotChangePlay(int,int,int, const QPoint &);
	QString getSkinPath();
  private:
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    MixxxView *view;
    /** Pointer to waveform summary generator */
    WaveSummary *m_pWaveSummary;
    /** Pointer to the bpm detector */
    BpmDetector *m_pBpmDetector;

    QFrame *frame;

    QApplication *app;
    EngineObject *engine;
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineMaster *master;
    //PlayerProxy *player;
    SoundManager *soundmanager;
    MidiObject *midi;
    ControlObject *control;
    ConfigObject<ConfigValue> *config;
    /** Pointer to active midi configuration */
    ConfigObject<ConfigValueMidi> *midiconfig;
    /** Pointer to active keyboard configuration */
    ConfigObject<ConfigValueKbd> *kbdconfig;
    /** Pointer to track object */
    Track *m_pTrack;

    /** file_menu contains all items of the menubar entry "File" */
    QMenu *fileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    QMenu *editMenu;
    /** playlist menu */
    QMenu *playlistsMenu;
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
    QAction *fileOpen;
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

    QAction *optionsBeatMark;
#ifdef __VINYLCONTROL__
    QAction *optionsVinylControl;
#endif
    QAction *optionsFullScreen;
    QAction *optionsPreferences;

    QAction *helpAboutApp;
#ifdef __SCRIPT__
    QAction *macroStudio;
#endif
    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences *prefDlg;

#ifdef __SCRIPT__
    ScriptEngine *scriptEng;
#endif

#ifdef __LADSPA__
    DlgLADSPA *ladspaDlg;
#endif

    // Fullscreen patch
    QPoint winpos;
};
#endif

