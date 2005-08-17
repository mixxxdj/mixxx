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

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#ifdef QT3_SUPPORT
#include <q3accel.h>
#include <Q3PopupMenu>
#include <q3table.h>
#include <q3listview.h>
#include <q3ptrlist.h>
#include <Q3Action>
#include <QPixmap>
#include <qicon.h>
#else
#include <qaccel.h>
#include <qtable.h>
#include <qptrlist.h>
#endif
#include <qlistview.h>
#include <qiconset.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>
#include <qsplashscreen.h>

#include "wknob.h"
#include "wslider.h"
#include "wpushbutton.h"
#include "wtracktable.h"
#include "woverview.h"
#include "mixxx.h"
#include "controlnull.h"
#include "readerextractwave.h"
#include "controlpotmeter.h"
#include "reader.h"
#include "enginebuffer.h"
#include "enginevumeter.h"
#include "track.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "mixxxsocketserver.h"
#include "mixxxmenuplaylists.h"
#include "wtreeitem.h"
#include "wavesummary.h"
#include "log.h"

#include "playerproxy.h"


MixxxApp::MixxxApp(QApplication *a, QStringList files, QSplashScreen *pSplash, QString qLogFileName)
{
    app = a;

    qDebug("Starting up...");
    setCaption(tr("Mixxx " VERSION));
#ifndef __MACX__
#ifndef QT3_SUPPORT
    setIcon(QPixmap::fromMimeSource("icon.png"));
#endif
#endif

    // Reset pointer to players
    player = 0;
    m_pTrack = 0;
    prefDlg = 0;

    // Read the config file from home directory
    config = new ConfigObject<ConfigValue>(QDir::homeDirPath().append("/").append(SETTINGS_FILE));
    QString qConfigPath = config->getConfigPath();
    
    // Store the path in the config database
    config->set(ConfigKey("[Config]","Path"), ConfigValue(qConfigPath));
    
    // Instantiate a ControlObject, and set static parent widget
    control = new ControlNull();

    
    if (pSplash)
        pSplash->message("Initializing control devices...",Qt::AlignLeft|Qt::AlignBottom);
        
    // Read keyboard configuration and set kdbConfig object in WWidget
    kbdconfig = new ConfigObject<ConfigValueKbd>(QString(qConfigPath).append("keyboard/").append("Standard.kbd.cfg"));
    WWidget::setKeyboardConfig(kbdconfig);


    if (pSplash)
        pSplash->message("Setting up sound engine...",Qt::AlignLeft|Qt::AlignBottom);
    
    // Sample rate used by Player object
    ControlObject *p = new ControlObject(ConfigKey("[Master]","samplerate"));
    p->set(44100.);

    // Master rate
    new ControlPotmeter(ConfigKey("[Master]","rate"),-1.,1.);
        
    // Init buffers/readers
    buffer1 = new EngineBuffer("[Channel1]");
    buffer2 = new EngineBuffer("[Channel2]");
    buffer1->setOtherEngineBuffer(buffer2);
    buffer2->setOtherEngineBuffer(buffer1);

    // Starting channels:
    channel1 = new EngineChannel("[Channel1]");
    channel2 = new EngineChannel("[Channel2]");

    // Starting the master (mixing of the channels and effects):
    master = new EngineMaster(buffer1, buffer2, channel1, channel2, "[Master]");

    // Initialize player device
    Player::setMaster(master);
    player = new PlayerProxy(config);

    if (pSplash)
        pSplash->message("Loading skin...",Qt::AlignLeft|Qt::AlignBottom);
    
    // Find path of skin
    QString qSkinPath(qConfigPath);
    qSkinPath.append("skins/");
    if (QDir(qSkinPath).exists())
    {
        // Is the skin listed in the config database there? If not, use default (outlineSmall) skin
        if ((config->getValueString(ConfigKey("[Config]","Skin")).length()>0 && QDir(QString(qSkinPath).append(config->getValueString(ConfigKey("[Config]","Skin")))).exists()))
            qSkinPath.append(config->getValueString(ConfigKey("[Config]","Skin")));
        else
        {
            config->set(ConfigKey("[Config]","Skin"), ConfigValue("outlineSmall"));
            config->Save();
            qSkinPath.append(config->getValueString(ConfigKey("[Config]","Skin")));
        }
    }
    else
        qFatal("Skin directory does not exist: %s",qSkinPath.latin1());

    // Get Music dir
    QDir dir(config->getValueString(ConfigKey("[Playlist]","Directory")));
    if ((config->getValueString(ConfigKey("[Playlist]","Directory")).length()<1) | (!dir.exists()))
    {
    	  #ifdef QT3_SUPPORT
        Q3FileDialog* fd = new Q3FileDialog(this, "", true);
        fd->setMode( Q3FileDialog::Directory );
        #else
        QFileDialog* fd = new QFileDialog(this, "", true);
        fd->setMode( QFileDialog::Directory );
        #endif
        fd->setCaption(QString("Choose directory with music files"));
        if ( fd->exec() == QDialog::Accepted )
        {
            config->set(ConfigKey("[Playlist]","Directory"), fd->selectedFile());
            config->Save();
        }
    }

    // Needed for Search class and Simple skin
    new ControlPotmeter(ConfigKey("[Channel1]","virtualplayposition"),0.,1.);
    
    // Initialize widgets
    bool bVisualsWaveform = true;
    if (config->getValueString(ConfigKey("[Controls]","Visuals")).toInt()==1)
        bVisualsWaveform = false;
    view=new MixxxView(this, kbdconfig, bVisualsWaveform, qSkinPath, config);
    if (bVisualsWaveform && !view->activeWaveform())
    {
        config->set(ConfigKey("[Controls]","Visuals"), ConfigValue(1));
        QMessageBox* mb = new QMessageBox(this);
        mb->setCaption(QString("Wavform displays"));
        mb->setIcon(QMessageBox::Information);
        mb->setText("OpenGL cannot be initialized, which means that\nthe waveform displays won't work. A simple\nmode will be used instead where you can still\nuse the mouse to change speed.");
        mb->show();
    }
    setCentralWidget(view);

    // Tell EngineBuffer to notify the visuals if they are WVisualWaveform
    if (view->activeWaveform())
    {
        buffer1->setVisual((WVisualWaveform *)view->m_pVisualCh1);
        buffer2->setVisual((WVisualWaveform *)view->m_pVisualCh2);

        // Dynamic zoom on visuals
        if (view->m_bZoom)
        {
            ControlObject::connectControls(ConfigKey("[Channel1]", "rate"), ConfigKey("[Channel1]", "VisualLengthScale-marks"));
            ControlObject::connectControls(ConfigKey("[Channel1]", "rate"), ConfigKey("[Channel1]", "VisualLengthScale-signal"));
            ControlObject::connectControls(ConfigKey("[Channel2]", "rate"), ConfigKey("[Channel2]", "VisualLengthScale-marks"));
            ControlObject::connectControls(ConfigKey("[Channel2]", "rate"), ConfigKey("[Channel2]", "VisualLengthScale-signal"));
        }
    }

    // Verify path for xml track file.
    QFile trackfile(config->getValueString(ConfigKey("[Playlist]","Listfile")));
    if ((config->getValueString(ConfigKey("[Playlist]","Listfile")).length()<1) | (!trackfile.exists()))
    {
        config->set(ConfigKey("[Playlist]","Listfile"), QDir::homeDirPath().append("/").append(TRACK_FILE));
        config->Save();
    }

    // Initialize wavefrom summary generation
    m_pWaveSummary = new WaveSummary();

    if (pSplash)
        pSplash->message("Loading song database...",Qt::AlignLeft|Qt::AlignBottom);

    // Initialize track object:
    m_pTrack = new Track(config->getValueString(ConfigKey("[Playlist]","Listfile")), view, buffer1, buffer2, m_pWaveSummary);
    WTreeItem::setTrack(m_pTrack);

    // Set up drag and drop to player visuals
    if (view->m_pVisualCh1)
        connect(view->m_pVisualCh1, SIGNAL(trackDropped(QString)), m_pTrack, SLOT(slotLoadPlayer1(QString)));
    if (view->m_pVisualCh2)
        connect(view->m_pVisualCh2, SIGNAL(trackDropped(QString)), m_pTrack, SLOT(slotLoadPlayer2(QString)));

    // Ensure that visual receive updates when new tracks are loaded
    if (view->m_pVisualCh1)
        connect(m_pTrack, SIGNAL(newTrackPlayer1(TrackInfoObject *)), view->m_pVisualCh1, SLOT(slotNewTrack()));
    if (view->m_pVisualCh2)
        connect(m_pTrack, SIGNAL(newTrackPlayer2(TrackInfoObject *)), view->m_pVisualCh2, SLOT(slotNewTrack()));



    // Setup state of End of track controls from config database
    ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode"))->queueFromThread(config->getValueString(ConfigKey("[Controls]","TrackEndModeCh1")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode"))->queueFromThread(config->getValueString(ConfigKey("[Controls]","TrackEndModeCh2")).toDouble());

    // Initialize preference dialog
    prefDlg = new DlgPreferences(this, view, player, m_pTrack, config);
    prefDlg->setHidden(true);

    // Try open player device If that fails, the preference panel is opened.
    if (!player->open())
        prefDlg->setHidden(false);

    //setFocusPolicy(QWidget::StrongFocus);
    //grabKeyboard();

    // Load tracks in files (command line arguments) into player 1 and 2:
