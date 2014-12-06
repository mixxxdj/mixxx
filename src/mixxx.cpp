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
#include <QTranslator>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>

#include "mixxx.h"

#include "analyserqueue.h"
#include "controlpotmeter.h"
#include "controlobjectslave.h"
#include "deck.h"
#include "defs_urls.h"
#include "dlgabout.h"
#include "dlgpreferences.h"
#include "dlgprefeq.h"
#include "dlgdevelopertools.h"
#include "engine/enginemaster.h"
#include "engine/enginemicrophone.h"
#include "effects/effectsmanager.h"
#include "effects/native/nativebackend.h"
#include "engine/engineaux.h"
#include "library/coverartcache.h"
#include "library/library.h"
#include "library/library_preferences.h"
#include "library/libraryscanner.h"
#include "library/librarytablemodel.h"
#include "controllers/controllermanager.h"
#include "mixxxkeyboard.h"
#include "playermanager.h"
#include "recording/defs_recording.h"
#include "recording/recordingmanager.h"
#include "shoutcast/shoutcastmanager.h"
#include "skin/legacyskinparser.h"
#include "skin/skinloader.h"
#include "soundmanager.h"
#include "soundmanagerutil.h"
#include "soundsourceproxy.h"
#include "trackinfoobject.h"
#include "upgrade.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wwaveformviewer.h"
#include "widget/wwidget.h"
#include "widget/wspinny.h"
#include "sharedglcontext.h"
#include "util/debug.h"
#include "util/statsmanager.h"
#include "util/timer.h"
#include "util/time.h"
#include "util/version.h"
#include "controlpushbutton.h"
#include "util/compatibility.h"
#include "util/sandbox.h"
#include "playerinfo.h"
#include "waveform/guitick.h"
#include "util/math.h"
#include "util/experiment.h"

#ifdef __VINYLCONTROL__
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#endif

#ifdef __MODPLUG__
#include "dlgprefmodplug.h"
#endif

// static
const int MixxxMainWindow::kMicrophoneCount = 4;
// static
const int MixxxMainWindow::kAuxiliaryCount = 4;

MixxxMainWindow::MixxxMainWindow(QApplication* pApp, const CmdlineArgs& args)
        : m_pWidgetParent(NULL),
          m_pDeveloperToolsDlg(NULL),
          m_runtime_timer("MixxxMainWindow::runtime"),
          m_cmdLineArgs(args),
          m_iNumConfiguredDecks(0) {
    // We use QSet<int> in signals in the library.
    qRegisterMetaType<QSet<int> >("QSet<int>");

    logBuildDetails();
    ScopedTimer t("MixxxMainWindow::MixxxMainWindow");
    m_runtime_timer.start();
    Time::start();
    initializeWindow();

    //Reset pointer to players
    m_pSoundManager = NULL;
    m_pPrefDlg = NULL;
    m_pControllerManager = NULL;
    m_pRecordingManager = NULL;
#ifdef __SHOUTCAST__
    m_pShoutcastManager = NULL;
#endif

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pConfig = upgrader.versionUpgrade(args.getSettingsPath());
    ControlDoublePrivate::setUserConfig(m_pConfig);

    Sandbox::initialize(m_pConfig->getSettingsPath().append("/sandbox.cfg"));

    // Only record stats in developer mode.
    if (m_cmdLineArgs.getDeveloper()) {
        StatsManager::create();
    }

    QString resourcePath = m_pConfig->getResourcePath();
    initializeTranslations(pApp);

    // Set the visibility of tooltips, default "1" = ON
    m_toolTipsCfg = m_pConfig->getValueString(ConfigKey("[Controls]", "Tooltips"), "1").toInt();

    // Store the path in the config database
    m_pConfig->set(ConfigKey("[Config]", "Path"), ConfigValue(resourcePath));

    setAttribute(Qt::WA_AcceptTouchEvents);
    m_pTouchShift = new ControlPushButton(ConfigKey("[Controls]", "touch_shift"));

    // Create the Effects subsystem.
    m_pEffectsManager = new EffectsManager(this, m_pConfig);

    // Starting the master (mixing of the channels and effects):
    m_pEngine = new EngineMaster(m_pConfig, "[Master]", m_pEffectsManager, true, true);

    // Create effect backends. We do this after creating EngineMaster to allow
    // effect backends to refer to controls that are produced by the engine.
    NativeBackend* pNativeBackend = new NativeBackend(m_pEffectsManager);
    m_pEffectsManager->addEffectsBackend(pNativeBackend);

    // Sets up the default EffectChains and EffectRacks
    m_pEffectsManager->setupDefaults();

    m_pRecordingManager = new RecordingManager(m_pConfig, m_pEngine);
#ifdef __SHOUTCAST__
    m_pShoutcastManager = new ShoutcastManager(m_pConfig, m_pEngine);
#endif

    // Initialize player device
    // while this is created here, setupDevices needs to be called sometime
    // after the players are added to the engine (as is done currently) -- bkgood
    m_pSoundManager = new SoundManager(m_pConfig, m_pEngine);

    // TODO(rryan): Fold microphone and aux creation into a manager
    // (e.g. PlayerManager, though they aren't players).

    ControlObject* pNumMicrophones = new ControlObject(ConfigKey("[Master]", "num_microphones"));
    pNumMicrophones->setParent(this);

    for (int i = 0; i < kMicrophoneCount; ++i) {
        QString group("[Microphone]");
        if (i > 0) {
            group = QString("[Microphone%1]").arg(i + 1);
        }
        EngineMicrophone* pMicrophone =
                new EngineMicrophone(group, m_pEffectsManager);
        // What should channelbase be?
        AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 0, i);
        m_pEngine->addChannel(pMicrophone);
        m_pSoundManager->registerInput(micInput, pMicrophone);
        pNumMicrophones->set(pNumMicrophones->get() + 1);
    }

    ControlObject* pNumAuxiliaries = new ControlObject(ConfigKey("[Master]", "num_auxiliaries"));
    pNumAuxiliaries->setParent(this);

    m_PassthroughMapper = new QSignalMapper(this);
    connect(m_PassthroughMapper, SIGNAL(mapped(int)),
            this, SLOT(slotControlPassthrough(int)));

    m_AuxiliaryMapper = new QSignalMapper(this);
    connect(m_AuxiliaryMapper, SIGNAL(mapped(int)),
            this, SLOT(slotControlAuxiliary(int)));

    for (int i = 0; i < kAuxiliaryCount; ++i) {
        QString group = QString("[Auxiliary%1]").arg(i + 1);
        EngineAux* pAux = new EngineAux(strdup(group.toStdString().c_str()),
                                        m_pEffectsManager);
        // What should channelbase be?
        AudioInput auxInput = AudioInput(AudioPath::AUXILIARY, 0, 0, i);
        m_pEngine->addChannel(pAux);
        m_pSoundManager->registerInput(auxInput, pAux);
        pNumAuxiliaries->set(pNumAuxiliaries->get() + 1);

        m_pAuxiliaryPassthrough.push_back(
                new ControlObjectSlave(group, "passthrough"));
        ControlObjectSlave* auxiliary_passthrough =
                m_pAuxiliaryPassthrough.back();

        // These non-vinyl passthrough COs have their index offset by the max
        // number of vinyl inputs.
        m_AuxiliaryMapper->setMapping(auxiliary_passthrough, i);
        auxiliary_passthrough->connectValueChanged(m_AuxiliaryMapper,
                                                   SLOT(map()));
    }

    // Do not write meta data back to ID3 when meta data has changed
    // Because multiple TrackDao objects can exists for a particular track
    // writing meta data may ruin your MP3 file if done simultaneously.
    // see Bug #728197
    // For safety reasons, we deactivate this feature.
    m_pConfig->set(ConfigKey("[Library]","WriteAudioTags"), ConfigValue(0));

    // library dies in seemingly unrelated qtsql error about not having a
    // sqlite driver if this path doesn't exist. Normally config->Save()
    // above would make it but if it doesn't get run for whatever reason
    // we get hosed -- bkgood
    if (!QDir(args.getSettingsPath()).exists()) {
        QDir().mkpath(args.getSettingsPath());
    }

    // Register TrackPointer as a metatype since we use it in signals/slots
    // regularly.
    qRegisterMetaType<TrackPointer>("TrackPointer");

    m_pGuiTick = new GuiTick();

#ifdef __VINYLCONTROL__
    m_pVCManager = new VinylControlManager(this, m_pConfig, m_pSoundManager);
#else
    m_pVCManager = NULL;
#endif

    // Create the player manager.
    m_pPlayerManager = new PlayerManager(m_pConfig, m_pSoundManager,
                                         m_pEffectsManager, m_pEngine);
    m_pPlayerManager->addConfiguredDecks();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addSampler();
    m_pPlayerManager->addPreviewDeck();

#ifdef __VINYLCONTROL__
    m_pVCManager->init();
#endif

    m_pNumDecks = new ControlObjectThread(ConfigKey("[Master]", "num_decks"),
                                          this);
    connect(m_pNumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksChanged(double)));

#ifdef __MODPLUG__
    // restore the configuration for the modplug library before trying to load a module
    DlgPrefModplug* pModplugPrefs = new DlgPrefModplug(0, m_pConfig);
    pModplugPrefs->loadSettings();
    pModplugPrefs->applySettings();
    delete pModplugPrefs; // not needed anymore
