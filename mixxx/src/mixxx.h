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
#include "mixxxdoc.h"
#include "mixxxview.h"
#include "enginebuffer.h"
#include "enginechannel.h"
#include "enginemaster.h"
#include "engineflanger.h"
#include "player.h"
#include "midiobject.h"
#include "controlobject.h"
#include "dlgpreferences.h"

class MixxxVisual;
class TrackList;
class TrackInfoObject;
class PowerMate;
class EngineVUmeter;

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
    /** construtor */
    MixxxApp(QApplication *app);
    /** destructor */
    ~MixxxApp();
    /** initializes all QActions of the application */
    void initActions();
    /** initMenuBar creates the menu_bar and inserts the menuitems */
    void initMenuBar();
    /** setup the document*/
    void initDoc();
    /** setup the mainview*/
    void initView();
    /** overloaded for Message box on last window exit */
    bool queryExit();
    /** Get pointer to the MixxxVisual object */
    MixxxVisual *getVisual();
  public slots:

    /** generate a new document in the actual view */
    void slotFileNew();
    /** open a document */
    void slotFileOpen();
    /** save a document */
    void slotFileSave();
    /** save a document under a different filename*/
    void slotFileSaveAs();
    /** close the actual file */
    void slotFileClose();
    /** print the actual file */
    void slotFilePrint();
    /** exits the application */
    void slotFileQuit();
    /** put the marked text/object into the clipboard and remove
     * it from the document */
    void slotEditCut();
    /** put the marked text/object into the clipboard*/
    void slotEditCopy();
    /** paste the clipboard into the document*/
    void slotEditPaste();
    /** toggle audio beat marks */
    void slotOptionsBeatMark(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** shows an about dlg*/
    void slotHelpAbout();
    /** Change of file to play */
    //void slotChangePlay(int,int,int, const QPoint &);
  protected:
    bool eventFilter(QObject *, QEvent *);
  private:
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    MixxxView *view;
    /** doc represents your actual document and is created only once. It keeps
     * information such as filename and does the serialization of your files.
     */
    MixxxDoc *doc;

    /** Pointer to MixxxVisual widget */
    MixxxVisual *visual;
    
    QApplication *app;
    EngineObject *engine;
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineMaster *master;
    EngineFlanger *flanger;
    EngineVUmeter *vumeter;
    Player *player;
    MidiObject *midi;
    ControlObject *control;
    std::vector<EngineObject *> engines;
    ConfigObject<ConfigValue> *config;
    ConfigObject<ConfigValueMidi> *midiconfig;
    /** Pointer to track list object */
    TrackList *m_pTracks;
    /** Pointer to PowerMate objects */
    PowerMate *powermate1, *powermate2;
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

    QAction *optionsLeft;
    QAction *optionsRight;
    QAction *optionsBeatMark;

    QAction *optionsPreferences;
    /** Pointer to preference dialog */
    DlgPreferences *prefDlg;

    QAction *helpAboutApp;
};
#endif 

