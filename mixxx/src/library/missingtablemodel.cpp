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
                  "library." + LIBRARYTABLE_ARTIST + "," +
                  "library." + LIBRARYTABLE_TITLE + "," +
                  "library." + LIBRARYTABLE_ALBUM + "," +
                  "library." + LIBRARYTABLE_YEAR + "," +
                  "library." + LIBRARYTABLE_DURATION + "," +
                  "library." + LIBRARYTABLE_GENRE + "," +
                  "library." + LIBRARYTABLE_FILETYPE + "," +
                  "library." + LIBRARYTABLE_TRACKNUMBER + "," +
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

    //Set the column heading labels, rename them for translations and have
    //proper capitalization
    setHeaderData(fieldIndex("track_locations.location"),
                  Qt::Horizontal, tr("Location"));
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
    setHeaderData(fieldIndex(LIBRARYTABLE_FILETYPE),
                  Qt::Horizontal, tr("Type"));
    setHeaderData(fieldIndex(LIBRARYTABLE_LOCATION),
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

Qt::ItemFlags MissingTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    //defaultFlags |= Qt::ItemIsDragEnabled;
    //defaultFlags = 0;

    return defaultFlags;
}

QItemDelegate* MissingTableModel::delegateForColumn(const int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags MissingTableModel::getCapabilities() const
{
    return 0;
}
