// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QMenu>
#include <QStringList>
#include <QVBoxLayout>

#include "library/features/mixxxlibrary/mixxxlibraryfeature.h"
#include "library/features/libraryfolder/libraryfoldermodel.h"

#include "library/basetrackcache.h"
#include "library/librarytablemodel.h"
#include "library/parser.h"
#include "library/queryutil.h"
#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarystack.h"
#include "widget/wtracktableview.h"

const QString MixxxLibraryFeature::kLibraryTitle = tr("Tracks");

const QStringList MixxxLibraryFeature::kGroupingText = 
    QStringList::fromStdList({
        tr("Artist > Album"),
        tr("Album"),
        tr("Genre > Artist > Album"),
        tr("Genre > Album"),
        tr("Folder")
});

const QList<QStringList> MixxxLibraryFeature::kGroupingOptions =
    QList<QStringList>::fromStdList({
        QStringList::fromStdList({ LIBRARYTABLE_ARTIST, LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_GENRE, LIBRARYTABLE_ARTIST, 
                                   LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_GENRE, LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYFOLDERMODEL_FOLDER })
});

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_foldersShown(false) {
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

    QSqlQuery query(pTrackCollection->database());
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

    m_pBaseTrackCache = pTrackCollection->getTrackSource();
    connect(&m_trackDao, SIGNAL(trackDirty(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackDirty(TrackId)));
    connect(&m_trackDao, SIGNAL(trackClean(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackClean(TrackId)));
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackChanged(TrackId)));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            m_pBaseTrackCache.data(), SLOT(slotTracksAdded(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            m_pBaseTrackCache.data(), SLOT(slotTracksRemoved(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(dbTrackAdded(TrackPointer)),
            m_pBaseTrackCache.data(), SLOT(slotDbTrackAdded(TrackPointer)));

    setChildModel(new LibraryFolderModel(this, m_pTrackCollection, m_pConfig));
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
    return ":/images/library/ic_library_tracks.png";
}

QString MixxxLibraryFeature::getSettingsName() const {
    return "MixxxLibraryFeature";
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return m_pChildModel;
}

QWidget* MixxxLibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_pSidebar->setIconSize(m_pChildModel->getDefaultIconSize());    
    m_pChildModel->reloadTree();
    return m_pSidebar;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
}

void MixxxLibraryFeature::onSearch(const QString&) {
    showBreadCrumb();
    if (!m_pSidebar.isNull()) {
        m_pSidebar->clearSelection();
    }
}

void MixxxLibraryFeature::setChildModel(TreeItemModel* pChild) {
    if (!m_pChildModel.isNull()) {
        delete m_pChildModel;
    }
    
    m_pChildModel = pChild;
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            m_pChildModel, SLOT(reloadTree()));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTree()));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTree()));
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
    
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    m_lastClickedIndex = index;
    
    if (!index.isValid()) return;

    QString query = index.data(AbstractRole::RoleQuery).toString();
    //qDebug() << "MixxxLibraryFeature::activateChild" << query;
    
    if (query == "$groupingSettings$") {
        // Act as right click
        onRightClickChild(QCursor::pos(), QModelIndex());
        return;
    }
    
    m_pLibraryTableModel->search(query);
    switchToFeature();
    showBreadCrumb(index.data(AbstractRole::RoleBreadCrumb).toString(), getIcon());
    restoreSearch(query);
}

void MixxxLibraryFeature::invalidateChild() {
    m_lastClickedIndex = QPersistentModelIndex();
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& pos, 
                                            const QModelIndex&) {
    
    // Create the sort menu
    QMenu menu;    
    QStringList currentSort = m_pChildModel->data(
            QModelIndex(), AbstractRole::RoleSorting).toStringList();
    bool recursive = m_pChildModel->data(
            QModelIndex(), AbstractRole::RoleSettings).toBool();
    
    
    QActionGroup* orderGroup = new QActionGroup(&menu);
    for (int i = 0; i < kGroupingOptions.size(); ++i) {
        QAction* action = menu.addAction(kGroupingText.at(i));
        action->setActionGroup(orderGroup);
        action->setData(kGroupingOptions.at(i));
        action->setCheckable(true);
        action->setChecked(currentSort == kGroupingOptions.at(i));
    }
    
    menu.addSeparator();
    QAction* folderRecursive = menu.addAction(tr("Get recursive folder search query"));
    folderRecursive->setCheckable(true);
    folderRecursive->setChecked(recursive);
    
    QAction* selected = menu.exec(pos);
    if (selected == nullptr) {
        return;
    } else if (selected == folderRecursive) {
        setTreeSettings(folderRecursive->isChecked(), 
                        AbstractRole::RoleSettings);
    } else {
        setTreeSettings(selected->data());
    }
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files = 
                DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

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

void MixxxLibraryFeature::setTreeSettings(const QVariant& settings, 
                                          AbstractRole role) {
    if (m_pChildModel.isNull()) {
        return;
    }
    m_pChildModel->setData(QModelIndex(), settings, role);
    m_pChildModel->reloadTree();
}