#endif

    CoverArtCache::create();

    m_pLibrary = new Library(this, m_pConfig,
                             m_pPlayerManager,
                             m_pRecordingManager);
    m_pPlayerManager->bindToLibrary(m_pLibrary);

    // Get Music dir
    bool hasChanged_MusicDir = false;

    QStringList dirs = m_pLibrary->getDirs();
    if (dirs.size() < 1) {
        // TODO(XXX) this needs to be smarter, we can't distinguish between an empty
        // path return value (not sure if this is normally possible, but it is
        // possible with the Windows 7 "Music" library, which is what
        // QDesktopServices::storageLocation(QDesktopServices::MusicLocation)
        // resolves to) and a user hitting 'cancel'. If we get a blank return
        // but the user didn't hit cancel, we need to know this and let the
        // user take some course of action -- bkgood
        QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose music library directory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
        if (!fd.isEmpty()) {
            // adds Folder to database.
            m_pLibrary->slotRequestAddDir(fd);
            hasChanged_MusicDir = true;
        }
    }

    // Call inits to invoke all other construction parts

    // Intialize default BPM system values
    if (m_pConfig->getValueString(ConfigKey("[BPM]", "BPMRangeStart"))
            .length() < 1) {
        m_pConfig->set(ConfigKey("[BPM]", "BPMRangeStart"),ConfigValue(65));
    }

    if (m_pConfig->getValueString(ConfigKey("[BPM]", "BPMRangeEnd"))
            .length() < 1) {
        m_pConfig->set(ConfigKey("[BPM]", "BPMRangeEnd"),ConfigValue(135));
    }

    if (m_pConfig->getValueString(ConfigKey("[BPM]", "AnalyzeEntireSong"))
            .length() < 1) {
        m_pConfig->set(ConfigKey("[BPM]", "AnalyzeEntireSong"),ConfigValue(1));
    }

    // Initialize controller sub-system,
    //  but do not set up controllers until the end of the application startup
    qDebug() << "Creating ControllerManager";
    m_pControllerManager = new ControllerManager(m_pConfig);

    WaveformWidgetFactory::create();
    WaveformWidgetFactory::instance()->startVSync(this);
    WaveformWidgetFactory::instance()->setConfig(m_pConfig);

    m_pSkinLoader = new SkinLoader(m_pConfig);
    connect(this, SIGNAL(newSkinLoaded()),
            this, SLOT(onNewSkinLoaded()));
    connect(this, SIGNAL(newSkinLoaded()),
            m_pLibrary, SLOT(onSkinLoadFinished()));

    // Initialize preference dialog
    m_pPrefDlg = new DlgPreferences(this, m_pSkinLoader, m_pSoundManager, m_pPlayerManager,
                                    m_pControllerManager, m_pVCManager, m_pEffectsManager,
                                    m_pConfig, m_pLibrary);
    m_pPrefDlg->setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));
    m_pPrefDlg->setHidden(true);

    initializeKeyboard();
    initActions();
    initMenuBar();

    // Before creating the first skin we need to create a QGLWidget so that all
    // the QGLWidget's we create can use it as a shared QGLContext.
    QGLWidget* pContextWidget = new QGLWidget(this);
    pContextWidget->hide();
    SharedGLContext::setWidget(pContextWidget);

    // Load skin to a QWidget that we set as the central widget. Assignment
    // intentional in next line.
    if (!(m_pWidgetParent = m_pSkinLoader->loadDefaultSkin(this, m_pKeyboard,
                                                           m_pPlayerManager,
                                                           m_pControllerManager,
                                                           m_pLibrary,
                                                           m_pVCManager,
                                                           m_pEffectsManager))) {
        reportCriticalErrorAndQuit(
            "default skin cannot be loaded see <b>mixxx</b> trace for more information.");

        //TODO (XXX) add dialog to warn user and launch skin choice page
        resize(640,480);
    } else {
        // this has to be after the OpenGL widgets are created or depending on a
        // million different variables the first waveform may be horribly
        // corrupted. See bug 521509 -- bkgood ?? -- vrince
        setCentralWidget(m_pWidgetParent);
    }

    //move the app in the center of the primary screen
    slotToCenterOfPrimaryScreen();

    // Check direct rendering and warn user if they don't have it
    checkDirectRendering();

    //Install an event filter to catch certain QT events, such as tooltips.
    //This allows us to turn off tooltips.
    pApp->installEventFilter(this); // The eventfilter is located in this
                                    // Mixxx class as a callback.

    // If we were told to start in fullscreen mode on the command-line or if
    // user chose always starts in fullscreen mode, then turn on fullscreen
    // mode.
    bool fullscreenPref = m_pConfig->getValueString(
        ConfigKey("[Config]", "StartInFullscreen"), "0").toInt();
    if (args.getStartInFullscreen() || fullscreenPref) {
        slotViewFullScreen(true);
    }
    emit(newSkinLoaded());

    // Wait until all other ControlObjects are set up before initializing
    // controllers
    m_pControllerManager->setUpDevices();

    // Scan the library for new files and directories
    bool rescan = m_pConfig->getValueString(
        ConfigKey("[Library]","RescanOnStartup")).toInt();
    // rescan the library if we get a new plugin
    QSet<QString> prev_plugins = QSet<QString>::fromList(
        m_pConfig->getValueString(
            ConfigKey("[Library]", "SupportedFileExtensions")).split(
                ",", QString::SkipEmptyParts));
    QSet<QString> curr_plugins = QSet<QString>::fromList(
        SoundSourceProxy::supportedFileExtensions());
    rescan = rescan || (prev_plugins != curr_plugins);
    m_pConfig->set(ConfigKey("[Library]", "SupportedFileExtensions"),
        QStringList(SoundSourceProxy::supportedFileExtensions()).join(","));

    // Scan the library directory. Initialize this after the skinloader has
    // loaded a skin, see Bug #1047435
    m_pLibraryScanner = new LibraryScanner(this, m_pLibrary->getTrackCollection());
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            this, SLOT(slotEnableRescanLibraryAction()));

    // Refresh the library models when the library (re)scan is finished.
    connect(m_pLibraryScanner, SIGNAL(scanFinished()),
            m_pLibrary, SLOT(slotRefreshLibraryModels()));

    if (rescan || hasChanged_MusicDir || upgrader.rescanLibrary()) {
        m_pLibraryScanner->scan();
    }
    slotNumDecksChanged(m_pNumDecks->get());

    // Try open player device If that fails, the preference panel is opened.
    int setupDevices = m_pSoundManager->setupDevices();
    unsigned int numDevices = m_pSoundManager->getConfig().getOutputs().count();
    // test for at least one out device, if none, display another dlg that
    // says "mixxx will barely work with no outs"
    while (setupDevices != OK || numDevices == 0) {
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

    // Load tracks in args.qlMusicFiles (command line arguments) into player
    // 1 and 2:
    const QList<QString>& musicFiles = args.getMusicFiles();
    for (int i = 0; i < (int)m_pPlayerManager->numDecks()
            && i < musicFiles.count(); ++i) {
        if (SoundSourceProxy::isFilenameSupported(musicFiles.at(i))) {
            m_pPlayerManager->slotLoadToDeck(musicFiles.at(i), i+1);
        }
    }
}

MixxxMainWindow::~MixxxMainWindow() {
    // TODO(rryan): Get rid of QTime here.
    QTime qTime;
    qTime.start();
    Timer t("MixxxMainWindow::~MixxxMainWindow");
    t.start();

    qDebug() << "Destroying MixxxMainWindow";

    qDebug() << "save config " << qTime.elapsed();
    m_pConfig->Save();

    // SoundManager depend on Engine and Config
    qDebug() << "delete soundmanager " << qTime.elapsed();
    delete m_pSoundManager;

    // GUI depends on MixxxKeyboard, PlayerManager, Library
    qDebug() << "delete view " << qTime.elapsed();
    delete m_pWidgetParent;

    // SkinLoader depends on Config
    qDebug() << "delete SkinLoader " << qTime.elapsed();
    delete m_pSkinLoader;

    // ControllerManager depends on Config
    qDebug() << "delete ControllerManager " << qTime.elapsed();
    delete m_pControllerManager;

#ifdef __VINYLCONTROL__
    // VinylControlManager depends on a CO the engine owns
    // (vinylcontrol_enabled in VinylControlControl)
    qDebug() << "delete vinylcontrolmanager " << qTime.elapsed();
    delete m_pVCManager;
    qDeleteAll(m_pVinylControlEnabled);
    delete m_VCControlMapper;
    delete m_VCCheckboxMapper;
#endif
    delete m_PassthroughMapper;
    delete m_AuxiliaryMapper;
    delete m_TalkoverMapper;

    // LibraryScanner depends on Library
    qDebug() << "delete library scanner " <<  qTime.elapsed();
    delete m_pLibraryScanner;

    // CoverArtCache is fairly independent of everything else.
    CoverArtCache::destroy();

    // Delete the library after the view so there are no dangling pointers to
    // the data models.
    // Depends on RecordingManager and PlayerManager
    qDebug() << "delete library " << qTime.elapsed();
    delete m_pLibrary;

    // PlayerManager depends on Engine, SoundManager, VinylControlManager, and Config
    qDebug() << "delete playerManager " << qTime.elapsed();
    delete m_pPlayerManager;

    // RecordingManager depends on config, engine
    qDebug() << "delete RecordingManager " << qTime.elapsed();
    delete m_pRecordingManager;

#ifdef __SHOUTCAST__
    // ShoutcastManager depends on config, engine
    qDebug() << "delete ShoutcastManager " << qTime.elapsed();
    delete m_pShoutcastManager;
#endif

    // Delete ControlObjectSlaves we created for checking passthrough and
    // talkover status.
    qDeleteAll(m_pAuxiliaryPassthrough);
    qDeleteAll(m_pPassthroughEnabled);
    qDeleteAll(m_micTalkoverControls);

    // EngineMaster depends on Config and m_pEffectsManager.
    qDebug() << "delete m_pEngine " << qTime.elapsed();
    delete m_pEngine;

    qDebug() << "deleting preferences, " << qTime.elapsed();
    delete m_pPrefDlg;

    // Must delete after EngineMaster and DlgPrefEq.
    qDebug() << "deleting effects manager, " << qTime.elapsed();
    delete m_pEffectsManager;

    delete m_pTouchShift;

    PlayerInfo::destroy();
    WaveformWidgetFactory::destroy();

    delete m_pGuiTick;

    // Check for leaked ControlObjects and give warnings.
    QList<QSharedPointer<ControlDoublePrivate> > leakedControls;
    QList<ConfigKey> leakedConfigKeys;

    ControlDoublePrivate::getControls(&leakedControls);

    if (leakedControls.size() > 0) {
        qDebug() << "WARNING: The following" << leakedControls.size()
                 << "controls were leaked:";
        foreach (QSharedPointer<ControlDoublePrivate> pCDP, leakedControls) {
            if (pCDP.isNull()) {
                continue;
            }
            ConfigKey key = pCDP->getKey();
            qDebug() << key.group << key.item << pCDP->getCreatorCO();
            leakedConfigKeys.append(key);
        }

       foreach (ConfigKey key, leakedConfigKeys) {
           // delete just to satisfy valgrind:
           // check if the pointer is still valid, the control object may have bin already
           // deleted by its parent in this loop
           ControlObject* pCo = ControlObject::getControl(key, false);
           if (pCo) {
               // it might happens that a control is deleted as child from an other control
               delete pCo;
           }
       }
    }
    qDebug() << "~MixxxMainWindow: All leaking controls deleted.";

    // HACK: Save config again. We saved it once before doing some dangerous
    // stuff. We only really want to save it here, but the first one was just
    // a precaution. The earlier one can be removed when stuff is more stable
    // at exit.
    m_pConfig->Save();

    qDebug() << "delete config " << qTime.elapsed();
    Sandbox::shutdown();

    ControlDoublePrivate::setUserConfig(NULL);
    delete m_pConfig;

    delete m_pKeyboard;
    delete m_pKbdConfig;
    delete m_pKbdConfigEmpty;

    t.elapsed(true);
    // Report the total time we have been running.
    m_runtime_timer.elapsed(true);
    StatsManager::destroy();
}

