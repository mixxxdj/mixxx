#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/librarytablemodel.h"

#include "mixxxutils.cpp"

const QString LibraryTableModel::DEFAULT_LIBRARYFILTER = "mixxx_deleted=0 AND fs_deleted=0";

LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.library"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_trackDao(pTrackCollection->getTrackDAO()) {

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID;

    QSqlQuery query(pTrackCollection->getDatabase());
    QString queryString = "CREATE TEMPORARY VIEW IF NOT EXISTS library_view AS SELECT "
            + columns.join(",") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    qDebug() << "Creating View:" << queryString;
    query.prepare(queryString);
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    // Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    setTable("library_view", LIBRARYTABLE_ID, tableColumns,
             pTrackCollection->getTrackSource("default"));

    //Set up a relation which maps our location column (which is a foreign key
    //into the track_locations) table. We tell Qt that our LIBRARYTABLE_LOCATION
    //column maps into the row of the track_locations table that has the id
    //equal to our location col. It then grabs the "location" col from that row
    //and shows it...
    //setRelation(fieldIndex(LIBRARYTABLE_LOCATION), QSqlRelation("track_locations", "id", "location"));


    // BaseSqlTabelModel will setup the header info
    initHeaderData();

    //Sets up the table filter so that we don't show "deleted" tracks (only show mixxx_deleted=0).
    slotSearch("");

    select(); //Populate the data model.

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

LibraryTableModel::~LibraryTableModel()
{

}

bool LibraryTableModel::addTrack(const QModelIndex& index, QString location)
{
    //Note: The model index is ignored when adding to the library track collection.
    //      The position in the library is determined by whatever it's being sorted by,
    //      and there's no arbitrary "unsorted" view.
    QFileInfo fileInfo(location);

    int trackId = m_trackDao.getTrackId(fileInfo.absoluteFilePath());
    if (trackId >= 0) {
        //If the track is already in the library, make sure it's marked as
        //not deleted. (This lets the user unremove a track from the library
        //by dragging-and-dropping it back into the library view.)
        m_trackDao.unremoveTrack(trackId);
        select();
        return true;
    }

    trackId = m_trackDao.addTrack(fileInfo);
    if (trackId >= 0) {
        select(); //Repopulate the data model.
        return true;
    }
    return false;
}

TrackPointer LibraryTableModel::getTrack(const QModelIndex& index) const
{
    int trackId = getTrackId(index);
    return m_trackDao.getTrack(trackId);
}

QString LibraryTableModel::getTrackLocation(const QModelIndex& index) const
{
    const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
    QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
    return location;
}

int LibraryTableModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
}

const QLinkedList<int> LibraryTableModel::getTrackRows(int trackId) const {
    return BaseSqlTableModel::getTrackRows(trackId);
}

void LibraryTableModel::removeTracks(const QModelIndexList& indices) {
    QList<int> trackIds;

    foreach (QModelIndex index, indices) {
        int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
        trackIds.append(trackId);
    }

    m_trackDao.removeTracks(trackIds);

    select(); //Repopulate the data model.
}

void LibraryTableModel::removeTrack(const QModelIndex& index)
{
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    m_trackDao.removeTrack(trackId);
    select(); //Repopulate the data model.
}

void LibraryTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    //Does nothing because we don't support reordering tracks in the library,
    //and getCapabilities() reports that.
}

void LibraryTableModel::search(const QString& searchText) {
    // qDebug() << "LibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void LibraryTableModel::slotSearch(const QString& searchText) {
    // qDebug() << "slotSearch()" << searchText << QThread::currentThread();
    BaseSqlTableModel::search(searchText);
}

const QString LibraryTableModel::currentSearch() {
    //qDebug() << "LibraryTableModel::currentSearch(): " << m_currentSearch;
    return BaseSqlTableModel::currentSearch();
}

bool LibraryTableModel::isColumnInternal(int column) {
    if ((column == fieldIndex(LIBRARYTABLE_ID)) ||
        (column == fieldIndex(LIBRARYTABLE_URL)) ||
        (column == fieldIndex(LIBRARYTABLE_CUEPOINT)) ||
        (column == fieldIndex(LIBRARYTABLE_REPLAYGAIN)) ||
        (column == fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX)) ||
        (column == fieldIndex(LIBRARYTABLE_SAMPLERATE)) ||
        (column == fieldIndex(LIBRARYTABLE_MIXXXDELETED)) ||
        (column == fieldIndex(LIBRARYTABLE_HEADERPARSED)) ||
        (column == fieldIndex(LIBRARYTABLE_PLAYED)) ||
        (column == fieldIndex(LIBRARYTABLE_CHANNELS)) ||
        (column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED))) {
        return true;
    }
    return false;
}

bool LibraryTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY))
        return true;
    return false;
}

QItemDelegate* LibraryTableModel::delegateForColumn(const int i) {
    return NULL;
}

QMimeData* LibraryTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) //Only add a URL once per row.
            {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else {
                    urls.append(url);
                }
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

TrackModel::CapabilitiesFlags LibraryTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_RECEIVEDROPS | TRACKMODELCAPS_ADDTOPLAYLIST |
            TRACKMODELCAPS_ADDTOCRATE | TRACKMODELCAPS_ADDTOAUTODJ;
}

/*
void LibraryTableModel::notifyBeginInsertRow(int row)
{

    QModelIndex foo = index(0, 0);
    beginInsertRows(foo, row, row);
}

void LibraryTableModel::notifyEndInsertRow(int row)
{
    endInsertRows();
}
*/
