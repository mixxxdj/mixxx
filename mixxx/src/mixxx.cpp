/***************************************************************************
                          mixxx.cpp  -  description
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

#include <qaccel.h>
#include <qpushbutton.h>
#include <qtable.h>
#include <qlistview.h>
#include <qiconset.h>
#include <qptrlist.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <signal.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>
#include <qdir.h>

#include "wknob.h"
#include "wslider.h"
#include "wplayposslider.h"
#include "mixxx.h"
#include "filesave.xpm"
#include "fileopen.xpm"
#include "filenew.xpm"
#include "images/a.xpm"
#include "images/b.xpm"
#include "controlnull.h"
#include "midiobjectnull.h"

#ifdef __ALSA__
  #include "playeralsa.h"
#endif

#ifdef __ALSAMIDI__
  #include "midiobjectalsa.h"
#endif

#ifdef __PORTAUDIO__
  #include "playerportaudio.h"
#endif

#ifdef __PORTMIDI__
  #include "midiobjectportmidi.h"
#endif

#ifdef __COREMIDI__
  #include "midiobjectcoremidi.h"
#endif

#ifdef __OSSMIDI__
  #include "midiobjectoss.h"
#endif

#ifdef __WINMIDI__
  #include "midiobjectwin.h"
#endif

MixxxApp::MixxxApp(QApplication *a)
{
  qDebug("Starting up...");
  setCaption(tr("Mixxx " VERSION));

  app = a;

  // Ensure that a directory named ~/.mixxx exists
  QDir().mkdir(QDir::homeDirPath().append("/.mixxx"));

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initActions();
  initMenuBar();
  initToolBar();
  //initStatusBar();

  // Reset pointer to preference dialog and players
  pDlg = 0;
  player = 0;

  // Read the config file
  config = new ConfigObject<ConfigValue>(QDir::homeDirPath().append("/.mixxx/mixxx.cfg"));

  initDoc();
  initView();

  //qWarning("Init playlist");
  PlaylistKey = ConfigKey("[Playlist]","Directory");
  QDir d( config->getValueString(PlaylistKey ));
  if ((config->getValueString(PlaylistKey ).length()<1) | (!d.exists())) {
      QFileDialog* fd = new QFileDialog( this, "Choose directory with music files", TRUE );
      fd->setMode( QFileDialog::Directory );
      if ( fd->exec() == QDialog::Accepted ) {
          config->set(PlaylistKey, fd->selectedFile());
      }
  }
  addFiles(config->getValueString(PlaylistKey).latin1());

  // Construct popup menu used to select playback channel on track selection
  playSelectMenu = new QPopupMenu(this);
  playSelectMenu->insertItem(QIconSet(a_xpm), "Player A",this, SLOT(slotChangePlay_1()));
  playSelectMenu->insertItem(QIconSet(b_xpm), "Player B",this, SLOT(slotChangePlay_2()));

  // Connect play list table selection with "select channel" popup menu
  connect(view->playlist->ListPlaylist, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
          this,                         SLOT(slotSelectPlay(QListViewItem *, const QPoint &, int)));

  // Open midi
  //qDebug("Init midi...");
  midi = 0;
#ifdef __ALSA__
  midi = new MidiObjectALSA(midiconfig,app,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __PORTMIDI__
  midi = new MidiObjectPortMidi(midiconfig,app,this,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __COREMIDI__
  midi = new MidiObjectCoreMidi(midiconfig,app,this,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __OSSMIDI__
  midi = new MidiObjectOSS(midiconfig,app,this,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __WINMIDI__
  midi = new MidiObjectWin(midiconfig,app,this,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
    
  if (midi == 0)
      midi = new MidiObjectNull(midiconfig,app,this,config->getValueString(ConfigKey("[Midi]","Device")));

  // Store default midi device
  config->set(ConfigKey("[Midi]","Device"), ConfigValue(midi->getOpenDevice()->latin1()));

  // Get directory where MIDI configurations are stored. If no is given, set to CONFIG_PATH
  if (config->getValueString(ConfigKey("[Midi]","Configdir")).length() == 0)
      config->set(ConfigKey("[Midi]","Configdir"),ConfigValue(CONFIG_PATH));

  // If the directory does not end with a "/", add one
  if (!config->getValueString(ConfigKey("[Midi]","Configdir")).endsWith("/"))
      config->set(ConfigKey("[Midi]","Configdir"),ConfigValue(config->getValueString(ConfigKey("[Midi]","Configdir")).append("/")));

  // Get list of available midi configurations, and read the default configuration. If no default
  // is given, use the first configuration found in the config directory.
  QStringList *midiConfigList = midi->getConfigList(config->getValueString(ConfigKey("[Midi]","Configdir")));
  midiconfig = 0;
  for (QStringList::Iterator it = midiConfigList->begin(); it != midiConfigList->end(); ++it )
      if (*it == config->getValueString(ConfigKey("[Midi]","Configfile")))
          midiconfig = new ConfigObject<ConfigValueMidi>(config->getValueString(ConfigKey("[Midi]","Configdir")).append(config->getValueString(ConfigKey("[Midi]","Configfile"))));
  if (midiconfig == 0)
  {
      if (midiConfigList->empty())
      {
          midiconfig = new ConfigObject<ConfigValueMidi>("");
          config->set(ConfigKey("[Midi]","Configfile"), ConfigValue(""));
      }
      else
      {
          midiconfig = new ConfigObject<ConfigValueMidi>(config->getValueString(ConfigKey("[Midi]","Configdir")).append((*midiConfigList->at(0)).latin1()));
          config->set(ConfigKey("[Midi]","Configfile"), ConfigValue((*midiConfigList->at(0)).latin1()));
      }
  }

  // Instantiate a ControlObject, and set the static midi and config pointer
  control = new ControlNull();
  control->midi = midi;
  control->config = midiconfig;

  //        if (visual>0)

  // Initialize master player
  //
  qDebug("Init master");

  // Initialize device
#ifdef __ALSA__
  player = new PlayerALSA(BUFFER_SIZE, &engines, config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")));
#else
  player = new PlayerPortAudio(config);
#endif

  // Open device using config data, if that fails, use default values. If that fails too, the
  // preference panel should be opened.
  if (!player->open(false))
      if (!player->open(true))
          pDlg = new DlgPreferences(this,"",midi,player,0,config,midiconfig);

  // Install event handler to update playpos slider and force screen update.
  // This method is used to avoid emitting signals (and QApplication::lock())
  // in the player thread. This ensures that the player will not lock because
  // of a (temporary) stalled GUI thread.
  installEventFilter(this);

  // Save main configuration file .... well, maybe only in the pref panel!!
  //config->Save();

  // Start engine
  engineStart();
}

MixxxApp::~MixxxApp()
{
    engineStop();
    delete player;
    delete control;
    delete midi;
    delete playSelectMenu;
    delete config;
    delete midiconfig;
}

bool MixxxApp::eventFilter(QObject *o, QEvent *e)
{
    // If a user event is received, update playpos sliders,
    // and force screen update
    if (e->type() == QEvent::User)
    {
        // Gain app lock
        //app->lock();

        view->playcontrol1->SliderPosition->setValue((int)buffer1->playposSliderNew);
        view->playcontrol2->SliderPosition->setValue((int)buffer2->playposSliderNew);

        // Force GUI update
        //app->flush();

        // Release app lock
        //app->unlock();

    }
    else
    {
        // Standard event processing
        return QWidget::eventFilter(o,e);
    }
    return TRUE;
}

void MixxxApp::engineStart()
{
    if (view->playlist->ListPlaylist->firstChild() != 0)
    {
		//qDebug("Init buffer 1... %s", view->playlist->ListPlaylist->firstChild()->text(1).ascii());
        buffer1 = new EngineBuffer(app, this, view->playcontrol1, "[Channel1]",
                                   view->playlist->ListPlaylist->firstChild()->text(1));

        if (view->playlist->ListPlaylist->firstChild()->nextSibling() != 0)
        {
            //qDebug("Init buffer 2... %s", view->playlist->ListPlaylist->firstChild()->nextSibling()->text(1).ascii());
            buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]",
                                       view->playlist->ListPlaylist->firstChild()->nextSibling()->text(1));
        } else
            buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]", 0);
    } else {
        buffer1 = new EngineBuffer(app, this, view->playcontrol1, "[Channel1]", 0);
        buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]", 0);
    }

    // Starting channels:
    channel1 = new EngineChannel(view->channel1, "[Channel1]");
    channel2 = new EngineChannel(view->channel2, "[Channel2]");

    // Starting effects:
    flanger = new EngineFlanger(view->flanger, "[Flanger]");

    //qDebug("Init master...");
    master = new EngineMaster(view->master, view->crossfader, view->channel1, view->channel2, 
			      buffer1, buffer2, channel1, channel2,flanger, "[Master]");

    /** Connect signals from option menu, selecting processing of channel 1 & 2, to
        EngineMaster */
    connect(optionsLeft, SIGNAL(toggled(bool)), master, SLOT(slotChannelMaster1(bool)));
    connect(optionsRight, SIGNAL(toggled(bool)), master, SLOT(slotChannelMaster2(bool)));
    master->slotChannelMaster1(optionsLeft->isOn());
    master->slotChannelMaster2(optionsRight->isOn());

    //qDebug("Starting buffers...");
    buffer1->start();
    buffer2->start();

    // Start audio
    //qDebug("Starting player...");
    player->setReader(master);
    player->start();
}

