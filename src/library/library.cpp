// library.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTranslator>
#include <QDir>
#include <QPointer>

#include "database/mixxxdb.h"

#include "library/library.h"
#include "library/library_preferences.h"
#include "library/librarycontrol.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/externaltrackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"

#include "library/autodj/autodjfeature.h"
#include "library/banshee/bansheefeature.h"
#include "library/browse/browsefeature.h"
#include "library/crate/cratefeature.h"
#include "library/itunes/itunesfeature.h"
#include "library/mixxxlibraryfeature.h"
#include "library/playlistfeature.h"
#include "library/recording/recordingfeature.h"
#include "library/rhythmbox/rhythmboxfeature.h"
#include "library/setlogfeature.h"
#include "library/traktor/traktorfeature.h"
#include "library/rekordbox/rekordboxfeature.h"
#include "library/analysisfeature.h"

#include "mixer/playermanager.h"

#include "recording/recordingmanager.h"

#include "util/db/dbconnectionpooled.h"
#include "util/sandbox.h"
#include "util/logger.h"
#include "util/assert.h"

#include "widget/wtracktableview.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wsearchlineedit.h"

#include "controllers/keyboard/keyboardeventfilter.h"

namespace {

const mixxx::Logger kLogger("Library");

} // anonymous namespace

//static
const QString Library::kConfigGroup("[Library]");

// This is the name which we use to register the WTrackTableView with the
// WLibrary
const QString Library::m_sTrackViewName = QString("WTrackTableView");

// The default row height of the library.
const int Library::kDefaultRowHeightPx = 20;

