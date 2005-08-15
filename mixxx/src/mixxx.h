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
#include <vector>
#include <qstringlist.h>

#ifdef QT3_SUPPORT
#include <Q3Action>
#include <q3mainwindow.h>
#include <q3popupmenu.h>
#include <q3whatsthis.h>
#include <q3filedialog.h>
#else
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>
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

class WVisual;
class Track;
class TrackInfoObject;
class PlayerProxy;
class WaveSummary;
class QSplashScreen;

/**
  * This Class is the base class for Mixxx. It sets up the main
  * window and providing a menubar.
  * For the main view, an instance of class MixxxView is
  * created which creates your view.
  */
#ifdef QT3_SUPPORT
class MixxxApp : public Q3MainWindow
#else
class MixxxApp : public QMainWindow
#endif
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
  public slots:

    /** Opens a file in player 1 */
    void slotFileOpen();
    /** exits the application */
    void slotFileQuit();
    /** toggle audio beat marks */
    void slotOptionsBeatMark(bool toggle);
    /** toogle full screen mode */
    void slotOptionsFullScreen(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** shows an about dlg*/
    void slotHelpAbout();
    /** Change of file to play */
    //void slotChangePlay(int,int,int, const QPoint &);
  private:
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    MixxxView *view;
    /** Pointer to waveform summary generator */
    WaveSummary *m_pWaveSummary;

    QApplication *app;
    EngineObject *engine;
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineMaster *master;
    PlayerProxy *player;
    MidiObject *midi;
    ControlObject *control;
    ConfigObject<ConfigValue> *config;
    /** Pointer to active midi configuration */
    ConfigObject<ConfigValueMidi> *midiconfig;
    /** Pointer to active keyboard configuration */
    ConfigObject<ConfigValueKbd> *kbdconfig;
    /** Pointer to track object */
    Track *m_pTrack;
    
    #ifdef QT3_SUPPORT
    /** file_menu contains all items of the menubar entry "File" */
    Q3PopupMenu *fileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    Q3PopupMenu *editMenu;
    /** playlist menu */
    Q3PopupMenu *playlistsMenu;
    /** options_menu contains all items of the menubar entry "Options" */
    Q3PopupMenu *optionsMenu;
    /** view_menu contains all items of the menubar entry "View" */
    Q3PopupMenu *viewMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    Q3PopupMenu *helpMenu;
   #else
   /** file_menu contains all items of the menubar entry "File" */
   QPopupMenu *fileMenu;
   /** edit_menu contains all items of the menubar entry "Edit" */
   QPopupMenu *editMenu;
   /** playlist menu */
   QPopupMenu *playlistsMenu;
   /** options_menu contains all items of the menubar entry "Options" */
   QPopupMenu *optionsMenu;
   /** view_menu contains all items of the menubar entry "View" */
   QPopupMenu *viewMenu;
   /** view_menu contains all items of the menubar entry "Help" */
   QPopupMenu *helpMenu;
   #endif

    /** actions for the application initialized in initActions() and used to en/disable them
      * according to your needs during the program */

    #ifdef QT3_SUPPORT
    Q3Action *fileNew;
    Q3Action *fileOpen;
    Q3Action *fileSave;
    Q3Action *fileSaveAs;
    Q3Action *fileClose;
    Q3Action *filePrint;
    Q3Action *fileQuit;

    Q3Action *editCut;
    Q3Action *editCopy;
    Q3Action *editPaste;

    Q3Action *playlistsNew;
    Q3Action *playlistsImport;
    Q3Action **playlistsList;

    Q3Action *optionsBeatMark;
    Q3Action *optionsFullScreen;
    Q3Action *optionsPreferences;

    Q3Action *helpAboutApp;
    #else
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
    QAction *optionsFullScreen;
    QAction *optionsPreferences;

    QAction *helpAboutApp;
    #endif
    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences *prefDlg;
};
#endif

