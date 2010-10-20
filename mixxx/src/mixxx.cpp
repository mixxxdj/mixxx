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

#include <QtDebug>
#include <QtCore>
#include <QtGui>

#include "widget/wknob.h"
#include "widget/wslider.h"
#include "widget/wpushbutton.h"
#include "widget/woverview.h"
#include "mixxx.h"
#include "controlnull.h"
#include "controlpotmeter.h"
#include "controlobjectthreadmain.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "engine/enginechannel.h"
#include "engine/enginevumeter.h"
#include "trackinfoobject.h"
#include "dlgabout.h"
#include "waveform/waveformrenderer.h"
#include "soundsourceproxy.h"

#include "analyserqueue.h"
#include "player.h"
#include "playermanager.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/libraryscanner.h"
#include "library/legacylibraryimporter.h"

#include "soundmanager.h"
#include "defs_urls.h"
#include "recording/defs_recording.h"

#include "midi/mididevicemanager.h"

#include "upgrade.h"

#include "build.h" // #defines of details of the build set up (flags,
// repo number, etc). This isn't a real file, SConscript generates it and it
// probably gets placed in $PLATFORM_build/. By including this file here and
// only here we make sure that updating src or changing the build flags doesn't
// force a rebuild of everything

#include "defs_version.h"

#ifdef __IPOD__
#include "gpod/itdb.h"
#endif

#ifdef __C_METRICS__
#include <cmetrics.h>
#include "defs_mixxxcmetrics.h"
#endif


extern "C" void crashDlg()
{
    QMessageBox::critical(0, "Mixxx",
        "Mixxx has encountered a serious error and needs to close.");
}


MixxxApp::MixxxApp(QApplication *a, struct CmdlineArgs args)
{
    m_pApp = a;

    QString buildRevision, buildFlags;
    #ifdef BUILD_REV
      buildRevision = BUILD_REV;
    #endif

    #ifdef BUILD_FLAGS
      buildFlags = BUILD_FLAGS;
    #endif

    if (buildRevision.trimmed().length() > 0) {
        if (buildFlags.trimmed().length() > 0)
            buildRevision = "(bzr r" + buildRevision + "; built on: "
                + __DATE__ + " @ " + __TIME__ + "; flags: "
                + buildFlags.trimmed() + ") ";
        else
            buildRevision = "(bzr r" + buildRevision + "; built on: "
                + __DATE__ + " @ " + __TIME__ + ") ";
    }

    qDebug() << "Mixxx" << VERSION << buildRevision << "is starting...";
    qDebug() << "Qt version is:" << qVersion();

    QCoreApplication::setApplicationName("Mixxx");
    QCoreApplication::setApplicationVersion(VERSION);
#if defined(AMD64) || defined(EM64T) || defined(x86_64)
    setWindowTitle(tr("Mixxx " VERSION " x64"));
#elif defined(IA64)
    setWindowTitle(tr("Mixxx " VERSION " Itanium"));
#else
    setWindowTitle(tr("Mixxx " VERSION));
#endif
    setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));

    //Reset pointer to players
    m_pSoundManager = 0;
    m_pPrefDlg = 0;
    m_pMidiDeviceManager = 0;

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pConfig = upgrader.versionUpgrade();
    bool bFirstRun = upgrader.isFirstRun();
    bool bUpgraded = upgrader.isUpgraded();
    QString qConfigPath = m_pConfig->getConfigPath();

#ifdef __C_METRICS__
    // Initialize Case Metrics if User is OK with that
    QString metricsAgree =
        m_pConfig->getValueString(
            ConfigKey("[User Experience]", "AgreedToUserExperienceProgram"));

    if (metricsAgree.isEmpty() || (metricsAgree != "yes" && metricsAgree != "no")) {
        metricsAgree = "no";
        int dlg = -1;
        while (dlg != 0 && dlg != 1) {
            dlg = QMessageBox::question(this, "Mixxx",
                "Mixxx's development is driven by community feedback.  At "
                "your discretion, Mixxx can automatically send data on your "
                "user experience back to the developers. Would you like to "
                "help us make Mixxx better by enabling this feature?",
                "Yes", "No", "Privacy Policy", 0, -1);
            switch (dlg) {
            case 0: metricsAgree = "yes";
            case 1: break;
            default: //show privacy policy
                QMessageBox::information(this, "Mixxx: Privacy Policy",
                    "Mixxx's development is driven by community feedback. "
                    "In order to help improve future versions Mixxx will with "
                    "your permission collect information on your hardware and "
                    "usage of Mixxx.  This information will primarily be used "
                    "to fix bugs, improve features, and determine the system "
                    "requirements of later versions.  Additionally this "
                    "information may be used in aggregate for statistical "
                    "purposes.\n\n"
                    "The hardware information will include:\n"
                    "\t- CPU model and features\n"
                    "\t- Total/Available Amount of RAM\n"
                    "\t- Available disk space\n"
                    "\t- OS version\n\n"
                    "Your usage information will include:\n"
                    "\t- Settings/Preferences\n"
                    "\t- Internal errors\n"
                    "\t- Internal debugging messages\n"
                    "\t- Performance statistics (average latency, CPU usage)\n"
                    "\nThis information will not be used to personally "
                    "identify you, contact you, advertise to you, or otherwise"
                    " bother you in any way.\n");
                break;
             }
        }
    }
    m_pConfig->set(
        ConfigKey("[User Experience]", "AgreedToUserExperienceProgram"),
        ConfigValue(metricsAgree)
    );

    // If the user agrees...
    if (metricsAgree == "yes") {
        // attempt to load the user ID from the config file
        if (m_pConfig->getValueString(ConfigKey("[User Experience]", "UID"))
                == "") {
            QString pUID = cm_generate_userid();
            if (!pUID.isEmpty()) {
                m_pConfig->set(
                    ConfigKey("[User Experience]", "UID"), ConigValue(pUID));
            }
        }
    }
    // Initialize cmetrics
    cm_init(100,20, metricsAgree == "yes", MIXXCMETRICS_RELEASE_ID,
        m_pConfig->getValueString(ConfigKey("[User Experience]", "UID"))
            .ascii());
    cm_set_crash_dlg(crashDlg);
    cm_writemsg_ascii(MIXXXCMETRICS_VERSION, VERSION);
