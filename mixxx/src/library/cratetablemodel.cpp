// cratetablemodel.cpp
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/cratetablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "mixxxutils.cpp"
#include "playermanager.h"

CrateTableModel::CrateTableModel(QObject* pParent, 
                                 TrackCollection* pTrackCollection)
        : BaseSqlTableModel(pParent, pTrackCollection,
                            "mixxx.db.model.crate"),
          m_iCrateId(-1),
          m_crateDAO(pTrackCollection->getCrateDAO()) {
}

CrateTableModel::~CrateTableModel() {
}

void CrateTableModel::setTableModel(int crateId) {
    //qDebug() << "CrateTableModel::setCrate()" << crateId;
    if (crateId == m_iCrateId) {
        qDebug() << "Already focused on crate " << crateId;
        return;
    }
    m_iCrateId = crateId;

    QString tableName = QString("crate_%1").arg(m_iCrateId);
    QSqlQuery query(m_database);
    FieldEscaper escaper(m_database);
    QString filter = "library.mixxx_deleted = 0";
    QStringList columns;
    columns << "crate_tracks."+CRATETRACKSTABLE_TRACKID + " as " + LIBRARYTABLE_ID
            << "'' as preview";

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
                               QString::number(crateId),
                               filter);
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = "preview";
    setTable(tableName, columns[0], columns,
             m_pTrackCollection->getTrackSource("default"));
    // BaseSqlTableModel sets up the header names
    initHeaderData();
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

bool CrateTableModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this playlist.
    QFileInfo fileInfo(location);

    TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();

    // Adds track, does not insert duplicates, handles unremoving logic.
    int iTrackId = trackDao.addTrack(fileInfo, true);

    bool success = false;
    if (iTrackId >= 0) {
        success = m_pTrackCollection->getCrateDAO().addTrackToCrate(iTrackId, m_iCrateId);
    }

    if (success) {
        // TODO(rryan) just add the track dont select
        select();
        return true;
    } else {
        qDebug() << "CrateTableModel::addTrack could not add track"
                 << fileInfo.absoluteFilePath() << "to crate" << m_iCrateId;
        return false;
    }
}

int CrateTableModel::addTracks(const QModelIndex& index,
                               const QList<QString> &locations) {
    Q_UNUSED(index);
    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this crate.
    QList<QFileInfo> fileInfoList;
    foreach(QString fileLocation, locations) {
        fileInfoList.append(QFileInfo(fileLocation));
    }

    QList<int> trackIDs = m_trackDAO.addTracks(fileInfoList, true);

    int tracksAdded = m_crateDAO.addTracksToCrate(
        trackIDs, m_iCrateId);
    if (tracksAdded > 0) {
        select();
    }

    if (locations.size() - tracksAdded > 0) {
        qDebug() << "CrateTableModel::addTracks could not add"
                 << locations.size() - tracksAdded
                 << "to crate" << m_iCrateId;
    }
    return tracksAdded;
}

void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    bool locked = m_crateDAO.isCrateLocked(m_iCrateId);

    if (!locked) {
        QList<int> trackIds;
        foreach (QModelIndex index, indices) {
            trackIds.append(getTrackId(index));
        }
        m_crateDAO.removeTracksFromCrate(trackIds, m_iCrateId);
        select();
    }
}

bool CrateTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(CRATETRACKSTABLE_TRACKID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(LIBRARYTABLE_BPM_LOCK) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED) ||
        (PlayerManager::numPreviewDecks() == 0 && column == fieldIndex("preview"))) {
        return true;
    }
    return false;
}

bool CrateTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY))
        return true;
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
            | TRACKMODELCAPS_BPMLOCK
            | TRACKMODELCAPS_CLEAR_BEATS
            | TRACKMODELCAPS_RESETPLAYED;

    bool locked = m_crateDAO.isCrateLocked(m_iCrateId);
    if (locked) {
        caps |= TRACKMODELCAPS_LOCKED;
    }

    return caps;
}
