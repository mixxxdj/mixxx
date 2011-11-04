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
#include <QTranslator>

#include "mixxx.h"
#include "controlnull.h"
#include "controlpotmeter.h"
#include "controlobjectthreadmain.h"
#include "engine/enginemaster.h"
#include "engine/enginemicrophone.h"
#include "trackinfoobject.h"
#include "dlgabout.h"
#include "waveformviewerfactory.h"
#include "waveform/waveformrenderer.h"
#include "soundsourceproxy.h"

#include "analyserqueue.h"
#include "playermanager.h"

#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/libraryscanner.h"

#include "soundmanager.h"
#include "soundmanagerutil.h"
#include "defs_urls.h"
#include "recording/defs_recording.h"


#include "midi/mididevicemanager.h"

#include "upgrade.h"
#include "mixxxkeyboard.h"
#include "skin/skinloader.h"
#include "skin/legacyskinparser.h"

#include "build.h" // #defines of details of the build set up (flags,
// repo number, etc). This isn't a real file, SConscript generates it and it
// probably gets placed in $PLATFORM_build/. By including this file here and
// only here we make sure that updating src or changing the build flags doesn't
// force a rebuild of everything

#include "defs_version.h"

#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"
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
#ifdef __APPLE__
    setWindowTitle(tr("Mixxx")); //App Store
#elif defined(AMD64) || defined(EM64T) || defined(x86_64)
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
    m_pRecordingManager = 0;

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pConfig = upgrader.versionUpgrade();
    bool bFirstRun = upgrader.isFirstRun();
    bool bUpgraded = upgrader.isUpgraded();
    QString qConfigPath = m_pConfig->getConfigPath();
    QString translationsFolder = qConfigPath + "translations/";

    // Load Qt base translations
    QString locale = args.locale;
    if (locale == "") {
        locale = QLocale::system().name();
    }

    // Load Qt translations for this locale
    QTranslator* qtTranslator = new QTranslator(a);
   if( qtTranslator->load("qt_" + locale,
           QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
       a->installTranslator(qtTranslator);
   }
   else{
       delete qtTranslator;
   }

    // Load Mixxx specific translations for this locale
    QTranslator* mixxxTranslator = new QTranslator(a);
    bool mixxxLoaded = mixxxTranslator->load("mixxx_" + locale, translationsFolder);
    qDebug() << "Loading translations for locale" << locale
            << "from translations folder" << translationsFolder << ":"
            << (mixxxLoaded ? "success" : "fail");
   if (mixxxLoaded) {
       a->installTranslator(mixxxTranslator);
   }
   else {
       delete mixxxTranslator;
   }

#ifdef __C_METRICS__
    // Initialize Case Metrics if User is OK with that
    QString metricsAgree =
        m_pConfig->getValueString(
            ConfigKey("[User Experience]", "AgreedToUserExperienceProgram"));

    if (metricsAgree.isEmpty() || (metricsAgree != "yes" && metricsAgree != "no")) {
        metricsAgree = "no";
        int dlg = -1;
        while (dlg != 0 && dlg != 1) {
            dlg = QMessageBox::question(this, tr("Mixxx"),
                tr("Mixxx's development is driven by community feedback.  At "
                "your discretion, Mixxx can automatically send data on your "
                "user experience back to the developers. Would you like to "
                "help us make Mixxx better by enabling this feature?"),
                tr("Yes"), tr("No"), tr("Privacy Policy"), 0, -1);
            switch (dlg) {
            case 0: metricsAgree = "yes";
            case 1: break;
            default: //show privacy policy
                QMessageBox::information(this, tr("Mixxx: Privacy Policy"),
                    tr("Mixxx's development is driven by community feedback. "
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
                    " bother you in any way.\n"));
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

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard =
        QDir::homePath().append("/").append(SETTINGS_PATH)
            .append("Custom.kbd.cfg");

    ConfigObject<ConfigValueKbd>* pKbdConfig = NULL;

    if (QFile::exists(userKeyboard)) {
        qDebug() << "Found and will use custom keyboard preset" << userKeyboard;
        pKbdConfig = new ConfigObject<ConfigValueKbd>(userKeyboard);
    }
    else
        // Otherwise use the default
        pKbdConfig =
                new ConfigObject<ConfigValueKbd>(
                    QString(qConfigPath)
                    .append("keyboard/").append("Standard.kbd.cfg"));

    // TODO(XXX) leak pKbdConfig, MixxxKeyboard owns it? Maybe roll all keyboard
    // initialization into MixxxKeyboard
    // Workaround for today: MixxxKeyboard calls delete
    m_pKeyboard = new MixxxKeyboard(pKbdConfig);

    //create RecordingManager
    m_pRecordingManager = new RecordingManager(m_pConfig);

    // Starting the master (mixing of the channels and effects):
    m_pEngine = new EngineMaster(m_pConfig, "[Master]");

    connect(m_pEngine, SIGNAL(isRecording(bool)),
            m_pRecordingManager,SLOT(slotIsRecording(bool)));
    connect(m_pEngine, SIGNAL(bytesRecorded(int)),
            m_pRecordingManager,SLOT(slotBytesRecorded(int)));

    // Initialize player device
    // while this is created here, setupDevices needs to be called sometime
    // after the players are added to the engine (as is done currently) -- bkgood
    m_pSoundManager = new SoundManager(m_pConfig, m_pEngine);

    EngineMicrophone* pMicrophone = new EngineMicrophone("[Microphone]");
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 0); // What should channelbase be?
    m_pEngine->addChannel(pMicrophone);
    m_pSoundManager->registerInput(micInput, pMicrophone);

    // Get Music dir
    bool hasChanged_MusicDir = false;
    QDir dir(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));
    if (m_pConfig->getValueString(
        ConfigKey("[Playlist]","Directory")).length() < 1 || !dir.exists())
    {
        // TODO this needs to be smarter, we can't distinguish between an empty
        // path return value (not sure if this is normally possible, but it is
        // possible with the Windows 7 "Music" library, which is what
        // QDesktopServices::storageLocation(QDesktopServices::MusicLocation)
        // resolves to) and a user hitting 'cancel'. If we get a blank return
        // but the user didn't hit cancel, we need to know this and let the
        // user take some course of action -- bkgood
        QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose music library directory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

        if (fd != "")
        {
            m_pConfig->set(ConfigKey("[Playlist]","Directory"), fd);
            m_pConfig->Save();
            hasChanged_MusicDir = true;
        }
    }
    /*
     * Do not write meta data back to ID3 when meta data has changed
     * Because multiple TrackDao objects can exists for a particular track
     * writing meta data may ruine your MP3 file if done simultaneously.
     * see Bug #728197
     * For safety reasons, we deactivate this feature.
     */
    m_pConfig->set(ConfigKey("[Library]","WriteAudioTags"), ConfigValue(0));


    // library dies in seemingly unrelated qtsql error about not having a
    // sqlite driver if this path doesn't exist. Normally config->Save()
    // above would make it but if it doesn't get run for whatever reason
    // we get hosed -- bkgood
    if (!QDir(QDir::homePath().append("/").append(SETTINGS_PATH)).exists()) {
        QDir().mkpath(QDir::homePath().append("/").append(SETTINGS_PATH));
    }



    m_pLibrary = new Library(this, m_pConfig,
                             bFirstRun || bUpgraded,
                             m_pRecordingManager);
    qRegisterMetaType<TrackPointer>("TrackPointer");

    // Create the player manager.
    m_pPlayerManager = new PlayerManager(m_pConfig, m_pEngine, m_pLibrary);
    m_pPlayerManager->addDeck();
    m_pPlayerManager->addDeck();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();

    // register the engine's outputs
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::MASTER),
        m_pEngine);
    m_pSoundManager->registerOutput(AudioOutput(AudioOutput::HEADPHONES),
        m_pEngine);
    for (unsigned int deck = 0; deck < m_pPlayerManager->numDecks(); ++deck) {
        // TODO(bkgood) make this look less dumb by putting channelBase after
        // index in the AudioOutput() params
        m_pSoundManager->registerOutput(
            AudioOutput(AudioOutput::DECK, 0, deck), m_pEngine);
    }

