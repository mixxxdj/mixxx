// cratetablemodel.cpp
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/cratetablemodel.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/dao/cratedao.h"

CrateTableModel::CrateTableModel(QObject* pParent, TrackCollection* pTrackCollection)
        : BaseSqlTableModel(pParent, pTrackCollection->getDatabase()),
          TrackModel(pTrackCollection->getDatabase(), "mixxx.db.model.crate"),
          m_pTrackCollection(pTrackCollection),
          m_iCrateId(-1),
          m_currentSearch("") {
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

    QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                  "SELECT "
                                  "library." + LIBRARYTABLE_ID + "," +
                                  LIBRARYTABLE_ARTIST + "," +
                                  LIBRARYTABLE_TITLE + "," +
                                  LIBRARYTABLE_ALBUM + "," +
                                  LIBRARYTABLE_YEAR + "," +
                                  LIBRARYTABLE_DURATION + "," +
                                  LIBRARYTABLE_GENRE + "," +
                                  LIBRARYTABLE_TRACKNUMBER + "," +
                                  LIBRARYTABLE_BPM + "," +
                                  LIBRARYTABLE_DATETIMEADDED + ","
                                  "track_locations.location," +
                                  LIBRARYTABLE_COMMENT + "," +
                                  LIBRARYTABLE_MIXXXDELETED + " " +
                                  "FROM library "
                                  "INNER JOIN " CRATE_TRACKS_TABLE
                                  " ON library.id = " CRATE_TRACKS_TABLE ".track_id "
                                  "INNER JOIN track_locations "
                                  " ON library.location = track_locations.id "
                                  "WHERE " CRATE_TRACKS_TABLE ".crate_id = %2");
    queryString = queryString.arg(tableName).arg(crateId);
    query.prepare(queryString);

    if (!query.exec()) {
        // TODO(XXX) feedback
        qDebug() << "Error creating temporary view for crate "
                 << crateId << ":" << query.executedQuery() << query.lastError();
    }

    setTable(tableName);

    // Enable the basic filters
    slotSearch("");

    select();

    setHeaderData(fieldIndex(LIBRARYTABLE_ID),
                  Qt::Horizontal, tr("ID"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ARTIST),
                  Qt::Horizontal, tr("Artist"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TITLE),
                  Qt::Horizontal, tr("Title"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ALBUM),
                  Qt::Horizontal, tr("Album"));
    setHeaderData(fieldIndex(LIBRARYTABLE_GENRE),
                  Qt::Horizontal, tr("Genre"));
    setHeaderData(fieldIndex(LIBRARYTABLE_YEAR),
                  Qt::Horizontal, tr("Year"));
    setHeaderData(fieldIndex("location"),
                  Qt::Horizontal, tr("Location"));
    setHeaderData(fieldIndex(LIBRARYTABLE_COMMENT),
                  Qt::Horizontal, tr("Comment"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DURATION),
                  Qt::Horizontal, tr("Duration"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TRACKNUMBER),
                  Qt::Horizontal, tr("Track #"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BITRATE),
                  Qt::Horizontal, tr("Bitrate"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));
}

void CrateTableModel::addTrack(const QModelIndex& index, QString location) {
    int iTrackId = m_pTrackCollection->getTrackDAO().getTrackId(location);
    bool success = false;
    if (iTrackId >= 0) {
        success = m_pTrackCollection->getCrateDAO().addTrackToCrate(iTrackId,
                                                                    m_iCrateId);
    }

    if (success) {
        select();
    } else {
        // TODO(XXX) feedback
        qDebug() << "CrateTableModel::addTrack could not add track"
                 << location << "to crate" << m_iCrateId;
    }
}

TrackInfoObject* CrateTableModel::getTrack(const QModelIndex& index) const {
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
    m_currentSearch = searchText;

    QString filter;
    if (searchText == "")
        filter = "(" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    else {
        QSqlField search("search", QVariant::String);
        search.setValue("%" + searchText + "%");
        QString escapedText = database().driver()->formatValue(search);
        filter = "(" + LibraryTableModel::DEFAULT_LIBRARYFILTER + " AND " +
                "(artist LIKE " + escapedText + " OR "
                "title  LIKE " + escapedText + "))";
    }

    setFilter(filter);
}

const QString CrateTableModel::currentSearch() {
    return m_currentSearch;
}

bool CrateTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED)) {
        return true;
    }
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
                QUrl url(getTrackLocation(index));
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

Qt::ItemFlags CrateTableModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform
    //widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

QItemDelegate* CrateTableModel::delegateForColumn(int i) {
    return NULL;
}

QVariant CrateTableModel::data(const QModelIndex& item, int role) const {
    if (!item.isValid())
        return QVariant();

    QVariant value = QSqlTableModel::data(item, role);

    if (role == Qt::DisplayRole &&
        item.column() == fieldIndex(LIBRARYTABLE_DURATION)) {
        if (qVariantCanConvert<int>(value)) {
            // TODO(XXX) Pull this out into a MixxxUtil or something.

            //Let's reformat this song length into a human readable MM:SS format.
            int totalSeconds = qVariantValue<int>(value);
            int seconds = totalSeconds % 60;
            int mins = totalSeconds / 60;
            //int hours = mins / 60; //Not going to worry about this for now. :)

            //Construct a nicely formatted duration string now.
            value = QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
        }
    }
    return value;
}

TrackModel::CapabilitiesFlags CrateTableModel::getCapabilities() const {
    return TRACKMODELCAPS_RECEIVEDROPS;
}
