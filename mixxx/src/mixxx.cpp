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
#include "controlnull.h"
#include "midiobjectnull.h"
#include "readerextractwave.h"
#include "controlengine.h"
#include "controlpotmeter.h"
#include "reader.h"
#include "enginebuffer.h"
#include "tracklist.h"
#include "powermate.h"
#include "joystick.h"
#include "enginevumeter.h"

#ifdef __UNIX__
#include "powermatelinux.h"
#endif
#ifdef __WIN__
#include "powermatewin.h"
#endif

#ifdef __UNIX__
#include "joysticklinux.h"
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

MixxxApp::MixxxApp(QApplication *a, bool bVisuals)
{
    app = a;

    qDebug("Starting up...");
    setCaption(tr("Mixxx " VERSION));

    // Reset pointer to players
    player = 0;
    m_pTracks = 0;
    prefDlg = 0;

    // Read the config file from home directory
    config = new ConfigObject<ConfigValue>(QDir::homeDirPath().append("/").append(SETTINGS_FILE));

    // Instantiate a ControlObject, and set static parent widget
    control = new ControlNull();
    control->setParentWidget(this);

    //
    // Find the config path, path where midi configuration files, skins etc. are stored.
    // On Linux the search order is whats listed in mixxx.cfg, then UNIX_SHARE_PATH
    // On Windows and Mac it is always (and only) app dir.
    //
    QString qConfigPath;
#ifdef __LINUX__
    // On Linux, check if the path is stored in the configuration database.
    if (config->getValueString(ConfigKey("[Config]","Path")).length()>0 && QDir(config->getValueString(ConfigKey("[Config]","Path"))).exists())
        qConfigPath = config->getValueString(ConfigKey("[Config]","Path"));
    else
    {
        // Set the path according to the compile time define, UNIX_SHARE_PATH
        qConfigPath = UNIX_SHARE_PATH;
    }
#endif
#ifdef __WIN__
    // On Windows, set the config dir relative to the application dir
    char *str = new char[200];
    GetModuleFileName(NULL, (unsigned short *)str, 200);
    qConfigPath = QFileInfo(str).dirPath();
#endif
#ifdef __MACX__
    // Set the path relative to the bundle directory
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    qConfigPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
#endif
    // If the directory does not end with a "/", add one
    if (!qConfigPath.endsWith("/"))
        qConfigPath.append("/");
    config->set(ConfigKey("[Config]","Path"), ConfigValue(qConfigPath));


    // Call inits to invoke all other construction parts
    initActions();
    initMenuBar();

    // Open midi
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
        
    // Get list of available midi configurations, and read the default configuration. If no default
    // is given, use the first configuration found in the config directory.
    QStringList *midiConfigList = midi->getConfigList(QString(qConfigPath).append("midi/"));
    midiconfig = 0;
    for (QStringList::Iterator it = midiConfigList->begin(); it != midiConfigList->end(); ++it )
        if (*it == config->getValueString(ConfigKey("[Midi]","File")))
            midiconfig = new ConfigObject<ConfigValueMidi>(QString(qConfigPath).append("midi/").append(config->getValueString(ConfigKey("[Midi]","File"))));
    if (midiconfig == 0)
    {
        if (midiConfigList->empty())
        {
            midiconfig = new ConfigObject<ConfigValueMidi>("");
            config->set(ConfigKey("[Midi]","File"), ConfigValue(""));
        }
        else
        {
            midiconfig = new ConfigObject<ConfigValueMidi>(QString(qConfigPath).append("midi/").append((*midiConfigList->at(0)).latin1()));
            config->set(ConfigKey("[Midi]","File"), ConfigValue((*midiConfigList->at(0)).latin1()));
        }
    }

    // Set the static config pointer for the ControlObject
    control->setConfig(midiconfig);

    // Configure ControlEngine object
    ControlEngine *m_pControlEngine = new ControlEngine(control);

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
		{
			qDebug("Found PowerMate 1");
	    }
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
 
    // Try initializing Joystick
    joystick1 = 0;
#ifdef __LINUX__
    joystick1 = new JoystickLinux(control);
#endif

    if (joystick1!=0)
    {
        if (joystick1->opendev())
        {
            qDebug("Found Joystick 1");
        }
        else
        {
            delete joystick1;
            joystick1 = 0;
        }
    }

    // Initialize player device
#ifdef __ALSA__
    player = new PlayerALSA(BUFFER_SIZE, &engines, config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")));
#else
    player = new PlayerPortAudio(config,control,app);
#endif

    // Init buffers/readers
    buffer1 = new EngineBuffer(powermate1, "[Channel1]");
    buffer2 = new EngineBuffer(powermate2, "[Channel2]");

    // Starting channels:
    channel1 = new EngineChannel("[Channel1]");
    channel2 = new EngineChannel("[Channel2]");

    // Starting effects:
    flanger = new EngineFlanger("[Flanger]");

    // Starting the master (mixing of the channels and effects):
    master = new EngineMaster(buffer1, buffer2, channel1, channel2, flanger, "[Master]");

    // Find path of skin
    QString qSkinPath(qConfigPath);
    qSkinPath.append("skins/");
    if (QDir(qSkinPath).exists())
    {
        // Is the skin listed in the config database there? If not, use default (outline) skin
        if ((config->getValueString(ConfigKey("[Config]","Skin")).length()>0 && QDir(QString(qSkinPath).append(config->getValueString(ConfigKey("[Config]","Skin")))).exists()))
            qSkinPath.append(config->getValueString(ConfigKey("[Config]","Skin")));
        else
        {
            config->set(ConfigKey("[Config]","Skin"), ConfigValue("outline"));
            config->Save();
            qSkinPath.append(config->getValueString(ConfigKey("[Config]","Skin")));
        }
    }
    else
        qFatal("Skin directory does not exist: %s",qSkinPath.latin1());

    // Initialize widgets
    view=new MixxxView(this,bVisuals, qSkinPath);
    setCentralWidget(view);

    // Tell EngineBuffer to notify the visuals
    buffer1->setVisual(view->m_pVisualCh1);
    buffer2->setVisual(view->m_pVisualCh2);

    // Dynamic zoom on visuals
    if (view->m_bZoom)
    {
        ControlObject::connectControls(ConfigKey("[Channel1]", "rate"), ConfigKey("[Channel1]", "VisualLengthScale-marks"));
        ControlObject::connectControls(ConfigKey("[Channel1]", "rate"), ConfigKey("[Channel1]", "VisualLengthScale-signal"));
        ControlObject::connectControls(ConfigKey("[Channel2]", "rate"), ConfigKey("[Channel2]", "VisualLengthScale-marks"));
        ControlObject::connectControls(ConfigKey("[Channel2]", "rate"), ConfigKey("[Channel2]", "VisualLengthScale-signal"));
    }
            
    // Initialize tracklist:
    m_pTracks = new TrackList(config->getValueString(ConfigKey("[Playlist]","Directory")), view->m_pTrackTable,
                              view->m_pTextCh1, view->m_pTextCh2, buffer1, buffer2);

    // Initialize preference dialog
    prefDlg = new DlgPreferences(this, view, midi, player, m_pTracks, config, midiconfig, control);
    prefDlg->setHidden(true);

    // Try open player device using config data, if that fails, use default values. If that fails too, the
    // preference panel should be opened.
    if (!player->open(false))
        if (!player->open(true))
            prefDlg->setHidden(false);


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
//    delete m_pControlEngine;
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

  optionsPreferences = new QAction(tr("Preferences"), tr("&Preferences..."), QAccel::stringToKey(tr("Ctrl+P")), this);
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
//  optionsBeatMark->addTo(optionsMenu);
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
                      tr("Mixxx\nVersion " VERSION "\nhttp://mixxx.sourceforge.net/\n\nDesign and programming: Tue Haste Andersen and Ken Haste Andersen.\nGraphics: Ludek Horácek - Traditional skin\n              Tue Haste Andersen - Outline skin\nOgg vorbis support: Svein Magne Bang.\nOther contributions: Lukas Zapletal.\n\nThanks to all DJ's and musicians giving feedback.\n\nReleased under the GNU General Public Licence version 2") );
}