#ifdef __VINYLCONTROL__
    m_pVCManager = new VinylControlManager(this, m_pConfig);
    for (unsigned int deck = 0; deck < m_pPlayerManager->numDecks(); ++deck) {
        m_pSoundManager->registerInput(
            AudioInput(AudioInput::VINYLCONTROL, 0, deck),
            m_pVCManager);
    }
#else
    m_pVCManager = NULL;
#endif

    //Scan the library directory.
    m_pLibraryScanner = new LibraryScanner(m_pLibrary->getTrackCollection());

    //Refresh the library models when the library (re)scan is finished.
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            m_pLibrary, SLOT(slotRefreshLibraryModels()));

    //Scan the library for new files and directories
    bool rescan = (bool)m_pConfig->getValueString(ConfigKey("[Library]","RescanOnStartup")).toInt();
    // rescan the library if we get a new plugin
    QSet<QString> prev_plugins = QSet<QString>::fromList(m_pConfig->getValueString(
        ConfigKey("[Library]", "SupportedFileExtensions")).split(",", QString::SkipEmptyParts));
    QSet<QString> curr_plugins = QSet<QString>::fromList(
        SoundSourceProxy::supportedFileExtensions());
    rescan = rescan || (prev_plugins != curr_plugins);

    if(rescan || hasChanged_MusicDir){
        m_pLibraryScanner->scan(
            m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")));
        qDebug() << "Rescan finished";
    }
    m_pConfig->set(ConfigKey("[Library]", "SupportedFileExtensions"),
        QStringList(SoundSourceProxy::supportedFileExtensions()).join(","));

    // Call inits to invoke all other construction parts

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

    //ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode"))->queueFromThread(m_pConfig->getValueString(ConfigKey("[Controls]","TrackEndModeCh1")).toDouble());
    //ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode"))->queueFromThread(m_pConfig->getValueString(ConfigKey("[Controls]","TrackEndModeCh2")).toDouble());

    qRegisterMetaType<MidiMessage>("MidiMessage");
    qRegisterMetaType<MidiStatusByte>("MidiStatusByte");

    // Initialise midi
    m_pMidiDeviceManager = new MidiDeviceManager(m_pConfig);
    m_pMidiDeviceManager->setupDevices();

    m_pSkinLoader = new SkinLoader(m_pConfig);

    // Initialize preference dialog
    m_pPrefDlg = new DlgPreferences(this, m_pSkinLoader, m_pSoundManager, m_pPlayerManager,
                                 m_pMidiDeviceManager, m_pVCManager, m_pConfig);
    m_pPrefDlg->setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));
    m_pPrefDlg->setHidden(true);

    // Try open player device If that fails, the preference panel is opened.
    int setupDevices = m_pSoundManager->setupDevices();
    unsigned int numDevices = m_pSoundManager->getConfig().getOutputs().count();
    // test for at least one out device, if none, display another dlg that
    // says "mixxx will barely work with no outs"
    while (setupDevices != OK || numDevices == 0)
    {

#ifdef __C_METRICS__
        cm_writemsg_ascii(MIXXXCMETRICS_FAILED_TO_OPEN_SNDDEVICE_AT_STARTUP,
                          "Mixxx failed to open audio device(s) on startup.");
#endif

        // Exit when we press the Exit button in the noSoundDlg dialog
        // only call it if setupDevices != OK
        if (setupDevices != OK) {
            if (noSoundDlg() != 0) {
                exit(0);
            }
        } else if (numDevices == 0) {
            bool continueClicked = false;
            int noOutput = noOutputDlg(&continueClicked);
            if (continueClicked) break;
            if (noOutput != 0) {
                exit(0);
            }
        }
        setupDevices = m_pSoundManager->setupDevices();
        numDevices = m_pSoundManager->getConfig().getOutputs().count();
    }

    //setFocusPolicy(QWidget::StrongFocus);
    //grabKeyboard();

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    for (int i = 0; i < (int)m_pPlayerManager->numDecks()
            && i < args.qlMusicFiles.count(); ++i) {
        m_pPlayerManager->slotLoadToDeck(args.qlMusicFiles.at(i), i+1);
    }

    //Automatically load specially marked promotional tracks on first run
    if (bFirstRun || bUpgraded) {
        QList<TrackPointer> tracksToAutoLoad =
            m_pLibrary->getTracksToAutoLoad();
        for (int i = 0; i < (int)m_pPlayerManager->numDecks()
                && i < tracksToAutoLoad.count(); i++) {
            m_pPlayerManager->slotLoadToDeck(tracksToAutoLoad.at(i)->getLocation(), i+1);
        }
    }

