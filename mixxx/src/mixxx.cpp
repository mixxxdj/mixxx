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

#ifdef __ALSA__
  #include "playeralsa.h"
#endif

#ifdef __ALSA__
  #include "midiobjectalsa.h"
#endif

#ifdef __PORTAUDIO__
  #include "playerportaudio.h"
#endif

#ifdef __PORTMIDI__
  #include "midiobjectportmidi.h"
#endif

#ifdef __OSSMIDI__
  #include "midiobjectoss.h"
#endif

MixxxApp::MixxxApp(QApplication *a)
{
  qDebug("Starting up...");
  setCaption(tr("Mixxx " VERSION));

  app = a;

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initActions();
  initMenuBar();
  initToolBar();
  //initStatusBar();

  // Reset pointer to preference dialog
  pDlg = 0;

  // Read the config file
  config = new ConfigObject<ConfigValue>("mixxx.cfg"); 

  // Get list of available midi configurations, and read the default configuration. If no default
  // is given, use the first configuration found in the config directory.
  QDir dir("config");
  dir.setFilter(QDir::Files);
  dir.setNameFilter("*.midi.cfg *.MIDI.CFG");
  const QFileInfoList *list = dir.entryInfoList();
  QFileInfoListIterator it(*list);        // create list iterator
  QFileInfo *fi;                          // pointer for traversing
  midiconfig = 0;
  while ((fi=it.current()))
  {
      midiConfigList.append(fi->fileName());
      if (fi->fileName() == config->getValueString(ConfigKey("[Midi]","Configfile")))
          midiconfig = new ConfigObject<ConfigValueMidi>(config->getValueString(ConfigKey("[Midi]","Configfile")));
      ++it;   // goto next list element
  }
  if (midiconfig == 0)
  {
      if (midiConfigList.empty())
      {
          midiconfig = new ConfigObject<ConfigValueMidi>("");
          config->set(ConfigKey("[Midi]","Configfile"), ConfigValue(""));
      }
      else
      {
          midiconfig = new ConfigObject<ConfigValueMidi>((*midiConfigList.at(0)).latin1());
          config->set(ConfigKey("[Midi]","Configfile"), ConfigValue((*midiConfigList.at(0)).latin1()));
      }
  }

  initDoc();

  initView();

  qDebug("Init playlist");
  PlaylistKey = ConfigKey("[Playlist]","Directory");
  QDir d( config->getValueString(PlaylistKey ));
  if (config->getValueString(PlaylistKey ).length()<1 | !d.exists()) {
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
  qDebug("Init midi...");
#ifdef __ALSA__
  midi = new MidiObjectALSA(midiconfig,app,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __PORTMIDI__
  midi = new MidiObjectPortMidi(midiconfig,app,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __OSSMIDI__
  midi = new MidiObjectOSS(midiconfig,app,config->getValueString(ConfigKey("[Midi]","Device")));
#endif

  // Store default midi device
  config->set(ConfigKey("[Midi]","Device"), ConfigValue(midi->getOpenDevice()->latin1()));

  // Instantiate a ControlObject, and set the static midi and config pointer
  control = new ControlNull();
  control->midi = midi;
  control->config = midiconfig;

  // Initialize player with a desired buffer size
  qDebug("Init player...");
#ifdef __ALSA__
  player = new PlayerALSA(BUFFER_SIZE, &engines, config->getValueString(ConfigKey("[Soundcard]","Device")));
#else
  player = new PlayerPortAudio(BUFFER_SIZE, &engines, config->getValueString(ConfigKey("[Soundcard]","Device")));
#endif

  // Ensure the correct configuration is chosen and stored in the config object
  player->reopen(config->getValueString(ConfigKey("[Soundcard]","Device")),
                 config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt(),
                 config->getValueString(ConfigKey("[Soundcard]","Bits")).toInt(),
                 BUFFER_SIZE);
  config->set(ConfigKey("[Soundcard]","Device"),ConfigValue(player->NAME));
  config->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue(player->SRATE));
  config->set(ConfigKey("[Soundcard]","Bits"),ConfigValue(player->BITS));

  // Install event handler to update playpos slider and force screen update.
  // This method is used to avoid emitting signals (and QApplication::lock())
  // in the player thread. This ensures that the player will not lock because
  // of a (temporary) stalled GUI thread.
  installEventFilter(this);

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
}

bool MixxxApp::eventFilter(QObject *o, QEvent *e)
{
    // If a user event is received, update playpos sliders,
    // and force screen update
    if (e->type() == QEvent::User)
    {
        view->playcontrol1->SliderPosition->setValue(buffer1->playposSliderNew);
        view->playcontrol2->SliderPosition->setValue(buffer2->playposSliderNew);

        // Force update
        app->flush();

        return TRUE;
    } else {
        // standard event processing
        return QWidget::eventFilter(o,e);
    }
}

void MixxxApp::engineStart()
{
    if (view->playlist->ListPlaylist->firstChild() != 0)
    {
        //qDebug("Init buffer 1... %s", view->playlist->ListPlaylist->firstChild()->text(1).ascii());
        buffer1 = new EngineBuffer(app, this, view->playcontrol1, "[Channel1]", view->playlist->ListPlaylist->firstChild()->text(1));

        if (view->playlist->ListPlaylist->firstChild()->nextSibling() != 0)
        {
            //qDebug("Init buffer 2... %s", view->playlist->ListPlaylist->firstChild()->nextSibling()->text(1).ascii());
            buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]", view->playlist->ListPlaylist->firstChild()->nextSibling()->text(1));
        } else
            buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]", 0);
    } else {
        buffer1 = new EngineBuffer(app, this, view->playcontrol1, "[Channel1]", 0);
        buffer2 = new EngineBuffer(app, this, view->playcontrol2, "[Channel2]", 0);
    }

    channel1 = new EngineChannel(view->channel1, "[Channel1]");
    channel2 = new EngineChannel(view->channel2, "[Channel2]");

    //qDebug("Init master...");
    master = new EngineMaster(view->master, view->crossfader, buffer1, buffer2, channel1, channel2, "[Master]");

    /** Connect signals from option menu, selecting processing of left and right channel, to
        EngineMaster */
    connect(optionsLeft, SIGNAL(toggled(bool)), master, SLOT(slotChannelLeft(bool)));
    connect(optionsRight, SIGNAL(toggled(bool)), master, SLOT(slotChannelRight(bool)));
    master->slotChannelLeft(optionsLeft->isOn());
    master->slotChannelRight(optionsRight->isOn());

    qDebug("Starting buffers...");
    buffer1->start();
    buffer2->start();

    // Start audio
    qDebug("Starting player...");
    player->start(master);
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
void MixxxApp::initActions(){

  QPixmap openIcon, saveIcon, newIcon;
  newIcon = QPixmap(filenew);
  openIcon = QPixmap(fileopen);
  saveIcon = QPixmap(filesave);


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
/*fileMenu=new QPopupMenu();
  fileNew->addTo(fileMenu);
  fileOpen->addTo(fileMenu);
  fileClose->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileSave->addTo(fileMenu);
  fileSaveAs->addTo(fileMenu);
  fileMenu->insertSeparator();
  filePrint->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileQuit->addTo(fileMenu);*/

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
  //menuBar()->insertItem(tr("&File"), fileMenu);
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
    {
        pDlg = new DlgPreferences(this);
        QPtrList<Player::Info> *pInfo = player->getInfo();

        // Fill dialog with info

        // Sound card info
        for (unsigned int j=0; j<pInfo->count(); j++)
        {
            Player::Info *p = pInfo->at(j);

            // Name of device.
            pDlg->ComboBoxSoundcard->insertItem(p->name);

            // If it's the first device, it becomes the default, if no device has been
            // selected previously. Thus update its properties
            slotOptionsPreferencesUpdateDeviceOptions();

            if (p->name == config->getValueString(ConfigKey("[Soundcard]","Device")))
            {
                pDlg->ComboBoxSoundcard->setCurrentItem(j);
                slotOptionsPreferencesUpdateDeviceOptions();
            }
        }

        // Midi configuration
        int j=0;
        for (QStringList::Iterator it = midiConfigList.begin(); it != midiConfigList.end(); ++it )
        {
            // Insert the file name into the list, with ending (.midi.cfg) stripped
            pDlg->ComboBoxMidiconf->insertItem((*it).left((*it).length()-9));

            if ((*it) == config->getValueString(ConfigKey("[Midi]","Configfile")))
                pDlg->ComboBoxMididevice->setCurrentItem(j);
            j++;
        }

        // Midi device
        QStringList *mididev = midi->getDeviceList();
        j=0;
        for (QStringList::Iterator it = mididev->begin(); it != mididev->end(); ++it )
        {
            pDlg->ComboBoxMididevice->insertItem(*it);
            if ((*it) == (*midi->getOpenDevice()))
                pDlg->ComboBoxMididevice->setCurrentItem(j);
            j++;
        }

        // Song path
        pDlg->LineEditSongfiles->setText(config->getValueString(PlaylistKey));

        // Connect buttons
        connect(pDlg->PushButtonOK,      SIGNAL(clicked()),      this, SLOT(slotOptionsSetPreferences()));
        connect(pDlg->PushButtonApply,   SIGNAL(clicked()),      this, SLOT(slotOptionsApplyPreferences()));
        connect(pDlg->PushButtonCancel,  SIGNAL(clicked()),      this, SLOT(slotOptionsClosePreferences()));
        connect(pDlg->ComboBoxSoundcard, SIGNAL(activated(int)), this, SLOT(slotOptionsPreferencesUpdateDeviceOptions()));
        connect(pDlg->PushButtonBrowsePlaylist, SIGNAL(clicked()),this,SLOT(slotBrowsePlaylistDir()));

        // Show dialog
        pDlg->show();
    }
}

void MixxxApp::slotOptionsPreferencesUpdateDeviceOptions()
{
    QPtrList<Player::Info> *pInfo = player->getInfo();
    Player::Info *p = pInfo->first();

    while (p != 0)
    {
        if (pDlg->ComboBoxSoundcard->currentText() == p->name)
        {
            // Sample rates
            pDlg->ComboBoxSamplerates->clear();
            for (unsigned int i=0; i<p->sampleRates.size(); i++)
            {
                pDlg->ComboBoxSamplerates->insertItem(QString("%1 Hz").arg(p->sampleRates[i]));
                if (p->sampleRates[i]==player->SRATE)
                    pDlg->ComboBoxSamplerates->setCurrentItem(i);
            }

            // Bits
            pDlg->ComboBoxBits->clear();
            for (unsigned int i=0; i<p->bits.size(); i++)
            {
                pDlg->ComboBoxBits->insertItem(QString("%1").arg(p->bits[i]));
                if (p->bits[i]==player->BITS)
                    pDlg->ComboBoxBits->setCurrentItem(i);
            }
        }

        // Get next device
        p = pInfo->next();
    }
}

void MixxxApp::slotOptionsDefaultPreferences()
{
    //config->set(ConfigKey("[Soundcard]","Device"), p->name);
    config->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue("44100"));
    config->set(ConfigKey("[Soundcard]","Bits"), ConfigValue("16"));

    config->set(ConfigKey("[Midi]","Configfile"), ConfigValue("midicontrol.cfg"));
    //config->set(ConfigKey("[Midi]","Device"), midi->getDeviceList().begin());
 
    config->set(ConfigKey("[Playlist]","Directory"), ConfigValue("music"));    
}