bool MixxxMainWindow::loadTranslations(const QLocale& systemLocale, QString userLocale,
                      const QString& translation, const QString& prefix,
                      const QString& translationPath, QTranslator* pTranslator) {
    if (userLocale.size() == 0) {
#if QT_VERSION >= 0x040800
        QStringList uiLanguages = systemLocale.uiLanguages();
        if (uiLanguages.size() > 0 && uiLanguages.first() == "en") {
            // Don't bother loading a translation if the first ui-langauge is
            // English because the interface is already in English. This fixes
            // the case where the user's install of Qt doesn't have an explicit
            // English translation file and the fact that we don't ship a
            // mixxx_en.qm.
            return false;
        }
        return pTranslator->load(systemLocale, translation, prefix, translationPath);
#else
        userLocale = systemLocale.name();
#endif  // QT_VERSION
    }
    return pTranslator->load(translation + prefix + userLocale, translationPath);
}

void MixxxMainWindow::logBuildDetails() {
    QString version = Version::version();
    QString buildBranch = Version::developmentBranch();
    QString buildRevision = Version::developmentRevision();
    QString buildFlags = Version::buildFlags();

    QStringList buildInfo;
    if (!buildBranch.isEmpty() && !buildRevision.isEmpty()) {
        buildInfo.append(
            QString("git %1 r%2").arg(buildBranch, buildRevision));
    } else if (!buildRevision.isEmpty()) {
        buildInfo.append(
            QString("git r%2").arg(buildRevision));
    }
    buildInfo.append("built on: " __DATE__ " @ " __TIME__);
    if (!buildFlags.isEmpty()) {
        buildInfo.append(QString("flags: %1").arg(buildFlags.trimmed()));
    }
    QString buildInfoFormatted = QString("(%1)").arg(buildInfo.join("; "));

    // This is the first line in mixxx.log
    qDebug() << "Mixxx" << version << buildInfoFormatted << "is starting...";
    qDebug() << "Qt version is:" << qVersion();

    qDebug() << "QDesktopServices::storageLocation(HomeLocation):"
             << QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    qDebug() << "QDesktopServices::storageLocation(DataLocation):"
             << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    qDebug() << "QCoreApplication::applicationDirPath()"
             << QCoreApplication::applicationDirPath();
}

void MixxxMainWindow::initializeWindow() {
    QString version = Version::version();
#ifdef __APPLE__
    setWindowTitle(tr("Mixxx")); //App Store
#elif defined(AMD64) || defined(EM64T) || defined(x86_64)
    setWindowTitle(tr("Mixxx %1 x64").arg(version));
#elif defined(IA64)
    setWindowTitle(tr("Mixxx %1 Itanium").arg(version));
#else
    setWindowTitle(tr("Mixxx %1").arg(version));
#endif
    setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));
}

void MixxxMainWindow::initializeTranslations(QApplication* pApp) {
    QString resourcePath = m_pConfig->getResourcePath();
    QString translationsFolder = resourcePath + "translations/";

    // Load Qt base translations
    QString userLocale = m_cmdLineArgs.getLocale();
    QLocale systemLocale = QLocale::system();

    // Attempt to load user locale from config
    if (userLocale.isEmpty()) {
        userLocale = m_pConfig->getValueString(ConfigKey("[Config]","Locale"));
    }

    if (userLocale.isEmpty()) {
        QLocale::setDefault(QLocale(systemLocale));
    } else {
        QLocale::setDefault(QLocale(userLocale));
    }

    // source language
    if (userLocale == "en_US") {
        return;
    }

    // Load Qt translations for this locale from the system translation
    // path. This is the lowest precedence QTranslator.
    QTranslator* qtTranslator = new QTranslator(pApp);
    if (loadTranslations(systemLocale, userLocale, "qt", "_",
                         QLibraryInfo::location(QLibraryInfo::TranslationsPath),
                         qtTranslator)) {
        pApp->installTranslator(qtTranslator);
    } else {
        delete qtTranslator;
    }

    // Load Qt translations for this locale from the Mixxx translations
    // folder.
    QTranslator* mixxxQtTranslator = new QTranslator(pApp);
    if (loadTranslations(systemLocale, userLocale, "qt", "_",
                         translationsFolder,
                         mixxxQtTranslator)) {
        pApp->installTranslator(mixxxQtTranslator);
    } else {
        delete mixxxQtTranslator;
    }

    // Load Mixxx specific translations for this locale from the Mixxx
    // translations folder.
    QTranslator* mixxxTranslator = new QTranslator(pApp);
    bool mixxxLoaded = loadTranslations(systemLocale, userLocale, "mixxx", "_",
                                        translationsFolder, mixxxTranslator);
    qDebug() << "Loading translations for locale"
             << (userLocale.size() > 0 ? userLocale : systemLocale.name())
             << "from translations folder" << translationsFolder << ":"
             << (mixxxLoaded ? "success" : "fail");
    if (mixxxLoaded) {
        pApp->installTranslator(mixxxTranslator);
    } else {
        delete mixxxTranslator;
    }
}

void MixxxMainWindow::initializeKeyboard() {
    QString resourcePath = m_pConfig->getResourcePath();

    // Set the default value in settings file
    if (m_pConfig->getValueString(ConfigKey("[Keyboard]","Enabled")).length() == 0)
        m_pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(1));

    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString userKeyboard = m_cmdLineArgs.getSettingsPath() + "Custom.kbd.cfg";

    //Empty keyboard configuration
    m_pKbdConfigEmpty = new ConfigObject<ConfigValueKbd>("");

    if (QFile::exists(userKeyboard)) {
        qDebug() << "Found and will use custom keyboard preset" << userKeyboard;
        m_pKbdConfig = new ConfigObject<ConfigValueKbd>(userKeyboard);
    } else {
        // Default to the locale for the main input method (e.g. keyboard).
        QLocale locale = inputLocale();

        // check if a default keyboard exists
        QString defaultKeyboard = QString(resourcePath).append("keyboard/");
        defaultKeyboard += locale.name();
        defaultKeyboard += ".kbd.cfg";

        if (!QFile::exists(defaultKeyboard)) {
            qDebug() << defaultKeyboard << " not found, using en_US.kbd.cfg";
            defaultKeyboard = QString(resourcePath).append("keyboard/").append("en_US.kbd.cfg");
            if (!QFile::exists(defaultKeyboard)) {
                qDebug() << defaultKeyboard << " not found, starting without shortcuts";
                defaultKeyboard = "";
            }
        }
        m_pKbdConfig = new ConfigObject<ConfigValueKbd>(defaultKeyboard);
    }

    // TODO(XXX) leak pKbdConfig, MixxxKeyboard owns it? Maybe roll all keyboard
    // initialization into MixxxKeyboard
    // Workaround for today: MixxxKeyboard calls delete
    bool keyboardShortcutsEnabled = m_pConfig->getValueString(
        ConfigKey("[Keyboard]", "Enabled")) == "1";
    m_pKeyboard = new MixxxKeyboard(keyboardShortcutsEnabled ? m_pKbdConfig : m_pKbdConfigEmpty);
}

void toggleVisibility(ConfigKey key, bool enable) {
    qDebug() << "Setting visibility for" << key.group << key.item << enable;
    ControlObject::set(key, enable ? 1.0 : 0.0);
}

void MixxxMainWindow::slotViewShowSamplers(bool enable) {
    toggleVisibility(ConfigKey("[Samplers]", "show_samplers"), enable);
}

void MixxxMainWindow::slotViewShowVinylControl(bool enable) {
    toggleVisibility(ConfigKey(VINYL_PREF_KEY, "show_vinylcontrol"), enable);
}

void MixxxMainWindow::slotViewShowMicrophone(bool enable) {
    toggleVisibility(ConfigKey("[Microphone]", "show_microphone"), enable);
}