#ifdef __SCRIPT__
    scriptEng = new ScriptEngine(this, m_pTrack);
#endif

    initActions();
    initMenuBar();

    // Use frame as container for view, needed for fullscreen display
    m_pView = new QFrame;

    m_pWidgetParent = NULL;
    // Loads the skin as a child of m_pView
    // assignment itentional in next line
    if (!(m_pWidgetParent = m_pSkinLoader->loadDefaultSkin(m_pView,
                                        m_pKeyboard,
                                        m_pPlayerManager,
                                        m_pLibrary,
                                        m_pVCManager))) {
        qDebug() << "Could not load default skin.";
    }

    // this has to be after the OpenGL widgets are created or depending on a
    // million different variables the first waveform may be horribly
    // corrupted. See bug 521509 -- bkgood
    setCentralWidget(m_pView);

    // keep gui centered (esp for fullscreen)
    // the layout will be deleted whenever m_pView gets deleted
    QHBoxLayout *pLayout = new QHBoxLayout(m_pView);
    pLayout->addWidget(m_pWidgetParent);
    pLayout->setContentsMargins(0, 0, 0, 0); // don't want margins

    // Check direct rendering and warn user if they don't have it
    checkDirectRendering();

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
    /* // TODO(bkgood) delete this block if the moving of setCentralWidget
     * //              totally fixes this first-wavefore-fubar issue for
     * //              everyone
    QString QtVersion = qVersion();
    if (QtVersion>="4.6.0") {
        qDebug() << "Qt v4.6.0 or higher detected. Using rebootMixxxView() "
            "workaround.\n    (See bug https://bugs.launchpad.net/mixxx/"
            "+bug/521509)";
        rebootMixxxView();
    } */
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

    qDebug() << "save config, " << qTime.elapsed();
    m_pConfig->Save();

    // Save state of End of track controls in config database
    //m_pConfig->set(ConfigKey("[Controls]","TrackEndModeCh1"), ConfigValue((int)ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode"))->get()));
    //m_pConfig->set(ConfigKey("[Controls]","TrackEndModeCh2"), ConfigValue((int)ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode"))->get()));

    // SoundManager depend on Engine and Config
    qDebug() << "delete soundmanager, " << qTime.elapsed();
    delete m_pSoundManager;