void MixxxApp::engineStop()
{
    player->stop();
    delete buffer1;
    delete buffer2;
    delete channel1;
    delete channel2;
    delete master;
}

/** initializes all QActions of the application */
void MixxxApp::initActions()
{
  QPixmap openIcon, saveIcon, newIcon;
  newIcon = QPixmap(filenew);
  openIcon = QPixmap(fileopen);
  saveIcon = QPixmap(filesave);

/*
  fileNew = new QAction(tr("New File"), newIcon, tr("&New"), QAccel::stringToKey(tr("Ctrl+N")), this);
  fileNew->setStatusTip(tr("Creates a new document"));
  fileNew->setWhatsThis(tr("New File\n\nCreates a new document"));
  connect(fileNew, SIGNAL(activated()), this, SLOT(slotFileNew()));

  fileOpen = new QAction(tr("Open File"), openIcon, tr("&Open..."), 0, this);
  fileOpen->setStatusTip(tr("Opens an existing document"));
  fileOpen->setWhatsThis(tr("Open File\n\nOpens an existing document"));
  connect(fileOpen, SIGNAL(activated()), this, SLOT(slotFileOpen()));

  fileSave = new QAction(tr("Save File"), saveIcon, tr("&Save"), QAccel::stringToKey(tr("Ctrl+S")), this);
  fileSave->setStatusTip(tr("Saves the actual document"));
  fileSave->setWhatsThis(tr("Save File.\n\nSaves the actual document"));
  connect(fileSave, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileSaveAs = new QAction(tr("Save File As"), tr("Save &as..."), 0, this);
  fileSaveAs->setStatusTip(tr("Saves the actual document under a new filename"));
  fileSaveAs->setWhatsThis(tr("Save As\n\nSaves the actual document under a new filename"));
  connect(fileSaveAs, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileClose = new QAction(tr("Close File"), tr("&Close"), QAccel::stringToKey(tr("Ctrl+W")), this);
  fileClose->setStatusTip(tr("Closes the actual document"));
  fileClose->setWhatsThis(tr("Close File\n\nCloses the actual document"));
  connect(fileClose, SIGNAL(activated()), this, SLOT(slotFileClose()));

  filePrint = new QAction(tr("Print File"), tr("&Print"), QAccel::stringToKey(tr("Ctrl+P")), this);
  filePrint->setStatusTip(tr("Prints out the actual document"));
  filePrint->setWhatsThis(tr("Print File\n\nPrints out the actual document"));
  connect(filePrint, SIGNAL(activated()), this, SLOT(slotFilePrint()));
*/
  fileQuit = new QAction(tr("Exit"), tr("E&xit"), QAccel::stringToKey(tr("Ctrl+Q")), this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
  connect(fileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));

  editCut = new QAction(tr("Cut"), tr("Cu&t"), QAccel::stringToKey(tr("Ctrl+X")), this);
  editCut->setStatusTip(tr("Cuts the selected section and puts it to the clipboard"));
  editCut->setWhatsThis(tr("Cut\n\nCuts the selected section and puts it to the clipboard"));
  connect(editCut, SIGNAL(activated()), this, SLOT(slotEditCut()));

  editCopy = new QAction(tr("Copy"), tr("&Copy"), QAccel::stringToKey(tr("Ctrl+C")), this);
  editCopy->setStatusTip(tr("Copies the selected section to the clipboard"));
  editCopy->setWhatsThis(tr("Copy\n\nCopies the selected section to the clipboard"));
  connect(editCopy, SIGNAL(activated()), this, SLOT(slotEditCopy()));

  editPaste = new QAction(tr("Paste"), tr("&Paste"), QAccel::stringToKey(tr("Ctrl+V")), this);
  editPaste->setStatusTip(tr("Pastes the clipboard contents to actual position"));
  editPaste->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to actual position"));
  connect(editPaste, SIGNAL(activated()), this, SLOT(slotEditPaste()));

  optionsLeft = new QAction(tr("Left channel"), tr("&Left channel"), QAccel::stringToKey(tr("Ctrl+L")), this, 0, true);
  optionsLeft->setOn(true);
  optionsRight = new QAction(tr("Right channel"), tr("&Right channel"), QAccel::stringToKey(tr("Ctrl+R")), this, 0, true);
  optionsRight->setOn(true);

  optionsPreferences = new QAction(tr("Preferences"), tr("&Preferences..."), 0, this);
  optionsPreferences->setStatusTip(tr("Preferences"));
  optionsPreferences->setWhatsThis(tr("Preferences\nPlayback and MIDI preferences"));
  connect(optionsPreferences, SIGNAL(activated()), this, SLOT(slotOptionsPreferences()));

/*
  viewToolBar = new QAction(tr("Toolbar"), tr("Tool&bar"), 0, this, 0, true);
  viewToolBar->setStatusTip(tr("Enables/disables the toolbar"));
  viewToolBar->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
  connect(viewToolBar, SIGNAL(toggled(bool)), this, SLOT(slotViewToolBar(bool)));
*/

/*
  viewStatusBar = new QAction(tr("Statusbar"), tr("&Statusbar"), 0, this, 0, true);
  viewStatusBar->setStatusTip(tr("Enables/disables the statusbar"));
  viewStatusBar->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
  connect(viewStatusBar, SIGNAL(toggled(bool)), this, SLOT(slotViewStatusBar(bool)));
*/
  helpAboutApp = new QAction(tr("About"), tr("&About..."), 0, this);
  helpAboutApp->setStatusTip(tr("About the application"));
  helpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
  connect(helpAboutApp, SIGNAL(activated()), this, SLOT(slotHelpAbout()));

}

