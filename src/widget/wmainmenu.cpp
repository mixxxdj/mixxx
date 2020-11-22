#include "widget/wmainmenu.h"

#include <QColor>
#include <QDesktopServices>
#include <QMainWindow>
#include <QPalette>
#include <QUrl>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "mixer/playermanager.h"
#include "util/cmdlineargs.h"
#include "util/experiment.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {

const int kMaxLoadToDeckActions = 4;
const auto kFeatureHideMenubar = ConfigKey("[Skin]", QLatin1String("feature_hide_menubar"));
const auto kFeatureSkinSettings = ConfigKey("[Skin]", QLatin1String("feature_skin_settings"));

QString buildWhatsThis(const QString& title, const QString& text) {
    QString preparedTitle = title;
    return QString("%1\n\n%2").arg(preparedTitle.remove("&"), text);
}

QString vinylControlDefaultKeyBinding(int deck) {
    // More bindings need to be defined if you increment
    // kMaximumVinylControlInputs.
    DEBUG_ASSERT(deck < kMaximumVinylControlInputs);
    switch (deck) {
    case 0:
        return QObject::tr("Ctrl+t");
    case 1:
        return QObject::tr("Ctrl+y");
    case 2:
        return QObject::tr("Ctrl+u");
    case 3:
        return QObject::tr("Ctrl+i");
    default:
        return QString();
    }
}

QString loadToDeckDefaultKeyBinding(int deck) {
    switch (deck) {
    case 0:
        return QObject::tr("Ctrl+o");
    case 1:
        return QObject::tr("Ctrl+Shift+O");
    default:
        return QString();
    }
}

QString showPreferencesKeyBinding() {
#ifdef __APPLE__
    return QObject::tr("Ctrl+,");
#else
    return QObject::tr("Ctrl+P");
#endif
}

} // namespace

WMainMenu::WMainMenu(QWidget* pParent,
        UserSettingsPointer pConfig,
        ConfigObject<ConfigValueKbd>* pKbdConfig)
        : QWidget(pParent),
          m_pConfig(pConfig),
          m_pKbdConfig(pKbdConfig),
          m_lastNumPlayers(0),
          m_pMenubarConnection(nullptr) {
    // feature COs
    m_pFeatureCOHideMenubar = new ControlObject(kFeatureHideMenubar);
    m_pFeatureCOSkinSettings = new ControlObject(kFeatureSkinSettings);

    // FILE MENU
    QString loadTrackText = tr("Load Track to Deck &%1");
    QString loadTrackStatusText = tr("Loads a track in deck %1");
    QString openText = tr("Open");
    for (unsigned int deck = 0; deck < kMaxLoadToDeckActions; ++deck) {
        QString playerLoadStatusText = loadTrackStatusText.arg(QString::number(deck + 1));
        QAction* pFileLoadSongToPlayer = new QAction(
                loadTrackText.arg(QString::number(deck + 1)), this);
        pFileLoadSongToPlayer->setData(deck + 1);

        QString binding = m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", QString("FileMenu_LoadDeck%1").arg(deck + 1)),
                loadToDeckDefaultKeyBinding(deck));
        if (!binding.isEmpty()) {
            pFileLoadSongToPlayer->setShortcut(QKeySequence(binding));
            pFileLoadSongToPlayer->setShortcutContext(Qt::ApplicationShortcut);
        }
        pFileLoadSongToPlayer->setStatusTip(playerLoadStatusText);
        pFileLoadSongToPlayer->setWhatsThis(
                buildWhatsThis(openText, playerLoadStatusText));
        // Visibility of load to deck actions is set in
        // WMainMenu::onNumberOfDecksChanged.
        pFileLoadSongToPlayer->setVisible(deck + 1 <= m_lastNumPlayers);
        connect(pFileLoadSongToPlayer, &QAction::triggered, this, [this, deck] {
            emit loadTrackToDeck(deck + 1);
        });

        m_loadToDeckActions.push_back(pFileLoadSongToPlayer);
    }

    QString quitTitle = tr("&Exit");
    QString quitText = tr("Quits Mixxx");
    m_pFileQuit = new QAction(quitTitle, this);
    m_pFileQuit->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(ConfigKey("[KeyboardShortcuts]", "FileMenu_Quit"),
                    tr("Ctrl+q"))));
    m_pFileQuit->setShortcutContext(Qt::ApplicationShortcut);
    m_pFileQuit->setStatusTip(quitText);
    m_pFileQuit->setWhatsThis(buildWhatsThis(quitTitle, quitText));
    m_pFileQuit->setMenuRole(QAction::QuitRole);
    connect(m_pFileQuit, SIGNAL(triggered()), this, SIGNAL(quit()));

    QString rescanTitle = tr("&Rescan Library");
    QString rescanText = tr("Rescans library folders for changes to tracks.");
    m_pLibraryRescan = new QAction(rescanTitle, this);
    m_pLibraryRescan->setStatusTip(rescanText);
    m_pLibraryRescan->setWhatsThis(buildWhatsThis(rescanTitle, rescanText));
    m_pLibraryRescan->setCheckable(false);
    connect(m_pLibraryRescan, SIGNAL(triggered()), this, SIGNAL(rescanLibrary()));
    // Disable the action when a scan is active.
    connect(this,
            SIGNAL(internalLibraryScanActive(bool)),
            m_pLibraryRescan,
            SLOT(setDisabled(bool)));

    QString createPlaylistTitle = tr("Create &New Playlist");
    QString createPlaylistText = tr("Create a new playlist");
    m_pLibraryCreatePlaylist = new QAction(createPlaylistTitle, this);
    m_pLibraryCreatePlaylist->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "LibraryMenu_NewPlaylist"),
                    tr("Ctrl+n"))));
    m_pLibraryCreatePlaylist->setShortcutContext(Qt::ApplicationShortcut);
    m_pLibraryCreatePlaylist->setStatusTip(createPlaylistText);
    m_pLibraryCreatePlaylist->setWhatsThis(buildWhatsThis(createPlaylistTitle, createPlaylistText));
    connect(m_pLibraryCreatePlaylist, SIGNAL(triggered()), this, SIGNAL(createPlaylist()));

    QString createCrateTitle = tr("Create New &Crate");
    QString createCrateText = tr("Create a new crate");
    m_pLibraryCreateCrate = new QAction(createCrateTitle, this);
    m_pLibraryCreateCrate->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(ConfigKey("[KeyboardShortcuts]",
                                                        "LibraryMenu_NewCrate"),
                    tr("Ctrl+Shift+N"))));
    m_pLibraryCreateCrate->setShortcutContext(Qt::ApplicationShortcut);
    m_pLibraryCreateCrate->setStatusTip(createCrateText);
    m_pLibraryCreateCrate->setWhatsThis(buildWhatsThis(createCrateTitle, createCrateText));
    connect(m_pLibraryCreateCrate, SIGNAL(triggered()), this, SIGNAL(createCrate()));

    // Skin Settings Menu
    QString mayNotBeSupported = tr("May not be supported on all skins.");
    QString showSkinSettingsTitle = tr("Show Skin Settings Menu");
    QString showSkinSettingsText =
            tr("Show the Skin Settings Menu of the currently selected Skin") +
            " " + mayNotBeSupported;
    m_pViewShowSkinSettings = new QAction(showSkinSettingsTitle, this);
    m_pViewShowSkinSettings->setCheckable(true);
    m_pViewShowSkinSettings->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowSkinSettings"),
                    tr("Ctrl+1", "Menubar|View|Show Skin Settings"))));
    m_pViewShowSkinSettings->setStatusTip(showSkinSettingsText);
    m_pViewShowSkinSettings->setWhatsThis(
            buildWhatsThis(showSkinSettingsTitle, showSkinSettingsText));
    createVisibilityControl(m_pViewShowSkinSettings,
            ConfigKey("[Master]", "skin_settings"),
            m_pFeatureCOSkinSettings);

    // Microphone Section
    QString showMicrophoneTitle = tr("Show Microphone Section");
    QString showMicrophoneText = tr("Show the microphone section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowMicrophone = new QAction(showMicrophoneTitle, this);
    m_pViewShowMicrophone->setCheckable(true);
    m_pViewShowMicrophone->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowMicrophone"),
                    tr("Ctrl+2", "Menubar|View|Show Microphone Section"))));
    m_pViewShowMicrophone->setStatusTip(showMicrophoneText);
    m_pViewShowMicrophone->setWhatsThis(buildWhatsThis(showMicrophoneTitle, showMicrophoneText));
    createVisibilityControl(m_pViewShowMicrophone, ConfigKey("[Microphone]", "show_microphone"));