#ifdef __VINYLCONTROL__
    // VinylControlManager depends on a CO the engine owns
    // (vinylcontrol_enabled in VinylControlControl)
    qDebug() << "delete vinylcontrolmanager, " << qTime.elapsed();
    delete m_pVCManager;
#endif

    // View depends on MixxxKeyboard, PlayerManager, Library
    qDebug() << "delete view, " << qTime.elapsed();
    delete m_pView;

    // SkinLoader depends on Config
    qDebug() << "delete SkinLoader";
    delete m_pSkinLoader;

    // MIDIDeviceManager depends on Config
    qDebug() << "delete MidiDeviceManager";
    delete m_pMidiDeviceManager;

    // PlayerManager depends on Engine, Library, and Config
    qDebug() << "delete playerManager" << qTime.elapsed();
    delete m_pPlayerManager;

    // EngineMaster depends on Config
    qDebug() << "delete m_pEngine, " << qTime.elapsed();
    delete m_pEngine;

    // LibraryScanner depends on Library
    qDebug() << "delete library scanner" <<  qTime.elapsed();
    delete m_pLibraryScanner;

    // Delete the library after the view so there are no dangling pointers to
    // Depends on RecordingManager
    // the data models.
    qDebug() << "delete library" << qTime.elapsed();
    delete m_pLibrary;

    // RecordingManager depends on config
    qDebug() << "delete RecordingManager" << qTime.elapsed();
    delete m_pRecordingManager;

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.

    //Disable shoutcast so when Mixxx starts again it will not connect
    m_pConfig->set(ConfigKey("[Shoutcast]", "enabled"),0);
    m_pConfig->Save();
    delete m_pPrefDlg;

#ifdef __C_METRICS__
    // cmetrics will cause this whole method to segfault on Linux/i386 if it is
    // called after config is deleted. Obviously, it depends on config somehow.
    qDebug() << "cmetrics to report:" << "Mixxx deconstructor complete.";
    cm_writemsg_ascii(MIXXXCMETRICS_MIXXX_DESTRUCTOR_COMPLETE,
            "Mixxx deconstructor complete.");
    cm_close(10);
#endif

    qDebug() << "delete config, " << qTime.elapsed();
    delete m_pConfig;

    // Check for leaked ControlObjects and give warnings.
    QList<ControlObject*> leakedControls;
    QList<ConfigKey> leakedConfigKeys;

    ControlObject::getControls(&leakedControls);

    if (leakedControls.size() > 0) {
        qDebug() << "WARNING: The following" << leakedControls.size() << "controls were leaked:";
        foreach (ControlObject* pControl, leakedControls) {
            ConfigKey key = pControl->getKey();
            qDebug() << key.group << key.item;
            leakedConfigKeys.append(key);
        }

       foreach (ConfigKey key, leakedConfigKeys) {
           // delete just to satisfy valgrind:
           // check if the pointer is still valid, the control object may have bin already
           // deleted by its parent in this loop
           delete ControlObject::getControl(key);
       }
   }
   qDebug() << "~MixxxApp: All leaking controls deleted.";

   delete m_pKeyboard;
}

