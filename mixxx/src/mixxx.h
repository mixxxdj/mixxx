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
#include <qapp.h>
#include <qmainwindow.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qmsgbox.h>
#include <qfiledialog.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qapplication.h>
#include <vector>

// application specific includes
#include "defs.h"
#include "mixxxview.h"
#include "enginebuffer.h"
#include "enginechannel.h"
#include "enginemaster.h"
#include "engineflanger.h"
#include "player.h"
#include "midiobject.h"
#include "controlobject.h"
#include "dlgpreferences.h"

class WVisual;
class TrackList;
class TrackInfoObject;
class PowerMate;
class Joystick;
class ControlEngine;

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
    /** Construtor */
    MixxxApp(QApplication *app);
    /** destructor */
    ~MixxxApp();
    /** initializes all QActions of the application */
    void initActions();
    /** initMenuBar creates the menu_bar and inserts the menuitems */
    void initMenuBar();
    /** overloaded for Message box on last window exit */
    bool queryExit();
  public slots:

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
  protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
  private:
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    MixxxView *view;

    QApplication *app;
    EngineObject *engine;
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineMaster *master;
    EngineFlanger *flanger;
    Player *player;
    MidiObject *midi;
    ControlObject *control;
    ControlEngine *m_pControlEngine;
    std::vector<EngineObject *> engines;
    ConfigObject<ConfigValue> *config;
    /** Pointer to active midi configuration */
    ConfigObject<ConfigValueMidi> *midiconfig;
    /** Pointer to active keyboard configuration */
    ConfigObject<ConfigValueKbd> *kbdconfig;
    /** Pointer to track list object */
    TrackList *m_pTracks;
    /** Pointer to PowerMate objects */
    PowerMate *powermate1, *powermate2;
    /** Pointer to Joystick object */
    Joystick *joystick1;
    /** file_menu contains all items of the menubar entry "File" */
    QPopupMenu *fileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    QPopupMenu *editMenu;
    /** options_menu contains all items of the menubar entry "Options" */
    QPopupMenu *optionsMenu;
    /** view_menu contains all items of the menubar entry "View" */
    QPopupMenu *viewMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    QPopupMenu *helpMenu;
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

    QAction *optionsBeatMark;
    QAction *optionsFullScreen;
    QAction *optionsPreferences;
    /** Pointer to preference dialog */
    DlgPreferences *prefDlg;

    QAction *helpAboutApp;
};
#endif 

