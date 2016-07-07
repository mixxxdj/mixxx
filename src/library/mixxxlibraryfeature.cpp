// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/parser.h"
#include "library/library.h"
#include "library/basetrackcache.h"
#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/hiddentablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/dlghidden.h"
#include "library/dlgmissing.h"
#include "treeitem.h"
#include "sources/soundsourceproxy.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarystack.h"
#include "widget/wlibrarysidebar.h"
#include "util/dnd.h"

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          kLibraryTitle(tr("Library")),
          kHiddenTitle(tr("Hidden Tracks")),
          kMissingTitle(tr("Missing Tracks")),
          m_pHiddenView(nullptr),
          m_pMissingView(nullptr),
          m_idExpandedHidden(-1),
          m_idExpandedMissing(-1),
          m_idExpandedControls(-1),
          m_idExpandedTree(-1),
          m_pHiddenTableModel(nullptr),
          m_pMissingTableModel(nullptr),
          m_pExpandedStack(nullptr),
          m_pSidebarTab(nullptr),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_pTrackCollection(pTrackCollection) {
    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "library." + LIBRARYTABLE_PLAYED
            << "library." + LIBRARYTABLE_TIMESPLAYED
            //has to be up here otherwise Played and TimesPlayed are not show
            << "library." + LIBRARYTABLE_ALBUMARTIST
            << "library." + LIBRARYTABLE_ALBUM
            << "library." + LIBRARYTABLE_ARTIST
            << "library." + LIBRARYTABLE_TITLE
            << "library." + LIBRARYTABLE_YEAR
            << "library." + LIBRARYTABLE_RATING
            << "library." + LIBRARYTABLE_GENRE
            << "library." + LIBRARYTABLE_COMPOSER
            << "library." + LIBRARYTABLE_GROUPING
            << "library." + LIBRARYTABLE_TRACKNUMBER
            << "library." + LIBRARYTABLE_KEY
            << "library." + LIBRARYTABLE_KEY_ID
            << "library." + LIBRARYTABLE_BPM
            << "library." + LIBRARYTABLE_BPM_LOCK
            << "library." + LIBRARYTABLE_DURATION
            << "library." + LIBRARYTABLE_BITRATE
            << "library." + LIBRARYTABLE_REPLAYGAIN
            << "library." + LIBRARYTABLE_FILETYPE
            << "library." + LIBRARYTABLE_DATETIMEADDED
            << "track_locations.location"
            << "track_locations.fs_deleted"
            << "library." + LIBRARYTABLE_COMMENT
            << "library." + LIBRARYTABLE_MIXXXDELETED
            << "library." + LIBRARYTABLE_COVERART_SOURCE
            << "library." + LIBRARYTABLE_COVERART_TYPE
            << "library." + LIBRARYTABLE_COVERART_LOCATION
            << "library." + LIBRARYTABLE_COVERART_HASH;

    QSqlQuery query(pTrackCollection->getDatabase());
    QString tableName = "library_cache_view";
    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
        "SELECT %2 FROM library "
        "INNER JOIN track_locations ON library.location = track_locations.id")
            .arg(tableName, columns.join(","));
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Strip out library. and track_locations.
    for (QStringList::iterator it = columns.begin();
         it != columns.end(); ++it) {
        if (it->startsWith("library.")) {
            *it = it->replace("library.", "");
        } else if (it->startsWith("track_locations.")) {
            *it = it->replace("track_locations.", "");
        }
    }

    BaseTrackCache* pBaseTrackCache = new BaseTrackCache(
            pTrackCollection, tableName, LIBRARYTABLE_ID, columns, true);
    connect(&m_trackDao, SIGNAL(trackDirty(TrackId)),
            pBaseTrackCache, SLOT(slotTrackDirty(TrackId)));
    connect(&m_trackDao, SIGNAL(trackClean(TrackId)),
            pBaseTrackCache, SLOT(slotTrackClean(TrackId)));
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            pBaseTrackCache, SLOT(slotTrackChanged(TrackId)));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            pBaseTrackCache, SLOT(slotTracksAdded(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            pBaseTrackCache, SLOT(slotTracksRemoved(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(dbTrackAdded(TrackPointer)),
            pBaseTrackCache, SLOT(slotDbTrackAdded(TrackPointer)));

    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    pTrackCollection->setTrackSource(m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection, "mixxx.db.model.library");

    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(this);
    
    TreeItem* pLibraryChildItem = new TreeItem(kLibraryTitle, kLibraryTitle,
                                               this, pRootItem);
    pLibraryChildItem->setIcon(getIcon());
    TreeItem* pmissingChildItem = new TreeItem(kMissingTitle, kMissingTitle,
                                               this, pRootItem);
    TreeItem* phiddenChildItem = new TreeItem(kHiddenTitle, kHiddenTitle,
                                              this, pRootItem);
    pRootItem->appendChild(pLibraryChildItem);
    pRootItem->appendChild(pmissingChildItem);
    pRootItem->appendChild(phiddenChildItem);

    m_childModel.setRootItem(pRootItem);
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pLibraryTableModel;
}

QWidget *MixxxLibraryFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, 
                                               int paneId) {
    WTrackTableView* pTable = LibraryFeature::createTableWidget(pKeyboard, paneId);
    
    connect(this, SIGNAL(unhideHidden()), pTable, SLOT(slotUnhide()));
    connect(this, SIGNAL(purgeHidden()), pTable, SLOT(slotPurge()));
    connect(this, SIGNAL(purgeMissing()), pTable, SLOT(slotPurge()));
    
    return pTable;
}

QWidget* MixxxLibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {    
    m_pSidebarTab = new QTabWidget(nullptr);
    
    // Create tree
    WLibrarySidebar* pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_idExpandedTree = m_pSidebarTab->addTab(pSidebar, tr("Tree"));
    
    // Create tabs
    m_pExpandedStack = new QStackedWidget(m_pSidebarTab);
     
    // Create Hidden View controls
    m_pHiddenView = new DlgHidden(pSidebar);    
    m_pHiddenView->setTableModel(getHiddenTableModel());
    m_pHiddenView->installEventFilter(pKeyboard);
    
    connect(m_pHiddenView, SIGNAL(unhide()), this, SIGNAL(unhideHidden()));
    connect(m_pHiddenView, SIGNAL(purge()), this, SIGNAL(purgeHidden()));
    connect(m_pHiddenView, SIGNAL(selectAll()), this, SLOT(selectAll()));
    m_idExpandedHidden = m_pExpandedStack->addWidget(m_pHiddenView);

    // Create Missing View controls
    m_pMissingView = new DlgMissing(pSidebar);
    m_pMissingView->setTableModel(getMissingTableModel());
    m_pMissingView->installEventFilter(pKeyboard);
    
    connect(m_pMissingView, SIGNAL(purge()), this, SIGNAL(purgeMissing()));
    connect(m_pMissingView, SIGNAL(selectAll()), this, SLOT(selectAll()));
    m_idExpandedMissing = m_pExpandedStack->addWidget(m_pMissingView);
    
    m_idExpandedControls = m_pSidebarTab->addTab(m_pExpandedStack, tr("Controls"));
    
    return m_pSidebarTab;
}

QVariant MixxxLibraryFeature::title() {
    return kLibraryTitle;
}

QIcon MixxxLibraryFeature::getIcon() {
    return QIcon(":/images/library/ic_library_library.png");
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
    if (!m_pMissingView.isNull()) {
        m_pMissingView->onShow();
    }
    if (!m_pHiddenView.isNull()) {
        m_pHiddenView->onShow();
    }
}

void MixxxLibraryFeature::selectionChanged(const QItemSelection&,
                                           const QItemSelection&) {
    WTrackTableView* pTable = getFocusedTable();
    if (pTable == nullptr) {
        return;
    }
    
    auto it = m_idPaneCurrent.find(m_featureFocus);
    if (it == m_idPaneCurrent.end()) {
        return;
    }
    
    const QModelIndexList& selection = pTable->selectionModel()->selectedIndexes();
    switch (*it) {
        case Panes::Hidden:
            m_pHiddenView->setSelectedIndexes(selection);
            break;
        case Panes::Missing:
            m_pMissingView->setSelectedIndexes(selection);
            break;
        default:
            break;
    }
}

void MixxxLibraryFeature::selectAll() {
    QPointer<WTrackTableView> pTable = getFocusedTable();
    if (!pTable.isNull()) {
        pTable->selectAll();
    }
}


void MixxxLibraryFeature::activate() {
    //qDebug() << "MixxxLibraryFeature::activate()";
    
    m_idPaneCurrent[m_featureFocus] = Panes::MixxxLibrary;
    QPointer<WTrackTableView> pTable = getFocusedTable();
    if (pTable.isNull()) {
        return;
    }
    
    m_pSidebarTab->setTabEnabled(m_idExpandedControls, false);
    pTable->setSortingEnabled(true);
    showTrackModel(m_pLibraryTableModel);
    m_pLibrary->restoreSearch("");
    m_pLibrary->showBreadCrumb(m_childModel.getItem(QModelIndex()));
    
    emit(enableCoverArtDisplay(true));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data(TreeItemModel::kDataPathRole).toString();
    TreeItem* pTree = static_cast<TreeItem*> (index.internalPointer());
    QPointer<WTrackTableView> pTable = getFocusedTable();    
    
    if (itemName == kLibraryTitle) {
        activate();
        return;
    } 
    
    DEBUG_ASSERT_AND_HANDLE(!m_pExpandedStack.isNull() && !pTable.isNull()) {
        return;
    }
    pTable->setSortingEnabled(false);
    
    if (itemName == kHiddenTitle) {        
        DEBUG_ASSERT_AND_HANDLE(!m_pHiddenView.isNull()) {
            return;
        }
        
        m_idPaneCurrent[m_featureFocus] = Panes::Hidden;
        pTable->loadTrackModel(getHiddenTableModel());
        
        m_pHiddenView->onShow();
        m_pExpandedStack->setCurrentIndex(m_idExpandedHidden);
        
    } else if (itemName == kMissingTitle) {
        DEBUG_ASSERT_AND_HANDLE(!m_pMissingView.isNull()) {
            return;
        }
        
        m_idPaneCurrent[m_featureFocus] = Panes::Missing;
        pTable->loadTrackModel(getMissingTableModel());
        
        m_pMissingView->onShow();
        m_pExpandedStack->setCurrentIndex(m_idExpandedMissing);
    } else {
        return;
    }
    
    // This is the only way to get the selection signal changing the track
    // models, every time the model changes the selection model changes too
    // so we need to reconnect
    connect(pTable->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,
            SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
    
    m_pSidebarTab->setTabEnabled(m_idExpandedControls, true);
    switchToFeature();
    m_pLibrary->restoreSearch("");
    m_pLibrary->showBreadCrumb(pTree);
    emit(enableCoverArtDisplay(true));
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

        // Adds track, does not insert duplicates, handles unremoving logic.
        QList<TrackId> trackIds = m_trackDao.addMultipleTracks(files, true);
        return trackIds.size() > 0;
    }
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

HiddenTableModel* MixxxLibraryFeature::getHiddenTableModel() {
    if (m_pHiddenTableModel.isNull()) {
        m_pHiddenTableModel = new HiddenTableModel(this, m_pTrackCollection);
    }
    return m_pHiddenTableModel;
}

MissingTableModel* MixxxLibraryFeature::getMissingTableModel() {
    if (m_pMissingTableModel.isNull()) {
        m_pMissingTableModel = new MissingTableModel(this, m_pTrackCollection);
    }
    return m_pMissingTableModel;
}