Library::Library(
        QObject* parent,
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        TrackCollectionManager* pTrackCollectionManager,
        PlayerManager* pPlayerManager,
        RecordingManager* pRecordingManager)
    : QObject(parent),
      m_pConfig(pConfig),
      m_pDbConnectionPool(std::move(pDbConnectionPool)),
      m_pTrackCollectionManager(pTrackCollectionManager),
      m_pSidebarModel(make_parented<SidebarModel>(this)),
      m_pLibraryControl(make_parented<LibraryControl>(this)),
      m_pMixxxLibraryFeature(nullptr),
      m_pPlaylistFeature(nullptr),
      m_pCrateFeature(nullptr),
      m_pAnalysisFeature(nullptr) {

    qRegisterMetaType<Library::RemovalType>("Library::RemovalType");

    m_pKeyNotation.reset(new ControlObject(ConfigKey(kConfigGroup, "key_notation")));

    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanFinished,
            this,
            &Library::slotRefreshLibraryModels);

    // TODO(rryan) -- turn this construction / adding of features into a static
    // method or something -- CreateDefaultLibrary
    m_pMixxxLibraryFeature = new MixxxLibraryFeature(
            this,
            m_pConfig);
    addFeature(m_pMixxxLibraryFeature);

    addFeature(new AutoDJFeature(this, m_pConfig, pPlayerManager));
    m_pPlaylistFeature = new PlaylistFeature(this, UserSettingsPointer(m_pConfig));
    addFeature(m_pPlaylistFeature);
    m_pCrateFeature = new CrateFeature(this, m_pConfig);
    addFeature(m_pCrateFeature);

    BrowseFeature* browseFeature = new BrowseFeature(
        this, m_pConfig, pRecordingManager);
    connect(browseFeature,
            &BrowseFeature::scanLibrary,
            m_pTrackCollectionManager,
            &TrackCollectionManager::startLibraryScan);
    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanStarted,
            browseFeature,
            &BrowseFeature::slotLibraryScanStarted);
    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanFinished,
            browseFeature,
            &BrowseFeature::slotLibraryScanFinished);
    addFeature(browseFeature);

    addFeature(new RecordingFeature(this, m_pConfig, pRecordingManager));
    addFeature(new SetlogFeature(this, UserSettingsPointer(m_pConfig)));

    m_pAnalysisFeature = new AnalysisFeature(this, m_pConfig);
    connect(m_pPlaylistFeature, &PlaylistFeature::analyzeTracks,
            m_pAnalysisFeature, &AnalysisFeature::analyzeTracks);
    connect(m_pCrateFeature, &CrateFeature::analyzeTracks,
            m_pAnalysisFeature, &AnalysisFeature::analyzeTracks);
    addFeature(m_pAnalysisFeature);
    // Suspend a batch analysis while an ad-hoc analysis of
    // loaded tracks is in progress and resume it afterwards.
    connect(pPlayerManager, &PlayerManager::trackAnalyzerProgress,
            this, &Library::onPlayerManagerTrackAnalyzerProgress);
    connect(pPlayerManager, &PlayerManager::trackAnalyzerIdle,
            this, &Library::onPlayerManagerTrackAnalyzerIdle);

    //iTunes and Rhythmbox should be last until we no longer have an obnoxious
    //messagebox popup when you select them. (This forces you to reach for your
    //mouse or keyboard if you're using MIDI control and you scroll through them...)
    if (RhythmboxFeature::isSupported() &&
        m_pConfig->getValue(ConfigKey(kConfigGroup,"ShowRhythmboxLibrary"), true)) {
        addFeature(new RhythmboxFeature(this, m_pConfig));
    }
    if (m_pConfig->getValue(ConfigKey(kConfigGroup,"ShowBansheeLibrary"), true)) {
        BansheeFeature::prepareDbPath(m_pConfig);
        if (BansheeFeature::isSupported()) {
            addFeature(new BansheeFeature(this, m_pConfig));
        }
    }
    if (ITunesFeature::isSupported() &&
        m_pConfig->getValue(ConfigKey(kConfigGroup,"ShowITunesLibrary"), true)) {
        addFeature(new ITunesFeature(this, m_pConfig));
    }
    if (TraktorFeature::isSupported() &&
        m_pConfig->getValue(ConfigKey(kConfigGroup,"ShowTraktorLibrary"), true)) {
        addFeature(new TraktorFeature(this, m_pConfig));
    }

    // TODO(XXX) Rekordbox feature added persistently as the only way to enable it to
    // dynamically appear/disappear when correctly prepared removable devices
    // are mounted/unmounted would be to have some form of timed thread to check
    // periodically. Not ideal perfomance wise.
    if (m_pConfig->getValue(ConfigKey(kConfigGroup, "ShowRekordboxLibrary"), true)) {
        addFeature(new RekordboxFeature(this, m_pConfig));
    }

    for (const auto& externalTrackCollection : m_pTrackCollectionManager->externalCollections()) {
        auto feature = externalTrackCollection->newLibraryFeature(this, m_pConfig);
        if (feature) {
            kLogger.info()
                    << "Adding library feature for"
                    << externalTrackCollection->name();
            addFeature(feature);
        } else {
            kLogger.info()
                    << "Library feature for"
                    << externalTrackCollection->name()
                    << "is not available";
        }
    }

    // On startup we need to check if all of the user's library folders are
    // accessible to us. If the user is using a database from <1.12.0 with
    // sandboxing then we will need them to give us permission.
    qDebug() << "Checking for access to user's library directories:";
    foreach (QString directoryPath, getDirs()) {
        QFileInfo directory(directoryPath);
        bool hasAccess = Sandbox::askForAccess(directory.canonicalFilePath());
        qDebug() << "Checking for access to" << directoryPath << ":" << hasAccess;
    }

    m_iTrackTableRowHeight = m_pConfig->getValue(
            ConfigKey(kConfigGroup, "RowHeight"), kDefaultRowHeightPx);
    QString fontStr = m_pConfig->getValueString(ConfigKey(kConfigGroup, "Font"));
    if (!fontStr.isEmpty()) {
        m_trackTableFont.fromString(fontStr);
    } else {
        m_trackTableFont = QApplication::font();
    }

    m_editMetadataSelectedClick = m_pConfig->getValue(
            ConfigKey(kConfigGroup, "EditMetadataSelectedClick"),
            PREF_LIBRARY_EDIT_METADATA_DEFAULT);
}

Library::~Library() {
    // Empty but required due to forward declarations in header file!
}

TrackCollectionManager* Library::trackCollections() const {
    // Cannot be implemented inline due to forward declarations
    return m_pTrackCollectionManager;
}

void Library::stopPendingTasks() {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->stopAnalysis();
        m_pAnalysisFeature = nullptr;
    }
}