#endif

    // Store the path in the config database
    m_pConfig->set(ConfigKey("[Config]", "Path"), ConfigValue(qConfigPath));

    // Instantiate a ControlObject, and set static parent widget
    m_pControl = new ControlNull();

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard =
        QDir::homePath().append("/").append(SETTINGS_PATH)
            .append("Custom.kbd.cfg");
    if (QFile::exists(userKeyboard)) {
        qDebug() << "Found and will use custom keyboard preset" << userKeyboard;
        m_pKbdConfig = new ConfigObject<ConfigValueKbd>(userKeyboard);
    }
    else
        // Otherwise use the default
        m_pKbdConfig =
            new ConfigObject<ConfigValueKbd>(QString(qConfigPath)
                .append("keyboard/").append("Standard.kbd.cfg"));
    WWidget::setKeyboardConfig(m_pKbdConfig);

    // Starting the master (mixing of the channels and effects):
    m_pEngine = new EngineMaster(m_pConfig, "[Master]");

    // Initialize player device

    m_pSoundManager = new SoundManager(m_pConfig, m_pEngine);
    m_pSoundManager->queryDevices();

    // Find path of skin
    QString qSkinPath = getSkinPath();

    // Get Music dir
    QDir dir(m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")));
    if (m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory"))
            .length() < 1 || !dir.exists()) {
        QString fd = QFileDialog::getExistingDirectory(this,
            tr("Choose music library directory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
        if (fd != "") {
            m_pConfig->set(ConfigKey("[Playlist]", "Directory"), fd);
            m_pConfig->Save();
        }
    }
    // Needed for Search class and Simple skin
    new ControlPotmeter(ConfigKey("[Channel1]", "virtualplayposition"),0.,1.);

    // Use frame as container for view, needed for fullscreen display
    m_pFrame = new QFrame;
    setCentralWidget(m_pFrame);

    m_pLibrary = new Library(this, m_pConfig, bFirstRun || bUpgraded);
    qRegisterMetaType<TrackPointer>("TrackPointer");

    // Create the player manager.
    m_pPlayerManager = new PlayerManager(m_pConfig, m_pEngine, m_pLibrary);
    m_pPlayerManager->addPlayer();
    m_pPlayerManager->addPlayer();

    m_pView = new MixxxView(m_pFrame, m_pKbdConfig, qSkinPath, m_pConfig,
                            m_pPlayerManager, m_pLibrary);

    //Scan the library directory.
    m_pLibraryScanner = new LibraryScanner(m_pLibrary->getTrackCollection());

    //Refresh the library models when the library (re)scan is finished.
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            m_pLibrary, SLOT(slotRefreshLibraryModels()));

    //Scan the library for new files and directories.
    m_pLibraryScanner->scan(
        m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")));


    // Call inits to invoke all other construction parts

    // TODO rryan : Move this to WaveformViewerFactory or something.
    /*
    if (bVisualsWaveform && !view->activeWaveform())
    {
        config->set(ConfigKey("[Controls]", "Visuals"), ConfigValue(1));
        QMessageBox * mb = new QMessageBox(this);
        mb->setWindowTitle(QString("Wavform displays"));
        mb->setIcon(QMessageBox::Information);
        mb->setText(
            "OpenGL cannot be initialized, which means that\n"
            "the waveform displays won't work. A simple\n"
            "mode will be used instead where you can still\n"
            "use the mouse to change speed.");
        mb->show();
    }
    */

    // Verify path for xml track file.
    QFile trackfile(
        m_pConfig->getValueString(ConfigKey("[Playlist]", "Listfile")));
    if (m_pConfig->getValueString(ConfigKey("[Playlist]", "Listfile"))
            .length() < 1 || !trackfile.exists())
    {
        m_pConfig->set(ConfigKey("[Playlist]", "Listfile"),
            QDir::homePath().append("/").append(SETTINGS_PATH)
                .append(TRACK_FILE));
        m_pConfig->Save();
    }

    // Intialize default BPM system values
    if (m_pConfig->getValueString(ConfigKey("[BPM]", "BPMRangeStart"))
            .length() < 1)
    {
        m_pConfig->set(ConfigKey("[BPM]", "BPMRangeStart"),ConfigValue(65));
    }

    if (m_pConfig->getValueString(ConfigKey("[BPM]", "BPMRangeEnd"))
            .length() < 1)
    {
        m_pConfig->set(ConfigKey("[BPM]", "BPMRangeEnd"),ConfigValue(135));
    }

    if (m_pConfig->getValueString(ConfigKey("[BPM]", "AnalyzeEntireSong"))
            .length() < 1)
    {
        m_pConfig->set(ConfigKey("[BPM]", "AnalyzeEntireSong"),ConfigValue(1));
    }

    // Initialise midi
    m_pMidiDeviceManager = new MidiDeviceManager(m_pConfig);
    //TODO: Try to open MIDI devices?
    m_pMidiDeviceManager->queryDevices();
    m_pMidiDeviceManager->setupDevices();


    // Initialize preference dialog
    m_pPrefDlg = new DlgPreferences(this, m_pView, m_pSoundManager,
                                 m_pMidiDeviceManager, m_pConfig);
    m_pPrefDlg->setHidden(true);

    // Try open player device If that fails, the preference panel is opened.
    while (m_pSoundManager->setupDevices() != 0)
    {

#ifdef __C_METRICS__
        cm_writemsg_ascii(MIXXXCMETRICS_FAILED_TO_OPEN_SNDDEVICE_AT_STARTUP,
                          "Mixxx failed to open audio device(s) on startup.");
#endif

        // Exit when we press the Exit button in the noSoundDlg dialog
        if ( noSoundDlg() != 0 )
            exit(0);
    }

    //setFocusPolicy(QWidget::StrongFocus);
    //grabKeyboard();

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    for (int i = 0; i < m_pPlayerManager->numPlayers()
            && i < args.qlMusicFiles.count(); ++i) {
        m_pPlayerManager->slotLoadToPlayer(args.qlMusicFiles.at(i), i+1);
    }

    //Automatically load specially marked promotional tracks on first run
    if (bFirstRun || bUpgraded) {
        QList<TrackPointer> tracksToAutoLoad =
            m_pLibrary->getTracksToAutoLoad();
        for (int i = 0; i < m_pPlayerManager->numPlayers()
                && i < tracksToAutoLoad.count(); i++) {
            m_pPlayerManager->slotLoadTrackToPlayer(tracksToAutoLoad.at(i), i+1);
        }
    }

#ifdef __SCRIPT__
    scriptEng = new ScriptEngine(this, m_pTrack);
#endif

    initActions();
    initMenuBar();

    // Check direct rendering and warn user if they don't have it
    m_pView->checkDirectRendering();

    //Install an event filter to catch certain QT events, such as tooltips.
    //This allows us to turn off tooltips.
    m_pApp->installEventFilter(this); // The eventfilter is located in this
                                      // Mixxx class as a callback.

    // If we were told to start in fullscreen mode on the command-line,
    // then turn on fullscreen mode.
    if (args.bStartInFullscreen)
        slotOptionsFullScreen(true);
#ifdef __C_METRICS__
    cm_writemsg_ascii(MIXXXCMETRICS_MIXXX_CONSTRUCTOR_COMPLETE,
            "Mixxx constructor complete.");
#endif

    // Refresh the GUI (workaround for Qt 4.6 display bug)
    QString QtVersion = qVersion();
    if (QtVersion>="4.6.0") {
        qDebug() << "Qt v4.6.0 or higher detected. Using rebootMixxxView() "
            "workaround.\n    (See bug https://bugs.launchpad.net/mixxx/"
            "+bug/521509)";
        rebootMixxxView();
    }
}

MixxxApp::~MixxxApp()
{
    QTime qTime;
    qTime.start();

    qDebug() << "Destroying MixxxApp";

// Moved this up to insulate macros you've worked hard on from being lost in
// a segfault that happens sometimes somewhere below here
#ifdef __SCRIPT__
    scriptEng->saveMacros();
    delete scriptEng;
#endif

#ifdef __IPOD__
    if (m_pTrack->m_qIPodPlaylist.getSongNum()) {
      qDebug() << "Dispose of iPod track collection";
      m_pTrack->m_qIPodPlaylist.clear();
    }
#endif
    qDebug() << "save config, " << qTime.elapsed();
    m_pConfig->Save();

    qDebug() << "close soundmanager" << qTime.elapsed();
    m_pSoundManager->closeDevices();
    qDebug() << "soundmanager->close() done";

    qDebug() << "delete MidiDeviceManager";
    delete m_pMidiDeviceManager;

    qDebug() << "delete soundmanager, " << qTime.elapsed();
    delete m_pSoundManager;

    qDebug() << "delete playerManager" << qTime.elapsed();
    delete m_pPlayerManager;

    qDebug() << "delete m_pEngine, " << qTime.elapsed();
    delete m_pEngine;

//    qDebug() << "delete prefDlg";
//    delete m_pControlEngine;

    qDebug() << "delete view, " << qTime.elapsed();
    delete m_pView;

    qDebug() << "delete library scanner" <<  qTime.elapsed();
    delete m_pLibraryScanner;

    // Delete the library after the view so there are no dangling pointers to
    // the data models.
    qDebug() << "delete library" << qTime.elapsed();
    delete m_pLibrary;

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pConfig->Save();

    delete m_pPrefDlg;

    delete m_pFrame;

#ifdef __C_METRICS__
    // cmetrics will cause this whole method to segfault on Linux/i386 if it
    // is called after config is deleted. Obviously, it depends on config somehow.
    qDebug() << "cmetrics to report:" << "Mixxx deconstructor complete.";
    cm_writemsg_ascii(MIXXXCMETRICS_MIXXX_DESTRUCTOR_COMPLETE,
            "Mixxx deconstructor complete.");
    cm_close(10);
#endif

    qDebug() << "delete config, " << qTime.elapsed();
    delete m_pConfig;
}

int MixxxApp::noSoundDlg(void)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Sound Device Busy");
    msgBox.setText(
        "<html>Mixxx cannot access the sound device <b>"+
        m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMaster"))+
        "</b>. "+
        "Another application is using the sound device or it is "+
        "not plugged in."+
        "<ul>"+
            "<li>"+
                "<b>Retry</b> after closing the other application "+
                "or reconnecting the sound device"+
            "</li>"+
            "<li>"+
                "<b>Reconfigure</b> Mixxx to use another sound device."+
            "</li>" +
            "<li>"+
                "Get <b>Help</b> from the Mixxx Wiki."+
            "</li>"+
            "<li>"+
                "<b>Exit</b> without saving your settings."+
            "</li>" +
        "</ul></html>"
    );

    QPushButton *retryButton = msgBox.addButton(tr("Retry"),
        QMessageBox::ActionRole);
    QPushButton *reconfigureButton = msgBox.addButton(tr("Reconfigure"),
        QMessageBox::ActionRole);
    QPushButton *wikiButton = msgBox.addButton(tr("Help"),
        QMessageBox::ActionRole);
    QPushButton *exitButton = msgBox.addButton(tr("Exit"),
        QMessageBox::ActionRole);

    while (true)
    {
        msgBox.exec();

        if (msgBox.clickedButton() == retryButton) {
            m_pSoundManager->queryDevices();
            return 0;
        } else if (msgBox.clickedButton() == wikiButton) {
            QDesktopServices::openUrl(QUrl(
                "http://mixxx.org/wiki/doku.php/troubleshooting"
                "#no_or_too_few_sound_cards_appear_in_the_preferences_dialog")
            );
            wikiButton->setEnabled(false);
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();
            m_pSoundManager->queryDevices();

            // This way of opening the dialog allows us to use it synchronously
            m_pPrefDlg->setWindowModality(Qt::ApplicationModal);
            m_pPrefDlg->exec();
            if (m_pPrefDlg->result() == QDialog::Accepted) {
                m_pSoundManager->queryDevices();
                return 0;
            }

            msgBox.show();

        } else if (msgBox.clickedButton() == exitButton) {
            return 1;
        }
    }
}


/** initializes all QActions of the application */
void MixxxApp::initActions()
{
    m_pFileLoadSongPlayer1 = new QAction(tr("&Load Song (Player 1)..."), this);
    m_pFileLoadSongPlayer1->setShortcut(tr("Ctrl+O"));
    m_pFileLoadSongPlayer1->setShortcutContext(Qt::ApplicationShortcut);

    m_pFileLoadSongPlayer2 = new QAction(tr("&Load Song (Player 2)..."), this);
    m_pFileLoadSongPlayer2->setShortcut(tr("Ctrl+Shift+O"));
    m_pFileLoadSongPlayer2->setShortcutContext(Qt::ApplicationShortcut);

    m_pFileQuit = new QAction(tr("E&xit"), this);
    m_pFileQuit->setShortcut(tr("Ctrl+Q"));
    m_pFileQuit->setShortcutContext(Qt::ApplicationShortcut);

    m_pLibraryRescan = new QAction(tr("&Rescan Library"), this);

    m_pPlaylistsNew = new QAction(tr("Add &new playlist"), this);
    m_pPlaylistsNew->setShortcut(tr("Ctrl+N"));
    m_pPlaylistsNew->setShortcutContext(Qt::ApplicationShortcut);

    m_pCratesNew = new QAction(tr("Add new &crate"), this);
    m_pCratesNew->setShortcut(tr("Ctrl+C"));
    m_pCratesNew->setShortcutContext(Qt::ApplicationShortcut);

    m_pPlaylistsImport = new QAction(tr("&Import playlist"), this);
    m_pPlaylistsImport->setShortcut(tr("Ctrl+I"));
    m_pPlaylistsImport->setShortcutContext(Qt::ApplicationShortcut);

#ifdef __IPOD__
    iPodToggle = new QAction(tr("iPod &Active"), this);
    iPodToggle->setShortcut(tr("Ctrl+A"));
    iPodToggle->setShortcutContext(Qt::ApplicationShortcut);
    iPodToggle->setCheckable(true);
    connect(iPodToggle, SIGNAL(toggled(bool)), this, SLOT(slotiPodToggle(bool)));
#endif

    m_pOptionsBeatMark = new QAction(tr("&Audio Beat Marks"), this);

    m_pOptionsFullScreen = new QAction(tr("&Full Screen"), this);
    m_pOptionsFullScreen->setShortcut(tr("F11"));
    m_pOptionsFullScreen->setShortcutContext(Qt::ApplicationShortcut);
    // QShortcut * shortcut = new QShortcut(QKeySequence(tr("Esc")),  this);
    // connect(shortcut, SIGNAL(activated()), this, SLOT(slotQuitFullScreen()));

    m_pOptionsPreferences = new QAction(tr("&Preferences"), this);
    m_pOptionsPreferences->setShortcut(tr("Ctrl+P"));
    m_pOptionsPreferences->setShortcutContext(Qt::ApplicationShortcut);

    m_pHelpAboutApp = new QAction(tr("&About..."), this);
    m_pHelpSupport = new QAction(tr("&Community Support..."), this);
#ifdef __VINYLCONTROL__
    m_pOptionsVinylControl = new QAction(tr("Enable &Vinyl Control"), this);
    m_pOptionsVinylControl->setShortcut(tr("Ctrl+Y"));
    m_pOptionsVinylControl->setShortcutContext(Qt::ApplicationShortcut);
#endif

#ifdef __SHOUTCAST__
    m_pOptionsShoutcast = new QAction(tr("Enable live broadcasting"), this);
    m_pOptionsShoutcast->setShortcut(tr("Ctrl+L"));
    m_pOptionsShoutcast->setShortcutContext(Qt::ApplicationShortcut);
#endif

    m_pOptionsRecord = new QAction(tr("&Record Mix"), this);
    //optionsRecord->setShortcut(tr("Ctrl+R"));
    m_pOptionsRecord->setShortcutContext(Qt::ApplicationShortcut);

#ifdef __SCRIPT__
    macroStudio = new QAction(tr("Show Studio"), this);
#endif

    m_pFileLoadSongPlayer1->setStatusTip(tr("Opens a song in player 1"));
    m_pFileLoadSongPlayer1->setWhatsThis(
        tr("Open\n\nOpens a song in player 1"));
    connect(m_pFileLoadSongPlayer1, SIGNAL(activated()),
            this, SLOT(slotFileLoadSongPlayer1()));

    m_pFileLoadSongPlayer2->setStatusTip(tr("Opens a song in player 2"));
    m_pFileLoadSongPlayer2->setWhatsThis(
        tr("Open\n\nOpens a song in player 2"));
    connect(m_pFileLoadSongPlayer2, SIGNAL(activated()),
            this, SLOT(slotFileLoadSongPlayer2()));

    m_pFileQuit->setStatusTip(tr("Quits the application"));
    m_pFileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
    connect(m_pFileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));

    m_pLibraryRescan->setStatusTip(tr("Rescans the song library"));
    m_pLibraryRescan->setWhatsThis(
        tr("Rescan library\n\nRescans the song library"));
    m_pLibraryRescan->setCheckable(false);
    connect(m_pLibraryRescan, SIGNAL(activated()),
            this, SLOT(slotScanLibrary()));
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            this, SLOT(slotEnableRescanLibraryAction()));

    m_pPlaylistsNew->setStatusTip(tr("Create a new playlist"));
    m_pPlaylistsNew->setWhatsThis(tr("New playlist\n\nCreate a new playlist"));
    connect(m_pPlaylistsNew, SIGNAL(activated()),
            m_pLibrary, SLOT(slotCreatePlaylist()));

    m_pCratesNew->setStatusTip(tr("Create a new crate"));
    m_pCratesNew->setWhatsThis(tr("New crate\n\nCreate a new crate."));
    connect(m_pCratesNew, SIGNAL(activated()),
            m_pLibrary, SLOT(slotCreateCrate()));

    m_pPlaylistsImport->setStatusTip(tr("Import playlist"));
    m_pPlaylistsImport->setWhatsThis(tr("Import playlist"));
    //connect(playlistsImport, SIGNAL(activated()),
    //        m_pTrack, SLOT(slotImportPlaylist()));
    //FIXME: Disabled due to library rework

    m_pOptionsBeatMark->setCheckable(false);
    m_pOptionsBeatMark->setChecked(false);
    m_pOptionsBeatMark->setStatusTip(tr("Audio Beat Marks"));
    m_pOptionsBeatMark->setWhatsThis(
        tr("Audio Beat Marks\nMark beats by audio clicks"));
    connect(m_pOptionsBeatMark, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsBeatMark(bool)));