void MixxxApp::initMenuBar()
{
  ///////////////////////////////////////////////////////////////////
  // MENUBAR

  ///////////////////////////////////////////////////////////////////
  // menuBar entry fileMenu
  fileMenu=new QPopupMenu();
/*
  fileNew->addTo(fileMenu);
  fileOpen->addTo(fileMenu);
  fileClose->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileSave->addTo(fileMenu);
  fileSaveAs->addTo(fileMenu);
  fileMenu->insertSeparator();
  filePrint->addTo(fileMenu);
  fileMenu->insertSeparator();
*/
  fileQuit->addTo(fileMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry editMenu
/*editMenu=new QPopupMenu();
  editCut->addTo(editMenu);
  editCopy->addTo(editMenu);
  editPaste->addTo(editMenu);*/

  ///////////////////////////////////////////////////////////////////
  // menuBar entry optionsMenu
  optionsMenu=new QPopupMenu();
  optionsMenu->setCheckable(true);
  optionsLeft->addTo(optionsMenu);
  optionsRight->addTo(optionsMenu);
  optionsMenu->insertSeparator();
  optionsPreferences->addTo(optionsMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry viewMenu
  viewMenu=new QPopupMenu();
  viewMenu->setCheckable(true);
  //viewToolBar->addTo(viewMenu);
  //viewStatusBar->addTo(viewMenu);
  ///////////////////////////////////////////////////////////////////
  // EDIT YOUR APPLICATION SPECIFIC MENUENTRIES HERE

  ///////////////////////////////////////////////////////////////////
  // menuBar entry helpMenu
  helpMenu=new QPopupMenu();
  helpAboutApp->addTo(helpMenu);

  ///////////////////////////////////////////////////////////////////
  // MENUBAR CONFIGURATION
  menuBar()->insertItem(tr("&File"), fileMenu);
  //menuBar()->insertItem(tr("&Edit"), editMenu);
  menuBar()->insertItem(tr("&Options"), optionsMenu);
  //menuBar()->insertItem(tr("&View"), viewMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem(tr("&Help"), helpMenu);

}

void MixxxApp::initToolBar()
{
  ///////////////////////////////////////////////////////////////////
  // TOOLBAR
/*
  fileToolbar = new QToolBar(this, "file operations");
  fileNew->addTo(fileToolbar);
  fileOpen->addTo(fileToolbar);
  fileSave->addTo(fileToolbar);
  fileToolbar->addSeparator();
  QWhatsThis::whatsThisButton(fileToolbar);
*/
}

void MixxxApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  //STATUSBAR
  statusBar()->message(tr("Ready."), 2000);
}

void MixxxApp::initDoc()
{
   doc=new MixxxDoc();
}

void MixxxApp::initView()
{
  ////////////////////////////////////////////////////////////////////
  // set the main widget here
  view=new MixxxView(this, doc);
  setCentralWidget(view);

  // Set visual vidget here
  visual = 0;
}

bool MixxxApp::queryExit()
{
  int exit=QMessageBox::information(this, tr("Quit..."),
                                    tr("Do your really want to quit?"),
                                    QMessageBox::Ok, QMessageBox::Cancel);

  if (exit==1)
  {

  }
  else
  {

  };

  return (exit==1);
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void MixxxApp::slotFileNew()
{
    statusBar()->message(tr("Creating new file..."));
    doc->newDoc();
    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileOpen()
{
    statusBar()->message(tr("Opening file..."));

    QString fileName = QFileDialog::getOpenFileName(0,0,this);
    if (!fileName.isEmpty())
    {
        doc->load(fileName);
        setCaption(fileName);
        QString message=tr("Loaded document: ")+fileName;
        statusBar()->message(message, 2000);
    }
    else
    {
        statusBar()->message(tr("Opening aborted"), 2000);
    }
}


void MixxxApp::slotFileSave()
{
    statusBar()->message(tr("Saving file..."));
    doc->save();
    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileSaveAs()
{
    statusBar()->message(tr("Saving file under new filename..."));
    QString fn = QFileDialog::getSaveFileName(0, 0, this);
    if (!fn.isEmpty())
    {
        doc->saveAs(fn);
    }
    else
    {
        statusBar()->message(tr("Saving aborted"), 2000);
    }

    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileClose()
{
    statusBar()->message(tr("Closing file..."));

    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFilePrint()
{
    statusBar()->message(tr("Printing..."));
    QPrinter printer;
    if (printer.setup(this))
    {
        QPainter painter;
        painter.begin(&printer);

        ///////////////////////////////////////////////////////////////////
        // TODO: Define printing by using the QPainter methods here

        painter.end();
    };

    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileQuit()
{
    statusBar()->message(tr("Exiting application..."));

    ///////////////////////////////////////////////////////////////////
    // exits the Application
    if(doc->isModified())
    {
        if(queryExit())
        {
            qApp->quit();
        }
        else
        {
        };
    }
    else
    {
        qApp->quit();
    };

    statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditCut()
{
  statusBar()->message(tr("Cutting selection..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditCopy()
{
  statusBar()->message(tr("Copying selection to clipboard..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditPaste()
{
  statusBar()->message(tr("Inserting clipboard contents..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotViewToolBar(bool toggle)
{
  statusBar()->message(tr("Toggle toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off

  if (toggle== false)
  {
//    fileToolbar->hide();
  }
  else
  {
//    fileToolbar->show();
  };

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotViewStatusBar(bool toggle)
{
  statusBar()->message(tr("Toggle statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off

  if (toggle == false)
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotOptionsPreferences()
{
    if (pDlg==0)
        pDlg = new DlgPreferences(this,"",midi,player,0,config,midiconfig);
}


void MixxxApp::slotBrowsePlaylistDir()
{
    QFileDialog* fd = new QFileDialog( this, QString::null, TRUE );
    fd->setMode( QFileDialog::Directory );
    fd->setCaption("Choose directory with music files");
    if ( fd->exec() == QDialog::Accepted )
    {
        pDlg->LineEditSongfiles->setText( fd->selectedFile() );
    }
}

void MixxxApp::slotOptionsClosePreferences()
{
    delete pDlg;
    pDlg = 0;
}

void MixxxApp::slotHelpAbout()
{
  QMessageBox::about(this,tr("About..."),
                      tr("Mixxx\nVersion " VERSION "\nBy Tue and Ken Haste Andersen\nReleased under the GNU General Public Licence version 2") );
}

void MixxxApp::slotChangePlay_1()
{
    buffer1->newtrack(selection.ascii());
}

void MixxxApp::slotChangePlay_2()
{
    buffer2->newtrack(selection.ascii());
}

void MixxxApp::slotSelectPlay(QListViewItem *item, const QPoint &pos, int)
{
    if (item!=0)
    {
        // Store the current selected track
        selection = item->text(1);

        // Display popup menu
        playSelectMenu->popup(pos);
    }
}

void MixxxApp::addFiles(const char *path)
{
    QDir dir(path);
    if (!dir.exists())
        qWarning( "Cannot find the directory %s.",path);
    else
    {
        // First run though all directories:
        dir.setFilter(QDir::Dirs);
        const QFileInfoList dir_list = *dir.entryInfoList();
        QFileInfoListIterator dir_it(dir_list);
        QFileInfo *d;
        dir_it += 2; // Traverse past "." and ".."
        while ((d=dir_it.current()))
        {
            addFiles(d->filePath());
            ++dir_it;
        }

        // ... and then all the files:
        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing

        while ((fi=it.current()))
        {
            view->playlist->ListPlaylist->insertItem(new QListViewItem(view->playlist->ListPlaylist,
            fi->fileName(),fi->filePath()));
            ++it;   // goto next list element
        }
    }
}

void MixxxApp::updatePlayList()
{
    view->playlist->ListPlaylist->clear();
    addFiles(config->getValueString(ConfigKey("[Playlist]","Directory")).latin1());
}

void MixxxApp::reopen()
{
    // Close devices, and open using config data.
    player->close();
    if (!player->open(false))
        QMessageBox::warning(0, "Configuration error","Problem opening audio device");
    else
        player->start();
            
    // Close MIDI
    midi->devClose();

    // Change MIDI configuration
    //midiconfig->clear(); // (is currently not implemented correctly)
    midiconfig->reopen(config->getValueString(ConfigKey("[Midi]","Configdir")).append(config->getValueString(ConfigKey("[Midi]","Configfile"))));

    // Open MIDI device
    midi->devOpen(config->getValueString(ConfigKey("[Midi]","Device")));
}
