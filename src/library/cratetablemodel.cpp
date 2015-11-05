
// cratetablemodel.cpp
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/cratetablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "playermanager.h"

CrateTableModel::CrateTableModel(QObject* pParent,
                                 TrackCollection* pTrackCollection)
        : BaseSqlTableModel(pParent, pTrackCollection,
                            "mixxx.db.model.crate"),
          m_iCrateId(-1){
}

CrateTableModel::~CrateTableModel() {
}

// Must be called from Main thread
void CrateTableModel::setTableModel(int crateId) {

    Q_ASSERT_X(QThread::currentThread()==qApp->thread(),__FILE__+__LINE__,__PRETTY_FUNCTION__);

    //qDebug() << "CrateTableModel::setCrate()" << crateId;
    if (crateId == m_iCrateId) {
        qDebug() << "Already focused on crate " << crateId;
        return;
    }
    m_iCrateId = crateId;
    QStringList columns;
    columns << "crate_tracks." + CRATETRACKSTABLE_TRACKID + " AS " + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover hash.
            << LIBRARYTABLE_COVERART_HASH + " AS " + LIBRARYTABLE_COVERART;
    QString tableName = QString("crate_%1").arg(m_iCrateId);

    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this, &columns, &tableName] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QSqlQuery query(pTrackCollectionPrivate->getDatabase());
        FieldEscaper escaper(pTrackCollectionPrivate->getDatabase());
        QString filter = "library.mixxx_deleted = 0";

        // We drop files that have been explicitly deleted from mixxx
        // (mixxx_deleted=0) from the view. There was a bug in <= 1.9.0 where
        // removed files were not removed from crates, so some users will have
        // libraries where this is the case.
        QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                      "SELECT %2 FROM %3 "
                                      "INNER JOIN library ON library.id = %3.%4 "
                                      "WHERE %3.%5 = %6 AND %7")
                .arg(escaper.escapeString(tableName),
                     columns.join(","),
                     CRATE_TRACKS_TABLE,
                     CRATETRACKSTABLE_TRACKID,
                     CRATETRACKSTABLE_CRATEID,
                     QString::number(m_iCrateId),
                     filter);
        query.prepare(queryString);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }, __PRETTY_FUNCTION__);

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(tableName, columns[0], columns,
             m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

// Must be called from Main thread
bool CrateTableModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this playlist.
    QFileInfo fileInfo(location);
    bool success = false;
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callSync(
                [this, &fileInfo, &success] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        // Adds track, does not insert duplicates, handles unremoving logic.
        int iTrackId = pTrackCollectionPrivate->getTrackDAO().addTrack(fileInfo, true);

        bool success = false;
        if (iTrackId >= 0) {
            success = pTrackCollectionPrivate->getCrateDAO().addTrackToCrate(iTrackId, m_iCrateId);
        }
        if (success) {
            // TODO(rryan) just add the track dont select
            select(pTrackCollectionPrivate);
        } else {
            qDebug() << "CrateTableModel::addTrack could not add track"
                     << fileInfo.absoluteFilePath() << "to crate" << m_iCrateId;
        }
    }, __PRETTY_FUNCTION__);
    return success;
}

// Must be called from Main thread
int CrateTableModel::addTracks(const QModelIndex& index,
                               const QList<QString>& locations) {
    Q_UNUSED(index);
    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this crate.
    QList<QFileInfo> fileInfoList;
    foreach(QString fileLocation, locations) {
        fileInfoList.append(QFileInfo(fileLocation));
    }

    int tracksAdded = 0;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this, &fileInfoList, &tracksAdded] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QList<int> trackIDs = pTrackCollectionPrivate->getTrackDAO().addTracks(fileInfoList, true);
        tracksAdded = pTrackCollectionPrivate->getCrateDAO().addTracksToCrate(m_iCrateId, &trackIDs);
        if (tracksAdded > 0) {
            select(pTrackCollectionPrivate);
        }
    }, __PRETTY_FUNCTION__);

    if (locations.size() - tracksAdded > 0) {
        qDebug() << "CrateTableModel::addTracks could not add"
                 << locations.size() - tracksAdded
                 << "to crate" << m_iCrateId;
    }
    return tracksAdded;
}

// Must be called from Main thread
void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, indices] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        bool locked = pTrackCollectionPrivate->getCrateDAO().isCrateLocked(m_iCrateId);

        if (!locked) {
            QList<int> trackIds;
            foreach (QModelIndex index, indices) {
                trackIds.append(getTrackId(index));
            }
            pTrackCollectionPrivate->getCrateDAO().removeTracksFromCrate(trackIds, m_iCrateId);
            select(pTrackCollectionPrivate);
        }
    }, __PRETTY_FUNCTION__);
}

bool CrateTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_CRATETRACKSTABLE_TRACKID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID)||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH)) {
        return true;
    }
    return false;
}

bool CrateTableModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}

TrackModel::CapabilitiesFlags CrateTableModel::getCapabilities() const {
    CapabilitiesFlags caps =  TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_RECEIVEDROPS
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_RELOADMETADATA
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_REMOVE
            | TRACKMODELCAPS_MANIPULATEBEATS
            | TRACKMODELCAPS_CLEAR_BEATS
            | TRACKMODELCAPS_RESETPLAYED;

    bool locked;
    m_pTrackCollection->callSync( [this, &locked] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        locked = pTrackCollectionPrivate->getCrateDAO().isCrateLocked(m_iCrateId);
    }, __PRETTY_FUNCTION__);
    if (locked) {
        caps |= TRACKMODELCAPS_LOCKED;
    }

    return caps;
}