#ifdef __VINYLCONTROL__
    // Either check or uncheck the vinyl control menu item depending on what
    // it was saved as.
    m_pOptionsVinylControl->setCheckable(true);
    if ((bool)m_pConfig->getValueString(ConfigKey("[VinylControl]", "Enabled"))
            .toInt() == true)
        m_pOptionsVinylControl->setChecked(true);
    else
        m_pOptionsVinylControl->setChecked(false);
    m_pOptionsVinylControl->setStatusTip(tr("Activate Vinyl Control"));
    m_pOptionsVinylControl->setWhatsThis(
        tr("Use timecoded vinyls on external turntables to control Mixxx"));
    connect(m_pOptionsVinylControl, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsVinylControl(bool)));
#endif

#ifdef __SHOUTCAST__
    m_pOptionsShoutcast->setCheckable(true);
    bool broadcastEnabled =
        (m_pConfig->getValueString(ConfigKey("[Shoutcast]", "enabled"))
            .toInt() == 1);

    m_pOptionsShoutcast->setChecked(broadcastEnabled);

    m_pOptionsShoutcast->setStatusTip(tr("Activate live broadcasting"));
    m_pOptionsShoutcast->setWhatsThis(
        tr("Stream your mixes to a shoutcast or icecast server"));

    connect(m_pOptionsShoutcast, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsShoutcast(bool)));