#ifdef __VINYLCONTROL__
    QString showVinylControlTitle = tr("Show Vinyl Control Section");
    QString showVinylControlText = tr("Show the vinyl control section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewVinylControl = new QAction(showVinylControlTitle, this);
    m_pViewVinylControl->setCheckable(true);
    m_pViewVinylControl->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowVinylControl"),
                    tr("Ctrl+3", "Menubar|View|Show Vinyl Control Section"))));
    m_pViewVinylControl->setStatusTip(showVinylControlText);
    m_pViewVinylControl->setWhatsThis(buildWhatsThis(showVinylControlTitle, showVinylControlText));
    createVisibilityControl(m_pViewVinylControl, ConfigKey(VINYL_PREF_KEY, "show_vinylcontrol"));
#endif

    QString showPreviewDeckTitle = tr("Show Preview Deck");
    QString showPreviewDeckText = tr("Show the preview deck in the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowPreviewDeck = new QAction(showPreviewDeckTitle, this);
    m_pViewShowPreviewDeck->setCheckable(true);
    m_pViewShowPreviewDeck->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowPreviewDeck"),
                    tr("Ctrl+4", "Menubar|View|Show Preview Deck"))));
    m_pViewShowPreviewDeck->setStatusTip(showPreviewDeckText);
    m_pViewShowPreviewDeck->setWhatsThis(buildWhatsThis(showPreviewDeckTitle, showPreviewDeckText));
    createVisibilityControl(m_pViewShowPreviewDeck, ConfigKey("[PreviewDeck]", "show_previewdeck"));

    QString showCoverArtTitle = tr("Show Cover Art");
    QString showCoverArtText = tr("Show cover art in the Mixxx interface.") +
            " " + mayNotBeSupported;
    m_pViewShowCoverArt = new QAction(showCoverArtTitle, this);
    m_pViewShowCoverArt->setCheckable(true);
    m_pViewShowCoverArt->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowCoverArt"),
                    tr("Ctrl+6", "Menubar|View|Show Cover Art"))));
    m_pViewShowCoverArt->setStatusTip(showCoverArtText);
    m_pViewShowCoverArt->setWhatsThis(buildWhatsThis(showCoverArtTitle, showCoverArtText));
    createVisibilityControl(m_pViewShowCoverArt, ConfigKey("[Library]", "show_coverart"));

    QString showMenubarTitle = tr("Show Menu Bar");
    QString showMenubarText = tr("Show traditional menu bar in Mixxx interface");
    m_pViewShowMenuBar = new QAction(showMenubarTitle, this);
    m_pViewShowMenuBar->setCheckable(true);
    m_pViewShowMenuBar->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowMenuBar"),
                    tr("Ctrl+7", "Menubar|View|Show Menu Bar"))));
    m_pViewShowMenuBar->setStatusTip(showMenubarText);
    m_pViewShowMenuBar->setWhatsThis(buildWhatsThis(showMenubarTitle, showMenubarText));
    m_pMenubarConnection = createVisibilityControl(
            m_pViewShowMenuBar, ConfigKey("[Skin]", "show_menubar"), m_pFeatureCOHideMenubar);
    connect(m_pViewShowMenuBar,
            &QAction::triggered,
            this,
            &WMainMenu::toggleMenubarVisible);

    QString maximizeLibraryTitle = tr("Maximize Library");
    QString maximizeLibraryText = tr("Maximize the track library to take up "
                                     "all the available screen space.") +
            " " + mayNotBeSupported;
    m_pViewMaximizeLibrary = new QAction(maximizeLibraryTitle, this);
    m_pViewMaximizeLibrary->setCheckable(true);
    m_pViewMaximizeLibrary->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_MaximizeLibrary"),
                    tr("Space", "Menubar|View|Maximize Library"))));
    m_pViewMaximizeLibrary->setStatusTip(maximizeLibraryText);
    m_pViewMaximizeLibrary->setWhatsThis(buildWhatsThis(maximizeLibraryTitle, maximizeLibraryText));
    createVisibilityControl(m_pViewMaximizeLibrary, ConfigKey("[Master]", "maximize_library"));

