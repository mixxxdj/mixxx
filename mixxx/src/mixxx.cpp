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
#include "powermate.h"
#include "enginevumeter.h"

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

    // Instantiate a ControlObject, and set static parent widget
    control = new ControlNull();
    control->setParentWidget(this);

    initView();

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
    buffer1 = new EngineBuffer(powermate1, "[Channel1]", view->m_pVisualCh1);
    buffer2 = new EngineBuffer(powermate2, "[Channel2]", view->m_pVisualCh2);

    // Initialize tracklist:
    m_pTracks = new TrackList(config->getValueString(ConfigKey("[Playlist]","Directory")), view->m_pTrackTable,
                              view->m_pTextCh1, view->m_pTextCh2, buffer1, buffer2);

    // Initialize preference dialog
    prefDlg = new DlgPreferences(this,midi,player,m_pTracks, config, midiconfig);
    prefDlg->setHidden(true);

    // Try open player device using config data, if that fails, use default values. If that fails too, the
    // preference panel should be opened.
    if (!player->open(false))
        if (!player->open(true))
            prefDlg->setHidden(false);
    
    // Starting channels:
    channel1 = new EngineChannel("[Channel1]");
    channel2 = new EngineChannel("[Channel2]");

    // Starting effects:
    flanger = new EngineFlanger("[Flanger]");

    // Starting the master (mixing of the channels and effects):
    master = new EngineMaster(buffer1, buffer2, channel1, channel2, flanger, "[Master]");

    // Assign widgets to corresponding ControlObjects
    view->assignWidgets(control);

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
  fileQuit = new QAction(tr("Exit"), tr("E&xit"), QAccel::stringToKey(tr("Ctrl+Q")), this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
  connect(fileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));

  optionsBeatMark = new QAction(tr("Audio Beat Marks"), tr("&Audio Beat Marks"), 0, this, 0, true);
  optionsBeatMark->setOn(false);
  optionsBeatMark->setStatusTip(tr("Audio Beat Marks"));
  optionsBeatMark->setWhatsThis(tr("Audio Beat Marks\nMark beats by audio clicks"));
  connect(optionsBeatMark, SIGNAL(toggled(bool)), this, SLOT(slotOptionsBeatMark(bool)));

  optionsFullScreen = new QAction(tr("Full Screen"), tr("&Full Screen"), QAccel::stringToKey(tr("Ctrl+F")), this, 0, this);
  optionsFullScreen->setOn(false);
  optionsFullScreen->setStatusTip(tr("Full Screen"));
  optionsFullScreen->setWhatsThis(tr("Display Mixxx using the full screen"));
  connect(optionsFullScreen, SIGNAL(toggled(bool)), this, SLOT(slotOptionsFullScreen(bool)));

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
  fileQuit->addTo(fileMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry optionsMenu
  optionsMenu=new QPopupMenu();
  optionsMenu->setCheckable(true);
  optionsBeatMark->addTo(optionsMenu);
  optionsFullScreen->addTo(optionsMenu);
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

void MixxxApp::initView()
{
    // set the main widget here
    view=new MixxxView(this);
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



void MixxxApp::slotFileQuit()
{
    qApp->quit();
}

void MixxxApp::slotOptionsBeatMark(bool)
{
// BEAT MARK STUFF
}

void MixxxApp::slotOptionsFullScreen(bool toggle)
{
    if (toggle)
    {
        menuBar()->hide();
        showFullScreen();
    }
    else
    {
        menuBar()->show();
        showNormal();
    }
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