void MixxxMainWindow::slotViewShowPreviewDeck(bool enable) {
    toggleVisibility(ConfigKey("[PreviewDeck]", "show_previewdeck"), enable);
}

void MixxxMainWindow::slotViewShowCoverArt(bool enable) {
    toggleVisibility(ConfigKey("[Library]", "show_coverart"), enable);
}

void setVisibilityOptionState(QAction* pAction, ConfigKey key) {
    ControlObject* pVisibilityControl = ControlObject::getControl(key);
    pAction->setEnabled(pVisibilityControl != NULL);
    pAction->setChecked(pVisibilityControl != NULL ? pVisibilityControl->get() > 0.0 : false);
}

void MixxxMainWindow::onNewSkinLoaded() {
#ifdef __VINYLCONTROL__
    setVisibilityOptionState(m_pViewVinylControl,
                             ConfigKey(VINYL_PREF_KEY, "show_vinylcontrol"));
#endif
    setVisibilityOptionState(m_pViewShowSamplers,
                             ConfigKey("[Samplers]", "show_samplers"));
    setVisibilityOptionState(m_pViewShowMicrophone,
                             ConfigKey("[Microphone]", "show_microphone"));
    setVisibilityOptionState(m_pViewShowPreviewDeck,
                             ConfigKey("[PreviewDeck]", "show_previewdeck"));
    setVisibilityOptionState(m_pViewShowCoverArt,
                             ConfigKey("[Library]", "show_coverart"));
}

int MixxxMainWindow::noSoundDlg(void)
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
                "#no_or_too_few_sound_cards_appear_in_the_preferences_dialog"));
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

int MixxxMainWindow::noOutputDlg(bool *continueClicked)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("No Output Devices"));
    msgBox.setText( "<html>" + tr("Mixxx was configured without any output sound devices. "
                    "Audio processing will be disabled without a configured output device.") +
                    "<ul>"
                        "<li>" +
                            tr("<b>Continue</b> without any outputs.") +
                        "</li>"
                        "<li>" +
                            tr("<b>Reconfigure</b> Mixxx's sound device settings.") +
                        "</li>"
                        "<li>" +
                            tr("<b>Exit</b> Mixxx.") +
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

QString buildWhatsThis(const QString& title, const QString& text) {
    QString preparedTitle = title;
    return QString("%1\n\n%2").arg(preparedTitle.replace("&", ""), text);
}

// initializes all QActions of the application
void MixxxMainWindow::initActions()
{
    QString loadTrackText = tr("Load Track to Deck %1");
    QString loadTrackStatusText = tr("Loads a track in deck %1");
    QString openText = tr("Open");

    QString player1LoadStatusText = loadTrackStatusText.arg(QString::number(1));
    m_pFileLoadSongPlayer1 = new QAction(loadTrackText.arg(QString::number(1)), this);
    m_pFileLoadSongPlayer1->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "FileMenu_LoadDeck1"),
                                                  tr("Ctrl+o"))));
    m_pFileLoadSongPlayer1->setShortcutContext(Qt::ApplicationShortcut);
    m_pFileLoadSongPlayer1->setStatusTip(player1LoadStatusText);
    m_pFileLoadSongPlayer1->setWhatsThis(
        buildWhatsThis(openText, player1LoadStatusText));
    connect(m_pFileLoadSongPlayer1, SIGNAL(triggered()),
            this, SLOT(slotFileLoadSongPlayer1()));

    QString player2LoadStatusText = loadTrackStatusText.arg(QString::number(2));
    m_pFileLoadSongPlayer2 = new QAction(loadTrackText.arg(QString::number(2)), this);
    m_pFileLoadSongPlayer2->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "FileMenu_LoadDeck2"),
                                                  tr("Ctrl+Shift+O"))));
    m_pFileLoadSongPlayer2->setShortcutContext(Qt::ApplicationShortcut);
    m_pFileLoadSongPlayer2->setStatusTip(player2LoadStatusText);
    m_pFileLoadSongPlayer2->setWhatsThis(
        buildWhatsThis(openText, player2LoadStatusText));
    connect(m_pFileLoadSongPlayer2, SIGNAL(triggered()),
            this, SLOT(slotFileLoadSongPlayer2()));

    QString quitTitle = tr("&Exit");
    QString quitText = tr("Quits Mixxx");
    m_pFileQuit = new QAction(quitTitle, this);
    m_pFileQuit->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]", "FileMenu_Quit"),
                                                  tr("Ctrl+q"))));
    m_pFileQuit->setShortcutContext(Qt::ApplicationShortcut);
    m_pFileQuit->setStatusTip(quitText);
    m_pFileQuit->setWhatsThis(buildWhatsThis(quitTitle, quitText));
    connect(m_pFileQuit, SIGNAL(triggered()), this, SLOT(slotFileQuit()));

    QString rescanTitle = tr("&Rescan Library");
    QString rescanText = tr("Rescans library folders for changes to tracks.");
    m_pLibraryRescan = new QAction(rescanTitle, this);
    m_pLibraryRescan->setStatusTip(rescanText);
    m_pLibraryRescan->setWhatsThis(buildWhatsThis(rescanTitle, rescanText));
    m_pLibraryRescan->setCheckable(false);
    connect(m_pLibraryRescan, SIGNAL(triggered()),
            this, SLOT(slotScanLibrary()));

    QString createPlaylistTitle = tr("Create &New Playlist");
    QString createPlaylistText = tr("Create a new playlist");
    m_pPlaylistsNew = new QAction(createPlaylistTitle, this);
    m_pPlaylistsNew->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "LibraryMenu_NewPlaylist"),
                                                  tr("Ctrl+n"))));
    m_pPlaylistsNew->setShortcutContext(Qt::ApplicationShortcut);
    m_pPlaylistsNew->setStatusTip(createPlaylistText);
    m_pPlaylistsNew->setWhatsThis(buildWhatsThis(createPlaylistTitle, createPlaylistText));
    connect(m_pPlaylistsNew, SIGNAL(triggered()),
            m_pLibrary, SLOT(slotCreatePlaylist()));

    QString createCrateTitle = tr("Create New &Crate");
    QString createCrateText = tr("Create a new crate");
    m_pCratesNew = new QAction(createCrateTitle, this);
    m_pCratesNew->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "LibraryMenu_NewCrate"),
                                                  tr("Ctrl+Shift+N"))));
    m_pCratesNew->setShortcutContext(Qt::ApplicationShortcut);
    m_pCratesNew->setStatusTip(createCrateText);
    m_pCratesNew->setWhatsThis(buildWhatsThis(createCrateTitle, createCrateText));
    connect(m_pCratesNew, SIGNAL(triggered()),
            m_pLibrary, SLOT(slotCreateCrate()));

    QString fullScreenTitle = tr("&Full Screen");
    QString fullScreenText = tr("Display Mixxx using the full screen");
    m_pViewFullScreen = new QAction(fullScreenTitle, this);
#ifdef __APPLE__
    QString fullscreen_key = tr("Ctrl+Shift+F");
#else
    QString fullscreen_key = tr("F11");
#endif
    m_pViewFullScreen->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "ViewMenu_Fullscreen"),
                                                  fullscreen_key)));
    m_pViewFullScreen->setShortcutContext(Qt::ApplicationShortcut);
    // QShortcut * shortcut = new QShortcut(QKeySequence(tr("Esc")),  this);
    // connect(shortcut, SIGNAL(triggered()), this, SLOT(slotQuitFullScreen()));
    m_pViewFullScreen->setCheckable(true);
    m_pViewFullScreen->setChecked(false);
    m_pViewFullScreen->setStatusTip(fullScreenText);
    m_pViewFullScreen->setWhatsThis(buildWhatsThis(fullScreenTitle, fullScreenText));
    connect(m_pViewFullScreen, SIGNAL(toggled(bool)),
            this, SLOT(slotViewFullScreen(bool)));

    QString keyboardShortcutTitle = tr("Enable &Keyboard Shortcuts");
    QString keyboardShortcutText = tr("Toggles keyboard shortcuts on or off");
    bool keyboardShortcutsEnabled = m_pConfig->getValueString(
        ConfigKey("[Keyboard]", "Enabled")) == "1";
    m_pOptionsKeyboard = new QAction(keyboardShortcutTitle, this);
    m_pOptionsKeyboard->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_EnableShortcuts"),
                                                  tr("Ctrl+`"))));
    m_pOptionsKeyboard->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsKeyboard->setCheckable(true);
    m_pOptionsKeyboard->setChecked(keyboardShortcutsEnabled);
    m_pOptionsKeyboard->setStatusTip(keyboardShortcutText);
    m_pOptionsKeyboard->setWhatsThis(buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
    connect(m_pOptionsKeyboard, SIGNAL(toggled(bool)),
            this, SLOT(slotOptionsKeyboard(bool)));

    QString preferencesTitle = tr("&Preferences");
    QString preferencesText = tr("Change Mixxx settings (e.g. playback, MIDI, controls)");
    m_pOptionsPreferences = new QAction(preferencesTitle, this);
#ifdef __APPLE__
    m_pOptionsPreferences->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_Preferences"),
                                                  tr("Ctrl+,"))));
#else
    m_pOptionsPreferences->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_Preferences"),
                                                  tr("Ctrl+P"))));