#endif

    m_pOptionsRecord->setCheckable(true);
    m_pOptionsRecord->setStatusTip(tr("Start Recording your Mix"));
    m_pOptionsRecord->setWhatsThis(tr("Record your mix to a file"));
    connect(m_pOptionsRecord, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsRecord(bool)));

    m_pOptionsFullScreen->setCheckable(true);
    m_pOptionsFullScreen->setChecked(false);
    m_pOptionsFullScreen->setStatusTip(tr("Full Screen"));
    m_pOptionsFullScreen->setWhatsThis(
        tr("Display Mixxx using the full screen"));
    connect(m_pOptionsFullScreen, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsFullScreen(bool)));

    m_pOptionsPreferences->setStatusTip(tr("Preferences"));
    m_pOptionsPreferences->setWhatsThis(
        tr("Preferences\nPlayback and MIDI preferences"));
    connect(m_pOptionsPreferences, SIGNAL(activated()),
            this, SLOT(slotOptionsPreferences()));

    m_pHelpSupport->setStatusTip(tr("Support..."));
    m_pHelpSupport->setWhatsThis(tr("Support\n\nGet help with Mixxx"));
    connect(m_pHelpSupport, SIGNAL(activated()), this, SLOT(slotHelpSupport()));

    m_pHelpAboutApp->setStatusTip(tr("About the application"));
    m_pHelpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
    connect(m_pHelpAboutApp, SIGNAL(activated()), this, SLOT(slotHelpAbout()));