#ifdef __VINYLCONTROL__
    QString vinylControlText = tr(
            "Use timecoded vinyls on external turntables to control Mixxx");

    for (unsigned int i = 0; i < kMaximumVinylControlInputs; ++i) {
        QString vinylControlTitle = tr("Enable Vinyl Control &%1").arg(i + 1);
        auto vc_checkbox = new QAction(vinylControlTitle, this);
        vc_checkbox->setData(i + 1);
        m_vinylControlEnabledActions.push_back(vc_checkbox);

        QString binding = m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]",
                        QString("OptionsMenu_EnableVinyl%1").arg(i + 1)),
                vinylControlDefaultKeyBinding(i));
        if (!binding.isEmpty()) {
            vc_checkbox->setShortcut(QKeySequence(binding));
            vc_checkbox->setShortcutContext(Qt::ApplicationShortcut);
        }

        // Either check or uncheck the vinyl control menu item depending on what
        // it was saved as.
        vc_checkbox->setCheckable(true);
        vc_checkbox->setChecked(false);
        // The visibility of these actions is set in
        // WMainMenu::onNumberOfDecksChanged.
        vc_checkbox->setVisible(i + 1 <= m_lastNumPlayers);
        vc_checkbox->setStatusTip(vinylControlText);
        vc_checkbox->setWhatsThis(buildWhatsThis(vinylControlTitle,
                vinylControlText));
        connect(vc_checkbox, &QAction::triggered, this, [this, i] { emit toggleVinylControl(i); });
    }
#endif

    QString recordTitle = tr("&Record Mix");
    QString recordText = tr("Record your mix to a file");
    m_pOptionsRecord = new QAction(recordTitle, this);
    m_pOptionsRecord->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_RecordMix"),
                    tr("Ctrl+R"))));
    m_pOptionsRecord->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsRecord->setCheckable(true);
    m_pOptionsRecord->setStatusTip(recordText);
    m_pOptionsRecord->setWhatsThis(buildWhatsThis(recordTitle, recordText));
    connect(m_pOptionsRecord, SIGNAL(triggered(bool)), this, SIGNAL(toggleRecording(bool)));
    connect(this,
            SIGNAL(internalRecordingStateChange(bool)),
            m_pOptionsRecord,
            SLOT(setChecked(bool)));

#ifdef __BROADCAST__
    QString broadcastingTitle = tr("Enable Live &Broadcasting");
    QString broadcastingText = tr("Stream your mixes to a shoutcast or icecast server");
    m_pOptionsBroadcasting = new QAction(broadcastingTitle, this);
    m_pOptionsBroadcasting->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]",
                            "OptionsMenu_EnableLiveBroadcasting"),
                    tr("Ctrl+L"))));
    m_pOptionsBroadcasting->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsBroadcasting->setCheckable(true);
    m_pOptionsBroadcasting->setStatusTip(broadcastingText);
    m_pOptionsBroadcasting->setWhatsThis(buildWhatsThis(broadcastingTitle, broadcastingText));

    connect(m_pOptionsBroadcasting,
            SIGNAL(triggered(bool)),
            this,
            SIGNAL(toggleBroadcasting(bool)));
    connect(this,
            SIGNAL(internalBroadcastingStateChange(bool)),
            m_pOptionsBroadcasting,
            SLOT(setChecked(bool)));