void MixxxApp::slotOptionsApplyPreferences()
{
    // Get parameters from dialog
    config->set(ConfigKey("[Soundcard]","Device"), pDlg->ComboBoxSoundcard->currentText());

    QString temp = pDlg->ComboBoxSamplerates->currentText();
    temp.truncate(temp.length()-3);
    config->set(ConfigKey("[Soundcard]","Samplerate"), temp);

    config->set(ConfigKey("[Soundcard]","Bits"),pDlg->ComboBoxBits->currentText());

    int bufferSize = BUFFER_SIZE;

    // Perform changes to sound card setup
    player->stop();
    player->reopen(config->getValueString(ConfigKey("[Soundcard]","Device")),
           config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt(),
           config->getValueString(ConfigKey("[Soundcard]","Bits")).toInt(),
           bufferSize);
    player->start(master);

    // Close MIDI
    midi->devClose();

    // Change MIDI configuration
    config->set(ConfigKey("[Midi]","Configfile"),pDlg->ComboBoxMidiconf->currentText().append(".midi.cfg"));
    //midiconfig->clear(); // (is currently not implemented correctly)
    midiconfig->reopen(config->getValueString(ConfigKey("[Midi]","Configfile")));

    // Open MIDI device
    config->set(ConfigKey("[Midi]","Device"), pDlg->ComboBoxMididevice->currentText());
    midi->devOpen(pDlg->ComboBoxMididevice->currentText());

    // Update playlist if path has changed
    if (pDlg->LineEditSongfiles->text() != config->getValueString(PlaylistKey))
    {
        config->set(ConfigKey("[Playlist]","Directory"), pDlg->LineEditSongfiles->text());
        view->playlist->ListPlaylist->clear();
        addFiles(config->getValueString(PlaylistKey).latin1());
    }
}


void MixxxApp::slotOptionsSetPreferences()
{
    slotOptionsApplyPreferences();

    // Save the preferences
    config->Save();

    // Close dialog
    slotOptionsClosePreferences();
}

void MixxxApp::slotBrowsePlaylistDir()
{
    QFileDialog* fd = new QFileDialog( this, "Choose directory with music files", TRUE );
    fd->setMode( QFileDialog::Directory );
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
    // Store the current selected track
    selection = item->text(1);

    // Display popup menu
    playSelectMenu->popup(pos);
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