#ifdef __SCRIPT__
    macroStudio->setStatusTip(tr("Shows the macro studio window"));
    macroStudio->setWhatsThis(
        tr("Show Studio\n\nMakes the macro studio visible"));
     connect(macroStudio, SIGNAL(activated()),
             scriptEng->getStudio(), SLOT(showStudio()));
#endif
}

void MixxxApp::initMenuBar()
{
    // MENUBAR
    m_pFileMenu=new QMenu(tr("&File"));
    m_pOptionsMenu=new QMenu(tr("&Options"));
    m_pLibraryMenu=new QMenu(tr("&Library"));
    m_pViewMenu=new QMenu(tr("&View"));
    m_pHelpMenu=new QMenu(tr("&Help"));
#ifdef __SCRIPT__
    macroMenu=new QMenu(tr("&Macro"));
#endif
    connect(m_pOptionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotOptionsMenuShow()));
    // menuBar entry fileMenu
    m_pFileMenu->addAction(m_pFileLoadSongPlayer1);
    m_pFileMenu->addAction(m_pFileLoadSongPlayer2);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pFileQuit);

    // menuBar entry optionsMenu
    //optionsMenu->setCheckable(true);
    //  optionsBeatMark->addTo(optionsMenu);
#ifdef __VINYLCONTROL__
    m_pOptionsMenu->addAction(m_pOptionsVinylControl);
#endif
    m_pOptionsMenu->addAction(m_pOptionsRecord);
#ifdef __SHOUTCAST__
    m_pOptionsMenu->addAction(m_pOptionsShoutcast);
#endif
    m_pOptionsMenu->addAction(m_pOptionsFullScreen);
    m_pOptionsMenu->addSeparator();
    m_pOptionsMenu->addAction(m_pOptionsPreferences);

    //    libraryMenu->setCheckable(true);
    m_pLibraryMenu->addAction(m_pLibraryRescan);
    m_pLibraryMenu->addSeparator();
    m_pLibraryMenu->addAction(m_pPlaylistsNew);
    m_pLibraryMenu->addAction(m_pCratesNew);
    //libraryMenu->addAction(playlistsImport);

#ifdef __IPOD__
    libraryMenu->addSeparator();
    libraryMenu->addAction(iPodToggle);
    connect(libraryMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotlibraryMenuAboutToShow()));
#endif

    // menuBar entry viewMenu
    //viewMenu->setCheckable(true);

    // menuBar entry helpMenu
    m_pHelpMenu->addAction(m_pHelpSupport);
    m_pHelpMenu->addSeparator();
    m_pHelpMenu->addAction(m_pHelpAboutApp);


#ifdef __SCRIPT__
    macroMenu->addAction(macroStudio);
#endif

    menuBar()->addMenu(m_pFileMenu);
    menuBar()->addMenu(m_pLibraryMenu);
    menuBar()->addMenu(m_pOptionsMenu);

    //    menuBar()->addMenu(viewMenu);
#ifdef __SCRIPT__
    menuBar()->addMenu(macroMenu);
#endif
    menuBar()->addSeparator();
    menuBar()->addMenu(m_pHelpMenu);

}