#endif

    QString keyboardShortcutTitle = tr("Enable &Keyboard Shortcuts");
    QString keyboardShortcutText = tr("Toggles keyboard shortcuts on or off");
    bool keyboardShortcutsEnabled = m_pConfig->getValueString(
                                            ConfigKey("[Keyboard]", "Enabled")) == "1";
    m_pOptionsKeyboard = new QAction(keyboardShortcutTitle, this);
    m_pOptionsKeyboard->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_EnableShortcuts"),
                    tr("Ctrl+`"))));
    m_pOptionsKeyboard->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsKeyboard->setCheckable(true);
    m_pOptionsKeyboard->setChecked(keyboardShortcutsEnabled);
    m_pOptionsKeyboard->setStatusTip(keyboardShortcutText);
    m_pOptionsKeyboard->setWhatsThis(buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
    connect(m_pOptionsKeyboard,
            SIGNAL(triggered(bool)),
            this,
            SIGNAL(toggleKeyboardShortcuts(bool)));

    QString preferencesTitle = tr("&Preferences");
    QString preferencesText = tr("Change Mixxx settings (e.g. playback, MIDI, controls)");
    m_pOptionsPreferences = new QAction(preferencesTitle, this);
    m_pOptionsPreferences->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_Preferences"),
                    showPreferencesKeyBinding())));
    m_pOptionsPreferences->setShortcutContext(Qt::ApplicationShortcut);
    m_pOptionsPreferences->setStatusTip(preferencesText);
    m_pOptionsPreferences->setWhatsThis(buildWhatsThis(preferencesTitle, preferencesText));
    m_pOptionsPreferences->setMenuRole(QAction::PreferencesRole);
    connect(m_pOptionsPreferences, SIGNAL(triggered()), this, SIGNAL(showPreferences()));

    QString fullScreenTitle = tr("&Full Screen");
    QString fullScreenText = tr("Display Mixxx using the full screen");
    m_pViewFullScreen = new QAction(fullScreenTitle, this);
    QList<QKeySequence> shortcuts;
    // We use F11 _AND_ the OS shortcut only on Linux and Windows because on
    // newer macOS versions there might be issues with getting F11 working.
    // https://github.com/mixxxdj/mixxx/pull/3011#issuecomment-678678328
#ifndef __APPLE__
    shortcuts << QKeySequence("F11");
