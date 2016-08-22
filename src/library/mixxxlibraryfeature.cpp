// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QMenu>
#include <QStringList>
#include <QVBoxLayout>

#include "library/mixxxlibraryfeature.h"

#include "library/basetrackcache.h"
#include "library/dlghidden.h"
#include "library/dlgmissing.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/parser.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarystack.h"
#include "widget/wtracktableview.h"

const QStringList MixxxLibraryFeature::kGroupingText = {
    tr("Artist > Album"),
    tr("Album"),
    tr("Genre > Artist > Album"),
    tr("Genre > Album")
};

const QList<QStringList> MixxxLibraryFeature::kGroupingOptions = {
        { LIBRARYTABLE_ARTIST, LIBRARYTABLE_ALBUM },
        { LIBRARYTABLE_ALBUM },
        { LIBRARYTABLE_GENRE, LIBRARYTABLE_ARTIST, LIBRARYTABLE_ALBUM },
        { LIBRARYTABLE_GENRE, LIBRARYTABLE_ALBUM }
};
        

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          kLibraryTitle(tr("Library")),
          m_trackDao(pTrackCollection->getTrackDAO()) {
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
            << "track_locations.directory"
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
    qDebug() << queryString;
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

    setChildModel(new LibraryTreeModel(this, m_pTrackCollection, m_pConfig));
    
    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    
    
    pTrackCollection->setTrackSource(m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection, "mixxx.db.model.library");
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pChildModel;
    delete m_pLibraryTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return kLibraryTitle;
}

QString MixxxLibraryFeature::getIconPath() {
    return ":/images/library/ic_library_library.png";
}

QString MixxxLibraryFeature::getSettingsName() const {
    return "MixxxLibraryFeature";
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return m_pChildModel;
}

QWidget* MixxxLibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    QWidget* pContainer = new QWidget;
    QLayout* pLayout = new QVBoxLayout(pContainer);
    m_pGroupingCombo = new QComboBox(pContainer);
    for (int i = 0; i < kGroupingOptions.size(); ++i) {
        m_pGroupingCombo->addItem(kGroupingText.at(i), kGroupingOptions.at(i));
    }
    
    QVariant varData = m_pChildModel->data(QModelIndex(), TreeItemModel::RoleSettings);
    m_pGroupingCombo->setCurrentIndex(kGroupingOptions.indexOf(varData.toStringList()));
    
    connect(m_pGroupingCombo.data(), SIGNAL(activated(int)),
            this, SLOT(slotComboActivated(int)));
    
    pLayout->addWidget(m_pGroupingCombo);
    
    m_pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_pSidebar->setParent(pContainer);
    m_pSidebar->setIconSize(m_pChildModel->getDefaultIconSize());
    pLayout->addWidget(m_pSidebar);
    pContainer->setLayout(pLayout);
    
    m_pChildModel->reloadTree();
    return pContainer;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
}

void MixxxLibraryFeature::selectAll() {
    QPointer<WTrackTableView> pTable = getFocusedTable();
    if (!pTable.isNull()) {
        pTable->selectAll();
    }
}

void MixxxLibraryFeature::onSearch(const QString&) {
    showBreadCrumb();
    m_pSidebar->clearSelection();
}

void MixxxLibraryFeature::setChildModel(TreeItemModel* pChild) {
    if (!m_pChildModel.isNull()) {
        delete m_pChildModel;
    }
    
    m_pChildModel = pChild;
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            m_pChildModel, SLOT(reloadTracksTree()));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTracksTree()));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTracksTree()));
}

void MixxxLibraryFeature::activate() {
    if (m_lastClickedIndex.isValid()) {
        activateChild(m_lastClickedIndex);
        return;
    }
    
    //qDebug() << "MixxxLibraryFeature::activate()";
    showTrackModel(m_pLibraryTableModel);
    restoreSearch("");
    showBreadCrumb();
    
    emit(enableCoverArtDisplay(true));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    m_lastClickedIndex = index;
    QString query = index.data(TreeItemModel::RoleQuery).toString();
    //qDebug() << "MixxxLibraryFeature::activateChild" << query;
    
    m_pLibraryTableModel->search(query);
    switchToFeature();
    showBreadCrumb(index.data(TreeItemModel::RoleBreadCrumb).toString(), getIcon());
    restoreSearch(query);
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& pos, 
                                            const QModelIndex&) {
    
    // Create the sort menu
    QMenu menu;    
    QVariant varSort = m_pChildModel->data(QModelIndex(), TreeItemModel::RoleSettings);
    QStringList currentSort = varSort.toStringList();
    
    QActionGroup* orderGroup = new QActionGroup(&menu);
    for (int i = 0; i < kGroupingOptions.size(); ++i) {
        QAction* action = menu.addAction(kGroupingText.at(i));
        action->setActionGroup(orderGroup);
        action->setData(kGroupingOptions.at(i));
        action->setCheckable(true);
        action->setChecked(currentSort == kGroupingOptions.at(i));
    }
    
    QAction* selected = menu.exec(pos);
    if (selected == nullptr) {
        return;
    }
    if (!m_pGroupingCombo.isNull()) {
        m_pGroupingCombo->setCurrentIndex(kGroupingOptions.indexOf(selected->data().toStringList()));
    }
    setTreeSettings(selected->data());
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

        // Adds track, does not insert duplicates, handles unremoving logic.
        QList<TrackId> trackIds = m_trackDao.addMultipleTracks(files, true);
        m_pChildModel->reloadTree();
        return trackIds.size() > 0;
    }
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

void MixxxLibraryFeature::setTreeSettings(const QVariant& settings) {
    if (m_pChildModel.isNull()) {
        return;
    }
    m_pChildModel->setData(QModelIndex(), settings, TreeItemModel::RoleSettings);
    m_pChildModel->reloadTree();
}

void MixxxLibraryFeature::slotComboActivated(int index) {
    if (m_pGroupingCombo.isNull()) {
        return;
    }
    setTreeSettings(m_pGroupingCombo->itemData(index));
}