void MixxxApp::slotiPodToggle(bool toggle) {
#ifdef __IPOD__
// iPod stuff
  QString iPodMountPoint =
      config->getValueString(ConfigKey("[iPod]", "MountPoint"));
  bool iPodAvailable = !iPodMountPoint.isEmpty() &&
                       QDir( iPodMountPoint + "/iPod_Control").exists();
  bool iPodActivated = iPodAvailable && toggle;

  iPodToggle->setEnabled(iPodAvailable);

  if (iPodAvailable && iPodActivated
          && view->m_pComboBox->findData(TABLE_MODE_IPOD) == -1 ) {
    view->m_pComboBox->addItem( "iPod", TABLE_MODE_IPOD );
    // Activate IPod model

    Itdb_iTunesDB *itdb;
    itdb = itdb_parse (iPodMountPoint, NULL);
    if (itdb == NULL) {
      qDebug() << "Error reading iPod database\n";
      return;
    }
    GList *it;
    int count = 0;
    m_pTrack->m_qIPodPlaylist.clear();

    for (it = itdb->tracks; it != NULL; it = it->next) {
       count++;
       Itdb_Track *song;
       song = (Itdb_Track *)it->data;

//     DON'T USE QFileInfo, it does a disk i/o stat on every file introducing a
//     VERY long delay in loading from the iPod
//   QFileInfo file(iPodMountPoint + QString(song->ipod_path).replace(':','/'));

       QString fullFilePath =
           iPodMountPoint + QString(song->ipod_path).mid(1).replace(':','/');
       QString filePath = fullFilePath.left(fullFilePath.lastIndexOf('/'));
       QString fileName = fullFilePath.mid(fullFilePath.lastIndexOf('/')+1);
       QString fileSuffix = fullFilePath.mid(fullFilePath.lastIndexOf('.')+1);

       if (song->movie_flag) {
           qDebug() << "Movies/Videos not supported." << song->title
               << fullFilePath;
           continue;
       }
       if (song->unk220 && fileSuffix == "m4p") {
           qDebug() << "Protected media not supported." << song->title
               << fullFilePath;
           continue;
       }
#ifndef __FFMPEGFILE__
       if (fileSuffix == "m4a") {
           qDebug() << "m4a media support (via FFMPEG) is not compiled into "
               "this build of Mixxx. :( " << song->title << fullFilePath;
           continue;
       }
#endif // __FFMPEGFILE__


//qDebug() << "iPod file" << filePath << "--"<< fileName << "--" << fileSuffix;

       TrackInfoObject* pTrack = new TrackInfoObject(filePath, fileName);
       pTrack->setBpm(song->BPM);
       pTrack->setBpmConfirm(song->BPM != 0);  //void setBeatFirst(float); ??
//       pTrack->setHeaderParsed(true);
       pTrack->setComment(song->comment);
//       pTrack->setType(file.suffix());
       pTrack->setType(fileSuffix);
       pTrack->setBitrate(song->bitrate);
       pTrack->setSampleRate(song->samplerate);
       pTrack->setDuration(song->tracklen/1000);
       pTrack->setTitle(song->title);
       pTrack->setArtist(song->artist);
       // song->rating // user rating
       // song->volume and song->soundcheck -- track level normalization /
       // gain info as determined by iTunes
       m_pTrack->m_qIPodPlaylist.addTrack(pTrack);
    }
    itdb_free (itdb);

    //qDebug() << "iPod playlist has" << m_pTrack->m_qIPodPlaylist.getSongNum()
    //<< "of"<< count <<"songs on the iPod.";

    view->m_pComboBox->setCurrentIndex(
        view->m_pComboBox->findData(TABLE_MODE_IPOD) );
    //m_pTrack->slotActivatePlaylist(
    //    view->m_pComboBox->findData(TABLE_MODE_IPOD) );
    //m_pTrack->resizeColumnsForLibraryMode();

    //FIXME: Commented out above due to library rework.

  } else if (view->m_pComboBox->findData(TABLE_MODE_IPOD) != -1 ) {
    view->m_pComboBox->setCurrentIndex(
        view->m_pComboBox->findData(TABLE_MODE_LIBRARY) );
    //m_pTrack->slotActivatePlaylist(
    //  view->m_pComboBox->findData(TABLE_MODE_LIBRARY) );
    //FIXME: library reworking

    view->m_pComboBox->removeItem(
        view->m_pComboBox->findData(TABLE_MODE_IPOD) );
    // Empty iPod model m_qIPodPlaylist
    //m_pTrack->m_qIPodPlaylist.clear();

  }
#else
  Q_UNUSED(toggle); // suppress gcc unused parameter warning
#endif
}