#endif
    QKeySequence osShortcut = QKeySequence::FullScreen;
    // Note(ronso0) Only add the OS shortcut if it's not empty and not F11.
    // In some Linux distros the window managers doesn't pass the OS fullscreen
    // key sequence to Mixxx for some reason.
    // Both adding an empty key sequence or the same sequence twice can render
    // the fullscreen shortcut nonfunctional.
    // https://bugs.launchpad.net/mixxx/+bug/1882474  PR #3011
    if (!osShortcut.isEmpty() && !shortcuts.contains(osShortcut)) {
        shortcuts << osShortcut;
    }
    m_pViewFullScreen->setShortcuts(shortcuts);
    m_pViewFullScreen->setShortcutContext(Qt::ApplicationShortcut);
    m_pViewFullScreen->setCheckable(true);
    m_pViewFullScreen->setChecked(false);
    m_pViewFullScreen->setStatusTip(fullScreenText);
    m_pViewFullScreen->setWhatsThis(buildWhatsThis(fullScreenTitle, fullScreenText));
    connect(m_pViewFullScreen, SIGNAL(triggered(bool)), this, SIGNAL(toggleFullScreen(bool)));
    connect(this,
            SIGNAL(internalFullScreenStateChange(bool)),
            m_pViewFullScreen,
            SLOT(setChecked(bool)));

    if (CmdlineArgs::Instance().getDeveloper()) {
        QString reloadSkinTitle = tr("&Reload Skin");
        QString reloadSkinText = tr("Reload the skin");
        m_pDeveloperReloadSkin = new QAction(reloadSkinTitle, this);
        m_pDeveloperReloadSkin->setShortcut(
                QKeySequence(m_pKbdConfig->getValue(
                        ConfigKey("[KeyboardShortcuts]", "OptionsMenu_ReloadSkin"),
                        tr("Ctrl+Shift+R"))));
        m_pDeveloperReloadSkin->setShortcutContext(Qt::ApplicationShortcut);
        m_pDeveloperReloadSkin->setStatusTip(reloadSkinText);
        m_pDeveloperReloadSkin->setWhatsThis(buildWhatsThis(reloadSkinTitle, reloadSkinText));
        connect(m_pDeveloperReloadSkin, SIGNAL(triggered()), this, SIGNAL(reloadSkin()));

        QString developerToolsTitle = tr("Developer &Tools");
        QString developerToolsText = tr("Opens the developer tools dialog");
        m_pDeveloperTools = new QAction(developerToolsTitle, this);
        m_pDeveloperTools->setShortcut(
                QKeySequence(m_pKbdConfig->getValue(
                        ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperTools"),
                        tr("Ctrl+Shift+T"))));
        m_pDeveloperTools->setShortcutContext(Qt::ApplicationShortcut);
        m_pDeveloperTools->setCheckable(true);
        m_pDeveloperTools->setChecked(false);
        m_pDeveloperTools->setStatusTip(developerToolsText);
        m_pDeveloperTools->setWhatsThis(buildWhatsThis(developerToolsTitle, developerToolsText));
        connect(m_pDeveloperTools,
                SIGNAL(triggered(bool)),
                this,
                SIGNAL(toggleDeveloperTools(bool)));
        connect(this,
                SIGNAL(internalDeveloperToolsStateChange(bool)),
                m_pDeveloperTools,
                SLOT(setChecked(bool)));

        QString enableExperimentTitle = tr("Stats: &Experiment Bucket");
        QString enableExperimentToolsText = tr(
                "Enables experiment mode. Collects stats in the EXPERIMENT tracking bucket.");
        m_pDeveloperStatsExperiment = new QAction(enableExperimentTitle, this);
        m_pDeveloperStatsExperiment->setShortcut(
                QKeySequence(m_pKbdConfig->getValue(
                        ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperStatsExperiment"),
                        tr("Ctrl+Shift+E"))));
        m_pDeveloperStatsExperiment->setShortcutContext(Qt::ApplicationShortcut);
        m_pDeveloperStatsExperiment->setStatusTip(enableExperimentToolsText);
        m_pDeveloperStatsExperiment->setWhatsThis(buildWhatsThis(
                enableExperimentTitle, enableExperimentToolsText));
        m_pDeveloperStatsExperiment->setCheckable(true);
        m_pDeveloperStatsExperiment->setChecked(Experiment::isExperiment());
        connect(m_pDeveloperStatsExperiment,
                SIGNAL(triggered(bool)),
                this,
                SLOT(slotDeveloperStatsExperiment(bool)));

        QString enableBaseTitle = tr("Stats: &Base Bucket");
        QString enableBaseToolsText = tr(
                "Enables base mode. Collects stats in the BASE tracking bucket.");
        m_pDeveloperStatsBase = new QAction(enableBaseTitle, this);
        m_pDeveloperStatsBase->setShortcut(
                QKeySequence(m_pKbdConfig->getValue(
                        ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperStatsBase"),
                        tr("Ctrl+Shift+B"))));
        m_pDeveloperStatsBase->setShortcutContext(Qt::ApplicationShortcut);
        m_pDeveloperStatsBase->setStatusTip(enableBaseToolsText);
        m_pDeveloperStatsBase->setWhatsThis(buildWhatsThis(
                enableBaseTitle, enableBaseToolsText));
        m_pDeveloperStatsBase->setCheckable(true);
        m_pDeveloperStatsBase->setChecked(Experiment::isBase());
        connect(m_pDeveloperStatsBase,
                SIGNAL(triggered(bool)),
                this,
                SLOT(slotDeveloperStatsBase(bool)));

        // "D" cannont be used with Alt here as it is already by the Developer menu
        QString scriptDebuggerTitle = tr("Deb&ugger Enabled");
        QString scriptDebuggerText = tr("Enables the debugger during skin parsing");
        bool scriptDebuggerEnabled = m_pConfig->getValueString(
                                             ConfigKey("[ScriptDebugger]", "Enabled")) == "1";
        m_pDeveloperDebugger = new QAction(scriptDebuggerTitle, this);
        m_pDeveloperDebugger->setShortcut(
                QKeySequence(m_pKbdConfig->getValue(
                        ConfigKey("[KeyboardShortcuts]", "DeveloperMenu_EnableDebugger"),
                        tr("Ctrl+Shift+D"))));
        m_pDeveloperDebugger->setShortcutContext(Qt::ApplicationShortcut);
        m_pDeveloperDebugger->setWhatsThis(
                buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
        m_pDeveloperDebugger->setCheckable(true);
        m_pDeveloperDebugger->setStatusTip(scriptDebuggerText);
        m_pDeveloperDebugger->setChecked(scriptDebuggerEnabled);
        connect(m_pDeveloperDebugger,
                SIGNAL(triggered(bool)),
                this,
                SLOT(slotDeveloperDebugger(bool)));
    }

    QString externalLinkSuffix = " =>";

    QString supportTitle = tr("&Community Support") + externalLinkSuffix;
    QString supportText = tr("Get help with Mixxx");
    m_pHelpSupport = new QAction(supportTitle, this);
    m_pHelpSupport->setStatusTip(supportText);
    m_pHelpSupport->setWhatsThis(buildWhatsThis(supportTitle, supportText));
    connect(m_pHelpSupport, &QAction::triggered, this, [this] { slotVisitUrl(MIXXX_SUPPORT_URL); });

    QDir resourceDir(m_pConfig->getResourcePath());
    // Default to the mixxx.org hosted version of the manual.
    QUrl qManualUrl(MIXXX_MANUAL_URL);
#if defined(__APPLE__)
    // FIXME: We don't include the PDF manual in the bundle on OSX.
    // Default to the web-hosted version.
#elif defined(__WINDOWS__)
    // On Windows, the manual PDF sits in the same folder as the 'skins' folder.
    if (resourceDir.exists(MIXXX_MANUAL_FILENAME)) {
        qManualUrl = QUrl::fromLocalFile(
                resourceDir.absoluteFilePath(MIXXX_MANUAL_FILENAME));
    }
#elif defined(__LINUX__)
    // On GNU/Linux, the manual is installed to e.g. /usr/share/mixxx/doc/
    if (resourceDir.cd("../doc/mixxx") && resourceDir.exists(MIXXX_MANUAL_FILENAME)) {
        qManualUrl = QUrl::fromLocalFile(
                resourceDir.absoluteFilePath(MIXXX_MANUAL_FILENAME));
    }
#else
    // No idea, default to the mixxx.org hosted version.
#endif

    QString manualTitle = tr("&User Manual") + externalLinkSuffix;
    QString manualText = tr("Read the Mixxx user manual.");
    m_pHelpManual = new QAction(manualTitle, this);
    m_pHelpManual->setStatusTip(manualText);
    m_pHelpManual->setWhatsThis(buildWhatsThis(manualTitle, manualText));
    connect(m_pHelpManual, &QAction::triggered, this, [this, qManualUrl] {
        slotVisitUrl(qManualUrl.toString());
    });

    QString shortcutsTitle = tr("&Keyboard Shortcuts") + externalLinkSuffix;
    QString shortcutsText = tr("Speed up your workflow with keyboard shortcuts.");
    m_pHelpShortcuts = new QAction(shortcutsTitle, this);
    m_pHelpShortcuts->setStatusTip(shortcutsText);
    m_pHelpShortcuts->setWhatsThis(buildWhatsThis(shortcutsTitle, shortcutsText));
    connect(m_pHelpShortcuts, &QAction::triggered, this, [this] {
        slotVisitUrl(MIXXX_MANUAL_SHORTCUTS_URL);
    });

    QString feedbackTitle = tr("Send Us &Feedback") + externalLinkSuffix;
    QString feedbackText = tr("Send feedback to the Mixxx team.");
    m_pHelpFeedback = new QAction(feedbackTitle, this);
    m_pHelpFeedback->setStatusTip(feedbackText);
    m_pHelpFeedback->setWhatsThis(buildWhatsThis(feedbackTitle, feedbackText));
    connect(m_pHelpFeedback, &QAction::triggered, this, [this] {
        slotVisitUrl(MIXXX_FEEDBACK_URL);
    });

    QString translateTitle = tr("&Translate This Application") + externalLinkSuffix;
    QString translateText = tr("Help translate this application into your language.");
    m_pHelpTranslation = new QAction(translateTitle, this);
    m_pHelpTranslation->setStatusTip(translateText);
    m_pHelpTranslation->setWhatsThis(buildWhatsThis(translateTitle, translateText));
    connect(m_pHelpTranslation, &QAction::triggered, this, [this] {
        slotVisitUrl(MIXXX_TRANSLATION_URL);
    });

    QString aboutTitle = tr("&About");
    QString aboutText = tr("About the application");
    m_pHelpAboutApp = new QAction(aboutTitle, this);
    m_pHelpAboutApp->setStatusTip(aboutText);
    m_pHelpAboutApp->setWhatsThis(buildWhatsThis(aboutTitle, aboutText));
    m_pHelpAboutApp->setMenuRole(QAction::AboutRole);
    connect(m_pHelpAboutApp, SIGNAL(triggered()), this, SIGNAL(showAbout()));
}