#endif
    m_pOptionsPreferences->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsPreferences->setStatusTip(preferencesText);
    m_pOptionsPreferences->setWhatsThis(buildWhatsThis(preferencesTitle, preferencesText));
    connect(m_pOptionsPreferences, SIGNAL(triggered()),
            this, SLOT(slotOptionsPreferences()));

    QString aboutTitle = tr("&About");
    QString aboutText = tr("About the application");
    m_pHelpAboutApp = new QAction(aboutTitle, this);
    m_pHelpAboutApp->setStatusTip(aboutText);
    m_pHelpAboutApp->setWhatsThis(buildWhatsThis(aboutTitle, aboutText));
    connect(m_pHelpAboutApp, SIGNAL(triggered()),
            this, SLOT(slotHelpAbout()));

    QString supportTitle = tr("&Community Support");
    QString supportText = tr("Get help with Mixxx");
    m_pHelpSupport = new QAction(supportTitle, this);
    m_pHelpSupport->setStatusTip(supportText);
    m_pHelpSupport->setWhatsThis(buildWhatsThis(supportTitle, supportText));
    connect(m_pHelpSupport, SIGNAL(triggered()), this, SLOT(slotHelpSupport()));

    QString manualTitle = tr("&User Manual");
    QString manualText = tr("Read the Mixxx user manual.");
    m_pHelpManual = new QAction(manualTitle, this);
    m_pHelpManual->setStatusTip(manualText);
    m_pHelpManual->setWhatsThis(buildWhatsThis(manualTitle, manualText));
    connect(m_pHelpManual, SIGNAL(triggered()), this, SLOT(slotHelpManual()));

    QString feedbackTitle = tr("Send Us &Feedback");
    QString feedbackText = tr("Send feedback to the Mixxx team.");
    m_pHelpFeedback = new QAction(feedbackTitle, this);
    m_pHelpFeedback->setStatusTip(feedbackText);
    m_pHelpFeedback->setWhatsThis(buildWhatsThis(feedbackTitle, feedbackText));
    connect(m_pHelpFeedback, SIGNAL(triggered()), this, SLOT(slotHelpFeedback()));

    QString translateTitle = tr("&Translate This Application");
    QString translateText = tr("Help translate this application into your language.");
    m_pHelpTranslation = new QAction(translateTitle, this);
    m_pHelpTranslation->setStatusTip(translateText);
    m_pHelpTranslation->setWhatsThis(buildWhatsThis(translateTitle, translateText));
    connect(m_pHelpTranslation, SIGNAL(triggered()), this, SLOT(slotHelpTranslation()));

#ifdef __VINYLCONTROL__
    QString vinylControlText = tr(
            "Use timecoded vinyls on external turntables to control Mixxx");
    QList<QString> vinylControlTitle;
    m_VCCheckboxMapper = new QSignalMapper(this);
    connect(m_VCCheckboxMapper, SIGNAL(mapped(int)),
            this, SLOT(slotCheckboxVinylControl(int)));
    m_VCControlMapper = new QSignalMapper(this);
    connect(m_VCControlMapper, SIGNAL(mapped(int)),
            this, SLOT(slotControlVinylControl(int)));

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        vinylControlTitle.push_back(
                tr("Enable Vinyl Control &%1").arg(i + 1));

        m_pOptionsVinylControl.push_back(
                new QAction(vinylControlTitle.back(), this));
        QAction* vc_checkbox = m_pOptionsVinylControl.back();

        QString binding;
        switch (i) {
        case 0:
            binding = tr("Ctrl+t");
            break;
        case 1:
            binding = tr("Ctrl+y");
            break;
        case 2:
            binding = tr("Ctrl+u");
            break;
        case 3:
            binding = tr("Ctrl+i");
            break;
        default:
            qCritical() << "Programming error: bindings need to be defined for "
                        "vinyl control enabling";
        }

        vc_checkbox->setShortcut(
                QKeySequence(m_pKbdConfig->getValueString(ConfigKey(
                        "[KeyboardShortcuts]",
                        QString("OptionsMenu_EnableVinyl%1").arg(i + 1)),
                                                         binding)));
        vc_checkbox->setShortcutContext(Qt::ApplicationShortcut);

        // Either check or uncheck the vinyl control menu item depending on what
        // it was saved as.
        vc_checkbox->setCheckable(true);
        vc_checkbox->setChecked(false);
        vc_checkbox->setStatusTip(vinylControlText);
        vc_checkbox->setWhatsThis(
                buildWhatsThis(vinylControlTitle.back(), vinylControlText));

        m_VCCheckboxMapper->setMapping(vc_checkbox, i);
        connect(vc_checkbox, SIGNAL(toggled(bool)),
                m_VCCheckboxMapper, SLOT(map()));
    }

#endif

#ifdef __SHOUTCAST__
    QString shoutcastTitle = tr("Enable Live &Broadcasting");
    QString shoutcastText = tr("Stream your mixes to a shoutcast or icecast server");
    m_pOptionsShoutcast = new QAction(shoutcastTitle, this);
    m_pOptionsShoutcast->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_EnableLiveBroadcasting"),
                                                  tr("Ctrl+L"))));
    m_pOptionsShoutcast->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsShoutcast->setCheckable(true);
    m_pOptionsShoutcast->setChecked(m_pShoutcastManager->isEnabled());
    m_pOptionsShoutcast->setStatusTip(shoutcastText);
    m_pOptionsShoutcast->setWhatsThis(buildWhatsThis(shoutcastTitle, shoutcastText));

    connect(m_pOptionsShoutcast, SIGNAL(triggered(bool)),
            m_pShoutcastManager, SLOT(setEnabled(bool)));
#endif

    QString mayNotBeSupported = tr("May not be supported on all skins.");
    QString showSamplersTitle = tr("Show Samplers");
    QString showSamplersText = tr("Show the sample deck section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowSamplers = new QAction(showSamplersTitle, this);
    m_pViewShowSamplers->setCheckable(true);
    m_pViewShowSamplers->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "ViewMenu_ShowSamplers"),
                                                  tr("Ctrl+1", "Menubar|View|Show Samplers"))));
    m_pViewShowSamplers->setStatusTip(showSamplersText);
    m_pViewShowSamplers->setWhatsThis(buildWhatsThis(showSamplersTitle, showSamplersText));
    connect(m_pViewShowSamplers, SIGNAL(toggled(bool)),
            this, SLOT(slotViewShowSamplers(bool)));

    QString showVinylControlTitle = tr("Show Vinyl Control Section");
    QString showVinylControlText = tr("Show the vinyl control section of the Mixxx interface.") +
            " " + mayNotBeSupported;
#ifdef __VINYLCONTROL__
    m_pViewVinylControl = new QAction(showVinylControlTitle, this);
    m_pViewVinylControl->setCheckable(true);
    m_pViewVinylControl->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(
            ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowVinylControl"),
            tr("Ctrl+3", "Menubar|View|Show Vinyl Control Section"))));
    m_pViewVinylControl->setStatusTip(showVinylControlText);
    m_pViewVinylControl->setWhatsThis(buildWhatsThis(showVinylControlTitle, showVinylControlText));
    connect(m_pViewVinylControl, SIGNAL(toggled(bool)),
            this, SLOT(slotViewShowVinylControl(bool)));
