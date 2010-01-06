#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"


PlaylistTableModel::PlaylistTableModel(QObject* parent,
                                       TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.playlist"),
          QSqlTableModel(parent, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_playlistDao(m_pTrackCollection->getPlaylistDAO()),
          m_trackDao(m_pTrackCollection->getTrackDAO()),
          m_iPlaylistId(-1),
          m_currentSearch("") {
    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

PlaylistTableModel::~PlaylistTableModel() {
}


void PlaylistTableModel::setPlaylist(int playlistId)
{
    qDebug() << "PlaylistTableModel::setPlaylist" << playlistId;
    m_iPlaylistId = playlistId;

    QString playlistTableName = "playlist_" + QString("%1").arg(m_iPlaylistId);

    QSqlQuery query;
    //query.prepare("DROP VIEW " + playlistTableName);
    //query.exec();

    //Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistTableName);

    query.prepare("CREATE TEMPORARY VIEW " + driver->formatValue(playlistNameField) + " AS "
                  "SELECT " +
                  "PlaylistTracks." + PLAYLISTTRACKSTABLE_POSITION + "," +
                  //"playlist_id, " + //DEBUG
                  "library." + LIBRARYTABLE_ID + "," +
                  "library." + LIBRARYTABLE_ARTIST + "," +
                  "library." + LIBRARYTABLE_TITLE + "," +
                  "library." + LIBRARYTABLE_ALBUM + "," +
                  "library." + LIBRARYTABLE_YEAR + "," +
                  "library." + LIBRARYTABLE_DURATION + "," +
                  "library." + LIBRARYTABLE_GENRE + "," +
                  "library." + LIBRARYTABLE_TRACKNUMBER + "," +
                  "library." + LIBRARYTABLE_DATETIMEADDED + "," +
                  "library." + LIBRARYTABLE_BPM + ","
                  "track_locations.location,"
                  "library." + LIBRARYTABLE_COMMENT + "," +
                  "library." + LIBRARYTABLE_MIXXXDELETED + " " +
                  "FROM library "
                  "INNER JOIN PlaylistTracks "
                  "ON library.id = PlaylistTracks.track_id "
                  "INNER JOIN track_locations "
                  "ON library.location = track_locations.id "
                  "WHERE PlaylistTracks.playlist_id = " + QString("%1").arg(playlistId) + " "
                  "ORDER BY PlaylistTracks.position ");
    //query.bindValue(":playlist_name", playlistTableName);
    //query.bindValue(":playlist_id", m_iPlaylistId);
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //qDebug() << query.executedQuery();

    //Print out any SQL error, if there was one.
    /*
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }*/

    setTable(playlistTableName);

    //Set the column heading labels, rename them for translations and have
    //proper capitalization
    setHeaderData(fieldIndex(PLAYLISTTRACKSTABLE_POSITION),
                  Qt::Horizontal, tr("#"));
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
    setHeaderData(fieldIndex(LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));

    slotSearch("");

    select(); //Populate the data model.

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}


void PlaylistTableModel::addTrack(const QModelIndex& index, QString location)
{
    //Note: The model index is ignored when adding to the library track collection.
    //      The position in the library is determined by whatever it's being sorted by,
    //      and there's no arbitrary "unsorted" view.
    const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumnIndex).data().toInt();

    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this playlist.
    if (!m_trackDao.trackExistsInDatabase(location))
    {
        m_trackDao.addTrack(location);
    }
    int trackId = m_trackDao.getTrackId(location);

    // Do nothing if the location still isn't in the database.
    if (trackId == -1)
        return;

    m_playlistDao.insertTrackIntoPlaylist(trackId, m_iPlaylistId, position);
    select(); //Repopulate the data model.

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}

TrackInfoObject* PlaylistTableModel::getTrack(const QModelIndex& index) const
{
    //FIXME: use position instead of location for playlist tracks?

    //const int locationColumnIndex = this->fieldIndex(LIBRARYTABLE_LOCATION);
    //QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
    int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
    return m_trackDao.getTrack(trackId);
}

QString PlaylistTableModel::getTrackLocation(const QModelIndex& index) const
{
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

void PlaylistTableModel::removeTrack(const QModelIndex& index)
{
    const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
    m_playlistDao.removeTrackFromPlaylist(m_iPlaylistId, position);
    select(); //Repopulate the data model.

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}

void PlaylistTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    QSqlRecord sourceRecord = this->record(sourceIndex.row());
    //sourceRecord.setValue("position", destIndex.row());
    //this->removeRows(sourceIndex.row(), 1);


    //this->insertRecord(destIndex.row(), sourceRecord);

    //TODO: execute a real query to DELETE the sourceIndex.row() row from the PlaylistTracks table.
    //int newPosition = destIndex.row();
    //int oldPosition = sourceIndex.row();
    //const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
    //int newPosition = index.sibling(destIndex.row(), positionColumnIndex).data().toInt();
    //int oldPosition = index.sibling(sourceIndex.row(), positionColumnIndex).data().toInt();
    int newPosition = this->record(destIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();
    int oldPosition = this->record(sourceIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();

    qDebug() << "old pos" << oldPosition << "new pos" << newPosition;

    //Invalid for the position to be 0 or less.
    if (newPosition < 0)
        return;
    else if (newPosition == 0) //Dragged out of bounds, which is past the end of the rows...
        newPosition = rowCount();

    //Start the transaction
    m_pTrackCollection->getDatabase().transaction();

    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query;

    //Insert the song into the PlaylistTracks table

    /** ALGORITHM for code below
      Case 1: destination < source (newPos < oldPos)
        1) Set position = -1 where pos=source -- Gives that track a dummy index to keep stuff simple.
        2) Decrement position where pos > source
        3) increment position where pos > dest
        4) Set position = dest where pos=-1 -- Move track from dummy pos to final destination.

      Case 2: destination > source (newPos > oldPos)
        1) Set position=-1 where pos=source -- Give track a dummy index again.
        2) Decrement position where pos > source AND pos <= dest
        3) Set postion=dest where pos=-1 -- Move that track from dummy pos to final destination
    */

    QString queryString;
    if (newPosition < oldPosition) {
        queryString =
            QString("UPDATE PlaylistTracks SET position=-1 "
                    "WHERE position=%1 AND "
                    "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        query.exec(queryString);
        //qDebug() << queryString;

        queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                            "WHERE position > %1 AND "
                            "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=position+1 "
                            "WHERE position >= %1 AND " //position < %2 AND "
                            "playlist_id=%3").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=%1 "
                            "WHERE position=-1 AND "
                            "playlist_id=%2").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);
    }
    else if (newPosition > oldPosition)
    {
        queryString = QString("UPDATE PlaylistTracks SET position=-1 "
                              "WHERE position = %1 AND "
                              "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        //qDebug() << queryString;
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                              "WHERE position > %1 AND position <= %2 AND "
                              "playlist_id=%3").arg(oldPosition).arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=%1 "
                              "WHERE position=-1 AND "
                              "playlist_id=%2").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);
    }

    m_pTrackCollection->getDatabase().commit();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }

    select();

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}

void PlaylistTableModel::search(const QString& searchText) {
    // qDebug() << "PlaylistTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void PlaylistTableModel::slotSearch(const QString& searchText)
{
    //FIXME: Need to keep filtering by playlist_id too
    //SQL is "playlist_id = " + QString(m_iPlaylistId)
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

    // setFilter() calls select() implicitly, so we have to fetchMore to prevent
    // locking the database.

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}

const QString PlaylistTableModel::currentSearch() {
    return m_currentSearch;
}

bool PlaylistTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED))
        return true;
    return false;
}

QMimeData* PlaylistTableModel::mimeData(const QModelIndexList &indexes) const {
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

Qt::ItemFlags PlaylistTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform widget to
    //load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

QItemDelegate* PlaylistTableModel::delegateForColumn(const int i) {
    return NULL;
}

QVariant PlaylistTableModel::data(const QModelIndex& item, int role) const {
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

TrackModel::CapabilitiesFlags PlaylistTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_RECEIVEDROPS | TRACKMODELCAPS_REORDER;
}
