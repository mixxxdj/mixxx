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
#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "treeitem.h"
#include "sources/soundsourceproxy.h"
#include "widget/wlibrary.h"
#include "util/dnd.h"
#include "library/dlghidden.h"
#include "library/dlgmissing.h"

MixxxLibraryFeature::MixxxLibraryFeature(Library* pLibrary,
                                         TrackCollection* pTrackCollection,
                                         UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary),
          kMissingTitle(tr("Missing Tracks")),
          kHiddenTitle(tr("Hidden Tracks")),
          m_pLibrary(pLibrary),
          m_pMissingView(NULL),
          m_pHiddenView(NULL),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_icon(":/images/library/ic_library_tracks.svg") {
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

    BaseTrackCache* pBaseTrackCache = new BaseTrackCache(
            pTrackCollection, tableName, LIBRARYTABLE_ID, columns, true);
    connect(&m_trackDao,
            &TrackDAO::trackDirty,
            pBaseTrackCache,
            &BaseTrackCache::slotTrackDirty);
    connect(&m_trackDao,
            &TrackDAO::trackClean,
            pBaseTrackCache,
            &BaseTrackCache::slotTrackClean);
    connect(&m_trackDao,
            &TrackDAO::trackChanged,
            pBaseTrackCache,
            &BaseTrackCache::slotTrackChanged);
    connect(&m_trackDao,
            &TrackDAO::tracksAdded,
            pBaseTrackCache,
            &BaseTrackCache::slotTracksAdded);
    connect(&m_trackDao,
            &TrackDAO::tracksRemoved,
            pBaseTrackCache,
            &BaseTrackCache::slotTracksRemoved);
    connect(&m_trackDao,
            &TrackDAO::dbTrackAdded,
            pBaseTrackCache,
            &BaseTrackCache::slotDbTrackAdded);

    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    pTrackCollection->setTrackSource(m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection, "mixxx.db.model.library");

    auto pRootItem = std::make_unique<TreeItem>(this);
    pRootItem->appendChild(kMissingTitle);
    pRootItem->appendChild(kHiddenTitle);

    m_childModel.setRootItem(std::move(pRootItem));
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pLibraryTableModel;
}

void MixxxLibraryFeature::bindWidget(WLibrary* pLibraryWidget,
                                     KeyboardEventFilter* pKeyboard) {
    m_pHiddenView = new DlgHidden(pLibraryWidget, m_pConfig, m_pLibrary,
                                  m_pTrackCollection, pKeyboard);
    pLibraryWidget->registerView(kHiddenTitle, m_pHiddenView);
    connect(m_pHiddenView,
            &DlgHidden::trackSelected,
            this,
            &MixxxLibraryFeature::trackSelected);

    m_pMissingView = new DlgMissing(pLibraryWidget, m_pConfig, m_pLibrary,
                                    m_pTrackCollection, pKeyboard);
    pLibraryWidget->registerView(kMissingTitle, m_pMissingView);
    connect(m_pMissingView,
            &DlgMissing::trackSelected,
            this,
            &MixxxLibraryFeature::trackSelected);
}

QVariant MixxxLibraryFeature::title() {
    return tr("Tracks");
}

QIcon MixxxLibraryFeature::getIcon() {
    return m_icon;
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
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModel));
    emit(enableCoverArtDisplay(true));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();
    emit(switchToView(itemName));
    if (m_pMissingView && itemName == kMissingTitle) {
        emit(restoreSearch(m_pMissingView->currentSearch()));
    } else if (m_pHiddenView && itemName == kHiddenTitle) {
        emit(restoreSearch(m_pHiddenView->currentSearch()));
    }
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