#endif

    QString showMicrophoneTitle = tr("Show Microphone Section");
    QString showMicrophoneText = tr("Show the microphone section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowMicrophone = new QAction(showMicrophoneTitle, this);
    m_pViewShowMicrophone->setCheckable(true);
    m_pViewShowMicrophone->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(
            ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowMicrophone"),
            tr("Ctrl+2", "Menubar|View|Show Microphone Section"))));
    m_pViewShowMicrophone->setStatusTip(showMicrophoneText);
    m_pViewShowMicrophone->setWhatsThis(buildWhatsThis(showMicrophoneTitle, showMicrophoneText));
    connect(m_pViewShowMicrophone, SIGNAL(toggled(bool)),
            this, SLOT(slotViewShowMicrophone(bool)));

    QString showPreviewDeckTitle = tr("Show Preview Deck");
    QString showPreviewDeckText = tr("Show the preview deck in the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowPreviewDeck = new QAction(showPreviewDeckTitle, this);
    m_pViewShowPreviewDeck->setCheckable(true);
    m_pViewShowPreviewDeck->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "ViewMenu_ShowPreviewDeck"),
                                                  tr("Ctrl+4", "Menubar|View|Show Preview Deck"))));
    m_pViewShowPreviewDeck->setStatusTip(showPreviewDeckText);
    m_pViewShowPreviewDeck->setWhatsThis(buildWhatsThis(showPreviewDeckTitle, showPreviewDeckText));
    connect(m_pViewShowPreviewDeck, SIGNAL(toggled(bool)),
            this, SLOT(slotViewShowPreviewDeck(bool)));

    QString showCoverArtTitle = tr("Show Cover Art");
    QString showCoverArtText = tr("Show cover art in the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowCoverArt = new QAction(showCoverArtTitle, this);
    m_pViewShowCoverArt->setCheckable(true);
    m_pViewShowCoverArt->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "ViewMenu_ShowCoverArt"),
                                                  tr("Ctrl+5", "Menubar|View|Show Cover Art"))));
    m_pViewShowCoverArt->setStatusTip(showCoverArtText);
    m_pViewShowCoverArt->setWhatsThis(buildWhatsThis(showCoverArtTitle, showCoverArtText));
    connect(m_pViewShowCoverArt, SIGNAL(toggled(bool)),
            this, SLOT(slotViewShowCoverArt(bool)));

    QString recordTitle = tr("&Record Mix");
    QString recordText = tr("Record your mix to a file");
    m_pOptionsRecord = new QAction(recordTitle, this);
    m_pOptionsRecord->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_RecordMix"),
                                                  tr("Ctrl+R"))));
    m_pOptionsRecord->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsRecord->setCheckable(true);
    m_pOptionsRecord->setStatusTip(recordText);
    m_pOptionsRecord->setWhatsThis(buildWhatsThis(recordTitle, recordText));
    connect(m_pOptionsRecord, SIGNAL(toggled(bool)),
            m_pRecordingManager, SLOT(slotSetRecording(bool)));

    QString reloadSkinTitle = tr("&Reload Skin");
    QString reloadSkinText = tr("Reload the skin");
    m_pDeveloperReloadSkin = new QAction(reloadSkinTitle, this);
    m_pDeveloperReloadSkin->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_ReloadSkin"),
                                                  tr("Ctrl+Shift+R"))));
    m_pDeveloperReloadSkin->setShortcutContext(Qt::ApplicationShortcut);
    m_pDeveloperReloadSkin->setCheckable(true);
    m_pDeveloperReloadSkin->setChecked(false);
    m_pDeveloperReloadSkin->setStatusTip(reloadSkinText);
    m_pDeveloperReloadSkin->setWhatsThis(buildWhatsThis(reloadSkinTitle, reloadSkinText));
    connect(m_pDeveloperReloadSkin, SIGNAL(toggled(bool)),
            this, SLOT(slotDeveloperReloadSkin(bool)));

    QString developerToolsTitle = tr("Developer Tools");
    QString developerToolsText = tr("Opens the developer tools dialog");
    m_pDeveloperTools = new QAction(developerToolsTitle, this);
    m_pDeveloperTools->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "OptionsMenu_DeveloperTools"),
                                                  tr("Ctrl+Shift+T"))));
    m_pDeveloperTools->setShortcutContext(Qt::ApplicationShortcut);
    m_pDeveloperTools->setStatusTip(developerToolsText);
    m_pDeveloperTools->setWhatsThis(buildWhatsThis(developerToolsTitle, developerToolsText));
    connect(m_pDeveloperTools, SIGNAL(triggered()),
            this, SLOT(slotDeveloperTools()));

    QString enableExperimentTitle = tr("Stats: Experiment Bucket");
    QString enableExperimentToolsText = tr(
        "Enables experiment mode. Collects stats in the EXPERIMENT tracking bucket.");
    m_pDeveloperStatsExperiment = new QAction(enableExperimentTitle, this);
    m_pDeveloperStatsExperiment->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                            "OptionsMenu_DeveloperStatsExperiment"),
                                                  tr("Ctrl+Shift+E"))));
    m_pDeveloperStatsExperiment->setShortcutContext(Qt::ApplicationShortcut);
    m_pDeveloperStatsExperiment->setStatusTip(enableExperimentToolsText);
    m_pDeveloperStatsExperiment->setWhatsThis(buildWhatsThis(
        enableExperimentTitle, enableExperimentToolsText));
    m_pDeveloperStatsExperiment->setCheckable(true);
    m_pDeveloperStatsExperiment->setChecked(Experiment::isExperiment());
    connect(m_pDeveloperStatsExperiment, SIGNAL(triggered()),
            this, SLOT(slotDeveloperStatsExperiment()));

    QString enableBaseTitle = tr("Stats: Base Bucket");
    QString enableBaseToolsText = tr(
        "Enables base mode. Collects stats in the BASE tracking bucket.");
    m_pDeveloperStatsBase = new QAction(enableBaseTitle, this);
    m_pDeveloperStatsBase->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                            "OptionsMenu_DeveloperStatsBase"),
                                                  tr("Ctrl+Shift+B"))));
    m_pDeveloperStatsBase->setShortcutContext(Qt::ApplicationShortcut);
    m_pDeveloperStatsBase->setStatusTip(enableBaseToolsText);
    m_pDeveloperStatsBase->setWhatsThis(buildWhatsThis(
        enableBaseTitle, enableBaseToolsText));
    m_pDeveloperStatsBase->setCheckable(true);
    m_pDeveloperStatsBase->setChecked(Experiment::isBase());
    connect(m_pDeveloperStatsBase, SIGNAL(triggered()),
            this, SLOT(slotDeveloperStatsBase()));




    QString scriptDebuggerTitle = tr("Debugger Enabled");
    QString scriptDebuggerText = tr("Enables the debugger during skin parsing");
    bool scriptDebuggerEnabled = m_pConfig->getValueString(
        ConfigKey("[ScriptDebugger]", "Enabled")) == "1";
    m_pDeveloperDebugger = new QAction(scriptDebuggerTitle, this);

    m_pDeveloperDebugger->setShortcut(
        QKeySequence(m_pKbdConfig->getValueString(ConfigKey("[KeyboardShortcuts]",
                                                  "DeveloperMenu_EnableDebugger"),
                                                  tr("Ctrl+Shift+D"))));
    m_pDeveloperDebugger->setShortcutContext(Qt::ApplicationShortcut);
    m_pDeveloperDebugger->setWhatsThis(buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
    m_pDeveloperDebugger->setCheckable(true);
    m_pDeveloperDebugger->setStatusTip(scriptDebuggerText);
    m_pDeveloperDebugger->setChecked(scriptDebuggerEnabled);
    connect(m_pDeveloperDebugger, SIGNAL(toggled(bool)),
            this, SLOT(slotDeveloperDebugger(bool)));



    // TODO: This code should live in a separate class.
    m_TalkoverMapper = new QSignalMapper(this);
    connect(m_TalkoverMapper, SIGNAL(mapped(int)),
            this, SLOT(slotTalkoverChanged(int)));
    for (int i = 0; i < kMicrophoneCount; ++i) {
        QString group("[Microphone]");
        if (i > 0) {
            group = QString("[Microphone%1]").arg(i + 1);
        }
        ControlObjectSlave* talkover_button(new ControlObjectSlave(
                group, "talkover", this));
        m_TalkoverMapper->setMapping(talkover_button, i);
        talkover_button->connectValueChanged(m_TalkoverMapper, SLOT(map()));
        m_micTalkoverControls.push_back(talkover_button);
    }
}

void MixxxMainWindow::initMenuBar()
{
    // MENUBAR
    m_pFileMenu = new QMenu(tr("&File"), menuBar());
    m_pOptionsMenu = new QMenu(tr("&Options"), menuBar());
    m_pLibraryMenu = new QMenu(tr("&Library"),menuBar());
    m_pViewMenu = new QMenu(tr("&View"), menuBar());
    m_pHelpMenu = new QMenu(tr("&Help"), menuBar());
    m_pDeveloperMenu = new QMenu(tr("Developer"), menuBar());
    connect(m_pOptionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotOptionsMenuShow()));
    // menuBar entry fileMenu
    m_pFileMenu->addAction(m_pFileLoadSongPlayer1);
    m_pFileMenu->addAction(m_pFileLoadSongPlayer2);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pFileQuit);

    // menuBar entry optionsMenu
    //optionsMenu->setCheckable(true);
#ifdef __VINYLCONTROL__
    m_pVinylControlMenu = new QMenu(tr("&Vinyl Control"), menuBar());
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_pVinylControlMenu->addAction(m_pOptionsVinylControl[i]);
    }

    m_pOptionsMenu->addMenu(m_pVinylControlMenu);
    m_pOptionsMenu->addSeparator();
#endif

    m_pOptionsMenu->addAction(m_pOptionsRecord);
#ifdef __SHOUTCAST__
    m_pOptionsMenu->addAction(m_pOptionsShoutcast);
#endif
    m_pOptionsMenu->addSeparator();
    m_pOptionsMenu->addAction(m_pOptionsKeyboard);
    m_pOptionsMenu->addSeparator();
    m_pOptionsMenu->addAction(m_pOptionsPreferences);

    m_pLibraryMenu->addAction(m_pLibraryRescan);
    m_pLibraryMenu->addSeparator();
    m_pLibraryMenu->addAction(m_pPlaylistsNew);
    m_pLibraryMenu->addAction(m_pCratesNew);

    // menuBar entry viewMenu
    //viewMenu->setCheckable(true);
    m_pViewMenu->addAction(m_pViewShowSamplers);
    m_pViewMenu->addAction(m_pViewShowMicrophone);
#ifdef __VINYLCONTROL__
    m_pViewMenu->addAction(m_pViewVinylControl);
#endif
    m_pViewMenu->addAction(m_pViewShowPreviewDeck);
    m_pViewMenu->addAction(m_pViewShowCoverArt);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addAction(m_pViewFullScreen);

    // Developer Menu
    m_pDeveloperMenu->addAction(m_pDeveloperReloadSkin);
    m_pDeveloperMenu->addAction(m_pDeveloperTools);
    m_pDeveloperMenu->addAction(m_pDeveloperStatsExperiment);
    m_pDeveloperMenu->addAction(m_pDeveloperStatsBase);
    m_pDeveloperMenu->addAction(m_pDeveloperDebugger);

    // menuBar entry helpMenu
    m_pHelpMenu->addAction(m_pHelpSupport);
    m_pHelpMenu->addAction(m_pHelpManual);
    m_pHelpMenu->addAction(m_pHelpFeedback);
    m_pHelpMenu->addAction(m_pHelpTranslation);
    m_pHelpMenu->addSeparator();
    m_pHelpMenu->addAction(m_pHelpAboutApp);

    menuBar()->addMenu(m_pFileMenu);
    menuBar()->addMenu(m_pLibraryMenu);
    menuBar()->addMenu(m_pViewMenu);
    menuBar()->addMenu(m_pOptionsMenu);

    if (m_cmdLineArgs.getDeveloper()) {
        menuBar()->addMenu(m_pDeveloperMenu);
    }

    menuBar()->addSeparator();
    menuBar()->addMenu(m_pHelpMenu);
}

void MixxxMainWindow::slotFileLoadSongPlayer(int deck) {
    QString group = m_pPlayerManager->groupForDeck(deck-1);

    QString loadTrackText = tr("Load track to Deck %1").arg(QString::number(deck));
    QString deckWarningMessage = tr("Deck %1 is currently playing a track.")
            .arg(QString::number(deck));
    QString areYouSure = tr("Are you sure you want to load a new track?");

    if (ControlObject::get(ConfigKey(group, "play")) > 0.0) {
        int ret = QMessageBox::warning(this, tr("Mixxx"),
            deckWarningMessage + "\n" + areYouSure,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return;
    }

    QString trackPath =
        QFileDialog::getOpenFileName(
            this,
            loadTrackText,
            m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR),
            QString("Audio (%1)")
                .arg(SoundSourceProxy::supportedFileExtensionsString()));


    if (!trackPath.isNull()) {
        // The user has picked a file via a file dialog. This means the system
        // sandboxer (if we are sandboxed) has granted us permission to this
        // folder. Create a security bookmark while we have permission so that
        // we can access the folder on future runs. We need to canonicalize the
        // path so we first wrap the directory string with a QDir.
        QFileInfo trackInfo(trackPath);
        Sandbox::createSecurityToken(trackInfo);

        m_pPlayerManager->slotLoadToDeck(trackPath, deck);
    }
}