#ifndef QT3_SUPPORT
    if (files.count()>1)
        m_pTrack->slotLoadPlayer1((*files.at(1)));
    if (files.count()>2)
        m_pTrack->slotLoadPlayer2((*files.at(2)));
#else
    if (files.count()>1)
        m_pTrack->slotLoadPlayer1(files.at(1));
    if (files.count()>2)
        m_pTrack->slotLoadPlayer2(files.at(2));
#endif

    // Set up socket interface
#ifndef __WIN__
    new MixxxSocketServer(m_pTrack);
#endif

    // Initialize visualization of temporal effects
    channel1->setVisual(buffer1);
    channel2->setVisual(buffer2);

    // Dynamic scaling of temporal effect curves
    if (view->m_bZoom)
    {
        ControlObject::connectControls(ConfigKey("[Channel1]", "rate"), ConfigKey("[Channel1]", "VisualLengthScale-temporal"));
        ControlObject::connectControls(ConfigKey("[Channel2]", "rate"), ConfigKey("[Channel2]", "VisualLengthScale-temporal"));
    }
    
    // Call inits to invoke all other construction parts
    initActions();
    initMenuBar();

    // Check direct rendering
    if (bVisualsWaveform)
        view->checkDirectRendering();
    
    // Initialize the log if a log file name was given on the command line
    Log *pLog = 0;
    if (qLogFileName.length()>0)
        pLog = new Log(qLogFileName, m_pTrack);
}