void Library::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    m_pLibraryControl->bindSidebarWidget(pSidebarWidget);

    // Setup the sources view
    pSidebarWidget->setModel(m_pSidebarModel);
    connect(m_pSidebarModel,
            &SidebarModel::selectIndex,
            pSidebarWidget,
            &WLibrarySidebar::selectIndex);
    connect(pSidebarWidget,
            &WLibrarySidebar::pressed,
            m_pSidebarModel,
            &SidebarModel::pressed);
    connect(pSidebarWidget,
            &WLibrarySidebar::clicked,
            m_pSidebarModel,
            &SidebarModel::clicked);
    // Lazy model: Let triangle symbol increment the model
    connect(pSidebarWidget,
            &WLibrarySidebar::expanded,
            m_pSidebarModel,
            &SidebarModel::doubleClicked);

    connect(pSidebarWidget,
            &WLibrarySidebar::rightClicked,
            m_pSidebarModel,
            &SidebarModel::rightClicked);

    pSidebarWidget->slotSetFont(m_trackTableFont);
    connect(this,
            &Library::setTrackTableFont,
            pSidebarWidget,
            &WLibrarySidebar::slotSetFont);


    for (const auto& feature : m_features) {
        feature->bindSidebarWidget(pSidebarWidget);
    }
}

void Library::bindLibraryWidget(WLibrary* pLibraryWidget,
                         KeyboardEventFilter* pKeyboard) {
    WTrackTableView* pTrackTableView =
            new WTrackTableView(
                    pLibraryWidget,
                    m_pConfig,
                    m_pTrackCollectionManager,
                    true);
    pTrackTableView->installEventFilter(pKeyboard);
    connect(this,
            &Library::showTrackModel,
            pTrackTableView,
            &WTrackTableView::loadTrackModel);
    connect(pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &Library::slotLoadTrack);
    connect(pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &Library::slotLoadTrackToPlayer);
    pLibraryWidget->registerView(m_sTrackViewName, pTrackTableView);

    connect(this,
            &Library::switchToView,
            pLibraryWidget,
            &WLibrary::switchToView);

    connect(pTrackTableView,
            &WTrackTableView::trackSelected,
            this,
            &Library::trackSelected);

    connect(this,
            &Library::setTrackTableFont,
            pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(this,
            &Library::setTrackTableRowHeight,
            pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(this,
            &Library::setSelectedClick,
            pTrackTableView,
            &WTrackTableView::setSelectedClick);

    m_pLibraryControl->bindLibraryWidget(pLibraryWidget, pKeyboard);

    for (const auto& feature : m_features) {
        feature->bindLibraryWidget(pLibraryWidget, pKeyboard);
    }

    // Set the current font and row height on all the WTrackTableViews that were
    // just connected to us.
    emit(setTrackTableFont(m_trackTableFont));
    emit(setTrackTableRowHeight(m_iTrackTableRowHeight));
    emit(setSelectedClick(m_editMetadataSelectedClick));
}

void Library::addFeature(LibraryFeature* feature) {
    VERIFY_OR_DEBUG_ASSERT(feature) {
        return;
    }
    m_features.push_back(feature);
    m_pSidebarModel->addLibraryFeature(feature);
    connect(feature,
            &LibraryFeature::showTrackModel,
            this,
            &Library::slotShowTrackModel);
    connect(feature,
            &LibraryFeature::switchToView,
            this,
            &Library::slotSwitchToView);
    connect(feature,
            &LibraryFeature::loadTrack,
            this,
            &Library::slotLoadTrack);
    connect(feature,
            &LibraryFeature::loadTrackToPlayer,
            this,
            &Library::slotLoadTrackToPlayer);
    connect(feature,
            &LibraryFeature::restoreSearch,
            this,
            &Library::slotRestoreSearch);
    connect(feature,
            &LibraryFeature::disableSearch,
            this,
            &Library::slotDisableSearch);
    connect(feature,
            &LibraryFeature::enableCoverArtDisplay,
            this,
            &Library::enableCoverArtDisplay);
    connect(feature,
            &LibraryFeature::trackSelected,
            this,
            &Library::trackSelected);
}

void Library::onPlayerManagerTrackAnalyzerProgress(
        TrackId /*trackId*/,AnalyzerProgress /*analyzerProgress*/) {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->suspendAnalysis();
    }
}

void Library::onPlayerManagerTrackAnalyzerIdle() {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->resumeAnalysis();
    }
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    //qDebug() << "Library::slotShowTrackModel" << model;
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    emit(showTrackModel(model));
    emit(switchToView(m_sTrackViewName));
    emit(restoreSearch(trackModel->currentSearch()));
}