void MixxxMainWindow::slotFileLoadSongPlayer1() {
    slotFileLoadSongPlayer(1);
}

void MixxxMainWindow::slotFileLoadSongPlayer2() {
    slotFileLoadSongPlayer(2);
}

void MixxxMainWindow::slotFileQuit()
{
    if (!confirmExit()) {
        return;
    }
    hide();
    qApp->quit();
}

void MixxxMainWindow::slotOptionsKeyboard(bool toggle) {
    if (toggle) {
        //qDebug() << "Enable keyboard shortcuts/mappings";
        m_pKeyboard->setKeyboardConfig(m_pKbdConfig);
        m_pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(1));
    } else {
        //qDebug() << "Disable keyboard shortcuts/mappings";
        m_pKeyboard->setKeyboardConfig(m_pKbdConfigEmpty);
        m_pConfig->set(ConfigKey("[Keyboard]","Enabled"), ConfigValue(0));
    }
}

void MixxxMainWindow::slotDeveloperReloadSkin(bool toggle) {
    Q_UNUSED(toggle);
    rebootMixxxView();
}

void MixxxMainWindow::slotDeveloperTools() {
    if (m_pDeveloperToolsDlg == NULL) {
        m_pDeveloperToolsDlg = new DlgDeveloperTools(this, m_pConfig);
        connect(m_pDeveloperToolsDlg, SIGNAL(destroyed()),
                this, SLOT(slotDeveloperToolsClosed()));
    }
    m_pDeveloperToolsDlg->show();
    m_pDeveloperToolsDlg->activateWindow();
}

void MixxxMainWindow::slotDeveloperDebugger(bool toggle) {
    m_pConfig->set(ConfigKey("[ScriptDebugger]","Enabled"),
                   ConfigValue(toggle ? 1 : 0));
}

void MixxxMainWindow::slotDeveloperToolsClosed() {
    m_pDeveloperToolsDlg = NULL;
}

void MixxxMainWindow::slotDeveloperStatsExperiment() {
    if (m_pDeveloperStatsExperiment->isChecked()) {
        m_pDeveloperStatsBase->setChecked(false);
        Experiment::setExperiment();
    } else {
        Experiment::disable();
    }
}

void MixxxMainWindow::slotDeveloperStatsBase() {
    if (m_pDeveloperStatsBase->isChecked()) {
        m_pDeveloperStatsExperiment->setChecked(false);
        Experiment::setBase();
    } else {
        Experiment::disable();
    }
}



void MixxxMainWindow::slotViewFullScreen(bool toggle)
{
    if (m_pViewFullScreen)
        m_pViewFullScreen->setChecked(toggle);

    if (isFullScreen() == toggle) {
        return;
    }

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
        showFullScreen();
#ifdef __LINUX__
        // Fix for "No menu bar with ubuntu unity in full screen mode" Bug #885890
        // Not for Mac OS because the native menu bar will unhide when moving
        // the mouse to the top of screen

        //menuBar()->setNativeMenuBar(false);
        // ^ This leaves a broken native Menu Bar with Ubuntu Unity Bug #1076789#
        // it is only allowed to change this prior initMenuBar()

        m_NativeMenuBarSupport = menuBar()->isNativeMenuBar();
        if (m_NativeMenuBarSupport) {
            setMenuBar(new QMenuBar(this));
            menuBar()->setNativeMenuBar(false);
            initMenuBar();
        }
#endif
    } else {
#ifdef __LINUX__
        if (m_NativeMenuBarSupport) {
            setMenuBar(new QMenuBar(this));
            menuBar()->setNativeMenuBar(m_NativeMenuBarSupport);
            initMenuBar();
        }
        //move(m_winpos);
#endif
        showNormal();
    }
}

void MixxxMainWindow::slotOptionsPreferences()
{
    m_pPrefDlg->setHidden(false);
    m_pPrefDlg->activateWindow();
}

void MixxxMainWindow::slotControlVinylControl(int deck) {
#ifdef __VINYLCONTROL__
    if (deck >= m_iNumConfiguredDecks) {
        qWarning() << "Tried to activate vinyl control on a deck that we "
                      "haven't configured -- ignoring request.";
        m_pVinylControlEnabled[deck]->set(0.0);
        return;
    }
    bool toggle = m_pVinylControlEnabled[deck]->get();
    if (m_pPlayerManager->hasVinylInput(deck)) {
        m_pOptionsVinylControl[deck]->setChecked((bool) toggle);
    } else {
        m_pOptionsVinylControl[deck]->setChecked(false);
        if (toggle) {
            QMessageBox::warning(
                    this,
                    tr("Mixxx"),
                    tr("There is no input device selected for this vinyl control.\n"
                       "Please select an input device in the sound hardware preferences first."),
                    QMessageBox::Ok, QMessageBox::Ok);
            m_pPrefDlg->show();
            m_pPrefDlg->showSoundHardwarePage();
            ControlObject::set(ConfigKey(PlayerManager::groupForDeck(deck),
                                         "vinylcontrol_status"),
                               (double) VINYL_STATUS_DISABLED);
            m_pVinylControlEnabled[deck]->set(0.0);
        }
    }
#endif
}