int MixxxApp::noSoundDlg(void)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Sound Device Busy"));
    msgBox.setText(
        "<html>" +
        tr("Mixxx was unable to access all the configured sound devices. "
        "Another application is using a sound device Mixxx is configured to "
        "use or a device is not plugged in.") +
        "<ul>"
            "<li>" +
                tr("<b>Retry</b> after closing the other application "
                "or reconnecting a sound device") +
            "</li>"
            "<li>" +
                tr("<b>Reconfigure</b> Mixxx's sound device settings.") +
            "</li>"
            "<li>" +
                tr("Get <b>Help</b> from the Mixxx Wiki.") +
            "</li>"
            "<li>" +
                tr("<b>Exit</b> Mixxx.") +
            "</li>"
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

int MixxxApp::noOutputDlg(bool *continueClicked)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("No Output Devices");
    msgBox.setText( "<html>Mixxx was configured without any output sound devices. "
                    "Audio processing will be disabled without a configured output device."
                    "<ul>"
                        "<li>"
                            "<b>Continue</b> without any outputs."
                        "</li>"
                        "<li>"
                            "<b>Reconfigure</b> Mixxx's sound device settings."
                        "</li>"
                        "<li>"
                            "<b>Exit</b> Mixxx."
                        "</li>"
                    "</ul></html>"
    );

    QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::ActionRole);
    QPushButton *reconfigureButton = msgBox.addButton(tr("Reconfigure"), QMessageBox::ActionRole);
    QPushButton *exitButton = msgBox.addButton(tr("Exit"), QMessageBox::ActionRole);

    while (true)
    {
        msgBox.exec();

        if (msgBox.clickedButton() == continueButton) {
            *continueClicked = true;
            return 0;
        } else if (msgBox.clickedButton() == reconfigureButton) {
            msgBox.hide();
            m_pSoundManager->queryDevices();

            // This way of opening the dialog allows us to use it synchronously
            m_pPrefDlg->setWindowModality(Qt::ApplicationModal);
            m_pPrefDlg->exec();
            if ( m_pPrefDlg->result() == QDialog::Accepted) {
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

    m_pFileQuit = new QAction(tr("&Exit"), this);
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

    m_pOptionsBeatMark = new QAction(tr("&Audio Beat Marks"), this);

    m_pOptionsFullScreen = new QAction(tr("&Full Screen"), this);

#ifdef __APPLE__
    m_pOptionsFullScreen->setShortcut(tr("Ctrl+F"));
#else
    m_pOptionsFullScreen->setShortcut(tr("F11"));
#endif

    m_pOptionsFullScreen->setShortcutContext(Qt::ApplicationShortcut);
    // QShortcut * shortcut = new QShortcut(QKeySequence(tr("Esc")),  this);
    // connect(shortcut, SIGNAL(triggered()), this, SLOT(slotQuitFullScreen()));

    m_pOptionsPreferences = new QAction(tr("&Preferences"), this);
    m_pOptionsPreferences->setShortcut(tr("Ctrl+P"));
    m_pOptionsPreferences->setShortcutContext(Qt::ApplicationShortcut);

    m_pHelpAboutApp = new QAction(tr("&About"), this);
    m_pHelpSupport = new QAction(tr("&Community Support"), this);
    m_pHelpFeedback = new QAction(tr("Send Us &Feedback"), this);
    m_pHelpTranslation = new QAction(tr("&Translate this application"), this);

#ifdef __VINYLCONTROL__
    m_pOptionsVinylControl = new QAction(tr("Enable &Vinyl Control 1"), this);
    m_pOptionsVinylControl->setShortcut(tr("Ctrl+Y"));
    m_pOptionsVinylControl->setShortcutContext(Qt::ApplicationShortcut);

    m_pOptionsVinylControl2 = new QAction(tr("Enable &Vinyl Control 2"), this);
    m_pOptionsVinylControl2->setShortcut(tr("Ctrl+U"));
    m_pOptionsVinylControl2->setShortcutContext(Qt::ApplicationShortcut);
#endif

#ifdef __SHOUTCAST__
    m_pOptionsShoutcast = new QAction(tr("Enable live broadcasting"), this);
    m_pOptionsShoutcast->setShortcut(tr("Ctrl+L"));
    m_pOptionsShoutcast->setShortcutContext(Qt::ApplicationShortcut);
#endif

    m_pOptionsRecord = new QAction(tr("&Record Mix"), this);
    m_pOptionsRecord->setShortcut(tr("Ctrl+R"));
    m_pOptionsRecord->setShortcutContext(Qt::ApplicationShortcut);

#ifdef __SCRIPT__
    macroStudio = new QAction(tr("Show Studio"), this);
#endif

    m_pFileLoadSongPlayer1->setStatusTip(tr("Opens a song in player 1"));
    m_pFileLoadSongPlayer1->setWhatsThis(
        tr("Open\n\nOpens a song in player 1"));
    connect(m_pFileLoadSongPlayer1, SIGNAL(triggered()),
            this, SLOT(slotFileLoadSongPlayer1()));

    m_pFileLoadSongPlayer2->setStatusTip(tr("Opens a song in player 2"));
    m_pFileLoadSongPlayer2->setWhatsThis(
        tr("Open\n\nOpens a song in player 2"));
    connect(m_pFileLoadSongPlayer2, SIGNAL(triggered()),
            this, SLOT(slotFileLoadSongPlayer2()));

    m_pFileQuit->setStatusTip(tr("Quits the application"));
    m_pFileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
    connect(m_pFileQuit, SIGNAL(triggered()), this, SLOT(slotFileQuit()));

    m_pLibraryRescan->setStatusTip(tr("Rescans the song library"));
    m_pLibraryRescan->setWhatsThis(
        tr("Rescan library\n\nRescans the song library"));
    m_pLibraryRescan->setCheckable(false);
    connect(m_pLibraryRescan, SIGNAL(triggered()),
            this, SLOT(slotScanLibrary()));
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            this, SLOT(slotEnableRescanLibraryAction()));

    m_pPlaylistsNew->setStatusTip(tr("Create a new playlist"));
    m_pPlaylistsNew->setWhatsThis(tr("New playlist\n\nCreate a new playlist"));
    connect(m_pPlaylistsNew, SIGNAL(triggered()),
            m_pLibrary, SLOT(slotCreatePlaylist()));

    m_pCratesNew->setStatusTip(tr("Create a new crate"));
    m_pCratesNew->setWhatsThis(tr("New crate\n\nCreate a new crate."));
    connect(m_pCratesNew, SIGNAL(triggered()),
            m_pLibrary, SLOT(slotCreateCrate()));

    m_pPlaylistsImport->setStatusTip(tr("Import playlist"));
    m_pPlaylistsImport->setWhatsThis(tr("Import playlist"));
    //connect(playlistsImport, SIGNAL(triggered()),
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
    m_pOptionsVinylControl->setChecked(false);
    m_pOptionsVinylControl->setStatusTip(tr("Activate Vinyl Control"));
    m_pOptionsVinylControl->setWhatsThis(
        tr("Use timecoded vinyls on external turntables to control Mixxx"));
    connect(m_pOptionsVinylControl, SIGNAL(toggled(bool)), this,
        SLOT(slotCheckboxVinylControl(bool)));

   ControlObjectThreadMain* enabled1 = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Channel1]", "vinylcontrol_enabled")),this);
    connect(enabled1, SIGNAL(valueChanged(double)), this,
        SLOT(slotControlVinylControl(double)));

    m_pOptionsVinylControl2->setCheckable(true);
    m_pOptionsVinylControl2->setChecked(false);
    m_pOptionsVinylControl2->setStatusTip(tr("Activate Vinyl Control"));
    m_pOptionsVinylControl2->setWhatsThis(
        tr("Use timecoded vinyls on external turntables to control Mixxx"));
    connect(m_pOptionsVinylControl2, SIGNAL(toggled(bool)), this,
        SLOT(slotCheckboxVinylControl2(bool)));

    ControlObjectThreadMain* enabled2 = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Channel2]", "vinylcontrol_enabled")),this);
    connect(enabled2, SIGNAL(valueChanged(double)), this,
        SLOT(slotControlVinylControl2(double)));
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
    connect(m_pOptionsPreferences, SIGNAL(triggered()),
            this, SLOT(slotOptionsPreferences()));

    m_pHelpSupport->setStatusTip(tr("Support..."));
    m_pHelpSupport->setWhatsThis(tr("Support\n\nGet help with Mixxx"));
    connect(m_pHelpSupport, SIGNAL(triggered()), this, SLOT(slotHelpSupport()));

    m_pHelpFeedback->setStatusTip(tr("Send feedback to the Mixxx team."));
    m_pHelpFeedback->setWhatsThis(tr("Support\n\nSend feedback to the Mixxx team."));
    connect(m_pHelpFeedback, SIGNAL(triggered()), this, SLOT(slotHelpFeedback()));

    m_pHelpTranslation->setStatusTip(tr("Help translate this application into your language."));
    m_pHelpTranslation->setWhatsThis(tr("Support\n\nHelp translate this application into your language."));
    connect(m_pHelpTranslation, SIGNAL(triggered()), this, SLOT(slotHelpTranslation()));

    m_pHelpAboutApp->setStatusTip(tr("About the application"));
    m_pHelpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
    connect(m_pHelpAboutApp, SIGNAL(triggered()), this, SLOT(slotHelpAbout()));

