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
#include <qptrlist.h>

#include "wknob.h"
#include "wslider.h"
#include "wplayposslider.h"
#include "wpushbutton.h"
#include "wtracktable.h"
#include "mixxx.h"
#include "images/a.xpm"
#include "images/b.xpm"
#include "controlnull.h"
#include "midiobjectnull.h"
#include "readerextractwave.h"
#include "controlengine.h"
#include "controlpotmeter.h"
#include "reader.h"
#include "enginebuffer.h"
#include "tracklist.h"
#include "dlgtracklist.h"
#include "dlgflanger.h"
#include "dlgplaylist.h"
#include "dlgmaster.h"
#include "dlgchannel.h"
#include "dlgplaycontrol.h"
#include "dlgcrossfader.h"
#include "dlgsplit.h"
#include "powermate.h"
// #include "enginevumeter.h"

#ifdef __UNIX__
#include "powermatelinux.h"
#endif
#ifdef __WIN__
#include "powermatewin.h"
#endif

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

#ifdef __VISUALS__
  #include "mixxxvisual.h"
#endif

MixxxApp::MixxxApp(QApplication *a)
{
    qDebug("Starting up...");
    setCaption(tr("Mixxx " VERSION));

    app = a;

    // Ensure that a directory named ~/.mixxx exists
    QDir().mkdir(QDir::homeDirPath().append("/").append(SETTINGS_DIR));

    // Call inits to invoke all other construction parts
    initActions();
    initMenuBar();

    // Reset pointer to players
    player = 0;
    m_pTracks = 0;
    prefDlg = 0;

    // Read the config file
    config = new ConfigObject<ConfigValue>(QDir::homeDirPath().append("/").append(SETTINGS_DIR).append("/mixxx.cfg"));
    qDebug(QDir::homeDirPath().append("/").append(SETTINGS_DIR).append("/mixxx.cfg"));

    initDoc();
    initView();

    // Instantiate a ControlObject, and set static parent widget
    control = new ControlNull();
    control->setParentWidget(this);
                                                       
    // Open midi
    //qDebug("Init midi...");
    midi = 0;
#ifdef __ALSA__
    midi = new MidiObjectALSA(midiconfig,app,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __PORTMIDI__
    midi = new MidiObjectPortMidi(midiconfig,app,control,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __COREMIDI__
    midi = new MidiObjectCoreMidi(midiconfig,app,control,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __OSSMIDI__
    midi = new MidiObjectOSS(midiconfig,app,control,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
#ifdef __WINMIDI__
    midi = new MidiObjectWin(midiconfig,app,control,config->getValueString(ConfigKey("[Midi]","Device")));
#endif
    
    if (midi == 0)
        midi = new MidiObjectNull(midiconfig,app,control,config->getValueString(ConfigKey("[Midi]","Device")));

    // Store default midi device
    config->set(ConfigKey("[Midi]","Device"), ConfigValue(midi->getOpenDevice()->latin1()));

    // On unix, get directory where MIDI configurations are stored. If no is given, set to CONFIG_PATH
#ifdef __UNIX__
#ifndef __MACX__
    if (config->getValueString(ConfigKey("[Midi]","Configdir")).length() == 0)
        config->set(ConfigKey("[Midi]","Configdir"),ConfigValue(CONFIG_PATH));
#endif
#endif

  // On Mac and Windows, always set the config dir relative to the application dir
#ifdef __WIN__
    config->set(ConfigKey("[Midi]","Configdir"),ConfigValue(QDir::currentDirPath().append(QString("/").append(CONFIG_PATH))));
#endif
#ifdef __MACX__
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    qDebug("Path = %s",pathPtr);
  
    config->set(ConfigKey("[Midi]","Configdir"),ConfigValue(QString(pathPtr).append(QString("/").append(CONFIG_PATH))));
#endif

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

    // Set the static config pointer for the ControlObject
    control->setConfig(midiconfig);

    // Configure ControlEngine object
    ControlEngine *controlengine = new ControlEngine(control);

    // Install event handler
    //installEventFilter(this);

    // Ensure that only one of the flanger buttons are pushed at a time.
    //connect(dlg_flanger->PushButtonChA, SIGNAL(valueOn()), flanger_b, SLOT(slotSetPositionOff()));
    //connect(dlg_flanger->PushButtonChB, SIGNAL(valueOn()), flanger_a, SLOT(slotSetPositionOff()));

    // Prepare the tracklist:
    QDir d(config->getValueString(ConfigKey("[Playlist]","Directory")));
    if ((config->getValueString(ConfigKey("[Playlist]","Directory")).length()<1) | (!d.exists()))
    {
        QFileDialog* fd = new QFileDialog(this, QString::null, true);
        fd->setCaption(QString("Choose directory with music files"));
        fd->setMode( QFileDialog::Directory );
        if ( fd->exec() == QDialog::Accepted )
        {
            config->set(ConfigKey("[Playlist]","Directory"), fd->selectedFile());
            config->Save();
        }
    }

    // Try initializing PowerMates
    powermate1 = 0;
    powermate2 = 0;
#ifdef __LINUX__
    powermate1 = new PowerMateLinux(control);
    powermate2 = new PowerMateLinux(control);
#endif
#ifdef __WIN__
    powermate1 = new PowerMateWin(control);
    powermate2 = new PowerMateWin(control);
#endif

    if (powermate1!=0)
    {
    if (powermate1->opendev())
        qDebug("Found PowerMate 1");
    else
    {
        delete powermate1;
        powermate1 = 0;
    }

    if (powermate2->opendev())
        qDebug("Found PowerMate 2");
    else
    {
        delete powermate2;
        powermate2 = 0;
    }
    }
 
    // Initialize player device
#ifdef __ALSA__
    player = new PlayerALSA(BUFFER_SIZE, &engines, config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")));
#else
    player = new PlayerPortAudio(config,control,app);
#endif

    // Init buffers/readers
    buffer1 = new EngineBuffer(this, optionsBeatMark, powermate1, view->playcontrol1, "[Channel1]");
    buffer2 = new EngineBuffer(this, optionsBeatMark, powermate2, view->playcontrol2, "[Channel2]");

    // Initialize tracklist:
    m_pTracks = new TrackList(config->getValueString(ConfigKey("[Playlist]","Directory")), view->tracklist->tableTracks,
                              view->playcontrol1, view->playcontrol2, buffer1, buffer2);

    // Initialize preference dialog
    prefDlg = new DlgPreferences(this,midi,player,m_pTracks, config, midiconfig);
    prefDlg->setHidden(true);

    // Try open player device using config data, if that fails, use default values. If that fails too, the
    // preference panel should be opened.
    if (!player->open(false))
        if (!player->open(true))
            prefDlg->setHidden(false);
    
    // Starting channels:
    channel1 = new EngineChannel(view->channel1, "[Channel1]");
    channel2 = new EngineChannel(view->channel2, "[Channel2]");

    // Starting effects:
    flanger = new EngineFlanger(view->flanger, "[Flanger]");

    // Starting vumeter:
    // vumeter = new EngineVUmeter(view->vumeter, "[VUmeter]");

    // Starting the master (mixing of the channels and effects):
    master = new EngineMaster(view->master, view->crossfader,
                              buffer1, buffer2, channel1, channel2,flanger,vumeter, "[Master]");

    // Start audio
    //qDebug("Starting player...");
    player->setMaster(master);
    player->start();
}

MixxxApp::~MixxxApp()
{
    qDebug("Destroying MixxxApp");
    player->stop();
    delete buffer1;
    delete buffer2;
    delete channel1;
    delete channel2;
    delete master;
    delete prefDlg;
    delete player;
    delete control;
    delete midi;
    delete config;
    delete midiconfig;
    delete m_pTracks;
    delete flanger;
    // delete vumeter;

#ifdef __UNIX__
    if (powermate1!=0)
        delete powermate1;
    if (powermate2!=0)
        delete powermate2;
#endif
}

bool MixxxApp::eventFilter(QObject *o, QEvent *e)
{
    // If a user event is received, update playpos sliders,
    // and force screen update
    if (e->type() == QEvent::User)
    {
        // Gain app lock
        //app->lock();

        //view->playcontrol1->SliderPosition->setValue((int)buffer1->playposSliderNew);
        //view->playcontrol2->SliderPosition->setValue((int)buffer2->playposSliderNew);

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

/** initializes all QActions of the application */
void MixxxApp::initActions()
{
//  QPixmap openIcon, saveIcon, newIcon;
//  newIcon = QPixmap(filenew);
//  openIcon = QPixmap(fileopen);
//  saveIcon = QPixmap(filesave);

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

/*
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
*/

  optionsLeft = new QAction(tr("Left channel"), tr("&Left channel"), QAccel::stringToKey(tr("Ctrl+L")), this, 0, true);
  optionsLeft->setOn(true);
  optionsRight = new QAction(tr("Right channel"), tr("&Right channel"), QAccel::stringToKey(tr("Ctrl+R")), this, 0, true);
  optionsRight->setOn(true);

  optionsBeatMark = new QAction(tr("Audio Beat Marks"), tr("&Audio Beat Marks"), 0, this, 0, true);
  optionsBeatMark->setOn(false);
  optionsBeatMark->setStatusTip(tr("Audio Beat Marks"));
  optionsBeatMark->setWhatsThis(tr("Audio Beat Marks\nMark beats by audio clicks"));
  connect(optionsBeatMark, SIGNAL(toggled(bool)), this, SLOT(slotOptionsBeatMark(bool)));

  optionsPreferences = new QAction(tr("Preferences"), tr("&Preferences..."), 0, this);
  optionsPreferences->setStatusTip(tr("Preferences"));
  optionsPreferences->setWhatsThis(tr("Preferences\nPlayback and MIDI preferences"));
  connect(optionsPreferences, SIGNAL(activated()), this, SLOT(slotOptionsPreferences()));

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
  optionsBeatMark->addTo(optionsMenu);
  optionsPreferences->addTo(optionsMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry viewMenu
  viewMenu=new QPopupMenu();
  viewMenu->setCheckable(true);

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
#ifdef __VISUALS__
  visual = new MixxxVisual(app);
  visual->show();
#endif
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
    doc->newDoc();
}

void MixxxApp::slotFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(0,0,this);
    if (!fileName.isEmpty())
    {
        doc->load(fileName);
        setCaption(fileName);
        QString message=tr("Loaded document: ")+fileName;
    }
}


void MixxxApp::slotFileSave()
{
    doc->save();
}

void MixxxApp::slotFileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(0, 0, this);
    if (!fn.isEmpty())
    {
        doc->saveAs(fn);
    }
}

void MixxxApp::slotFileClose()
{
}

void MixxxApp::slotFilePrint()
{
    QPrinter printer;
    if (printer.setup(this))
    {
        QPainter painter;
        painter.begin(&printer);

        // Define printing by using the QPainter methods here

        painter.end();
    };
}

void MixxxApp::slotFileQuit()
{
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
}

void MixxxApp::slotEditCut()
{
}

void MixxxApp::slotEditCopy()
{
}

void MixxxApp::slotEditPaste()
{
}

void MixxxApp::slotOptionsBeatMark(bool)
{
// BEAT MARK STUFF
}

void MixxxApp::slotOptionsPreferences()
{
    prefDlg->setHidden(false);
}

void MixxxApp::slotHelpAbout()
{
    QMessageBox::about(this,tr("About..."),
                      tr("Mixxx\nVersion " VERSION "\nBy Tue and Ken Haste Andersen\nReleased under the GNU General Public Licence version 2") );
}

MixxxVisual *MixxxApp::getVisual()
{
    return visual;
}