void MixxxMainWindow::slotControlPassthrough(int index) {
#ifdef __VINYLCONTROL__
    if (index >= kMaximumVinylControlInputs || index >= m_iNumConfiguredDecks) {
        qWarning() << "Tried to activate passthrough on a deck that we "
                      "haven't configured -- ignoring request.";
        m_pPassthroughEnabled[index]->set(0.0);
        return;
    }
    bool toggle = static_cast<bool>(m_pPassthroughEnabled[index]->get());
    if (toggle) {
        if (m_pPlayerManager->hasVinylInput(index)) {
            return;
        }
        // Else...
        m_pOptionsVinylControl[index]->setChecked(false);
        m_pPassthroughEnabled[index]->set(0.0);

        QMessageBox::warning(
                this,
                tr("Mixxx"),
                tr("There is no input device selected for this passthrough control.\n"
                   "Please select an input device in the sound hardware preferences first."),
                QMessageBox::Ok, QMessageBox::Ok);
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
#endif
}

void MixxxMainWindow::slotControlAuxiliary(int index) {
    if (index >= kAuxiliaryCount || index >= m_iNumConfiguredDecks) {
        qWarning() << "Tried to activate auxiliary input that we "
                      "haven't configured -- ignoring request.";
        m_pAuxiliaryPassthrough[index]->set(0.0);
        return;
    }
    bool passthrough = static_cast<bool>(m_pAuxiliaryPassthrough[index]->get());
    if (passthrough) {
        if (ControlObject::getControl(
                m_pAuxiliaryPassthrough[index]->getKey().group,
                "enabled")->get()) {
            return;
        }
        // Else...
        m_pAuxiliaryPassthrough[index]->set(0.0);

        QMessageBox::warning(
                this,
                tr("Mixxx"),
                tr("There is no input device selected for this auxiliary input.\n"
                   "Please select an input device in the sound hardware preferences first."),
                QMessageBox::Ok, QMessageBox::Ok);
        m_pPrefDlg->show();
        m_pPrefDlg->showSoundHardwarePage();
    }
}

void MixxxMainWindow::slotCheckboxVinylControl(int deck) {
#ifdef __VINYLCONTROL__
    if (deck >= m_iNumConfiguredDecks) {
        qWarning() << "Tried to activate vinyl control on a deck that we "
                      "haven't configured -- ignoring request.";
        m_pOptionsVinylControl[deck]->setChecked(false);
        return;
    }
    bool toggle = m_pOptionsVinylControl[deck]->isChecked();
    m_pVinylControlEnabled[deck]->set((double) toggle);
    slotControlVinylControl(deck);
#endif
}

void MixxxMainWindow::slotNumDecksChanged(double dNumDecks) {
    int num_decks = math_min<int>(dNumDecks, kMaximumVinylControlInputs);

#ifdef __VINYLCONTROL__
    // Only show menu items to activate vinyl inputs that exist.
    for (int i = m_iNumConfiguredDecks; i < num_decks; ++i) {
        m_pOptionsVinylControl[i]->setVisible(true);
        m_pVinylControlEnabled.push_back(
                new ControlObjectSlave(PlayerManager::groupForDeck(i),
                                        "vinylcontrol_enabled"));
        ControlObjectSlave* vc_enabled = m_pVinylControlEnabled.back();
        m_VCControlMapper->setMapping(vc_enabled, i);
        vc_enabled->connectValueChanged(m_VCControlMapper, SLOT(map()));

        m_pPassthroughEnabled.push_back(
                new ControlObjectSlave(PlayerManager::groupForDeck(i),
                                        "passthrough"));
        ControlObjectSlave* passthrough_enabled = m_pPassthroughEnabled.back();
        m_PassthroughMapper->setMapping(passthrough_enabled, i);
        passthrough_enabled->connectValueChanged(m_PassthroughMapper,
                                                 SLOT(map()));
    }
    for (int i = num_decks; i < kMaximumVinylControlInputs; ++i) {
        m_pOptionsVinylControl[i]->setVisible(false);
    }
#endif
    m_iNumConfiguredDecks = num_decks;
}

void MixxxMainWindow::slotTalkoverChanged(int mic_num) {
    if (mic_num >= m_micTalkoverControls.length()) {
        qWarning() << "Got a talkover change notice from outside the range.";
    }
    ControlObject* configured =
            ControlObject::getControl(m_micTalkoverControls[mic_num]->getKey().group,
                                      "enabled",
                                      false);

    // If the microphone is already configured, we are ok.
    if ((configured && configured->get() > 0.0) ||
            m_micTalkoverControls[mic_num]->get() == 0.0) {
        return;
    }
    m_micTalkoverControls[mic_num]->set(0.0);
    QMessageBox::warning(
                this,
                tr("Mixxx"),
                tr("There is no input device selected for this microphone.\n"
                   "Please select an input device in the sound hardware preferences first."),
                QMessageBox::Ok, QMessageBox::Ok);
    m_pPrefDlg->show();
    m_pPrefDlg->showSoundHardwarePage();
}

void MixxxMainWindow::slotHelpAbout() {
    DlgAbout *about = new DlgAbout(this);
    about->show();
}

void MixxxMainWindow::slotHelpSupport() {
    QUrl qSupportURL;
    qSupportURL.setUrl(MIXXX_SUPPORT_URL);
    QDesktopServices::openUrl(qSupportURL);
}

void MixxxMainWindow::slotHelpFeedback() {
    QUrl qFeedbackUrl;
    qFeedbackUrl.setUrl(MIXXX_FEEDBACK_URL);
    QDesktopServices::openUrl(qFeedbackUrl);
}

void MixxxMainWindow::slotHelpTranslation() {
    QUrl qTranslationUrl;
    qTranslationUrl.setUrl(MIXXX_TRANSLATION_URL);
    QDesktopServices::openUrl(qTranslationUrl);
}

void MixxxMainWindow::slotHelpManual() {
    QDir resourceDir(m_pConfig->getResourcePath());
    // Default to the mixxx.org hosted version of the manual.
    QUrl qManualUrl(MIXXX_MANUAL_URL);
#if defined(__APPLE__)
    // We don't include the PDF manual in the bundle on OSX. Default to the
    // web-hosted version.
#elif defined(__WINDOWS__)
    // On Windows, the manual PDF sits in the same folder as the 'skins' folder.
    if (resourceDir.exists(MIXXX_MANUAL_FILENAME)) {
        qManualUrl = QUrl::fromLocalFile(
                resourceDir.absoluteFilePath(MIXXX_MANUAL_FILENAME));
    }
#elif defined(__LINUX__)
    // On GNU/Linux, the manual is installed to e.g. /usr/share/mixxx/doc/
    if (resourceDir.cd("doc") && resourceDir.exists(MIXXX_MANUAL_FILENAME)) {
        qManualUrl = QUrl::fromLocalFile(
                resourceDir.absoluteFilePath(MIXXX_MANUAL_FILENAME));
    }
#else
    // No idea, default to the mixxx.org hosted version.
#endif
    QDesktopServices::openUrl(qManualUrl);
}

void MixxxMainWindow::setToolTipsCfg(int tt) {
    m_pConfig->set(ConfigKey("[Controls]","Tooltips"),
                   ConfigValue(tt));
    m_toolTipsCfg = tt;
}

void MixxxMainWindow::rebootMixxxView() {
    qDebug() << "Now in rebootMixxxView...";

    QPoint initPosition = pos();
    QSize initSize = size();

    if (m_pWidgetParent) {
        m_pWidgetParent->hide();
        WaveformWidgetFactory::instance()->destroyWidgets();
        delete m_pWidgetParent;
        m_pWidgetParent = NULL;
    }

    // Workaround for changing skins while fullscreen, just go out of fullscreen
    // mode. If you change skins while in fullscreen (on Linux, at least) the
    // window returns to 0,0 but and the backdrop disappears so it looks as if
    // it is not fullscreen, but acts as if it is.
    bool wasFullScreen = m_pViewFullScreen->isChecked();
    slotViewFullScreen(false);

    // Load skin to a QWidget that we set as the central widget. Assignment
    // intentional in next line.
    if (!(m_pWidgetParent = m_pSkinLoader->loadDefaultSkin(this,
                                                           m_pKeyboard,
                                                           m_pPlayerManager,
                                                           m_pControllerManager,
                                                           m_pLibrary,
                                                           m_pVCManager,
                                                           m_pEffectsManager))) {

        QMessageBox::critical(this,
                              tr("Error in skin file"),
                              tr("The selected skin cannot be loaded."));
        // m_pWidgetParent is NULL, we can't continue.
        return;
    }

    setCentralWidget(m_pWidgetParent);
    update();
    adjustSize();

    if (wasFullScreen) {
        slotViewFullScreen(true);
    } else {
        move(initPosition.x() + (initSize.width() - m_pWidgetParent->width()) / 2,
             initPosition.y() + (initSize.height() - m_pWidgetParent->height()) / 2);
    }

#ifdef __APPLE__
    // Original the following line fixes issue on OSX where menu bar went away
    // after a skin change. It was original surrounded by #if __OSX__
    // Now it seems it causes the opposite see Bug #1000187
    //menuBar()->setNativeMenuBar(m_NativeMenuBarSupport);
#endif

    qDebug() << "rebootMixxxView DONE";
    emit(newSkinLoaded());
}

/** Event filter to block certain events. For example, this function is used
  * to disable tooltips if the user specifies in the preferences that they
  * want them off. This is a callback function.
  */
bool MixxxMainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        // return true for no tool tips
        if (m_toolTipsCfg == 2) {
            // ON (only in Library)
            WBaseWidget* pWidget = dynamic_cast<WBaseWidget*>(obj);
            return pWidget != NULL;
        } else if (m_toolTipsCfg == 1) {
            // ON
            return false;
        } else {
            // OFF
            return true;
        }
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

bool MixxxMainWindow::event(QEvent* e) {
    switch(e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        // If the touch event falls trough to the main Widget, no touch widget
        // was touched, so we resend it as a mouse events.
        // We have to accept it here, so QApplication will continue to deliver
        // the following events of this touch point as well.
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(e);
        touchEvent->accept();
        return true;
    }
    default:
        break;
    }
    return QWidget::event(e);
}

void MixxxMainWindow::closeEvent(QCloseEvent *event) {
    if (!confirmExit()) {
        event->ignore();
    }
}

void MixxxMainWindow::slotScanLibrary() {
    emit(libraryScanStarted());
    m_pLibraryRescan->setEnabled(false);
    m_pLibraryScanner->scan();
}

void MixxxMainWindow::slotEnableRescanLibraryAction() {
    m_pLibraryRescan->setEnabled(true);
    emit(libraryScanFinished());
}

void MixxxMainWindow::slotOptionsMenuShow() {
    // Check recording if it is active.
    m_pOptionsRecord->setChecked(m_pRecordingManager->isRecordingActive());
#ifdef __SHOUTCAST__
    m_pOptionsShoutcast->setChecked(m_pShoutcastManager->isEnabled());
#endif
}

void MixxxMainWindow::slotToCenterOfPrimaryScreen() {
    if (!m_pWidgetParent)
        return;

    QDesktopWidget* desktop = QApplication::desktop();
    int primaryScreen = desktop->primaryScreen();
    QRect primaryScreenRect = desktop->availableGeometry(primaryScreen);

    move(primaryScreenRect.left() + (primaryScreenRect.width() - m_pWidgetParent->width()) / 2,
         primaryScreenRect.top() + (primaryScreenRect.height() - m_pWidgetParent->height()) / 2);
}

void MixxxMainWindow::checkDirectRendering() {
    // IF
    //  * A waveform viewer exists
    // AND
    //  * The waveform viewer is an OpenGL waveform viewer
    // AND
    //  * The waveform viewer does not have direct rendering enabled.
    // THEN
    //  * Warn user

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    if (!factory)
        return;

    if (!factory->isOpenGLAvailable() &&
        m_pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes")) {
        QMessageBox::warning(
            0, tr("OpenGL Direct Rendering"),
            tr("Direct rendering is not enabled on your machine.<br><br>"
               "This means that the waveform displays will be very<br>"
               "<b>slow and may tax your CPU heavily</b>. Either update your<br>"
               "configuration to enable direct rendering, or disable<br>"
               "the waveform displays in the Mixxx preferences by selecting<br>"
               "\"Empty\" as the waveform display in the 'Interface' section.<br><br>"
               "NOTE: If you use NVIDIA hardware,<br>"
               "direct rendering may not be present, but you should<br>"
               "not experience degraded performance."));
        m_pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), QString("yes"));
    }
}

bool MixxxMainWindow::confirmExit() {
    bool playing(false);
    bool playingSampler(false);
    unsigned int deckCount = m_pPlayerManager->numDecks();
    unsigned int samplerCount = m_pPlayerManager->numSamplers();
    for (unsigned int i = 0; i < deckCount; ++i) {
        if (ControlObject::get(
                ConfigKey(PlayerManager::groupForDeck(i), "play"))) {
            playing = true;
            break;
        }
    }
    for (unsigned int i = 0; i < samplerCount; ++i) {
        if (ControlObject::get(
                ConfigKey(PlayerManager::groupForSampler(i), "play"))) {
            playingSampler = true;
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
    } else if (playingSampler) {
        QMessageBox::StandardButton btn = QMessageBox::question(this,
            tr("Confirm Exit"),
            tr("A sampler is currently playing. Exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
    }
    if (m_pPrefDlg->isVisible()) {
        QMessageBox::StandardButton btn = QMessageBox::question(
            this, tr("Confirm Exit"),
            tr("The preferences window is still open.") + "<br>" +
            tr("Discard any changes and exit Mixxx?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn == QMessageBox::No) {
            return false;
        }
        else {
            m_pPrefDlg->close();
        }
    }
    return true;
}