QMenuBar* WMainMenu::createMainMenuBar(QMainWindow* parent, bool native) {
    qDebug() << "createMainMenuBar: native" << native;
    auto pMainMenuBar = new QMenuBar(parent);
    parent->setMenuBar(pMainMenuBar);
    pMainMenuBar->setObjectName(QStringLiteral("MainMenuBar"));

    // why ?
    QPalette Pal(palette());
    // safe default QMenuBar background
    QColor MenuBarBackground(pMainMenuBar->palette().color(QPalette::Window));
    Pal.setColor(QPalette::Window, QColor(0x202020));
    setAutoFillBackground(true);
    setPalette(Pal);
    // restore default QMenuBar background
    Pal.setColor(QPalette::Window, MenuBarBackground);
    pMainMenuBar->setPalette(Pal);

    createMenuInternal([pMainMenuBar](QMenu* menu, QAction* action, bool separator) {
        if (menu) {
            //    menu->setParent(pMainMenuBar);
            //   pMainMenuBar->addMenu(menu);
        }

        Q_UNUSED(menu);
        Q_UNUSED(action);
        Q_UNUSED(separator);
    },
            pMainMenuBar,
            this);
    pMainMenuBar->setNativeMenuBar(native);
    return pMainMenuBar;
}

void WMainMenu::createMenu(FnAddMenu fnAddMenu, QWidget* parent) {
    createMenuInternal(fnAddMenu, nullptr, parent);
}

