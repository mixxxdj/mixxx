// cratetablemodel.cpp
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/cratetablemodel.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/dao/cratedao.h"

#include "mixxxutils.cpp"

CrateTableModel::CrateTableModel(QObject* pParent, TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(), "mixxx.db.model.crate"),
          BaseSqlTableModel(pParent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_iCrateId(-1) {
    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

CrateTableModel::~CrateTableModel() {

}

void CrateTableModel::setCrate(int crateId) {
    qDebug() << "CrateTableModel::setCrate()" << crateId;
    m_iCrateId = crateId;

    QString tableName = QString("crate_%1").arg(m_iCrateId);
    QSqlQuery query;

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                  "SELECT "
                                  + columns.join(",") +
                                  " FROM library "
                                  "INNER JOIN " CRATE_TRACKS_TABLE
                                  " ON library.id = " CRATE_TRACKS_TABLE ".track_id "
                                  "INNER JOIN track_locations "
                                  " ON library.location = track_locations.id "
                                  "WHERE " CRATE_TRACKS_TABLE ".crate_id = %2 AND ("
                                  + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")");
    queryString = queryString.arg(tableName).arg(crateId);
    query.prepare(queryString);

    if (!query.exec()) {
        // TODO(XXX) feedback
        qDebug() << "Error creating temporary view for crate "
                 << crateId << ":" << query.executedQuery() << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));
    // BaseSqlTableModel sets up the header names
    initHeaderData();
    // Enable the basic filters
    slotSearch("");
    select();
}

bool CrateTableModel::addTrack(const QModelIndex& index, QString location) {
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

int CrateTableModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
}

const QLinkedList<int> CrateTableModel::getTrackRows(int trackId) const {
    return BaseSqlTableModel::getTrackRows(trackId);
}

TrackPointer CrateTableModel::getTrack(const QModelIndex& index) const {
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    return m_pTrackCollection->getTrackDAO().getTrack(trackId);
}

QString CrateTableModel::getTrackLocation(const QModelIndex& index) const {
    //const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
    //QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    QString location = m_pTrackCollection->getTrackDAO().getTrackLocation(trackId);
    return location;
}

void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    const int trackIdIndex = fieldIndex(LIBRARYTABLE_ID);

    QList<int> trackIds;
    foreach (QModelIndex index, indices) {
        int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
        trackIds.append(trackId);
    }

    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    foreach (int trackId, trackIds) {
        crateDao.removeTrackFromCrate(trackId, m_iCrateId);
    }

    select();
}

void CrateTableModel::removeTrack(const QModelIndex& index) {
    const int trackIdIndex = fieldIndex(LIBRARYTABLE_ID);
    int trackId = index.sibling(index.row(), trackIdIndex).data().toInt();
    if (m_pTrackCollection->getCrateDAO().removeTrackFromCrate(trackId, m_iCrateId)) {
        select();
    } else {
        // TODO(XXX) feedback
    }
}

void CrateTableModel::moveTrack(const QModelIndex& sourceIndex,
                                const QModelIndex& destIndex) {
    return;
}

void CrateTableModel::search(const QString& searchText) {
    // qDebug() << "CrateTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void CrateTableModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

const QString CrateTableModel::currentSearch() {
    return BaseSqlTableModel::currentSearch();
}

bool CrateTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED)) {
        return true;
    }
    return false;
}
bool CrateTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY))
        return true;
    return false;
}


QMimeData* CrateTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else
                    urls.append(url);
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

QItemDelegate* CrateTableModel::delegateForColumn(int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags CrateTableModel::getCapabilities() const {
    return TRACKMODELCAPS_RECEIVEDROPS | TRACKMODELCAPS_ADDTOPLAYLIST |
            TRACKMODELCAPS_ADDTOCRATE | TRACKMODELCAPS_ADDTOAUTODJ;
}