MixxxApp::~MixxxApp()
{
//    qDebug("Destroying MixxxApp");

//    qDebug("close player");
    player->close();

    // Save state of End of track controls in config database
    config->set(ConfigKey("[Controls]","TrackEndModeCh1"), ConfigValue((int)ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode"))->get()));
    config->set(ConfigKey("[Controls]","TrackEndModeCh2"), ConfigValue((int)ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode"))->get()));

//    qDebug("delete player");
    delete player;
//    qDebug("delete master");
    delete master;
//    qDebug("delete channel1");
    delete channel1;
//    qDebug("delete channel2");
    delete channel2;
//    qDebug("delete buffer1");
    delete buffer1;
//    qDebug("delete buffer2");
    delete buffer2;
//    qDebug("delete prefDlg");
//    delete m_pControlEngine;
//    qDebug("delete midi");
//    qDebug("delete midiconfig");

    qDebug("Write track xml");
    m_pTrack->writeXML(config->getValueString(ConfigKey("[Playlist]","Listfile")));

    qDebug("delete tracks");
    delete m_pTrack;
    qDebug("delete view");
    delete view;

    delete prefDlg;
    
    qDebug("save config");
    config->Save();
    qDebug("delete config");
    delete config;

}

/** initializes all QActions of the application */
void MixxxApp::initActions()
{
#ifdef QT3_SUPPORT
    fileOpen = new Q3Action(tr("Open..."), tr("&Open"), Q3Accel::stringToKey(tr("Ctrl+O")), this);
    fileQuit = new Q3Action(tr("Exit"), tr("E&xit"), Q3Accel::stringToKey(tr("Ctrl+Q")), this);
    playlistsNew = new Q3Action(tr("Add new playlist"), tr("&New playlist"), Q3Accel::stringToKey(tr("Ctrl+N")), this, 0, this);
    playlistsImport = new Q3Action(tr("Import playlist"), tr("&Import playlist"), Q3Accel::stringToKey(tr("Ctrl+I")), this, 0, this);
    optionsBeatMark = new Q3Action(tr("Audio Beat Marks"), tr("&Audio Beat Marks"), 0, this, 0, true);
    optionsFullScreen = new Q3Action(tr("Full Screen"), tr("&Full Screen"), Q3Accel::stringToKey(tr("Esc")), this, 0, this);
    optionsPreferences = new Q3Action(tr("Preferences"), tr("&Preferences..."), Q3Accel::stringToKey(tr("Ctrl+P")), this);
    helpAboutApp = new Q3Action(tr("About"), tr("&About..."), 0, this);
#else
    fileOpen = new QAction(tr("Open..."), tr("&Open"), QAccel::stringToKey(tr("Ctrl+O")), this);
    fileQuit = new QAction(tr("Exit"), tr("E&xit"), QAccel::stringToKey(tr("Ctrl+Q")), this);
    playlistsNew = new QAction(tr("Add new playlist"), tr("&New playlist"), QAccel::stringToKey(tr("Ctrl+N")), this, 0, this);
    playlistsImport = new QAction(tr("Import playlist"), tr("&Import playlist"), QAccel::stringToKey(tr("Ctrl+I")), this, 0, this);
    optionsBeatMark = new QAction(tr("Audio Beat Marks"), tr("&Audio Beat Marks"), 0, this, 0, true);
    optionsFullScreen = new QAction(tr("Full Screen"), tr("&Full Screen"), QAccel::stringToKey(tr("Esc")), this, 0, this);
    optionsPreferences = new QAction(tr("Preferences"), tr("&Preferences..."), QAccel::stringToKey(tr("Ctrl+P")), this);
    helpAboutApp = new QAction(tr("About"), tr("&About..."), 0, this);
#endif

    fileOpen->setStatusTip(tr("Opens a file in player 1"));
    fileOpen->setWhatsThis(tr("Open\n\nOpens a file in player 1"));
    connect(fileOpen, SIGNAL(activated()), this, SLOT(slotFileOpen()));
    
    fileQuit->setStatusTip(tr("Quits the application"));
    fileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
    connect(fileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));
    
    playlistsNew->setStatusTip(tr("Add new playlist"));
    playlistsNew->setWhatsThis(tr("Add a new playlist"));
    connect(playlistsNew, SIGNAL(activated()), m_pTrack, SLOT(slotNewPlaylist()));

    playlistsImport->setStatusTip(tr("Import playlist"));
    playlistsImport->setWhatsThis(tr("Import playlist"));
    connect(playlistsImport, SIGNAL(activated()), m_pTrack, SLOT(slotImportPlaylist()));

    optionsBeatMark->setOn(false);
    optionsBeatMark->setStatusTip(tr("Audio Beat Marks"));
    optionsBeatMark->setWhatsThis(tr("Audio Beat Marks\nMark beats by audio clicks"));
    connect(optionsBeatMark, SIGNAL(toggled(bool)), this, SLOT(slotOptionsBeatMark(bool)));

    optionsFullScreen->setOn(false);
    optionsFullScreen->setStatusTip(tr("Full Screen"));
    optionsFullScreen->setWhatsThis(tr("Display Mixxx using the full screen"));
    connect(optionsFullScreen, SIGNAL(toggled(bool)), this, SLOT(slotOptionsFullScreen(bool)));

    optionsPreferences->setStatusTip(tr("Preferences"));
    optionsPreferences->setWhatsThis(tr("Preferences\nPlayback and MIDI preferences"));
    connect(optionsPreferences, SIGNAL(activated()), this, SLOT(slotOptionsPreferences()));

    helpAboutApp->setStatusTip(tr("About the application"));
    helpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
    connect(helpAboutApp, SIGNAL(activated()), this, SLOT(slotHelpAbout()));
}

void MixxxApp::initMenuBar()
{
    // MENUBAR

    #ifdef QT3_SUPPORT
    fileMenu=new Q3PopupMenu();
    optionsMenu=new Q3PopupMenu();
    playlistsMenu=new Q3PopupMenu();
    viewMenu=new Q3PopupMenu();
    helpMenu=new Q3PopupMenu();
    #else
    fileMenu=new QPopupMenu();
    optionsMenu=new QPopupMenu();
    playlistsMenu=new QPopupMenu();
    viewMenu=new QPopupMenu();
    helpMenu=new QPopupMenu();
    #endif

    // menuBar entry fileMenu
    fileOpen->addTo(fileMenu);
    fileQuit->addTo(fileMenu);

    // menuBar entry optionsMenu
    optionsMenu->setCheckable(true);
    //  optionsBeatMark->addTo(optionsMenu);
    optionsFullScreen->addTo(optionsMenu);
    optionsPreferences->addTo(optionsMenu);

    playlistsMenu->setCheckable(true);
    playlistsNew->addTo(playlistsMenu);
    playlistsImport->addTo(playlistsMenu);
    playlistsMenu->insertSeparator();

    new MixxxMenuPlaylists(playlistsMenu, m_pTrack);

    // menuBar entry viewMenu
    viewMenu->setCheckable(true);

    // menuBar entry helpMenu
    helpAboutApp->addTo(helpMenu);

    // MENUBAR CONFIGURATION
    menuBar()->insertItem(tr("&File"), fileMenu);
    //menuBar()->insertItem(tr("&Edit"), editMenu);
    menuBar()->insertItem(tr("&Playlists"), playlistsMenu);
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

void MixxxApp::slotFileOpen()
{
	#ifdef QT3_SUPPORT
    QString s = Q3FileDialog::getOpenFileName(config->getValueString(ConfigKey("[Playlist]","Directory")),
                                             "Audio (*.wav *.ogg *.mp3 *.aiff)",
                                             this,
                                             "Open file");
    #else
    QString s = QFileDialog::getOpenFileName(config->getValueString(ConfigKey("[Playlist]","Directory")),
                                             "Audio (*.wav *.ogg *.mp3 *.aiff)",
                                             this,
                                             "Open file");
    #endif
    TrackInfoObject *pTrack = m_pTrack->getTrackCollection()->getTrack(s);
    if (pTrack)
        m_pTrack->slotLoadPlayer1(pTrack);
}

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
                      tr("<qt>"
                         "<table cellspacing=0 cellpadding=0>"
                         "<tr><td>Mixxx</td></tr>"
                         "<tr><td>Version " VERSION "</td></tr>"
                         "<tr><td><a href=\"http://mixxx.sourceforge.net/\">http://mixxx.sourceforge.net/</a></td></tr>"
                         "</table><br><br>"
                         "<table cellspacing=0 cellpadding=0>"
                         "<tr><td>Design and programming:</td><td>Tue Haste Andersen</td></tr>"
                         "<tr><td></td><td>Ken Haste Andersen</td></tr>"
                         "<tr><td>Skins:</td><td>Ludek Horácek (Traditional)</td></tr>"
                         "<tr><td></td><td>Tue Haste Andersen (Outline)</td></tr>"
                         "<tr><td>Ogg vorbis support:</td><td>Svein Magne Bang</td></tr>"
                         "<tr><td>Beat tracking:</td><td>Tue Haste Andersen</td></tr>"
                         "<tr><td></td><td>Kristoffer Jensen</td></tr>"
                         "<tr><td>Playlist import & ASIO:</td><td>Ingo Kossyk</td></tr>"
             "<tr><td>Beat phase sync:</td><td>Torben Hohn</td></tr>"
             "<tr><td>ALSA support:</td><td>Peter Chang</td></tr>"
             "<tr><td>SoundTouch support:</td><td>Mads Holm</td></tr>"
                         "<tr><td>Other contributions:</td><td>Lukas Zapletal</td></tr>"
                         "<tr><td></td><td>Jeremie Zimmermann</td></tr>"
                         "<tr><td></td><td>Gianluca Romanin</td></tr>"
                         "</table><br><br>"
                         "<table cellspacing=0 cellpadding=0>"
                         "<tr><td>Thanks to all DJ's and musicians giving feedback.</td></tr>"
                         "<tr><td>Released under the GNU General Public Licence version 2.</td></tr>"
                         "</table></qt>") );
}
