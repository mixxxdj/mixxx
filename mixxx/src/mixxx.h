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
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qwhatsthis.h>
#include <qstring.h>
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
#include "mixxxdoc.h"
#include "enginebuffer.h"
#include "enginechannel.h"
#include "enginemaster.h"
#include "player.h"
#include "midiobject.h"
#include "configmapping.h"
#include "controlobject.h"
#include "dlgpreferences.h"


/**
  * This Class is the base class for your application. It sets up the main
  * window and providing a menubar, toolbar
  * and statusbar. For the main view, an instance of class MixxxView is
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
    /** this creates the toolbars. Change the toobar look and add new toolbars in this
     * function */
    void initToolBar();
    /** setup the statusbar */
    void initStatusBar();
    /** setup the document*/
    void initDoc();
    /** setup the mainview*/
    void initView();

    /** overloaded for Message box on last window exit */
    bool queryExit();

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
    /** toggle the toolbar */
    void slotViewToolBar(bool toggle);
    /** toggle the statusbar*/
    void slotViewStatusBar(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** Update QComboBox values when devices are changed */
    void slotOptionsPreferencesUpdateDeviceOptions();
    /** Set preferences from dialog */
    void slotOptionsSetPreferences();
    /** Cancel preferences from dialog */
    void slotOptionsClosePreferences();
    /** shows an about dlg*/
    void slotHelpAbout();

	/** Change of file to play */
	//void slotChangePlay(int,int,int, const QPoint &);
    void slotChangePlay_1();
    void slotChangePlay_2();
    void slotSelectPlay(QListViewItem *item, const QPoint &pos, int);
  protected:
    bool eventFilter(QObject *, QEvent *);
  private:
    void engineStart();
    void engineStop();
    void addFiles(const char *path);

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
    QApplication *app;
    EngineObject *engine;
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineMaster *master;
    Player *player;
    MidiObject *midi;
    ControlObject *control;
    ConfigMapping *configMap;
	std::vector<EngineObject *> engines;
    QString songpath;

    /** Popup menu used to select player when a track has been selected */
    QPopupMenu *playSelectMenu;
    QString selection;

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
    /** the main toolbar */
//    QToolBar *fileToolbar;
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

    QAction *optionsPreferences;
    /** Pointer to preference dialog */
    DlgPreferences *pDlg;

//    QAction *viewToolBar;
    QAction *viewStatusBar;

    QAction *helpAboutApp;
};
#endif 

