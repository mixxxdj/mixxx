// cratetablemodel.cpp
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/cratetablemodel.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/dao/cratedao.h"

#include "mixxxutils.cpp"

CrateTableModel::CrateTableModel(QObject* pParent, TrackCollection* pTrackCollection)
        : BaseSqlTableModel(pParent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            "mixxx.db.model.crate"),
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
    columns << CRATETRACKSTABLE_TRACKID;

    QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                  "SELECT %2 FROM " CRATE_TRACKS_TABLE
                                  " WHERE " + CRATETRACKSTABLE_CRATEID +
                                  " = %3");
    queryString = queryString
            .arg(tableName)
            .arg(columns.join(","))
            .arg(crateId);
    query.prepare(queryString);

    if (!query.exec()) {
        // TODO(XXX) feedback
        qDebug() << "Error creating temporary view for crate "
                 << crateId << ":" << query.executedQuery()
                 << query.lastError();
    }

    QStringList tableColumns;
    setTable(tableName, columns[0], columns,
             m_pTrackCollection->getTrackSource("default"));
    // BaseSqlTableModel sets up the header names
    initHeaderData();
    // Enable the basic filters
    setSearch("", LibraryTableModel::DEFAULT_LIBRARYFILTER);
}

bool CrateTableModel::addTrack(const QModelIndex& index, QString location) {
    QFileInfo fileInfo(location);
    location = fileInfo.absoluteFilePath();

    TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
    int iTrackId = trackDao.getTrackId(location);

    // If the track is not in the library, add it
    if (iTrackId < 0)
        iTrackId = trackDao.addTrack(fileInfo);

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
                 << location << "to crate" << m_iCrateId;
        return false;
    }
}

TrackPointer CrateTableModel::getTrack(const QModelIndex& index) const {
    int trackId = getTrackId(index);
    return m_pTrackCollection->getTrackDAO().getTrack(trackId);
}

void CrateTableModel::removeTracks(const QModelIndexList& indices) {
    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    bool locked = crateDao.isCrateLocked(m_iCrateId);

    if (!locked) {
        QList<int> trackIds;
        foreach (QModelIndex index, indices) {
            trackIds.append(getTrackId(index));
        }
        foreach (int trackId, trackIds) {
            crateDao.removeTrackFromCrate(trackId, m_iCrateId);
        }
        select();
    }
}

void CrateTableModel::removeTrack(const QModelIndex& index) {
    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    bool locked = crateDao.isCrateLocked(m_iCrateId);

    if (!locked) {
        int trackId = getTrackId(index);
        if (m_pTrackCollection->getCrateDAO().
            removeTrackFromCrate(trackId, m_iCrateId)) {
            select();
        } else {
            // TODO(XXX) feedback
        }
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
    BaseSqlTableModel::search(
        searchText, LibraryTableModel::DEFAULT_LIBRARYFILTER);
}

bool CrateTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(CRATETRACKSTABLE_TRACKID) ||
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

QItemDelegate* CrateTableModel::delegateForColumn(int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags CrateTableModel::getCapabilities() const {
    CapabilitiesFlags caps =  TRACKMODELCAPS_RECEIVEDROPS  |
                              TRACKMODELCAPS_ADDTOPLAYLIST |
                              TRACKMODELCAPS_ADDTOCRATE    |
                              TRACKMODELCAPS_ADDTOAUTODJ   |
                              TRACKMODELCAPS_RELOADMETADATA;

    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    bool locked = crateDao.isCrateLocked(m_iCrateId);

    if (locked) {
        caps |= TRACKMODELCAPS_LOCKED;
    }

    return caps;
}
