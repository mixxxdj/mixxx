// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QMenu>

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

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          kLibraryTitle(tr("Library")),
          m_childModel(this, pTrackCollection, pConfig),
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

    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            &m_childModel, SLOT(reloadTracksTree()));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            &m_childModel, SLOT(reloadTracksTree()));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            &m_childModel, SLOT(reloadTracksTree()));
    
    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    
    
    pTrackCollection->setTrackSource(m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection, "mixxx.db.model.library");
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pLibraryTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return kLibraryTitle;
}

QString MixxxLibraryFeature::getIconPath() {
    return ":/images/library/ic_library_library.png";
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

QWidget* MixxxLibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_pSidebar->setIconSize(QSize(32, 32));
    m_childModel.reloadTracksTree();
    return m_pSidebar;
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
    m_pSidebar->clearSelection();
}


void MixxxLibraryFeature::activate() {
    //qDebug() << "MixxxLibraryFeature::activate()";
    showTrackModel(m_pLibraryTableModel);
    restoreSearch("");
    showBreadCrumb();
    
    emit(enableCoverArtDisplay(true));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString query = index.data(TreeItemModel::RoleQuery).toString();
    //qDebug() << "MixxxLibraryFeature::activateChild" << query;
    
    m_pLibraryTableModel->search(query);
    switchToFeature();
    showBreadCrumb(index.data(TreeItemModel::RoleBreadCrumb).toString(), getIcon());
    restoreSearch(query);
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& pos, 
                                            const QModelIndex&) {
    bool recursive = m_childModel.getFolderRecursive();
    
    // Create the sort menu
    QMenu menu;
    QAction* showRecursive = menu.addAction(tr("Show recursive view in folders"));
    showRecursive->setCheckable(true);
    showRecursive->setChecked(recursive);
    
    menu.addSeparator();
    QStringList currentSort = m_childModel.getSortOrder();
    
    QStringList orderArtistAlbum, orderAlbum, orderGenreArtist, orderGenreAlbum;
    orderArtistAlbum    << LIBRARYTABLE_ARTIST << LIBRARYTABLE_ALBUM;
    orderAlbum          << LIBRARYTABLE_ALBUM;
    orderGenreArtist    << LIBRARYTABLE_GENRE << LIBRARYTABLE_ARTIST
                        << LIBRARYTABLE_ALBUM;
    orderGenreAlbum     << LIBRARYTABLE_GENRE << LIBRARYTABLE_ALBUM;
    
    QAction* artistAlbum = menu.addAction(tr("Artist > Album"));
    QAction* album = menu.addAction(tr("Album"));
    QAction* genreArtist = menu.addAction(tr("Genre > Artist > Album"));
    QAction* genreAlbum  = menu.addAction(tr("Genre > Album"));
    
    QActionGroup* orderGroup = new QActionGroup(&menu);
    artistAlbum->setActionGroup(orderGroup);
    album->setActionGroup(orderGroup);
    genreArtist->setActionGroup(orderGroup);
    genreAlbum->setActionGroup(orderGroup);
    
    artistAlbum->setCheckable(true);
    album->setCheckable(true);
    genreArtist->setCheckable(true);
    genreAlbum->setCheckable(true);
    
    artistAlbum->setChecked(currentSort == orderArtistAlbum);
    album->setChecked(currentSort == orderAlbum);
    genreArtist->setChecked(currentSort == orderGenreArtist);
    genreAlbum->setChecked(currentSort == orderGenreAlbum);
    
    QAction* selected = menu.exec(pos);
    
    if (selected == showRecursive) {
        recursive = showRecursive->isChecked();
        m_childModel.setFolderRecursive(recursive);
    } else if (selected == artistAlbum) {
        m_childModel.setSortOrder(orderArtistAlbum);
    } else if (selected == album) {
        m_childModel.setSortOrder(orderAlbum);
    } else if (selected == genreArtist) {
        m_childModel.setSortOrder(orderGenreArtist);
    } else if (selected == genreAlbum) {
        m_childModel.setSortOrder(orderGenreAlbum);
    } else {
        // Menu rejected
        return;
    }
    m_childModel.reloadTracksTree();
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

        // Adds track, does not insert duplicates, handles unremoving logic.
        QList<TrackId> trackIds = m_trackDao.addMultipleTracks(files, true);
        m_childModel.reloadTracksTree();
        return trackIds.size() > 0;
    }
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}