void Library::slotSwitchToView(const QString& view) {
    //qDebug() << "Library::slotSwitchToView" << view;
    emit(switchToView(view));
}

void Library::slotLoadTrack(TrackPointer pTrack) {
    emit(loadTrack(pTrack));
}

void Library::slotLoadLocationToPlayer(QString location, QString group) {
    auto trackRef = TrackRef::fromFileInfo(location);
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(trackRef);
    if (pTrack) {
        emit(loadTrackToPlayer(pTrack, group));
    }
}

void Library::slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play) {
    emit(loadTrackToPlayer(pTrack, group, play));
}

void Library::slotRestoreSearch(const QString& text) {
    emit restoreSearch(text);
}

void Library::slotDisableSearch() {
    emit disableSearch();
}

void Library::slotRefreshLibraryModels() {
   m_pMixxxLibraryFeature->refreshLibraryModels();
   m_pAnalysisFeature->refreshLibraryModels();
}

void Library::slotCreatePlaylist() {
    m_pPlaylistFeature->slotCreatePlaylist();
}

void Library::slotCreateCrate() {
    m_pCrateFeature->slotCreateCrate();
}

void Library::onSkinLoadFinished() {
    // Enable the default selection when a new skin is loaded.
    m_pSidebarModel->activateDefaultSelection();
}

void Library::slotRequestAddDir(QString dir) {
    // We only call this method if the user has picked a new directory via a
    // file dialog. This means the system sandboxer (if we are sandboxed) has
    // granted us permission to this folder. Create a security bookmark while we
    // have permission so that we can access the folder on future runs. We need
    // to canonicalize the path so we first wrap the directory string with a
    // QDir.
    QDir directory(dir);
    Sandbox::createSecurityToken(directory);

    if (!m_pTrackCollectionManager->addDirectory(dir)) {
        QMessageBox::information(0, tr("Add Directory to Library"),
                tr("Could not add the directory to your library. Either this "
                    "directory is already in your library or you are currently "
                    "rescanning your library."));
    }
    // set at least one directory in the config file so that it will be possible
    // to downgrade from 1.12
    if (m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR).length() < 1) {
        m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, dir);
    }
}

void Library::slotRequestRemoveDir(QString dir, RemovalType removalType) {
    switch (removalType) {
        case Library::HideTracks:
            // Mark all tracks in this directory as deleted but DON'T purge them
            // in case the user re-adds them manually.
            m_pTrackCollectionManager->hideAllTracks(dir);
            break;
        case Library::PurgeTracks:
            // The user requested that we purge all metadata.
            m_pTrackCollectionManager->purgeAllTracks(dir);
            break;
        case Library::LeaveTracksUnchanged:
        default:
            break;

    }

    // Remove the directory from the directory list.
    m_pTrackCollectionManager->removeDirectory(dir);

    // Also update the config file if necessary so that downgrading is still
    // possible.
    QString confDir = m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR);

    if (QDir(dir) == QDir(confDir)) {
        QStringList dirList = getDirs();
        if (!dirList.isEmpty()) {
            m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, dirList.first());
        } else {
            // Save empty string so that an old version of mixxx knows it has to
            // ask for a new directory.
            m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, QString());
        }
    }
}

void Library::slotRequestRelocateDir(QString oldDir, QString newDir) {
    m_pTrackCollectionManager->relocateDirectory(oldDir, newDir);

    // also update the config file if necessary so that downgrading is still
    // possible
    QString conDir = m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR);
    if (oldDir == conDir) {
        m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, newDir);
    }
}

QStringList Library::getDirs() {
    return m_pTrackCollectionManager->internalCollection()->getDirectoryDAO().getDirs();
}

void Library::setFont(const QFont& font) {
    m_trackTableFont = font;
    emit(setTrackTableFont(font));
}

void Library::setRowHeight(int rowHeight) {
    m_iTrackTableRowHeight = rowHeight;
    emit(setTrackTableRowHeight(rowHeight));
}

void Library::setEditMedatataSelectedClick(bool enabled) {
    m_editMetadataSelectedClick = enabled;
    emit(setSelectedClick(enabled));
}

TrackCollection& Library::trackCollection() {
    DEBUG_ASSERT(m_pTrackCollectionManager);
    DEBUG_ASSERT(m_pTrackCollectionManager->internalCollection());
    return *m_pTrackCollectionManager->internalCollection();
}