#ifdef __SCRIPT__
    macroStudio->setStatusTip(tr("Shows the macro studio window"));
    macroStudio->setWhatsThis(
        tr("Show Studio\n\nMakes the macro studio visible"));
     connect(macroStudio, SIGNAL(triggered()),
             scriptEng->getStudio(), SLOT(showStudio()));
#endif
}

void MixxxApp::initMenuBar()
{
    // MENUBAR
   m_pFileMenu = new QMenu(tr("&File"), menuBar());
   m_pOptionsMenu = new QMenu(tr("&Options"), menuBar());
   m_pLibraryMenu = new QMenu(tr("&Library"),menuBar());
   m_pViewMenu = new QMenu(tr("&View"), menuBar());
   m_pHelpMenu = new QMenu(tr("&Help"), menuBar());
#ifdef __SCRIPT__
   macroMenu=new QMenu(tr("&Macro"), menuBar());
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
    m_pVinylControlMenu = new QMenu(tr("&Vinyl Control"), menuBar());
    m_pVinylControlMenu->addAction(m_pOptionsVinylControl);
    m_pVinylControlMenu->addAction(m_pOptionsVinylControl2);
    m_pOptionsMenu->addMenu(m_pVinylControlMenu);
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

    // menuBar entry viewMenu
    //viewMenu->setCheckable(true);

    // menuBar entry helpMenu
    m_pHelpMenu->addAction(m_pHelpSupport);
    m_pHelpMenu->addAction(m_pHelpFeedback);
    m_pHelpMenu->addAction(m_pHelpTranslation);
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

void MixxxApp::slotlibraryMenuAboutToShow(){
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
        m_pPlayerManager->slotLoadToDeck(s, 1);
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
        m_pPlayerManager->slotLoadToDeck(s, 2);
    }
}