void WMainMenu::createMenuInternal(FnAddMenu fnAddMenu, QMenuBar* pMenuBar, QWidget* parent) {
    // FILE MENU
    QMenu* pFileMenu;
    if (pMenuBar) {
        pFileMenu = pMenuBar->addMenu(tr("&File"));
    } else {
        pFileMenu = new QMenu(tr("&File"), parent);
    }

    for (unsigned int deck = 0; deck < kMaxLoadToDeckActions; ++deck) {
        pFileMenu->addAction(m_loadToDeckActions.at(deck));
    }
    pFileMenu->addSeparator();
    // exit button is at the end of main menu in WMainMenuButton
    if (pMenuBar) {
        pFileMenu->addAction(m_pFileQuit);
    }

    fnAddMenu(pFileMenu, nullptr, false);
    // LIBRARY MENU
    QMenu* pLibraryMenu;
    if (pMenuBar) {
        pLibraryMenu = pMenuBar->addMenu(tr("&Library"));
    } else {
        pLibraryMenu = new QMenu(tr("&Library"), parent);
    }

    pLibraryMenu->addAction(m_pLibraryRescan);
    pLibraryMenu->addSeparator();
    pLibraryMenu->addAction(m_pLibraryCreatePlaylist);
    pLibraryMenu->addAction(m_pLibraryCreateCrate);

    fnAddMenu(pLibraryMenu, nullptr, false);

#if defined(__APPLE__)
    // Note: On macOS 10.11 ff. we have to deal with "automagic" menu items,
    // when ever a menu "View" is present. QT (as of 5.12.3) does not handle this for us.
    // Add an invisible suffix to the View item string so it doesn't string-equal "View" ,
    // and the magic menu items won't get injected.
    // https://bugs.launchpad.net/mixxx/+bug/1534292
    QString viewTitle = tr("&View") + QStringLiteral("\u200C");
#else
    QString viewTitle = tr("&View") + QStringLiteral("\u200C");
#endif

    QMenu* pViewMenu;
    if (pMenuBar) {
        pViewMenu = pMenuBar->addMenu(viewTitle);
    } else {
        pViewMenu = new QMenu(viewTitle, parent);
    }

    pViewMenu->addAction(m_pViewShowSkinSettings);
    pViewMenu->addAction(m_pViewShowMicrophone);
#ifdef __VINYLCONTROL__
    pViewMenu->addAction(m_pViewVinylControl);
#endif
    pViewMenu->addAction(m_pViewShowPreviewDeck);
    pViewMenu->addAction(m_pViewShowCoverArt);
    pViewMenu->addAction(m_pViewShowMenuBar);
    pViewMenu->addAction(m_pViewMaximizeLibrary);
    pViewMenu->addSeparator();
    pViewMenu->addAction(m_pViewFullScreen);

    fnAddMenu(pViewMenu, nullptr, false);

    // OPTIONS MENU
    QMenu* pOptionsMenu;
    if (pMenuBar) {
        pOptionsMenu = pMenuBar->addMenu(tr("&Options"));
    } else {
        pOptionsMenu = new QMenu(tr("&Options"), parent);
    }
#ifdef __VINYLCONTROL__
    QMenu* pVinylControlMenu = new QMenu(tr("&Vinyl Control"));
    for (unsigned int i = 0; i < kMaximumVinylControlInputs; ++i) {
        pVinylControlMenu->addAction(m_vinylControlEnabledActions.at(i));
    }
    pOptionsMenu->addMenu(pVinylControlMenu);
    pOptionsMenu->addSeparator();
#endif
    pOptionsMenu->addAction(m_pOptionsRecord);
#ifdef __BROADCAST__
    pOptionsMenu->addAction(m_pOptionsBroadcasting);
#endif
    pOptionsMenu->addSeparator();
    pOptionsMenu->addAction(m_pOptionsKeyboard);
    pOptionsMenu->addSeparator();
    pOptionsMenu->addAction(m_pOptionsPreferences);

    fnAddMenu(pOptionsMenu, nullptr, false);

    // DEVELOPER MENU
    if (CmdlineArgs::Instance().getDeveloper()) {
        QMenu* pDeveloperMenu = new QMenu(tr("&Developer"));

        pDeveloperMenu->addAction(m_pDeveloperReloadSkin);
        pDeveloperMenu->addAction(m_pDeveloperTools);
        pDeveloperMenu->addAction(m_pDeveloperStatsExperiment);
        pDeveloperMenu->addAction(m_pDeveloperStatsBase);
        pDeveloperMenu->addAction(m_pDeveloperDebugger);

        fnAddMenu(pDeveloperMenu, nullptr, false);
    }

    // HELP MENU
    QMenu* pHelpMenu;
    if (pMenuBar) {
        pHelpMenu = pMenuBar->addMenu(tr("&Help"));
    } else {
        pHelpMenu = new QMenu(tr("&Help"), parent);
    }

    pHelpMenu->addAction(m_pHelpSupport);

    pHelpMenu->addAction(m_pHelpManual);
    pHelpMenu->addAction(m_pHelpShortcuts);
    pHelpMenu->addAction(m_pHelpShortcuts);
    pHelpMenu->addAction(m_pHelpTranslation);
    pHelpMenu->addSeparator();
    pHelpMenu->addAction(m_pHelpAboutApp);

    fnAddMenu(pHelpMenu, nullptr, false);

    if (!pMenuBar) {
        fnAddMenu(nullptr, nullptr, true);
        fnAddMenu(nullptr, m_pFileQuit, false);
    }

    // we already know the num
    if (m_lastNumPlayers) {
        onNumberOfDecksChanged(m_lastNumPlayers);
    }
}

void WMainMenu::resetFeatureFlags() {
    m_pFeatureCOHideMenubar->set(0.0);
    m_pFeatureCOSkinSettings->set(0.0);
}

bool WMainMenu::shouldBeVisible() {
    if (CmdlineArgs::Instance().getSafeMode()) {
        return true;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pMenubarConnection) {
        return true;
    }
    if (!m_pFeatureCOHideMenubar->toBool()) {
        return true;
    }
    return m_pMenubarConnection->value() > 0.0;
}

