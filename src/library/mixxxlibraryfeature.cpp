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

const QString MixxxLibraryFeature::m_sMixxxLibraryViewName = "MixxxLibraryFeature";

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, parent),
          kLibraryTitle(tr("Library")),
          kHiddenTitle(tr("Hidden Tracks")),
          kMissingTitle(tr("Missing Tracks")),
          m_pHiddenView(nullptr),
          m_pMissingView(nullptr),
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
    TreeItem* pLibraryChildItem = new TreeItem(kLibraryTitle, m_sMixxxLibraryViewName,
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

void MixxxLibraryFeature::bindPaneWidget(WLibrary* pLibraryWidget,
                                         KeyboardEventFilter* pKeyboard,
                                         int paneId) {
    WLibraryStack* pStack = new WLibraryStack(pLibraryWidget);
    m_pPaneStack = pStack;
    
    WTrackTableView* pHiddenTable = 
            new WTrackTableView(pStack, m_pConfig, m_pTrackCollection, false);
    pHiddenTable->installEventFilter(pKeyboard);
    m_hiddenPaneId[paneId] = pStack->addWidget(pHiddenTable);
    
    if (m_pHiddenView) {
        m_pHiddenView->setTrackTable(m_pLibrary, pHiddenTable, paneId);
    } else {
        m_hiddenPane[paneId] = pHiddenTable;
    }
    
    WTrackTableView* pMissingTable = 
            new WTrackTableView(pStack, m_pConfig, m_pTrackCollection, false);
    pMissingTable->installEventFilter(pKeyboard);
    m_missingPaneId[paneId] = pStack->addWidget(pMissingTable);
    
    if (m_pMissingView) {
        m_pMissingView->setTrackTable(m_pLibrary, pMissingTable, paneId);
    } else {
        m_missingPane[paneId] = pMissingTable;
    }
    pLibraryWidget->registerView(m_sMixxxLibraryViewName, pStack);
}

QWidget *MixxxLibraryFeature::createPaneWidget(KeyboardEventFilter* pKeyboard, 
                                               int paneId) {
    WLibraryStack* pStack = new WLibraryStack(nullptr);
    m_pPaneStack = pStack;
    
    WTrackTableView* pHiddenTable = 
            new WTrackTableView(pStack, m_pConfig, m_pTrackCollection, false);
    pHiddenTable->installEventFilter(pKeyboard);
    m_hiddenPaneId[paneId] = pStack->addWidget(pHiddenTable);
    
    if (m_pHiddenView) {
        m_pHiddenView->setTrackTable(m_pLibrary, pHiddenTable, paneId);
    } else {
        m_hiddenPane[paneId] = pHiddenTable;
    }
    
    WTrackTableView* pMissingTable = 
            new WTrackTableView(pStack, m_pConfig, m_pTrackCollection, false);
    pMissingTable->installEventFilter(pKeyboard);
    m_missingPaneId[paneId] = pStack->addWidget(pMissingTable);
    
    if (m_pMissingView) {
        m_pMissingView->setTrackTable(m_pLibrary, pMissingTable, paneId);
    } else {
        m_missingPane[paneId] = pMissingTable;
    }
    
    return pStack;
}

void MixxxLibraryFeature::bindSidebarWidget(WBaseLibrary* pLibraryWidget,
                                            KeyboardEventFilter*) {
    QTabWidget* pTab = new QTabWidget(pLibraryWidget);
    
    // Create tree
    WLibrarySidebar* pSidebar = new WLibrarySidebar(pTab);
    pSidebar->setModel(&m_childModel);
    
    // Tree connections
    connect(pSidebar, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(activateChild(const QModelIndex&)));
    connect(pSidebar, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    connect(pSidebar, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            this, SLOT(onRightClickChild(const QPoint&, const QModelIndex&)));
    connect(pSidebar, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(onLazyChildExpandation(const QModelIndex&)));
    
    pTab->addTab(pSidebar, tr("Tree"));
    
    // Create tabs
    QStackedWidget* pStack = new QStackedWidget(pTab);
    m_pExpandedStack = pStack;
    
    // Create Hidden View controls
    m_pHiddenView = new DlgHidden(pLibraryWidget, m_pTrackCollection);
    m_hiddenExpandedId = pStack->addWidget(m_pHiddenView);
    connect(m_pHiddenView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    
    // Add Track tables to Hidden view
    for (auto it = m_hiddenPane.begin(); it != m_hiddenPane.end(); ++it) {
        m_pHiddenView->setTrackTable(m_pLibrary, it.value(), it.key());
    }
    m_hiddenPane.clear();

    // Create Missing View controls
    m_pMissingView = new DlgMissing(pLibraryWidget, m_pTrackCollection);
    m_missingExpandedId = pStack->addWidget(m_pMissingView);
    connect(m_pMissingView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    
    // Add Track tables to Missing view
    for (auto it = m_missingPane.begin(); it != m_missingPane.end(); ++it) {
        m_pMissingView->setTrackTable(m_pLibrary, it.value(), it.key());
    }
    m_missingPane.clear();
    
    pTab->addTab(pStack, tr("Controls"));
    pLibraryWidget->registerView(m_sMixxxLibraryViewName, pTab);
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
    if (m_pMissingView) {
        m_pMissingView->onShow();
    }
    if (m_pHiddenView) {
        m_pHiddenView->onShow();
    }
}

void MixxxLibraryFeature::activate() {
    //qDebug() << "MixxxLibraryFeature::activate()";
    m_pHiddenView->setFocusedPane(m_featureFocus);
    m_pMissingView->setFocusedPane(m_featureFocus);
    
    emit(switchToView(m_sMixxxLibraryViewName));
    emit(showTrackModel(m_pLibraryTableModel));
    emit(enableCoverArtDisplay(true));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data(TreeItemModel::kDataPathRole).toString();
    if (itemName == m_sMixxxLibraryViewName) {
        activate();
    } else if (itemName == kHiddenTitle) {
        
    } else if (itemName == kMissingTitle) {
        
    }
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