void MixxxApp::slotFileQuit()
{
    if (!confirmExit()) {
        return;
    }
    hide();
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

    if (toggle) {
#if defined(__LINUX__) || defined(__APPLE__)
         // this and the later move(m_winpos) doesn't seem necessary
         // here on kwin, if it's necessary with some other x11 wm, re-enable
         // it, I guess -bkgood
         //m_winpos = pos();
         // fix some x11 silliness -- for some reason the move(m_winpos)
         // is moving the currentWindow to (0, 0), not the frame (as it's
         // supposed to, I might add)
         // if this messes stuff up on your distro yell at me -bkgood
         //m_winpos.setX(m_winpos.x() + (geometry().x() - x()));
         //m_winpos.setY(m_winpos.y() + (geometry().y() - y()));
#endif
        menuBar()-> setNativeMenuBar(false);
        showFullScreen();
    } else {
        menuBar()-> setNativeMenuBar(true);
        showNormal();
#ifdef __LINUX__
        //move(m_winpos);
#endif
    }
}

void MixxxApp::slotOptionsPreferences()
{
    m_pPrefDlg->setHidden(false);
    m_pPrefDlg->activateWindow();
}

void MixxxApp::slotControlVinylControl(double toggle)
{
#ifdef __VINYLCONTROL__
    if (m_pVCManager->vinylInputEnabled(1)) {
        m_pOptionsVinylControl->setChecked((bool)toggle);
    } else {
        m_pOptionsVinylControl->setChecked(false);
        if (toggle) {
            QMessageBox::warning(this, tr("Mixxx"),
                tr("No input device(s) select.\nPlease select your soundcard(s) "
                    "in the sound hardware preferences."),
                QMessageBox::Ok,
                QMessageBox::Ok);
            m_pPrefDlg->show();
            m_pPrefDlg->showSoundHardwarePage();
            ControlObject::getControl(ConfigKey("[Channel1]", "vinylcontrol_status"))->set(VINYL_STATUS_DISABLED);
            ControlObject::getControl(ConfigKey("[Channel1]", "vinylcontrol_enabled"))->set(0);
        }
    }
#endif
}

void MixxxApp::slotCheckboxVinylControl(bool toggle)
{
#ifdef __VINYLCONTROL__
    ControlObject::getControl(ConfigKey("[Channel1]", "vinylcontrol_enabled"))->set((double)toggle);
#endif
}

void MixxxApp::slotControlVinylControl2(double toggle)
{
#ifdef __VINYLCONTROL__
    if (m_pVCManager->vinylInputEnabled(2)) {
        m_pOptionsVinylControl2->setChecked((bool)toggle);
    } else {
        m_pOptionsVinylControl2->setChecked(false);
        if (toggle) {
            QMessageBox::warning(this, tr("Mixxx"),
                tr("No input device(s) select.\nPlease select your soundcard(s) "
                    "in the sound hardware preferences."),
                QMessageBox::Ok,
                QMessageBox::Ok);
            m_pPrefDlg->show();
            m_pPrefDlg->showSoundHardwarePage();
            ControlObject::getControl(ConfigKey("[Channel2]", "vinylcontrol_status"))->set(VINYL_STATUS_DISABLED);
            ControlObject::getControl(ConfigKey("[Channel2]", "vinylcontrol_enabled"))->set(0);
        }
    }
#endif
}

void MixxxApp::slotCheckboxVinylControl2(bool toggle)
{
#ifdef __VINYLCONTROL__
    ControlObject::getControl(ConfigKey("[Channel2]", "vinylcontrol_enabled"))->set((double)toggle);
#endif
}

//Also can't ifdef this (MOC again)
void MixxxApp::slotOptionsRecord(bool toggle)
{
    //Only start recording if checkbox was set to true and recording is inactive
    if(toggle && !m_pRecordingManager->isRecordingActive()) //start recording
        m_pRecordingManager->startRecording();
    //Only stop recording if checkbox was set to false and recording is active
    else if(!toggle && m_pRecordingManager->isRecordingActive())
        m_pRecordingManager->stopRecording();
}

void MixxxApp::slotHelpAbout()
{

    DlgAbout *about = new DlgAbout(this);
    about->version_label->setText(VERSION);
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
"Bill Good<br>"
"Owen Williams<br>"
"Vittorio Colao<br>"

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
"Andreas Pflug<br>"
"Bas van Schaik<br>"
"J&aacute;n Jockusch<br>"
"Oliver St&ouml;neberg<br>"
"Jan Jockusch<br>"
"C. Stewart<br>"
"Bill Egert<br>"
"Zach Shutters<br>"
"Owen Bullock<br>"
"Graeme Mathieson<br>"
"Sebastian Actist<br>"
"Jussi Sainio<br>"
"David Gnedt<br>"
"Antonio Passamani<br>"
"Guy Martin<br>"
"Anders Gunnarsson<br>"
"Alex Barker<br>"
"Mikko Jania<br>"
"Juan Pedro Bol&iacute;var Puente<br>"
"Linus Amvall<br>"
"Irwin C&eacute;spedes B<br>"
"Micz Flor<br>"
"Daniel James<br>"
"Mika Haulo<br>"
"Matthew Mikolay<br>"
"Tom Mast<br>"
"Miko Kiiski<br>"
"Vin&iacute;cius Dias dos Santos<br>"
"Joe Colosimo<br>"
"Shashank Kumar<br>"
"Till Hofmann<br>"
"Daniel Sch&uuml;rmann<br>"
"Peter V&aacute;gner<br>"
"Thanasis Liappis<br>"
"Jens Nachtigall<br>"

"</p>"
"<p align=\"center\"><b>And special thanks to:</b></p>"
"<p align=\"center\">"
"Vestax<br>"
"Stanton<br>"
"Hercules<br>"
"EKS<br>"
"Echo Digital Audio<br>"
"JP Disco<br>"
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
"Bruno Buccolo<br>"
"Ryan Baker<br>"
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
    "</p>").arg(VERSION);

    about->textBrowser->setHtml(credits);
    about->show();

}