void MixxxApp::slotlibraryMenuAboutToShow(){

#ifdef __IPOD__
  QString iPodMountPoint =
      m_pConfig->getValueString(ConfigKey("[iPod]", "MountPoint"));
  bool iPodAvailable = !iPodMountPoint.isEmpty() &&
                       QDir( iPodMountPoint + "/iPod_Control").exists();
  iPodToggle->setEnabled(iPodAvailable);

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

void MixxxApp::slotFileLoadSongPlayer1()
{
    ControlObject* play =
        ControlObject::getControl(ConfigKey("[Channel1]", "play"));

    if (play->get() == 1.)
    {
        int ret = QMessageBox::warning(this, tr("Mixxx"),
            tr("Player 1 is currently playing a song.\n"
            "Are you sure you want to load a new song?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return;
    }

    QString s =
        QFileDialog::getOpenFileName(
            this,
            tr("Load Song into Player 1"),
            m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")),
            QString("Audio (%1)")
                .arg(SoundSourceProxy::supportedFileExtensionsString()));

    if (s != QString::null) {
        m_pPlayerManager->slotLoadToPlayer(s, 1);
    }
}

void MixxxApp::slotFileLoadSongPlayer2()
{
    ControlObject* play =
        ControlObject::getControl(ConfigKey("[Channel2]", "play"));

    if (play->get() == 1.)
    {
        int ret = QMessageBox::warning(this, tr("Mixxx"),
            tr("Player 2 is currently playing a song.\n"
            "Are you sure you want to load a new song?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return;
    }

    QString s =
        QFileDialog::getOpenFileName(
            this,
            tr("Load Song into Player 2"),
            m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")),
            QString("Audio (%1)")
                .arg(SoundSourceProxy::supportedFileExtensionsString()));

    if (s != QString::null) {
        m_pPlayerManager->slotLoadToPlayer(s, 2);
    }
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
    if (m_pOptionsFullScreen)
        m_pOptionsFullScreen->setChecked(toggle);

    // Making a fullscreen window on linux and windows is harder than you
    // could possibly imagine...
    if (toggle)
    {
#if defined(__LINUX__) || defined(__APPLE__)
         m_winpos = pos();
         // Can't set max to -1,-1 or 0,0 for unbounded?
         setMaximumSize(32767,32767);
#endif

        showFullScreen();
        //menuBar()->hide();
        // FWI: Begin of fullscreen patch
#if defined(__LINUX__) || defined(__APPLE__)
        // Crazy X window managers break this so I'm told by Qt docs
        //         int deskw = app->desktop()->width();
        //         int deskh = app->desktop()->height();

        //support for xinerama
        int deskw = m_pApp->desktop()->screenGeometry(m_pFrame).width();
        int deskh = m_pApp->desktop()->screenGeometry(m_pFrame).height();
#else
        int deskw = width();
        int deskh = height();
#endif
        m_pView->move(
            (deskw - m_pView->width())/2, (deskh - m_pView->height())/2);
        // FWI: End of fullscreen patch
    }
    else
    {
        // FWI: Begin of fullscreen patch
        m_pView->move(0,0);
        menuBar()->show();
        showNormal();

#ifdef __LINUX__
        if (size().width() != m_pView->width() ||
            size().height() != m_pView->height() + menuBar()->height()) {
          setFixedSize(m_pView->width(),
                  m_pView->height() + menuBar()->height());
        }
        move(m_winpos);
#endif

        // FWI: End of fullscreen patch
    }
}

void MixxxApp::slotOptionsPreferences()
{
    m_pPrefDlg->setHidden(false);
}

//Note: Can't #ifdef this because MOC doesn't catch it.
void MixxxApp::slotOptionsVinylControl(bool toggle)
{
#ifdef __VINYLCONTROL__
    //qDebug() << "slotOptionsVinylControl: toggle is " << (int)toggle;

    QString device1 =
        m_pConfig->getValueString(ConfigKey("[VinylControl]", "DeviceInputDeck1")
    );
    QString device2 =
        m_pConfig->getValueString(ConfigKey("[VinylControl]", "DeviceInputDeck2")
    );

    if (device1 == "" && device2 == "" && (toggle==true))
    {
        QMessageBox::warning(this, tr("Mixxx"),
            tr("No input device(s) select.\n"
            "Please select your soundcard(s) in vinyl control preferences."),
            QMessageBox::Ok,
            QMessageBox::Ok);
        m_pPrefDlg->show();
        m_pPrefDlg->showVinylControlPage();
        m_pOptionsVinylControl->setChecked(false);
    }
    else
    {
        m_pConfig->set(
            ConfigKey("[VinylControl]", "Enabled"), ConfigValue((int)toggle));
        ControlObject::getControl(ConfigKey("[VinylControl]", "Enabled"))->set(
            (int)toggle);
    }
#endif
}

//Also can't ifdef this (MOC again)
void MixxxApp::slotOptionsRecord(bool toggle)
{
    ControlObjectThreadMain *recordingControl =
        new ControlObjectThreadMain(ControlObject::getControl(
                    ConfigKey("[Master]", "Record")));
    QString recordPath = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Path"));
    QString encodingType = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Encoding"));
    QString encodingFileFilter = QString("Audio (*.%1)").arg(encodingType);
    bool proceedWithRecording = true;

    if (toggle == true)
    {
        //If there was no recording path set,
        if (recordPath == "")
        {
            QString selectedFile = QFileDialog::getSaveFileName(NULL,
                    tr("Save Recording As..."),
                    recordPath,
                    encodingFileFilter);
            if (selectedFile.toLower() != "")
            {
                if(!selectedFile.toLower().endsWith(
                            "." + encodingType.toLower()))
                {
                    selectedFile.append("." + encodingType.toLower());
                }
                //Update the saved Path
                m_pConfig->set(
                    ConfigKey(RECORDING_PREF_KEY, "Path"), selectedFile);
            }
            else
                proceedWithRecording = false; //Empty filename, so don't record
        }
        else //If there was already a recording path set
        {
            //... and the file already exists, ask the user if they want to over
            // write it.
            int result;
            if(QFile::exists(recordPath))
            {
                QFileInfo fi(recordPath);
                result = QMessageBox::question(this, tr("Mixxx Recording"),
                    tr("The file %1 already exists. Would you like to overwrite"
                       " it?\nSelecting \"No\" will abort the recording.")
                    .arg(fi.fileName()), QMessageBox::Yes | QMessageBox::No);
                if (result == QMessageBox::Yes)
                    //If the user selected, "yes, overwrite the recording"...
                    proceedWithRecording = true;
                else
                    proceedWithRecording = false;
            }
        }

        if (proceedWithRecording == true)
        {
            qDebug() << "Setting record status: READY";
            recordingControl->slotSet(RECORD_READY);
        }
        else
        {
            m_pOptionsRecord->setChecked(false);
        }

    }
    else
    {
        qDebug() << "Setting record status: OFF";
        recordingControl->slotSet(RECORD_OFF);
    }

    delete recordingControl;
}

void MixxxApp::slotHelpAbout()
{

    DlgAbout *about = new DlgAbout(this);
#if defined(AMD64) || defined(EM64T) || defined(x86_64)
    about->version_label->setText(VERSION " x64");
#elif defined(IA64)
    about->version_label->setText(VERSION " IA64");
#else
    about->version_label->setText(VERSION);
#endif
    QString credits =
    QString("<p align=\"center\"><b>Mixxx %1 Development Team</b></p>"
"<p align=\"center\">"
"Adam Davison<br>"
"Albert Santoni<br>"
"RJ Ryan<br>"
"Garth Dahlstrom<br>"
"Sean Pappalardo<br>"
"Phillip Whelan<br>"
"Tobias Rafreider<br>"
"S. Brandt<br>"

"</p>"
"<p align=\"center\"><b>With contributions from:</b></p>"
"<p align=\"center\">"
"Mark Hills<br>"
"Andre Roth<br>"
"Robin Sheat<br>"
"Mark Glines<br>"
"Mathieu Rene<br>"
"Miko Kiiski<br>"
"Brian Jackson<br>"
"Owen Williams<br>"
"Andreas Pflug<br>"
"Bas van Schaik<br>"
"J&aacute;n Jockusch<br>"
"Oliver St&ouml;neberg<br>"
"Jan Jockusch<br>"
"C. Stewart<br>"
"Bill Egert<br>"
"Zach Shutters<br>"
"Owen Bullock<br>"
"Bill Good<br>"
"Graeme Mathieson<br>"
"Sebastian Actist<br>"
"Jussi Sainio<br>"
"David Gnedt<br>"
"Antonio Passamani<br>"
"Guy Martin<br>"
"Anders Gunnarson<br>"

"</p>"
"<p align=\"center\"><b>And special thanks to:</b></p>"
"<p align=\"center\">"
"Stanton<br>"
"Hercules<br>"
"EKS<br>"
"Echo Digital Audio<br>"
"Adam Bellinson<br>"
"Alexandre Bancel<br>"
"Melanie Thielker<br>"
"Julien Rosener<br>"
"Pau Arum&iacute;<br>"
"David Garcia<br>"
"Seb Ruiz<br>"
"Joseph Mattiello<br>"
"</p>"

"<p align=\"center\"><b>Past Developers</b></p>"
"<p align=\"center\">"
"Tue Haste Andersen<br>"
"Ken Haste Andersen<br>"
"Cedric Gestes<br>"
"John Sully<br>"
"Torben Hohn<br>"
"Peter Chang<br>"
"Micah Lee<br>"
"Ben Wheeler<br>"
"Wesley Stessens<br>"
"Nathan Prado<br>"
"Zach Elko<br>"
"Tom Care<br>"
"Pawel Bartkiewicz<br>"
"Nick Guenther<br>"
"</p>"

"<p align=\"center\"><b>Past Contributors</b></p>"
"<p align=\"center\">"
"Ludek Hor&#225;cek<br>"
"Svein Magne Bang<br>"
"Kristoffer Jensen<br>"
"Ingo Kossyk<br>"
"Mads Holm<br>"
"Lukas Zapletal<br>"
"Jeremie Zimmermann<br>"
"Gianluca Romanin<br>"
"Tim Jackson<br>"
"Stefan Langhammer<br>"
"Frank Willascheck<br>"
"Jeff Nelson<br>"
"Kevin Schaper<br>"
"Alex Markley<br>"
"Oriol Puigb&oacute;<br>"
"Ulrich Heske<br>"
"James Hagerman<br>"
"quil0m80<br>"
"Martin Sakm&#225;r<br>"
"Ilian Persson<br>"
"Alex Barker<br>"
"Dave Jarvis<br>"
"Thomas Baag<br>"
"Karlis Kalnins<br>"
"Amias Channer<br>"
"Sacha Berger<br>"
"James Evans<br>"
"Martin Sakmar<br>"
"Navaho Gunleg<br>"
"Gavin Pryke<br>"
"Michael Pujos<br>"
"Claudio Bantaloukas<br>"
"Pavol Rusnak<br>"

#if defined(AMD64) || defined(EM64T) || defined(x86_64)
    "</p>").arg(VERSION " x64");
#elif defined(IA64)
    "</p>").arg(VERSION " IA64");
#else
    "</p>").arg(VERSION);
#endif

    about->textBrowser->setHtml(credits);
    about->show();

}

void MixxxApp::slotHelpSupport()
{
    QUrl qSupportURL;
    qSupportURL.setUrl(MIXXX_SUPPORT_URL);
    QDesktopServices::openUrl(qSupportURL);
}

void MixxxApp::rebootMixxxView() {
    // Ok, so wierdly if you call setFixedSize with the same value twice,
    // Qt breaks
    // So we check and if the size hasn't changed we don't make the call
    int oldh = m_pView->height();
    int oldw = m_pView->width();
    qDebug() << "Now in Rebootmixxview...";

    // Workaround for changing skins while fullscreen, just go out of fullscreen
    // mode. If you change skins while in fullscreen (on Linux, at least) the
    // window returns to 0,0 but and the backdrop disappears so it looks as if
    // it is not fullscreen, but acts as if it is.
    slotOptionsFullScreen(false);

    QString qSkinPath = getSkinPath();

    m_pView->rebootGUI(m_pFrame, m_pConfig, qSkinPath);

    qDebug() << "rebootgui DONE";

    if (oldw != m_pView->width()
            || oldh != m_pView->height() + menuBar()->height()) {
      setFixedSize(m_pView->width(), m_pView->height() + menuBar()->height());
    }
}

QString MixxxApp::getSkinPath() {
    QString qConfigPath = m_pConfig->getConfigPath();

    QString qSkinPath(qConfigPath);
    qSkinPath.append("skins/");
    if (QDir(qSkinPath).exists())
    {
        // Is the skin listed in the config database there? If not, use
        // default (outlineSmall) skin
        if ((m_pConfig->getValueString(ConfigKey("[Config]", "Skin"))
                .length() > 0
            && QDir(QString(qSkinPath).append(
                m_pConfig->getValueString(ConfigKey("[Config]", "Skin"))))
                    .exists()))
            qSkinPath.append(
                m_pConfig->getValueString(ConfigKey("[Config]", "Skin")));
        else
        {
            m_pConfig->set(
                ConfigKey("[Config]", "Skin"), ConfigValue("outlineNetbook"));
            m_pConfig->Save();
            qSkinPath.append(
                m_pConfig->getValueString(ConfigKey("[Config]", "Skin")));
        }
    }
    else
        qCritical() << "Skin directory does not exist:" << qSkinPath;

    return qSkinPath;
}

/** Event filter to block certain events. For example, this function is used
  * to disable tooltips if the user specifies in the preferences that they
  * want them off. This is a callback function.
  */
bool MixxxApp::eventFilter(QObject *obj, QEvent *event)
{
    static int tooltips =
        m_pConfig->getValueString(ConfigKey("[Controls]", "Tooltips")).toInt();

    if (event->type() == QEvent::ToolTip) {
        // QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        // unused, remove? TODO(bkgood)
        if (tooltips == 1)
            return false;
        else
            return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }

}

void MixxxApp::slotScanLibrary()
{
    m_pLibraryRescan->setEnabled(false);
    m_pLibraryScanner->scan(
        m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")));
}

void MixxxApp::slotEnableRescanLibraryAction()
{
    m_pLibraryRescan->setEnabled(true);
}

void MixxxApp::slotOptionsMenuShow(){
    ControlObjectThread* ctrlRec =
        new ControlObjectThread(ControlObject::getControl(
                    ConfigKey("[Master]", "Record")));

    if(ctrlRec->get() == RECORD_OFF){
        //uncheck Recording
    m_pOptionsRecord->setChecked(false);
    }

#ifdef __SHOUTCAST__
    bool broadcastEnabled =
        (m_pConfig->getValueString(ConfigKey("[Shoutcast]", "enabled")).toInt()
            == 1);
    if (broadcastEnabled)
      m_pOptionsShoutcast->setChecked(true);
    else
      m_pOptionsShoutcast->setChecked(false);
#endif
}

void MixxxApp::slotOptionsShoutcast(bool value){
#ifdef __SHOUTCAST__
    m_pOptionsShoutcast->setChecked(value);
    m_pConfig->set(ConfigKey("[Shoutcast]", "enabled"),ConfigValue(value));
#else
    Q_UNUSED(value);
#endif
}
