#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/missingtablemodel.h"
#include "library/librarytablemodel.h"

#include "mixxxutils.cpp"

const QString MissingTableModel::MISSINGFILTER = "mixxx_deleted=0 AND fs_deleted=1";

MissingTableModel::MissingTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.missing"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_trackDao(m_pTrackCollection->getTrackDAO()) {

    QSqlQuery query;
    //query.prepare("DROP VIEW " + playlistTableName);
    //query.exec();
    QString tableName("missing_songs");

    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " + tableName + " AS "
                  "SELECT " +
                  "library." + LIBRARYTABLE_ID + "," +
                  "library." + LIBRARYTABLE_PLAYED + "," +
                  "library." + LIBRARYTABLE_TIMESPLAYED + "," +
                  "library." + LIBRARYTABLE_ARTIST + "," +
                  "library." + LIBRARYTABLE_TITLE + "," +
                  "library." + LIBRARYTABLE_ALBUM + "," +
                  "library." + LIBRARYTABLE_YEAR + "," +
                  "library." + LIBRARYTABLE_DURATION + "," +
                  "library." + LIBRARYTABLE_RATING + "," +
                  "library." + LIBRARYTABLE_GENRE + "," +
                  "library." + LIBRARYTABLE_FILETYPE + "," +
                  "library." + LIBRARYTABLE_TRACKNUMBER + "," +
                  "library." + LIBRARYTABLE_KEY + "," +
                  "library." + LIBRARYTABLE_DATETIMEADDED + "," +
                  "library." + LIBRARYTABLE_BPM + "," +
                  "track_locations.location," +
                  "track_locations.fs_deleted," +
                  "library." + LIBRARYTABLE_COMMENT + "," +
                  "library." + LIBRARYTABLE_MIXXXDELETED + " "
                  "FROM library "
                  "INNER JOIN track_locations "
                  "ON library.location=track_locations.id "
                  "WHERE track_locations.fs_deleted=1 ");
    //query.bindValue(":playlist_name", playlistTableName);
    //query.bindValue(":playlist_id", m_iPlaylistId);
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    setTable(tableName);

    qDebug() << "Created MissingTracksModel!";

    initHeaderData();    //derived from BaseSqlModel

    slotSearch("");

    select(); //Populate the data model.

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

}

MissingTableModel::~MissingTableModel() {
}

bool MissingTableModel::addTrack(const QModelIndex& index, QString location)
{
    return false;
}

TrackPointer MissingTableModel::getTrack(const QModelIndex& index) const
{
    //FIXME: use position instead of location for playlist tracks?

    //const int locationColumnIndex = this->fieldIndex(LIBRARYTABLE_LOCATION);
    //QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    return m_trackDao.getTrack(trackId);
}

QString MissingTableModel::getTrackLocation(const QModelIndex& index) const
{
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    QString location = m_trackDao.getTrackLocation(trackId);
    return location;
}

void MissingTableModel::removeTrack(const QModelIndex& index)
{
}

void MissingTableModel::removeTracks(const QModelIndexList& indices)
{
}

void MissingTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
}

void MissingTableModel::search(const QString& searchText)
{
    // qDebug() << "MissingTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void MissingTableModel::slotSearch(const QString& searchText) {
    if (!m_currentSearch.isNull() && m_currentSearch == searchText)
        return;
    m_currentSearch = searchText;

    QString filter;
    if (searchText == "")
        filter = "(" + MissingTableModel::MISSINGFILTER + ")";
    else {
        QSqlField search("search", QVariant::String);
        search.setValue("%" + searchText + "%");
        QString escapedText = database().driver()->formatValue(search);
        filter = "(" + MissingTableModel::MISSINGFILTER + " AND " +
                "(artist LIKE " + escapedText + " OR " +
                "album LIKE " + escapedText + " OR " +
                "title LIKE " + escapedText + "))";
    }
    setFilter(filter);
}

const QString MissingTableModel::currentSearch() {
    return m_currentSearch;
}

bool MissingTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_PLAYED) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED))
        return true;
    else
        return false;
}

QMimeData* MissingTableModel::mimeData(const QModelIndexList &indexes) const {
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

/** Override flags from BaseSqlModel since we don't want edit this model */
Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const
{
    return readOnlyFlags(index);
}

QItemDelegate* MissingTableModel::delegateForColumn(const int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags MissingTableModel::getCapabilities() const
{
    return 0;
}