void MixxxApp::slotHelpSupport() {
    QUrl qSupportURL;
    qSupportURL.setUrl(MIXXX_SUPPORT_URL);
    QDesktopServices::openUrl(qSupportURL);
}

void MixxxApp::slotHelpFeedback() {
    QUrl qFeedbackUrl;
    qFeedbackUrl.setUrl(MIXXX_FEEDBACK_URL);
    QDesktopServices::openUrl(qFeedbackUrl);
}

void MixxxApp::slotHelpTranslation() {
    QUrl qTranslationUrl;
    qTranslationUrl.setUrl(MIXXX_TRANSLATION_URL);
    QDesktopServices::openUrl(qTranslationUrl);
}

void MixxxApp::rebootMixxxView() {

    if (!m_pWidgetParent || !m_pView)
        return;

    qDebug() << "Now in Rebootmixxview...";

    // Workaround for changing skins while fullscreen, just go out of fullscreen
    // mode. If you change skins while in fullscreen (on Linux, at least) the
    // window returns to 0,0 but and the backdrop disappears so it looks as if
    // it is not fullscreen, but acts as if it is.
    slotOptionsFullScreen(false);

    // TODO(XXX) Make getSkinPath not public
    QString qSkinPath = m_pSkinLoader->getConfiguredSkinPath();

    m_pView->hide();
    delete m_pView;
    m_pView = new QFrame();

    // assignment in next line intentional
    if (!(m_pWidgetParent = m_pSkinLoader->loadDefaultSkin(m_pView,
                                        m_pKeyboard,
                                        m_pPlayerManager,
                                        m_pLibrary,
                                        m_pVCManager))) {
        qDebug() << "Could not reload the skin.";
    }

    // don't move this before loadDefaultSkin above. bug 521509 --bkgood
    setCentralWidget(m_pView);

    // keep gui centered (esp for fullscreen)
    // the layout will be deleted whenever m_pView gets deleted
    QHBoxLayout *pLayout = new QHBoxLayout(m_pView);
    pLayout->addWidget(m_pWidgetParent);
    pLayout->setContentsMargins(0, 0, 0, 0); // don't want margins

    // if we move from big skin to smaller skin, size the window down to fit
    // (qt scales up for us if we go the other way) -bkgood
    // this doesn't always seem to snap down tight on Windows... sigh -bkgood
    setFixedSize(m_pView->width(), m_pView->height());
    setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    qDebug() << "rebootgui DONE";
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

void MixxxApp::closeEvent(QCloseEvent *event) {
    if (!confirmExit()) {
        event->ignore();
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
    // Check recording if it is active.
    m_pOptionsRecord->setChecked(m_pRecordingManager->isRecordingActive());

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

void MixxxApp::checkDirectRendering() {
    // IF
    //  * A waveform viewer exists
    // AND
    //  * The waveform viewer is an OpenGL waveform viewer
    // AND
    //  * The waveform viewer does not have direct rendering enabled.
    // THEN
    //  * Warn user

    if (WaveformViewerFactory::numViewers(WAVEFORM_GL) > 0 &&
        !WaveformViewerFactory::isDirectRenderingEnabled() &&
        m_pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes")) {
		    QMessageBox::warning(0, "OpenGL Direct Rendering",
                             "Direct rendering is not enabled on your machine.\n\nThis means that the waveform displays will be very\nslow and take a lot of CPU time. Either update your\nconfiguration to enable direct rendering, or disable\nthe waveform displays in the control panel by\nselecting \"Simple\" under waveform displays.\nNOTE: In case you run on NVidia hardware,\ndirect rendering may not be present, but you will\nnot experience a degradation in performance.");
        m_pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), ConfigValue(QString("yes")));
    }
}

bool MixxxApp::confirmExit() {
    bool playing(false);
    unsigned int deckCount = m_pPlayerManager->numDecks();
    for (unsigned int i = 0; i < deckCount; ++i) {
        ControlObject *pPlayCO(
            ControlObject::getControl(
                ConfigKey(QString("[Channel%1]").arg(i + 1), "play")
            )
        );
        if (pPlayCO && pPlayCO->get()) {
            playing = true;
            break;
        }
    }
    if (playing) {
        QMessageBox::StandardButton btn = QMessageBox::question(this,
            tr("Confirm Exit"),
            tr("A deck is currently playing. Exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
    }
    return true;
}
