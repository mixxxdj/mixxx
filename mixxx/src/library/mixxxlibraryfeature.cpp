// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/basetrackcache.h"
#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/hiddentablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "treeitem.h"
#include "soundsourceproxy.h"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          kMissingTitle(tr("Missing Tracks")),
          kHiddenTitle(tr("Hidden Tracks")),
          m_trackDao(pTrackCollection->getTrackDAO()) {
    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "library." + LIBRARYTABLE_PLAYED
            << "library." + LIBRARYTABLE_TIMESPLAYED
            //has to be up here otherwise Played and TimesPlayed are not show
            << "library." + LIBRARYTABLE_ARTIST
            << "library." + LIBRARYTABLE_TITLE
            << "library." + LIBRARYTABLE_ALBUM
            << "library." + LIBRARYTABLE_YEAR
            << "library." + LIBRARYTABLE_DURATION
            << "library." + LIBRARYTABLE_RATING
            << "library." + LIBRARYTABLE_GENRE
            << "library." + LIBRARYTABLE_COMPOSER
            << "library." + LIBRARYTABLE_FILETYPE
            << "library." + LIBRARYTABLE_TRACKNUMBER
            << "library." + LIBRARYTABLE_KEY
            << "library." + LIBRARYTABLE_DATETIMEADDED
            << "library." + LIBRARYTABLE_BPM
            << "library." + LIBRARYTABLE_BPM_LOCK
            << "library." + LIBRARYTABLE_BITRATE
            << "track_locations.location"
            << "track_locations.fs_deleted"
            << "library." + LIBRARYTABLE_COMMENT
            << "library." + LIBRARYTABLE_MIXXXDELETED;

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
    connect(&m_trackDao, SIGNAL(trackDirty(int)),
            pBaseTrackCache, SLOT(slotTrackDirty(int)));
    connect(&m_trackDao, SIGNAL(trackClean(int)),
            pBaseTrackCache, SLOT(slotTrackClean(int)));
    connect(&m_trackDao, SIGNAL(trackChanged(int)),
            pBaseTrackCache, SLOT(slotTrackChanged(int)));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<int>)),
            pBaseTrackCache, SLOT(slotTracksAdded(QSet<int>)));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<int>)),
            pBaseTrackCache, SLOT(slotTracksRemoved(QSet<int>)));
    connect(&m_trackDao, SIGNAL(dbTrackAdded(TrackPointer)),
            pBaseTrackCache, SLOT(slotDbTrackAdded(TrackPointer)));

    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    pTrackCollection->addTrackSource(QString("default"), m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection);
    m_pMissingTableModel = new MissingTableModel(this, pTrackCollection);
    m_pHiddenTableModel = new HiddenTableModel(this, pTrackCollection);

    TreeItem* pRootItem = new TreeItem();
    TreeItem* pmissingChildItem = new TreeItem(kMissingTitle, kMissingTitle,
                                       this, pRootItem);
    TreeItem* phiddenChildItem = new TreeItem(kHiddenTitle, kHiddenTitle,
                                       this, pRootItem);
    pRootItem->appendChild(pmissingChildItem);
    pRootItem->appendChild(phiddenChildItem);

    m_childModel.setRootItem(pRootItem);
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pLibraryTableModel;
    delete m_pMissingTableModel;
    delete m_pHiddenTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return tr("Library");
}

QIcon MixxxLibraryFeature::getIcon() {
    return QIcon(":/images/library/ic_library_library.png");
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

void MixxxLibraryFeature::refreshLibraryModels()
{
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
    if (m_pMissingTableModel) {
        m_pMissingTableModel->select();
    }
    if (m_pHiddenTableModel) {
        m_pHiddenTableModel->select();
    }
}

void MixxxLibraryFeature::activate() {
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModel));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();

    /*if (itemName == m_childModel.stringList().at(0))
        emit(showTrackModel(m_pMissingTableModel));
     */
    if (itemName == kMissingTitle) {
        emit(showTrackModel(m_pMissingTableModel));
    }
    if (itemName == kHiddenTitle) {
        emit(showTrackModel(m_pHiddenTableModel));
    }
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QWidget *pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files;
        foreach (QUrl url, urls) {
            // XXX: Possible WTF alert - Previously we thought we needed toString() here
            // but what you actually want in any case when converting a QUrl to a file
            // system path is QUrl::toLocalFile(). This is the second time we have
            // flip-flopped on this, but I think toLocalFile() should work in any
            // case. toString() absolutely does not work when you pass the result to a
            files.append(url.toLocalFile());
        }
    
        // Adds track, does not insert duplicates, handles unremoving logic.
        QList<int> trackIds = m_trackDao.addTracks(files, true);
        return trackIds.size() > 0;
    }
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    QFileInfo file(url.toLocalFile());
    return SoundSourceProxy::isFilenameSupported(file.fileName());
}