void WMainMenu::onLibraryScanStarted() {
    emit internalLibraryScanActive(true);
}

void WMainMenu::onLibraryScanFinished() {
    emit internalLibraryScanActive(false);
}

void WMainMenu::onNewSkinLoaded() {
    emit internalOnNewSkinLoaded();
    // The skin defines if it supports hiding the menubar
    emit toggleMenubarVisible(shouldBeVisible());
}

void WMainMenu::onNewSkinAboutToLoad() {
    resetFeatureFlags();
    emit internalOnNewSkinAboutToLoad();
}

void WMainMenu::onRecordingStateChange(bool recording) {
    emit internalRecordingStateChange(recording);
}

void WMainMenu::onBroadcastingStateChange(bool broadcasting) {
    emit internalBroadcastingStateChange(broadcasting);
}

void WMainMenu::onDeveloperToolsShown() {
    emit internalDeveloperToolsStateChange(true);
}

void WMainMenu::onDeveloperToolsHidden() {
    emit internalDeveloperToolsStateChange(false);
}

void WMainMenu::onFullScreenStateChange(bool fullscreen) {
    emit internalFullScreenStateChange(fullscreen);
}

void WMainMenu::onVinylControlDeckEnabledStateChange(int deck, bool enabled) {
    if (deck < 0 || deck >= m_vinylControlEnabledActions.size()) {
        DEBUG_ASSERT(false);
        return;
    }
    m_vinylControlEnabledActions.at(deck)->setChecked(enabled);
}

void WMainMenu::slotDeveloperStatsBase(bool enable) {
    if (enable) {
        Experiment::setBase();
    } else {
        Experiment::disable();
    }
}

void WMainMenu::slotDeveloperStatsExperiment(bool enable) {
    if (enable) {
        Experiment::setExperiment();
    } else {
        Experiment::disable();
    }
}

void WMainMenu::slotDeveloperDebugger(bool toggle) {
    m_pConfig->set(ConfigKey("[ScriptDebugger]", "Enabled"),
            ConfigValue(toggle ? 1 : 0));
}

void WMainMenu::slotVisitUrl(const QString& url) {
    QDesktopServices::openUrl(QUrl(url));
}

void WMainMenu::finalize() {
    for (VisibilityControlConnection* connection : qAsConst(m_visibilityConnections)) {
        disconnect(connection, nullptr, nullptr, nullptr);
        disconnect(this, nullptr, connection, nullptr);
        connection->slotClearControl();
    }
}

VisibilityControlConnection* WMainMenu::createVisibilityControl(QAction* pAction,
        const ConfigKey& key,
        ControlObject* feature) {
    auto pConnection = new VisibilityControlConnection(this, pAction, key, feature);
    connect(this,
            &WMainMenu::internalOnNewSkinLoaded,
            pConnection,
            &VisibilityControlConnection::slotReconnectControl);
    connect(this,
            &WMainMenu::internalOnNewSkinAboutToLoad,
            pConnection,
            &VisibilityControlConnection::slotClearControl);
    m_visibilityConnections.push_back(pConnection);
    return pConnection;
}

void WMainMenu::onNumberOfDecksChanged(int decks) {
    m_lastNumPlayers = decks;
    for (QAction* pVinylControlEnabled : m_vinylControlEnabledActions) {
        pVinylControlEnabled->setVisible(pVinylControlEnabled->data().toUInt() <= m_lastNumPlayers);
    }
    for (QAction* pLoadToDeck : m_loadToDeckActions) {
        pLoadToDeck->setVisible(pLoadToDeck->data().toUInt() <= m_lastNumPlayers);
    }
}

VisibilityControlConnection::VisibilityControlConnection(
        QObject* pParent, QAction* pAction, const ConfigKey& key, ControlObject* feature)
        : QObject(pParent),
          m_key(key),
          m_pCOFeature(feature),
          m_pAction(pAction) {
    connect(m_pAction, SIGNAL(triggered(bool)), this, SLOT(slotActionToggled(bool)));
}

double VisibilityControlConnection::value() {
    return m_pControl ? m_pControl->get() : 0.0;
}

bool VisibilityControlConnection::valid() {
    return m_pControl ? m_pControl->valid() : false;
}

void VisibilityControlConnection::slotClearControl() {
    m_pControl.reset();
    m_pAction->setEnabled(false);
}

void VisibilityControlConnection::slotReconnectControl() {
    m_pControl.reset(new ControlProxy(m_key, this, ControlFlag::NoAssertIfMissing));
    m_pControl->connectValueChanged(this, &VisibilityControlConnection::slotControlChanged);
    if (!m_pControl->valid()) {
        m_pAction->setEnabled(false);
    } else if (m_pCOFeature) {
        qDebug() << "check Skin Feature" << m_pCOFeature->toBool();
        m_pAction->setEnabled(m_pCOFeature->toBool());
    } else {
        m_pAction->setEnabled(true);
    }
    slotControlChanged();
}

void VisibilityControlConnection::slotControlChanged() {
    if (m_pControl) {
        m_pAction->setChecked(m_pControl->toBool());
    }
}

void VisibilityControlConnection::slotActionToggled(bool toggle) {
    if (m_pControl) {
        m_pControl->set(toggle ? 1.0 : 0.0);
    }
}
